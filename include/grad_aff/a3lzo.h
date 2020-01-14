#pragma once

#include "StreamUtil.h"

#include <vector>
#include <fstream>
#include <cassert>

#define M2_MAX_OFFSET   0x0800

namespace grad_aff {
    size_t Decompress(std::istream& i, std::vector<uint8_t>& output, size_t expectedSize);
};
