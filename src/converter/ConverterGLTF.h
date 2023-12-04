#pragma once
#include "Converter.h"
#include <tiny_gltf.h>
#include <map>
#include <unordered_set>

class ConverterGLTF : public Converter
{
private:

	tinygltf::Model model;
	std::map<std::string, int> materials;
	int currentAccessor;
	int currentBufView;

	template<class T>
	int putValue(std::vector<BYTE> &buffer, T val);

	void InsertAccBufView(int byteOffset, int byteLength, int target, int componentType, int count, int type, std::vector<double> min = {}, std::vector<double> max = {});

	int insertIndices(std::vector<BYTE>& buffer, rw::Split* split, int bytesOffset);
	int insertVertices(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset);
	int insertNormals(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset);
	int insertUVTexture(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset);
	int insertBones(std::vector<BYTE>& buffer, tinygltf::Mesh& mesh, rw::Geometry* geometry, int bytesOffset);
	int insertInverseMatBones(std::vector<BYTE>& buffer, tinygltf::Skin& skin, rw::Geometry* geometry, int bytesOffset);
	void putRotationTranslation(rw::Frame* frame, tinygltf::Node& node);

	bool insertImageTexture(rw::NativeTexture* txdTex);

	void resetModel();

	virtual bool convert(std::string output, rw::Clump& dff, rw::TextureDictionary& txd);

public:
	ConverterGLTF();

	virtual bool convert(std::string output, std::string inputDff, std::string inputTxd);
	virtual bool convert(std::string output, std::string inputDff);

};

