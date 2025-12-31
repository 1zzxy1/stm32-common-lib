

/**
 ******************************************************************************
 * @file    motor_driver.h
 * @brief   TB6612 电机驱动库头文件 (通用映射版)
 * @author  (你的名字)
 * @date    2025年07月08日
 ******************************************************************************
 * @attention
 *
 * 1. 本库专为TB6612FNG电机驱动芯片设计。
 * 2. 采用“速度映射”思想，对外接口使用百分比速度
 * 对内自动映射为硬件PWM值，具有高通用性。
 * 3. 支持双电机独立控制。
 *
 ******************************************************************************
 */

#ifndef __MOTOR_DRIVER_TB6612_H__
#define __MOTOR_DRIVER_TB6612_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
// 根据你的工程包含必要的HAL库头文件
#include "bsp_system.h"

/* Exported constants --------------------------------------------------------*/

//=============================================================================
//=====                  用户核心配置区域 (User Core Configuration)         =====
//=============================================================================

/**
 * @brief 物理PWM最大计数值 (与硬件定时器配置强相关)
 * @note  该值应为你的定时器ARR(Auto-Reload Register)值 + 1。
 * 例如，如果你的 `htim.Init.Period` 设置为 999, 此处应设置为 1000。
 */
#define MOTOR_PWM_MAX_PULSE 100

/**
 * @brief 最小物理PWM启动阈值 (单位：PWM计数值)
 * @note  用于解决电机在PWM占空比过低时无法启动的问题。
 * 如果计算出的PWM值大于0但小于此阈值，则会被强制提升到此阈值。
 * 如果不需要此功能，可以将其设置为 0。
 */
#define MOTOR_MIN_PHYSICAL_PWM 0

/**
 * @brief TB6612 待机(Standby)引脚定义
 */
#define TB6612_STBY_PORT GPIOB
#define TB6612_STBY_PIN GPIO_PIN_10

//=============================================================================
//=====                  库内部宏定义 (Library Internal Macros)             =====
//=============================================================================

/**
 * @brief 逻辑速度范围 (提供给用户的接口)，固定为百分比
 * @note
 */
#define MOTOR_LOGIC_SPEED_MAX 100  // 逻辑最大正转速度
#define MOTOR_LOGIC_SPEED_MIN -100 // 逻辑最大反转速度

    /* Exported types ------------------------------------------------------------*/

    /**
     * @brief 电机状态枚举
     */
    typedef enum
    {
        MOTOR_STATE_STOP = 0, // 停止/刹车
        MOTOR_STATE_FORWARD,  // 正转
        MOTOR_STATE_BACKWARD, // 反转
        MOTOR_STATE_ERROR     // 错误
    } MotorState_t;

    /**
     * @brief 电机硬件配置结构体
     */
    typedef struct
    {
        TIM_HandleTypeDef *htim; // PWM定时器句柄
        uint32_t channel;        // PWM通道 (TIM_CHANNEL_x)

        GPIO_TypeDef *in1_port; // 方向控制引脚1 端口
        uint16_t in1_pin;       // 方向控制引脚1 管脚

        GPIO_TypeDef *in2_port; // 方向控制引脚2 端口
        uint16_t in2_pin;       // 方向控制引脚2 管脚
    } MotorHW_t;

    /**
     * @brief 电机驱动实体结构体
     */
    typedef struct
    {
        MotorHW_t hw;       // 硬件配置
        int16_t speed;      // 当前速度 (-100% ~ +100%)
        MotorState_t state; // 当前状态
        uint8_t enable;     // 软件使能标志 (1:使能, 0:失能)
        uint8_t reverse;    // 电机安装方向 (0:正装, 1:反装)
    } Motor_t;

    /* Exported functions prototypes ---------------------------------------------*/

    /**
     * @brief 初始化TB6612芯片，控制其STBY引脚。
     * @param enable: 1-使能芯片(退出待机), 0-让芯片进入待机模式。
     */
    void TB6612_Init(uint8_t enable);

    /**
     * @brief 创建并初始化一个电机实例。
     * @param motor:    指向要初始化的电机实体结构体的指针。
     * @param htim:     控制该电机的PWM定时器句柄。
     * @param channel:  PWM通道 (例如 TIM_CHANNEL_1)。
     * @param in1_port: 方向引脚1所在的GPIO端口 (例如 GPIOE)。
     * @param in1_pin:  方向引脚1的管脚号 (例如 GPIO_PIN_9)。
     * @param in2_port: 方向引脚2所在的GPIO端口。
     * @param in2_pin:  方向引脚2的管脚号。
     * @param reverse:  电机安装方向 (0 表示正装, 1 表示反装，反装时速度会自动取反)。
     * @retval 0: 成功, -1: 传入参数错误。
     */
    int8_t Motor_Create(Motor_t *motor,
                        TIM_HandleTypeDef *htim,
                        uint32_t channel,
                        GPIO_TypeDef *in1_port, uint16_t in1_pin,
                        GPIO_TypeDef *in2_port, uint16_t in2_pin,
                        uint8_t reverse);

    /**
     * @brief 设置电机速度。
     * @param motor: 指向电机实体的指针。
     * @param speed: 速度值
     * - 正数代表正转。
     * - 负数代表反转。
     * - 0 代表刹车。
     * @retval 0: 成功, -1: 传入参数错误。
     */
    int8_t Motor_SetSpeed(Motor_t *motor, int16_t speed);

    /**
     * @brief 快速停止电机 (刹车模式)。
     * @param motor: 指向电机实体的指针。
     * @retval 0: 成功, -1: 传入参数错误。
     */
    int8_t Motor_Stop(Motor_t *motor);

    /**
     * @brief 获取电机当前状态。
     * @param motor: 指向电机实体的指针。
     * @retval 电机状态 (MOTOR_STATE_FORWARD, MOTOR_STATE_BACKWARD, MOTOR_STATE_STOP)。
     */
    MotorState_t Motor_GetState(Motor_t *motor);

    /**
     * @brief (软件层面)使能或失能单个电机。
     * @note  失能后，电机将立即停止，且无法通过SetSpeed再次启动，除非重新使能。
     * @param motor:  指向电机实体的指针。
     * @param enable: 1-使能电机, 0-失能电机。
     * @retval 0: 成功, -1: 传入参数错误。
     */
    int8_t Motor_SetEnable(Motor_t *motor, uint8_t enable);
	
	
	extern Motor_t motor_A;

#ifdef __cplusplus
}
#endif

#endif

