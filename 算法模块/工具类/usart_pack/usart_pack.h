/**
 ******************************************************************************
 * @file    usart_pack.h
 * @brief   通用串口数据包协议 - 支持多种数据类型的灵活打包/解包
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * 基于模板的串口通信协议模块
 * 支持 BYTE/SHORT/INT/FLOAT 等数据类型的灵活组合
 * 帧格式: [帧头] [数据区] [校验和] [帧尾]
 *
 ******************************************************************************
 */

#ifndef _USART_PACK_H_
#define _USART_PACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* 配置选项 */
#ifndef USART_PACK_MAX_VARIABLES
#define USART_PACK_MAX_VARIABLES 16  /* 模板中最大变量数 */
#endif

#ifndef USART_PACK_HEADER
#define USART_PACK_HEADER 0xA5       /* 帧头 */
#endif

#ifndef USART_PACK_TAIL
#define USART_PACK_TAIL 0x5A         /* 帧尾 */
#endif

/* 最小帧长度: 帧头(1) + 校验和(1) + 帧尾(1) */
#define USART_PACK_MIN_FRAME_LEN 3

/* 数据类型枚举 */
typedef enum {
    PACK_TYPE_BYTE,     /* 1字节 (uint8_t / int8_t) */
    PACK_TYPE_SHORT,    /* 2字节 (uint16_t / int16_t) */
    PACK_TYPE_INT,      /* 4字节 (uint32_t / int32_t) */
    PACK_TYPE_FLOAT     /* 4字节浮点数 (float) */
} usart_pack_type_t;

/* 协议实例结构体 */
typedef struct {
    usart_pack_type_t types[USART_PACK_MAX_VARIABLES];  /* 类型模板 */
    void *vars[USART_PACK_MAX_VARIABLES];               /* 变量地址映射 */
    uint16_t count;                                      /* 变量数量 */
    uint8_t header;                                      /* 帧头 */
    uint8_t tail;                                        /* 帧尾 */
} usart_pack_t;

/**
 * @brief 初始化协议实例
 * @param pack: 协议实例指针
 */
void usart_pack_init(usart_pack_t *pack);

/**
 * @brief 初始化协议实例 (自定义帧头帧尾)
 * @param pack: 协议实例指针
 * @param header: 自定义帧头
 * @param tail: 自定义帧尾
 */
void usart_pack_init_custom(usart_pack_t *pack, uint8_t header, uint8_t tail);

/**
 * @brief 设置解析/打包模板
 * @param pack: 协议实例指针
 * @param types: 数据类型数组
 * @param vars: 变量地址数组
 * @param count: 变量数量
 * @retval 0: 成功, -1: 失败
 */
int usart_pack_set_template(usart_pack_t *pack,
                            usart_pack_type_t *types,
                            void **vars,
                            uint16_t count);

/**
 * @brief 添加单个变量到模板
 * @param pack: 协议实例指针
 * @param type: 数据类型
 * @param var: 变量地址
 * @retval 0: 成功, -1: 失败
 */
int usart_pack_add_var(usart_pack_t *pack, usart_pack_type_t type, void *var);

/**
 * @brief 清空模板
 * @param pack: 协议实例指针
 */
void usart_pack_clear(usart_pack_t *pack);

/**
 * @brief 解析接收到的数据帧
 * @param pack: 协议实例指针
 * @param buffer: 接收缓冲区
 * @param length: 数据长度
 * @retval 0: 成功, -1: 帧格式错误, -2: 校验和错误
 */
int usart_pack_parse(usart_pack_t *pack, uint8_t *buffer, uint16_t length);

/**
 * @brief 打包数据帧
 * @param pack: 协议实例指针
 * @param buffer: 输出缓冲区
 * @param max_len: 缓冲区最大长度
 * @retval 实际帧长度, 0表示失败
 */
uint16_t usart_pack_build(usart_pack_t *pack, uint8_t *buffer, uint16_t max_len);

/**
 * @brief 计算数据区所需字节数
 * @param pack: 协议实例指针
 * @retval 数据区字节数
 */
uint16_t usart_pack_calc_data_size(usart_pack_t *pack);

/**
 * @brief 计算完整帧所需字节数
 * @param pack: 协议实例指针
 * @retval 完整帧字节数 (数据区 + 帧头 + 校验和 + 帧尾)
 */
uint16_t usart_pack_calc_frame_size(usart_pack_t *pack);

#ifdef __cplusplus
}
#endif

#endif /* _USART_PACK_H_ */
