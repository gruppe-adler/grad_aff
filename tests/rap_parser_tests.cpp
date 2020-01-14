#pragma once

#include <catch2/catch.hpp>

#include "grad_aff/rap/RapParser.h"

#include <fstream>

TEST_CASE("parser simple class", "[parse-simple-class]") {
    std::string name;
    pegtl::file_input in("test.cpp");
    REQUIRE_NOTHROW(pegtl::parse< grad_aff::RapParser::topLevel, grad_aff::RapParser::action >(in, name));
}
