#include "paa/Paa.h"

#ifdef GRAD_AFF_USE_CPP17_PARALLELISM
#include <execution>
#include <algorithm>
#elif defined GRAD_AFF_USE_OPENMP
#include <omp.h>
#endif

#include <fstream>

#include <squish.h>
#include <lzokay.hpp>

#include "grad_aff/paa/squishMod.h"

#include <boost/gil.hpp>
#include <boost/gil/extension/numeric/resample.hpp>
#include <boost/gil/extension/numeric/sampler.hpp>

namespace bg = boost::gil;

using namespace grad::aff;

grad::aff::Paa::Paa::Paa() { }

/**
 * Private Methods
*/

void grad::aff::Paa::Paa::readLazyFromStream(bool lazy) {
    stream->exceptions(std::iostream::badbit | std::iostream::failbit | std::iostream::eofbit);
    //bool lazy = false;
    stream->seekg(0, std::ios::beg);
    magicNumber = readAs<uint16_t>();
    this->typeOfPax = static_cast<TypeOfPaX>(magicNumber);
    switch (this->typeOfPax)
    {
    case TypeOfPaX::DXT1:
    case TypeOfPaX::DXT5:
        break;
    case TypeOfPaX::DXT2:
    case TypeOfPaX::DXT3:
    case TypeOfPaX::DXT4:
    case TypeOfPaX::RGBA4444:
    case TypeOfPaX::RGBA5551:
    case TypeOfPaX::RGBA8888:
    case TypeOfPaX::GRAYwAlpha:
        throw grad::aff::core::ReadException("unsupported paa type");
    default:
        throw grad::aff::core::ReadException("invalid file/magic number");
    }

    // Taggs
    while (stream->peek() != 0)
    {
        Tagg tagg;
        tagg.signature = readString(8);
        auto dataSize = readAs<uint32_t>();
        tagg.data = readAsBytes(dataSize);
        taggs.push_back(tagg);

        if (tagg.signature == "GGATGALF") {
            hasTransparency = true;
        }
    }
    auto pos3 = stream->tellg();
    // TODO
    palette.dataLength = readAs<uint16_t>();
    if (palette.dataLength > 0) {
        palette.data = readAsBytes(palette.dataLength); // @NOCOVERAGE
    }

    // MipMaps
    while (peekAs<uint16_t>() != 0) {
        auto pos = stream->tellg();

        Mipmap mipMap;
        mipMap.width = readAs<uint16_t>();
        mipMap.height = readAs<uint16_t>();
        mipMap.dataSize = readAsArmaUShort();

        if ((mipMap.width & 0x8000) != 0) {
            mipMap.width &= 0x7FFF;
            mipMap.isLzoCompressed = true;
        }
        else {
            mipMap.isLzoCompressed = false;
        }
        
        if (lazy) {
            stream->seekg(static_cast<std::streamoff>(mipMap.dataSize), std::ios::cur);

            mipmaps.push_back(mipMap);
            mipmapPositions.push_back(pos);
        }
        else {
            auto compressedData = readAsBytes(mipMap.dataSize);
            mipMap.data = decompressMipMap(compressedData, mipMap.width, mipMap.height, mipMap.isLzoCompressed);

            mipmaps.push_back(mipMap);
            mipmapPositions.push_back(pos);
        }
    }

    auto pos = stream->tellg();
}

void grad::aff::Paa::Paa::readMipMapsData() {
    if (mipmaps.size() == 0) {
        throw grad::aff::core::InvalidStateException("no mipmaps exist");
    }

    for (size_t i = 0; i < mipmaps.size(); i++) {
        if (mipmaps[i].data.size() == 0) {
            readMipMapData(i);
        }
    }

}

void grad::aff::Paa::Paa::readMipMapData(uint8_t level) {
    if (level >= mipmaps.size() || level >= mipmapPositions.size() || mipmapPositions[level] == 0) {
        throw grad::aff::core::ArgumentException("level out of range");
    }
    auto& mipMap = mipmaps[level];
    auto pos = mipmapPositions[level] + static_cast<std::streampos>(7); // 2 * uint16 + uint24
    stream->seekg(pos, std::ios::beg);
    auto test = readAs<uint8_t>();
    stream->seekg(pos, std::ios::beg);
    mipMap.data = decompressMipMap(readAsBytes(mipMap.dataSize), mipMap.width, mipMap.height, mipMap.isLzoCompressed);
}

