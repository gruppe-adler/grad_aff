#include "grad_aff/paa/Paa.h"

#include <squish.h>
#include <lzo/lzo1x.h>

#ifndef GRAD_AFF_LITE_BUILD
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/string_view.h>

using namespace OIIO;
#endif

grad_aff::Paa::Paa() {
    this->typeOfPax = TypeOfPaX::DXT5;
};


grad_aff::Paa::Paa(std::string pboFilename) {
    this->is = std::make_shared<std::ifstream>(pboFilename, std::ios::binary);
};

grad_aff::Paa::Paa(std::vector<uint8_t> data) {
    this->is = std::make_shared<std::stringstream>(std::string(data.begin(), data.end()));
}

void grad_aff::Paa::readPaa() {

    //std::ifstream ifs(filename, std::ios::binary);

    magicNumber = readBytes<uint16_t>(*is);
    switch (magicNumber)
    {
    case 0xff01:
        this->typeOfPax = TypeOfPaX::DXT1;
        break;
    case 0xff02:
        this->typeOfPax = TypeOfPaX::DXT2;
        break;
    case 0xff03:
        this->typeOfPax = TypeOfPaX::DXT3;
        break;
    case 0xff04:
        this->typeOfPax = TypeOfPaX::DXT4;
        break;
    case 0xff05:
        this->typeOfPax = TypeOfPaX::DXT5;
        break;
    case 0x4444:
        this->typeOfPax = TypeOfPaX::RGBA4444;
        break;
    case 0x1555:
        this->typeOfPax = TypeOfPaX::RGBA5551;
        break;
    case 0x8888:
        this->typeOfPax = TypeOfPaX::RGBA8888;
        break;
    case 0x8080:
        this->typeOfPax = TypeOfPaX::GRAYwAlpha;
        break;
    default:
        throw std::runtime_error("Invalid file/magic number");
        break;
    }

    // Taggs
    while (is->peek() != 0)
    {
        Tagg tagg;
        tagg.signature = readString(*is, 8);
        tagg.dataLength = readBytes<uint32_t>(*is);
        tagg.data = readBytes(*is, tagg.dataLength);
        taggs.push_back(tagg);
    }

    // TODO
    palette.dataLength = readBytes<uint16_t>(*is);
    if (palette.dataLength > 0) {
        palette.data = readBytes(*is, palette.dataLength);
    }
    
    // MipMaps
    while (peekBytes<uint16_t>(*is) != 0) {
        MipMap mipmap;
        mipmap.width = readBytes<uint16_t>(*is);
        mipmap.height = readBytes<uint16_t>(*is);
        mipmap.dataLength = readBytesAsArmaUShort(*is);
        mipmap.data = readBytes(*is, mipmap.dataLength);

        // check if top most bit is set, which indicates lzo compression for DXT files
        if ((mipmap.width & 0x8000) != 0) {
            // correct width
            mipmap.width &= 0x7FFF;
            mipmap.lzoCompressed = true;
        }
        else {
            mipmap.lzoCompressed = false;
        }

        if (mipmap.lzoCompressed) {
            if (lzo_init() == LZO_E_OK) {
                auto lzoUncompressed = std::vector<uint8_t>((size_t)mipmap.dataLength * 4 * 100); // TODO
                lzo_uint out_len;

                if (lzo1x_decompress(reinterpret_cast<const uint8_t*>(mipmap.data.data()), mipmap.dataLength, lzoUncompressed.data(), &out_len, NULL) != LZO_E_OK) {
                    throw std::runtime_error("LZO Decompression failed");
                };

                mipmap.data = std::vector<uint8_t>(lzoUncompressed.data(), lzoUncompressed.data() + out_len);
                mipmap.dataLength = out_len;
                mipmap.data.resize(mipmap.dataLength);
            }
            else {
                throw std::runtime_error("LZO Init failed!");
            }
        }

        // decompress
        if (typeOfPax == TypeOfPaX::DXT1) {
            // DXT1 compression ratio in this case is for whatever reason 8:1
            size_t uncompressedSize = (size_t)mipmap.dataLength * 8;
            auto uncompressedData = std::vector<squish::u8>(uncompressedSize);

            squish::DecompressImage(uncompressedData.data(), mipmap.height, mipmap.height, mipmap.data.data(), squish::kDxt1);

            mipmap.dataLength = uncompressedSize;
            mipmap.data = std::vector<uint8_t>(uncompressedData.data(), uncompressedData.data() + uncompressedSize);
        }
        else if (typeOfPax == TypeOfPaX::DXT5) {
            // DXT5 compression ratio is 4:1
            size_t uncompressedSize = (size_t)mipmap.dataLength * 4;
            auto uncompressedData = std::vector<squish::u8>(uncompressedSize);

            squish::DecompressImage(uncompressedData.data(), (int)mipmap.width, (int)mipmap.height, mipmap.data.data(), squish::kDxt5);

            mipmap.dataLength = uncompressedSize;
            mipmap.data = std::vector<uint8_t>(uncompressedData.data(), uncompressedData.data() + uncompressedSize);
        }
        // TODO: other pax

        mipMaps.push_back(mipmap);
    }
}

