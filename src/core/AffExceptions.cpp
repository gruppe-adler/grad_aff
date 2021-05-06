#include "core/AffExceptions.h"

bool grad::aff::core::ExceptionHelper::ValidHandle(void* handle) {
    return handle != nullptr;
}

void grad::aff::core::ExceptionHelper::SetLastError(std::exception& ex)
{
    if (dynamic_cast<InvalidStateException*>(&ex) != nullptr) {
        core::ExceptionHelper::LastErrorCode = static_cast<uint32_t>(AFFError::InvalidState);
    }
    else if (dynamic_cast<CompressionException*>(&ex) != nullptr) {
        core::ExceptionHelper::LastErrorCode = static_cast<uint32_t>(AFFError::CompressionError);
    }
    else if (dynamic_cast<ArgumentException*>(&ex) != nullptr) {
        core::ExceptionHelper::LastErrorCode = static_cast<uint32_t>(AFFError::ArgumentError);
    }
    else if (dynamic_cast<ReadException*>(&ex) != nullptr) {
        core::ExceptionHelper::LastErrorCode = static_cast<uint32_t>(AFFError::ReadError);
    }
    else if (dynamic_cast<std::ios::failure*>(&ex) != nullptr) {
        core::ExceptionHelper::LastErrorCode = static_cast<uint32_t>(AFFError::IOError);
    }
    else {
        core::ExceptionHelper::LastErrorCode = static_cast<uint32_t>(AFFError::UnkownError);
    }
}

void grad::aff::core::ExceptionHelper::SetLastError(grad::aff::core::AFFError error)
{
    core::ExceptionHelper::LastErrorCode = static_cast<uint32_t>(error);
}

int32_t grad::aff::core::ExceptionHelper::LastErrorCode = 0;

