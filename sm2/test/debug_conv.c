#include <stdio.h>
#include <string.h>
#include "../include/sm2.h"
#include "../include/utils.h"
#include "../include/ecc.h"
#include "../include/fieldOp.h"

void print_u32_hex(const char* name, const u32* v) {
    printf("%s = 0x%016llx%016llx%016llx%016llx\n",
           name, v->v[3], v->v[2], v->v[1], v->v[0]);
}

int main() {
    gen_tables();

    printf("=== Testing conversion functions ===\n\n");

    // Test 1: affine -> jacobian -> affine (regular)
    printf("--- Test 1: Regular affine -> jacobian -> affine ---\n");
    print_u32_hex("G.x (input)", &SM2_G.x);
    print_u32_hex("G.y (input)", &SM2_G.y);

    JPoint G_jac;
    affine_to_jacobian(&SM2_G, &G_jac);
    printf("After affine_to_jacobian:\n");
    print_u32_hex("  z", &G_jac.z);

    AFPoint G_back;
    jacobian_to_affine(&G_jac, &G_back);
    printf("After jacobian_to_affine:\n");
    print_u32_hex("  x", &G_back.x);
    print_u32_hex("  y", &G_back.y);
    printf("Matches G: %s\n",
           u32_eq(&G_back.x, &SM2_G.x) && u32_eq(&G_back.y, &SM2_G.y) ? "YES" : "NO");
    printf("On curve: %s\n", is_on_curve(&G_back) ? "YES" : "NO");

    // Test 2: affine -> montgomery jacobian -> affine
    printf("\n--- Test 2: Montgomery affine -> jacobian -> affine ---\n");
    JPoint G_montg_jac;
    montg_apoint_to_jpoint(&SM2_G, &G_montg_jac);
    printf("After montg_apoint_to_jpoint:\n");
    print_u32_hex("  x", &G_montg_jac.x);
    print_u32_hex("  y", &G_montg_jac.y);
    print_u32_hex("  z", &G_montg_jac.z);

    u32 x_back, y_back;
    montg_jpoint_to_apoint(&G_montg_jac, x_back.v, y_back.v);
    printf("After montg_jpoint_to_apoint:\n");
    print_u32_hex("  x", &x_back);
    print_u32_hex("  y", &y_back);
    printf("Matches G: %s\n",
           u32_eq(&x_back, &SM2_G.x) && u32_eq(&y_back, &SM2_G.y) ? "YES" : "NO");

    // Test 3: Test regular inv_for_mul_mod_p
    printf("\n--- Test 3: inv_for_mul_mod_p(2) * 2 == 1? ---\n");
    u32 two = { 2, 0, 0, 0 };
    u32 inv2;
    inv_for_mul_mod_p(&two, &inv2);
    u32 product;
    mul_mod_p(&two, &inv2, &product);
    u32 one = { 1, 0, 0, 0 };
    print_u32_hex("2 * inv(2)", &product);
    printf("== 1? %s\n", u32_eq(&product, &one) ? "YES" : "NO");

    // Test 4: Test jacobian_to_affine with Z=2
    printf("\n--- Test 4: jacobian_to_affine with Z != 1 ---\n");
    // Point (2X, 2Y, 2) should equal (X/2, Y/4) after conversion... no that's wrong
    // In Jacobian: (X : Y : Z) -> affine (X/Z^2, Y/Z^3)
    // So (X', Y', 2) -> affine (X'/4, Y'/8)
    // If we want affine (X, Y), Jacobian should be (4X, 8Y, 2)

    u32 Gx_4, Gy_8;
    mul_by_2_mod_p(&SM2_G.x, &Gx_4);
    mul_by_2_mod_p(&Gx_4, &Gx_4);  // 4X

    mul_by_2_mod_p(&SM2_G.y, &Gy_8);
    mul_by_2_mod_p(&Gy_8, &Gy_8);
    mul_by_2_mod_p(&Gy_8, &Gy_8);  // 8Y

    JPoint test_jac;
    memcpy(&test_jac.x, &Gx_4, sizeof(u32));
    memcpy(&test_jac.y, &Gy_8, sizeof(u32));
    test_jac.z.v[0] = 2; test_jac.z.v[1] = 0; test_jac.z.v[2] = 0; test_jac.z.v[3] = 0;

    AFPoint test_af;
    jacobian_to_affine(&test_jac, &test_af);
    printf("jacobian_to_affine((4Gx, 8Gy, 2)):\n");
    print_u32_hex("  x", &test_af.x);
    printf("Matches G.x: %s\n", u32_eq(&test_af.x, &SM2_G.x) ? "YES" : "NO");

    return 0;
}
