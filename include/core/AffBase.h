#pragma once

#include <cstdint>
#include <memory>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cassert>
#include <cstring>
#include <vector>

#ifdef _WIN32
#include <ObjIdl.h>
#endif

#include "AffTypes.h"
#include "AffDefines.h"

#include "AffExceptions.h"

namespace grad::aff::core {
    class AffBase {
    public:
        AffBase() {}

        virtual void read(std::filesystem::path path);
        virtual void read(std::string path);
        virtual void read(std::wstring path);
        virtual void read(const char* path);
        virtual void read(std::vector<char> data);
        virtual void read(std::vector<uint8_t> data);

#ifdef _WIN32
        virtual void read(IStream* istream);
        virtual void write(IStream* istream);
#endif
        virtual void write();
        virtual void write(std::filesystem::path path);
        virtual void write(const char* path);
        virtual void write(std::string path);
        virtual void write(std::vector<uint8_t>& data);

    protected:
        std::shared_ptr<std::basic_iostream<char>> stream = {};
        std::vector<char> streamData = {};

        virtual void readFromStream() = 0;
        virtual void writeToStream(std::shared_ptr<std::basic_iostream<char>> stream) = 0;

        /// @brief 
        /// @tparam T 
        /// @param is 
        /// @return 
        template<typename T>
        T readAs() {
            T t = {};
            auto size = sizeof(T);
            stream->read(reinterpret_cast<char*>(&t), size);
            return t;
        }

        template<typename T>
        T readAtAs(std::streamoff offset)
        {
            size_t pos = stream->tellg();
            stream->seekg(offset, std::ios::cur);
            T t = readAs<T>();
            stream->seekg(pos);
            return t;
        }

        /// @brief 
        /// @param is 
        /// @return 
        uint32_t readAsArmaUShort() {
            uint32_t t = 0;
            stream->read(reinterpret_cast<char*>(&t), 3);
            return t;
        };

        // https://community.bistudio.com/wiki/raP_File_Format_-_OFP#CompressedInteger
        
        /// @brief See also <a href="https://community.bistudio.com/wiki/raP_File_Format_-_OFP#CompressedInteger">CompressedInteger</a>
        /// @param is 
        /// @return 
        uint32_t readCompressedInteger() {
            auto val = readAs<uint8_t>();
            auto ret = val;
            while (val & 0x80) {
                val = readAs<uint8_t>();
                ret += (val - 1) * 0x80;
            }
            return ret;
        }

        types::XYZTriplet readXYZTriplet() {
            return types::XYZTriplet { readAs<float_t>(), readAs<float_t>(), readAs<float_t>() };
        }


        types::TransformMatrix readTransformMatrix() {
            types::TransformMatrix transformMatrix = {};
            for (auto i = 0; i < 4; i++) {
                transformMatrix[i] = readXYZTriplet();
            }
            return transformMatrix;
        }

        types::D3DCOLORVALUE readD3DColorValue() {
            types::D3DCOLORVALUE colorValue = {};
            for (auto i = 0; i < 4; i++) {
                colorValue[i] = readAs<float_t>();
            }
            return colorValue;
        }

        std::string readString(size_t length) {
            std::vector<uint8_t> result(length, 0);
            stream->read(reinterpret_cast<char*>(&result[0]), length);
            return std::string(result.begin(), result.end());
        }

        std::vector<uint8_t> readAsBytes(std::streamsize length) {
            if (length == 0)
                return {}; // @NOCOVERAGE

            std::vector<uint8_t> result(length, 0);
            stream->read(reinterpret_cast<char*>(&result[0]), length);
            return result;
        }

        std::string readZeroTerminatedString() {
            std::string result;
            std::getline(*stream, result, '\0');
            return result;
        }

        std::chrono::milliseconds readTimestamp() {
            return std::chrono::milliseconds(std::chrono::duration<long>(readAs<uint32_t>()));
        }

        std::pair<std::vector<uint8_t>, size_t> readLZOCompressed(size_t expectedSize) {
            auto retVec = std::vector<uint8_t>(expectedSize);
            auto retCode = decompressLZO(retVec, expectedSize);
            return std::make_pair(retVec, retCode);
        }

        template <typename T>
        std::pair<std::vector<T>, size_t> readLZOCompressed(size_t expectedSize) {
            if (expectedSize == 0)
                return {};

            auto bVec = readLZOCompressed(expectedSize);

            std::vector<T> retVec = {};
            retVec.reserve(sizeof(T) * expectedSize);

            for (size_t i = 0; i < bVec.first.size(); i += 4) {
                T f = {};
                memcpy(&f, &bVec.first.data()[i], sizeof(T));
                retVec.push_back(f);
            }
            return std::make_pair(retVec, bVec.second);

        }

