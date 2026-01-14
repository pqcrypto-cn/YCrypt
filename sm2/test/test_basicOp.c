#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/fieldOp.h"
#include "../include/basicOp.h"
#include "../include/sm2_const.h"

static int test_count = 0;
static int pass_count = 0;

#define TEST(name) printf("\n=== Testing %s ===\n", name)
#define PASS(msg) do { test_count++; pass_count++; printf("[PASS] %s\n", msg); } while(0)
#define FAIL(msg) do { test_count++; printf("[FAIL] %s\n", msg); } while(0)
#define CHECK(cond, msg) do { if (cond) PASS(msg); else FAIL(msg); } while(0)

void print_u8(const char* name, UINT64 val) {
    printf("%s = 0x%016llx\n", name, val);
}

void print_u32_val(const char* name, const u32* v) {
    printf("%s = { 0x%016llx, 0x%016llx, 0x%016llx, 0x%016llx }\n",
           name, v->v[0], v->v[1], v->v[2], v->v[3]);
}

void print_arr8(const char* name, const UINT64* arr, int len) {
    printf("%s = { ", name);
    for (int i = 0; i < len; i++) {
        printf("0x%016llx%s", arr[i], i < len-1 ? ", " : " }\n");
    }
}

// ============================================================================
// Test portable_addcarryx_u64
// ============================================================================
void test_addcarryx() {
    TEST("portable_addcarryx_u64");

    u8 result;
    u1 carry;

    // Test 1: simple addition, no carry
    carry = portable_addcarryx_u64(0, 5, 3, &result);
    CHECK(result == 8 && carry == 0, "5 + 3 = 8, carry=0");

    // Test 2: addition with incoming carry
    carry = portable_addcarryx_u64(1, 5, 3, &result);
    CHECK(result == 9 && carry == 0, "5 + 3 + 1 = 9, carry=0");

    // Test 3: overflow generating carry
    carry = portable_addcarryx_u64(0, 0xFFFFFFFFFFFFFFFF, 1, &result);
    CHECK(result == 0 && carry == 1, "0xFFFF...FFFF + 1 = 0, carry=1");

    // Test 4: overflow with incoming carry
    carry = portable_addcarryx_u64(1, 0xFFFFFFFFFFFFFFFF, 0, &result);
    CHECK(result == 0 && carry == 1, "0xFFFF...FFFF + 0 + 1 = 0, carry=1");

    // Test 5: max + max
    carry = portable_addcarryx_u64(0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, &result);
    CHECK(result == 0xFFFFFFFFFFFFFFFE && carry == 1, "max + max = max-1, carry=1");

    // Test 6: max + max + 1
    carry = portable_addcarryx_u64(1, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, &result);
    CHECK(result == 0xFFFFFFFFFFFFFFFF && carry == 1, "max + max + 1 = max, carry=1");
}

// ============================================================================
// Test portable_mulx_u64
// ============================================================================
void test_mulx() {
    TEST("portable_mulx_u64");

    u8 lo, hi;

    // Test 1: simple multiplication
    lo = portable_mulx_u64(2, 3, &hi);
    CHECK(lo == 6 && hi == 0, "2 * 3 = 6, hi=0");

    // Test 2: larger values
    lo = portable_mulx_u64(0x100000000, 0x100000000, &hi);
    CHECK(lo == 0 && hi == 1, "2^32 * 2^32 = 2^64, hi=1");

    // Test 3: max * 1
    lo = portable_mulx_u64(0xFFFFFFFFFFFFFFFF, 1, &hi);
    CHECK(lo == 0xFFFFFFFFFFFFFFFF && hi == 0, "max * 1 = max, hi=0");

    // Test 4: max * 2
    lo = portable_mulx_u64(0xFFFFFFFFFFFFFFFF, 2, &hi);
    // max * 2 = 2*2^64 - 2 = (1, 0xFFFFFFFFFFFFFFFE)
    CHECK(lo == 0xFFFFFFFFFFFFFFFE && hi == 1, "max * 2");

    // Test 5: max * max
    // (2^64 - 1)^2 = 2^128 - 2*2^64 + 1 = (2^64-2, 1)
    lo = portable_mulx_u64(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, &hi);
    CHECK(lo == 1 && hi == 0xFFFFFFFFFFFFFFFE, "max * max");

    // Test 6: known value (verified with Python)
    // 0x1234567890ABCDEF * 0xFEDCBA0987654321 = 0x121fa000a3723a57c24a442fe55618cf
    lo = portable_mulx_u64(0x1234567890ABCDEF, 0xFEDCBA0987654321, &hi);
    printf("  0x1234567890ABCDEF * 0xFEDCBA0987654321 = hi:0x%016llx, lo:0x%016llx\n", hi, lo);
    CHECK(hi == 0x121fa000a3723a57 && lo == 0xc24a442fe55618cf, "known multiplication");
}

