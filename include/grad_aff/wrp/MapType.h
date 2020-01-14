#pragma once

#include <array>

struct MapType {
    uint32_t infoType;
    uint32_t mapType;
};

struct MapType1 : MapType {
    uint32_t objectId;
    float_t x, y;
};

struct MapType2 : MapType {
    uint32_t objectId;
    std::array<float_t, 8> bounds;
};

struct MapType3 : MapType {
    uint32_t color;
    uint32_t indicator;
    std::array<float_t, 4> floats;
};

struct MapType4 : MapType {
    uint32_t objectId;
    std::array<float_t, 8> bounds;
    std::array<uint8_t, 4> color;
};

struct MapType5 : MapType {
    uint32_t objectId;
    std::array<float_t, 4> floats;
};

struct MapType35 : MapType {
    uint32_t objectId;
    std::array<float_t, 6> floats;
    uint8_t unknown;
};