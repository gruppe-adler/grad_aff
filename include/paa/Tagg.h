#pragma once


#include "../core/AffApi.h"

#ifdef __cplusplus

#include <vector>
#include "../core/AffBase.h"

namespace grad::aff::Paa {
    class GRAD_AFF_API Tagg {
    public:
        std::string signature = "";
        std::vector<uint8_t> data = {};
    };
#else

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef
struct Tagg
    Tagg;
#endif

#ifdef __cplusplus
extern "C" {
#endif
    extern GRAD_AFF_API Tagg* TaggCreate();
    extern GRAD_AFF_API Tagg* TaggClone(Tagg* rhsTagg);
    extern GRAD_AFF_API bool TaggDestroy(Tagg* tagg);


    extern GRAD_AFF_API bool TaggSetSignature(Tagg* tagg, const char* signature, size_t size);
    extern GRAD_AFF_API size_t TaggGetSignatureSize(Tagg* tagg);
    extern GRAD_AFF_API bool TaggGetSignature(Tagg* tagg, char* signature, size_t size);

    extern GRAD_AFF_API size_t TaggGetDataSize(Tagg* tagg);
    extern GRAD_AFF_API bool TaggSetData(Tagg* tagg, uint8_t* data, size_t size);
    extern GRAD_AFF_API bool TaggGetData(Tagg* tagg, uint8_t* data, size_t size);


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


