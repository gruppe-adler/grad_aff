#pragma once

#include <string>
#include <array>
#include <optional>

#include "../Types.h"

struct LodProxy {
    std::string p3dProxyName = "";
    std::array<XYZTriplet, 4> transform = {};
    uint32_t proxySeqenceID = 0;
    uint32_t namedSelectionIndex = 0;
    int32_t boneIndex = 0;
    std::optional<uint32_t> sectionIndex = {};
};