// ============================================================================
// Test raw_mul
// ============================================================================
void test_raw_mul() {
    TEST("raw_mul");

    // Test 1: multiply by 1
    u32 a = { 0x1234567890ABCDEF, 0xFEDCBA0987654321, 0x0123456789ABCDEF, 0xFEDCBA9876543210 };
    u32 one = { 1, 0, 0, 0 };
    UINT64 result[9] = {0};

    raw_mul(a.v, one.v, result);
    CHECK(result[0] == a.v[0] && result[1] == a.v[1] &&
          result[2] == a.v[2] && result[3] == a.v[3] &&
          result[4] == 0 && result[5] == 0 && result[6] == 0 && result[7] == 0,
          "a * 1 = a");

    // Test 2: R * 1 (where R = 2^256 mod P)
    u32 R = { 0x0000000000000001, 0x00000000FFFFFFFF, 0x0000000000000000, 0x0000000100000000 };
    memset(result, 0, sizeof(result));
    raw_mul(R.v, one.v, result);
    CHECK(result[0] == R.v[0] && result[1] == R.v[1] &&
          result[2] == R.v[2] && result[3] == R.v[3] &&
          result[4] == 0 && result[5] == 0 && result[6] == 0 && result[7] == 0,
          "R * 1 = R");
    print_arr8("R * 1", result, 8);

    // Test 3: use known test vector from sm2_gadget_test.c
    u32 ta = { 0x1351534EF350E2BB, 0x14E68D77BC131F7B, 0x6A7171A01A638E75, 0x4F9EA7A816AB7908 };
    u32 tb = { 0x141CC66D0595B6F0, 0xC85BF76622E07301, 0x5B261629F8AD4D45, 0x7DE9CF63BC635636 };
    u8 expected[8] = {
        0x866d99203adc8150, 0xc623d9758ed1332c, 0x3b1dab20b950e375, 0xbc165cad5d713996,
        0x63e9be904aa539b5, 0x7edc6525c6a1f17c, 0x2a99a65d2ec61248, 0x27292fc3f99184ca
    };

    memset(result, 0, sizeof(result));
    raw_mul(ta.v, tb.v, result);

    int match = 1;
    for (int i = 0; i < 8; i++) {
        if (result[i] != expected[i]) match = 0;
    }
    CHECK(match, "raw_mul known test vector");
    if (!match) {
        print_arr8("  expected", expected, 8);
        print_arr8("  got     ", result, 8);
    }
}

// ============================================================================
// Test raw_pow (squaring)
// ============================================================================
void test_raw_pow() {
    TEST("raw_pow");

    // raw_pow(a) should equal raw_mul(a, a)
    u32 a = { 0x1234567890ABCDEF, 0xFEDCBA0987654321, 0x0123456789ABCDEF, 0xFEDCBA9876543210 };
    UINT64 mul_result[9] = {0};
    UINT64 pow_result[9] = {0};

    raw_mul(a.v, a.v, mul_result);
    raw_pow(a.v, pow_result);

    int match = 1;
    for (int i = 0; i < 8; i++) {
        if (mul_result[i] != pow_result[i]) match = 0;
    }
    CHECK(match, "raw_pow(a) == raw_mul(a, a)");
    if (!match) {
        print_arr8("  mul result", mul_result, 8);
        print_arr8("  pow result", pow_result, 8);
    }
}