std::vector<uint8_t> grad::aff::Paa::Paa::decompressMipMap(std::vector<uint8_t> data, uint16_t width, uint16_t height, bool isLzoCompressed) {

    if (isLzoCompressed) {
        auto uncompressedSize = (size_t)width * height;
        if (this->typeOfPax == TypeOfPaX::DXT1) {
            uncompressedSize /= 2;
        }
        auto lzoUncompressed = std::vector<uint8_t>(uncompressedSize);

        size_t decompressedSize = 0;
        auto error = lzokay::decompress(data.data(), data.size(), lzoUncompressed.data(), lzoUncompressed.size(), decompressedSize);

        if (error != lzokay::EResult::Success) {
            throw grad::aff::core::CompressionException("LZO Decompression failed");  // @NOCOVERAGE
        }

        data = std::vector<uint8_t>(lzoUncompressed.data(), lzoUncompressed.data() + decompressedSize);
        data.resize(decompressedSize);
    }

    // LZO compression
    uint32_t ratio = 0;
    int flags = 0;
    if (this->typeOfPax == TypeOfPaX::DXT1) {
        ratio = 8;
        flags = squish::kDxt1;
    }
    else if (this->typeOfPax == TypeOfPaX::DXT5) {
        ratio = 4;
        flags = squish::kDxt5;
    }
    else {
        throw grad::aff::core::CompressionException("invalid compression type"); // @NOCOVERAGE
    }

    size_t uncompressedSize = data.size() * ratio;
    auto uncompressedData = std::vector<squish::u8>(uncompressedSize);

    squish::DecompressImage(uncompressedData.data(), width, height, data.data(), flags);

    return std::vector<uint8_t>(uncompressedData.data(), uncompressedData.data() + uncompressedSize);
}

/*
* Public Methods
*/


void grad::aff::Paa::Paa::setMipMaps(std::vector<Mipmap> mipmaps) {
    this->mipmaps = mipmaps;
}


std::vector<grad::aff::Paa::Mipmap> grad::aff::Paa::Paa::getMipMaps() {
    readMipMapsData();
    return this->mipmaps;
}

grad::aff::Paa::Mipmap grad::aff::Paa::Paa::getMipMap(uint8_t level) {
    if (level >= mipmaps.size()) {
        throw grad::aff::core::ArgumentException("level out of range");
    }

    if (mipmaps[level].data.size() == 0) {
        readMipMapData(level);
    }

    return mipmaps[level];
}

void grad::aff::Paa::Paa::setMipMap(grad::aff::Paa::Mipmap mipMap, uint8_t level) {
    if (level >= mipmaps.size()) {
        mipmaps.resize(static_cast<size_t>(level) + 1);
    }
    mipmaps[level] = mipMap;
}

uint8_t grad::aff::Paa::Paa::getOptimalMipMapIndex(uint16_t cx) {
    if (mipmaps.size() <= 0) {
        throw grad::aff::core::InvalidStateException("no mipmaps exist");

    }
    uint8_t result = 0;
    for (size_t i = 0; i < mipmaps.size(); i++)
    {
        auto maxSize = std::max(mipmaps[i].height, mipmaps[i].width);

        if (maxSize <= cx || maxSize == 4) {
            return i;
        }
        result = i;
    }
    return result; // @NOCOVERAGE
}

grad::aff::Paa::Mipmap grad::aff::Paa::Paa::getOptimalMipMap(uint16_t cx) {
    return getMipMap(getOptimalMipMapIndex(cx));
}

void grad::aff::Paa::Paa::setTaggs(std::vector<grad::aff::Paa::Tagg> taggs) {
    this->taggs = taggs;
}

std::vector<grad::aff::Paa::Tagg> grad::aff::Paa::Paa::getTaggs() const {
    return taggs;
}

grad::aff::Paa::Tagg grad::aff::Paa::Paa::getTagg(std::string signature) const {
    for (auto& tagg : taggs) {
        if (tagg.signature == signature) {
            return tagg;
        }
    }
    throw grad::aff::core::ArgumentException("signature not found");
}

std::vector<uint8_t> grad::aff::Paa::Paa::getPixelData(uint8_t level) {
    if (mipmaps.size() == 0 || mipmaps[0].data.size() == 0) {
        throw grad::aff::core::InvalidStateException("no mipmaps exist");
    }
    else {
        return getMipMap(level).data;
    }
}

