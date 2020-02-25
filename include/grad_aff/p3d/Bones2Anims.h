#pragma once

#include <vector>

struct Bone2AnimClassList
{
    uint32_t nAnimClasses = 0;
    std::vector<uint32_t> animationClassIndex = {};
};

struct Bones2Anims {
    uint32_t nBones = 0;
    std::vector<Bone2AnimClassList> bone2AnimClassLists = {};
};