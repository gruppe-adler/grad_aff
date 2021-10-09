#pragma once

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "grad_aff/pbo/Pbo.h"

TEST_CASE("lzss pbo read", "[lzss-read-pbo]") {
    grad_aff::Pbo testPbo("tembelan.pbo");
    REQUIRE_NOTHROW(testPbo.readPbo(false));
    //testPbo.extractPbo("out");
}

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

TEST_CASE("Test has entry", "[has-entry]") {
    grad_aff::Pbo mehPbo("A3.pbo");
    REQUIRE_NOTHROW(mehPbo.readPbo());
    REQUIRE_FALSE(mehPbo.hasEntry("1337"));
    REQUIRE_FALSE(mehPbo.hasEntry("config.cpp"));
    REQUIRE(mehPbo.hasEntry("config.bin"));
    REQUIRE(mehPbo.hasEntry("data\\env_co.paa"));
    REQUIRE_FALSE(mehPbo.hasEntry("data\\test.paa"));
}


#ifdef GRAD_AFF_USE_OPENSSL
TEST_CASE("Hash Test", "[hash-test]") {
    grad_aff::Pbo mehPbo("map_altis_data_layers_00_01.pbo");
    REQUIRE_NOTHROW(mehPbo.readPbo());
    REQUIRE(mehPbo.checkHash());
}
#endif