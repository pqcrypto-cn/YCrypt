#!/usr/bin/env python3
"""
Verify point doubling step by step.
"""

P = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
a = P - 3  # a = -3 for SM2

# Generator point G
Gx = 0x32c4ae2c1f1981195f9904466a39c9948fe30bbff2660be1715a4589334c74c7
Gy = 0xbc3736a2f4f6779c59bdcee36b692153d0a9877cc62a474002df32e52139f0a0

def modinv(a, p=P):
    """Modular inverse using Fermat's little theorem."""
    return pow(a, p - 2, p)

def double_affine(x, y):
    """Double point in affine coordinates."""
    # lambda = (3*x^2 + a) / (2*y)
    lam = ((3 * x * x + a) * modinv(2 * y)) % P
    x3 = (lam * lam - 2 * x) % P
    y3 = (lam * (x - x3) - y) % P
    return x3, y3

def double_jacobian(X, Y, Z):
    """Double point in Jacobian coordinates using the Intel-IPP algorithm."""
    # This matches the C code: double_JPoint at line 282
    S = (2 * Y) % P           # S = 2Y
    U = (Z * Z) % P           # U = Z^2
    M = (S * S) % P           # M = 4Y^2
    rZ = (S * Z) % P          # rZ = 2YZ
    rY = (M * M) % P          # rY = 16Y^4
    S = (M * X) % P           # S = 4XY^2
    rY = (rY * modinv(2)) % P # rY = 8Y^4

    # Get lambda1
    M = (U + X) % P           # M = X+Z^2
    U = (X - U) % P           # U = X-Z^2  (NOTE: might need to handle negative)
    M = (M * U) % P           # M = (X+Z^2)*(X-Z^2) = X^2-Z^4
    M = (3 * M) % P           # M = 3*(X^2-Z^4)

    U = (2 * S) % P           # U = 8XY^2
    rX = (M * M) % P          # rX = M^2
    rX = (rX - U) % P         # rX = M^2 - U

    S = (S - rX) % P          # S = 4XY^2 - rX
    S = (S * M) % P           # S = M*(4XY^2 - rX)
    rY = (S - rY) % P         # rY = M*(4XY^2-rX) - 8Y^4

    return rX, rY, rZ

def jacobian_to_affine(X, Y, Z):
    """Convert Jacobian to affine coordinates."""
    Zinv = modinv(Z)
    Zinv2 = (Zinv * Zinv) % P
    Zinv3 = (Zinv2 * Zinv) % P
    x = (X * Zinv2) % P
    y = (Y * Zinv3) % P
    return x, y

print("=== Verification of Point Doubling ===\n")

print("Input G (affine):")
print(f"  x = 0x{Gx:064x}")
print(f"  y = 0x{Gy:064x}")

# Method 1: Direct affine doubling
x2_aff, y2_aff = double_affine(Gx, Gy)
print("\n2*G via affine doubling:")
print(f"  x = 0x{x2_aff:064x}")
print(f"  y = 0x{y2_aff:064x}")

# Method 2: Jacobian doubling (starting with Z=1)
X, Y, Z = Gx, Gy, 1
X2, Y2, Z2 = double_jacobian(X, Y, Z)
print("\n2*G via Jacobian doubling (before affine conversion):")
print(f"  X = 0x{X2:064x}")
print(f"  Y = 0x{Y2:064x}")
print(f"  Z = 0x{Z2:064x}")

x2_jac, y2_jac = jacobian_to_affine(X2, Y2, Z2)
print("\n2*G via Jacobian (after affine conversion):")
print(f"  x = 0x{x2_jac:064x}")
print(f"  y = 0x{y2_jac:064x}")

print("\n=== Step-by-step trace of Jacobian doubling with Z=1 ===")
X, Y, Z = Gx, Gy, 1

S = (2 * Y) % P
print(f"S = 2Y = 0x{S:064x}")

U = (Z * Z) % P
print(f"U = Z^2 = 0x{U:064x}")

M = (S * S) % P
print(f"M = S^2 = 4Y^2 = 0x{M:064x}")

rZ = (S * Z) % P
print(f"rZ = S*Z = 2YZ = 0x{rZ:064x}")

rY = (M * M) % P
print(f"rY = M^2 = 16Y^4 = 0x{rY:064x}")

S = (M * X) % P
print(f"S = M*X = 4XY^2 = 0x{S:064x}")

rY_8y4 = (rY * modinv(2)) % P
print(f"rY = rY/2 = 8Y^4 = 0x{rY_8y4:064x}")

# Get lambda1
M_xz2 = (U + X) % P
print(f"M = U+X = X+Z^2 = 0x{M_xz2:064x}")

U_xmz2 = (X - U) % P  # This could be negative but mod P handles it
print(f"U = X-U = X-Z^2 = 0x{U_xmz2:064x}")

M_prod = (M_xz2 * U_xmz2) % P
print(f"M = (X+Z^2)*(X-Z^2) = X^2-Z^4 = 0x{M_prod:064x}")

M_3 = (3 * M_prod) % P
print(f"M = 3*M = 3*(X^2-Z^4) = 0x{M_3:064x}")

U_8xy2 = (2 * S) % P
print(f"U = 2*S = 8XY^2 = 0x{U_8xy2:064x}")

rX = (M_3 * M_3) % P
print(f"rX = M^2 = 0x{rX:064x}")

rX = (rX - U_8xy2) % P
print(f"rX = M^2 - U = 0x{rX:064x}")

S_diff = (S - rX) % P
print(f"S = 4XY^2 - rX = 0x{S_diff:064x}")

S_prod = (S_diff * M_3) % P
print(f"S = M*(4XY^2-rX) = 0x{S_prod:064x}")

rY_final = (S_prod - rY_8y4) % P
print(f"rY = S - 8Y^4 = 0x{rY_final:064x}")

print(f"\nFinal rZ = 0x{rZ:064x}")

# Verify
x_final, y_final = jacobian_to_affine(rX, rY_final, rZ)
print(f"\nConverted to affine:")
print(f"  x = 0x{x_final:064x}")
print(f"  y = 0x{y_final:064x}")
print(f"\nMatches affine doubling: {x_final == x2_aff and y_final == y2_aff}")

# Verify point is on curve
lhs = (y_final * y_final) % P
rhs = (x_final * x_final * x_final + a * x_final + 0x28e9fa9e9d9f5e344d5a9e4bcf6509a7f39789f515ab8f92ddbcbd414d940e93) % P
print(f"On curve: {lhs == rhs}")
