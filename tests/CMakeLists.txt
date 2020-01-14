
cmake_minimum_required (VERSION 3.8)

find_package(Catch2 CONFIG REQUIRED)

include_directories (${tests_SOURCE_DIR}/include)

#add_executable (grad_aff_tests "grad_aff_tests.cpp" "rap_parser_tests.cpp" "pbo_tests.cpp" "paa_tests.cpp" "wrp_tests.cpp")
add_executable (grad_aff_tests "grad_aff_tests.cpp" "rap_parser_tests.cpp" "pbo_tests.cpp" "paa_tests.cpp")

#add_executable (grad_aff_tests "grad_aff_tests.cpp" "wrp_tests.cpp")
#add_executable (grad_aff_tests "grad_aff_tests.cpp" "pbo_tests.cpp")
#add_executable (grad_aff_tests "grad_aff_tests.cpp" "rap_tests.cpp")
#add_executable (grad_aff_tests "grad_aff_tests.cpp" "rap_parser_tests.cpp")
#add_executable (grad_aff_tests "grad_aff_tests.cpp" "paa_tests.cpp")

target_link_libraries(grad_aff_tests PRIVATE Catch2::Catch2)

target_link_libraries(grad_aff_tests PRIVATE grad_aff)
