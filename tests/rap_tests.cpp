#pragma once

#include <catch2/catch.hpp>

#include "grad_aff/rap/rap.h"

#include <fstream>
#include <vector>

TEST_CASE("parse simple config", "[parse-simple-config]") {
    grad_aff::Rap test_rap_obj;
    REQUIRE_NOTHROW(test_rap_obj.parseConfig("test.cpp"));
}

TEST_CASE("read simple config", "[read-simple-config]") {
    grad_aff::Rap test_rap_obj("configTest.bin");
    REQUIRE_NOTHROW(test_rap_obj.readRap());
}

TEST_CASE("read binarized rvmat", "[read-bin-rvmat]") {
    grad_aff::Rap test_rap_obj("P_000-000_L00.rvmat");
    REQUIRE_NOTHROW(test_rap_obj.readRap());
}