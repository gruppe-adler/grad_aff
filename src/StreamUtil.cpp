#include "grad_aff/StreamUtil.h"

/*
    Read
*/

template<typename T>
T grad_aff::readBytes(std::istream& i, std::streamoff offset)
{
    size_t pos = i.tellg();
    uint8_t b;
    i.seekg(offset, std::ios::cur);
    T t = readBytes<T>(i);
    i.seekg(pos);
    return t;
}

// byte
template uint8_t grad_aff::readBytes<uint8_t>(std::istream& is, std::streamoff offset);

template<typename T>
T grad_aff::readBytes(std::istream& is) {
    T t = 0;
    is.read(reinterpret_cast<char*>(&t), sizeof(T));
    return t;
}

uint32_t grad_aff::readBytesAsArmaUShort(std::istream& is) {
    uint32_t t = 0;
    is.read(reinterpret_cast<char*>(&t), 3);
    return t;
};
// bool
template bool grad_aff::readBytes<bool>(std::istream& is);
// byte
template uint8_t grad_aff::readBytes<uint8_t>(std::istream& is);
// byte
template int8_t grad_aff::readBytes<int8_t>(std::istream& is);
// ulong
template uint32_t grad_aff::readBytes<uint32_t>(std::istream& is);
// long
template int32_t grad_aff::readBytes<int32_t>(std::istream& is);
// ushort
template uint16_t grad_aff::readBytes<uint16_t>(std::istream& is);
// short
template int16_t grad_aff::readBytes<int16_t>(std::istream& is);
// float
template float_t grad_aff::readBytes<float_t>(std::istream& is);

// https://community.bistudio.com/wiki/raP_File_Format_-_OFP#CompressedInteger
uint32_t grad_aff::readCompressedInteger(std::istream& is) {
    auto val = readBytes<uint8_t>(is);
    auto ret = val;
    while (val & 0x80) {
        val = readBytes<uint8_t>(is);
        ret += (val - 1) * 0x80;
    }
    return ret;
}

template<typename T>
T grad_aff::peekBytes(std::istream& is) {
    auto pos = is.tellg();
    T t = 0;
    is.read(reinterpret_cast<char*>(&t), sizeof(T));
    is.seekg(pos);
    return t;
}
// byte
template uint8_t grad_aff::peekBytes<uint8_t>(std::istream& is);
// ulong
template uint32_t grad_aff::peekBytes<uint32_t>(std::istream& is);
// ushort
template uint16_t grad_aff::peekBytes<uint16_t>(std::istream& is);
// float
template float_t grad_aff::peekBytes<float_t>(std::istream& is);

XYZTriplet grad_aff::readXYZTriplet(std::istream& is) {
    return std::array<float_t, 3> { readBytes<float_t>(is), readBytes<float_t>(is), readBytes<float_t>(is) };
}

TransformMatrix grad_aff::readMatrix(std::istream& is) {
    TransformMatrix matrix = {};
    for (auto i = 0; i < 4; i++) {
        matrix[i] = readXYZTriplet(is);
    }
    return matrix;
}

D3DCOLORVALUE grad_aff::readD3ColorValue(std::istream& is) {
    D3DCOLORVALUE colorValue = {};
    for (auto i = 0; i < 4; i++) {
        colorValue[i] = readBytes<float_t>(is);
    }
    return colorValue;
}

std::string grad_aff::readString(std::istream& is, int count) {
    std::vector<uint8_t> result(count);
    is.read(reinterpret_cast<char*>(&result[0]), count);
    return std::string(result.begin(), result.end());
}

std::vector<uint8_t> grad_aff::readBytes(std::istream& is, std::streamsize length) {

    if (length == 0)
        return {};

    std::vector<uint8_t> result(length);
    is.read(reinterpret_cast<char*>(&result[0]), length);
    return result;
}

std::string grad_aff::readZeroTerminatedString(std::istream& is) {
    std::string result;
    std::getline(is, result, '\0');
    return result;
}

std::chrono::milliseconds grad_aff::readTimestamp(std::istream& is) {
    return std::chrono::milliseconds(std::chrono::duration<long>(readBytes<uint32_t>(is)));
}

std::pair<std::vector<uint8_t>, size_t> grad_aff::readLZOCompressed(std::istream& is, size_t expectedSize) {
    auto retVec = std::vector<uint8_t>(expectedSize);
    auto retCode = Decompress(is, retVec, expectedSize);
    return std::make_pair(retVec, retCode);
}

