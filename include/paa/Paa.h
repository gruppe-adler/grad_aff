#pragma once

#ifdef __cplusplus

#include <cstdint>
#include <optional>
#include <map>

#include "../core/AffApi.h"
#include "../core/AffLazy.h"

#endif

#include "../paa/Mipmap.h"
#include "../paa/Tagg.h"

#ifdef __cplusplus

//#include "../grad_aff/paa/tagg.h"
#include "../grad_aff/paa/palette.h"

namespace grad::aff::Paa {
    class GRAD_AFF_API Paa : public core::AffLazy {
    public:

        inline static bool isPowerOfTwo(uint32_t x) noexcept;

        /**
         * @brief PaX Type enum.
        */
        enum class TypeOfPaX : int32_t {
            UNKNOWN = 0,            /**< enum value 1 */
            DXT1 = 0xff01,          /* DXT1 PAA */
            DXT2 = 0xff02,          /* DXT2 PAA */
            DXT3 = 0xff03,          /* DXT3 PAA */
            DXT4 = 0xff04,          /* DXT4 PAA */
            DXT5 = 0xff05,          /* DXT5 PAA */
            RGBA4444 = 0x4444,      /* */
            RGBA5551 = 0x1555,      /* */
            RGBA8888 = 0x8888,      /* */
            GRAYwAlpha = 0x8080     /* */
        };

    private:
        uint16_t magicNumber = 0;
        Palette palette;
        size_t averageRed = 0;
        size_t averageBlue = 0;
        size_t averageGreen = 0;
        size_t averageAlpha = 0;

        std::vector<std::streampos> mipmapPositions = {};
        //void readPaa(std::shared_ptr<std::basic_iostream<char>> is, bool peek);
        //void writePaaToOutStream(TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);
        //std::vector<uint8_t> readMipMapData(size_t mipMapSize, uint16_t width, uint16_t height, bool isLzoCompressed);
        std::vector<uint8_t> decompressMipMap(std::vector<uint8_t> data, uint16_t width, uint16_t height, bool isLzoCompressed);

        void readMipMapsData();
        void readMipMapData(uint8_t level);

        bool hasTransparency = false;

    protected:
        //virtual void read(std::shared_ptr<std::basic_iostream<char>>& stream, bool peek) override;
        //virtual void read(std::shared_ptr<std::basic_iostream<char>>& stream, bool peek) override;
        //virtual void readLazyFromStream(std::shared_ptr<std::basic_iostream<char>>& stream, bool lazy) override;
        //virtual void readLazy(std::shared_ptr<std::basic_iostream<char>>& stream, bool lazy) override;
        virtual void readLazyFromStream(bool lazy) override;
        virtual void writeToStream(std::shared_ptr<std::basic_iostream<char>> stream) override;
       
    public:

        Paa();

        TypeOfPaX typeOfPax = TypeOfPaX::UNKNOWN;

        // MipMaps
        /**
        * The PAA mipmaps.
        */
        std::vector<Mipmap> mipmaps = {};

        /**
         * @brief Set the PAA mipmaps.
         * @param mipmaps Mipmaps to set.
        */
        void setMipMaps(std::vector<Mipmap> mipmaps);

        /**
         * @brief Get the PAA mipmaps.
         * @return The mipmaps in a vector. 
        */
        std::vector<Mipmap> getMipMaps();

        /**
         * @brief Get the mipmaps of the specified level.
         * @param level Mipmap level (zero-based index).
         * @return 
        */
        Mipmap getMipMap(uint8_t level);

        /**
         * @brief Set the mipmap for the specified level.
         * @param mipMap The mipmap to set.
         * @param level Mipmap level (zero-base index).
        */
        void setMipMap(Mipmap mipMap, uint8_t level);

        /**
         * @brief Returns the index of the mipmap with the next highest 
         * dimension (max of height and width) than 
         * the given dimension.
         * @param cx The dimension.
         * @return Index of the mipmap (zero-index based).
        */
        uint8_t getOptimalMipMapIndex(uint16_t cx);

        /**
         * @brief Returns the mipmap with the next highest
         * dimension (max of height and width) than
         * the given dimension.
         * @param cx The dimension.
         * @return The mipmap.
        */
        Mipmap getOptimalMipMap(uint16_t cx);


        // Taggs

        std::vector<Tagg> taggs = {};

        void setTaggs(std::vector<Tagg> taggs);
        std::vector<Tagg> getTaggs() const;

        Tagg getTagg(std::string signature) const;

        // Pixel

        std::vector<uint8_t> getPixelData(uint8_t level = 0);
        void setPixelData(std::vector<uint8_t> data, uint8_t level = 0);

