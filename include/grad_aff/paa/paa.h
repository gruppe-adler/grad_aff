#pragma once

#if defined(_MSC_VER)
    #pragma warning(disable : 4251)
#endif

#include <iostream>
#include <sstream>
#include <memory>

#include "../grad_aff.h"
#include "../StreamUtil.h"

#include "mipmap.h"
#include "tagg.h"
#include "palette.h"

#include "core/AffBase.hpp"

#ifdef __cplusplus

namespace grad_aff {
    class GRAD_AFF_API Paa {
    public:

        static bool isPowerOfTwo(uint32_t x) noexcept;

        enum class TypeOfPaX {
            UNKNOWN,
            DXT1,
            DXT2,
            DXT3,
            DXT4,
            DXT5,
            RGBA4444,
            RGBA5551,
            RGBA8888,
            GRAYwAlpha
        };

    private:
        uint16_t magicNumber = 0;
        Palette palette;
        size_t averageRed = 0;
        size_t averageBlue = 0;
        size_t averageGreen = 0;
        size_t averageAlpha = 0;

        void readPaa(std::shared_ptr<std::istream> is, bool peek);
        void writePaa(std::ostream& os, TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);
    public:
        bool hasTransparency = false;
        TypeOfPaX typeOfPax;

        std::vector<MipMap> mipMaps = {};
        std::vector<Tagg> taggs = {};

        Paa();

        void readPaa(std::string filename, bool peek = false);
        void readPaa(std::vector<uint8_t> data, bool peek = false);

        void writePaa(std::string filename, TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);
        std::vector<uint8_t> writePaa(TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);

        void calculateMipmapsAndTaggs();

        std::vector<uint8_t> getRawPixelData(uint8_t level = 0);
        std::array<uint8_t, 4> getRawPixelDataAt(size_t x, size_t y, uint8_t level = 0);

        void setRawPixelData(std::vector<uint8_t> data, uint8_t level = 0);
        void setRawPixelDataAt(size_t x, size_t y, std::array<uint8_t, 4> data, uint8_t level = 0);

        void setMipMaps(std::vector<MipMap> mipMaps);
        std::vector<MipMap> getMipMaps() const;

        bool getHasTransparency() const;
        bool isValid() const noexcept;

        MipMap getOptimalMipMap(uint16_t cx);

#ifdef GRAD_AFF_USE_OIIO
        void readImage(std::string filename);
        void writeImage(std::string filename, int level = 0);
#endif
    };

#else
typedef
struct Paa
    Paa;
#endif

#ifdef __cplusplus
extern "C" {
#endif
    extern GRAD_AFF_API Paa* newPaa();
    extern GRAD_AFF_API void delPaa(Paa* paaPtr);

    extern GRAD_AFF_API void readPaa(Paa* paaPtr, const char* filename, bool peek);
    extern GRAD_AFF_API void readPaaData(Paa* paaPtr, const uint8_t* data, size_t size, bool peek);

    extern GRAD_AFF_API void writePaa(Paa* paaPtr, const char* filename, int typeOfPax);
    extern GRAD_AFF_API uint8_t* writePaaData(Paa* paaPtr, size_t* sizeOut, int typeOfPax);

    extern GRAD_AFF_API void calculateMipmapsAndTaggs(Paa* paaPtr);

    extern GRAD_AFF_API void freeDataPtr(uint8_t* dataPtr);

    extern GRAD_AFF_API size_t getMipMapCount(Paa* paaPtr);
    extern GRAD_AFF_API void setMipMap(Paa* paaPtr, uint16_t width, uint16_t height, uint8_t* data, size_t dataSize, int level);
    extern GRAD_AFF_API void getMipMap(Paa* paaPtr, uint16_t* width, uint16_t* height, uint8_t** data, size_t* dataSize, bool* lzoCompressed, int level);

#ifdef __cplusplus
}
};
#endif