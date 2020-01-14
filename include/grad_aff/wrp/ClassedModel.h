#pragma once

#include <vector>

struct ClassedModel {
    std::string className = "";
    std::string modelPath = "";
    std::array<float_t, 3> position = {};
    uint32_t unkonwn = 0;
};