#pragma once

#include <array>
#include <math.h>

using XYZTriplet = std::array<float_t, 3>;
using TransformMatrix = std::array<XYZTriplet, 4>;
using D3DCOLORVALUE = std::array<float_t, 4>;