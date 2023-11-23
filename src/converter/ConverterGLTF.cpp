#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ConverterGLTF.h"
#include "util.h"
#include <lodepng.h>

#include <fstream>
#include <algorithm>
#include <climits>
#include <cfloat>

ConverterGLTF::ConverterGLTF()
{
    currentAccessor = 0;
    currentBufView = 0;
}

template<class T>
int ConverterGLTF::putValue(std::vector<BYTE>& buffer, T val)
{
    BYTE* ptr = reinterpret_cast<BYTE*>(&val);
    BYTE singleByte;

    for (int i = 0; i < sizeof(T); i++)
    {
        singleByte = *ptr;
        buffer.push_back(singleByte);
        ptr++;
    }

    return sizeof(T);
}

bool ConverterGLTF::insertImageTexture(rw::NativeTexture* txdTex)
{
    tinygltf::Image imageTexture;

    BYTE* pngEncoded = NULL;
    size_t pngSize = 0;

    txdTex->decompressDxt(); //decomprimere prima

    int w = txdTex->width[0];
    int h = txdTex->height[0];
    int d = txdTex->depth / 8;

    //da BGR a RGB
    if (d == 3)
    {
        BYTE* rgbImage = new BYTE[w * h * d];
        BYTE* bgrImage = txdTex->texels[0];

        for (int i = 0; i < w * h; i++)
        {
            rgbImage[i * d] = bgrImage[i * d + 2]; //INVERSIONE
            rgbImage[i * d + 1] = bgrImage[i * d + 1];
            rgbImage[i * d + 2] = bgrImage[i * d]; //INVERSIONE
        }

        lodepng_encode24(&pngEncoded, &pngSize, rgbImage, w, h);

        delete rgbImage;
    }
    else if (d == 4) //da BGRA a RGBA
    {
        BYTE* rgbaImage = new BYTE[w * h * d];
        BYTE* bgraImage = txdTex->texels[0];

        for (int i = 0; i < w * h; i++)
        {
            rgbaImage[i * d] = bgraImage[i * d + 2]; //INVERSIONE
            rgbaImage[i * d + 1] = bgraImage[i * d + 1];
            rgbaImage[i * d + 2] = bgraImage[i * d]; //INVERSIONE
            rgbaImage[i * d + 3] = bgraImage[i * d + 3];
        }

        lodepng_encode32(&pngEncoded, &pngSize, rgbaImage, w, h);

        delete rgbaImage;
    }


    if (pngSize > 0)
    {
        std::string base64Png = base64_encode(pngEncoded, pngSize);

        imageTexture.uri = "data:image/png;base64,";
        imageTexture.uri += base64Png;

        free(pngEncoded);
    }

    model.images.push_back(imageTexture);

    return true;
}

void ConverterGLTF::putRotationTranslation(rw::Frame* frame, tinygltf::Node& node)
{
    //devo convertire la matrice 3x3 in quaternione
    float* mat = frame->rotationMatrix;
    float* pos = frame->position;
    float qw, qx, qy, qz;

    convertMat3x3ToQuaternion(mat, qx, qy, qz, qw);
    convertEulerToQuaternion(90, 90, 0, qx, qy, qz, qw);

    node.rotation = { qx, qy, qz, qw };
    node.translation = { pos[0], pos[1], pos[2] };
}

void ConverterGLTF::InsertAccBufView(int byteOffset, int byteLength, int target, int componentType, int count, int type, std::vector<double> min, std::vector<double> max)
{
    tinygltf::BufferView bufView;
    tinygltf::Accessor accessor;

    bufView.buffer = 0;
    bufView.byteOffset = byteOffset;
    bufView.byteLength = byteLength;
    bufView.target = target;

    accessor.bufferView = currentBufView;
    accessor.byteOffset = 0;
    accessor.componentType = componentType;
    accessor.count = count;
    accessor.type = type;
    accessor.minValues = min;
    accessor.maxValues = max;

    model.accessors.push_back(accessor);
    model.bufferViews.push_back(bufView);

    currentAccessor++;
    currentBufView++;
}

int ConverterGLTF::insertInverseMatBones(std::vector<BYTE>& buffer, tinygltf::Skin& skin, rw::Geometry* geometry, int bytesOffset)
{
    int bytesLength = 0;

    for (int i = 0; i < geometry->inverseMatrices.size(); i++)
    {
        //ogni 16 valori � una matrice nuova 4x4
        float valueMat = geometry->inverseMatrices[i];

        //per evitare che il rango della matrice diventi 3 
        if ((i + 1) % 16 == 0 && valueMat == 0)
            valueMat = 1;
        
        bytesLength += putValue(buffer, valueMat);


    }

    InsertAccBufView(
        bytesOffset,
        bytesLength,
        0,
        TINYGLTF_COMPONENT_TYPE_FLOAT,
        geometry->inverseMatrices.size() / 16,
        TINYGLTF_TYPE_MAT4);

    skin.inverseBindMatrices = currentAccessor - 1;

    return bytesLength;
}

