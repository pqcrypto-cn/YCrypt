# YCrypt

[**YCrypt**](https://pqcrypto.dev/benchmarkings/ycrypt) 是一个高性能的国密算法库，实现了 SM2 椭圆曲线公钥密码算法、SM3 密码杂凑算法和 SM4 分组密码算法。该项目由郁昱教授团队（[上海交通大学](https://crypto.sjtu.edu.cn/lab/) 、[上海期智研究院](https://sqz.ac.cn/password-48)）开发和维护，致力于提供自主可控、安全高效的国密算法实现，为国密应用提供完整的解决方案。

YCrypt 在性能上表现优异，**领先于国内外所有开源国密实现**，在 **2024 年金融密码杯第二阶段赛事中荣获最高奖 —— 一等奖**。

### 主要特性

- **高性能实现**：针对 SM2/SM3/SM4 算法进行深度优化，性能领先
- **标准合规**：严格遵循国家密码管理局发布的 GM/T 系列标准
- **跨平台支持**：支持 x86-64、ARM 等主流平台架构
- **易于集成**：提供简洁清晰的 API 接口，方便快速集成
- **全面测试**：包含完整的测试用例和性能基准测试

**YCrypt** is a high-performance Chinese National Cryptographic Algorithms library, implementing SM2 elliptic curve public key cryptography, SM3 cryptographic hash algorithm, and SM4 block cipher algorithm. This project is developed and maintained by Professor Yu Yu's team ([Shanghai Jiao Tong University](https://crypto.sjtu.edu.cn/lab/) and the [Shanghai Qi Zhi Institute](https://sqz.ac.cn/password-48)), committed to providing autonomous, controllable, secure and efficient implementations of Chinese national cryptographic algorithms, offering complete solutions for cryptographic applications.

YCrypt demonstrates excellent performance, **leading all domestic and international open-source implementations of Chinese national cryptographic algorithms**. It won **the highest award - First Prize in the second phase of the 2024 Financial Cryptography Cup**.

### Key Features

- **High Performance**: Deeply optimized implementations of SM2/SM3/SM4 algorithms with leading performance
- **Standards Compliant**: Strictly follows GM/T series standards published by the State Cryptography Administration
- **Cross-Platform**: Supports mainstream platforms including x86-64, ARM, etc.
- **Easy Integration**: Provides clean and simple API interfaces for quick integration
- **Comprehensive Testing**: Includes complete test suites and performance benchmarks

---

## Versions

| Version | YCrypt-std | YCrypt-adv |
|---------|------------|------------|
| SM2     | ✅         | ✅         |
| SM3     | ✅         | ✅         |
| SM4     | ✅         | ✅         |
| Features | Cross-platform/architecture with high compatibility | Customized high-performance optimizations for x64, ARM etc. |
| Source Code | Open-source in this repository | Please contact us for advanced version support |

**Note**: The current repository contains the **open-source standard version (YCrypt-std)**. For the advanced version (YCrypt-adv) with platform-specific optimizations, please contact us.

---

## Build Instructions

### Prerequisites

- CMake 3.16 or higher
- GCC or Clang compiler with C11 support
- (Optional) OpenSSL for comparison tests

### Quick Start

```bash
# Clone the repository
git clone https://github.com/yourusername/YCrypt.git
cd YCrypt

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make

# Run tests
./sm2/test_sm2
./sm3/test_sm3
./sm4/test_sm4
```

### CMake Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `YCRYPT_BUILD_TESTS` | Build test programs | ON |
| `YCRYPT_BUILD_SPEED` | Build speed benchmark programs | OFF |
| `YCRYPT_WITH_OPENSSL` | Enable OpenSSL cross-verification in tests | OFF |

**Note**: When `YCRYPT_WITH_OPENSSL` is enabled, test programs will include cross-verification tests against OpenSSL's SM2/SM3/SM4 implementations. This helps verify correctness and compatibility with OpenSSL 3.x+.

**Examples**:

Build with all tests and OpenSSL comparison:
```bash
cmake -DYCRYPT_BUILD_TESTS=ON -DYCRYPT_WITH_OPENSSL=ON ..
make
```

Build with speed benchmarks and OpenSSL comparison:
```bash
cmake -DYCRYPT_BUILD_SPEED=ON -DYCRYPT_WITH_OPENSSL=ON ..
make
```

Build everything (tests + benchmarks + OpenSSL):
```bash
cmake -DYCRYPT_BUILD_TESTS=ON -DYCRYPT_BUILD_SPEED=ON -DYCRYPT_WITH_OPENSSL=ON ..
make
```

### Build Targets

- **Unified Shared Library**: `libycrypt.so` - **Recommended** - Single library integrating all SM2/SM3/SM4 algorithms (~1.1MB)
- **Individual Static Libraries**: `sm2`, `sm3`, `sm4`
- **Individual Shared Libraries**: `sm2_shared`, `sm3_shared`, `sm4_shared`
- **Tests**: `test_sm2`, `test_sm3`, `test_sm4`
- **Benchmarks**: `test_speed_sm2`, `test_speed_sm3`, `test_speed_sm4`

**Note**: For most applications, we recommend using the unified `libycrypt.so` library for simplicity and ease of integration.

---

## Testing

### Running Tests

After building with `YCRYPT_BUILD_TESTS=ON`, run the test programs:

```bash
# Run SM3 tests
./build/bin/test_sm3

# Run SM4 tests
./build/bin/test_sm4

# Run SM2 tests
./build/bin/test_sm2
```

### OpenSSL Cross-Verification

YCrypt includes comprehensive cross-verification tests against OpenSSL 3.x+. Build with OpenSSL support:

```bash
cmake -DYCRYPT_WITH_OPENSSL=ON ..
make
./build/bin/test_sm3   # Includes 10,000 SM3 + SM3-HMAC tests vs OpenSSL
./build/bin/test_sm4   # Includes 10,000 SM4-CTR tests vs OpenSSL
./build/bin/test_sm2   # Includes SM2 sign/verify tests vs OpenSSL
```

Each test program performs:
- **SM3**: Hash and HMAC comparison with random messages (0-5000 bytes)
- **SM4**: CTR mode encryption comparison with random data (1-4096 bytes)
- **SM2**: Key generation, signature, and verification cross-testing

### Using Makefile (Individual Modules)

You can also build and test individual modules using their standalone Makefiles:

```bash
# SM3 with OpenSSL comparison
cd sm3
make test TEST_WITH_OPENSSL=1
./test/test_sm3

# SM4 with OpenSSL comparison
cd sm4
make test TEST_WITH_OPENSSL=1
./test/test_sm4

# SM2 with OpenSSL comparison
cd sm2
make test TEST_WITH_OPENSSL=1
./test/test_sm2
```

---

## Usage

### Using the Unified Library

The easiest way to use YCrypt is through the unified library `libycrypt.so`:

```bash
# Compile your application
gcc your_app.c -o your_app -lycrypt -I/path/to/YCrypt/include

# Run your application
./your_app
```

All SM2, SM3, and SM4 functions are available in this single library.

### SM3 Hash Algorithm

```c
#include "include/sm3.h"

// Example: Compute SM3 hash
unsigned char message[] = "Hello, SM3!";
unsigned char hash[32];

SM3_CTX ctx;
sm3_init(&ctx);
sm3_update(&ctx, message, strlen((char*)message));
sm3_final(&ctx, hash);
```

### SM4 Block Cipher

```c
#include "include/sm4.h"

// Example: SM4 encryption
unsigned char key[16] = {0x01, 0x23, 0x45, 0x67, ...};
unsigned char plaintext[16] = "Hello, SM4!";
unsigned char ciphertext[16];

SM4_KEY sm4_key;
sm4_set_key(key, &sm4_key);
sm4_encrypt(plaintext, ciphertext, &sm4_key);
sm4_decrypt(ciphertext, plaintext, &sm4_key);
```

### SM2 Elliptic Curve Cryptography

```c
#include "include/sm2.h"

// Example: SM2 key generation and signature
SM2_KEY_PAIR key_pair;
unsigned char message[] = "Hello, SM2!";
unsigned char signature[64];

// Generate key pair
sm2_generate_key_pair(&key_pair);

// Sign message
sm2_sign(message, strlen((char*)message), &key_pair, signature);

// Verify signature
int result = sm2_verify(message, strlen((char*)message),
                        &key_pair.public_key, signature);
```

---

## Performance

YCrypt achieves leading performance across all implementations:

| Algorithm | Operation | Performance |
|-----------|-----------|-------------|
| SM3 | Hash (MB/s) | See benchmark results |
| SM4 | Encryption (MB/s) | See benchmark results |
| SM2 | Sign/Verify (ops/s) | See benchmark results |

### Running Benchmarks

Build with speed benchmarks enabled:

```bash
cmake -DYCRYPT_BUILD_SPEED=ON ..
make
```

Run the speed benchmarks to see performance on your platform:

```bash
# Using CMake build
./build/bin/test_speed_sm3
./build/bin/test_speed_sm4
./build/bin/test_speed_sm2

# Or using individual Makefiles
cd sm3 && make speed && ./test/test_speed
cd sm4 && make speed && ./test/test_speed
cd sm2 && make speed && ./test/test_speed
```

**Note**: Speed benchmarks link against the static library (`libycrypt.a`) for optimal performance, while test programs use the shared library (`libycrypt.so`) for easier development and testing.

---

## Project Structure

```
YCrypt/
├── CMakeLists.txt          # Root CMake configuration
├── include/                # Public header files
│   ├── sm2.h
│   ├── sm3.h
│   ├── sm4.h
│   └── sm_interface.h
├── sm2/                    # SM2 implementation
│   ├── CMakeLists.txt
│   ├── sm2.c
│   ├── ecc.c
│   └── test/
├── sm3/                    # SM3 implementation
│   ├── CMakeLists.txt
│   ├── sm3.c
│   └── test/
└── sm4/                    # SM4 implementation
    ├── CMakeLists.txt
    ├── sm4.c
    └── test/
```

---

## Standards Compliance

YCrypt implements the following Chinese national cryptographic standards:

- **GM/T 0003-2012**: SM2 Elliptic Curve Public Key Cryptography
- **GM/T 0004-2012**: SM3 Cryptographic Hash Algorithm
- **GM/T 0002-2012**: SM4 Block Cipher Algorithm

---

## Contributing

We welcome contributions! Please feel free to submit issues and pull requests.

---

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

Copyright 2024 YCrypt Development Team

---

## Contact

For the advanced version (YCrypt-adv) with platform-specific optimizations, commercial support, or other inquiries, please contact us.

