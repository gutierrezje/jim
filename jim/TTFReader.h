#pragma once

#include <fstream>
#include <variant>

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
	ttf::HeadTable readHeadTable(uint32_t headOffset);
	ttf::TableDirectory readTableDirectory();
	std::pair<std::wstring, ttf::TableRecord> readTableRecord();
	uint32_t readGlyphOffset(int index, uint32_t locaOffset, uint16_t indexToLocFormat);
	std::variant<ttf::SimpleGlyph, ttf::ComplexGlyph> readGlyph(int index, uint32_t locaOffset, ttf::TableRecord glyfTable, uint16_t indexToLocFormat);
	ttf::SimpleGlyph readSimpleGlyph(ttf::GlyphDescription);

	uint32_t calculateChecksum(uint32_t offset, uint32_t length);

private:
	std::ifstream mByteStream;
};
