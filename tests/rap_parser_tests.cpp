#pragma once

#include <catch2/catch.hpp>


#include <tao/pegtl/contrib/parse_tree.hpp>

#include "grad_aff/rap/ClassEntry.h"
#include "grad_aff/rap/RapParser.h"


#include <fstream>

TEST_CASE("parser simple class", "[parse-simple-class]") {
    std::string name;
    //rap.readConfig("test.cpp");

    auto sharedClass = std::make_shared<RapClass>();
    sharedClass->name = "Lol";

    pegtl::file_input in("test.cpp");
    pegtl::parse< grad_aff::RapParser2::topLevel, grad_aff::RapParser2::action >(in, sharedClass);
}
