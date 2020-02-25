#pragma once

#include <math.h>
#include <optional>

struct FaceData {
    int32_t headerFaceCount = 0;
    uint32_t color = 0;
    int8_t special = 0;
    uint8_t orHints = 0;

    // v >= 39
    std::optional<bool> hasSkeleton = {};

    // v >= 51
    std::optional<int32_t> nVertices = {};
    std::optional<float_t> faceArea = {};
};