#pragma once

#ifdef __cplusplus

#include <stdexcept>
#include <ios>

namespace grad::aff::core {

    class ReadException : public std::runtime_error {
    public:
        ReadException(std::string msg) : std::runtime_error(msg) {}
    };

    class ArgumentException : public std::runtime_error {
    public:
        ArgumentException(std::string msg) : std::runtime_error(msg) {}
    };

    class CompressionException : public std::runtime_error {
    public:
        CompressionException(std::string msg) : std::runtime_error(msg) {}
    };

    class InvalidStateException : public std::runtime_error {
    public:
        InvalidStateException(std::string msg) : std::runtime_error(msg) {}
    };
#endif

    enum 
#ifdef __cplusplus
        class
#endif
        AFFError
#ifdef __cplusplus
        : int32_t
#endif
    {
        OK = 0,
        UnkownError = -1,
        ReadError = -2,
        ArgumentError = -3,
        InvalidState = -4,
        IOError = -5,
        CompressionError = -6,
        InvalidHandle = -7
    };

    class ExceptionHelper {
    public:

        static int32_t LastErrorCode;

        static bool ValidHandle(void* handle);

        static void SetLastError(std::exception& ex);

        static void SetLastError(AFFError error);
    };

#ifdef __cplusplus
};
#endif

