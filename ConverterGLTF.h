#pragma once
#include "Converter.h"
#include <tiny_gltf.h>
#include <map>
#include <unordered_set>

#ifdef _DEBUG
#pragma comment(lib, "tinygltfd.lib")
#else
#pragma comment(lib, "tinygltf.lib")
#endif
class ConverterGLTF : public Converter
{
private:

	tinygltf::Model model;
	std::map<std::string, int> materials;
	int currentAccessor;
	int currentBufView;

	template<class T>
	void putValue(std::vector<BYTE> &buffer, T val);

	int insertIndices(std::vector<BYTE>& buffer, rw::Split* split, int bytesOffset);
	int insertVertices(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset);
	int insertNormals(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset);
	int insertUVTexture(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rw::Geometry* geometry, int bytesOffset);
	bool insertImageTexture(rw::NativeTexture* txdTex);

	void resetModel()
	{
		materials.clear();
		model = tinygltf::Model();
		currentAccessor = 0;
		currentBufView = 0;
	}

public:
	ConverterGLTF();

	virtual bool convert(char* output, rw::Clump& dff, rw::TextureDictionary& txd);
};