void grad::aff::Paa::Paa::setPixelData(std::vector<uint8_t> data, uint8_t level) {
    this->mipmaps[level].data = data;
}

std::array<uint8_t, 4> grad::aff::Paa::Paa::getPixel(size_t x, size_t y, uint8_t level) {
    if (mipmaps.size() == 0) {
        throw grad::aff::core::InvalidStateException("no mipmaps exist");
    }
    else if (level >= mipmaps.size()) {
        throw grad::aff::core::ArgumentException("level out of range");
    }

    auto mipMap = getMipMap(level);
    if (x < mipMap.width && y < mipMap.height) {

        std::array<uint8_t, 4> result = {};
        auto arrPos = (x + y * mipmaps[level].width) * 4;
        for (int i = 0; i < 4; i++) {
            result[i] = mipmaps[level].data[arrPos + i];
        }
        return result;
    }
    else {
        throw grad::aff::core::ArgumentException("x/y out of range");
    }
}


void grad::aff::Paa::Paa::setPixel(size_t x, size_t y, std::array<uint8_t, 4> data, uint8_t level) {
    if (level >= mipmaps.size()) {
        throw grad::aff::core::ArgumentException("level out of range");
    }
    auto arrPos = (x + y * mipmaps[level].width) * 4;
    for (int i = 0; i < 4; i++) {
        mipmaps[level].data[arrPos + i] = data[i];
    }
}

bool grad::aff::Paa::Paa::isTransparent() const {
    return this->hasTransparency;
}

bool grad::aff::Paa::Paa::isValid() const noexcept {
    return mipmaps.size() > 0 && isPowerOfTwo(mipmaps[0].height) && isPowerOfTwo(mipmaps[0].height);
}

bool grad::aff::Paa::Paa::isPowerOfTwo(uint32_t x) noexcept {
    return (x != 0) && ((x & (x - 1)) == 0);
}

//void grad::aff::Paa::Paa::writePaa(std::string fileName, TypeOfPaX typeOfPaX) {
//    // Write everything
//    outStream = std::dynamic_pointer_cast<std::basic_iostream<char>>(std::make_shared<std::ofstream>(fileName, std::ios::binary));
//    writePaaToOutStream(typeOfPaX);
//    auto ofStream = std::dynamic_pointer_cast<std::ofstream> (outStream);
//    ofStream->close();
//}
//
//std::vector<uint8_t> grad::aff::Paa::Paa::writePaa(TypeOfPaX typeOfPax) {
//    outStream = std::make_shared<std::stringstream>();
//    writePaaToOutStream(typeOfPax);
//    auto sstream = std::dynamic_pointer_cast<std::stringstream> (outStream);
//    auto outputString = sstream->str();
//    return std::vector<uint8_t>(outputString.data(), outputString.data() + outputString.length());
//}

