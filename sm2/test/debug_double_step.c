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

// Copy of double_JPoint with debug output
void debug_double_JPoint(const JPoint* pt, JPoint* result)
{
    u32 S, U, M;
    if (equ_to_JPoint_one(pt))
    {
        *result = *pt;
        return;
    }

    printf("=== Step-by-step C double_JPoint ===\n");
    print_u32_hex("Input X", &pt->x);
    print_u32_hex("Input Y", &pt->y);
    print_u32_hex("Input Z", &pt->z);

    double_mod_p(&(pt->y), &S);  // S = 2Y
    print_u32_hex("S = 2Y", &S);
    // Expected: 0x786e6d46e9ecef38b37b9dc6d6d242a7a1530efa8c548e7f05be65ca4273e141

    pow_mod_p(&(pt->z), &U);       // U = Z^2
    print_u32_hex("U = Z^2", &U);
    // Expected: 0x0000...0001

    pow_mod_p(&S, &M);           // M = 4Y^2
    print_u32_hex("M = S^2 = 4Y^2", &M);
    // Expected: 0xefcbb7774a337bc19244a1fa1df68d9d3eee564a39880297c275b874e3530797

    mul_mod_p(&S, &(pt->z), &(result->z));  // rZ = 2YZ
    print_u32_hex("rZ = S*Z = 2YZ", &result->z);
    // Expected: 0x786e6d46e9ecef38b37b9dc6d6d242a7a1530efa8c548e7f05be65ca4273e141

    pow_mod_p(&M, &(result->y));          // rY = 16Y^4
    print_u32_hex("rY = M^2 = 16Y^4", &result->y);
    // Expected: 0x29e8a363ee6d26eb1caf6ad78a39fa3984844fa436474a3b37ec3d93bdc5d87f

    mul_mod_p(&M, &(pt->x), &S);      // S = 4XY^2
    print_u32_hex("S = M*X = 4XY^2", &S);
    // Expected: 0x1e0381bf9660855ab1f451d3e59a1ddde45a418086fc8386f0da67c3619ec368

    div_by_2_mod_p(&(result->y), &(result->y)); // rY = 8Y^4
    print_u32_hex("rY = rY/2 = 8Y^4", &result->y);
    // Expected: 0x94f451b1773693758e57b56bc51cfd1cc24227d19b23a51e1bf61ec9dee2ec3f

    // Get lambda1
    add_mod_p(&U, &(pt->x), &M);          // M = X+Z^2
    print_u32_hex("M = U+X = X+Z^2", &M);
    // Expected: 0x32c4ae2c1f1981195f9904466a39c9948fe30bbff2660be1715a4589334c74c8

    sub_mod_p(&(pt->x), &U, &U);          // U = X-Z^2
    print_u32_hex("U = X-U = X-Z^2", &U);
    // Expected: 0x32c4ae2c1f1981195f9904466a39c9948fe30bbff2660be1715a4589334c74c6

    mul_mod_p(&M, &U, &M);                 // N = (X+Z^2)*(X-Z^2) = X^2-Z^4
    print_u32_hex("M = (X+Z^2)*(X-Z^2)", &M);
    // Expected: 0xf4e2cca0bcfd67fba8531eebff519e4cb3d47f9fe8c5eff5151f4c497ec99fbe

    mul_by_3_mod_p(&M, &M);              // M = 3*(X^2-Z^4)
    print_u32_hex("M = 3*M", &M);
    // Expected: 0xdea865e436f837f2f8f95cc3fdf4dae61b7d7ee1ba51cfdd3f5de4dc7c5cdf3c

    double_mod_p(&S, &U);                // U = 8XY^2
    print_u32_hex("U = 2*S = 8XY^2", &U);
    // Expected: 0x3c07037f2cc10ab563e8a3a7cb343bbbc8b483010df9070de1b4cf86c33d86d0

    pow_mod_p(&M, &(result->x));             // rX = M^2
    print_u32_hex("rX = M^2", &result->x);
    // Expected: 0xe6f96a985fc8fb7190ada500738e0090615ddd21d336854c219504d04dda4acb

    sub_mod_p(&(result->x), &U, &(result->x));  // rX = M^2-U
    print_u32_hex("rX = M^2-U", &result->x);
    // Expected: 0xaaf267193307f0bc2cc50158a859c4d498a95a20c53d7e3e3fe035498a9cc3fb

    sub_mod_p(&S, &(result->x), &S);      // S = 4XY^2-rX
    print_u32_hex("S = 4XY^2-rX", &S);
    // Expected: 0x73111aa56358949e852f507b3d4059094bb0e75ec1bf0549b0fa3279d701ff6c

    mul_mod_p(&S, &M, &S);                 // S = M*(4XY^2 - rX)
    print_u32_hex("S = M*(4XY^2-rX)", &S);
    // Expected: 0x30ca1f001aa4b36b81a5469f875bd16c5df337d527430105e849c954850442ca

    sub_mod_p(&S, &(result->y), &(result->y)); // rY = M(4XY^2-rX) -8Y^4
    print_u32_hex("rY = final", &result->y);
    // Expected: 0x9bd5cd4da36e1ff5f34d9133c23ed44f9bb110028c1f5be8cc53aa8aa621568a
}

int main() {
    gen_tables();

    printf("=== Testing Regular double_JPoint step by step ===\n\n");

    // Convert G to Jacobian
    JPoint G_jac;
    affine_to_jacobian(&SM2_G, &G_jac);

    // Double with debug
    JPoint G2_jac;
    debug_double_JPoint(&G_jac, &G2_jac);

    // Convert to affine
    printf("\n=== Converting result to affine ===\n");
    AFPoint G2_af;
    jacobian_to_affine(&G2_jac, &G2_af);
    print_u32_hex("Final x", &G2_af.x);
    print_u32_hex("Final y", &G2_af.y);

    printf("\n=== Expected (from Python) ===\n");
    printf("  x = 0x56cefd60d7c87c000d58ef57fa73ba4d9c0dfa08c08a7331495c2e1da3f2bd52\n");
    printf("  y = 0x31b7e7e6cc8189f668535ce0f8eaf1bd6de84c182f6c8e716f780d3a970a23c3\n");

    printf("\nOn curve: %s\n", is_on_curve(&G2_af) ? "YES" : "NO");

    return 0;
}
