#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "grad_aff/wrp/wrp.h"

#include <fstream>
#include <vector>
//
//TEST_CASE("read chern winter", "[read-chern-winter]") {
//    grad_aff::Wrp test_wrp_obj("chernarus_winter.wrp");
//    REQUIRE_NOTHROW(test_wrp_obj.readWrp());
//}

TEST_CASE("read chern summer", "[read-chern-summer]") {
    grad_aff::Wrp test_wrp_obj("chernarus_summer.wrp");
    REQUIRE_NOTHROW(test_wrp_obj.readWrp());
}



TEST_CASE("read stratis", "[read-stratis]") {
    grad_aff::Wrp test_wrp_obj("Tembelan.wrp");
    REQUIRE_NOTHROW(test_wrp_obj.readWrp());

    for (auto& s : test_wrp_obj.compressedBytes3) {
        if (s != 0)
            std::cout << "Hit" << std::endl;
    }
}

TEST_CASE("read takis", "[read-takis]") {
    grad_aff::Wrp test_wrp_obj("takistan.wrp");
    REQUIRE_NOTHROW(test_wrp_obj.readWrp());
}

TEST_CASE("empty wrp read", "[empty-wrp--read]") {
    grad_aff::Wrp test_wrp_obj("");
    REQUIRE_THROWS_WITH(test_wrp_obj.readWrp(), "Invalid file!");
}


TEST_CASE("read Altis in memory", "[read-altis-memory]") {
    std::ifstream ifs("stratis.wrp", std::ios::binary);
    std::vector<uint8_t> data;
    CHECK(!ifs.eof());
    CHECK(!ifs.fail());

    ifs.seekg(0, std::ios_base::end);
    std::streampos fileSize = ifs.tellg();
    data.resize(fileSize);

    ifs.seekg(0, std::ios_base::beg);
    ifs.read(reinterpret_cast<char*>(&data[0]), fileSize);
  
    grad_aff::Wrp test_wrp_obj(data);
    test_wrp_obj.readWrp();
    
}