void grad::aff::Paa::Paa::writeToStream(std::shared_ptr<std::basic_iostream<char>> stream) {
    stream->seekg(0, std::ios::beg);

    if (mipmaps.size() <= 1)
        calculateMipmapsAndTaggs();

    auto encodedMipMaps = mipmaps;

    // Compression
    if (this->typeOfPax == TypeOfPaX::UNKNOWN) {
        this->typeOfPax = hasTransparency ? TypeOfPaX::DXT5 : TypeOfPaX::DXT1;
    }

    uint8_t compFactor = 0;
    uint16_t flag = 0;
    if (typeOfPax == TypeOfPaX::DXT5) {
        compFactor = 4;
        flag = squish::kDxt5;
    }
    else if (typeOfPax == TypeOfPaX::DXT1) {
        compFactor = 8;
        flag = squish::kDxt1;
    }

#ifdef GRAD_AFF_USE_CPP17_PARALLELISM
    std::for_each(std::execution::par_unseq, encodedMipMaps.begin(), encodedMipMaps.end(), [&](auto& mipmap) {
#else
#if defined GRAD_AFF_USE_OPENMP
    #pragma omp parallel for
#endif
    for (size_t i = 0; i < encodedMipMaps.size(); i++) {
        auto mipmap = encodedMipMaps[i];
#endif
        auto compressedDataLength = mipmap.data.size() / compFactor;
        auto compressedData = std::vector<uint8_t>(compressedDataLength);

        compressImage(reinterpret_cast<const uint8_t*>(mipmap.data.data()), (int)mipmap.width, (int)mipmap.height, (int)mipmap.width * 4, compressedData.data(), flag);

        mipmap.data = compressedData;
        mipmap.data.resize(compressedDataLength);

        encodedMipMaps[i] = mipmap;
    }
#ifdef GRAD_AFF_USE_CPP17_PARALLELISM
);
#endif

#ifdef GRAD_AFF_USE_CPP17_PARALLELISM
    std::for_each(std::execution::par_unseq, encodedMipMaps.begin(), encodedMipMaps.end(), [&](auto& mipmap) {
#else
#if defined GRAD_AFF_USE_OPENMP
#pragma omp parallel for
#endif
for (size_t i = 0; i < encodedMipMaps.size(); i++) {
    auto mipmap = encodedMipMaps[i];
#endif

        lzokay::Dict<> dict;
        if (mipmap.width > 128) {
            mipmap.isLzoCompressed = true;
            std::size_t estimatedSize = lzokay::compress_worst_size(mipmap.data.size());
            std::vector<uint8_t> outputData(estimatedSize);
            size_t compressedSize = 0;

            auto error = lzokay::compress(mipmap.data.data(), mipmap.data.size(), outputData.data(), estimatedSize, compressedSize, dict);
            if (error < lzokay::EResult::Success) {
                throw grad::aff::core::CompressionException("LZO Compression failed"); // @NOCOVERAGE
            }

            //encodedMipMap.data = std::vector<uint8_t>(outputData.data(), outputData.data() + compressedSize);
            mipmap.data = outputData;
            mipmap.data.resize(compressedSize);

            mipmap.width |= 0x8000;

            encodedMipMaps[i] = mipmap;
        }
    }
#ifdef GRAD_AFF_USE_CPP17_PARALLELISM
    );
#endif

    Tagg taggOffs;
    taggOffs.signature = "GGATSFFO";


    size_t initalOffset = 0;
    initalOffset += 2; // magic

    for (auto& tagg : taggs) {
        initalOffset += 8 + 4; // sig + size of length
        initalOffset += 4; // tagg.dataLength;
    }

    initalOffset += 8 + 4 + 16 * 4; // sig + size of length + 16 * 4byte
    initalOffset += 2; // palletteLength
    if (palette.dataLength > 0) {
        // TODO:
    }
    // initalOffset += 8;

    std::vector<char> offsetAsChars(4);
    int counter = 0;

    for (auto& mipmap : encodedMipMaps) {
        offsetAsChars = std::vector<char>(reinterpret_cast<char*>(&initalOffset), reinterpret_cast<char*>(&initalOffset) + 4);

        for (int i = 0; i < 4; i++) {
            taggOffs.data.push_back(offsetAsChars[i]);
        }

        initalOffset += mipmap.data.size() + 2 * 2 + 3;
        counter++;
    }

    // TODO Test
    if (taggOffs.data.size() < 64) {
        taggOffs.data.resize(64, 0x00);
    }

    //stream->trun
     // Write magic
    auto pos = stream->tellg();
    writeAs<uint16_t>(stream, static_cast<uint16_t>(typeOfPax));
    auto pos2 = stream->tellg();
    for (auto& tagg : taggs) {
        writeString(stream, tagg.signature);
        writeAs<uint32_t>(stream, static_cast<uint32_t>(tagg.data.size()));
        writeBytes(stream, tagg.data);
    }

    // Write offset Tag
    writeString(stream, taggOffs.signature);
    writeAs<uint32_t>(stream, static_cast<uint32_t>(taggOffs.data.size()));
    writeBytes(stream, taggOffs.data);

    writeAs<uint16_t>(stream, palette.dataLength);
    if (palette.dataLength > 0) {
        // TODO:
    }

    for (auto& mipmap : encodedMipMaps) {
        writeAs<uint16_t>(stream, mipmap.width);
        writeAs<uint16_t>(stream, mipmap.height);
        writeAsArmaUShort(stream, static_cast<uint32_t>(mipmap.data.size()));
        writeBytes(stream, mipmap.data);
    }

    writeAs<uint16_t>(stream, 0x00);
    writeAs<uint16_t>(stream, 0x00);
    writeAs<uint16_t>(stream, 0x00);

}

void grad::aff::Paa::Paa::calculateMipmapsAndTaggs() {

    readMipMapsData();
    
    if (mipmaps.size() == 0 || mipmaps[0].data.size() != static_cast<size_t>(mipmaps[0].width) * mipmaps[0].height * 4) {
        throw grad::aff::core::InvalidStateException("first mipmap is empty"); // @NOCOVERAGE
    }

    auto initalMipmap = mipmaps[0];
    auto initalWidth = initalMipmap.width;
    auto initalHeight = initalMipmap.height;
    auto initalData = initalMipmap.data;

    mipmaps.clear();
    mipmaps.push_back(initalMipmap);

    auto levelCount = static_cast<size_t>(std::log2(std::min(initalMipmap.height, initalMipmap.width)) - 1);
    mipmaps.reserve(levelCount + 1);

    auto prevData = initalData;
    
    // http://download.nvidia.com/developer/Papers/2005/NP2_Mipmapping/NP2_Mipmap_Creation.pdf
    // https://cgl.ethz.ch/teaching/former/vc_master_06/Downloads/Mipmaps_1.pdf
    // Boxfilter Mipmap Generation
    for (size_t level = 1; level < levelCount; level++) {

        uint16_t desiredWidth = initalWidth >> level;
        uint16_t desiredHeight = initalHeight >> level;
        uint16_t prevWidth = initalWidth >> (level - 1);

        std::vector<uint8_t> data(static_cast<size_t>(desiredWidth) * desiredHeight * 4);

        std::vector<uint16_t> height(desiredHeight);
        std::iota(height.begin(), height.end(), 0);

#ifdef GRAD_AFF_USE_CPP17_PARALLELISM
        std::for_each(std::execution::par_unseq, height.begin(), height.end(), [&](size_t y) {
#else
#if defined GRAD_AFF_USE_OPENMP
        #pragma omp parallel for
#endif
        for (size_t i = 0; i < height.size(); i++) {
            auto y = height[i];
#endif
            size_t y0 = static_cast<size_t>(y) << 1;
            size_t y1 = std::min(y0 + 1, std::max(static_cast<size_t>(1), static_cast<size_t>(initalHeight >> (level - 1)) - 1));
            for (size_t x = 0; x < desiredWidth; x++) {
                size_t x0 = x << 1;
                size_t x1 = std::min(x0 + 1, static_cast<size_t>(1, initalWidth >> (level - 1)) - 1);

                for (size_t i = 0; i < 4; i++) {
                    data[x * 4 + static_cast<size_t>(y) * 4 * desiredWidth + i] = static_cast<uint8_t>((1.0f / 4.0f) * (
                        prevData[x0 * 4 + y0 * 4 * prevWidth + i] +
                        prevData[x1 * 4 + y0 * 4 * prevWidth + i] +
                        prevData[x0 * 4 + y1 * 4 * prevWidth + i] +
                        prevData[x1 * 4 + y1 * 4 * prevWidth + i]
                        ));
                }
            }
        }
#ifdef GRAD_AFF_USE_CPP17_PARALLELISM
        );
#endif

        Mipmap mipmap;
        mipmap.width = desiredWidth;
        mipmap.height = desiredHeight;
        mipmap.data = data;

        mipmaps.push_back(mipmap);

        prevData = data;

    }

    // Calculate average color
    for (size_t i = 0; i < mipmaps[0].data.size(); i += 4) {
        averageRed += mipmaps[0].data[i];
        averageGreen += mipmaps[0].data[i + 1];
        averageBlue += mipmaps[0].data[i + 2];
        averageAlpha += mipmaps[0].data[i + 3];
    }

    auto pixelCount = mipmaps[0].width * mipmaps[0].height;

    averageRed /= pixelCount;
    averageGreen /= pixelCount;
    averageBlue /= pixelCount;
    averageAlpha /= pixelCount;

    taggs.clear();

    // Write average Color Tagg
    Tagg taggAvg;
    taggAvg.signature = "GGATCGVA";
    taggAvg.data.push_back(static_cast<uint8_t>(averageRed));
    taggAvg.data.push_back(static_cast<uint8_t>(averageGreen));
    taggAvg.data.push_back(static_cast<uint8_t>(averageBlue));
    taggAvg.data.push_back(static_cast<uint8_t>(averageAlpha));
    taggs.push_back(taggAvg);

    Tagg taggMax;
    taggMax.signature = "GGATCXAM";
    for (int i = 0; i < 4; i++) {
        taggMax.data.push_back(0xFF);
    }
    taggs.push_back(taggMax);

    // Write Transparency Flag Tagg
    if (averageAlpha != 255) {
        hasTransparency = true;
        Tagg taggFlag;
        taggFlag.signature = "GGATGALF";
        taggFlag.data.push_back(0x01);
        for (int i = 0; i < 3; i++) {
            taggFlag.data.push_back(0xFF);
        }
        taggs.push_back(taggFlag);
    }
    else {
        hasTransparency = false;
    }
}

// C Implementation
grad::aff::Paa::Paa* grad::aff::Paa::PaaCreate() {
    try {
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return new grad::aff::Paa::Paa();
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

grad::aff::Paa::Paa* grad::aff::Paa::PaaCreateFromData(uint16_t width, uint16_t height, uint8_t* data, size_t size) {
    try
    {
        if (width == 0 || height == 0 || size == 0 || data == nullptr) {
            core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
            return nullptr;
        }

        auto paa = new grad::aff::Paa::Paa();
        Mipmap mipmap;
        mipmap.width = width;
        mipmap.height = height;
        mipmap.data = std::vector<uint8_t>(data, data + size);
        paa->setMipMap(mipmap, 0);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return paa;
    }
    catch (std::exception& ex)
    {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

bool grad::aff::Paa::PaaDestroy(Paa* paa) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        delete paa;
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    } catch (std::exception &ex) {
        grad::aff::core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaReadFile(Paa* paa, const char* filename, bool lazy) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        if (lazy) {
            paa->readLazy(filename);
        }
        else {
            paa->read(filename);
        }
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    } catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaReadData(Paa* paa, const uint8_t* data, size_t size, bool lazy) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        std::vector<uint8_t> dataVec(data, data + size);
        if (lazy) {
            paa->readLazy(dataVec);
        }
        else {
            paa->read(dataVec);
        }
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    } catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaWriteFile(Paa* paa, const char* filename) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        paa->write(filename);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    } catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

uint8_t* grad::aff::Paa::PaaWriteData(Paa* paa, size_t* size) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return nullptr;
    }
    try {
        std::vector<uint8_t> out;
        paa->write(out);

        uint8_t* outData = new uint8_t[out.size()];
        std::memcpy(outData, out.data(), out.size());
        *size = out.size();
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return outData;
    } catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

bool grad::aff::Paa::PaaDestroyWrittenData(uint8_t* data) {
    if (!core::ExceptionHelper::ValidHandle(data)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        delete[] data;
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

uint32_t grad::aff::Paa::PaaGetTypeOfPax(Paa* paa) {
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return (uint32_t)paa->typeOfPax;
}

bool grad::aff::Paa::PaaSetMipmaps(Paa* paa, Mipmap** mipmaps, uint8_t size) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        std::vector<Mipmap> newMipmaps(size);
        for (uint8_t i = 0; i < size; i++) {
            newMipmaps[i] = *mipmaps[i];
        }

        paa->setMipMaps(newMipmaps);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    } catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaGetMipmaps(Paa* paa, Mipmap** mipmaps, uint8_t size) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        auto srcMipmaps = paa->getMipMaps();

        if (mipmaps && size >= srcMipmaps.size()) {
            for (uint8_t i = 0; i < srcMipmaps.size(); i++) {
                auto mipmapPtr = MipmapCreate();
                *mipmapPtr = srcMipmaps[i];
                mipmaps[i] = mipmapPtr;
            }
            core::ExceptionHelper::SetLastError(core::AFFError::OK);
            return true;
        }
        else {
            core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
            return false;
        }
    } catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaCalcMipmapsAndTaggs(Paa* paa) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        paa->calculateMipmapsAndTaggs();
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

size_t grad::aff::Paa::PaaGetMipmapCount(Paa* paa) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
        return 0;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return paa->mipmaps.size();
}

bool grad::aff::Paa::PaaSetMipmap(Paa* paa, Mipmap* mipmap, uint8_t level) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        paa->setMipMap(*mipmap, level);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

grad::aff::Paa::Mipmap* grad::aff::Paa::PaaGetMipmap(Paa* paa, uint8_t level) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return nullptr;
    }
    try {
        if (level >= paa->mipmaps.size() - 1) {
            core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
            return nullptr;
        }
        else {
            auto mipmap = paa->getMipMap(level);

            auto mipmapPtr = MipmapCreate();
            *mipmapPtr = mipmap;
            core::ExceptionHelper::SetLastError(core::AFFError::OK);
            return mipmapPtr;
        }
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

uint8_t grad::aff::Paa::PaaGetOptimalMipMapIndex(Paa* paa, uint16_t cx) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return 0xFF;
    }
    try {
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return paa->getOptimalMipMapIndex(cx);
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return 0xFF;
    }
}

