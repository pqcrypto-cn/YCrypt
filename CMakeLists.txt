cmake_minimum_required(VERSION 3.16)
project(YCrypt
    VERSION 1.0.0
    DESCRIPTION "Chinese National Cryptographic Algorithms (SM2/SM3/SM4)"
    LANGUAGES C
)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(YCRYPT_BUILD_TESTS "Build test programs" ON)
option(YCRYPT_BUILD_SPEED "Build speed benchmark programs" OFF)
option(YCRYPT_WITH_OPENSSL "Enable OpenSSL comparison in tests" OFF)

# Backward compatibility: support legacy variable names
if(DEFINED ENABLE_TEST_SPEED)
    set(YCRYPT_BUILD_SPEED ${ENABLE_TEST_SPEED})
    message(STATUS "Note: ENABLE_TEST_SPEED is deprecated, please use YCRYPT_BUILD_SPEED instead")
endif()

# Print configuration
message(STATUS "YCrypt Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build tests: ${YCRYPT_BUILD_TESTS}")
message(STATUS "  Build speed benchmarks: ${YCRYPT_BUILD_SPEED}")
message(STATUS "  OpenSSL comparison: ${YCRYPT_WITH_OPENSSL}")

# Common compiler flags
set(COMMON_C_FLAGS -Wall -Wextra -Wno-strict-aliasing -Wno-missing-braces)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND COMMON_C_FLAGS -O3 -fomit-frame-pointer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND COMMON_C_FLAGS -g -DDEBUG)
endif()

# OpenSSL configuration
if(YCRYPT_WITH_OPENSSL)
    find_package(OpenSSL REQUIRED)
    message(STATUS "  OpenSSL version: ${OPENSSL_VERSION}")
    add_compile_definitions(TEST_WITH_OPENSSL)
endif()

# Include directories
# Add project root so "include/sm_interface.h" resolves correctly
include_directories(${CMAKE_SOURCE_DIR})

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories
add_subdirectory(sm3)
add_subdirectory(sm4)
add_subdirectory(sm2)

# Unified YCrypt shared library (libycrypt.so)
# Combines all SM2/SM3/SM4 algorithms into a single library
add_library(ycrypt SHARED
    # SM3 sources
    sm3/sm3.c
    # SM4 sources
    sm4/sm4.c
    sm4/mode/sm4_ctr.c
    # SM2 sources
    sm2/extra.c
    sm2/basicOp.c
    sm2/fieldOp.c
    sm2/ecc.c
    sm2/sm2.c
    sm2/utils.c
    sm2/ecc_montg.c
    sm2/ecc_basepoint_mul.c
)

target_include_directories(ycrypt
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/sm2/include
        ${CMAKE_SOURCE_DIR}/sm3/include
        ${CMAKE_SOURCE_DIR}/sm4/include
)

target_compile_options(ycrypt PRIVATE ${COMMON_C_FLAGS})

# Set library version
set_target_properties(ycrypt PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Unified YCrypt static library (libycrypt.a)
# Combines all SM2/SM3/SM4 algorithms into a single static library
add_library(ycrypt_static STATIC
    # SM3 sources
    sm3/sm3.c
    # SM4 sources
    sm4/sm4.c
    sm4/mode/sm4_ctr.c
    # SM2 sources
    sm2/extra.c
    sm2/basicOp.c
    sm2/fieldOp.c
    sm2/ecc.c
    sm2/sm2.c
    sm2/utils.c
    sm2/ecc_montg.c
    sm2/ecc_basepoint_mul.c
)

target_include_directories(ycrypt_static
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/sm2/include
        ${CMAKE_SOURCE_DIR}/sm3/include
        ${CMAKE_SOURCE_DIR}/sm4/include
)

target_compile_options(ycrypt_static PRIVATE ${COMMON_C_FLAGS})

# Set output name to libycrypt.a (without _static suffix)
set_target_properties(ycrypt_static PROPERTIES OUTPUT_NAME "ycrypt")

# Summary
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  Unified shared library: libycrypt.so (SM2+SM3+SM4) -> lib/")
message(STATUS "  Unified static library: libycrypt.a (SM2+SM3+SM4) -> lib/")
message(STATUS "  Individual libraries: sm3, sm4, sm2 -> lib/")
if(YCRYPT_BUILD_TESTS)
    message(STATUS "  Tests: test_sm3, test_sm4, test_sm2 (link libycrypt.so) -> bin/")
endif()
if(YCRYPT_BUILD_SPEED)
    message(STATUS "  Speed: test_speed_sm3, test_speed_sm4, test_speed_sm2 (link libycrypt.a) -> bin/")
endif()
message(STATUS "")
