#pragma once

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "grad_aff/paa/paa.h"

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
