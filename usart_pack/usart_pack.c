/**
 ******************************************************************************
 * @file    usart_pack.c
 * @brief   通用串口数据包协议实现
 * @version 1.0.0
 ******************************************************************************
 */

#include "usart_pack.h"
#include <string.h>

/**
 * @brief 初始化协议实例
 */
void usart_pack_init(usart_pack_t *pack)
{
    if (pack == NULL) {
        return;
    }

    pack->count = 0;
    pack->header = USART_PACK_HEADER;
    pack->tail = USART_PACK_TAIL;

    for (int i = 0; i < USART_PACK_MAX_VARIABLES; i++) {
        pack->types[i] = PACK_TYPE_BYTE;
        pack->vars[i] = NULL;
    }
}

/**
 * @brief 初始化协议实例 (自定义帧头帧尾)
 */
void usart_pack_init_custom(usart_pack_t *pack, uint8_t header, uint8_t tail)
{
    usart_pack_init(pack);
    if (pack != NULL) {
        pack->header = header;
        pack->tail = tail;
    }
}

/**
 * @brief 设置解析/打包模板
 */
int usart_pack_set_template(usart_pack_t *pack,
                            usart_pack_type_t *types,
                            void **vars,
                            uint16_t count)
{
    if (pack == NULL || types == NULL || vars == NULL) {
        return -1;
    }

    if (count > USART_PACK_MAX_VARIABLES) {
        return -1;
    }

    for (uint16_t i = 0; i < count; i++) {
        pack->types[i] = types[i];
        pack->vars[i] = vars[i];
    }

    pack->count = count;
    return 0;
}

/**
 * @brief 添加单个变量到模板
 */
int usart_pack_add_var(usart_pack_t *pack, usart_pack_type_t type, void *var)
{
    if (pack == NULL || var == NULL) {
        return -1;
    }

    if (pack->count >= USART_PACK_MAX_VARIABLES) {
        return -1;
    }

    pack->types[pack->count] = type;
    pack->vars[pack->count] = var;
    pack->count++;

    return 0;
}

/**
 * @brief 清空模板
 */
void usart_pack_clear(usart_pack_t *pack)
{
    if (pack == NULL) {
        return;
    }
    pack->count = 0;
}

/**
 * @brief 计算数据区所需字节数
 */
uint16_t usart_pack_calc_data_size(usart_pack_t *pack)
{
    if (pack == NULL) {
        return 0;
    }

    uint16_t size = 0;
    for (uint16_t i = 0; i < pack->count; i++) {
        switch (pack->types[i]) {
            case PACK_TYPE_BYTE:  size += 1; break;
            case PACK_TYPE_SHORT: size += 2; break;
            case PACK_TYPE_INT:   size += 4; break;
            case PACK_TYPE_FLOAT: size += 4; break;
        }
    }
    return size;
}

/**
 * @brief 计算完整帧所需字节数
 */
uint16_t usart_pack_calc_frame_size(usart_pack_t *pack)
{
    return usart_pack_calc_data_size(pack) + USART_PACK_MIN_FRAME_LEN;
}

/**
 * @brief 解析接收到的数据帧
 */
int usart_pack_parse(usart_pack_t *pack, uint8_t *buffer, uint16_t length)
{
    if (pack == NULL || buffer == NULL) {
        return -1;
    }

    /* 检查最小长度 */
    if (length < USART_PACK_MIN_FRAME_LEN) {
        return -1;
    }

    /* 检查帧头帧尾 */
    if (buffer[0] != pack->header || buffer[length - 1] != pack->tail) {
        return -1;
    }

    /* 校验和验证 */
    uint16_t data_len = length - USART_PACK_MIN_FRAME_LEN;
    uint8_t checksum = 0;
    for (uint16_t i = 1; i <= data_len; i++) {
        checksum += buffer[i];
    }
    if ((checksum & 0xFF) != buffer[length - 2]) {
        return -2;
    }

    /* 解析数据区 */
    uint16_t idx = 1;  /* 跳过帧头 */
    for (uint16_t i = 0; i < pack->count && idx < length - 2; i++) {
        switch (pack->types[i]) {
            case PACK_TYPE_BYTE:
            {
                uint8_t *var = (uint8_t *)pack->vars[i];
                *var = buffer[idx++];
                break;
            }
            case PACK_TYPE_SHORT:
            {
                if (idx + 1 >= length - 2) return -1;
                uint16_t *var = (uint16_t *)pack->vars[i];
                /* 大端模式 */
                *var = (buffer[idx] << 8) | buffer[idx + 1];
                idx += 2;
                break;
            }
            case PACK_TYPE_INT:
            {
                if (idx + 3 >= length - 2) return -1;
                uint32_t *var = (uint32_t *)pack->vars[i];
                /* 大端模式 */
                *var = ((uint32_t)buffer[idx] << 24) |
                       ((uint32_t)buffer[idx + 1] << 16) |
                       ((uint32_t)buffer[idx + 2] << 8) |
                       buffer[idx + 3];
                idx += 4;
                break;
            }
            case PACK_TYPE_FLOAT:
            {
                if (idx + 3 >= length - 2) return -1;
                float *var = (float *)pack->vars[i];
                /* 直接内存拷贝 (小端模式) */
                memcpy(var, &buffer[idx], 4);
                idx += 4;
                break;
            }
        }
    }

    return 0;
}

/**
 * @brief 打包数据帧
 */
uint16_t usart_pack_build(usart_pack_t *pack, uint8_t *buffer, uint16_t max_len)
{
    if (pack == NULL || buffer == NULL) {
        return 0;
    }

    uint16_t frame_size = usart_pack_calc_frame_size(pack);
    if (max_len < frame_size) {
        return 0;
    }

    uint16_t idx = 0;

    /* 帧头 */
    buffer[idx++] = pack->header;

    /* 数据区 */
    for (uint16_t i = 0; i < pack->count; i++) {
        switch (pack->types[i]) {
            case PACK_TYPE_BYTE:
            {
                uint8_t *var = (uint8_t *)pack->vars[i];
                buffer[idx++] = *var;
                break;
            }
            case PACK_TYPE_SHORT:
            {
                uint16_t *var = (uint16_t *)pack->vars[i];
                /* 大端模式 */
                buffer[idx++] = (*var >> 8) & 0xFF;
                buffer[idx++] = *var & 0xFF;
                break;
            }
            case PACK_TYPE_INT:
            {
                uint32_t *var = (uint32_t *)pack->vars[i];
                /* 大端模式 */
                buffer[idx++] = (*var >> 24) & 0xFF;
                buffer[idx++] = (*var >> 16) & 0xFF;
                buffer[idx++] = (*var >> 8) & 0xFF;
                buffer[idx++] = *var & 0xFF;
                break;
            }
            case PACK_TYPE_FLOAT:
            {
                float *var = (float *)pack->vars[i];
                /* 直接内存拷贝 (小端模式) */
                uint8_t *bytes = (uint8_t *)var;
                buffer[idx++] = bytes[0];
                buffer[idx++] = bytes[1];
                buffer[idx++] = bytes[2];
                buffer[idx++] = bytes[3];
                break;
            }
        }
    }

    /* 校验和 */
    uint8_t checksum = 0;
    for (uint16_t i = 1; i < idx; i++) {
        checksum += buffer[i];
    }
    buffer[idx++] = checksum & 0xFF;

    /* 帧尾 */
    buffer[idx++] = pack->tail;

    return idx;
}
