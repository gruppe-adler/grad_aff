#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "grad_aff/p3d/odol.h"


TEST_CASE("read test", "[read-test]") {
    //grad_aff::Odol test_odol_obj("Bridge_01_F.p3d");
    //grad_aff::Odol test_odol_obj("UWreck_Mv22_F.p3d");
    //grad_aff::Odol test_odol_obj("HistoricalPlaneWreck_02_wing_left_F.p3d");
    grad_aff::Odol test_odol_obj("invisibleRoadway_square_F.p3d");
    //grad_aff::Odol test_odol_obj("Track_01_bumper_F.p3d");
    //grad_aff::Odol test_odol_obj("aif_fence_chmelnice.p3d");
    //grad_aff::Odol test_odol_obj("TentHangar_V1_F.p3d");
    //grad_aff::Odol test_odol_obj("t_stub_picea.p3d");
    //grad_aff::Odol test_odol_obj("aif_zluty_statek_in.p3d");
    //grad_aff::Odol test_odol_obj("palm_04.p3d");
    //grad_aff::Odol test_odol_obj("dum_mesto3_istan.p3d");
    REQUIRE_NOTHROW(test_odol_obj.readOdol());
}

TEST_CASE("read test mv22", "[read-test-mv22]") {
    grad_aff::Odol test_odol_obj("UWreck_Mv22_F.p3d");
    REQUIRE_NOTHROW(test_odol_obj.readOdol());
}


TEST_CASE("read test bridge", "[read-test-bridge]") {
    grad_aff::Odol test_odol_obj("Bridge_01_F.p3d");
    REQUIRE_NOTHROW(test_odol_obj.readOdol());
}

TEST_CASE("read test planewreck", "[read-test-planewreck]") {
    grad_aff::Odol test_odol_obj("HistoricalPlaneWreck_02_wing_left_F.p3d");
    REQUIRE_NOTHROW(test_odol_obj.readOdol());
}



TEST_CASE("read odol chapel", "[read-odol-chapel]") {
    grad_aff::Odol test_odol_obj("Chapel_V2_F.p3d");
    REQUIRE_NOTHROW(test_odol_obj.readOdol());
}

TEST_CASE("peek odol chapel", "[peek-odol-chapel]") {
    grad_aff::Odol test_odol_obj("Chapel_V2_F.p3d");
    REQUIRE_NOTHROW(test_odol_obj.peekLodTypes());
    auto s = test_odol_obj.readLod(6);

}