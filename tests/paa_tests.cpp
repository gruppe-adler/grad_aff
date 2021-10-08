
//#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_RUNNER

#include <catch2/catch.hpp>

#include "paa/Paa.h"
#include "paa/Mipmap.h"



#include <filesystem>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

#include <boost/gil.hpp>

#ifdef _WIN32
#include <boost/gil/extension/io/png.hpp> // fix for gcc sometime
#endif

#ifdef _WIN32
#include <Windows.h>
#include <wrl/client.h>
#include <Shlwapi.h>
#include "core/devices/IStreamDevice.hpp"
#endif

static const std::filesystem::path testdataPath("../../../../tests/data/paa/");
static const std::filesystem::path testdataOutPath = testdataPath / "out";
static const std::filesystem::path AdlerDXT1Path = testdataPath / "AdlerDXT1.paa";
static const std::filesystem::path AdlerDXT1_2k = testdataPath / "DXT1_LZO_Test.paa";
static const std::filesystem::path AdlerDXT5Path = testdataPath / "AdlerDXT5.paa";
static const std::filesystem::path DXT128x2048Path = testdataPath / "128x2048DXT1.paa";

static const std::array<uint8_t, 4> whitePixel = { 0xFF, 0xFF, 0xFF, 0xFF };
static const std::array<uint8_t, 4> blackPixel = { 0x00, 0x00, 0x02, 0xFF };

using namespace grad::aff::core;
using namespace grad::aff::Paa;

namespace bg = boost::gil;
namespace fs = std::filesystem;


int main(int argc, char* argv[]) {

    return Catch::Session().run(argc, argv);
}

TEST_CASE("AI88 Test") {
    Paa paa;
    paa.read(testdataPath / "add_gs.paa");
}

#pragma region C API

TEST_CASE("PAA C Test Wasm Shit") {

    int dim = 32;

    std::vector<uint8_t> data(32 * 32 * 4);

    for (size_t i = 0; i < 32*32*4; i+=4)
    {
        data[i] = 0xFF;
        data[i + 1] = 0;
        data[i + 2] = 0;
        data[i + 3] = 0xFF;
    }

    Paa* paa = PaaCreateFromData(dim, dim, data.data(), 32 * 32 * 4);

    size_t size = 0;
    auto dataOut = PaaWriteData(paa, &size);

    std::ofstream out("testwasm.paa", std::ios::out | std::ios::binary);
    out.write(reinterpret_cast<char*>(dataOut), size);
    out.close();
}

TEST_CASE("Tagg C API") {
    auto tagg = TaggCreate();

    std::string testSig("TestSignature");
    TaggSetSignature(tagg, testSig.c_str(), testSig.size());

    uint8_t testData[] = { 0x01, 0x03, 0x03, 0x07 };
    TaggSetData(tagg, testData, 4);

    auto taggClone = TaggClone(tagg);

    testSig = "NewSignature";
    testData[2] = 0x05;

    auto sigSize = TaggGetSignatureSize(tagg);
    char* sigOut = new char[sigSize];
    TaggGetSignature(tagg, sigOut, sigSize);

    auto dataSize = TaggGetDataSize(tagg);
    auto dataOut = new uint8_t[dataSize];
    TaggGetData(tagg, dataOut, dataSize);

    REQUIRE(std::string(sigOut) == "TestSignature");
    REQUIRE(dataOut[2] == 0x03);
    REQUIRE(dataOut[3] == 0x07);

    TaggSetSignature(taggClone, "", 0);
    REQUIRE(tagg->signature == "TestSignature");
    REQUIRE(taggClone->signature == "");

    char* sigOut2 = new char[sigSize];
    REQUIRE(TaggGetSignature(tagg, sigOut, 0) == static_cast<uint32_t>(AFFError::ArgumentError));

    auto dataOut2 = new uint8_t[dataSize];
    TaggGetData(tagg, dataOut2, 0);
    delete[] dataOut2;

    delete[] sigOut;
    delete[] dataOut;
    TaggDestroy(tagg);
    TaggDestroy(taggClone);
}

