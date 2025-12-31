/**
 ******************************************************************************
 * @file    bit_array.h
 * @brief   位数组操作库 - 纯头文件实现
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * 支持位操作、逻辑运算、移位、复制等功能
 * 纯C实现，无硬件依赖
 * Header-only库，直接包含即可使用
 *
 ******************************************************************************
 */

#ifndef _BIT_ARRAY_H_
#define _BIT_ARRAY_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 64位系统可定义 BIT_ARRAY_CONFIG_64 */
#ifdef BIT_ARRAY_CONFIG_64
typedef uint64_t bit_array_t;
#define BIT_ARRAY_BIT(n) (1ULL << (n))
#else
typedef uint32_t bit_array_t;
#define BIT_ARRAY_BIT(n) (1UL << (n))
#endif
typedef bit_array_t bit_array_val_t;

#define BIT_ARRAY_BITS (sizeof(bit_array_val_t) * 8)
#define BIT_ARRAY_BIT_WORD(bit)  ((bit) / BIT_ARRAY_BITS)
#define BIT_ARRAY_BIT_INDEX(bit) ((bit_array_val_t)(bit) & (BIT_ARRAY_BITS - 1U))
#define BIT_ARRAY_MASK(bit)       BIT_ARRAY_BIT(BIT_ARRAY_BIT_INDEX(bit))
#define BIT_ARRAY_ELEM(addr, bit) ((addr)[BIT_ARRAY_BIT_WORD(bit)])
#define BIT_ARRAY_WORD_MAX (~(bit_array_val_t)0)
#define BIT_ARRAY_SUB_MASK(nbits) ((nbits) ? BIT_ARRAY_WORD_MAX >> (BIT_ARRAY_BITS - (nbits)) : (bit_array_val_t)0)
#define bitmask_merge(a, b, abits) (b ^ ((a ^ b) & abits))
#define BIT_ARRAY_BITMAP_SIZE(num_bits) (1 + ((num_bits)-1) / BIT_ARRAY_BITS)
#define BIT_ARRAY_DEFINE(name, num_bits) bit_array_t name[BIT_ARRAY_BITMAP_SIZE(num_bits)]

/* Popcount实现 */
static inline bit_array_val_t _bit_array_popcount(bit_array_val_t w)
{
    w = w - ((w >> 1) & (bit_array_val_t) ~(bit_array_val_t)0 / 3);
    w = (w & (bit_array_val_t) ~(bit_array_val_t)0 / 15 * 3) + ((w >> 2) & (bit_array_val_t) ~(bit_array_val_t)0 / 15 * 3);
    w = (w + (w >> 4)) & (bit_array_val_t) ~(bit_array_val_t)0 / 255 * 15;
    return (bit_array_val_t)(w * ((bit_array_val_t) ~(bit_array_val_t)0 / 255)) >> (sizeof(bit_array_val_t) - 1) * 8;
}

#define POPCOUNT(x) _bit_array_popcount(x)
#define bits_in_top_word(nbits) ((nbits) ? BIT_ARRAY_BIT_INDEX((nbits)-1) + 1 : 0)

static inline void _bit_array_mask_top_word(bit_array_t *target, int num_bits)
{
    int num_of_words = BIT_ARRAY_BITMAP_SIZE(num_bits);
    int bits_active = bits_in_top_word(num_bits);
    target[num_of_words - 1] &= BIT_ARRAY_SUB_MASK(bits_active);
}

/* 基本位操作 */
static inline int bit_array_get(const bit_array_t *target, int bit)
{
    bit_array_val_t val = BIT_ARRAY_ELEM(target, bit);
    return (1 & (val >> (bit & (BIT_ARRAY_BITS - 1)))) != 0;
}

static inline void bit_array_clear(bit_array_t *target, int bit)
{
    bit_array_val_t mask = BIT_ARRAY_MASK(bit);
    BIT_ARRAY_ELEM(target, bit) &= ~mask;
}

static inline void bit_array_set(bit_array_t *target, int bit)
{
    bit_array_val_t mask = BIT_ARRAY_MASK(bit);
    BIT_ARRAY_ELEM(target, bit) |= mask;
}

static inline void bit_array_toggle(bit_array_t *target, int bit)
{
    bit_array_val_t mask = BIT_ARRAY_MASK(bit);
    BIT_ARRAY_ELEM(target, bit) ^= mask;
}

static inline void bit_array_assign(bit_array_t *target, int bit, int val)
{
    bit_array_val_t mask = BIT_ARRAY_MASK(bit);
    if (val)
        BIT_ARRAY_ELEM(target, bit) |= mask;
    else
        BIT_ARRAY_ELEM(target, bit) &= ~mask;
}

/* 批量操作 */
static inline void bit_array_clear_all(bit_array_t *target, int num_bits)
{
    memset((void *)target, 0, BIT_ARRAY_BITMAP_SIZE(num_bits) * sizeof(bit_array_val_t));
}

static inline void bit_array_set_all(bit_array_t *target, int num_bits)
{
    memset((void *)target, 0xff, BIT_ARRAY_BITMAP_SIZE(num_bits) * sizeof(bit_array_val_t));
    _bit_array_mask_top_word(target, num_bits);
}

static inline void bit_array_toggle_all(bit_array_t *target, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
        target[i] ^= BIT_ARRAY_WORD_MAX;
    _bit_array_mask_top_word(target, num_bits);
}

/* 统计函数 */
static inline int bit_array_num_bits_set(bit_array_t *target, int num_bits)
{
    int num_of_bits_set = 0;
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
        if (target[i] > 0)
            num_of_bits_set += POPCOUNT(target[i]);
    return num_of_bits_set;
}

static inline int bit_array_num_bits_cleared(bit_array_t *target, int num_bits)
{
    return num_bits - bit_array_num_bits_set(target, num_bits);
}

/* 复制函数 */
static inline void bit_array_copy_all(bit_array_t *dst, const bit_array_t *src, int num_bits)
{
    memcpy(dst, src, BIT_ARRAY_BITMAP_SIZE(num_bits) * sizeof(bit_array_val_t));
}

/* 逻辑运算 */
static inline void bit_array_and(bit_array_t *dest, const bit_array_t *src1, const bit_array_t *src2, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
        dest[i] = src1[i] & src2[i];
}

static inline void bit_array_or(bit_array_t *dest, const bit_array_t *src1, const bit_array_t *src2, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
        dest[i] = src1[i] | src2[i];
}

static inline void bit_array_xor(bit_array_t *dest, const bit_array_t *src1, const bit_array_t *src2, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
        dest[i] = src1[i] ^ src2[i];
}

static inline void bit_array_not(bit_array_t *dest, const bit_array_t *src, int num_bits)
{
    for (int i = 0; i < BIT_ARRAY_BITMAP_SIZE(num_bits); i++)
        dest[i] = ~src[i];
    _bit_array_mask_top_word(dest, num_bits);
}

/* 比较函数 */
static inline int bit_array_cmp(const bit_array_t *bitarr1, const bit_array_t *bitarr2, int num_bits)
{
    return memcmp(bitarr1, bitarr2, BIT_ARRAY_BITMAP_SIZE(num_bits) * sizeof(bit_array_val_t));
}

/* 字符串转换 */
static inline char *bit_array_to_str(const bit_array_t *bitarr, int num_bits, char *str)
{
    for (int i = 0; i < num_bits; i++)
        str[i] = bit_array_get(bitarr, i) ? '1' : '0';
    str[num_bits] = '\0';
    return str;
}

#ifdef __cplusplus
}
#endif

#endif /* _BIT_ARRAY_H_ */
