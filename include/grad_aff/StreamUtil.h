#pragma once

#include "a3lzo.h"

#include "Types.h"

#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <chrono>
#include <array>
#include <type_traits>
#include <cstring>

namespace grad_aff {

    // Read offset
    template<typename T>
    T readBytes(std::istream& i, std::streamoff offset);

    // Read
    template<typename T>
    T readBytes(std::istream& is);
    
    uint32_t readBytesAsArmaUShort(std::istream& is);

    template<typename T>
    T peekBytes(std::istream& is);

    XYZTriplet readXYZTriplet(std::istream &is);
    TransformMatrix readMatrix(std::istream& is);
    D3DCOLORVALUE readD3ColorValue(std::istream& is);
    
    std::string readString(std::istream& is, int count);
    std::vector<uint8_t> readBytes(std::istream& is, std::streamsize length);

    std::string readZeroTerminatedString(std::istream& is);
    std::chrono::milliseconds readTimestamp(std::istream& is);

    uint32_t readCompressedInteger(std::istream& is);

    std::pair<std::vector<uint8_t>, size_t> readLZOCompressed(std::istream& is, size_t expectedSize);
    template<typename T>
    std::pair<std::vector<T>, size_t> readLZOCompressed(std::istream& is, size_t expectedSize);
    
    std::vector<uint8_t> readCompressed(std::istream& is, size_t expectedSize, bool useCompressionFlag);

    template<typename T>
    std::vector<T> readCompressedArray(std::istream& is, size_t expectedSize, bool useCompressionFlag);

    template<typename T>
    std::vector<T> readCompressedArray(std::istream& is, size_t expectedSize, bool useCompressionFlag, size_t arrSize);

    template<typename T>
    std::vector<T> readCompressedArrayOld(std::istream& is, size_t expectedSize, bool useCompressionFlag);

    template<typename T>
    std::vector<T> readCompressedFillArray(std::istream& is, bool useCompressionFlag);

    // Write
    template<typename T>
    void writeBytes(std::ostream& ofs, T t);
    
    void writeBytesAsArmaUShort(std::ostream& ofs, uint32_t t);

    void writeString(std::ostream& ofs, std::string string);
    void writeBytes(std::ostream& ofs, std::vector<uint8_t> bytes);

    void writeZeroTerminatedString(std::ostream& ofs, std::string string);
    void writeTimestamp(std::ostream& ofs, std::chrono::milliseconds milliseconds);


    // Compression
    std::vector<uint8_t> readCompressedLZOLZSS(std::istream& is, size_t expectedSize, bool useLzo);
    std::vector<uint8_t> readLzssBlock(std::istream& is, size_t expectedSize);

    size_t readLzss(std::vector<uint8_t> in, std::vector<uint8_t>& out);
    size_t readLzssFile(std::istream& is, std::vector<uint8_t>& out);
    size_t readLzssSized(std::istream& is, std::vector<uint8_t>& out, size_t expectedSize, bool useSignedChecksum);
}
