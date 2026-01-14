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

    printf("=== Debug montg_double_jpoint vs regular double_JPoint ===\n\n");

    // Method 1: Regular Jacobian doubling
    printf("--- Method 1: Regular Jacobian ---\n");
    JPoint G_jac;
    affine_to_jacobian(&SM2_G, &G_jac);
    printf("G in Jacobian:\n");
    print_u32_hex("  x", &G_jac.x);
    print_u32_hex("  y", &G_jac.y);
    print_u32_hex("  z", &G_jac.z);

    JPoint G2_jac;
    double_JPoint(&G_jac, &G2_jac);

    AFPoint G2_af;
    jacobian_to_affine(&G2_jac, &G2_af);
    printf("\n2*G (regular):\n");
    print_u32_hex("  x", &G2_af.x);
    print_u32_hex("  y", &G2_af.y);
    printf("On curve: %s\n", is_on_curve(&G2_af) ? "YES" : "NO");

    // Method 2: Montgomery Jacobian doubling
    printf("\n--- Method 2: Montgomery Jacobian ---\n");
    JPoint G_montg;
    montg_apoint_to_jpoint(&SM2_G, &G_montg);
    printf("G in Montgomery Jacobian:\n");
    print_u32_hex("  x", &G_montg.x);
    print_u32_hex("  y", &G_montg.y);
    print_u32_hex("  z", &G_montg.z);

    JPoint G2_montg;
    montg_double_jpoint(&G_montg, &G2_montg);
    printf("\n2*G Montgomery Jacobian (before conversion):\n");
    print_u32_hex("  x", &G2_montg.x);
    print_u32_hex("  y", &G2_montg.y);
    print_u32_hex("  z", &G2_montg.z);

    u32 x_result, y_result;
    montg_jpoint_to_apoint(&G2_montg, x_result.v, y_result.v);
    printf("\n2*G (Montgomery, converted to affine):\n");
    print_u32_hex("  x", &x_result);
    print_u32_hex("  y", &y_result);

    AFPoint af_montg;
    memcpy(&af_montg.x, &x_result, sizeof(u32));
    memcpy(&af_montg.y, &y_result, sizeof(u32));
    printf("On curve: %s\n", is_on_curve(&af_montg) ? "YES" : "NO");

    // Method 3: Montgomery times_base_point with k=2
    printf("\n--- Method 3: montg_times_base_point(2) ---\n");
    u32 k2 = { 2, 0, 0, 0 };
    JPoint G2_montg_tbp;
    montg_times_base_point(&k2, &G2_montg_tbp);

    u32 x_tbp, y_tbp;
    montg_jpoint_to_apoint(&G2_montg_tbp, x_tbp.v, y_tbp.v);
    printf("2*G via times_base_point:\n");
    print_u32_hex("  x", &x_tbp);
    print_u32_hex("  y", &y_tbp);

    // Compare results
    printf("\n=== Comparison ===\n");
    printf("Regular == Montgomery double? %s\n",
           u32_eq(&G2_af.x, &x_result) && u32_eq(&G2_af.y, &y_result) ? "YES" : "NO");
    printf("Regular == times_base_point? %s\n",
           u32_eq(&G2_af.x, &x_tbp) && u32_eq(&G2_af.y, &y_tbp) ? "YES" : "NO");
    printf("Montgomery double == times_base_point? %s\n",
           u32_eq(&x_result, &x_tbp) && u32_eq(&y_result, &y_tbp) ? "YES" : "NO");

    return 0;
}
