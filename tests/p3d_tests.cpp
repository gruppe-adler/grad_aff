#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "grad_aff/p3d/odol.h"


TEST_CASE("read test", "[read-test]") {
    //grad_aff::Odol test_odol_obj("Bridge_01_F.p3d");
    //grad_aff::Odol test_odol_obj("UWreck_Mv22_F.p3d");
    //grad_aff::Odol test_odol_obj("HistoricalPlaneWreck_02_wing_left_F.p3d");
    //grad_aff::Odol test_odol_obj("invisibleRoadway_square_F.p3d");
    //grad_aff::Odol test_odol_obj("Crash_barrier_F.p3d");
    //grad_aff::Odol test_odol_obj("Track_01_bumper_F.p3d");
    //grad_aff::Odol test_odol_obj("aif_fence_chmelnice.p3d");
    //grad_aff::Odol test_odol_obj("TentHangar_V1_F.p3d");
    //grad_aff::Odol test_odol_obj("t_stub_picea.p3d");
    //grad_aff::Odol test_odol_obj("aif_zluty_statek_in.p3d");
    //grad_aff::Odol test_odol_obj("palm_04.p3d");
    //grad_aff::Odol test_odol_obj("dum_mesto3_istan.p3d");
    grad_aff::Odol test_odol_obj("Bridge.p3d");
    REQUIRE_NOTHROW(test_odol_obj.readOdol());
}

TEST_CASE("legacy", "[legacy]") {
    grad_aff::Odol test1("Bridge_01_F.p3d");
    REQUIRE_NOTHROW(test1.readOdol());

    grad_aff::Odol test2("UWreck_Mv22_F.p3d");
    REQUIRE_NOTHROW(test2.readOdol());

    grad_aff::Odol test3("HistoricalPlaneWreck_02_wing_left_F.p3d");
    REQUIRE_NOTHROW(test3.readOdol());

    grad_aff::Odol test4("invisibleRoadway_square_F.p3d");
    REQUIRE_NOTHROW(test4.readOdol());

    grad_aff::Odol test5("Crash_barrier_F.p3d");
    REQUIRE_NOTHROW(test5.readOdol());

    grad_aff::Odol test6("Track_01_bumper_F.p3d");
    REQUIRE_NOTHROW(test6.readOdol());

    grad_aff::Odol test7("aif_fence_chmelnice.p3d");
    REQUIRE_NOTHROW(test7.readOdol());

    grad_aff::Odol test8("TentHangar_V1_F.p3d");
    REQUIRE_NOTHROW(test8.readOdol());

    grad_aff::Odol test9("t_stub_picea.p3d");
    REQUIRE_NOTHROW(test9.readOdol());

    grad_aff::Odol test10("aif_zluty_statek_in.p3d");
    REQUIRE_NOTHROW(test10.readOdol());

    grad_aff::Odol test11("palm_04.p3d");
    REQUIRE_NOTHROW(test11.readOdol());


    grad_aff::Odol test12("farmwall.p3d");
    REQUIRE_NOTHROW(test12.readOdol());


    grad_aff::Odol test13("marketstand1.p3d");
    REQUIRE_NOTHROW(test13.readOdol());
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