#ifndef GRAD_AFF_LITE_BUILD

void grad_aff::Paa::readImage(fs::path filename) {
    auto inImage = ImageBuf(filename.string());
    auto curWidth = inImage.spec().width;
    auto curHeight = inImage.spec().height;
    
    mipMaps.clear();
    size_t counter = 1;

    for (int level = 0; (curHeight < curWidth ? curHeight : curHeight) > 2; level++) {
        MipMap mipmap;
        mipmap.width = inImage.spec().width;
        mipmap.height = inImage.spec().height;

        size_t pixelSize = (size_t)mipmap.width * mipmap.height;
        mipmap.data.resize(pixelSize * 4);

        for (size_t i = 0; i < pixelSize; i++) {
            size_t x = i % inImage.spec().width;
            size_t y = i / inImage.spec().height;

            std::vector<float_t> pixelData(inImage.nchannels());
            inImage.getpixel(x, y, pixelData.data());

            mipmap.data[i * 4] = (uint8_t)(pixelData[0] * 255);
            mipmap.data[i * 4 + 1] = (uint8_t)(pixelData[1] * 255);
            mipmap.data[i * 4 + 2] = (uint8_t)(pixelData[2] * 255);
            mipmap.data[i * 4 + 3] = (uint8_t)(pixelData[3] * 255);

            if (level == 0) {
                averageRed += (uint8_t)(pixelData[0] * 255);
                averageGreen += (uint8_t)(pixelData[1] * 255);
                averageBlue += (uint8_t)(pixelData[2] * 255);
                averageAlpha += (uint8_t)(pixelData[3] * 255);
                counter++;
            }
        }

        mipmap.dataLength = mipmap.data.size();
        mipMaps.push_back(mipmap);

        curHeight /= 2;
        curWidth /= 2;
        inImage = ImageBufAlgo::resize(inImage, "", 0, ROI(0, curWidth, 0, curHeight, 0, inImage.nchannels()));
    }

    averageRed /= counter;
    averageGreen /= counter;
    averageBlue /= counter;
    averageAlpha /= counter;

    // Write average Color Tagg
    Tagg taggAvg;
    taggAvg.signature = "GGATCGVA";
    taggAvg.data.push_back(averageRed);
    taggAvg.data.push_back(averageGreen);
    taggAvg.data.push_back(averageBlue);
    taggAvg.data.push_back(averageAlpha);
    taggAvg.dataLength = taggAvg.data.size();
    taggs.push_back(taggAvg);

    Tagg taggMax;
    taggMax.signature = "GGATCXAM";
    for (int i = 0; i < 4; i++)
        taggMax.data.push_back(0xFF);
    taggMax.dataLength = taggMax.data.size();
    taggs.push_back(taggMax);


    // Write Transparency Flag Tagg
    if (inImage.spec().alpha_channel != -1) {
        hasTransparency = true;
        Tagg taggFlag;
        taggFlag.signature = "GGATGALF";
        taggFlag.data.push_back(0x01);
        for (int i = 0; i < 3; i++)
            taggFlag.data.push_back(0xFF);
        taggFlag.dataLength = taggFlag.data.size();
        taggs.push_back(taggFlag);
    }
    
}

void grad_aff::Paa::writeImage(std::string filename, int level) {
    if (level >= mipMaps.size()) {
        std::stringstream exStream;
        exStream << "Level " << level << " exceeds the mipmap count of " << mipMaps.size();
        throw std::out_of_range(exStream.str());
    }

    int width = mipMaps[level].width;
    int height = mipMaps[level].height;

    auto outImage = ImageOutput::create(filename);
    if (!outImage) {
        throw std::runtime_error("Couldn't create output image!");
    }

    ImageSpec imgSpec(width, height, 4, TypeDesc::UINT8);
    outImage->open(filename, imgSpec);
    outImage->write_image(TypeDesc::UINT8, mipMaps[level].data.data());
    outImage->close();
    
}

#endif

