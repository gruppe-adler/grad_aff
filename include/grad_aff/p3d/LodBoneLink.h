#pragma once

#include <vector>

struct LodBoneLink {
    uint32_t nLinks = 0;
    std::vector<uint32_t> link = {};
};