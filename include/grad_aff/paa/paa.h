#pragma once

#include <iostream>
#include <sstream>
#include <filesystem>

#include "../grad_aff.h"
#include "../StreamUtil.h"

#include "mipmap.h"
#include "tagg.h"
#include "palette.h"

namespace fs = std::filesystem;

/*
GRAD_AFF_EXTIMP template class GRAD_AFF_API std::allocator<MipMap>;
GRAD_AFF_EXTIMP template class GRAD_AFF_API std::allocator<Tagg>;
GRAD_AFF_EXTIMP template class GRAD_AFF_API std::vector<MipMap>;
GRAD_AFF_EXTIMP template class GRAD_AFF_API std::vector<Tagg>;
GRAD_AFF_EXTIMP template class std::_Compressed_pair<std::allocator<Tagg>, std::_Vector_val<std::_Simple_types<Tagg>>, true>;
*/

namespace grad_aff {
    class GRAD_AFF_API Paa {
    private:
        uint16_t magicNumber = 0;
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
        std::vector<MipMap> mipMaps = {};
        std::vector<Tagg> taggs = {};
        TypeOfPaX typeOfPax;

        Paa();
        Paa(std::string filename);
        Paa(std::vector<uint8_t> data);
        
        void readPaa(bool peek = false);
        void readImage(fs::path filename);

        std::vector<uint8_t> getRawPixelData(uint8_t level = 0);
        uint8_t getRawPixelDataAt(size_t x, size_t y, uint8_t level = 0);

        void calculateMipmapsAndTaggs();

        void writeImage(std::string filename, int level = 0);
        void writePaa(std::string filename, TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);
        void writePaa(std::ostream& os, TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);
        void setRawPixelData(std::vector<uint8_t> data, uint8_t level = 0);
        void setRawPixelDataAt(size_t x, size_t y, uint8_t pixelData, uint8_t level = 0);
    };
};
