#include "TTFReader.h"

#include <array>
#include <vector>

#include <iostream>
#include <bitset>

TTFReader::TTFReader(const std::string& fileName)
{
    mByteStream.open(fileName, std::ios::in | std::ios::binary);
}

TTFReader::~TTFReader()
{
    mByteStream.close();
}

uint8_t TTFReader::readUInt8()
{
    std::array<std::byte, 1> bytes{};
    mByteStream.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
    uint8_t value;
    std::memcpy(&value, bytes.data(), bytes.size());
    return value;
}

uint16_t TTFReader::readUInt16()
{
    uint16_t value = readUInt8() << 8 | readUInt8();
    return value;
}

uint32_t TTFReader::readUInt32()
{
    uint32_t value = readUInt16() << 16 | readUInt16();
    return value;
}

std::wstring TTFReader::readString(int length)
{
    std::wstring str = L"";
    for (int i = 0; i < length; i += 1) {
        str += static_cast<wchar_t>(readUInt8());
    }
    return str;
}


uint32_t TTFReader::calculateChecksum(uint32_t offset, uint32_t length)
{
    auto old = mByteStream.tellg();
    mByteStream.seekg(offset, std::ios::beg);
    uint32_t sum = 0;
    int nlongs = ((length + 3) / 4) | 0;
    while (nlongs -= 1) {
        sum = (sum + readUInt32() & 0xffffffff);
    }
    mByteStream.seekg(old);
    old = mByteStream.tellg();
    return sum;
}