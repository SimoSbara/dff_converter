#pragma once

#include <renderware.h>

class Converter
{
	virtual bool convert(char* output, rw::Clump &dff, rw::TextureDictionary &txd) = 0;
};

