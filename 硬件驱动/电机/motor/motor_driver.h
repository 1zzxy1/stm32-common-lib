/**
 ******************************************************************************
 * @file    motor_driver.h
 * @brief   电机驱动库 - 支持H桥驱动芯片 (A4950/TB6612等)
 * @version 1.0.0
 ******************************************************************************
 * @attention
 *
 * 基于STM32 HAL库实现
 * 支持PWM+方向控制模式
 * 适用于A4950、TB6612等H桥驱动芯片
 *
 ******************************************************************************
 */

#ifndef _MOTOR_DRIVER_H_
#define _MOTOR_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 需要包含STM32 HAL库头文件 */
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* 配置选项 */
#ifndef MOTOR_SPEED_MAX
#define MOTOR_SPEED_MAX         100     /* 最大速度 (百分比) */
#endif

#ifndef MOTOR_SPEED_MIN
#define MOTOR_SPEED_MIN         -100    /* 最小速度 (百分比) */
#endif

#ifndef MOTOR_PWM_PERIOD
#define MOTOR_PWM_PERIOD        99      /* PWM周期 (ARR值) */
#endif

#ifndef MOTOR_MIN_PWM_THRESHOLD
#define MOTOR_MIN_PWM_THRESHOLD 0       /* 最小PWM阈值 (死区补偿) */
#endif

/* 电机状态枚举 */
typedef enum {
    MOTOR_STATE_STOP = 0,     /* 停止 */
    MOTOR_STATE_FORWARD,      /* 正转 */
    MOTOR_STATE_BACKWARD,     /* 反转 */
    MOTOR_STATE_ERROR         /* 错误 */
} motor_state_t;

/* 电机硬件配置结构体 */
typedef struct {
    TIM_HandleTypeDef *htim;        /* PWM定时器句柄 */
    uint32_t channel;               /* PWM通道 */
    GPIO_TypeDef *dir_port;         /* 方向控制GPIO端口 */
    uint16_t dir_pin;               /* 方向控制GPIO引脚 */
} motor_hw_t;

/* 电机驱动结构体 */
typedef struct {
    motor_hw_t hw;                  /* 硬件配置 */
    int8_t speed;                   /* 当前速度 (-100 到 +100) */
    motor_state_t state;            /* 当前状态 */
    uint8_t enable;                 /* 使能标志 */
    uint8_t reverse;                /* 电机安装方向 (0-正装, 1-反装) */
} motor_t;

/**
 * @brief 创建电机实例
 * @param motor: 电机实例指针
 * @param htim: PWM定时器句柄
 * @param channel: PWM通道 (TIM_CHANNEL_1~4)
 * @param dir_port: 方向控制GPIO端口
 * @param dir_pin: 方向控制GPIO引脚
 * @param reverse: 电机安装方向 (0-正装, 1-反装)
 * @retval 0: 成功, -1: 参数错误
 */
int8_t motor_create(motor_t *motor,
                    TIM_HandleTypeDef *htim,
                    uint32_t channel,
                    GPIO_TypeDef *dir_port,
                    uint16_t dir_pin,
                    uint8_t reverse);

/**
 * @brief 设置电机速度
 * @param motor: 电机实例指针
 * @param speed: 速度值 (-100 到 +100)，正数正转，负数反转，0停止
 * @retval 0: 成功, -1: 参数错误
 */
int8_t motor_set_speed(motor_t *motor, int8_t speed);

/**
 * @brief 停止电机
 * @param motor: 电机实例指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t motor_stop(motor_t *motor);

/**
 * @brief 获取电机状态
 * @param motor: 电机实例指针
 * @retval 电机状态
 */
motor_state_t motor_get_state(motor_t *motor);

/**
 * @brief 获取电机当前速度
 * @param motor: 电机实例指针
 * @retval 当前速度值
 */
int8_t motor_get_speed(motor_t *motor);

/**
 * @brief 使能/失能电机
 * @param motor: 电机实例指针
 * @param enable: 1-使能, 0-失能
 * @retval 0: 成功, -1: 参数错误
 */
int8_t motor_enable(motor_t *motor, uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif /* _MOTOR_DRIVER_H_ */
