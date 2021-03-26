#pragma once

#include <string>
#include <filesystem>

//#include <Windows.h>
#include <shlwapi.h>

#include <boost/iostreams/stream.hpp>

namespace grad::aff::core::devices {
    class IStreamDevice {
    public:
        typedef typename char char_type;
        //typedef bi::seekable_device_tag category;

        struct category :
            boost::iostreams::seekable_device_tag,
            boost::iostreams::closable_tag
        {};

        IStreamDevice(IStream* istream)
            : istream_(istream), pos_(0) {
            istream->AddRef();
        }

        std::streamsize read(char_type* s, std::streamsize n) {
            istream_->Stat(&stat, STATFLAG_NONAME);

            std::streamsize maxStreamsize = static_cast<std::streamsize>(stat.cbSize.QuadPart - pos_);
            std::streamsize result = (std::min)(n, maxStreamsize);

            if (result != 0) {
                ULONG read = 0;
                auto hresult = istream_->Read(s, static_cast<ULONG>(result), &read);
                if (hresult != S_OK) {
                    throw std::ios_base::failure(std::system_category().message(hresult)); // @NOCOVERAGE
                }
                pos_ += read;
                return read;
            }
            else {
                return -1; // eof  // @NOCOVERAGE
            }
        }

        std::streamsize write(const char_type* s, std::streamsize n) {
            istream_->Stat(&stat, STATFLAG_NONAME);

            ULONG written = 0;

            auto hresult = istream_->Write(s, static_cast<ULONG>(n), &written);
            if (hresult != S_OK) {
                throw std::ios_base::failure(std::system_category().message(hresult)); // @NOCOVERAGE
            }

            pos_ += written;
            return n;
        }

        boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way) {
            ULARGE_INTEGER newPos = { 0 };

            DWORD origin{};

            switch (way)
            {
            case std::ios_base::_Seekdir::_Seekbeg:
                origin = STREAM_SEEK_SET;
                break;
            case std::ios_base::_Seekdir::_Seekcur:
                origin = STREAM_SEEK_CUR;
                break;
            case std::ios_base::_Seekdir::_Seekend:
                origin = STREAM_SEEK_END; // @NOCOVERAGE
                break; // @NOCOVERAGE
            default:
                throw std::ios_base::failure("bad seek direction");  // @NOCOVERAGE
                break;
            }
            LARGE_INTEGER lint{};
            lint.QuadPart = off;
            istream_->Seek(lint, origin, &newPos);
            
            pos_ = newPos.QuadPart;

            return newPos.QuadPart;
        }
        
        void close() {
            istream_->Release();
        }

        static std::wstring toWstring(std::string const& inStr) {
            std::wstring outStr{};

            if (inStr.length() > 0) {
                auto wStrLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, inStr.c_str(), static_cast<int>(inStr.size()), NULL, 0);

                if (wStrLen == 0) {
                    throw std::runtime_error("Invalid character sequence"); // @NOCOVERAGE
                }

                outStr.resize(wStrLen);

                MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, inStr.c_str(), static_cast<int>(inStr.size()), outStr.data(), static_cast<int>(outStr.size()));
            }

            return outStr;
        }

    private:
        typedef typename int64_t size_type;
        IStream* istream_;
        size_type pos_;
        STATSTG stat = {};

    };
}