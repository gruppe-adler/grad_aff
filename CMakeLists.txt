# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("grad_aff")

include(ExternalProject)
option(USE_STATIC_LINKING "Static Build" ON)

if(USE_STATIC_LINKING)
    add_definitions(-DOIIO_STATIC_DEFINE)
    set(LIB_TYPE STATIC)
else()
    # TODO
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(LIB_TYPE SHARED)
endif()

set(CMAKE_CXX_STANDARD 17)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(DEBUG_SUFFIX "d")
else()
    set(DEBUG_SUFFIX "")
endif()

# LZO
FIND_PATH(LZO2_INCLUDE_DIR lzo/lzoconf.h)
FIND_LIBRARY(LZO2_LIBRARY NAMES lzo2 liblzo2)

# DXT
find_path(SQUISH_INCLUDE_DIR squish.h)
find_library(SQUISH_LIBRARY squish${DEBUG_SUFFIX})

# Image
find_package(OpenImageIO CONFIG REQUIRED)

# Parsing
find_package(pegtl CONFIG REQUIRED)

# SHA1 PBO
find_package(OpenSSL REQUIRED)

if(USE_STATIC_LINKING)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)
endif()

find_package(Boost 1.71.0 COMPONENTS system filesystem thread iostreams) 

find_package(tsl-ordered-map CONFIG REQUIRED)

include_directories(${LZO2_INCLUDE_DIR})
include_directories(${SQUISH_INCLUDE_DIR})
include_directories(${OIIO_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS}) 
include_directories("include")

add_library (grad_aff ${LIB_TYPE} "src/grad_aff.cpp" "src/paa/Paa.cpp" "src/StreamUtil.cpp" "src/GridBlockTree.cpp" "src/a3lzo.cpp" "src/pbo/Pbo.cpp" "src/rap/rap.cpp" "src/rap/RapParser.cpp" "src/wrp/wrp.cpp")

if(USE_STATIC_LINKING)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        target_compile_options(grad_aff PUBLIC "/MTd")
    else()
        target_compile_options(grad_aff PUBLIC "/MT")
    endif()
else()
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        target_compile_options(grad_aff PUBLIC "/MDd")
    else()
        target_compile_options(grad_aff PUBLIC "/MD")
    endif()
endif()

target_include_directories(grad_aff PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(grad_aff PUBLIC ${OPENIMAGEIO_INCLUDE_DIR})

target_link_libraries(grad_aff PUBLIC OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util)
target_link_libraries(grad_aff PUBLIC ${LZO2_LIBRARY})
target_link_libraries(grad_aff PUBLIC ${SQUISH_LIBRARY})
target_link_libraries(grad_aff PUBLIC ${Boost_LIBRARIES})
target_link_libraries(grad_aff PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(grad_aff PUBLIC taocpp::pegtl)
target_link_libraries(grad_aff PUBLIC tsl::ordered_map)

# Include sub-projects.
add_subdirectory ("tests")

# TOOD dyn build
add_custom_command(TARGET grad_aff POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_LINKER_FILE:grad_aff> ${PROJECT_SOURCE_DIR}/lib/grad_aff.lib
)