TEST_CASE("Mipmap C API") {
    auto mipmap = MipmapCreate();

    MipmapSetWidth(mipmap, 512);
    MipmapSetHeight(mipmap, 256);
    //MipmapSetLzoCompressed(mipmap, true);

    uint8_t testData[] = { 0x01, 0x03, 0x03, 0x07 };
    MipmapSetData(mipmap, testData, 4);
    testData[2] = 0x05;

    REQUIRE(MipmapGetWidth(mipmap) == 512);
    REQUIRE(MipmapGetHeight(mipmap) == 256);
    //REQUIRE(MipmapIsLzoCompressed(mipmap));
    
    auto dataSize = MipmapGetDataSize(mipmap);
    auto dataOut = new uint8_t[dataSize];
    MipmapGetData(mipmap, dataOut, dataSize);
    REQUIRE(dataOut[2] == 0x03);
    REQUIRE(dataOut[3] == 0x07);

    uint8_t* dataNull = nullptr;
    MipmapGetData(mipmap, dataNull, 0);
    REQUIRE(dataNull == nullptr);
    dataNull = new uint8_t[1];
    MipmapGetData(mipmap, dataNull, 0);
    //REQUIRE(dataNull == nullptr); TODO: error
    delete[] dataNull;

    auto clonedMipmap = MipmapClone(mipmap);
    MipmapSetWidth(mipmap, 1337);
    REQUIRE(MipmapGetWidth(clonedMipmap) == 512);

    delete[] dataOut;
    MipmapDestroy(mipmap);
    MipmapDestroy(clonedMipmap);
}

TEST_CASE("Read DXT1 C API") {
    auto paa = PaaCreate();
    PaaReadFile(paa, AdlerDXT1Path.string().c_str(), false);
    auto mipmapCount = PaaGetMipmapCount(paa);

    Mipmap** mipmaps = new Mipmap*[mipmapCount];
    PaaGetMipmaps(paa, mipmaps, mipmapCount);
    REQUIRE(mipmaps[0]->width == 512);
    PaaSetMipmaps(paa, mipmaps, mipmapCount);
    PaaCalcMipmapsAndTaggs(paa);
    PaaWriteFile(paa, (testdataOutPath / "AdlerDXT1_C_out.paa").string().c_str());

    for (size_t i = 0; i < mipmapCount; i++) {
        MipmapDestroy(mipmaps[i]);
    }
    delete[] mipmaps;
    PaaDestroy(paa);
}

TEST_CASE("Read DXT1 C API Lazy") {
    auto paa = PaaCreate();
    PaaReadFile(paa, AdlerDXT1Path.string().c_str(), true);
    auto mipmapCount = PaaGetMipmapCount(paa);

    Mipmap** mipmaps = new Mipmap*[mipmapCount];
    PaaGetMipmaps(paa, mipmaps, mipmapCount);
    REQUIRE(mipmaps[0]->width == 512);
    PaaSetMipmaps(paa, mipmaps, mipmapCount);
    PaaCalcMipmapsAndTaggs(paa);
    PaaWriteFile(paa, (testdataOutPath / "AdlerDXT1_C_out_lazy.paa").string().c_str());

    for (size_t i = 0; i < mipmapCount; i++) {
        MipmapDestroy(mipmaps[i]);
    }
    delete[] mipmaps;
    PaaDestroy(paa);
}

