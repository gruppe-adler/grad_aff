#pragma once

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "grad_aff/paa/paa.h"


TEST_CASE("read uneven png", "[read-uneven-png-write-paa]") {
    grad_aff::Paa test_paa_obj("DXT1_LZO_Test.paa");
    REQUIRE_NOTHROW(test_paa_obj.readPaa());
    test_paa_obj.calculateMipmapsAndTaggs();
    test_paa_obj.writePaa("DXT1_LZO_Test_gil.paa");
}

TEST_CASE("empty paa read", "[empty-paa-read]") {
    grad_aff::Paa test_paa_obj("");
    REQUIRE_THROWS_WITH(test_paa_obj.readPaa(), "Invalid file/magic number");
}
#ifdef GRAD_AFF_USE_OIIO
TEST_CASE("read uneven png2", "[read-uneven-png-write-paa-2]") {
    grad_aff::Paa test_paa_obj;
    REQUIRE_NOTHROW(test_paa_obj.readImage("uneven_test.png"));
    test_paa_obj.writePaa("uneven_test.paa", grad_aff::Paa::TypeOfPaX::DXT1);
}


TEST_CASE("read DXT1 LZO", "[read-dxt1-lzo]") {
    grad_aff::Paa test_paa_obj("Bundle_Test.paa");
    REQUIRE_NOTHROW(test_paa_obj.readPaa());
    REQUIRE_NOTHROW(test_paa_obj.writeImage("tile.png"));
}

TEST_CASE("read DXT1 LZO and write png, bmp,jpg", "[read-dxt1-lzo-write]") {
    grad_aff::Paa test_paa_obj("DXT1_LZO_Test.paa");
    REQUIRE_NOTHROW(test_paa_obj.readPaa());
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
