/**
 ******************************************************************************
 * @file    JY901S_driver.h
 * @brief   JY901S 陀螺仪驱动库头文件
 * @author  XiFeng
 * @date    2025-06-27
 ******************************************************************************
 * @attention
 * 
 * 本库专为JY901S陀螺仪传感器设计
 * 支持角速度、角度数据读取，寄存器配置，校准等功能
 * 提供低耦合接口，用户只需提供串口收发接口
 * 
 ******************************************************************************
 */

#ifndef __JY901S_DRIVER_H__
#define __JY901S_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mydefine.h"

/* 看门狗功能控制宏定义 */
#define JY901S_WATCHDOG_ENABLE      1       // 1-开启看门狗, 0-关闭看门狗
#define JY901S_WATCHDOG_TIMEOUT     1000    // 看门狗超时时间(ms)
#define JY901S_WATCHDOG_CHECK_INTERVAL  100 // 看门狗检查间隔(ms)

/* Exported types ------------------------------------------------------------*/

void jy901s_task(void);

/**
 * @brief JY901S状态枚举
 */
typedef enum {
    JY901S_STATE_IDLE = 0,        // 空闲
    JY901S_STATE_RECEIVING,       // 接收中
    JY901S_STATE_DATA_READY,      // 数据就绪
    JY901S_STATE_ERROR            // 错误
} JY901S_State_t;

/**
 * @brief JY901S硬件配置结构体
 */
typedef struct {
    UART_HandleTypeDef* huart;    // 串口句柄
    uint32_t timeout_ms;          // 超时时间
} JY901S_HW_t;

/**
 * @brief JY901S数据结构体
 */
typedef struct {
//    float gyro_z_raw;             // 原始角速度Z (°/s)
    float gyro_z;                 // 校准后角速度Z (°/s)
    float yaw;                    // 偏航角Z (°)
//		float gyro_x_raw;             // 原始角速度Z (°/s)
    float gyro_x;                 // 校准后角速度Z (°/s)
    float roll;                   // 滚转角X
//		float gyro_y_raw;             // 原始角速度Z (°/s)
    float gyro_y;                 // 校准后角速度Z (°/s)
    float pitch;                  // 俯仰角Y
    uint16_t version;             // 版本号
    uint32_t timestamp;           // 时间戳
    uint8_t data_valid;           // 数据有效标志

    // 新增传感器数据字段
    float acc_x, acc_y, acc_z;    // 加速度数据(g)
    float mag_x, mag_y, mag_z;    // 磁场数据(mGauss)
    float temperature;            // 温度数据(°C)
} JY901S_Data_t;

/**
 * @brief JY901S驱动实体结构体
 */
typedef struct {
    JY901S_HW_t hw;               // 硬件配置
    JY901S_Data_t data;           // 传感器数据
    JY901S_State_t state;         // 当前状态
    uint8_t enable;               // 使能标志
    uint8_t rx_buffer[32];        // 接收缓冲区
    uint8_t rx_index;             // 接收索引

#if JY901S_WATCHDOG_ENABLE
    // 串口看门狗相关字段（仅在使能时编译）
    uint32_t last_data_time;      // 上次收到数据的时间戳
    uint32_t watchdog_timeout;    // 看门狗超时时间(ms)
    uint8_t watchdog_enable;      // 看门狗使能标志
    uint32_t restart_count;       // 串口重启次数统计
#endif
} JY901S_t;

/* Exported constants --------------------------------------------------------*/
#define JY901S_PACKET_SIZE          11      // 数据包大小
#define JY901S_TIMEOUT_MS           1000    // 默认超时时间
#define JY901S_BUFFER_SIZE          32      // 接收缓冲区大小

/* JY901S协议常量 */
#define JY901S_HEADER               0x55    // 数据包头

#define JY901S_TYPE_TIME            0x50    // 时间数据类型
#define JY901S_TYPE_ACCEL           0x51    // 加速度数据类型
#define JY901S_TYPE_GYRO            0x52    // 角速度数据类型
#define JY901S_TYPE_ANGLE           0x53    // 角度数据类型
#define JY901S_TYPE_MAG             0x54    // 磁场数据类型
#define JY901S_TYPE_PORT_STATE      0x55    // 端口状态数据类型
#define JY901S_TYPE_PRESSURE        0x56    // 气压高度数据类型
#define JY901S_TYPE_LONLAT          0x57    // 经纬度数据类型
#define JY901S_TYPE_GROUND_SPEED    0x58    // 地速数据类型
#define JY901S_TYPE_QUATERNION      0x59    // 四元数数据类型
#define JY901S_TYPE_GPS_ACCURACY    0x5A    // GPS定位精度数据类型
#define JY901S_TYPE_DATA_TYPE       0x5F    // 读取数据类型

