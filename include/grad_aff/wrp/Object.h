#pragma once

#include "../Types.h"

#include <vector>

#define GRAD_AFF_SIZE_OF_WRPOBJECT 60

struct Object {
    uint32_t objectId = 0;
    uint32_t modelIndex = 0;
    std::array<XYZTriplet, 4> transformMatrix = {};
    uint32_t static0x02 = 0;
};