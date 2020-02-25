#pragma once

#include <vector>

#include "../Types.h"

struct LodFrame {
    float_t frameTime = 0;
    uint32_t nBones = 0;
    std::vector<XYZTriplet> bonePositions = {};
};