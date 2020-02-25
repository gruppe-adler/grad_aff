#pragma once

#include <string>
#include <optional>

struct Skeleton {
    std::string name = "";
    
    std::optional<bool> isDiscrete = {};

    uint32_t nBones = 0;
    std::vector<std::string> bones = {};

    std::optional<std::string> pivotsNameObsolete = {};
};