/* JY901S命令常量 */
#define JY901S_CMD_HEADER1          0xFF    // 命令头1
#define JY901S_CMD_HEADER2          0xAA    // 命令头2
#define JY901S_UNLOCK_CODE1         0x69    // 解锁码1
#define JY901S_UNLOCK_CODE2         0x88    // 解锁码2
#define JY901S_UNLOCK_CODE3         0xB5    // 解锁码3

/* 寄存器地址定义 */
#define JY901S_REG_SAVE             0x00    // 保存寄存器
#define JY901S_REG_CALSW            0x01    // 校准模式 
#define JY901S_REG_RSW              0x02    // 输出内容(数据流包含内容)
#define JY901S_REG_RRATE            0x03    // 输出速率寄存器
#define JY901S_REG_BAUD             0x04    // 波特率寄存器
//AXOFFSET~HZOFFSET（零偏设置）
#define JY901S_REG_AXOFFSET         0x05    //加速度X轴零偏，实际加速度零偏=AXOFFSET[15:0]/10000(g)
#define JY901S_REG_AYOFFSET         0x06    
#define JY901S_REG_AZOFFSET         0x07    
#define JY901S_REG_GXOFFSET         0x08    //角速度X轴零偏，实际角速度零偏=GXOFFSET[15:0]/10000(°/s)
#define JY901S_REG_GYOFFSET         0x09    
#define JY901S_REG_GZOFFSET         0x0A    
#define JY901S_REG_HXOFFSET         0x0B    //磁场X轴零偏
#define JY901S_REG_HYOFFSET         0x0C   
#define JY901S_REG_HZOFFSET         0x0D    
//MAGRANGX~MAGRANGZ（磁场校准范围）
#define JY901S_REG_MAGRANGX         0x1C    //磁场校准X轴范围
#define JY901S_REG_MAGRANGY         0x1D    //磁场校准Y轴范围
#define JY901S_REG_MAGRANGZ         0x1E    //磁场校准Z轴范围
//BANDWIDTH（带宽）
#define JY901S_REG_BANDWIDTH        0x1F
//GYRORANGE（陀螺仪量程）
#define JY901S_REG_GYRORANGE        0x20
//ACCRANGE（加速度计量程）
#define JY901S_REG_ACCRANGE         0x21
//SLEEP（休眠）
#define JY901S_REG_SLEEP            0x22
//ORIENT（安装方向）
#define JY901S_REG_ORIENT           0x23
//AXIS6（算法）
#define JY901S_REG_AXIS6            0x24
//FILTK（K值滤波）
#define JY901S_REG_FILTK            0x25

//Roll~Yaw（角度）
#define JY901S_REG_Roll             0x3D
#define JY901S_REG_Pitch            0x3E
#define JY901S_REG_Yaw              0x3F

#define JY901S_REG_CALIYAW          0x76    // Z轴角度归零寄存器
#define JY901S_REG_MANUALCALI       0xA6    // 手动校准寄存器
#define JY901S_REG_NOAUTOCALI       0xA7    // 自动校准开关寄存器

/* RSW输出内容配置常量 */
#define JY901S_RSW_TIME             0x01    // 时间数据包
#define JY901S_RSW_ACC              0x02    // 加速度数据包
#define JY901S_RSW_GYRO             0x04    // 角速度数据包
#define JY901S_RSW_ANGLE            0x08    // 角度数据包
#define JY901S_RSW_MAG              0x10    // 磁场数据包
#define JY901S_RSW_PORT             0x20    // 端口状态数据包
#define JY901S_RSW_PRESS            0x40    // 气压高度数据包
#define JY901S_RSW_GPS              0x80    // GPS数据包

/* 带宽配置常量 */
#define JY901S_BANDWIDTH_256HZ      0x00    // 256Hz带宽
#define JY901S_BANDWIDTH_184HZ      0x01    // 184Hz带宽
#define JY901S_BANDWIDTH_94HZ       0x02    // 94Hz带宽
#define JY901S_BANDWIDTH_44HZ       0x03    // 44Hz带宽
#define JY901S_BANDWIDTH_21HZ       0x04    // 21Hz带宽
#define JY901S_BANDWIDTH_10HZ       0x05    // 10Hz带宽
#define JY901S_BANDWIDTH_5HZ        0x06    // 5Hz带宽

/* 校准模式常量 */
#define JY901S_CALSW_NORMAL         0x00    // 正常模式
#define JY901S_CALSW_CALGYROACC     0x01    // 加速度校准模式
#define JY901S_CALSW_CALMAGMM       0x07    // 磁场校准模式

