#pragma once


#include "../core/AffApi.h"

#ifdef __cplusplus

#include <vector>
#include "../core/AffBase.h"

namespace grad::aff::Paa {
    class GRAD_AFF_API Mipmap {
    public:
        uint16_t width = 0;
        uint16_t height = 0;
        bool isLzoCompressed = false;
        std::vector<uint8_t> data = {};
        uint32_t dataSize = 0;
    };
#else

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef
struct Mipmap
    Mipmap;
#endif

#ifdef __cplusplus
extern "C" {
#endif
    extern GRAD_AFF_API Mipmap* MipmapCreate();
    extern GRAD_AFF_API Mipmap* MipmapClone(Mipmap* rhsMipmap);
    extern GRAD_AFF_API bool MipmapDestroy(Mipmap* mipmap);

    extern GRAD_AFF_API uint16_t MipmapGetWidth(Mipmap* mipmap);
    extern GRAD_AFF_API bool MipmapSetWidth(Mipmap* mipmap, uint16_t width);

    extern GRAD_AFF_API uint16_t MipmapGetHeight(Mipmap* mipmap);
    extern GRAD_AFF_API bool MipmapSetHeight(Mipmap* mipmap, uint16_t height);

    extern GRAD_AFF_API bool MipmapIsLzoCompressed(Mipmap* mipmap);
    //extern GRAD_AFF_API void MipmapSetLzoCompressed(Mipmap* mipmap, bool isLzoCompressed);

    extern GRAD_AFF_API size_t MipmapGetDataSize(Mipmap* mipmap);
    extern GRAD_AFF_API bool MipmapSetData(Mipmap* mipmap, uint8_t* data, size_t size);
    extern GRAD_AFF_API bool MipmapGetData(Mipmap* mipmap, uint8_t* data, size_t size);

#ifdef __cplusplus
}
};
#endif
/*
struct MipMap {
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t dataLength = 0;
    std::vector<uint8_t> data = {};
    bool lzoCompressed = false;
};
*/
