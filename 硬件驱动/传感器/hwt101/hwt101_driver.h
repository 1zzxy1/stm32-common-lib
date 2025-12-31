/**
 ******************************************************************************
 * @file    hwt101_driver.h
 * @brief   HWT101 陀螺仪驱动库头文件
 * @author  XiFeng
 * @date    2025-06-27
 ******************************************************************************
 * @attention
 * 
 * 本库专为HWT101陀螺仪传感器设计
 * 支持角速度、角度数据读取，寄存器配置，校准等功能
 * 提供低耦合接口，用户只需提供串口收发接口
 * 
 ******************************************************************************
 */

#ifndef __HWT101_DRIVER_H__
#define __HWT101_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"

/* Exported types ------------------------------------------------------------*/

/**
 * @brief HWT101状态枚举
 */
typedef enum {
    HWT101_STATE_IDLE = 0,        // 空闲
    HWT101_STATE_RECEIVING,       // 接收中
    HWT101_STATE_DATA_READY,      // 数据就绪
    HWT101_STATE_ERROR            // 错误
} HWT101_State_t;

/**
 * @brief HWT101硬件配置结构体
 */
typedef struct {
    UART_HandleTypeDef* huart;    // 串口句柄
    uint32_t timeout_ms;          // 超时时间
} HWT101_HW_t;

/**
 * @brief HWT101数据结构体
 */
typedef struct {
    float gyro_z_raw;             // 原始角速度Z (°/s)
    float gyro_z;                 // 校准后角速度Z (°/s)  
    float yaw;                    // 偏航角Z (°)
    uint16_t version;             // 版本号
    uint32_t timestamp;           // 时间戳
    uint8_t data_valid;           // 数据有效标志
} HWT101_Data_t;

/**
 * @brief HWT101驱动实体结构体
 */
typedef struct {
    HWT101_HW_t hw;               // 硬件配置
    HWT101_Data_t data;           // 传感器数据
    HWT101_State_t state;         // 当前状态
    uint8_t enable;               // 使能标志
    uint8_t rx_buffer[32];        // 接收缓冲区
    uint8_t rx_index;             // 接收索引
} HWT101_t;

/* Exported constants --------------------------------------------------------*/
#define HWT101_PACKET_SIZE          11      // 数据包大小
#define HWT101_TIMEOUT_MS           1000    // 默认超时时间
#define HWT101_BUFFER_SIZE          32      // 接收缓冲区大小

/* HWT101协议常量 */
#define HWT101_HEADER               0x55    // 数据包头
#define HWT101_TYPE_GYRO            0x52    // 角速度数据类型
#define HWT101_TYPE_ANGLE           0x53    // 角度数据类型

/* HWT101命令常量 */
#define HWT101_CMD_HEADER1          0xFF    // 命令头1
#define HWT101_CMD_HEADER2          0xAA    // 命令头2
#define HWT101_UNLOCK_CODE1         0x69    // 解锁码1
#define HWT101_UNLOCK_CODE2         0x88    // 解锁码2
#define HWT101_UNLOCK_CODE3         0xB5    // 解锁码3

/* 寄存器地址定义 */
#define HWT101_REG_SAVE             0x00    // 保存寄存器
#define HWT101_REG_RRATE            0x03    // 输出速率寄存器
#define HWT101_REG_BAUD             0x04    // 波特率寄存器
#define HWT101_REG_CALIYAW          0x76    // Z轴角度归零寄存器
#define HWT101_REG_MANUALCALI       0xA6    // 手动校准寄存器
#define HWT101_REG_NOAUTOCALI       0xA7    // 自动校准开关寄存器

/* Exported macros -----------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 创建HWT101实体
 * @param hwt: HWT101实体指针
 * @param huart: 串口句柄
 * @param timeout_ms: 超时时间
 * @retval 0: 成功, -1: 参数错误
 */
int8_t HWT101_Create(HWT101_t* hwt, UART_HandleTypeDef* huart, uint32_t timeout_ms);

/**
 * @brief 处理接收数据缓冲区
 * @param hwt: HWT101实体指针
 * @param buffer: 接收数据缓冲区
 * @param length: 数据长度
 * @retval 0: 成功, -1: 参数错误, 1: 数据包不完整
 * @note 用户在串口接收中断或DMA完成回调中调用此函数
 */
int8_t HWT101_ProcessBuffer(HWT101_t* hwt, uint8_t* buffer, uint16_t length);

/**
 * @brief 获取角速度Z
 * @param hwt: HWT101实体指针
 * @retval 角速度Z值 (°/s)，无效时返回0.0
 */
float HWT101_GetGyroZ(HWT101_t* hwt);

/**
 * @brief 获取偏航角
 * @param hwt: HWT101实体指针
 * @retval 偏航角值 (°)，无效时返回0.0
 */
float HWT101_GetYaw(HWT101_t* hwt);

/**
 * @brief 获取完整数据
 * @param hwt: HWT101实体指针
 * @retval 数据结构体指针，无效时返回NULL
 */
HWT101_Data_t* HWT101_GetData(HWT101_t* hwt);

/**
 * @brief 设置波特率
 * @param hwt: HWT101实体指针
 * @param baud_code: 波特率代码
 *                   1: 4800bps, 2: 9600bps, 3: 19200bps
 *                   4: 38400bps, 5: 57600bps, 6: 115200bps, 7: 230400bps
 * @retval 0: 成功, -1: 参数错误
 * @note 设置后需要重新配置MCU串口波特率
 */
int8_t HWT101_SetBaudRate(HWT101_t* hwt, uint8_t baud_code);

/**
 * @brief 设置输出频率
 * @param hwt: HWT101实体指针
 * @param rate_code: 频率代码
 *                   1: 0.2Hz, 2: 0.5Hz, 3: 1Hz, 4: 2Hz, 5: 5Hz
 *                   6: 10Hz, 7: 20Hz, 8: 50Hz, 9: 100Hz
 *                   11: 200Hz, 12: 500Hz, 13: 1000Hz
 * @retval 0: 成功, -1: 参数错误
 */
int8_t HWT101_SetOutputRate(HWT101_t* hwt, uint8_t rate_code);

/**
 * @brief 开始手动校准
 * @param hwt: HWT101实体指针
 * @retval 0: 成功, -1: 参数错误
 * @note 校准期间必须保持传感器静止
 */
int8_t HWT101_StartManualCalibration(HWT101_t* hwt);

/**
 * @brief 停止手动校准
 * @param hwt: HWT101实体指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t HWT101_StopManualCalibration(HWT101_t* hwt);

/**
 * @brief Z轴角度归零
 * @param hwt: HWT101实体指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t HWT101_ResetYaw(HWT101_t* hwt);

/**
 * @brief 保存配置
 * @param hwt: HWT101实体指针
 * @retval 0: 成功, -1: 参数错误
 * @note 所有配置修改后都需要调用此函数保存
 */
int8_t HWT101_SaveConfig(HWT101_t* hwt);

/**
 * @brief 获取HWT101状态
 * @param hwt: HWT101实体指针
 * @retval HWT101状态
 */
HWT101_State_t HWT101_GetState(HWT101_t* hwt);

/**
 * @brief 使能/失能HWT101
 * @param hwt: HWT101实体指针
 * @param enable: 1-使能, 0-失能
 * @retval 0: 成功, -1: 参数错误
 */
int8_t HWT101_Enable(HWT101_t* hwt, uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif /* __HWT101_DRIVER_H__ */
