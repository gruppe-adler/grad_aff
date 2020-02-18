#pragma once

#include <iostream>
#include <sstream>
#include <filesystem>

#include "../grad_aff.h"
#include "../StreamUtil.h"

namespace fs = std::filesystem;

namespace grad_aff {
    class GRAD_AFF_API Odol {
    private:
        std::string filetype;

        std::shared_ptr<std::istream> is;
    public:
        uint32_t version = 0;
        uint32_t numberOfLods = 0;

        Odol(std::string filename);
        Odol(std::vector<uint8_t> data);

        void readOdol();

        void readModelInfo();
    };
};