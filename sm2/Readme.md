Readme.md



# **Performance of YogSM2 :**

| **Performance**  | **Sign （times/s）** |  **Verify（times/s）**  |
| ---- | ---- | ---- |
| **SM2**   | **85222** | **17646** |



# **Performance of Intel IPP SM2 :**

| **Performance**  | **Sign （times/s）** |  **Verify（times/s）**  |
| ---- | ---- | ---- |
| **SM2**   | **15690** | **11822** |



## **Platform :**

| Item   | Specification          |
| :----- | ---------------------- |
| CPU    | Intel I7 6700 @3.4GHz  |
| Memory | 16 GB DDR4             |
| Disk   | 512 GB SSD + 2 TB HDD  |
| OS     | Windows 7 x64 Ultimate |





# **News:**



## **[2019-4-15-22-01]:**

After adding function of ```mul_by_3_mod_p```,  the performance of sign is 50210 (times/s), verify: 9954  (times/s).

## **[2019-4-15-22-13]:**

After adding function of ```sub_mod_p``` and ```sub_mod_n```,  the performance of sign is 52795 (times/s), verify: 10839  (times/s).

## **[2019-4-16-16-37]:**

Add function: montg_mul_mod_p(), performance:

| Item                  | Speed            |
| --------------------- | ---------------- |
| **montg_mul_mod_p()** | 61576354 times/s |
| sm2p_mong_mul()       | 24461839 times/s |

## **[2019-4-16-16-46]:**

Add function: montg_sqr_mod_p(), performance:

| Item                  | Speed            |
| --------------------- | ---------------- |
| **montg_sqr_mod_p()** | 75075075 times/s |
| sm2p_mong_sqr()       | 23457658 times/s |

## **[2019-4-16-20-29]:**

Update performance for difference compiler:
| Item                | Sign          | Verify       |
| ------------------- | ------------- | ------------ |
| Visual Studio 2015  | 48104 times/s | 9731 times/s |
| Intel Compiler 18.0 | 39560 times/s | 8903 times/s |

## **[2019-4-16-20-47]:**

Update benchmark result of point operations:

| **项目**   | **YogSM2**   | **Intel IPP** |
| ---------- | ------------ | ------------- |
| 固定点点乘 | 88339 次/s   | 74962 次/s    |
| 不定点点乘 | 11590 次/s   | 14402 次/s    |
| 点加       | 3843197 次/s | 3011141 次/s  |
| 倍点       | 4828585 次/s | 5995203 次/s  |

## **[2019-4-17-16-13]:**

Update benchmark for point add in difference ways:

| Functions                                     | Input parameters         | Speed           |
| --------------------------------------------- | ------------------------ | --------------- |
| add_JPoint                                    | Two JPoint               | 2325581 times/s |
| add_JPoint_and_AFPoint and jacobian_to_affine | Two JPoint               | 260254 times/s  |
| add_JPoint_and_AFPoint                        | One AFPoint + One JPoint | 3753753 times/s |

In YogSM2.



## **[2019-4-18-20-18]:**

Add ML-version of base point multiplication, which  performance has some promotion.

| **项目**   | **YogSM2 原始方案** | **ML-Version** |
| ---------- | ------------------- | -------------- |
| 固定点点乘 | 86088 次/s          | 117329 次/s    |



## **[2019-4-19-12-18]:**

Something odd with YogSM2:

| 项目                                   | 签名       | 验签      |
| -------------------------------------- | ---------- | --------- |
| YogSM2 (NAF in times_point() function) | 57240 次/s | 8681 次/s |
| YogSM2 (BINARY_METHOD in times_point()) | 57940 次/s | 10488 次/s |



## **[2019-4-22-10-50]:**

Update NAF-method for windows equal to 3, and its performance boost about 24% comparing to the original method when windows equal to 5. Comparing to the binary method, its performance boost 14%.
| 项目                                   | 点乘性能 |
| -------------------------------------- | ---------- |
| BINARY Method | 11899 次/s |
| NAF (w = 2) | 13370 次/s |
| NAF (w = 3) | 13646 次/s |
| NAF (w = 4) | 12740 次/s |
| NAF (w = 5) | 9475 次/s |

## **[2019-4-26-19-55]:**

Update inversion method for modulo P operation. and the performance is better than before.

| 求逆方案                      | 耗时 （us） |
| ----------------------------- | ----------- |
| 以前的扩展欧几里得算法        | 2.39 us     |
| 现在的蒙哥马利算法（针对模P） | 1.738 us    |



## **[2019-4-26-20-56]:**

Update inversion method for modulo N operation. and the performance is better than before.

| Item   | Speed             |
| ------ | ----------------- |
| Sign   | **67051** times/s |
| Verify | 11763 times/s     |

## **[2019-5-8-21-57]:**

Update verification procedure, and converting all operation into Montgomery domain when doing scalar multiplication, including point-double and point-add.

| Item   | Speed         |
| ------ | ------------- |
| Verify | 14700 times/s |

## **[2019-5-10-20-02]:**

Update base point multiplication in Montgomery domain, and the performance of sign was boosted.

| Item   | Speed             |
| ------ | ----------------- |
| Sign   | **77339** times/s |
| Verify | 14795 times/s     |

## **[2019-5-23-15-45]:**

Optimize the implementation of low level function, including point-add, point-double and their sub function. We use assembly code to implement those function, and remove the push and pop operation in their sub function. So, their sub function are customized for performance, and you shouldn't call those sub function outside the "ecc_as.asm" file. Because they have no function prologue and epilogue, they looks don't like a normal function.

| Item   | Speed             |
| ------ | ----------------- |
| Sign   | **85222** times/s |
| Verify | 17646 times/s     |

We find that the performance data can be more  high when the test procedure were performing after the machine restart again. The following data is the test result when the target machine reboot again.
| Item   | Speed             |
| ------ | ----------------- |
| Sign   | **90181** times/s |
| Verify | 18392 times/s     |






