#include "paa/Mipmap.h"

grad::aff::Paa::Mipmap* grad::aff::Paa::MipmapCreate() {
    try {
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return new grad::aff::Paa::Mipmap();
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

grad::aff::Paa::Mipmap* grad::aff::Paa::MipmapClone(Mipmap* rhsMipmap) {
    if (!core::ExceptionHelper::ValidHandle(rhsMipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return nullptr;
    }
    try {
        auto newMipmap = MipmapCreate();
        *newMipmap = *rhsMipmap;
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return newMipmap;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

bool grad::aff::Paa::MipmapDestroy(Mipmap* mipmap) {
    if (!core::ExceptionHelper::ValidHandle(mipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        delete mipmap;
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception &ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

uint16_t grad::aff::Paa::MipmapGetWidth(Mipmap* mipmap) {
    if (!core::ExceptionHelper::ValidHandle(mipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return 0;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return mipmap->width;
}
bool grad::aff::Paa::MipmapSetWidth(Mipmap* mipmap, uint16_t width) {
    if (!core::ExceptionHelper::ValidHandle(mipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    mipmap->width = width;
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return true;
}

uint16_t grad::aff::Paa::MipmapGetHeight(Mipmap* mipmap) {
    if (!core::ExceptionHelper::ValidHandle(mipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return 0;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return mipmap->height;
}
bool grad::aff::Paa::MipmapSetHeight(Mipmap* mipmap, uint16_t height) {
    if (!core::ExceptionHelper::ValidHandle(mipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    mipmap->height = height;
    return true;
}

bool grad::aff::Paa::MipmapIsLzoCompressed(Mipmap* mipmap) {
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return mipmap->isLzoCompressed;
}

//int32_t grad::aff::Paa::MipmapSetLzoCompressed(Mipmap* mipmap, bool isLzoCompressed) {
//    mipmap->isLzoCompressed = isLzoCompressed;
//}

size_t grad::aff::Paa::MipmapGetDataSize(Mipmap* mipmap) {
    if (!core::ExceptionHelper::ValidHandle(mipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return 0;
    }
    try {
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return mipmap->data.size();
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return 0;
    }
}

bool grad::aff::Paa::MipmapSetData(Mipmap* mipmap, uint8_t* data, size_t dataSize) {
    if (!core::ExceptionHelper::ValidHandle(mipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        mipmap->data = std::vector<uint8_t>(data, data + dataSize);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::MipmapGetData(Mipmap* mipmap, uint8_t* data, size_t size) {
    if (!core::ExceptionHelper::ValidHandle(mipmap)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    if (!data || size < mipmap->data.size()) {
        core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
        return false;
    }
    else {
        try {
            std::memcpy(data, mipmap->data.data(), mipmap->data.size());
            core::ExceptionHelper::SetLastError(core::AFFError::OK);
            return true;
        }
        catch (std::exception& ex) {
            core::ExceptionHelper::SetLastError(ex);
            return false;
        }
    }
}
