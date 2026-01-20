# SM4 - Chinese National Block Cipher Algorithm
# Library and test targets

# SM4 Library
add_library(sm4 STATIC
    sm4.c
    mode/sm4_ctr.c
    mode/sm4_cbc.c
)

target_include_directories(sm4
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(sm4 PRIVATE ${COMMON_C_FLAGS})

# SM4 Shared Library
add_library(sm4_shared SHARED
    sm4.c
    mode/sm4_ctr.c
    mode/sm4_cbc.c
)

target_include_directories(sm4_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(sm4_shared PRIVATE ${COMMON_C_FLAGS})
set_target_properties(sm4_shared PROPERTIES OUTPUT_NAME "sm4")

# Test program - links to unified shared library (libycrypt.so)
if(YCRYPT_BUILD_TESTS)
    add_executable(test_sm4
        test/test_sm4.c
    )

    target_link_libraries(test_sm4 PRIVATE ycrypt)
    target_compile_options(test_sm4 PRIVATE ${COMMON_C_FLAGS})

    if(YCRYPT_WITH_OPENSSL)
        target_link_libraries(test_sm4 PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
endif()

# Speed benchmark - links to unified static library (libycrypt.a)
if(YCRYPT_BUILD_SPEED)
    add_executable(test_speed_sm4
        test/test_speed.c
    )

    target_link_libraries(test_speed_sm4 PRIVATE ycrypt_static)
    target_compile_options(test_speed_sm4 PRIVATE ${COMMON_C_FLAGS})

    if(YCRYPT_WITH_OPENSSL)
        target_link_libraries(test_speed_sm4 PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
endif()