int ConverterGLTF::insertBones(std::vector<BYTE>& buffer, tinygltf::Mesh& mesh, rw::Geometry* geometry, int bytesOffset)
{
    int bytesLength = 0;
    int bytesInds = 0;
    int bytesWeights = 0;

    if (geometry->hasSkin && model.nodes.size() > 0)
    {
        tinygltf::Primitive primitiveSkeleton;

        int indexNode = model.nodes.size() - 1;
        int numVerts = geometry->vertexBoneIndices.size();

        tinygltf::Skin skin;
        model.nodes[indexNode].skin = 0;

        //AGGIUNTO MATRICI INVERSE PER OSSA
        bytesLength += insertInverseMatBones(buffer, skin, geometry, bytesOffset);
        
        int firstBone = 0;

        //JOINTS SCHELETRO
        for (int i = 0; i < geometry->boneCount; i++)
        {
            tinygltf::Node boneNode;
            int indexJoint = i + indexNode + 1;

            skin.joints.push_back(indexJoint);

            boneNode.name = "Bone";
            boneNode.name += std::to_string(i + 1);

            if (i > 0)
                model.nodes[firstBone].children.push_back(firstBone + i);
            else if (i == 0)
            {
                firstBone = model.nodes.size();
                skin.skeleton = firstBone;
            }

            model.nodes.push_back(boneNode);
        }

        model.scenes[0].nodes.push_back(firstBone);

        //INDICI OSSA PER VERTICE
        for (int i = 0; i < numVerts; i++)
        {
            //ogni vertice pu� essere pesato da 4 ossa massimo
            //devo espandere ogni byte in un unsigned short (2 bytes)
            BYTE* curBones = reinterpret_cast<BYTE*>(&geometry->vertexBoneIndices[i]);
            USHORT boneVal;
            int weightIndex = i * 4;

            boneVal = (geometry->vertexBoneWeights[weightIndex] > 0) ? *curBones : 0;
            bytesInds += putValue(buffer, boneVal);

            boneVal = (geometry->vertexBoneWeights[weightIndex + 1] > 0) ? *(curBones + 1) : 0;
            bytesInds += putValue(buffer, boneVal);

            boneVal = (geometry->vertexBoneWeights[weightIndex + 2] > 0) ? *(curBones + 2) : 0;
            bytesInds += putValue(buffer, boneVal);

            boneVal = (geometry->vertexBoneWeights[weightIndex + 3] > 0) ? *(curBones + 3) : 0;
            bytesInds += putValue(buffer, boneVal);
        }

        InsertAccBufView(
            bytesOffset + bytesLength,
            bytesInds,
            TINYGLTF_TARGET_ARRAY_BUFFER,
            TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,
            numVerts,
            TINYGLTF_TYPE_VEC4);

        int accessorJoints = currentAccessor - 1;

        bytesLength += bytesInds;

        //PESI OSSA PER VERTICE
        for (int i = 0; i < numVerts * 4; i++)
        {
            float weight = geometry->vertexBoneWeights[i];

            bytesWeights += putValue(buffer, weight);
        }

        InsertAccBufView(
            bytesOffset + bytesLength,
            bytesWeights,
            TINYGLTF_TARGET_ARRAY_BUFFER,
            TINYGLTF_COMPONENT_TYPE_FLOAT,
            numVerts,
            TINYGLTF_TYPE_VEC4);

        int accessorWeights = currentAccessor - 1;

        bytesLength += bytesWeights;

        model.skins.push_back(skin);

        primitiveSkeleton.attributes["POSITION"] = 0;
        primitiveSkeleton.attributes["JOINTS_0"] = accessorJoints;
        primitiveSkeleton.attributes["WEIGHTS_0"] = accessorWeights;
        primitiveSkeleton.mode = TINYGLTF_MODE_POINTS;

        mesh.primitives.push_back(primitiveSkeleton);
    }

    return bytesLength;
}

