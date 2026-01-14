# a =  0x3A02E7B0CBAC0ACD507E40EC1228D642FC20C8432F3518964ED90CBB66509399
# sm2P = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
# sm2N = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
# def dumpvalueTo64bitSeperateList(value):
#     hex_value = hex(value).replace("0x", "")
#     _64bit_count = len(hex_value) // 16
#     rest_bit = len(hex_value) % 16
#     res = []
#     if rest_bit > 0:
#         res.append(hex_value[0:rest_bit])
#     for i in range(_64bit_count):
#         res.append(hex_value[rest_bit + i * 16 : rest_bit + (i + 1) * 16])
#     return res

# print(dumpvalueTo64bitSeperateList((a*a)))
# print(dumpvalueTo64bitSeperateList(sm2N//2))

a = [
    0x943d92300ac642e9, 0x8b47a4531b874134, 0x87b3fd9e391b76f, 0xd8f2367ef50a1f47, 
0x71052c6a87476a31, 0xaf375c5f4513cd27, 0x984f0350f2b19a5b, 0x48c849d4fff241a3
]

b = [
0x943d92300ac642e9, 0x8b47a4531b874134, 0x87b3fd9e391b76f, 0xd8f2367ef50a1f47, 
0x71052c6a87476a31, 0xaf375c5f4513cd27, 0x984f0350f2b19a5b, 0x48c849d4fff241a3
]

# 0x7E1572F8DB6CDF7B, 0xBEEAA65882A612F8, 0xB0C5A94B7F700AAB, 0x748142F61A9C13A3, 
# y = 
# 0xC67DE7DBC88812E0, 0x6E71DDAD8D1B611B, 0x93D62ADB8C016D4C, 0x742FE1D2351C64AE

for i in range(len(a)):
    if a[i] != b[i]:
        print(f"ERROR at idx:{i}, a:{a[i]}, b:{b[i]}")

print("FINISH")
