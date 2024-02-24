#pragma once

#include <unordered_map>
#include <string>

#include "TTFDataTypes.h"
#include "TTFReader.h"

class TrueTypeFont
{
public:
    TrueTypeFont(const std::string& ttf_file);
    std::variant<ttf::SimpleGlyph, ttf::ComplexGlyph> readGlyph(int index);

private:

    TTFReader mTTFReader;
    ttf::TableDirectory mTableDirectory;
    std::unordered_map<std::wstring, ttf::TableRecord> mTables;
    ttf::HeadTable mHeadTable;
    uint32_t mLocaTableOffset;
    ttf::TableRecord mGlyfTable;
};