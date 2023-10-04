#include "ConverterGLTF.h"
#include "ximage.h"
#include "util.h"

#include <algorithm>

ConverterGLTF::ConverterGLTF()
{
    currentAccessor = 0;
    currentBufView = 0;
}

template<class T>
void ConverterGLTF::putValue(std::vector<BYTE>& buffer, T val)
{
    BYTE* ptr = reinterpret_cast<BYTE*>(&val);
    BYTE singleByte;

    for (int i = 0; i < sizeof(T); i++)
    {
        singleByte = *ptr;
        buffer.push_back(singleByte);
        ptr++;
    }
}

bool ConverterGLTF::insertImageTexture(rw::NativeTexture* txdTex)
{
    tinygltf::Image imageTexture;

    BYTE* pngEncoded = NULL;
    long pngSize = 0;

    txdTex->decompressDxt(); //decomprimere prima

    int w = txdTex->width[0];
    int h = txdTex->height[0];
    int d = txdTex->depth / 8;

    CxImage img;
    img.CreateFromArray(txdTex->texels[0], w, h, txdTex->depth, w * d, true);
    img.Encode(pngEncoded, pngSize, CXIMAGE_FORMAT_PNG);

    if (pngSize > 0)
    {
        std::string base64Png = base64_encode(pngEncoded, pngSize);

        imageTexture.uri = "data:image/png;base64,";
        imageTexture.uri += base64Png;
    }

    model.images.push_back(imageTexture);

    img.FreeMemory(pngEncoded);

    return true;
}

int ConverterGLTF::insertUVTexture(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset)
{
    int bytesLength = 0;
    int numCoords = 0;

    //COORDINATE UV... da R3 a R2... i matematici capiranno...
    //U corrisponde alla X
    //V corrisponde alla Y
    //ovviamente coordinate normalizzate
    for (int i = 0; i < indeces.size(); i++)
    {
        int index = indeces[i];

        index /= 3; //divido per 3 perchè devo soppiantare le 3 dimensioni
        index *= 2; //moltiplico per avere la dimensionalità giusta
        float U = geometry->texCoords[0][index];
        float V = geometry->texCoords[0][index + 1];

        putValue(buffer, U);
        putValue(buffer, V);

        bytesLength += 2 * sizeof(float);

        numCoords++;
    }

    tinygltf::BufferView bufView;
    tinygltf::Accessor accessor;

    bufView.buffer = 0;
    bufView.byteOffset = bytesOffset;
    bufView.byteLength = bytesLength;
    bufView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    accessor.bufferView = currentBufView;
    accessor.byteOffset = 0;
    accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessor.count = numCoords;
    accessor.type = TINYGLTF_TYPE_VEC2;

    model.accessors.push_back(accessor);
    model.bufferViews.push_back(bufView);

    currentAccessor++;
    currentBufView++;

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

        putValue(buffer, nx);
        putValue(buffer, ny);
        putValue(buffer, nz);

        checkMinMax(minX, maxX, nx);
        checkMinMax(minY, maxY, ny);
        checkMinMax(minZ, maxZ, nz);

        bytesLength += 3 * sizeof(float);
    }

    tinygltf::BufferView bufView;
    tinygltf::Accessor accessor;

    bufView.buffer = 0;
    bufView.byteOffset = bytesOffset;
    bufView.byteLength = bytesLength;
    bufView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    accessor.bufferView = currentBufView;
    accessor.byteOffset = 0;
    accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessor.count = indeces.size();
    accessor.type = TINYGLTF_TYPE_VEC3;
    accessor.maxValues = { maxX, maxY, maxZ };
    accessor.minValues = { minX, minY, minZ };

    model.accessors.push_back(accessor);
    model.bufferViews.push_back(bufView);

    currentAccessor++;
    currentBufView++;

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

        putValue(buffer, x);
        putValue(buffer, y);
        putValue(buffer, z);

        checkMinMax(minX, maxX, x);
        checkMinMax(minY, maxY, y);
        checkMinMax(minZ, maxZ, z);

        bytesLength += 3 * sizeof(float);
    }

    tinygltf::BufferView bufView;
    tinygltf::Accessor accessor;

    bufView.buffer = 0;
    bufView.byteOffset = bytesOffset;
    bufView.byteLength = bytesLength;
    bufView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    accessor.bufferView = currentBufView;
    accessor.byteOffset = 0;
    accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessor.count = indeces.size();
    accessor.type = TINYGLTF_TYPE_VEC3;
    accessor.maxValues = { maxX, maxY, maxZ };
    accessor.minValues = { minX, minY, minZ };

    model.accessors.push_back(accessor);
    model.bufferViews.push_back(bufView);

    currentAccessor++;
    currentBufView++;

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
        putValue(buffer, v);

        checkMinMax(minVertIndex, maxVertIndex, v);

        bytesLength += sizeof(USHORT);
    }

    int padding = bytesLength % 4; //aggiungere eventuali padding essendo bytesLength multiplo di 2

    for(int i = 0; i < padding; i++)
        putValue(buffer, (BYTE)0);

    bytesLength += padding;

    tinygltf::BufferView bufView;
    tinygltf::Accessor accessor;

    bufView.buffer = 0;
    bufView.byteOffset = bytesOffset;
    bufView.byteLength = bytesLength;
    bufView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    accessor.bufferView = currentBufView;
    accessor.byteOffset = 0;
    accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
    accessor.count = vertices;
    accessor.type = TINYGLTF_TYPE_SCALAR;
    accessor.maxValues.push_back(maxVertIndex);
    accessor.minValues.push_back(minVertIndex);

    model.accessors.push_back(accessor);
    model.bufferViews.push_back(bufView);

    currentAccessor++;
    currentBufView++;

    return bytesLength;
}

bool ConverterGLTF::convert(char* output, rw::Clump& dff, rw::TextureDictionary& txd)
{
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
    int splits = geometry->splits.size();
    int curBytesOffset = 0;

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
    //COORDINATE UV
    curBytesOffset += insertUVTexture(buffer.data, indeces, geometry, curBytesOffset);
    //NORMALI
    curBytesOffset += insertNormals(buffer.data, indeces, geometry, curBytesOffset);

    //TODO: BONES

    //ogni split è una parte della mesh con materiale diverso
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
        primitive.attributes["POSITION"] = 0;

        //NORMALI
        if(geometry->hasNormals)
            primitive.attributes["NORMAL"] = 2;

        //COORDINATE UV
        std::string texCoord = "TEXCOORD_0";
        primitive.attributes[texCoord] = 1;

        primitive.material = material;
        primitive.mode = TINYGLTF_MODE_TRIANGLES;

        mesh.primitives.push_back(primitive);
    }

    node.mesh = 0;
    scene.nodes.push_back(0);

    asset.version = "2.0";
    asset.generator = "dff_converter by SimoSbara (using tiny_gltf)";

    model.scenes.push_back(scene);
    model.meshes.push_back(mesh);
    model.nodes.push_back(node);
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