grad::aff::Paa::Mipmap* grad::aff::Paa::PaaGetOptimalMipMap(Paa* paa, uint16_t cx) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return nullptr;
    }
    try {
        auto mipmapIndex = paa->getOptimalMipMapIndex(cx);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return PaaGetMipmap(paa, mipmapIndex);
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}


size_t grad::aff::Paa::PaaGetTaggCount(Paa* paa) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return 0;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return paa->taggs.size();
}

bool grad::aff::Paa::PaaSetTaggs(Paa* paa, Tagg** taggs, size_t size) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        std::vector<Tagg> newTaggs(size);
        for (size_t i = 0; i < size; i++) {
            newTaggs[i] = *taggs[i];
        }
        paa->setTaggs(newTaggs);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaGetTaggs(Paa* paa, Tagg** taggs, size_t size) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        auto srcTaggs = paa->getTaggs();

        if (taggs && size >= srcTaggs.size()) {
            for (size_t i = 0; i < srcTaggs.size(); i++) {
                auto taggPtr = TaggCreate();
                *taggPtr = srcTaggs[i];
                taggs[i] = taggPtr;
            }
            core::ExceptionHelper::SetLastError(core::AFFError::OK);
            return true;
        }
        else {
            core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
            return false;
        }
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

grad::aff::Paa::Tagg* grad::aff::Paa::PaaGetTagg(Paa* paa, const char* signature, size_t size) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return nullptr;
    }
    try {
        std::string sig(signature, size);

        auto tagg = paa->getTagg(sig);
        auto taggPtr = TaggCreate();
        *taggPtr = tagg;
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return taggPtr;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return nullptr;
    }
}