TEST_CASE("Read/Write DXT1 C API Data") {
    std::ifstream file(AdlerDXT1Path.string(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    file.close();

    auto paaData = PaaCreate();
    PaaReadData(paaData, buffer.data(), buffer.size(), false);

    PaaCalcMipmapsAndTaggs(paaData);

    PaaDestroy(paaData);
}

TEST_CASE("Read DXT1 C API Data Lazy") {
    std::ifstream file(AdlerDXT1Path.string(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    file.close();

    auto paaData = PaaCreate();
    PaaReadData(paaData, buffer.data(), buffer.size(), true);

    PaaGetMipmap(paaData, 0);

    size_t outDataSize = 0;
    auto outData = PaaWriteData(paaData, &outDataSize);

    PaaDestroy(paaData);

    std::ofstream outFile(testdataOutPath / "AdlerDXT1_mem_out_c.paa", std::ios::binary);
    outFile.write(reinterpret_cast<char*>(outData), outDataSize);
    outFile.close();
    PaaDestroyWrittenData(outData);
}


TEST_CASE("Read/Write General C API") {

    REQUIRE(PaaIsPowerOfTwo(512));
    REQUIRE_FALSE(PaaIsPowerOfTwo(1337));

    auto paa = PaaCreate();
    PaaReadFile(paa, AdlerDXT1Path.string().c_str(), true);
    
    REQUIRE(PaaGetTypeOfPax(paa) == static_cast<uint32_t>(Paa::TypeOfPaX::DXT1));
    REQUIRE_FALSE(PaaIsTransparent(paa));
    REQUIRE(PaaIsValid(paa));
    REQUIRE(PaaGetOptimalMipMapIndex(paa, 256) == 1);
    auto optMipmap = PaaGetOptimalMipMap(paa, 137);
    REQUIRE(optMipmap->width == 128);
    MipmapDestroy(optMipmap);

    Mipmap* nullMipmaps = nullptr;
    size_t nullSize = 0;
    PaaGetMipmaps(paa, &nullMipmaps, nullSize);
    REQUIRE(nullMipmaps == nullptr);

    REQUIRE(PaaGetMipmap(paa, 42) == nullptr);

    auto mipmap0 = PaaGetMipmap(paa, 0);
    
    PaaSetMipmaps(paa, nullptr, 0);
    PaaSetMipmap(paa, mipmap0, 0);

    PaaSetPixel(paa, 0, 0, 0, 255, 0, 0, 255);

    uint8_t r = 0, g = 0, b = 0, a = 0;
    PaaGetPixel(paa, 0, 0, 0, &r, &g, &b, &a);
    REQUIRE(r == 255);
    REQUIRE(g == 0);
    REQUIRE(b == 0);
    REQUIRE(a == 255);

    auto pixelDataCount = PaaGetPixelDataCount(paa, 0);
    auto pixelData = new uint8_t[pixelDataCount];
    PaaGetPixelData(paa, pixelData, pixelDataCount, 0);
    PaaSetPixelData(paa, pixelData, pixelDataCount, 0);
    delete[] pixelData;

    auto taggCount = PaaGetTaggCount(paa);
    auto taggs = new Tagg*[taggCount];
    PaaGetTaggs(paa, taggs, taggCount);
    PaaSetTaggs(paa, taggs, taggCount);
    delete[] taggs;

    std::string sig("GGATCGVA");
    auto sigTagg = PaaGetTagg(paa, sig.data(), sig.size());
    REQUIRE(sigTagg->data[3] == 255);
    TaggDestroy(sigTagg);

    PaaWriteFile(paa, (testdataOutPath / "AdlerDXT1_out_c_redpixel.paa").string().c_str());

    MipmapDestroy(mipmap0);
    PaaDestroy(paa);
}

TEST_CASE("Test Special C API") {
    auto paa = PaaCreate();
    PaaReadFile(paa, AdlerDXT5Path.string().c_str(), false);

    REQUIRE(PaaIsTransparent(paa));

    uint8_t* data = nullptr;
    PaaGetPixelData(paa, data, 0, 0);
    //REQUIRE(data == nullptr);

    Tagg* tagg = nullptr;
    PaaGetTaggs(paa, &tagg, 0);
    REQUIRE(tagg == nullptr);

    PaaDestroy(paa);

    auto paa2 = PaaCreate();
    auto newMipmap = MipmapCreate();
    PaaSetMipmap(paa2, newMipmap, 1);
    MipmapDestroy(newMipmap);
    PaaDestroy(paa2);
}

#pragma endregion


#pragma region Special

TEST_CASE("Unsupported Paa") {
    Paa paa;
    REQUIRE_THROWS_AS(paa.read(testdataPath / "UnsupportedPaa.paa"), std::runtime_error);
}

TEST_CASE("Invalid Paa") {
    Paa paa;
    REQUIRE_THROWS_AS(paa.read(testdataPath / "InvalidPaa.paa"), std::runtime_error);
}

TEST_CASE("Read non existent file") {
    Paa paa;
    REQUIRE_THROWS_AS(paa.read("Bundle_Test_not_found_1337.paa"), std::runtime_error);
    REQUIRE_THROWS_AS(paa.read(std::string("Bundle_Test_not_found_1337.paa")), std::runtime_error);
    REQUIRE_THROWS_AS(paa.read(std::wstring(L"Bundle_Test_not_found_1337.paa")), std::runtime_error);
    REQUIRE_THROWS_AS(paa.readLazy("Bundle_Test_not_found_1337.paa"), std::runtime_error);
    REQUIRE_THROWS_AS(paa.readLazy(std::string("Bundle_Test_not_found_1337.paa")), std::runtime_error);
    REQUIRE_THROWS_AS(paa.readLazy(std::wstring(L"Bundle_Test_not_found_1337.paa")), std::runtime_error);
    REQUIRE_THROWS_AS(paa.getMipMap(3), std::runtime_error);
}

TEST_CASE("Operations on empty paa") {
    Paa paa;
    REQUIRE_THROWS_AS(paa.calculateMipmapsAndTaggs(), std::runtime_error);
    REQUIRE_THROWS_AS(paa.getMipMaps(), std::runtime_error);
    REQUIRE_THROWS_AS(paa.getMipMap(42), std::runtime_error);
    REQUIRE_THROWS_AS(paa.setPixel(0, 0, {}, 42), std::runtime_error);
    REQUIRE_THROWS_AS(paa.getPixelData(42), std::runtime_error);
    REQUIRE_THROWS_AS(paa.getTagg(""), std::runtime_error);
    REQUIRE_THROWS_AS(paa.getOptimalMipMapIndex(0), std::runtime_error);

    Mipmap mipmap;
    mipmap.height = 1337;
    paa.setMipMap(mipmap, 0);
    REQUIRE_THROWS_AS(paa.calculateMipmapsAndTaggs(), std::runtime_error);
}


TEST_CASE("Optimal index min") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path));
    REQUIRE(paa.getOptimalMipMapIndex(0) == 7);
}

#pragma endregion


#pragma region Read

void checkPaaAdlerDXT1(Paa& paa) {
    
    auto mipMaps = paa.getMipMaps();
    auto mipMapsSize = mipMaps.size();

    REQUIRE(paa.typeOfPax == Paa::TypeOfPaX::DXT1);
    REQUIRE(!paa.isTransparent());
    REQUIRE(paa.isValid());

    REQUIRE(paa.getTaggs().size() == 3);

    REQUIRE(mipMapsSize == 8);

    REQUIRE(mipMaps[0].width == 512);
    REQUIRE(mipMaps[0].height == 512);
    REQUIRE(mipMaps[0].data.size() == 512 * 512 * 4);

    REQUIRE(mipMaps[3].width == 64);
    REQUIRE(mipMaps[3].height == 64);
    REQUIRE(mipMaps[3].data.size() == 64 * 64 * 4);

    REQUIRE(mipMaps[mipMapsSize - 1].width == 4);
    REQUIRE(mipMaps[mipMapsSize - 1].height == 4);
    REQUIRE(mipMaps[mipMapsSize - 1].data.size() == 4 * 4 * 4);

    REQUIRE(paa.getPixel(0, 0, 0) == whitePixel);
    REQUIRE(paa.getPixel(160, 250, 0) == blackPixel);
    REQUIRE(paa.getPixel(511, 511, 0) == whitePixel);
    REQUIRE_THROWS_AS(paa.getPixel(512, 512, 0), std::runtime_error);

    REQUIRE(paa.getPixelData(0).size() == 512 * 512 * 4);
    REQUIRE(paa.getPixelData(3).size() == 64 * 64 * 4);
    REQUIRE(paa.getPixelData(mipMapsSize - 1).size() == 4 * 4 * 4);

    REQUIRE(paa.getOptimalMipMapIndex(256) == 1);
    REQUIRE(paa.getOptimalMipMapIndex(1337) == 0);
    REQUIRE(paa.getOptimalMipMap(256).data.size() == 256 * 256 * 4);
}

void checkPaaAdlerDXT5(Paa& paa) {

    auto mipMaps = paa.getMipMaps();
    auto mipMapsSize = mipMaps.size();

    REQUIRE(paa.typeOfPax == Paa::TypeOfPaX::DXT5);
    REQUIRE(paa.isTransparent());
    REQUIRE(paa.isValid());

    std::string flagTagg("FLAGTAGG");
    std::reverse(flagTagg.begin(), flagTagg.end());
    auto tagg = paa.getTagg(flagTagg);
    REQUIRE(tagg.data.size() == 4);
    REQUIRE(tagg.data[0] == 0x01);

    REQUIRE(paa.getTaggs().size() == 4);

    REQUIRE(mipMapsSize == 8);

    REQUIRE(mipMaps[0].width == 512);
    REQUIRE(mipMaps[0].height == 512);
    REQUIRE(mipMaps[0].data.size() == 512 * 512 * 4);

    REQUIRE(mipMaps[3].width == 64);
    REQUIRE(mipMaps[3].height == 64);
    REQUIRE(mipMaps[3].data.size() == 64 * 64 * 4);

    REQUIRE(mipMaps[mipMapsSize - 1].width == 4);
    REQUIRE(mipMaps[mipMapsSize - 1].height == 4);
    REQUIRE(mipMaps[mipMapsSize - 1].data.size() == 4 * 4 * 4);

    REQUIRE(paa.getPixel(0, 0, 0)[3] == 0);
    REQUIRE(paa.getPixel(160, 250, 0) == blackPixel);
    REQUIRE(paa.getPixel(511, 511, 0)[3] == 0);
    REQUIRE_THROWS_AS(paa.getPixel(512, 512, 0), std::runtime_error);

    REQUIRE(paa.getPixelData(0).size() == 512 * 512 * 4);
    REQUIRE(paa.getPixelData(3).size() == 64 * 64 * 4);
    REQUIRE(paa.getPixelData(mipMapsSize - 1).size() == 4 * 4 * 4);

    REQUIRE(paa.getOptimalMipMapIndex(256) == 1);
    REQUIRE(paa.getOptimalMipMapIndex(1337) == 0);
    REQUIRE(paa.getOptimalMipMap(256).data.size() == 256 * 256 * 4);
    //REQUIRE();
}

TEST_CASE("Read MipmapsData DXT1 special") {
    Paa paa;
    REQUIRE_NOTHROW(paa.readLazy(AdlerDXT1Path));
    REQUIRE_NOTHROW(paa.getMipMaps());
}

TEST_CASE("Read MipmapsData DXT5 special") {
    Paa paa;
    REQUIRE_NOTHROW(paa.readLazy(AdlerDXT5Path));
    REQUIRE_NOTHROW(paa.getMipMaps());
}

TEST_CASE("READ LAZY DX1 512x512") {
    Paa paa;
    //REQUIRE_NOTHROW(paa.readLazy(AdlerDXT1Path));
    REQUIRE_NOTHROW(paa.readLazy(AdlerDXT1Path));
    auto mipmap0 = paa.getMipMap(0);
    REQUIRE(mipmap0.data.size() == mipmap0.height * mipmap0.width * 4);

    auto mipmap3 = paa.getMipMap(3);
    REQUIRE(mipmap3.data.size() == mipmap3.height * mipmap3.width * 4);

    auto mipmapLast = paa.getMipMap(paa.mipmaps.size() - 1);
    REQUIRE(mipmapLast.data.size() == mipmapLast.height * mipmapLast.width * 4);
}

TEST_CASE("Read Lazy DX1 Memory 512x512") {
    Paa paa;

    std::ifstream file(AdlerDXT1Path.string(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    REQUIRE_NOTHROW(paa.readLazy(buffer));
    paa.getMipMaps();
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT1 Path 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1Path));
    checkPaaAdlerDXT1(paa);
    //paa.
}

TEST_CASE("READ DXT5 Path 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path));
    checkPaaAdlerDXT5(paa);
    //paa.
}

TEST_CASE("READ DXT1 String 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1Path.string()));
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT5 String 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path.string()));
    checkPaaAdlerDXT5(paa);
}