int ConverterGLTF::insertUVTexture(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset)
{
    int bytesLength = 0;
    int numCoords = 0;

    //COORDINATE UV... da R3 a R2... i matematici capiranno...
    //U corrisponde alla X
    //V corrisponde alla Y
    //ovviamente coordinate normalizzate
    for (int i = 0; i < indeces.size(); i++, numCoords++)
    {
        int index = indeces[i];

        index /= 3; //divido per 3 perch� devo soppiantare le 3 dimensioni
        index *= 2; //moltiplico per avere la dimensionalit� giusta
        float U = geometry->texCoords[0][index];
        float V = geometry->texCoords[0][index + 1];

        bytesLength += putValue(buffer, U);
        bytesLength += putValue(buffer, V);
    }

    InsertAccBufView(
        bytesOffset,
        bytesLength,
        TINYGLTF_TARGET_ARRAY_BUFFER,
        TINYGLTF_COMPONENT_TYPE_FLOAT,
        numCoords,
        TINYGLTF_TYPE_VEC2);

    return bytesLength;
}

int ConverterGLTF::insertNormals(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset)
{
    int bytesLength = 0;

    float minX = FLT_MAX;
    float maxX = FLT_MIN;
    float minY = FLT_MAX;
    float maxY = FLT_MIN;
    float minZ = FLT_MAX;
    float maxZ = FLT_MIN;

    if (!geometry->hasNormals)
        return 0;

    //VERSORI FACCE TRIANGOLI
    for(int i = 0; i < indeces.size(); i++)
    {
        int index = indeces[i];
        float nx = geometry->normals[index];
        float ny = geometry->normals[index + 1];
        float nz = geometry->normals[index + 2];

        bytesLength += putValue(buffer, nx);
        bytesLength += putValue(buffer, ny);
        bytesLength += putValue(buffer, nz);

        checkMinMax(minX, maxX, nx);
        checkMinMax(minY, maxY, ny);
        checkMinMax(minZ, maxZ, nz);
    }

    InsertAccBufView(
        bytesOffset,
        bytesLength,
        TINYGLTF_TARGET_ARRAY_BUFFER,
        TINYGLTF_COMPONENT_TYPE_FLOAT,
        indeces.size(),
        TINYGLTF_TYPE_VEC3,
        { minX, minY, minZ },
        { maxX, maxY, maxZ });

    return bytesLength;
}

int ConverterGLTF::insertVertices(std::vector<BYTE>& buffer, std::vector<USHORT> &indeces, rw::Geometry* geometry, int bytesOffset)
{
    float minX = FLT_MAX;
    float maxX = FLT_MIN;
    float minY = FLT_MAX;
    float maxY = FLT_MIN;
    float minZ = FLT_MAX;
    float maxZ = FLT_MIN;

    int bytesLength = 0;

    for (int i = 0; i < indeces.size(); i++)
    {
        int index = indeces[i];
        float x = geometry->vertices[index]; //x
        float y = geometry->vertices[index + 1]; //y
        float z = geometry->vertices[index + 2]; //z

        bytesLength += putValue(buffer, x);
        bytesLength += putValue(buffer, y);
        bytesLength += putValue(buffer, z);

        checkMinMax(minX, maxX, x);
        checkMinMax(minY, maxY, y);
        checkMinMax(minZ, maxZ, z);
    }

    InsertAccBufView(
        bytesOffset,
        bytesLength,
        TINYGLTF_TARGET_ARRAY_BUFFER,
        TINYGLTF_COMPONENT_TYPE_FLOAT,
        indeces.size(),
        TINYGLTF_TYPE_VEC3,
        { minX, minY, minZ },
        { maxX, maxY, maxZ });

    return bytesLength;
}

int ConverterGLTF::insertIndices(std::vector<BYTE>& buffer, rw::Split* split, int bytesOffset)
{
    USHORT maxVertIndex = 0;
    USHORT minVertIndex = USHRT_MAX;
    
    int bytesLength = 0;

    int vertices = split->indices.size();

    for (int j = 0; j < vertices; j++)
    {
        USHORT v = split->indices[j];
        bytesLength += putValue(buffer, v);

        checkMinMax(minVertIndex, maxVertIndex, v);
    }

    int padding = bytesLength % 4; //aggiungere eventuali padding essendo bytesLength multiplo di 2

    for(int i = 0; i < padding; i++)
        putValue(buffer, (BYTE)0);

    bytesLength += padding;

    InsertAccBufView(
        bytesOffset,
        bytesLength,
        TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER,
        TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,
        vertices,
        TINYGLTF_TYPE_SCALAR,
        { (double)minVertIndex },
        { (double)maxVertIndex });

    return bytesLength;
}

