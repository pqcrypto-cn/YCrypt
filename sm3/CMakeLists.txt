# SM3 - Chinese National Hash Algorithm
# Library and test targets

# SM3 Library
add_library(sm3 STATIC
    sm3.c
)

target_include_directories(sm3
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(sm3 PRIVATE ${COMMON_C_FLAGS})

# SM3 Shared Library
add_library(sm3_shared SHARED
    sm3.c
)

target_include_directories(sm3_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(sm3_shared PRIVATE ${COMMON_C_FLAGS})
set_target_properties(sm3_shared PROPERTIES OUTPUT_NAME "sm3")

# Test program - links to unified shared library (libycrypt.so)
if(YCRYPT_BUILD_TESTS)
    add_executable(test_sm3
        test/test_sm3.c
        utils.c
    )

    target_link_libraries(test_sm3 PRIVATE ycrypt)
    target_compile_options(test_sm3 PRIVATE ${COMMON_C_FLAGS})

    if(YCRYPT_WITH_OPENSSL)
        target_link_libraries(test_sm3 PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
endif()

# Speed benchmark - links to unified static library (libycrypt.a)
if(YCRYPT_BUILD_SPEED)
    add_executable(test_speed_sm3
        test/test_speed.c
    )

    target_link_libraries(test_speed_sm3 PRIVATE ycrypt_static)
    target_compile_options(test_speed_sm3 PRIVATE ${COMMON_C_FLAGS})

    if(YCRYPT_WITH_OPENSSL)
        target_link_libraries(test_speed_sm3 PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    endif()
endif()
