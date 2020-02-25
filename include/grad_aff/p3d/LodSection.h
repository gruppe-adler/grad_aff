#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

struct LodSection {
    int32_t faceLowerIndex = 0;
    int32_t faceUpperIndex = 0;
    int32_t minBonexIndex = 0;
    int32_t bonesCount = 0;
    uint32_t commonPointsUserValue = 0;

    uint16_t commonTextureIndex = 0;
    uint32_t commonFaceFlags = 0;

    int32_t materialIndex = 0;
    
    std::optional<std::string> material = {};
    std::optional<uint32_t> nStages = {};
    std::optional<std::vector<float_t>> areaOverTex = {};
    std::optional<std::array<float_t, 11>> floats = {};
};