/* Exported macros -----------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 创建JY901S实体
 * @param hwt: JY901S实体指针
 * @param huart: 串口句柄
 * @param timeout_ms: 超时时间
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_Create(JY901S_t* hwt, UART_HandleTypeDef* huart, uint32_t timeout_ms);

/**
 * @brief 处理接收数据缓冲区
 * @param hwt: JY901S实体指针
 * @param buffer: 接收数据缓冲区
 * @param length: 数据长度
 * @retval 0: 成功, -1: 参数错误, 1: 数据包不完整
 * @note 用户在串口接收中断或DMA完成回调中调用此函数
 */
int8_t JY901S_ProcessBuffer(JY901S_t* hwt, uint8_t* buffer, uint16_t length);

/**
 * @brief 获取角速度Z
 * @param hwt: JY901S实体指针
 * @retval 角速度Z值 (°/s)，无效时返回0.0
 */
float JY901S_GetGyroZ(JY901S_t* hwt);
float JY901S_GetGyroY(JY901S_t* hwt);
float JY901S_GetGyroX(JY901S_t* hwt);

/**
 * @brief 获取偏航角
 * @param hwt: JY901S实体指针
 * @retval 偏航角值 (°)，无效时返回0.0
 */
float JY901S_GetYaw(JY901S_t* hwt);
float JY901S_GetRoll(JY901S_t* hwt);
float JY901S_GetPitch(JY901S_t* hwt);

/**
 * @brief 获取加速度数据
 * @param hwt: JY901S实体指针
 * @retval 加速度值 (g)，无效时返回0.0
 */
float JY901S_GetAccX(JY901S_t* hwt);
float JY901S_GetAccY(JY901S_t* hwt);
float JY901S_GetAccZ(JY901S_t* hwt);

/**
 * @brief 获取温度数据
 * @param hwt: JY901S实体指针
 * @retval 温度值 (°C)，无效时返回0.0
 */
float JY901S_GetTemperature(JY901S_t* hwt);

/**
 * @brief 获取磁场数据
 * @param hwt: JY901S实体指针
 * @retval 磁场值 (mGauss)，无效时返回0.0
 */
float JY901S_GetMagX(JY901S_t* hwt);
float JY901S_GetMagY(JY901S_t* hwt);
float JY901S_GetMagZ(JY901S_t* hwt);

/**
 * @brief 获取完整数据
 * @param hwt: JY901S实体指针
 * @retval 数据结构体指针，无效时返回NULL
 */
JY901S_Data_t* JY901S_GetData(JY901S_t* hwt);

/**
 * @brief 设置波特率
 * @param hwt: JY901S实体指针
 * @param baud_code: 波特率代码
 *                   1: 4800bps, 2: 9600bps, 3: 19200bps
 *                   4: 38400bps, 5: 57600bps, 6: 115200bps, 7: 230400bps
 * @retval 0: 成功, -1: 参数错误
 * @note 设置后需要重新配置MCU串口波特率
 */
int8_t JY901S_SetBaudRate(JY901S_t* hwt, uint8_t baud_code);

/**
 * @brief 设置输出频率
 * @param hwt: JY901S实体指针
 * @param rate_code: 频率代码
 *                   1: 0.2Hz, 2: 0.5Hz, 3: 1Hz, 4: 2Hz, 5: 5Hz
 *                   6: 10Hz, 7: 20Hz, 8: 50Hz, 9: 100Hz
 *                   11: 200Hz, 12: 500Hz, 13: 1000Hz
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_SetOutputRate(JY901S_t* hwt, uint8_t rate_code);

/**
 * @brief 设置输出内容
 * @param hwt: JY901S实体指针
 * @param content: 输出内容位掩码
 *                 JY901S_RSW_TIME: 时间数据包
 *                 JY901S_RSW_ACC: 加速度数据包
 *                 JY901S_RSW_GYRO: 角速度数据包
 *                 JY901S_RSW_ANGLE: 角度数据包
 *                 JY901S_RSW_MAG: 磁场数据包
 *                 可使用位或操作组合多个选项
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_SetContent(JY901S_t* hwt, uint16_t content);

/**
 * @brief 设置数字滤波器带宽
 * @param hwt: JY901S实体指针
 * @param bandwidth: 带宽配置
 *                   JY901S_BANDWIDTH_256HZ: 256Hz带宽
 *                   JY901S_BANDWIDTH_184HZ: 184Hz带宽
 *                   JY901S_BANDWIDTH_94HZ: 94Hz带宽
 *                   JY901S_BANDWIDTH_44HZ: 44Hz带宽
 *                   JY901S_BANDWIDTH_21HZ: 21Hz带宽
 *                   JY901S_BANDWIDTH_10HZ: 10Hz带宽
 *                   JY901S_BANDWIDTH_5HZ: 5Hz带宽
 * @retval 0: 成功, -1: 参数错误
 * @note 较低带宽噪声小但响应慢，较高带宽响应快但噪声大
 */
