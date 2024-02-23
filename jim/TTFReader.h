#pragma once

#include <fstream>
#include "TTFDataTypes.h"

class TTFReader
{
public:
	TTFReader(const std::string& fileName);
	~TTFReader();

	// Basic data types
	uint8_t readUInt8();
	uint16_t readUInt16();
	uint32_t readUInt32();
	int16_t readInt16();
	int32_t readInt32();
	int64_t readDate();
	float readFixed();
	std::wstring readString(int length);

	// TTF Data Types
	TTF::HeadTable readHeadTable(uint32_t headOffset);
	TTF::TableDirectory readTableDirectory();
	std::pair<std::wstring, TTF::TableRecord> readTableRecord();
	uint32_t readGlyphOffset(uint32_t locaOffset, int index, uint16_t indexToLocFormat);

	uint32_t calculateChecksum(uint32_t offset, uint32_t length);

private:
	std::ifstream mByteStream;
};