        std::vector<uint8_t> readCompressed(size_t expectedSize, bool useCompressionFlag) {
            if (expectedSize == 0)
                return {};
            bool flag = expectedSize >= 1024;
            if (useCompressionFlag) {
                flag = readAs<bool>();
            }
            if (!flag) {
                return readAsBytes(expectedSize);
            }
            return readLZOCompressed<uint8_t>(expectedSize).first;
        }

        template<typename T>
        std::vector<T> readCompressedArray(size_t expectedSize, bool useCompressionFlag) {
            auto n = readAs<uint32_t>();
            if (n == 0)
                return {};

            std::vector<uint8_t> uncompressedData = readCompressed(n * expectedSize, useCompressionFlag);
            std::vector<T> retVec = {};
            retVec.reserve(sizeof(T) * expectedSize);

            for (size_t i = 0; i < uncompressedData.size(); i += 4) {
                T f = {};
                memcpy(&f, &uncompressedData.data()[i], sizeof(T));
                retVec.push_back(f);
            }
            return retVec;
        }

        template<typename T>
        std::vector<T> readCompressedArray(size_t expectedSize, bool useCompressionFlag, size_t arrSize) {
            std::vector<uint8_t> uncompressed = readCompressed(arrSize * expectedSize, useCompressionFlag);
            std::vector<T> retVec = {};
            retVec.reserve(sizeof(T) * expectedSize);

            for (size_t i = 0; i < uncompressed.size(); i += 4) {
                T dt = {};
                memcpy(&dt, &uncompressed.data()[i], sizeof(T));
                retVec.push_back(dt);
            }
            return retVec;
        }

        template<typename T>
        std::vector<T> readCompressedFillArray(bool useCompressionFlag) {
            auto count = readAs<uint32_t>();

            auto defaultFill = readAs<bool>();

            std::vector<T> data = {};
            if (defaultFill) {
                T fillValue = readAs<T>();

                for (size_t i = 0; i < count; i++) {
                    data.push_back(defaultFill);
                }
            }
            else {
                data = readCompressedArray<T>(count, useCompressionFlag);
            }
            return data;
        }

        // peek
        template<typename T>
        T peekAs() {
            auto pos = stream->tellg();
            T t = {};
            stream->read(reinterpret_cast<char*>(&t), sizeof(T));
            stream->seekg(pos, std::ios::beg);
            return t;
        }

        // Helper

