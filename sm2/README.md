# SM2 Implementation

SM2 椭圆曲线公钥密码算法实现。

## 编译

```bash
# 编译全部（库、测试、性能测试）
make all

# 仅编译库
make lib

# 仅编译测试
make test

# 仅编译性能测试
make speed

# Debug 模式编译
make all DEBUG=1

# 清理
make clean
```

## 运行测试

```bash
./test/test_sm2
```

## 与第三方库交叉验证

### GmSSL 交叉验证

需要先编译 GmSSL 并修改其头文件以避免符号冲突。

**1. 修改 GmSSL 头文件**

在 `GmSSL/include/gmssl/sm3.h` 的 `#define GMSSL_SM3_H` 之后添加：

```c
// Rename conflicting symbols to avoid collision with YCrypt
#define SM3_CTX GMSSL_SM3_CTX
#define sm3_init gmssl_sm3_init
#define sm3_update gmssl_sm3_update
#define sm3_finish gmssl_sm3_finish
#define SM3_HMAC_CTX GMSSL_SM3_HMAC_CTX
#define sm3_hmac_init gmssl_sm3_hmac_init
#define sm3_hmac_update gmssl_sm3_hmac_update
#define sm3_hmac_finish gmssl_sm3_hmac_finish
```

在 `GmSSL/include/gmssl/sm2.h` 的 `#define GMSSL_SM2_H` 之后添加：

```c
// Rename conflicting symbols to avoid collision with YCrypt
#define sm2_sign gmssl_sm2_sign
#define sm2_verify gmssl_sm2_verify
```

**2. 编译并运行测试**

```bash
# 编译（指定 GmSSL 路径）
make test TEST_WITH_GMSSL=1 GMSSL_ROOT=/path/to/GmSSL

# 运行测试（需要设置库路径）
LD_LIBRARY_PATH=/path/to/GmSSL/build/bin ./test/test_sm2
```

### OpenSSL 交叉验证

需要系统安装 OpenSSL 开发库。

```bash
# Debian/Ubuntu
sudo apt install libssl-dev

# 编译
make test TEST_WITH_OPENSSL=1

# 运行测试
./test/test_sm2
```

### 同时启用两者

```bash
make test TEST_WITH_GMSSL=1 GMSSL_ROOT=/path/to/GmSSL TEST_WITH_OPENSSL=1
LD_LIBRARY_PATH=/path/to/GmSSL/build/bin ./test/test_sm2
```

## 性能测试

```bash
make speed
./test/test_speed
```