size_t grad::aff::Paa::PaaGetPixelDataCount(Paa* paa, uint8_t level) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return 0;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return paa->getPixelData(level).size();
}

bool grad::aff::Paa::PaaSetPixelData(Paa* paa, uint8_t* data, size_t size, uint8_t level) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        paa->setPixelData(std::vector<uint8_t>(data, data + size), level);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaGetPixelData(Paa* paa, uint8_t* data, size_t size, uint8_t level) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        auto pixelData = paa->getPixelData(level);
        if (size < pixelData.size()) {
            core::ExceptionHelper::SetLastError(core::AFFError::ArgumentError);
            return false;
        }
        else {
            std::memcpy(data, pixelData.data(), pixelData.size());
            core::ExceptionHelper::SetLastError(core::AFFError::OK);
            return true;
        }
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaSetPixel(Paa* paa, size_t x, size_t y, uint8_t level, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        paa->setPixel(x, y, { r, g, b, a }, level);
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaGetPixel(Paa* paa, size_t x, size_t y, uint8_t level, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    try {
        auto pixel = paa->getPixel(x, y, level);
        *r = pixel[0];
        *g = pixel[1];
        *b = pixel[2];
        *a = pixel[3];
        core::ExceptionHelper::SetLastError(core::AFFError::OK);
        return true;
    }
    catch (std::exception& ex) {
        core::ExceptionHelper::SetLastError(ex);
        return false;
    }
}

bool grad::aff::Paa::PaaIsTransparent(Paa* paa) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return paa->isTransparent();
}

bool grad::aff::Paa::PaaIsValid(Paa* paa) {
    if (!core::ExceptionHelper::ValidHandle(paa)) {
        core::ExceptionHelper::SetLastError(core::AFFError::InvalidHandle);
        return false;
    }
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return paa->isValid();
}

bool grad::aff::Paa::PaaIsPowerOfTwo(uint32_t x) {
    core::ExceptionHelper::SetLastError(core::AFFError::OK);
    return grad::aff::Paa::Paa::isPowerOfTwo(x);
}

