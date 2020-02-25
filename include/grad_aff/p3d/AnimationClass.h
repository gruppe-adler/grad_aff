#pragma once

#include <string>
#include <optional>

#include "../Types.h"
#include "AnimTransformType.h"

struct AnimationClass {
    uint32_t animTransformType = 10000;
    std::string animClassName = "";
    std::string animSource = "";
    float_t minPhase = 0;
    float_t maxPhase = 0;
    float_t minValue = 0;
    float_t maxValue = 0;

    std::optional<float_t> animPeriod = {};
    std::optional<float_t> initPhase = {};

    uint32_t sourceAddress = 0; // should be 953267991

    std::shared_ptr<AnimTransformType> animType;

};