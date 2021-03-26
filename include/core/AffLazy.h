#pragma once


#include <cstdint>
#include <memory>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#include <ObjIdl.h>
#endif

#include "AffTypes.h"
#include "AffDefines.h"

#include "AffBase.h"

namespace grad::aff::core {
    class AffLazy : public AffBase {
    public:

        AffLazy() {}

        virtual void readLazy(std::filesystem::path path);
        virtual void readLazy(std::string path);
        virtual void readLazy(std::wstring path);
        virtual void readLazy(const char* path);
        virtual void readLazy(std::vector<char> data);
        virtual void readLazy(std::vector<uint8_t> data);

#ifdef _WIN32
        virtual void readLazy(IStream* istream);
#endif

    protected:
        virtual void readFromStream() override;
        virtual void readLazyFromStream(bool lazy) = 0;
    };
}

