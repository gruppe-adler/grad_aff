#pragma once

#include <vector>

struct MipMap {
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t dataLength = 0;
    std::vector<uint8_t> data = {};
    bool lzoCompressed = false;
};