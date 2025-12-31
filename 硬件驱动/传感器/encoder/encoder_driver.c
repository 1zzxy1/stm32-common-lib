/**
 ******************************************************************************
 * @file    encoder_driver.c
 * @brief   编码器驱动库 - 支持正交编码器测速
 * @version 1.0.0
 ******************************************************************************
 */

#include "encoder_driver.h"

/**
 * @brief 初始化编码器
 */
int8_t encoder_init(encoder_t *encoder, TIM_HandleTypeDef *htim, uint8_t reverse)
{
    if (encoder == NULL || htim == NULL) {
        return -1;
    }

    encoder->htim = htim;
    encoder->reverse = reverse;

    /* 启动定时器编码器模式 */
    HAL_TIM_Encoder_Start(encoder->htim, TIM_CHANNEL_ALL);

    /* 清零计数器 */
    __HAL_TIM_SetCounter(encoder->htim, 0);

    /* 初始化数据 */
    encoder->count = 0;
    encoder->total_count = 0;
    encoder->speed_mm_s = 0.0f;
    encoder->speed_rpm = 0.0f;

    return 0;
}

/**
 * @brief 更新编码器数据
 */
void encoder_update(encoder_t *encoder)
{
    if (encoder == NULL || encoder->htim == NULL) {
        return;
    }

    /* 读取计数值 */
    encoder->count = (int16_t)__HAL_TIM_GetCounter(encoder->htim);

    /* 处理方向反转 */
    if (encoder->reverse) {
        encoder->count = -encoder->count;
    }

    /* 清零硬件计数器 */
    __HAL_TIM_SetCounter(encoder->htim, 0);

    /* 累计总数 */
    encoder->total_count += encoder->count;

    /* 计算速度 (mm/s) */
    /* 速度 = (计数值 / 总PPR) * 周长 / 采样时间 */
    encoder->speed_mm_s = (float)encoder->count / ENCODER_TOTAL_PPR
                          * ENCODER_WHEEL_CIRCUMFERENCE_MM / ENCODER_SAMPLE_TIME_S;

    /* 计算转速 (RPM) */
    /* RPM = (计数值 / 总PPR) / 采样时间(秒) * 60 */
    encoder->speed_rpm = (float)encoder->count / ENCODER_TOTAL_PPR
                         / ENCODER_SAMPLE_TIME_S * 60.0f;
}

/**
 * @brief 获取当前速度 (mm/s)
 */
float encoder_get_speed_mm_s(encoder_t *encoder)
{
    if (encoder == NULL) {
        return 0.0f;
    }
    return encoder->speed_mm_s;
}

/**
 * @brief 获取当前速度 (cm/s)
 */
float encoder_get_speed_cm_s(encoder_t *encoder)
{
    if (encoder == NULL) {
        return 0.0f;
    }
    return encoder->speed_mm_s / 10.0f;
}

/**
 * @brief 获取当前转速 (RPM)
 */
float encoder_get_speed_rpm(encoder_t *encoder)
{
    if (encoder == NULL) {
        return 0.0f;
    }
    return encoder->speed_rpm;
}

/**
 * @brief 获取累计计数值
 */
int32_t encoder_get_total_count(encoder_t *encoder)
{
    if (encoder == NULL) {
        return 0;
    }
    return encoder->total_count;
}

/**
 * @brief 获取累计距离 (mm)
 */
float encoder_get_distance_mm(encoder_t *encoder)
{
    if (encoder == NULL) {
        return 0.0f;
    }
    return (float)encoder->total_count / ENCODER_TOTAL_PPR * ENCODER_WHEEL_CIRCUMFERENCE_MM;
}

/**
 * @brief 重置累计计数
 */
void encoder_reset(encoder_t *encoder)
{
    if (encoder == NULL) {
        return;
    }

    encoder->count = 0;
    encoder->total_count = 0;
    encoder->speed_mm_s = 0.0f;
    encoder->speed_rpm = 0.0f;

    if (encoder->htim != NULL) {
        __HAL_TIM_SetCounter(encoder->htim, 0);
    }
}
