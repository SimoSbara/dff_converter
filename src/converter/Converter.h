#pragma once

#include <rwtools.h>

#ifndef WIN32
	typedef unsigned char BYTE;
	typedef unsigned short USHORT;
#endif

class Converter
{
protected:
	double rx, ry, rz;
	double tx, ty, tz;

	virtual void setRotation(double rx, double ry, double rz) = 0;
	virtual void setTranslation(double tx, double ty, double tz) = 0;

	virtual bool convert(std::string output, std::string inputDff, std::string inputTxd, bool ignoreCorruptedTXD) = 0;
	virtual bool convert(std::string output, std::string inputDff) = 0;
	virtual bool convert(std::string output, rwtools::Clump& dff, rwtools::TextureDictionary& txd) = 0;
};