TEST_CASE("READ DXT1 C String 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1Path.string().c_str()));
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT5 C String 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path.string().c_str()));
    checkPaaAdlerDXT5(paa);
}

TEST_CASE("READ DXT1 WString 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1Path.wstring()));
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT5 WString 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path.wstring()));
    checkPaaAdlerDXT5(paa);
}

TEST_CASE("READ DXT1 WString Lazy 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.readLazy(AdlerDXT1Path.wstring()));
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT5 WString Lazy 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.readLazy(AdlerDXT5Path.wstring()));
    checkPaaAdlerDXT5(paa);
}

TEST_CASE("READ DXT1 Memory 512x512") {
    Paa paa;

    std::ifstream file(AdlerDXT1Path.string(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    REQUIRE_NOTHROW(paa.read(buffer));
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT5 Memory 512x512") {
    Paa paa;

    std::ifstream file(AdlerDXT5Path.string(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    REQUIRE_NOTHROW(paa.read(buffer));
    checkPaaAdlerDXT5(paa);
}

TEST_CASE("READ DXT1 Memory uint8 512x512") {
    Paa paa;

    std::ifstream file(AdlerDXT1Path.string(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    REQUIRE_NOTHROW(paa.read(buffer));
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT5 Memory uint8 512x512") {
    Paa paa;

    std::ifstream file(AdlerDXT5Path.string(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    REQUIRE_NOTHROW(paa.read(buffer));
    checkPaaAdlerDXT5(paa);
}

#ifdef _WIN32

TEST_CASE("READ DXT1 I-Stream 512x512") {
    IStream* fileStream;
    auto hr = SHCreateStreamOnFileEx(devices::IStreamDevice::toWstring(AdlerDXT1Path.string()).c_str(), STGM_READWRITE, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &fileStream);
    REQUIRE(hr == S_OK);

    Paa paa;
    REQUIRE_NOTHROW(paa.read(fileStream));
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT5 I-Stream 512x512") {

    IStream* fileStream;
    auto hr = SHCreateStreamOnFileEx(devices::IStreamDevice::toWstring(AdlerDXT5Path.string()).c_str(), STGM_READWRITE, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &fileStream);
    REQUIRE(hr == S_OK);

    Paa paa;
    REQUIRE_NOTHROW(paa.read(fileStream));
    checkPaaAdlerDXT5(paa);
}

TEST_CASE("READ DXT1 I-Stream Lazy 512x512") {
    IStream* fileStream;
    auto hr = SHCreateStreamOnFileEx(devices::IStreamDevice::toWstring(AdlerDXT1Path.string()).c_str(), STGM_READWRITE, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &fileStream);
    REQUIRE(hr == S_OK);

    Paa paa;
    REQUIRE_NOTHROW(paa.readLazy(fileStream));
    paa.getMipMaps();
    checkPaaAdlerDXT1(paa);
}

TEST_CASE("READ DXT5 I-Stream  Lazy 512x512") {

    IStream* fileStream;
    auto hr = SHCreateStreamOnFileEx(devices::IStreamDevice::toWstring(AdlerDXT5Path.string()).c_str(), STGM_READWRITE, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &fileStream);
    REQUIRE(hr == S_OK);

    Paa paa;
    REQUIRE_NOTHROW(paa.readLazy(fileStream));
    paa.getMipMaps();
    checkPaaAdlerDXT5(paa);
}

#endif



#pragma endregion


#pragma region Write

TEST_CASE("Write Memory") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1Path));
    paa.calculateMipmapsAndTaggs();

    std::vector<uint8_t> outBuffer;
    paa.write(outBuffer);

    std::ofstream file(testdataOutPath / "AdlerDXT1_mem_out.paa", std::ios::binary | std::ios::ate);
    file.write(reinterpret_cast<char*>(outBuffer.data()), outBuffer.size());
    file.close();
}

TEST_CASE("READ WRITE 128x2048") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(DXT128x2048Path));
    paa.taggs.pop_back();

    auto mipmap = paa.getMipMap(0);
    paa.setMipMaps({});
    paa.setTaggs({});
    paa.setMipMap(mipmap, 0);

    paa.write(testdataOutPath / "DXT128x2048.paa");
}

TEST_CASE("WRITE DXT1 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1Path));
    paa.taggs.pop_back();

    auto mipmap = paa.getMipMap(0);
    paa.setMipMaps({});
    paa.setTaggs({});
    paa.setMipMap(mipmap, 0);
    paa.setPixel(0, 0, { 255, 0, 0, 255 }, 0);
    paa.setPixel(511, 511, { 255, 0, 0, 255 }, 0);

    paa.write(testdataOutPath / "AdlerDXT1SetRedPixel.paa");

}

TEST_CASE("WRITE DXT5 512x512") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path));
    paa.taggs.pop_back();

    auto mipmap = paa.getMipMap(0);
    paa.setMipMaps({});
    paa.setTaggs({});
    paa.setMipMap(mipmap, 0);
    paa.setPixel(0, 0, { 255, 0, 0, 255 }, 0);
    paa.setPixel(511, 511, { 255, 0, 0, 255 }, 0);

    paa.write(testdataOutPath / "AdlerDXT5SetRedPixel.paa");

}

