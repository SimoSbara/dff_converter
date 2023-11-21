#pragma once

#include <renderware.h>

#ifndef WIN32
	typedef unsigned char BYTE;
	typedef unsigned short USHORT;
#endif

class Converter
{
	virtual bool convert(char* output, rw::Clump &dff, rw::TextureDictionary &txd) = 0;
	virtual bool convert(char* output, rw::Clump &dff) = 0;
};