// ============================================================================
// Test sm2p_mong_mul
// ============================================================================
void test_mong_mul() {
    TEST("sm2p_mong_mul");

    u32 R = SM2_rhoP;  // R = 2^256 mod P
    u32 one = { 1, 0, 0, 0 };
    u32 result;

    // Test 1: R * 1 should give R * 1 * R^(-1) mod P = 1
    printf("Testing: sm2p_mong_mul(R, 1) -> expect 1\n");
    print_u32_val("  R", &R);
    print_u32_val("  one", &one);

    sm2p_mong_mul(&R, &one, &result);
    print_u32_val("  result", &result);

    CHECK(u32_eq(&result, &one), "sm2p_mong_mul(R, 1) == 1");

    // Test 2: 1 * 1 should give 1 * 1 * R^(-1) mod P = R^(-1) mod P
    printf("\nTesting: sm2p_mong_mul(1, 1)\n");
    sm2p_mong_mul(&one, &one, &result);
    print_u32_val("  result", &result);

    // Test 3: R * R should give R * R * R^(-1) mod P = R mod P = R (since R < P)
    printf("\nTesting: sm2p_mong_mul(R, R) -> expect R\n");
    sm2p_mong_mul(&R, &R, &result);
    print_u32_val("  result", &result);
    CHECK(u32_eq(&result, &R), "sm2p_mong_mul(R, R) == R");
}

// ============================================================================
// Test montg_to/back conversions
// ============================================================================
void test_montg_conversion() {
    TEST("Montgomery conversion");

    u32 one = { 1, 0, 0, 0 };
    u32 two = { 2, 0, 0, 0 };
    u32 R = SM2_rhoP;
    u32 result, back;

    // montg_to_mod_p(x) = x * R mod P
    // montg_back_mod_p(x) = x * R^(-1) mod P

    // Test 1: montg_to(1) should give R
    printf("Testing: montg_to_mod_p(1) -> expect R\n");
    montg_to_mod_p(&one, &result);
    print_u32_val("  result", &result);
    print_u32_val("  R     ", &R);
    CHECK(u32_eq(&result, &R), "montg_to_mod_p(1) == R");

    // Test 2: montg_back(R) should give 1
    printf("\nTesting: montg_back_mod_p(R) -> expect 1\n");
    montg_back_mod_p(&R, &result);
    print_u32_val("  result", &result);
    CHECK(u32_eq(&result, &one), "montg_back_mod_p(R) == 1");

    // Test 3: round-trip: montg_back(montg_to(x)) == x
    printf("\nTesting: montg_back(montg_to(2)) -> expect 2\n");
    montg_to_mod_p(&two, &result);
    print_u32_val("  montg_to(2)", &result);
    montg_back_mod_p(&result, &back);
    print_u32_val("  back       ", &back);
    CHECK(u32_eq(&back, &two), "round-trip works for 2");
}

// ============================================================================
// Test inv_for_mul_mod_p
// ============================================================================
void test_inv_for_mul() {
    TEST("inv_for_mul_mod_p");

    u32 one = { 1, 0, 0, 0 };
    u32 two = { 2, 0, 0, 0 };
    u32 result, verify;

    // inv_for_mul_mod_p(x) should give x^(-1) mod P
    // So x * inv_for_mul_mod_p(x) = 1 mod P

    // Test 1: inv_for_mul(1) should give 1
    printf("Testing: inv_for_mul_mod_p(1) -> expect 1\n");
    inv_for_mul_mod_p(&one, &result);
    print_u32_val("  result", &result);
    CHECK(u32_eq(&result, &one), "inv_for_mul_mod_p(1) == 1");

    // Test 2: 2 * inv_for_mul(2) should give 1
    printf("\nTesting: 2 * inv_for_mul_mod_p(2) -> expect 1\n");
    inv_for_mul_mod_p(&two, &result);
    print_u32_val("  inv(2)", &result);
    mul_mod_p(&two, &result, &verify);
    print_u32_val("  2*inv(2)", &verify);
    CHECK(u32_eq(&verify, &one), "2 * inv(2) == 1");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    printf("============================================\n");
    printf("basicOp.c / fieldOp.c Unit Tests\n");
    printf("============================================\n");

    test_addcarryx();
    test_mulx();
    test_raw_mul();
    test_raw_pow();
    test_mong_mul();
    test_montg_conversion();
    test_inv_for_mul();

    printf("\n============================================\n");
    printf("Results: %d / %d tests passed\n", pass_count, test_count);
    printf("============================================\n");

    return (pass_count == test_count) ? 0 : 1;
}
