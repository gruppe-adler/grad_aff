#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "grad_aff/rap/rap.h"
#include "grad_aff/StreamUtil.h"

#include <fstream>
#include <vector>

TEST_CASE("lzss test", "[lzss-test]") {
    grad_aff::Rap test_rap_obj;
    //auto ifs = std::make_shared<std::ifstream>("Tembelan.wrp", std::ios::binary);
   /* auto ifs = std::make_shared<std::ifstream>("RoadsLibLzss.cfg", std::ios::binary);

    std::vector<uint8_t> out;
    grad_aff::readLzssFile(*ifs, out);

    std::ofstream fout("data.dat", std::ios::out | std::ios::binary);
    fout.write(reinterpret_cast<char*>(out.data()), out.size());
    fout.close();*/

    test_rap_obj.parseConfig("RoadsLibLzss.cfg");
    grad_aff::Rap test_rap_obj2("LzssRvmat.rvmat");
    test_rap_obj2.readRap();
    std::stringstream outTest;
    test_rap_obj2.classEntries[0]->write(outTest, "    ");
}

TEST_CASE("parse enoch roadslib", "[parse-enoch-roadslib]") {
    grad_aff::Rap test_rap_obj;
    test_rap_obj.parseConfig("roadslib_enoch.cfg");
}


TEST_CASE("parse with array", "[parse-array-simple-config]") {
    grad_aff::Rap test_rap_obj;
    test_rap_obj.parseConfig("roadslibArray.cfg");
}

TEST_CASE("parse simple config", "[parse-simple-config]") {
    grad_aff::Rap test_rap_obj;
    //REQUIRE_NOTHROW(test_rap_obj.parseConfig("test.cpp"));
}

TEST_CASE("read simple config", "[read-simple-config]") {
    grad_aff::Rap test_rap_obj("configTest.bin");
    REQUIRE_NOTHROW(test_rap_obj.readRap());
}

TEST_CASE("read binarized rvmat", "[read-bin-rvmat]") {
    grad_aff::Rap test_rap_obj("P_000-000_L00.rvmat");
    REQUIRE_NOTHROW(test_rap_obj.readRap());
}