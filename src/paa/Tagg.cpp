#include "paa/Tagg.h"

#include "core/AffCompat.h"

grad::aff::Paa::Tagg* grad::aff::Paa::TaggCreate() {
    try {
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return new grad::aff::Paa::Tagg();
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

grad::aff::Paa::Tagg* grad::aff::Paa::TaggClone(Tagg* rhsTagg) {
    if (!core::ExceptionHelper::ValidHandle(rhsTagg)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return nullptr;
    }
    try {
        auto newTagg = TaggCreate();
        *newTagg = *rhsTagg;
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return newTagg;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

bool grad::aff::Paa::TaggDestroy(Tagg* tagg) {
    if (!core::ExceptionHelper::ValidHandle(tagg)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    delete tagg;
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return true;
}

bool grad::aff::Paa::TaggSetSignature(Tagg* tagg, const char* signature, size_t size) {
    if (!core::ExceptionHelper::ValidHandle(tagg)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        std::string sig(signature, size);
        tagg->signature = sig;
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

size_t grad::aff::Paa::TaggGetSignatureSize(Tagg* tagg) {
    if (!core::ExceptionHelper::ValidHandle(tagg)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return 0;
    }
    try {
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return tagg->signature.size() + 1;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return 0;
    }
}

bool grad::aff::Paa::TaggGetSignature(Tagg* tagg, char* signature, size_t size) {
    if (!core::ExceptionHelper::ValidHandle(tagg)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    if (size < tagg->signature.size() + 1) {
        core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
        return false;
    }
    else {
        try {
            std::memcpy(signature, tagg->signature.data(), tagg->signature.size());
            signature[tagg->signature.size()] = '\0';
            core::ExceptionHelper::SetLastError(core::AFFError::OK);
            return true;
        }
        catch (std::exception& ex) {
            core::ExceptionHelper::SetLastError(ex);
            return false;
        }
    }
}

size_t grad::aff::Paa::TaggGetDataSize(Tagg* tagg) {
    if (!core::ExceptionHelper::ValidHandle(tagg)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return 0;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return tagg->data.size();
}

bool grad::aff::Paa::TaggSetData(Tagg* tagg, uint8_t* data, size_t size) {
    if (!core::ExceptionHelper::ValidHandle(tagg)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        std::vector<uint8_t> dataVec(data, data + size);
        tagg->data = dataVec;
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::TaggGetData(Tagg* tagg, uint8_t* data, size_t size) {
    if (!core::ExceptionHelper::ValidHandle(tagg)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    if (size < tagg->data.size()) {
        core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
        return false;
    }
    else {
        try {
            std::memcpy(data, tagg->data.data(), tagg->data.size());
            return true;
        }
        catch (std::exception& ex) {
            core::ExceptionHelper::SetLastError(ex);
            return false;
        }
    }
}