TEST_CASE("WRITE DXT1 2kx2k") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1_2k));
    paa.taggs.pop_back();

    auto mipmap = paa.getMipMap(0);
    paa.setMipMaps({});
    paa.setTaggs({});
    paa.setMipMap(mipmap, 0);
    paa.write(testdataOutPath / "DXT1_2k.paa");
}

TEST_CASE("WRITE DXT1 512x512 SET PIXELS") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1Path));
    paa.taggs.pop_back();

    auto mipmap = paa.getMipMap(0);
    paa.setMipMaps({});
    paa.setTaggs({});
    paa.setMipMap(mipmap, 0);
    paa.setPixelData(mipmap.data, 0);

    paa.write(testdataOutPath / "AdlerDXT1SetPixel.paa");

}

TEST_CASE("WRITE DXT5 512x512 SET PIXELS") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path));
    paa.taggs.pop_back();

    auto mipmap = paa.getMipMap(0);
    paa.setMipMaps({});
    paa.setTaggs({});
    paa.setMipMap(mipmap, 0);
    paa.setPixelData(mipmap.data, 0);

    paa.write(testdataOutPath / "AdlerDXT5SetPixel.paa");

}

TEST_CASE("WRITE DXT1 512x512 Unknown Type") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT1Path));
    paa.typeOfPax = Paa::TypeOfPaX::UNKNOWN;

    paa.write(testdataOutPath / "AdlerDXT1AutoType.paa");

}

