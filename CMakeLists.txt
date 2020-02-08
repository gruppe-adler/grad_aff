# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("grad_aff")

include(ExternalProject)
option(USE_STATIC_LINKING "Static Build" ON)
option(BUILD_TESTS "Build Tests" ON)
option(BUILD_WITH_OIIO "Build with OpenImageIO" ON)
option(BUILD_WITH_OPENSSL "Build with OpenSSL" ON)

if(BUILD_WITH_OIIO)
    add_definitions(-DGRAD_AFF_USE_OIIO)
endif()

if(BUILD_WITH_OPENSSL)
    add_definitions(-DGRAD_AFF_USE_OPENSSL)
endif()

if(USE_STATIC_LINKING)
    add_definitions(-DOIIO_STATIC_DEFINE)
    set(LIB_TYPE STATIC)
else()
    set(LIB_TYPE SHARED)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if((${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND NOT MINGW))
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
if(BUILD_WITH_OIIO)
    if(MINGW)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(OIIO_PKG REQUIRED IMPORTED_TARGET OpenImageIO)

        if(USE_STATIC_LINKING)
            pkg_check_modules(OPENEXR_PKG REQUIRED IMPORTED_TARGET OpenEXR)
            pkg_check_modules(ZLIB_PKG REQUIRED IMPORTED_TARGET zlib)
            pkg_check_modules(TIFF_PKG REQUIRED IMPORTED_TARGET libtiff-4)
            pkg_check_modules(JPEG_PKG REQUIRED IMPORTED_TARGET libjpeg)
            pkg_check_modules(PNG_PKG REQUIRED IMPORTED_TARGET libpng)
        endif()
    else()
        find_package(OpenImageIO CONFIG REQUIRED)
    endif()
endif()
# Parsing
find_package(pegtl CONFIG REQUIRED)

if(BUILD_WITH_OPENSSL)
    # SHA1 PBO
    find_package(OpenSSL REQUIRED)
endif()

if(USE_STATIC_LINKING)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)
endif()

find_package(Boost 1.70.0 COMPONENTS)

find_package(tsl-ordered-map CONFIG REQUIRED)

include_directories(${LZO2_INCLUDE_DIR})
include_directories(${SQUISH_INCLUDE_DIR})
if(BUILD_WITH_OIIO)
    if(MINGW)
        include_directories(${OIIO_PKG_INCLUDE_DIRS}/..)

        if(USE_STATIC_LINKING)
            link_directories(${OIIO_PKG_STATIC_LIBRARY_DIRS})
        endif()
    else()
        include_directories(${OIIO_INCLUDE_DIR})
    endif()
endif()
include_directories(${Boost_INCLUDE_DIRS}) 
include_directories("include")

add_library (grad_aff ${LIB_TYPE} "src/grad_aff.cpp" "src/paa/Paa.cpp" "src/StreamUtil.cpp" "src/GridBlockTree.cpp" "src/a3lzo.cpp" "src/pbo/Pbo.cpp" "src/rap/rap.cpp" "src/rap/RapParser.cpp" "src/wrp/wrp.cpp")
target_compile_definitions (grad_aff PUBLIC GRAD_AFF_EXPORTS)
if(MSVC)
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
endif()

target_include_directories(grad_aff PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(grad_aff PUBLIC ${OPENIMAGEIO_INCLUDE_DIR})
if(BUILD_WITH_OIIO)
    if(MINGW)
        if(USE_STATIC_LINKING)
            target_link_libraries(grad_aff PUBLIC ${OIIO_PKG_STATIC_LIBRARIES} ${OIIO_PKG_STATIC_LIBRARIES}_Util)
            target_link_libraries(grad_aff PUBLIC PkgConfig::OPENEXR_PKG)
            target_link_libraries(grad_aff PUBLIC PkgConfig::ZLIB_PKG)
            target_link_libraries(grad_aff PUBLIC PkgConfig::TIFF_PKG)
            target_link_libraries(grad_aff PUBLIC PkgConfig::JPEG_PKG)
            target_link_libraries(grad_aff PUBLIC PkgConfig::PNG_PKG)
            target_link_libraries(grad_aff PUBLIC ws2_32)
        else()
            target_link_libraries(grad_aff PUBLIC PkgConfig::OIIO_PKG)
        endif()
    else()
        target_link_libraries(grad_aff PUBLIC OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util)
    endif()
endif()
target_link_libraries(grad_aff PUBLIC ${LZO2_LIBRARY})
target_link_libraries(grad_aff PUBLIC ${SQUISH_LIBRARY})
target_link_libraries(grad_aff PUBLIC ${Boost_LIBRARIES})
target_link_libraries(grad_aff PUBLIC taocpp::pegtl)
target_link_libraries(grad_aff PUBLIC tsl::ordered_map)

if(BUILD_WITH_OPENSSL)
    target_link_libraries(grad_aff PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()

if(MINGW)
    target_link_libraries(grad_aff PUBLIC stdc++fs)
endif()

if(BUILD_TESTS)
    # Include sub-projects.
    add_subdirectory ("tests")
endif()

# TOOD Install Target