template <typename T>
std::pair<std::vector<T>, size_t> grad_aff::readLZOCompressed(std::istream& is, size_t expectedSize) {
    if (expectedSize == 0)
        return {};

    auto bVec = readLZOCompressed(is, expectedSize);

    std::vector<T> retVec;
    retVec.reserve(sizeof(T) * expectedSize);

    for (size_t i = 0; i < bVec.first.size(); i += 4) {
        T f;
        memcpy(&f, &bVec.first.data()[i], sizeof(T));
        retVec.push_back(f);
    }
    return std::make_pair(retVec, bVec.second);

}

template std::pair<std::vector<float_t>, size_t> grad_aff::readLZOCompressed(std::istream& is, size_t expectedSize);
template std::pair<std::vector<uint8_t>, size_t> grad_aff::readLZOCompressed(std::istream& is, size_t expectedSize);
template std::pair<std::vector<uint16_t>, size_t> grad_aff::readLZOCompressed(std::istream& is, size_t expectedSize);
template std::pair<std::vector<uint32_t>, size_t> grad_aff::readLZOCompressed(std::istream& is, size_t expectedSize);


std::vector<uint8_t> grad_aff::readCompressed(std::istream& is, size_t expectedSize, bool useCompressionFlag) {
    if (expectedSize == 0)
        return {};
    bool flag = expectedSize >= 1024;
    if (useCompressionFlag) {
        flag = readBytes<bool>(is);
    }
    if (!flag) {
        return readBytes(is, expectedSize);
    }
    return readLZOCompressed<uint8_t>(is, expectedSize).first;
}

template<typename T>
std::vector<T> grad_aff::readCompressedArray(std::istream& is, size_t expectedSize, bool useCompressionFlag) {
    auto n = readBytes<uint32_t>(is);
    if (n == 0)
        return {};

    auto uncomp = readCompressed(is, n * expectedSize, useCompressionFlag);
    std::vector<T> retVec;
    retVec.reserve(sizeof(T) * expectedSize);

    for (size_t i = 0; i < uncomp.size(); i += 4) {
        T f;
        memcpy(&f, &uncomp.data()[i], sizeof(T));
        retVec.push_back(f);
    }
    return retVec;
}

template std::vector<uint32_t> grad_aff::readCompressedArray(std::istream& is, size_t expectedSize, bool useCompressionFlag);
template std::vector<uint16_t> grad_aff::readCompressedArray(std::istream& is, size_t expectedSize, bool useCompressionFlag);
template std::vector<float_t> grad_aff::readCompressedArray(std::istream& is, size_t expectedSize, bool useCompressionFlag);

template<typename T>
std::vector<T> grad_aff::readCompressedArray(std::istream& is, size_t expectedSize, bool useCompressionFlag, size_t arrSize) {
    auto uncomp = readCompressed(is, arrSize * expectedSize, useCompressionFlag);
    std::vector<T> retVec;
    retVec.reserve(sizeof(T) * expectedSize);

    for (size_t i = 0; i < uncomp.size(); i += 4) {
        T f;
        memcpy(&f, &uncomp.data()[i], sizeof(T));
        retVec.push_back(f);
    }
    return retVec;
}
template std::vector<uint32_t> grad_aff::readCompressedArray(std::istream& is, size_t expectedSize, bool useCompressionFlag, size_t arrSize);

/*
    Write
*/

template<typename T>
void grad_aff::writeBytes(std::ostream& ofs, T t) {
    ofs.write(reinterpret_cast<char*>(&t), sizeof(T));
}

void grad_aff::writeBytesAsArmaUShort(std::ostream& ofs, uint32_t t) {
    ofs.write(reinterpret_cast<char*>(&t), 3);
};

// byte
template void grad_aff::writeBytes<uint8_t>(std::ostream & is, uint8_t t);
// ulong
template void grad_aff::writeBytes<uint32_t>(std::ostream& ofs, uint32_t t);
// ushort
template void grad_aff::writeBytes<uint16_t>(std::ostream& ofs, uint16_t t);
// float
template void grad_aff::writeBytes<float_t>(std::ostream& ofs, float_t t);

void grad_aff::writeString(std::ostream& ofs, std::string string) {
    ofs.write(string.data(), string.size());
}

void grad_aff::writeBytes(std::ostream& ofs, std::vector<uint8_t> bytes) {
    ofs.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

void grad_aff::writeZeroTerminatedString(std::ostream& ofs, std::string string) {
    writeString(ofs, string);
    ofs.write("\0", 1);
}

void grad_aff::writeTimestamp(std::ostream& ofs, std::chrono::milliseconds milliseconds) {
    writeBytes<uint32_t>(ofs, milliseconds.count());
}