        // Based on https://community.bistudio.com/wiki/Compressed_LZO_File_Format
        size_t decompressLZO(std::vector<uint8_t>& output, size_t expectedSize)
        {
            auto startPos = stream->tellg();
            std::vector<uint8_t>::iterator op;
            unsigned int t = 0;
            std::vector<uint8_t>::iterator m_pos;

            auto op_end = output.end();
            op = output.begin();

            if (peekAs<uint8_t>() > 17)
            {
                t = readAs<uint8_t>() - 17U;
                if (t < 4) goto match_next;

                assert(t > 0);
                if ((op_end - op) < (t)) {
                    throw std::overflow_error("Output Overun");
                }
                do *op++ = readAs<uint8_t>(); while (--t > 0);
                goto first_literal_run;
            }

        B_3:
            t = (size_t)readAs<uint8_t>();
            if (t >= 16) goto match;

            if (t == 0)
            {
                while (peekAs<uint8_t>() == 0)
                {
                    t += 255;
                    stream->seekg(1, std::ios_base::cur);
                }
                t += 15U + readAs<uint8_t>();
            }
            assert(t > 0);
            if ((op_end - op) < (static_cast<long long>(t) + 3)) {
                throw std::overflow_error("Output Overrun");
            }

            *op++ = readAs<uint8_t>();
            *op++ = readAs<uint8_t>();
            *op++ = readAs<uint8_t>();
            *op++ = readAs<uint8_t>();
            if (--t > 0)
            {
                if (t >= 4)
                {
                    do
                    {
                        *op++ = readAs<uint8_t>();
                        *op++ = readAs<uint8_t>();
                        *op++ = readAs<uint8_t>();
                        *op++ = readAs<uint8_t>();
                        t -= 4;
                    } while (t >= 4);
                    if (t > 0) do *op++ = readAs<uint8_t>(); while (--t > 0);
                }
                else
                    do *op++ = readAs<uint8_t>(); while (--t > 0);
            }

        first_literal_run:
            t = readAs<uint8_t>();
            if (t >= 16) goto match;

            m_pos = op - (1 + M2_MAX_OFFSET);
            m_pos -= t >> 2;
            m_pos -= readAs<uint8_t>() << 2;

            if (m_pos < output.begin() || m_pos >= op) {
                throw std::underflow_error("Lookbehind Overrun");
            }
            if ((op_end - op) < (3)) {
                throw std::overflow_error("Output Overrun");
            }
            *op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos;

            goto match_done;

        match:
            if (t >= 64)
            {
                m_pos = op - 1;
                m_pos -= (t >> 2) & 7;
                m_pos -= readAs<uint8_t>() << 3;
                t = (t >> 5) - 1;
                if (m_pos < output.begin() || m_pos >= op) {
                    throw std::underflow_error("Lookbeding Overrun");
                }
                assert(t > 0);
                if ((op_end - op) < (t + 2)) {
                    throw std::overflow_error("Output Overrun");
                }
                goto copy_match;
            }
            else if (t >= 32)
            {
                t &= 31;
                if (t == 0)
                {
                    while (peekAs<uint8_t>() == 0)
                    {
                        t += 255;
                        stream->seekg(1, std::ios_base::cur);
                    }
                    t += 31U + readAs<uint8_t>();
                }

                m_pos = op - 1;
                m_pos -= (readAtAs<uint8_t>(0) >> 2) + (readAtAs<uint8_t>(1) << 6);

                stream->seekg(2, std::ios_base::cur);
            }
            else if (t >= 16)
            {
                m_pos = op;
                m_pos -= (t & 8) << 11;

                t &= 7;
                if (t == 0)
                {
                    while (peekAs<uint8_t>() == 0)
                    {
                        t += 255;
                        stream->seekg(1, std::ios_base::cur);
                    }
                    t += 7U + readAs<uint8_t>();
                }

                m_pos -= (readAtAs<uint8_t>(0) >> 2) + (readAtAs<uint8_t>(1) << 6);

                stream->seekg(2, std::ios_base::cur);

                if (m_pos == op)
                {
                    assert(t == 1);
                    if (m_pos != op_end) {
                        throw std::overflow_error("Output Overrun");
                    }
                    return stream->tellg() - startPos;
                }
                m_pos -= 0x4000;
            }
            else
            {
                m_pos = op - 1;
                m_pos -= t >> 2;
                m_pos -= readAs<uint8_t>() << 2;

                if (m_pos < output.begin() || m_pos >= op) {
                    throw std::underflow_error("Lookbehind Overrun");
                }
                if ((op_end - op) < (2)) {
                    throw std::overflow_error("Output Overrun");
                }
                *op++ = *m_pos++; *op++ = *m_pos;
                goto match_done;
            }

            if (m_pos < output.begin() || m_pos >= op) {
                throw std::underflow_error("Lookbehind Overrun");
            }
            assert(t > 0);
            if ((op_end - op) < (t + 2)) {
                throw std::overflow_error("Output Overrun");
            }

            if (t >= 2 * 4 - (3 - 1) && (op - m_pos) >= 4)
            {
                *op++ = *m_pos++;
                *op++ = *m_pos++;
                *op++ = *m_pos++;
                *op++ = *m_pos++;
                t -= 4 - (3 - 1);
                do
                {
                    *op++ = *m_pos++;
                    *op++ = *m_pos++;
                    *op++ = *m_pos++;
                    *op++ = *m_pos++;
                    t -= 4;
                } while (t >= 4);
                if (t > 0) do *op++ = *m_pos++; while (--t > 0);
                goto match_done;
            }

        copy_match:
            *op++ = *m_pos++; *op++ = *m_pos++;
            do *op++ = *m_pos++; while (--t > 0);

        match_done:
            t = readAtAs<uint8_t>(-2) & 3U;
            if (t == 0) goto B_3;

        match_next:
            assert(t > 0 && t < 4);
            if ((op_end - op) < (t)) {
                throw std::overflow_error("Output Overrun");
            }

            *op++ = readAs<uint8_t>();
            if (t > 1) { *op++ = readAs<uint8_t>(); if (t > 2) { *op++ = readAs<uint8_t>(); } }

            t = readAs<uint8_t>();
            goto match;
        }

        // Write

        template<typename T>
        void writeAs(const std::shared_ptr<std::basic_iostream<char>>& outStream, T t) {
            outStream->write(reinterpret_cast<char*>(&t), sizeof(T));
        }

        void writeAsArmaUShort(const std::shared_ptr<std::basic_iostream<char>>& outStream, uint32_t t) {
            outStream->write(reinterpret_cast<char*>(&t), 3);
        };

        void writeString(const std::shared_ptr<std::basic_iostream<char>>& outStream, std::string string) {
            outStream->write(string.data(), string.size());
        }

        void writeBytes(const std::shared_ptr<std::basic_iostream<char>>& outStream, std::vector<uint8_t> bytes) {
            outStream->write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        }

        void writeZeroTerminatedString(const std::shared_ptr<std::basic_iostream<char>>& outStream, std::string string) {
            writeString(outStream, string);
            outStream->write("\0", 1);
        }

        void writeTimestamp(const std::shared_ptr<std::basic_iostream<char>>& outStream, std::chrono::milliseconds milliseconds) {
            writeAs<uint32_t>(outStream, static_cast<uint32_t>(milliseconds.count()));
        }

    };
}
