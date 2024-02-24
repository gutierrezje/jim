#include "TrueTypeFont.h"

#include <iostream>

TrueTypeFont::TrueTypeFont(const std::string& ttf_file) 
    : mTTFReader{ ttf_file }, mLocaTableOffset{0}, mHeadTable{0}, mGlyfTable{0}
{
    mTableDirectory = mTTFReader.readTableDirectory();

    for (int i = 0; i < mTableDirectory.numTables; i += 1) {
        auto nextRecord = mTTFReader.readTableRecord();
        mTables.emplace(nextRecord);
    }
    if (auto headRecord = mTables.find(L"head"); headRecord != mTables.end()) {
        mHeadTable = mTTFReader.readHeadTable(headRecord->second.offset);
    }
    else {
        std::wcerr << "head table not found in TTF file.\n";
    }
    if (auto locaRecord = mTables.find(L"loca"); locaRecord != mTables.end()) {
        mLocaTableOffset = locaRecord->second.offset;
    }
    else {
        std::wcerr << "loca table not found in TTF file.\n";
    }
    if (auto glyfRecord = mTables.find(L"glyf"); glyfRecord != mTables.end()) {
        mGlyfTable = glyfRecord->second;
    }
    else {
        std::wcerr << "glyf table not found in TTF file.\n";
    }
    std::cout << "DONE\n";
}

std::variant<ttf::SimpleGlyph, ttf::ComplexGlyph> TrueTypeFont::readGlyph(int index)
{
    return mTTFReader.readGlyph(index, mLocaTableOffset, mGlyfTable, mHeadTable.indexToLocFormat);
}