#pragma once

#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

struct Entry {
    fs::path filename = "";
    uint32_t packingMethod = 0;
    uint32_t orginalSize = 0;
    uint32_t reserved = 0;
    uint32_t timestamp = 0;
    std::vector<uint8_t> data = {};
    uint32_t dataSize = 0;
};