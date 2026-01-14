# SM2 - Chinese National Elliptic Curve Cryptography Algorithm
# Library and test targets
# Note: SM2 depends on SM3 for hashing operations

# SM2 source files (excluding sm3.c - now uses sm3 module)
set(SM2_SOURCES
    extra.c
    basicOp.c
    fieldOp.c
    ecc.c
    sm2.c
    utils.c
    ecc_montg.c
    ecc_basepoint_mul.c
)

# SM2 Library
add_library(sm2 STATIC
    ${SM2_SOURCES}
)

target_include_directories(sm2
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

# Link with SM3
target_link_libraries(sm2 PUBLIC sm3)

target_compile_options(sm2 PRIVATE ${COMMON_C_FLAGS})

# SM2 Shared Library
add_library(sm2_shared SHARED
    ${SM2_SOURCES}
)

target_include_directories(sm2_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

# Link with SM3
target_link_libraries(sm2_shared PUBLIC sm3)

target_compile_options(sm2_shared PRIVATE ${COMMON_C_FLAGS})
set_target_properties(sm2_shared PROPERTIES OUTPUT_NAME "sm2")

# Test program - links to unified shared library (libycrypt.so)
if(YCRYPT_BUILD_TESTS)
    add_executable(test_sm2
        test/test_sm2.c
    )

    target_link_libraries(test_sm2 PRIVATE ycrypt)
    target_compile_options(test_sm2 PRIVATE ${COMMON_C_FLAGS})

    if(YCRYPT_WITH_OPENSSL)
        target_link_libraries(test_sm2 PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
endif()

# Speed benchmark - links to unified static library (libycrypt.a)
if(YCRYPT_BUILD_SPEED)
    add_executable(test_speed_sm2
        test/test_speed.c
    )

    target_link_libraries(test_speed_sm2 PRIVATE ycrypt_static)
    target_compile_options(test_speed_sm2 PRIVATE ${COMMON_C_FLAGS})

    if(YCRYPT_WITH_OPENSSL)
        target_link_libraries(test_speed_sm2 PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
endif()
