#pragma once

#include <math.h>
#include <optional>
#include <variant>

struct UVSet {
    std::optional<float_t> minU = 0;
    std::optional<float_t> minV = 0;
    std::optional<float_t> maxU = 0;
    std::optional<float_t> maxV = 0;

    uint32_t nVertices = 0;
    bool defaultFill = false;
    std::optional<std::variant<float_t, std::pair<float_t, float_t>>> defaultValue = {};
    std::vector<uint8_t> uvData = {};
};