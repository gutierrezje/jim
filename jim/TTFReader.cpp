#include "TTFReader.h"

#include <algorithm>
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

ttf::HeadTable TTFReader::readHeadTable(uint32_t headOffset)
{
    auto oldPos = mByteStream.tellg();
    mByteStream.seekg(headOffset);
    ttf::HeadTable headTable{
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

ttf::TableDirectory TTFReader::readTableDirectory()
{
    return ttf::TableDirectory{
        .scalarType = readUInt32(),
        .numTables = readUInt16(),
        .searchRange = readUInt16(),
        .entrySelector = readUInt16(),
        .rangeShift = readUInt16()
    };
}

std::pair<std::wstring, ttf::TableRecord> TTFReader::readTableRecord()
{
    std::wstring tag = readString(4);

    ttf::TableRecord record{
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

uint32_t TTFReader::readGlyphOffset(int index, uint32_t locaOffset, uint16_t indexToLocFormat)
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

std::variant<ttf::SimpleGlyph, ttf::ComplexGlyph> TTFReader::readGlyph(int index, uint32_t locaOffset, ttf::TableRecord glyfTable, uint16_t indexToLocFormat)
{
    uint32_t offset = glyfTable.offset + readGlyphOffset(index, locaOffset, indexToLocFormat);
    if (offset >= glyfTable.offset + glyfTable.length) {
        return ttf::SimpleGlyph{ 0 };
    }

    mByteStream.seekg(offset);

    ttf::GlyphDescription glyphDesc{
        .numberOfCountours = readInt16(),
        .xMin = readInt16(),
        .yMin = readInt16(),
        .xMax = readInt16(),
        .yMax = readInt16()
    };

    if (glyphDesc.numberOfCountours == -1) {
        std::cout << "TODO\n";
        return ttf::ComplexGlyph{ 0 };
    }
    else {
        return readSimpleGlyph(glyphDesc);
    }
}

ttf::SimpleGlyph TTFReader::readSimpleGlyph(ttf::GlyphDescription glyphDesc)
{
    ttf::SimpleGlyph glyph{ 0 };
    glyph.glyphDesc = glyphDesc;

    glyph.endPtsOfContours = std::vector<uint16_t>(glyphDesc.numberOfCountours, 0);
    for (int i = 0; i < glyphDesc.numberOfCountours; i += 1) {
        glyph.endPtsOfContours[i] = readUInt16();
    }

    // Skipping instructions
    mByteStream.seekg(readUInt16(), std::ios::cur);

    if (glyphDesc.numberOfCountours == 0) {
        return glyph;
    }

    int numPoints = std::ranges::max(glyph.endPtsOfContours) + 1;
    std::cout << numPoints << std::endl;

    glyph.flags = std::vector<uint8_t>(numPoints);
    for (int i = 0; i < numPoints; i += 1) {
        uint8_t flag = readUInt8();
        glyph.flags[i] = flag;
        if (flag & ttf::GlyphFlags::REPEAT) {
            auto repeatCount = readUInt8();
            
            while (repeatCount -= 1) {
                i += 1;
                glyph.flags[i] = flag;
            }
        }
    }

    // Read coordinates
    int value = 0;
    glyph.xCoordinates = std::vector<uint16_t>(numPoints, 0);
    for (int i = 0; i < numPoints; i += 1) {
        uint8_t flag = glyph.flags[i];
        if (flag & ttf::GlyphFlags::X_SHORT) {
            if (flag & ttf::GlyphFlags::X_SAME_OR_POS) {
                value += readUInt8();
            }
            else {
                value -= readUInt8();
            }
        }
        else if (~flag & ttf::GlyphFlags::X_SAME_OR_POS) {
            value += readInt16();
        }
        glyph.xCoordinates[i] = value;
    }

    value = 0;
    glyph.yCoordinates = std::vector<uint16_t>(numPoints, 0);
    for (int i = 0; i < numPoints; i += 1) {
        uint8_t flag = glyph.flags[i];
        if (flag & ttf::GlyphFlags::Y_SHORT) {
            if (flag & ttf::GlyphFlags::Y_SAME_OR_POS) {
                value += readUInt8();
            }
            else {
                value -= readUInt8();
            }
        }
        else if (~flag & ttf::GlyphFlags::Y_SAME_OR_POS) {
            value += readInt16();
        }
        glyph.yCoordinates[i] = value;
    }

    return glyph;
}