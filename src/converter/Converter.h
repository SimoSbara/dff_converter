#pragma once

#include <rwtools.h>

#ifndef WIN32
	typedef unsigned char BYTE;
	typedef unsigned short USHORT;
#endif

class Converter
{
	virtual bool convert(std::string output, std::string inputDff, std::string inputTxd) = 0;
	virtual bool convert(std::string output, std::string inputDff) = 0;
	virtual bool convert(std::string output, rw::Clump& dff, rw::TextureDictionary& txd) = 0;
};

