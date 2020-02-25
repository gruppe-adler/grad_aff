#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "grad_aff/p3d/odol.h"

/*
TEST_CASE("read odol tank", "[read-odol-tank]") {
    grad_aff::Odol test_odol_obj("APC_Tracked_01_aa_F.p3d");
    REQUIRE_NOTHROW(test_odol_obj.readOdol());
}
*/

TEST_CASE("read odol chapel", "[read-odol-chapel]") {
    grad_aff::Odol test_odol_obj("Chapel_V2_F.p3d");
    REQUIRE_NOTHROW(test_odol_obj.readOdol());
}

TEST_CASE("peek odol chapel", "[peek-odol-chapel]") {
    grad_aff::Odol test_odol_obj("Chapel_V2_F.p3d");
    REQUIRE_NOTHROW(test_odol_obj.peekLodTypes());
    auto s = test_odol_obj.readLod(6);

}