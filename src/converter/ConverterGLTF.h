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

	int insertIndices(std::vector<BYTE>& buffer, rwtools::Split* split, int bytesOffset);
	int insertVertices(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rwtools::Geometry* geometry, int bytesOffset);
	int insertNormals(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rwtools::Geometry* geometry, int bytesOffset);
	int insertUVTexture(std::vector<BYTE>& buffer, std::vector<USHORT>& indeces, rwtools::Geometry* geometry, int bytesOffset);
	int insertBones(std::vector<BYTE>& buffer, tinygltf::Mesh& mesh, rwtools::Geometry* geometry, int bytesOffset);
	int insertInverseMatBones(std::vector<BYTE>& buffer, tinygltf::Skin& skin, rwtools::Geometry* geometry, int bytesOffset);
	void putRotationTranslation(rwtools::Frame* frame, tinygltf::Node& node);

	bool insertImageTexture(rwtools::NativeTexture* txdTex);

	void resetModel();

	virtual bool convert(std::string output, rwtools::Clump& dff, rwtools::TextureDictionary& txd);

public:
	ConverterGLTF();

	virtual void setRotation(double rx, double ry, double rz)
	{
		this->rx = rx;
		this->ry = ry;
		this->rz = rz;
	}

	virtual void setTranslation(double tx, double ty, double tz)
	{
		this->tx = tx;
		this->ty = ty;
		this->tz = tz;
	}

	virtual bool convert(std::string output, std::string inputDff, std::string inputTxd, bool ignoreCorruptedTXD = true);
	virtual bool convert(std::string output, std::string inputDff);

};