TEST_CASE("WRITE DXT5 512x512 Unknown Type") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path));
    paa.typeOfPax = Paa::TypeOfPaX::UNKNOWN;

    paa.write(testdataOutPath / "AdlerDXT5AutoType.paa");

}

TEST_CASE("Write wstring") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path));

    paa.write((testdataOutPath / "AdlerDXT5WString.paa").wstring());
}

#ifdef _WIN32

TEST_CASE("Write IStream") {
    Paa paa;
    REQUIRE_NOTHROW(paa.read(AdlerDXT5Path));

    fs::path file = testdataOutPath / "AdlerDXT5IStreamOut.paa";

    if (fs::exists(file)) {
        fs::remove(file);
    }

    IStream* istream;
    REQUIRE(
        SHCreateStreamOnFileEx(
            file.wstring().c_str(),
            STGM_READWRITE | STGM_DIRECT_SWMR, 
            FILE_ATTRIBUTE_NORMAL, true, NULL, &istream) 
        == S_OK);

    paa.write(istream);
    istream->Release();
}

#endif

TEST_CASE("Write to same file") {

    fs::path sameFile(testdataOutPath / "AdlerDXT5WriteSameFile.paa");
    fs::copy_file(AdlerDXT5Path, sameFile, fs::copy_options::overwrite_existing);

    Paa paa;
    REQUIRE_NOTHROW(paa.read(sameFile));
    paa.calculateMipmapsAndTaggs();
    paa.write();
}

