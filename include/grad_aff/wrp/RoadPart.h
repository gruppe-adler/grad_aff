#pragma once

#include "../Types.h"

#include <vector>
#include <tuple>
#include <string>

struct RoadPart {
    uint16_t nRoadPositions = 0;
    std::vector<XYZTriplet> roadPositions = {};
    std::vector<uint8_t> flags = {};
    std::string p3dModel = "";
    std::array<XYZTriplet, 4> transformMatrix = {};
};