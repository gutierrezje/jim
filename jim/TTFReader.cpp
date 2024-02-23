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


uint32_t TTFReader::calculateChecksum(uint32_t offset, uint32_t length)
{
    auto oldPos = mByteStream.tellg();
    mByteStream.seekg(offset, std::ios::beg);
    uint32_t sum = 0;
    uint32_t nLongs = offset + ((length + 3) & ~3);
    while (mByteStream.tellg() < nLongs) {
        sum += readUInt32();
    }
    mByteStream.seekg(oldPos);
    return sum;
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
    return static_cast<uint16_t>(readUInt8()) << 8 | readUInt8();
}

uint32_t TTFReader::readUInt32()
{
    return static_cast<uint32_t>(readUInt16()) << 16 | readUInt16();
}

int16_t TTFReader::readInt16()
{
    return static_cast<int16_t>(readUInt8()) << 8 | readUInt8();
}

int32_t TTFReader::readInt32()
{
    return static_cast<int32_t>(readUInt16()) << 16 | readUInt16();
}

int64_t TTFReader::readDate()
{
    return static_cast<int64_t>(readUInt32()) << 32 | readUInt32();
}

float TTFReader::readFixed()
{
    return readInt32() / (1 << 16);
}

std::wstring TTFReader::readString(int length)
{
    std::wstring str = L"";
    for (int i = 0; i < length; i += 1) {
        str += static_cast<wchar_t>(readUInt8());
    }
    return str;
}

TTF::HeadTable TTFReader::readHeadTable(uint32_t headOffset)
{
    auto oldPos = mByteStream.tellg();
    mByteStream.seekg(headOffset);
    TTF::HeadTable headTable{
        .version = readFixed(),
        .fontRevision = readFixed(),
        .checksumAdjustment = readUInt32(),
        .magicNumber = readUInt32(),
        .flags = readUInt16(),
        .unitsPerEm = readUInt16(),
        .created = readDate(),
        .modified = readDate(),
        .xMin = readInt16(),
        .yMin = readInt16(),
        .xMax = readInt16(),
        .yMax = readInt16(),
        .macStyle = readUInt16(),
        .lowestRecPPEM = readUInt16(),
        .fontDirectionHint = readInt16(),
        .indexToLocFormat = readInt16(),
        .glyphDataFormat = readInt16()
    };
    if (headTable.magicNumber != 0x5F0F3CF5) {
        std::wcerr << "head table magic number did not match.\n";
    }
    mByteStream.seekg(oldPos);
    return headTable;
}

TTF::TableDirectory TTFReader::readTableDirectory()
{
    return TTF::TableDirectory{
        .scalarType = readUInt32(),
        .numTables = readUInt16(),
        .searchRange = readUInt16(),
        .entrySelector = readUInt16(),
        .rangeShift = readUInt16()
    };
}

std::pair<std::wstring, TTF::TableRecord> TTFReader::readTableRecord()
{
    std::wstring tag = readString(4);

    TTF::TableRecord record{
        .checksum = readUInt32(),
        .offset = readUInt32(),
        .length = readUInt32()
    };

    if (tag != L"head") {
        if (calculateChecksum(record.offset, record.length) != record.checksum) {
            std::wcerr << "Checksum failed for table record: " << tag << '\n';
        }
    }

    return std::make_pair(tag, record);
}


uint32_t TTFReader::readGlyphOffset(uint32_t locaOffset, int index, uint16_t indexToLocFormat)
{
    auto oldPos = mByteStream.tellg();
    uint32_t offset;
    if (indexToLocFormat == 1) {
        mByteStream.seekg(locaOffset + index * 4);
        offset = readUInt32();
    }
    else {
        offset = readUInt16() * 2;
    }

    mByteStream.seekg(oldPos);
    return offset;
}