void grad_aff::Paa::writePaa(std::string filename, TypeOfPaX typeOfPaX) {

    std::vector<MipMap> encodedMipMaps = mipMaps;

    // Compression
    this->typeOfPax = typeOfPaX;
    if (this->typeOfPax == TypeOfPaX::UNKNOWN) {
        this->typeOfPax = hasTransparency ? TypeOfPaX::DXT5 : TypeOfPaX::DXT1;
    }

    if (typeOfPax == TypeOfPaX::DXT5) {
        for (auto& mipmap : encodedMipMaps) {
            auto compressedDataLength = mipmap.dataLength / 4;
            auto compressedData = std::vector<uint8_t>(compressedDataLength);

            squish::CompressImage(reinterpret_cast<const uint8_t*>(mipmap.data.data()), (int)mipmap.width, (int)mipmap.height, compressedData.data(), squish::kDxt5);

            mipmap.data = compressedData;
            mipmap.dataLength = compressedDataLength;
        }
        magicNumber = 0xff05;
    }
    else if (typeOfPax == TypeOfPaX::DXT1) {
        for (auto& mipmap : encodedMipMaps) {
            auto compressedDataLength = mipmap.dataLength / 8;
            auto compressedData = std::vector<uint8_t>(compressedDataLength);

            squish::CompressImage(reinterpret_cast<const uint8_t*>(mipmap.data.data()), (int)mipmap.width, (int)mipmap.height, compressedData.data(), squish::kDxt1);

            mipmap.data = compressedData;
            mipmap.dataLength = compressedDataLength;
        }
        magicNumber = 0xff01;
    }
    
    // compress with lzo, if needed
    if (encodedMipMaps[0].width > 128) {
        if (lzo_init() == LZO_E_OK) {

            for (int i = 0; i < encodedMipMaps.size() && encodedMipMaps[i].width > 128; i++) {
                encodedMipMaps[i].lzoCompressed = true;

                size_t out_len = 0;
                          
                std::vector<unsigned char> outputData(encodedMipMaps[i].data.size() * 2);
                std::vector<unsigned char> workMemory(LZO1X_MEM_COMPRESS);
                
                if (lzo1x_1_compress(reinterpret_cast<const uint8_t*>(encodedMipMaps[i].data.data()), encodedMipMaps[i].dataLength, outputData.data(), &out_len, workMemory.data()) != LZO_E_OK) {
                    throw std::runtime_error("LZO Compression failed");
                }
                
                encodedMipMaps[i].data = std::vector<uint8_t>(outputData.data(), outputData.data() + out_len);
                encodedMipMaps[i].dataLength = out_len;

                encodedMipMaps[i].width |= 0x8000;
            }
        }
        else {
            throw std::runtime_error("LZO Init failed!");
        }
    }

    Tagg taggOffs;
    taggOffs.signature = "GGATSFFO";


    uint32_t initalOffset = 0;
    initalOffset += 2; // magic

    for (auto tagg : taggs) {
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
        offsetAsChars = std::vector<char>(reinterpret_cast<char*>(&initalOffset), reinterpret_cast<char*>(&initalOffset) +4);

        for (int i = 0; i < 4; i++) {
            taggOffs.data.push_back(offsetAsChars[i]);
        }


        initalOffset += mipmap.dataLength + 2 * 2 + 3;
        counter++;
    }
    taggOffs.dataLength = taggOffs.data.size();
    
    // Write everything
    std::ofstream ofs(filename + ".paa", std::ios::binary);

    // Write magic
    writeBytes<uint16_t>(ofs, magicNumber);
    for (auto& tagg : taggs) {
        writeString(ofs, tagg.signature);
        writeBytes<uint32_t>(ofs, tagg.dataLength);
        writeBytes(ofs, tagg.data);
    }

    // Write offset Tag
    writeString(ofs, taggOffs.signature);
    writeBytes<uint32_t>(ofs, taggOffs.dataLength);
    writeBytes(ofs, taggOffs.data);

    writeBytes<uint16_t>(ofs, palette.dataLength);
    if (palette.dataLength > 0) {
        // TODO:
    }

    for (auto& mipmap : encodedMipMaps) {
        writeBytes<uint16_t>(ofs, mipmap.width);
        writeBytes<uint16_t>(ofs, mipmap.height);
        writeBytesAsArmaUShort(ofs, mipmap.dataLength);
        writeBytes(ofs, mipmap.data);
    }

    writeBytes<uint16_t>(ofs, 0x00);
    writeBytes<uint16_t>(ofs, 0x00);
    writeBytes<uint16_t>(ofs, 0x00);

    ofs.close();
}

std::vector<uint8_t> grad_aff::Paa::getRawPixelData(uint8_t level)
{
    if (this->mipMaps.size() == 0) {
        return {};
    } else {
        return this->mipMaps[level].data;
    }
};


uint8_t grad_aff::Paa::getRawPixelDataAt(size_t x, size_t y, uint8_t level) {
    if (this->mipMaps.size() == 0) {
        return {};
    }
    else {
        return this->mipMaps[level].data[x + y * mipMaps[level].width];
    }
}

void grad_aff::Paa::setRawPixelData(std::vector<uint8_t> data, uint8_t level) {
    this->mipMaps[level].data = data;
}
void grad_aff::Paa::setRawPixelDataAt(size_t x, size_t y, uint8_t pixelData, uint8_t level) {
    this->mipMaps[level].data[x + y * mipMaps[level].width] = pixelData;
}