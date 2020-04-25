#pragma once

struct Tagg {
    std::string signature = "";
    uint32_t dataLength = 0;
    std::vector<uint8_t> data = {};
};