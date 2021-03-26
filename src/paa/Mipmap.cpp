#include "paa/MipMap.h"

grad::aff::Paa::Mipmap* grad::aff::Paa::MipmapCreate() {
    return new grad::aff::Paa::Mipmap();
}

grad::aff::Paa::Mipmap* grad::aff::Paa::MipmapClone(Mipmap* rhsMipmap) {
    auto newMipmap = MipmapCreate();
    *newMipmap = *rhsMipmap;
    return newMipmap;
}

void grad::aff::Paa::MipmapDestroy(Mipmap* mipMap) {
    delete mipMap;
}

uint16_t grad::aff::Paa::MipmapGetWidth(Mipmap* mipMap) {
    return mipMap->width;
}
void grad::aff::Paa::MipmapSetWidth(Mipmap* mipMap, uint16_t width) {
    mipMap->width = width;
}

uint16_t grad::aff::Paa::MipmapGetHeight(Mipmap* mipMap) {
    return mipMap->height;
}
void grad::aff::Paa::MipmapSetHeight(Mipmap* mipMap, uint16_t height) {
    mipMap->height = height;
}

bool grad::aff::Paa::MipmapIsLzoCompressed(Mipmap* mipMap) {
    return mipMap->isLzoCompressed;
}

void grad::aff::Paa::MipmapSetLzoCompressed(Mipmap* mipmap, bool isLzoCompressed) {
    mipmap->isLzoCompressed = isLzoCompressed;
}

size_t grad::aff::Paa::MipmapGetDataSize(Mipmap* mipMap) {
    return mipMap->data.size();
}

void grad::aff::Paa::MipmapSetData(Mipmap* mipMap, uint8_t* data, size_t dataSize) {
    mipMap->data = std::vector<uint8_t>(data, data + dataSize);
}

void grad::aff::Paa::MipmapGetData(Mipmap* mipmap, uint8_t** data, size_t size) {
    if (!data || !*data) {
        return;
    }
    if (size < mipmap->data.size()) {
        *data = nullptr;
    }
    else {
        std::memcpy(*data, mipmap->data.data(), mipmap->data.size());
    }
    /*uint8_t* dataPtr = (uint8_t*)malloc(mipMap->data.size());

    if (dataPtr) {
        std::memcpy(dataPtr, mipMap->data.data(), mipMap->data.size());
        *data = dataPtr;
        *dataSize = mipMap->data.size();
    }
    else {
        *data = nullptr;
        *dataSize = 0;
    }*/
}
