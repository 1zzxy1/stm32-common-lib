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

#include "ringbuffer.h"

enum rb_ringbuffer_state rb_ringbuffer_status(struct rb_ringbuffer *rb)
{
    if (rb->read_index == rb->write_index)
    {
        if (rb->read_mirror == rb->write_mirror)
            return RB_RINGBUFFER_EMPTY;
        else
            return RB_RINGBUFFER_FULL;
    }
    return RB_RINGBUFFER_HALFFULL;
}

void rb_ringbuffer_init(struct rb_ringbuffer *rb, rb_uint8_t *pool, rb_int16_t size)
{
    RB_ASSERT(rb != RB_NULL);
    RB_ASSERT(size > 0);

    rb->read_mirror = rb->read_index = 0;
    rb->write_mirror = rb->write_index = 0;

    rb->buffer_ptr = pool;
    rb->buffer_size = RB_ALIGN_DOWN(size, 4);
}

rb_size_t rb_ringbuffer_put(struct rb_ringbuffer *rb, const rb_uint8_t *ptr, rb_uint16_t length)
{
    rb_uint16_t size;

    RB_ASSERT(rb != RB_NULL);

    size = rb_ringbuffer_space_len(rb);

    if (size == 0)
        return 0;

    if (size < length)
        length = size;

    if (rb->buffer_size - rb->write_index > length)
    {
        rb_memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        rb->write_index += length;
        return length;
    }

    rb_memcpy(&rb->buffer_ptr[rb->write_index], &ptr[0], rb->buffer_size - rb->write_index);
    rb_memcpy(&rb->buffer_ptr[0], &ptr[rb->buffer_size - rb->write_index],
              length - (rb->buffer_size - rb->write_index));

    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    return length;
}

rb_size_t rb_ringbuffer_put_force(struct rb_ringbuffer *rb, const rb_uint8_t *ptr, rb_uint16_t length)
{
    rb_uint16_t space_length;

    RB_ASSERT(rb != RB_NULL);

    space_length = rb_ringbuffer_space_len(rb);

    if (length > rb->buffer_size)
    {
        ptr = &ptr[length - rb->buffer_size];
        length = rb->buffer_size;
    }

    if (rb->buffer_size - rb->write_index > length)
    {
        rb_memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        rb->write_index += length;

        if (length > space_length)
            rb->read_index = rb->write_index;

        return length;
    }

    rb_memcpy(&rb->buffer_ptr[rb->write_index], &ptr[0], rb->buffer_size - rb->write_index);
    rb_memcpy(&rb->buffer_ptr[0], &ptr[rb->buffer_size - rb->write_index],
              length - (rb->buffer_size - rb->write_index));

    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    if (length > space_length)
    {
        if (rb->write_index <= rb->read_index)
            rb->read_mirror = ~rb->read_mirror;
        rb->read_index = rb->write_index;
    }

    return length;
}

rb_size_t rb_ringbuffer_get(struct rb_ringbuffer *rb, rb_uint8_t *ptr, rb_uint16_t length)
{
    rb_size_t size;

    RB_ASSERT(rb != RB_NULL);

    size = rb_ringbuffer_data_len(rb);

    if (size == 0)
        return 0;

    if (size < length)
        length = size;

    if (rb->buffer_size - rb->read_index > length)
    {
        rb_memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
        rb->read_index += length;
        return length;
    }

    rb_memcpy(&ptr[0], &rb->buffer_ptr[rb->read_index], rb->buffer_size - rb->read_index);
    rb_memcpy(&ptr[rb->buffer_size - rb->read_index], &rb->buffer_ptr[0],
              length - (rb->buffer_size - rb->read_index));

    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = length - (rb->buffer_size - rb->read_index);

    return length;
}

rb_size_t rb_ringbuffer_peek(struct rb_ringbuffer *rb, rb_uint8_t **ptr)
{
    rb_size_t size;

    RB_ASSERT(rb != RB_NULL);

    *ptr = RB_NULL;

    size = rb_ringbuffer_data_len(rb);

    if (size == 0)
        return 0;

    *ptr = &rb->buffer_ptr[rb->read_index];

    if ((rb_size_t)(rb->buffer_size - rb->read_index) > size)
    {
        rb->read_index += size;
        return size;
    }

    size = rb->buffer_size - rb->read_index;

    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = 0;

    return size;
}

rb_size_t rb_ringbuffer_putchar(struct rb_ringbuffer *rb, const rb_uint8_t ch)
{
    RB_ASSERT(rb != RB_NULL);

    if (!rb_ringbuffer_space_len(rb))
        return 0;

    rb->buffer_ptr[rb->write_index] = ch;

    if (rb->write_index == rb->buffer_size - 1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    }
    else
    {
        rb->write_index++;
    }

    return 1;
}

rb_size_t rb_ringbuffer_putchar_force(struct rb_ringbuffer *rb, const rb_uint8_t ch)
{
    enum rb_ringbuffer_state old_state;

    RB_ASSERT(rb != RB_NULL);

    old_state = rb_ringbuffer_status(rb);

    rb->buffer_ptr[rb->write_index] = ch;

    if (rb->write_index == rb->buffer_size - 1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
        if (old_state == RB_RINGBUFFER_FULL)
        {
            rb->read_mirror = ~rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    }
    else
    {
        rb->write_index++;
        if (old_state == RB_RINGBUFFER_FULL)
            rb->read_index = rb->write_index;
    }

    return 1;
}

rb_size_t rb_ringbuffer_getchar(struct rb_ringbuffer *rb, rb_uint8_t *ch)
{
    RB_ASSERT(rb != RB_NULL);

    if (!rb_ringbuffer_data_len(rb))
        return 0;

    *ch = rb->buffer_ptr[rb->read_index];

    if (rb->read_index == rb->buffer_size - 1)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    }
    else
    {
        rb->read_index++;
    }

    return 1;
}

rb_size_t rb_ringbuffer_data_len(struct rb_ringbuffer *rb)
{
    switch (rb_ringbuffer_status(rb))
    {
    case RB_RINGBUFFER_EMPTY:
        return 0;
    case RB_RINGBUFFER_FULL:
        return rb->buffer_size;
    case RB_RINGBUFFER_HALFFULL:
    default:
    {
        rb_size_t wi = rb->write_index, ri = rb->read_index;

        if (wi > ri)
            return wi - ri;
        else
            return rb->buffer_size - (ri - wi);
    }
    }
}

void rb_ringbuffer_reset(struct rb_ringbuffer *rb)
{
    RB_ASSERT(rb != RB_NULL);

    rb->read_mirror = 0;
    rb->read_index = 0;
    rb->write_mirror = 0;
    rb->write_index = 0;
}