bool ConverterGLTF::convert(char* output, char* inputDff, char* inputTxd)
{
    if (!inputDff || !inputTxd || !output)
        return false;

    std::ifstream dff(inputDff, std::ios::binary);
    std::ifstream txd(inputTxd, std::ios::binary);

    rw::Clump dffStruct;
    rw::TextureDictionary txdStruct;

    dffStruct.read(dff);
    txdStruct.read(txd);

    return convert(output, dffStruct, txdStruct);
}

bool ConverterGLTF::convert(char* output, rw::Clump& dff)
{
    rw::TextureDictionary emptyTXD;

    return convert(output, dff, emptyTXD);
}

bool ConverterGLTF::convert(char* output, rw::Clump& dff, rw::TextureDictionary& txd)
{
    if (!output)
        return false;

    if (dff.geometryList.size() <= 0)
        return false;

    resetModel();

    tinygltf::Scene scene;
    tinygltf::Mesh mesh;
    tinygltf::Primitive primitive;
    tinygltf::Node node;
    tinygltf::Buffer buffer;
    tinygltf::Asset asset;
    std::vector<USHORT> indeces;
    rw::Geometry* geometry = &dff.geometryList[0];
    rw::Frame* frame = &dff.frameList[0];

    putRotationTranslation(frame, node);

    node.mesh = 0;
    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.nodes.push_back(node);


    int splits = geometry->splits.size();
    int curBytesOffset = 0;

    int positionsIndex = -1;
    int normalsIndex = -1;
    int texCoordIndex = -1;


    materials.clear();

    //TEXTURES
    for (int i = 0; i < txd.texList.size(); i++)
    {
        rw::NativeTexture* txdTex = &txd.texList[i];
        tinygltf::Texture texture;
        tinygltf::Material mat;
        tinygltf::Sampler sampler;

        //MATERIALE
        mat.pbrMetallicRoughness.baseColorTexture.index = i;
        mat.doubleSided = true;
        mat.name = txdTex->name;
        model.materials.push_back(mat);

        materials[txdTex->name] = i;

        //TEXTURE
        texture.sampler = i;
        texture.source = i;
        model.textures.push_back(texture);

        //SAMPLER con l'algoritmo di resampling
        model.samplers.push_back(sampler);

        //IMMAGINE TEXTURE
        insertImageTexture(txdTex);
    }

    for (int i = 0; i < geometry->vertexCount; i++)
        indeces.push_back(i * 3);

    //VERTICI
    curBytesOffset += insertVertices(buffer.data, indeces, geometry, curBytesOffset);
    positionsIndex = currentAccessor - 1;
    //COORDINATE UV
    if (materials.size() > 0)
    {
        curBytesOffset += insertUVTexture(buffer.data, indeces, geometry, curBytesOffset);
        texCoordIndex = currentAccessor - 1;
    }
    //NORMALI
    curBytesOffset += insertNormals(buffer.data, indeces, geometry, curBytesOffset);
    normalsIndex = currentAccessor - 1;



    //OSSA DELLO SCHELETRO
    //curBytesOffset += insertBones(buffer.data, mesh, geometry, curBytesOffset);

    //ogni split � una parte della mesh con materiale diverso
    //prima salvo tutti i vertici, coordinate uv e normali dei triangoli A PARTE
    //poi man mano gli indici dei vertici per formare i triangoli in funzione del materiale
    for (int i = 0; i < splits; i++)
    {
        tinygltf::Primitive primitive; //un primitive per ogni split

        std::string textureName = geometry->materialList[i].texture.name;

        int material = materials[textureName];

        //INDICI TRIANGOLI
        curBytesOffset += insertIndices(buffer.data, &geometry->splits[i], curBytesOffset);
        primitive.indices = currentAccessor - 1;

        //VERTICI
        primitive.attributes["POSITION"] = positionsIndex;

        //NORMALI
        if(geometry->hasNormals)
            primitive.attributes["NORMAL"] = normalsIndex;

        //COORDINATE UV
        if (texCoordIndex >= 0)
        {
            std::string texCoord = "TEXCOORD_0";
            primitive.attributes[texCoord] = 1;
            primitive.material = material;
        }

        primitive.mode = TINYGLTF_MODE_TRIANGLES;

        mesh.primitives.push_back(primitive);
    }

    asset.version = "2.0";
    asset.generator = "dff_converter by SimoSbara (using tiny_gltf)";

    model.meshes.push_back(mesh);
    model.buffers.push_back(buffer);
    model.asset = asset;

    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&model, output,
        true, // embedImages
        true, // embedBuffers
        true, // pretty print
        false); // write binary

	return true;
}
