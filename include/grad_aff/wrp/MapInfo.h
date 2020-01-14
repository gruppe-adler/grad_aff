#pragma once

#include "MapType.h"

#include <vector>

struct MapInfo {
    uint32_t infoType = 0;
    std::vector<MapType> mapData = {};
};