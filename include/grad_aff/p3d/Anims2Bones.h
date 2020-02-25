#pragma once

#include <vector>
#include <optional>

#include "../Types.h"

struct AnimBones {
    int32_t skeletonBoneNameIndex = -1;
    std::optional<XYZTriplet> axisPos = {};
    std::optional<XYZTriplet> axisDir = {};
};

struct Anims2Bones {
    std::vector<AnimBones> animBones = {};
};