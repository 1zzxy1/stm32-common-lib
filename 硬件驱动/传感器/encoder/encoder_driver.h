/**
 ******************************************************************************
 * @file    encoder_driver.h
 * @brief   编码器驱动库 - 支持正交编码器测速
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * 基于STM32 HAL库的定时器编码器模式
 * 支持速度测量和位置累计
 * 支持编码器反向配置
 *
 ******************************************************************************
 */

#ifndef _ENCODER_DRIVER_H_
#define _ENCODER_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 需要包含STM32 HAL库头文件 */
#include "main.h"
#include "tim.h"

#include <stdint.h>

/* 配置选项 - 用户可根据实际情况修改 */
#ifndef ENCODER_PPR
#define ENCODER_PPR (500 * 4)       /* 编码器每转脉冲数 (线数 * 4倍频) */
#endif

#ifndef ENCODER_GEAR_RATIO
#define ENCODER_GEAR_RATIO 1        /* 减速比 (如 1:20 则填 20) */
#endif

#ifndef ENCODER_WHEEL_DIAMETER_MM
#define ENCODER_WHEEL_DIAMETER_MM 65.0f  /* 轮子直径 (毫米) */
#endif

#ifndef ENCODER_SAMPLE_TIME_MS
#define ENCODER_SAMPLE_TIME_MS 10   /* 采样周期 (毫秒) */
#endif

/* 自动计算常量 */
#define ENCODER_PI 3.14159265f
#define ENCODER_TOTAL_PPR (ENCODER_PPR * ENCODER_GEAR_RATIO)
#define ENCODER_WHEEL_CIRCUMFERENCE_MM (ENCODER_WHEEL_DIAMETER_MM * ENCODER_PI)
#define ENCODER_SAMPLE_TIME_S (ENCODER_SAMPLE_TIME_MS / 1000.0f)

/* 编码器结构体 */
typedef struct {
    TIM_HandleTypeDef *htim;    /* 定时器句柄 */
    uint8_t reverse;            /* 编码器方向反转 (0-正常, 1-反转) */
    int16_t count;              /* 当前采样周期内的计数值 */
    int32_t total_count;        /* 累计总计数值 */
    float speed_mm_s;           /* 速度 (mm/s) */
    float speed_rpm;            /* 转速 (RPM) */
} encoder_t;

/**
 * @brief 初始化编码器
 * @param encoder: 编码器实例指针
 * @param htim: 定时器句柄 (需配置为编码器模式)
 * @param reverse: 方向反转 (0-正常, 1-反转)
 * @retval 0: 成功, -1: 参数错误
 */
int8_t encoder_init(encoder_t *encoder, TIM_HandleTypeDef *htim, uint8_t reverse);

/**
 * @brief 更新编码器数据 (需周期性调用，周期与ENCODER_SAMPLE_TIME_MS一致)
 * @param encoder: 编码器实例指针
 */
void encoder_update(encoder_t *encoder);

/**
 * @brief 获取当前速度 (mm/s)
 * @param encoder: 编码器实例指针
 * @retval 速度值 (mm/s)
 */
float encoder_get_speed_mm_s(encoder_t *encoder);

/**
 * @brief 获取当前速度 (cm/s)
 * @param encoder: 编码器实例指针
 * @retval 速度值 (cm/s)
 */
float encoder_get_speed_cm_s(encoder_t *encoder);

/**
 * @brief 获取当前转速 (RPM)
 * @param encoder: 编码器实例指针
 * @retval 转速值 (RPM)
 */
float encoder_get_speed_rpm(encoder_t *encoder);

/**
 * @brief 获取累计计数值
 * @param encoder: 编码器实例指针
 * @retval 累计计数值
 */
int32_t encoder_get_total_count(encoder_t *encoder);

/**
 * @brief 获取累计距离 (mm)
 * @param encoder: 编码器实例指针
 * @retval 累计距离 (mm)
 */
float encoder_get_distance_mm(encoder_t *encoder);

/**
 * @brief 重置累计计数
 * @param encoder: 编码器实例指针
 */
void encoder_reset(encoder_t *encoder);

#ifdef __cplusplus
}
#endif

#endif /* _ENCODER_DRIVER_H_ */