int8_t JY901S_SetBandwidth(JY901S_t* hwt, uint8_t bandwidth);

/**
 * @brief 开始加速度校准
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 参数错误
 * @note 校准期间必须保持传感器水平静止，避免移动设备
 */
int8_t JY901S_StartAccCalibration(JY901S_t* hwt);

/**
 * @brief 停止加速度校准
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_StopAccCalibration(JY901S_t* hwt);

/**
 * @brief 开始磁场校准
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 参数错误
 * @note 校准期间需要缓慢转动设备，让传感器感受各个方向的磁场，校准时间通常需要几十秒
 */
int8_t JY901S_StartMagCalibration(JY901S_t* hwt);

/**
 * @brief 停止磁场校准
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_StopMagCalibration(JY901S_t* hwt);

/**
 * @brief 自动检测JY901S当前使用的波特率
 * @param hwt: JY901S实体指针
 * @retval 检测到的波特率，失败返回0
 * @note 检测过程会修改串口波特率，建议在初始化阶段使用
 */
uint32_t JY901S_AutoScanBaudRate(JY901S_t* hwt);

/**
 * @brief 读取寄存器
 * @param hwt: JY901S实体指针
 * @param reg_addr: 寄存器地址
 * @retval 0: 成功, -1: 参数错误
 * @note 用于波特率检测时的通信测试
 */
int8_t JY901S_ReadRegister(JY901S_t* hwt, uint8_t reg_addr);

/**
 * @brief 开始手动校准
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 参数错误
 * @note 校准期间必须保持传感器静止
 */
int8_t JY901S_StartManualCalibration(JY901S_t* hwt);

/**
 * @brief 停止手动校准
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_StopManualCalibration(JY901S_t* hwt);

/**
 * @brief Z轴角度归零
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_ResetYaw(JY901S_t* hwt);

/**
 * @brief 保存配置
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 参数错误
 * @note 所有配置修改后都需要调用此函数保存
 */
int8_t JY901S_SaveConfig(JY901S_t* hwt);

/**
 * @brief 获取JY901S状态
 * @param hwt: JY901S实体指针
 * @retval JY901S状态
 */
JY901S_State_t JY901S_GetState(JY901S_t* hwt);

/**
 * @brief 使能/失能JY901S
 * @param hwt: JY901S实体指针
 * @param enable: 1-使能, 0-失能
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_Enable(JY901S_t* hwt, uint8_t enable);

/**
 * @brief JY901S配置函数
 * @note 设置波特率、输出频率、输出内容、带宽等参数
 */
void jy901s_set(void);

/**
 * @brief JY901S完整校准函数
 * @note 按顺序校准加速度、磁场传感器，提高测量精度
 */
void jy901s_calibration(void);

/**
 * @brief JY901S看门狗初始化和使能函数
 * @note 建议在jy901s_set()之后调用，设置3秒超时自动重启串口
 */
void jy901s_watchdog_init(void);

// 看门狗功能函数声明（通过宏控制编译）
/**
 * @brief 初始化JY901S串口看门狗
 * @param hwt: JY901S实体指针
 * @param timeout_ms: 看门狗超时时间(ms)，建议设置为1000-5000ms
 * @retval 0: 成功, -1: 参数错误
 * @note 当超过指定时间未收到数据时，自动重启串口
 */
int8_t JY901S_WatchdogInit(JY901S_t* hwt, uint32_t timeout_ms);

/**
 * @brief 使能/失能JY901S串口看门狗
 * @param hwt: JY901S实体指针
 * @param enable: 1-使能, 0-失能
 * @retval 0: 成功, -1: 参数错误
 */
int8_t JY901S_WatchdogEnable(JY901S_t* hwt, uint8_t enable);

/**
 * @brief JY901S串口看门狗检查函数
 * @param hwt: JY901S实体指针
 * @note 需要在主循环中定期调用，建议每100ms调用一次
 */
void JY901S_WatchdogCheck(JY901S_t* hwt);

/**
 * @brief 喂狗函数 - 更新数据接收时间戳
 * @param hwt: JY901S实体指针
 * @note 在收到有效数据时调用
 */
void JY901S_WatchdogFeed(JY901S_t* hwt);

/**
 * @brief 获取串口重启次数
 * @param hwt: JY901S实体指针
 * @retval 串口重启次数
 */
uint32_t JY901S_GetRestartCount(JY901S_t* hwt);

#ifdef __cplusplus
}
#endif

#endif /* __JY901S_DRIVER_H__ */
