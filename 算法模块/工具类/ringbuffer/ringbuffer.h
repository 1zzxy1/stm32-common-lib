/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-09-30     Bernard      first version.
 * 2013-05-08     Grissiom     reimplement
 * 2016-08-18     heyuanjie    add interface
 * 2021-07-20     arminker     fix write_index bug
 * 2021-08-14     Jackistang   add comments
 */

#ifndef RINGBUFFER_H__
#define RINGBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

/* 类型定义 */
typedef uint8_t     rb_uint8_t;
typedef uint16_t    rb_uint16_t;
typedef int16_t     rb_int16_t;
typedef size_t      rb_size_t;

/* 宏定义 */
#define RB_ASSERT(x)    assert(x)
#define RB_NULL         NULL
#define rb_memcpy       memcpy

#define RB_ALIGN_DOWN(size, align)  ((size) & ~((align) - 1))

/* 环形缓冲区结构体 */
struct rb_ringbuffer
{
    rb_uint8_t *buffer_ptr;
    rb_uint16_t read_mirror : 1;
    rb_uint16_t read_index : 15;
    rb_uint16_t write_mirror : 1;
    rb_uint16_t write_index : 15;
    rb_int16_t buffer_size;
};

/* 环形缓冲区状态枚举 */
enum rb_ringbuffer_state
{
    RB_RINGBUFFER_EMPTY,
    RB_RINGBUFFER_FULL,
    RB_RINGBUFFER_HALFFULL,
};

/**
 * @brief 初始化环形缓冲区
 * @param rb 环形缓冲区指针
 * @param pool 缓冲区内存
 * @param size 缓冲区大小
 */
void rb_ringbuffer_init(struct rb_ringbuffer *rb, rb_uint8_t *pool, rb_int16_t size);

/**
 * @brief 重置环形缓冲区
 * @param rb 环形缓冲区指针
 */
void rb_ringbuffer_reset(struct rb_ringbuffer *rb);

/**
 * @brief 获取环形缓冲区状态
 * @param rb 环形缓冲区指针
 * @return 缓冲区状态
 */
enum rb_ringbuffer_state rb_ringbuffer_status(struct rb_ringbuffer *rb);

/**
 * @brief 写入数据到环形缓冲区（不覆盖）
 * @param rb 环形缓冲区指针
 * @param ptr 数据指针
 * @param length 数据长度
 * @return 实际写入的字节数
 */
rb_size_t rb_ringbuffer_put(struct rb_ringbuffer *rb, const rb_uint8_t *ptr, rb_uint16_t length);

/**
 * @brief 强制写入数据到环形缓冲区（覆盖旧数据）
 * @param rb 环形缓冲区指针
 * @param ptr 数据指针
 * @param length 数据长度
 * @return 实际写入的字节数
 */
rb_size_t rb_ringbuffer_put_force(struct rb_ringbuffer *rb, const rb_uint8_t *ptr, rb_uint16_t length);

/**
 * @brief 写入单个字节（不覆盖）
 * @param rb 环形缓冲区指针
 * @param ch 字节
 * @return 成功返回1，失败返回0
 */
rb_size_t rb_ringbuffer_putchar(struct rb_ringbuffer *rb, const rb_uint8_t ch);

/**
 * @brief 强制写入单个字节（覆盖旧数据）
 * @param rb 环形缓冲区指针
 * @param ch 字节
 * @return 始终返回1
 */
rb_size_t rb_ringbuffer_putchar_force(struct rb_ringbuffer *rb, const rb_uint8_t ch);

/**
 * @brief 从环形缓冲区读取数据
 * @param rb 环形缓冲区指针
 * @param ptr 目标缓冲区
 * @param length 读取长度
 * @return 实际读取的字节数
 */
rb_size_t rb_ringbuffer_get(struct rb_ringbuffer *rb, rb_uint8_t *ptr, rb_uint16_t length);

/**
 * @brief 获取可读数据的指针
 * @param rb 环形缓冲区指针
 * @param ptr 返回的数据指针
 * @return 可连续读取的字节数
 */
rb_size_t rb_ringbuffer_peek(struct rb_ringbuffer *rb, rb_uint8_t **ptr);

/**
 * @brief 读取单个字节
 * @param rb 环形缓冲区指针
 * @param ch 目标字节指针
 * @return 成功返回1，失败返回0
 */
rb_size_t rb_ringbuffer_getchar(struct rb_ringbuffer *rb, rb_uint8_t *ch);

/**
 * @brief 获取缓冲区中数据长度
 * @param rb 环形缓冲区指针
 * @return 数据长度
 */
rb_size_t rb_ringbuffer_data_len(struct rb_ringbuffer *rb);

/**
 * @brief 获取缓冲区大小
 * @param rb 环形缓冲区指针
 * @return 缓冲区大小
 */
static inline rb_uint16_t rb_ringbuffer_get_size(struct rb_ringbuffer *rb)
{
    RB_ASSERT(rb != RB_NULL);
    return rb->buffer_size;
}

/**
 * @brief 获取缓冲区剩余空间
 */
#define rb_ringbuffer_space_len(rb) ((rb)->buffer_size - rb_ringbuffer_data_len(rb))

#ifdef __cplusplus
}
#endif

#endif /* RINGBUFFER_H__ */
