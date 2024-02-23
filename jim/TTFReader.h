#pragma once

#include <fstream>

class TTFReader
{
public:
	TTFReader(const std::string& fileName);
	~TTFReader();

	uint8_t readUInt8();
	uint16_t readUInt16();
	uint32_t readUInt32();
	std::wstring readString(int length);

	uint32_t calculateChecksum(uint32_t offset, uint32_t length);

private:
	std::ifstream mByteStream;
};
