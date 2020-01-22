#pragma once

#include <iostream>
#include <sstream>
#include <filesystem>

#include "MipMap.h"
#include "Tagg.h"
#include "Palette.h"

#include "../StreamUtil.h"

namespace fs = std::filesystem;

namespace grad_aff {
    class Paa {
    private:
        uint16_t magicNumber = 0;
        std::vector<Tagg> taggs = {};
        Palette palette;
        size_t averageRed = 0;
        size_t averageBlue = 0;
        size_t averageGreen = 0;
        size_t averageAlpha = 0;

        std::shared_ptr<std::istream> is;
    public:
        enum class TypeOfPaX {
            DXT1,
            DXT2,
            DXT3,
            DXT4,
            DXT5,
            RGBA4444,
            RGBA5551,
            RGBA8888,
            GRAYwAlpha,
            UNKNOWN
        };
        bool hasTransparency = false;
        std::vector<uint8_t> getRawPixelData(uint8_t level = 0);
        std::vector<MipMap> mipMaps = {};
        TypeOfPaX typeOfPax;

        Paa();
        Paa(std::string filename);
        Paa(std::vector<uint8_t> data);
        
        void readPaa();
        void readImage(fs::path filename);
        uint8_t getRawPixelDataAt(size_t x, size_t y, uint8_t level = 0);

        void calculateMipmapsAndTaggs();

        void writeImage(std::string filename, int level = 0);
        void writePaa(std::string filename, TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);
        void setRawPixelData(std::vector<uint8_t> data, uint8_t level = 0);
        void setRawPixelDataAt(size_t x, size_t y, uint8_t pixelData, uint8_t level = 0);

    };
};