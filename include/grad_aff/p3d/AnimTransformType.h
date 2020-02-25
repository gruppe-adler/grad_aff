#pragma once

#include <math.h>
#include "../Types.h"

#include <optional>

enum class AnimTransformTypeEnum
{
    ROTATION = 0,
    ROTATIONX = 1,
    ROTATIONY = 2,
    ROTATIONZ = 3,
    TRANSLATION = 4,
    TRANSLATIONX = 5,
    TRANSLATIONY = 6,
    TRANSLATIONZ = 7,
    DIRECT = 8,
    HIDE = 9
};

struct AnimTransformType {
    AnimTransformTypeEnum type;
};

struct AnimTransformRotation : AnimTransformType {
    float_t angle0 = 0;
    float_t angle1 = 0;
};

struct AnimTransformTranslation : AnimTransformType {
    float_t offset0 = 0;
    float_t offset1 = 0;
};

struct AnimTransformDirect : AnimTransformType {
    XYZTriplet axisPos = {};
    XYZTriplet axisDir = {};
    float_t angle = 0; // In radians whereas the model.cfg entry is in degrees
    float_t axisOffset = 0;
};

struct AnimTransformHide : AnimTransformType {
    float_t hideValue = 0;
    std::optional<float_t> unknownFloat = {};
};