#pragma endregion

#ifdef _WIN32
#pragma region Visual/PNG Test

TEST_CASE("Write DXT1 as PNG") {
    Paa paa;
    REQUIRE_NOTHROW(paa.readLazy(AdlerDXT1Path));
    auto mipmap = paa.getMipMap(0);
    auto view = bg::interleaved_view(mipmap.width, mipmap.height, reinterpret_cast<bg::rgba8_pixel_t*>(mipmap.data.data()), static_cast<size_t>(mipmap.width) * 4);
    
    auto outPath = (testdataOutPath / "AdlerDXT1.png").string();
    bg::write_view(outPath, view, bg::png_tag());

}

TEST_CASE("Write DXT5 as PNG") {
    Paa paa;
    REQUIRE_NOTHROW(paa.readLazy(AdlerDXT5Path));
    auto mipmap = paa.getMipMap(0);
    auto view = bg::interleaved_view(mipmap.width, mipmap.height, reinterpret_cast<bg::rgba8_pixel_t*>(mipmap.data.data()), static_cast<size_t>(mipmap.width) * 4);

    auto outPath = (testdataOutPath / "AdlerDXT5.png").string();
    bg::write_view(outPath, view, bg::png_tag());

}
#pragma endregion
#endif

/*
TEST_CASE("test 2048x128", "[read-write-2048x128]") {
    grad_aff::Paa test_paa_obj;
    test_paa_obj.readPaa("128x2048.paa");
    test_paa_obj.calculateMipmapsAndTaggs();
    test_paa_obj.writePaa("128x2048_out.paa");
}

TEST_CASE("test set raw pixel at", "[read-write-raw-pixel-set]") {
    grad_aff::Paa test_paa_obj;
    test_paa_obj.readPaa("Bundle_Test.paa");
    test_paa_obj.setRawPixelDataAt(0, 0, {255, 192, 203, 255});
    test_paa_obj.writePaa("setRawPixelDataAt_out.paa");
}

TEST_CASE("read paa from buffer", "[read-paa-from-buffer]") {
    std::ifstream ifs("Bundle_Test.paa", std::ios::binary);
    std::vector<uint8_t> data;
    CHECK(!ifs.eof());
    CHECK(!ifs.fail());

    ifs.seekg(0, std::ios_base::end);
    std::streampos fileSize = ifs.tellg();
    data.resize(fileSize);

    ifs.seekg(0, std::ios_base::beg);
    ifs.read(reinterpret_cast<char*>(&data[0]), fileSize);

    grad_aff::Paa test_paa_obj;
    test_paa_obj.readPaa(data);
    test_paa_obj.writePaa("Bundle_Test_from_buffer_out.paa");

}

TEST_CASE("write buffer to file", "[read-write-buffer]") {
    grad_aff::Paa test_paa_obj;
    test_paa_obj.readPaa("Bundle_Test.paa");
    test_paa_obj.setRawPixelDataAt(0, 0, { 255, 192, 203, 255 });
    std::ofstream out("Bundle_Test_buffer_out.paa", std::ios::out | std::ios::binary);
    auto buffer = test_paa_obj.writePaa();
    out.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
    out.close();
}

TEST_CASE("read/write dxt1 lzo", "[read-write-dxt1-lzo]") {
    //grad_aff::Paa test_paa_obj("grad_dlc_low_messenger_medic_co.paa");
    grad_aff::Paa test_paa_obj;
    //grad_aff::Paa test_paa_obj("de_v2.paa");
    REQUIRE_NOTHROW(test_paa_obj.readPaa("DXT1_LZO_Test.paa"));
    test_paa_obj.calculateMipmapsAndTaggs();
    test_paa_obj.writePaa("DXT1_LZO_Test_out.paa");// , grad_aff::Paa::TypeOfPaX::DXT5);
    //test_paa_obj.writePaa("de_v2_2.paa");
    //test_paa_obj.writePaa("grad_dlc_low_messenger_medic_co.paa");
    //grad_aff::Paa test_paa_obj_2("de_v2_2.paa");
    //REQUIRE_NOTHROW(test_paa_obj_2.readPaa());
}

TEST_CASE("read/write DXT5", "[read-write-dxt5]") {
    grad_aff::Paa test_paa_obj;
    REQUIRE_NOTHROW(test_paa_obj.readPaa("Bundle_Test.paa"));
    test_paa_obj.writePaa("Bundle_Text_out.paa");
    grad_aff::Paa test_paa_obj_2;
    REQUIRE_NOTHROW(test_paa_obj_2.readPaa("Bundle_Text_out.paa"));
}

TEST_CASE("empty paa read", "[empty-paa-read]") {
    grad_aff::Paa test_paa_obj;
    REQUIRE_THROWS_WITH(test_paa_obj.readPaa(""), "Invalid file/magic number");
}
#ifdef GRAD_AFF_USE_OIIO
TEST_CASE("read uneven png2", "[read-uneven-png-write-paa-2]") {
    grad_aff::Paa test_paa_obj;
    REQUIRE_NOTHROW(test_paa_obj.readImage("uneven_test.png"));
    test_paa_obj.writePaa("uneven_test.paa", grad_aff::Paa::TypeOfPaX::DXT1);
}


TEST_CASE("read DXT1 LZO", "[read-dxt1-lzo]") {
    grad_aff::Paa test_paa_obj;
    REQUIRE_NOTHROW(test_paa_obj.readPaa("Bundle_Test.paa"));
    REQUIRE_NOTHROW(test_paa_obj.writeImage("tile.png"));
}

TEST_CASE("read DXT1 LZO and write png, bmp,jpg", "[read-dxt1-lzo-write]") {
    grad_aff::Paa test_paa_obj;
    REQUIRE_NOTHROW(test_paa_obj.readPaa("DXT1_LZO_Test.paa"));
    REQUIRE_NOTHROW(test_paa_obj.writeImage("DXT1_LZO_Test.png", 0));
    REQUIRE_NOTHROW(test_paa_obj.writeImage("DXT1_LZO_Test.jpg", 0));
    REQUIRE_NOTHROW(test_paa_obj.writeImage("DXT1_LZO_Test.bmp", 0));
}

TEST_CASE("read png, write paa", "[read-png-write-paa]") {
    grad_aff::Paa test_paa_obj;
    REQUIRE_NOTHROW(test_paa_obj.readImage("Bundle_Test.png"));
    test_paa_obj.writePaa("Bundle_Test_out.paa", grad_aff::Paa::TypeOfPaX::DXT5);
}


TEST_CASE("read bmp, write paa", "[read-bmp-write-paa]") {
    grad_aff::Paa test_paa_obj;
    REQUIRE_NOTHROW(test_paa_obj.readImage("BigTest.bmp"));
    test_paa_obj.writePaa("BigTest_out.paa", grad_aff::Paa::TypeOfPaX::DXT1);
}
#endif
*/

