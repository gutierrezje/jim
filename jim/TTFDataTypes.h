#pragma once
#include <cstdint>
#include <vector>

using fword = int16_t;
using ufword = uint16_t;
using date = int64_t;

namespace ttf {
    enum class flags {

    };
    struct TableRecord {
        uint32_t checksum;
        uint32_t offset;
        uint32_t length;
    };

    struct TableDirectory {
        uint32_t scalarType;
        uint16_t numTables;
        uint16_t searchRange;
        uint16_t entrySelector;
        uint16_t rangeShift;
    };

    struct HeadTable {
        float version;
        float fontRevision;
        uint32_t checksumAdjustment;
        uint32_t magicNumber;
        uint16_t flags;
        uint16_t unitsPerEm;
        int64_t created;
        int64_t modified;
        int16_t xMin;
        int16_t yMin;
        int16_t xMax;
        int16_t yMax;
        uint16_t macStyle;
        uint16_t lowestRecPPEM;
        int16_t fontDirectionHint;
        int16_t indexToLocFormat;
        int16_t glyphDataFormat;
    };

    enum GlyphFlags : uint8_t
    {
        ON_CURVE        = 1,
        X_SHORT         = 1 << 1,
        Y_SHORT         = 1 << 2,
        REPEAT          = 1 << 3,
        X_SAME_OR_POS   = 1 << 4,
        Y_SAME_OR_POS   = 1 << 5
    };
    //DEFINE_ENUM_FLAG_OPERATORS(ttf::GlyphFlags);

    struct GlyphDescription {
        int16_t numberOfCountours;
        int16_t xMin;
        int16_t yMin;
        int16_t xMax;
        int16_t yMax;
    };

    struct SimpleGlyph {
        GlyphDescription glyphDesc;
        std::vector<uint16_t> endPtsOfContours;
        uint16_t instructionLength;
        std::vector<uint8_t> instructions;
        std::vector<uint8_t> flags;
        std::vector<uint16_t> xCoordinates;
        std::vector<uint16_t> yCoordinates;
    };

    struct ComplexGlyph {
        GlyphDescription glyphDesc;
    };
}