        std::array<uint8_t, 4>  getPixel(size_t x, size_t y, uint8_t level = 0);
        void setPixel(size_t x, size_t y, std::array<uint8_t, 4> data, uint8_t level = 0);

        // Util

        bool isTransparent() const;
        bool isValid() const noexcept;

        void calculateMipmapsAndTaggs();

        //void readPaa(std::string filename, bool peek = false);
        //void readPaa(std::vector<uint8_t> data, bool peek = false);

        //void writePaa(std::string filename, TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);
        //std::vector<uint8_t> writePaa(TypeOfPaX typeOfPaX = TypeOfPaX::UNKNOWN);


        /*
        std::vector<uint8_t> getRawPixelData(uint8_t level = 0);
        std::array<uint8_t, 4> getRawPixelDataAt(size_t x, size_t y, uint8_t level = 0);

        void setRawPixelData(std::vector<uint8_t> data, uint8_t level = 0);
        void setRawPixelDataAt(size_t x, size_t y, std::array<uint8_t, 4> data, uint8_t level = 0);


        bool getHasTransparency() const;
        */

    };

#else


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../core/AffApi.h"

typedef
struct Paa
    Paa;
#endif

#ifdef __cplusplus
extern "C" {
#endif
    /// @brief Creates a new Paa Object.
    /// @return NULL on failure, or a new Paa.
    extern GRAD_AFF_API Paa* PaaCreate();

    extern GRAD_AFF_API Paa* PaaCreateFromData(uint16_t width, uint16_t height, uint8_t* data, size_t size);

    /// @brief Destroys a Paa Object.
    /// @param paa The object to destroy.
    extern GRAD_AFF_API bool PaaDestroy(Paa* paa);

    extern GRAD_AFF_API uint32_t PaaGetTypeOfPax(Paa* paa);

    extern GRAD_AFF_API bool PaaSetMipmaps(Paa* paa, Mipmap** mipmap, uint8_t size);
    extern GRAD_AFF_API size_t PaaGetMipmapCount(Paa* paa);
    extern GRAD_AFF_API bool PaaGetMipmaps(Paa* paa, Mipmap** mipmaps, uint8_t size);
    extern GRAD_AFF_API Mipmap* PaaGetMipmap(Paa* paa, uint8_t level);
    extern GRAD_AFF_API bool PaaSetMipmap(Paa* paa, Mipmap* mipMap, uint8_t level);

    extern GRAD_AFF_API uint8_t PaaGetOptimalMipMapIndex(Paa* paa, uint16_t cx);
    extern GRAD_AFF_API Mipmap* PaaGetOptimalMipMap(Paa* paa, uint16_t cx);

    extern GRAD_AFF_API size_t PaaGetTaggCount(Paa* paa);
    extern GRAD_AFF_API bool PaaSetTaggs(Paa* paa, Tagg** taggs, size_t size);
    extern GRAD_AFF_API bool PaaGetTaggs(Paa* paa, Tagg** taggs, size_t size);

    extern GRAD_AFF_API Tagg* PaaGetTagg(Paa* paa, const char* signature, size_t size);

    extern GRAD_AFF_API size_t PaaGetPixelDataCount(Paa* paa, uint8_t level);
    extern GRAD_AFF_API bool PaaSetPixelData(Paa* paa, uint8_t* data, size_t size, uint8_t level);
    extern GRAD_AFF_API bool PaaGetPixelData(Paa* paa, uint8_t* data, size_t size, uint8_t level);

    extern GRAD_AFF_API bool PaaSetPixel(Paa* paa, size_t x, size_t y, uint8_t level, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    extern GRAD_AFF_API bool PaaGetPixel(Paa* paa, size_t x, size_t y, uint8_t level, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);

    extern GRAD_AFF_API bool PaaIsTransparent(Paa* paa);
    extern GRAD_AFF_API bool PaaIsValid(Paa* paa);

    extern GRAD_AFF_API bool PaaCalcMipmapsAndTaggs(Paa* paa);

    extern GRAD_AFF_API bool PaaIsPowerOfTwo(uint32_t x);

    extern GRAD_AFF_API bool PaaReadFile(Paa* paa, const char* filename, bool lazy);
    extern GRAD_AFF_API bool PaaReadData(Paa* paa, const uint8_t* data, size_t size, bool lazy);

    extern GRAD_AFF_API bool PaaWriteFile(Paa* paa, const char* filename);
    extern GRAD_AFF_API uint8_t* PaaWriteData(Paa* paa, size_t* size);
    extern GRAD_AFF_API bool PaaDestroyWrittenData(uint8_t* data);




#ifdef __cplusplus
}
};
#endif
