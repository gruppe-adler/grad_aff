# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("grad_aff")

include(ExternalProject)
include(CheckIncludeFileCXX)

option(DYNAMIC_LIB "Dynamic Lib" ON)
option(USE_STATIC_LINKING "Static Build" ON)
option(BUILD_TESTS "Build Tests" ON)
option(BUILD_WITH_OPENSSL "Build with OpenSSL" OFF)
option(GRAD_AFF_ENABLE_PARALLELISM "Enable Parallelism" ON)

if(NOT DEFINED CMAKE_BUILD_TYPE OR "${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-DNOMINMAX)

if(BUILD_WITH_OPENSSL)
    add_definitions(-DGRAD_AFF_USE_OPENSSL)
endif()

if(DYNAMIC_LIB)
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if((${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND NOT MINGW))
    set(DEBUG_SUFFIX "d")
else()
    set(DEBUG_SUFFIX "")
endif()

# LZO
find_package(lzokay CONFIG REQUIRED)

# DXT
find_path(SQUISH_INCLUDE_DIR squish.h)
find_library(SQUISH_LIBRARY squish${DEBUG_SUFFIX})

# Image
if(BUILD_WITH_OIIO)
   
   # Let OIIO find external libsquish
    set (LIBSQUISH_INCLUDES ${SQUISH_INCLUDE_DIR})
    set (LIBSQUISH_LIBRARIES ${SQUISH_LIBRARY})

    find_package (OpenEXR CONFIG)

    if (NOT TARGET Libsquish::Libsquish)
        add_library(Libsquish::Libsquish UNKNOWN IMPORTED)
        set_target_properties(Libsquish::Libsquish PROPERTIES
           INTERFACE_INCLUDE_DIRECTORIES "${LIBSQUISH_INCLUDES}")

        set_property(TARGET Libsquish::Libsquish APPEND PROPERTY
            IMPORTED_LOCATION "${LIBSQUISH_LIBRARIES}")
    endif ()

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

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(Boost_USE_DEBUG_LIBS ON)
    set(Boost_USE_RELEASE_LIBS OFF)
    set(Boost_USE_DEBUG_RUNTIME ON)
else()
    set(Boost_USE_DEBUG_LIBS OFF)
    set(Boost_USE_RELEASE_LIBS ON)
    set(Boost_USE_DEBUG_RUNTIME OFF)
endif()

find_package(Boost 1.70.0 COMPONENTS)

find_package(tsl-ordered-map CONFIG REQUIRED)

include_directories(${SQUISH_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories("include")

file(GLOB_RECURSE GRAD_AFF_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE GRAD_AFF_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE GRAD_AFF_HPP ${PROJECT_SOURCE_DIR}/include/*.hpp)
message("Lib Type: " ${LIB_TYPE})
add_library (grad_aff ${LIB_TYPE} ${GRAD_AFF_HPP} ${GRAD_AFF_SOURCES} ${GRAD_AFF_HEADERS} "src/paa/Tagg.cpp"  "include/core/AffCompat.h" "include/core/AffExceptions.h" "src/core/AffExceptions.cpp")# "src/grad_aff.cpp" "src/p3d/odol.cpp" "src/paa/Paa.cpp" "src/StreamUtil.cpp" "src/GridBlockTree.cpp" "src/a3lzo.cpp" "src/pbo/Pbo.cpp" "src/rap/rap.cpp" "src/rap/RapParser.cpp" "src/wrp/wrp.cpp")
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

if(MSVC AND NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_options(grad_aff PUBLIC "/O2")
endif()

if(MSVC)
  #target_compile_options(grad_aff PUBLIC /W4 /WX)
else()
  #target_compile_options(grad_aff PUBLIC -Wall -Wextra -pedantic -Werror)
endif()

target_include_directories(grad_aff PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(grad_aff PUBLIC ${OPENIMAGEIO_INCLUDE_DIR})

target_link_libraries(grad_aff PUBLIC ${SQUISH_LIBRARY})
target_link_libraries(grad_aff PUBLIC ${Boost_LIBRARIES})
target_link_libraries(grad_aff PUBLIC taocpp::pegtl)
target_link_libraries(grad_aff PUBLIC tsl::ordered_map)
target_link_libraries(grad_aff PUBLIC lzokay::lzokay)

if(GRAD_AFF_ENABLE_PARALLELISM)
    # Check for CPP 17 Parallelism
    CHECK_INCLUDE_FILE_CXX(execution GRAD_AFF_CPP17_PAR)

    if(GRAD_AFF_CPP17_PAR)
        message(STATUS "Using C++17 parallelism")
        add_definitions(-DGRAD_AFF_USE_CPP17_PARALLELISM)
        if(MINGW)
            target_link_libraries(grad_aff PUBLIC tbb)
        endif()
    else()
        # Check for OpenMP
        find_package(OpenMP)
        if (OPENMP_FOUND)
            message(STATUS "Using OpenMP for parallelism")
            add_definitions(-DGRAD_AFF_USE_OPENMP)
            target_link_libraries(grad_aff PUBLIC OpenMP::OpenMP_CXX)
        endif()
        #else()
        # Check for CPP 11 Threads
        #CHECK_INCLUDE_FILE_CXX(thread GRAD_AFF_CPP11_THREADS)
        #message(STATUS "Using C++11 threads for parallelism")
        #add_definitions(-DGRAD_AFF_USE_CPP11_THREADS)

    endif()
endif()

if(TRUE)
    #target_link_libraries(grad_aff PUBLIC -static-libgcc -static-libstdc++)
endif()

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

# Install
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION "include/grad_aff" FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
if(NOT USE_STATIC_LINKING AND MSVC)
    install(FILES $<TARGET_PDB_FILE:grad_aff> DESTINATION bin OPTIONAL)

    # Copy Dep DLLs
    file(GLOB GRAD_AFF_DLSS "${CMAKE_CURRENT_BINARY_DIR}/*.dll")
    install(FILES ${GRAD_AFF_DLSS} DESTINATION bin)
endif()

install(TARGETS grad_aff
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
