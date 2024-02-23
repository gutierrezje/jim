#pragma once
#include <cstdint>

using fword = int16_t;
using ufword = uint16_t;
using date = int64_t;

namespace TTF {
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
}