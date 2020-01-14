#pragma once

#include "RoadPart.h"

#include <vector>

struct RoadNet {
    uint32_t nRoadParts = 0;
    std::vector<RoadPart> roadParts = {};
};