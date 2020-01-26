#pragma once

#include <catch2/catch.hpp>

#include "grad_aff/pbo/pbo.h"

TEST_CASE("empty pbo read", "[empty-read-pbo]") {
    grad_aff::Pbo testPbo("");
    REQUIRE_THROWS_WITH(testPbo.readPbo(), "Invalid file/magic number");
}

TEST_CASE("extract single file", "[single-extract-pbo]") {
    grad_aff::Pbo testPbo("A3.pbo");
    REQUIRE_NOTHROW(testPbo.readPbo());
    testPbo.extractPbo("unpack");
    
    testPbo.extractSingleFile("data\\env_cloth_neutral_co.paa", "unpack2");
    testPbo.extractSingleFile("data\\env_cloth_neutral_co.paa", "unpack3", false);
}

TEST_CASE("prefix read pbo", "[prefix-read-pbo]") {
    grad_aff::Pbo testPbo("map_altis_data_layers_00_01.pbo");
    REQUIRE(testPbo.getEntryData("a3\\map_altis\\data\\layers\\00_01\\m_003_037_lca.paa").size() > 0);
}

TEST_CASE("meh", "[meh]") {
    grad_aff::Pbo mehPbo("grad_meh_main.pbo");
    REQUIRE_NOTHROW(mehPbo.readPbo());
    REQUIRE_NOTHROW(mehPbo.writePbo(""));
}

#ifdef GRAD_AFF_USE_OPENSSL
TEST_CASE("Hash Test", "[hash-test]") {
    grad_aff::Pbo mehPbo("map_altis_data_layers_00_01.pbo");
    REQUIRE_NOTHROW(mehPbo.readPbo());
    REQUIRE(mehPbo.checkHash());
}
#endif