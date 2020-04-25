
cmake_minimum_required (VERSION 3.8)

find_package(Catch2 CONFIG REQUIRED)
find_package(OpenMP REQUIRED)

include_directories (${tests_SOURCE_DIR}/include)

#add_executable (grad_aff_tests "grad_aff_tests.cpp" "rap_parser_tests.cpp" "pbo_tests.cpp" "paa_tests.cpp" "wrp_tests.cpp")
#add_executable (grad_aff_tests "grad_aff_tests.cpp" "rap_parser_tests.cpp" "pbo_tests.cpp" "paa_tests.cpp")

#add_executable (grad_aff_tests "grad_aff_tests.cpp" "wrp_tests.cpp")
#add_executable (grad_aff_tests "grad_aff_tests.cpp" "pbo_tests.cpp")
add_executable (grad_aff_tests_wrp "wrp_tests.cpp")
#add_executable (grad_aff_tests "grad_aff_tests.cpp" "pbo_tests.cpp")
#add_executable (grad_aff_tests "grad_aff_tests.cpp" "rap_tests.cpp")
add_executable(grad_aff_tests_rap "rap_tests.cpp")
add_executable(grad_aff_tests_paa "paa_tests.cpp")
add_executable(grad_aff_tests_p3d "p3d_tests.cpp")

target_link_libraries(grad_aff_tests_p3d PRIVATE Catch2::Catch2)
target_link_libraries(grad_aff_tests_wrp PRIVATE Catch2::Catch2)
target_link_libraries(grad_aff_tests_paa PRIVATE Catch2::Catch2)
target_link_libraries(grad_aff_tests_rap PRIVATE Catch2::Catch2)

target_link_libraries(grad_aff_tests_wrp PRIVATE grad_aff)
target_link_libraries(grad_aff_tests_p3d PRIVATE grad_aff)
target_link_libraries(grad_aff_tests_paa PRIVATE grad_aff)
target_link_libraries(grad_aff_tests_rap PRIVATE grad_aff)

target_link_libraries(grad_aff_tests_wrp PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(grad_aff_tests_p3d PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(grad_aff_tests_paa PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(grad_aff_tests_rap PUBLIC OpenMP::OpenMP_CXX)
