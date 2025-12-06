#include "bno08x_hal.h"
#include <math.h>
#include <string.h>
#include "main.h" // 包含GPIO引脚定义

extern I2C_HandleTypeDef hi2c1;  // 获取I2C句柄
extern UART_HandleTypeDef huart1; // 获取串口句柄用于调试输出

/** @defgroup BNO080_Variables BNO080全局变量
 * @{
 */

// 静态函数前向声明
static uint8_t receivePacket(void);
static uint8_t sendPacket(uint8_t channelNumber, uint8_t dataLength);

/** I2C句柄，用于HAL库通信 */
static I2C_HandleTypeDef *hi2c_bno080;

/** 设备I2C地址（7位地址） */
static uint8_t _deviceAddress;

/** SHTP协议头部缓冲区，4字节 */
static uint8_t shtpHeader[4];

/** SHTP协议数据缓冲区 */
static uint8_t shtpData[MAX_PACKET_SIZE];

/** 各通道的序列号，用于SHTP通信 */
static uint8_t sequenceNumber[6] = {0, 0, 0, 0, 0, 0};

/** 命令序列号 */
static uint8_t commandSequenceNumber = 0;

/** FRS元数据缓冲区 */
static uint32_t metaData[MAX_METADATA_SIZE];

/** 原始传感器数据 */
static uint16_t rawAccelX, rawAccelY, rawAccelZ;            // 加速度计原始数据
static uint8_t accelAccuracy;                               // 加速度计精度
static uint16_t rawLinAccelX, rawLinAccelY, rawLinAccelZ;   // 线性加速度原始数据
static uint8_t accelLinAccuracy;                            // 线性加速度精度
static uint16_t rawGyroX, rawGyroY, rawGyroZ;               // 陀螺仪原始数据
static uint8_t gyroAccuracy;                                // 陀螺仪精度
static uint16_t rawMagX, rawMagY, rawMagZ;                  // 磁力计原始数据
static uint8_t magAccuracy;                                 // 磁力计精度
static uint16_t rawQuatI, rawQuatJ, rawQuatK, rawQuatReal;  // 四元数原始数据
static uint16_t rawQuatRadianAccuracy;                      // 四元数弧度精度
static uint8_t quatAccuracy;                                // 四元数精度
static uint16_t stepCount;                                  // 步数计数
static uint8_t stabilityClassifier;                         // 稳定性分类
static uint8_t activityClassifier;                          // 活动分类
static uint8_t activityConfidences[10];                     // 活动置信度数组
static uint8_t *_activityConfidences = activityConfidences; // 指向活动置信度的指针

/** Q值，用于固定点数到浮点数转换 */
static int rotationVector_Q1 = 14;      // 旋转向量的Q值
static int accelerometer_Q1 = 8;        // 加速度计的Q值
static int linear_accelerometer_Q1 = 8; // 线性加速度计的Q值
static int gyro_Q1 = 9;                 // 陀螺仪的Q值
static int magnetometer_Q1 = 4;         // 磁力计的Q值

/**
 * @}
 */

/** @defgroup BNO080_Functions BNO080函数实现
 * @{
 */

/**
 * @brief 硬件复位BNO08X传感器
 * @note 控制nRESET引脚进行硬件复位，等待Product ID Response
 * @retval 0: 成功, -1: 超时失败
 */
int8_t BNO080_HardwareReset(void)
{
    uint32_t timeout = 0;
    uint8_t resetComplete = 0;

    // 1. 拉低nRESET引脚至少10ms进行硬复位
    HAL_GPIO_WritePin(BNO_RST_GPIO_Port, BNO_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20); // 保持复位状态20ms，确保充分复位

    // 2. 释放nRESET引脚，传感器开始启动
    HAL_GPIO_WritePin(BNO_RST_GPIO_Port, BNO_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100); // 等待传感器内部启动，通常需要50-100ms

    // 3. 等待并检查Product ID Response或启动完成信号
    timeout = HAL_GetTick();
    while ((HAL_GetTick() - timeout) < 1000) { // 1秒超时
        // 尝试接收数据包
        if (receivePacket() == 1) {
            // 检查是否为Product ID Response (0xF8)
            if (shtpData[0] == SHTP_REPORT_PRODUCT_ID_RESPONSE) {
                resetComplete = 1;
                break;
            }
        }
        HAL_Delay(10); // 短暂延时避免过度轮询
    }

    if (!resetComplete) {
        return -1; // 超时失败
    }

    // 4. 清空接收缓冲区，移除启动过程中的其他数据包
    while (receivePacket() == 1) {
        HAL_Delay(1);
    }

    return 0; // 成功
}

/**
 * @brief 初始化BNO080传感器
 * @param hi2c I2C句柄指针，由STM32 HAL库提供
 * @param address 设备I2C地址（7位地址，例如0x4B）
 */
void BNO080_Init(I2C_HandleTypeDef *hi2c, uint8_t address)
{
    hi2c_bno080 = hi2c;       // 保存I2C句柄
    _deviceAddress = address; // 保存设备地址
}

/**
 * @brief 执行软件复位
 *        发送复位命令并清空接收缓冲区
 */
void softReset(void)
{
    shtpData[0] = 1;                   // 设置复位命令
    sendPacket(CHANNEL_EXECUTABLE, 1); // 发送到可执行通道
    HAL_Delay(50);                     // 等待50ms以确保复位完成
    while (receivePacket() == 1)
        ;          // 清空接收缓冲区
    HAL_Delay(50); // 再次等待50ms
    while (receivePacket() == 1)
        ; // 再次清空缓冲区
}

/**
 * @brief 获取复位原因
 * @return 复位原因代码，0表示失败或无响应
 */
uint8_t resetReason(void)
{
    shtpData[0] = SHTP_REPORT_PRODUCT_ID_REQUEST; // 设置产品ID请求
    shtpData[1] = 0;                              // 参数填充
    sendPacket(CHANNEL_CONTROL, 2);               // 发送请求到控制通道
    if (receivePacket() == 1)
    { // 如果接收到数据
        if (shtpData[0] == SHTP_REPORT_PRODUCT_ID_RESPONSE)
        {                       // 检查响应类型
            return shtpData[1]; // 返回复位原因
        }
    }
    return 0; // 未收到有效响应，返回0
}

/**
 * @brief 将固定点数转换为浮点数
 * @param fixedPointValue 固定点数值
 * @param qPoint Q点位置（小数点后的位数）
 * @return 转换后的浮点数值
 */
float qToFloat(int16_t fixedPointValue, uint8_t qPoint)
{
    float qFloat = (float)fixedPointValue; // 转换为浮点数
    qFloat *= powf(2.0f, -qPoint);         // 根据Q点位置缩放
    return qFloat;                         // 返回结果
}

/**
 * @brief 检查是否有新数据可用
 * @return 1表示有新数据，0表示无数据
 */
uint8_t dataAvailable(void)
{
    if (receivePacket() == 1)
    { // 尝试接收数据包
        if (shtpHeader[2] == CHANNEL_REPORTS && shtpData[0] == SHTP_REPORT_BASE_TIMESTAMP)
        {                       // 检查是否为报告通道和基础时间戳
            parseInputReport(); // 解析接收到的报告
            return 1;           // 有数据可用
        }
    }
    return 0; // 无数据
}

/**
 * @brief 解析输入报告数据
 *        根据报告ID提取并存储传感器数据
 */
void parseInputReport(void)
{
    int dataLength = ((uint16_t)shtpHeader[1] << 8 | shtpHeader[0]) & ~(1 << 15); // 计算数据长度（去除继续位）
    dataLength -= 4;                                                              // 减去头部长度

    uint8_t status = shtpData[5 + 2] & 0x03;                                                           // 提取状态/精度（低2位）
    uint16_t data1 = (uint16_t)shtpData[5 + 5] << 8 | shtpData[5 + 4];                                 // 提取数据1
    uint16_t data2 = (uint16_t)shtpData[5 + 7] << 8 | shtpData[5 + 6];                                 // 提取数据2
    uint16_t data3 = (uint16_t)shtpData[5 + 9] << 8 | shtpData[5 + 8];                                 // 提取数据3
    uint16_t data4 = (dataLength - 5 > 9) ? ((uint16_t)shtpData[5 + 11] << 8 | shtpData[5 + 10]) : 0;  // 提取数据4（若存在）
    uint16_t data5 = (dataLength - 5 > 11) ? ((uint16_t)shtpData[5 + 13] << 8 | shtpData[5 + 12]) : 0; // 提取数据5（若存在）

    switch (shtpData[5])
    {                                   // 根据报告ID处理数据
    case SENSOR_REPORTID_ACCELEROMETER: // 加速度计报告
        accelAccuracy = status;
        rawAccelX = data1;
        rawAccelY = data2;
        rawAccelZ = data3;
        break;
    case SENSOR_REPORTID_LINEAR_ACCELERATION: // 线性加速度报告
        accelLinAccuracy = status;
        rawLinAccelX = data1;
        rawLinAccelY = data2;
        rawLinAccelZ = data3;
        break;
    case SENSOR_REPORTID_GYROSCOPE: // 陀螺仪报告
        gyroAccuracy = status;
        rawGyroX = data1;
        rawGyroY = data2;
        rawGyroZ = data3;
        break;
    case SENSOR_REPORTID_MAGNETIC_FIELD: // 磁力计报告
        magAccuracy = status;
        rawMagX = data1;
        rawMagY = data2;
        rawMagZ = data3;
        break;
    case SENSOR_REPORTID_ROTATION_VECTOR:      // 旋转向量报告
    case SENSOR_REPORTID_GAME_ROTATION_VECTOR: // 游戏旋转向量报告
        quatAccuracy = status;
        rawQuatI = data1;
        rawQuatJ = data2;
        rawQuatK = data3;
        rawQuatReal = data4;
        rawQuatRadianAccuracy = data5;
        break;
    case SENSOR_REPORTID_STEP_COUNTER: // 步数计数器报告
        stepCount = data3;
        break;
    case SENSOR_REPORTID_STABILITY_CLASSIFIER: // 稳定性分类器报告
        stabilityClassifier = shtpData[5 + 4];
        break;
    case SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER: // 个人活动分类器报告
        activityClassifier = shtpData[5 + 5];
        for (uint8_t x = 0; x < 9; x++)
        {
            _activityConfidences[x] = shtpData[5 + 6 + x]; // 存储置信度
        }
        break;
    default:
        break; // 未识别的报告类型
    }
}

/**
 * @brief 获取四元数I分量
 * @return 浮点数表示的I分量
 */
float getQuatI(void)
{
    return qToFloat(rawQuatI, rotationVector_Q1);
}

/**
 * @brief 获取四元数J分量
 * @return 浮点数表示的J分量
 */
float getQuatJ(void)
{
    return qToFloat(rawQuatJ, rotationVector_Q1);
}

/**
 * @brief 获取四元数K分量
 * @return 浮点数表示的K分量
 */
float getQuatK(void)
{
    return qToFloat(rawQuatK, rotationVector_Q1);
}

/**
 * @brief 获取四元数实部
 * @return 浮点数表示的实部
 */
float getQuatReal(void)
{
    return qToFloat(rawQuatReal, rotationVector_Q1);
}

/**
 * @brief 获取四元数弧度精度
 * @return 浮点数表示的弧度精度
 */
float getQuatRadianAccuracy(void)
{
    return qToFloat(rawQuatRadianAccuracy, rotationVector_Q1);
}

/**
 * @brief 获取四元数精度级别
 * @return 精度级别（0-3）
 */
uint8_t getQuatAccuracy(void)
{
    return quatAccuracy;
}

/**
 * @brief 获取X轴加速度
 * @return 浮点数表示的X轴加速度（单位：m/s?）
 */
float getAccelX(void)
{
    return qToFloat(rawAccelX, accelerometer_Q1);
}

/**
 * @brief 获取Y轴加速度
 * @return 浮点数表示的Y轴加速度（单位：m/s?）
 */
float getAccelY(void)
{
    return qToFloat(rawAccelY, accelerometer_Q1);
}

/**
 * @brief 获取Z轴加速度
 * @return 浮点数表示的Z轴加速度（单位：m/s?）
 */
float getAccelZ(void)
{
    return qToFloat(rawAccelZ, accelerometer_Q1);
}

/**
 * @brief 获取加速度精度级别
 * @return 精度级别（0-3）
 */
uint8_t getAccelAccuracy(void)
{
    return accelAccuracy;
}

/**
 * @brief 获取X轴线性加速度
 * @return 浮点数表示的X轴线性加速度（单位：m/s?）
 */
float getLinAccelX(void)
{
    return qToFloat(rawLinAccelX, linear_accelerometer_Q1);
}

/**
 * @brief 获取Y轴线性加速度
 * @return 浮点数表示的Y轴线性加速度（单位：m/s?）
 */
float getLinAccelY(void)
{
    return qToFloat(rawLinAccelY, linear_accelerometer_Q1);
}

/**
 * @brief 获取Z轴线性加速度
 * @return 浮点数表示的Z轴线性加速度（单位：m/s?）
 */
float getLinAccelZ(void)
{
    return qToFloat(rawLinAccelZ, linear_accelerometer_Q1);
}

/**
 * @brief 获取线性加速度精度级别
 * @return 精度级别（0-3）
 */
uint8_t getLinAccelAccuracy(void)
{
    return accelLinAccuracy;
}

/**
 * @brief 获取X轴陀螺仪数据
 * @return 浮点数表示的X轴角速度（单位：rad/s）
 */
float getGyroX(void)
{
    return qToFloat(rawGyroX, gyro_Q1);
}

/**
 * @brief 获取Y轴陀螺仪数据
 * @return 浮点数表示的Y轴角速度（单位：rad/s）
 */
float getGyroY(void)
{
    return qToFloat(rawGyroY, gyro_Q1);
}

/**
 * @brief 获取Z轴陀螺仪数据
 * @return 浮点数表示的Z轴角速度（单位：rad/s）
 */
float getGyroZ(void)
{
    return qToFloat(rawGyroZ, gyro_Q1);
}

/**
 * @brief 获取陀螺仪精度级别
 * @return 精度级别（0-3）
 */
uint8_t getGyroAccuracy(void)
{
    return gyroAccuracy;
}

/**
 * @brief 获取X轴磁力计数据
 * @return 浮点数表示的X轴磁场强度（单位：μT）
 */
float getMagX(void)
{
    return qToFloat(rawMagX, magnetometer_Q1);
}

/**
 * @brief 获取Y轴磁力计数据
 * @return 浮点数表示的Y轴磁场强度（单位：μT）
 */
float getMagY(void)
{
    return qToFloat(rawMagY, magnetometer_Q1);
}

/**
 * @brief 获取Z轴磁力计数据
 * @return 浮点数表示的Z轴磁场强度（单位：μT）
 */
float getMagZ(void)
{
    return qToFloat(rawMagZ, magnetometer_Q1);
}

/**
 * @brief 获取磁力计精度级别
 * @return 精度级别（0-3）
 */
uint8_t getMagAccuracy(void)
{
    return magAccuracy;
}

/**
 * @brief 获取步数计数
 * @return 当前步数
 */
uint16_t getStepCount(void)
{
    return stepCount;
}

/**
 * @brief 获取稳定性分类
 * @return 稳定性状态
 */
uint8_t getStabilityClassifier(void)
{
    return stabilityClassifier;
}

/**
 * @brief 获取活动分类
 * @return 当前活动类型
 */
uint8_t getActivityClassifier(void)
{
    return activityClassifier;
}

/**
 * @brief 校准加速度计
 */
void calibrateAccelerometer(void)
{
    sendCalibrateCommand(CALIBRATE_ACCEL);
}

/**
 * @brief 校准陀螺仪
 */
void calibrateGyro(void)
{
    sendCalibrateCommand(CALIBRATE_GYRO);
}

/**
 * @brief 校准磁力计
 */
void calibrateMagnetometer(void)
{
    sendCalibrateCommand(CALIBRATE_MAG);
}

/**
 * @brief 校准平面加速度计
 */
void calibratePlanarAccelerometer(void)
{
    sendCalibrateCommand(CALIBRATE_PLANAR_ACCEL);
}

/**
 * @brief 校准所有传感器
 */
void calibrateAll(void)
{
    sendCalibrateCommand(CALIBRATE_ACCEL_GYRO_MAG);
}

/**
 * @brief 结束校准过程
 */
void endCalibration(void)
{
    sendCalibrateCommand(CALIBRATE_STOP);
}

/**
 * @brief 保存校准数据
 */
void saveCalibration(void)
{
    for (uint8_t x = 3; x < 12; x++)
    {
        shtpData[x] = 0; // 清空数据字段
    }
    sendCommand(COMMAND_DCD); // 发送保存DCD命令
}

/**
 * @brief 设置传感器特征命令
 * @param reportID 报告ID（如SENSOR_REPORTID_ROTATION_VECTOR）
 * @param timeBetweenReports 报告间隔时间（毫秒）
 * @param specificConfig 特定配置参数（通常为0）
 */
void setFeatureCommand(uint8_t reportID, uint32_t timeBetweenReports, uint32_t specificConfig)
{
    uint32_t microsBetweenReports = timeBetweenReports * 1000; // 转换为微秒
    shtpData[0] = SHTP_REPORT_SET_FEATURE_COMMAND;             // 设置特征命令
    shtpData[1] = reportID;                                    // 报告ID
    shtpData[2] = 0;                                           // 保留字节
    shtpData[3] = 0;                                           // 保留字节
    shtpData[4] = 0;                                           // 保留字节
    shtpData[5] = (microsBetweenReports >> 0) & 0xFF;          // 报告间隔低字节
    shtpData[6] = (microsBetweenReports >> 8) & 0xFF;          // 报告间隔次低字节
    shtpData[7] = (microsBetweenReports >> 16) & 0xFF;         // 报告间隔次高字节
    shtpData[8] = (microsBetweenReports >> 24) & 0xFF;         // 报告间隔高字节
    shtpData[9] = 0;                                           // 保留字节
    shtpData[10] = 0;                                          // 保留字节
    shtpData[11] = 0;                                          // 保留字节
    shtpData[12] = 0;                                          // 保留字节
    shtpData[13] = (specificConfig >> 0) & 0xFF;               // 配置参数低字节
    shtpData[14] = (specificConfig >> 8) & 0xFF;               // 配置参数次低字节
    shtpData[15] = (specificConfig >> 16) & 0xFF;              // 配置参数次高字节
    shtpData[16] = (specificConfig >> 24) & 0xFF;              // 配置参数高字节
    sendPacket(CHANNEL_CONTROL, 17);                           // 发送到控制通道，长度17字节
}

/**
 * @brief 发送命令
 * @param command 命令ID（如COMMAND_DCD）
 */
void sendCommand(uint8_t command)
{
    shtpData[0] = SHTP_REPORT_COMMAND_REQUEST; // 命令请求
    shtpData[1] = commandSequenceNumber++;     // 递增序列号
    shtpData[2] = command;                     // 命令ID
    for (uint8_t i = 3; i < 12; i++)
    {
        shtpData[i] = 0; // 填充剩余字节
    }
    sendPacket(CHANNEL_CONTROL, 12); // 发送到控制通道，长度12字节
}

/**
 * @brief 发送校准命令
 * @param thingToCalibrate 要校准的对象（如CALIBRATE_ACCEL）
 */
void sendCalibrateCommand(uint8_t thingToCalibrate)
{
    for (uint8_t x = 3; x < 12; x++)
    {
        shtpData[x] = 0; // 清空数据字段
    }
    switch (thingToCalibrate)
    { // 根据校准对象设置标志位
    case CALIBRATE_ACCEL:
        shtpData[3] = 1; // 加速度计校准
        break;
    case CALIBRATE_GYRO:
        shtpData[4] = 1; // 陀螺仪校准
        break;
    case CALIBRATE_MAG:
        shtpData[5] = 1; // 磁力计校准
        break;
    case CALIBRATE_PLANAR_ACCEL:
        shtpData[7] = 1; // 平面加速度计校准
        break;
    case CALIBRATE_ACCEL_GYRO_MAG:
        shtpData[3] = 1; // 加速度计
        shtpData[4] = 1; // 陀螺仪
        shtpData[5] = 1; // 磁力计
        break;
    case CALIBRATE_STOP:
        // 无需设置标志，保持全0表示停止
        break;
    default:
        break;
    }
    sendCommand(COMMAND_ME_CALIBRATE); // 发送ME校准命令
}

/**
 * @brief 启用旋转向量报告
 * @param timeBetweenReports 报告间隔时间（毫秒）
 */
void enableRotationVector(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_ROTATION_VECTOR, timeBetweenReports, 0);
}

/**
 * @brief 启用游戏旋转向量报告
 * @param timeBetweenReports 报告间隔时间（毫秒）
 */
void enableGameRotationVector(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_GAME_ROTATION_VECTOR, timeBetweenReports, 0);
}

/**
 * @brief 启用加速度计报告
 * @param timeBetweenReports 报告间隔时间（毫秒）
 */
void enableAccelerometer(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_ACCELEROMETER, timeBetweenReports, 0);
}

/**
 * @brief 启用线性加速度计报告
 * @param timeBetweenReports 报告间隔时间（毫秒）
 */
void enableLinearAccelerometer(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_LINEAR_ACCELERATION, timeBetweenReports, 0);
}

/**
 * @brief 启用陀螺仪报告
 * @param timeBetweenReports 报告间隔时间（毫秒）
 */
void enableGyro(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_GYROSCOPE, timeBetweenReports, 0);
}

/**
 * @brief 启用磁力计报告
 * @param timeBetweenReports 报告间隔时间（毫秒）
 */
void enableMagnetometer(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_MAGNETIC_FIELD, timeBetweenReports, 0);
}

/**
 * @brief 启用步数计数器报告
 * @param timeBetweenReports 报告间隔时间（毫秒）
 */
void enableStepCounter(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_STEP_COUNTER, timeBetweenReports, 0);
}

/**
 * @brief 启用稳定性分类器报告
 * @param timeBetweenReports 报告间隔时间（毫秒）
 */
void enableStabilityClassifier(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_STABILITY_CLASSIFIER, timeBetweenReports, 0);
}

/**
 * @brief 获取FRS记录的Q1值
 * @param recordID FRS记录ID（如FRS_RECORDID_ACCELEROMETER）
 * @return Q1值，若失败返回0
 */
int16_t getQ1(uint16_t recordID)
{
    if (readFRSdata(recordID, 7, 1))
    {                                           // 读取第7个字（Q1）
        return (int16_t)(metaData[0] & 0xFFFF); // 返回低16位作为Q1
    }
    return 0; // 读取失败返回0
}

/**
 * @brief 获取FRS记录的Q2值
 * @param recordID FRS记录ID
 * @return Q2值，若失败返回0
 */
int16_t getQ2(uint16_t recordID)
{
    if (readFRSdata(recordID, 8, 1))
    {                                        // 读取第8个字（Q2）
        return (int16_t)(metaData[0] >> 16); // 返回高16位作为Q2
    }
    return 0;
}

/**
 * @brief 获取FRS记录的Q3值
 * @param recordID FRS记录ID
 * @return Q3值，若失败返回0
 */
int16_t getQ3(uint16_t recordID)
{
    if (readFRSdata(recordID, 9, 1))
    {                                           // 读取第9个字（Q3）
        return (int16_t)(metaData[0] & 0xFFFF); // 返回低16位作为Q3
    }
    return 0;
}

/**
 * @brief 获取传感器分辨率
 * @param recordID FRS记录ID
 * @return 分辨率（浮点数），若失败返回0
 */
float getResolution(uint16_t recordID)
{
    if (readFRSdata(recordID, 4, 1))
    { // 读取第4个字（分辨率）
        uint32_t res = metaData[0];
        return *(float *)&res; // 将32位数据转换为浮点数
    }
    return 0.0f;
}

/**
 * @brief 获取传感器量程
 * @param recordID FRS记录ID
 * @return 量程（浮点数），若失败返回0
 */
float getRange(uint16_t recordID)
{
    if (readFRSdata(recordID, 2, 1))
    { // 读取第2个字（量程）
        uint32_t range = metaData[0];
        return *(float *)&range; // 将32位数据转换为浮点数
    }
    return 0.0f;
}

/**
 * @brief 读取FRS单个字数据
 * @param recordID FRS记录ID
 * @param wordNumber 字编号（从0开始）
 * @return 读取的32位数据，若失败返回0
 */
uint32_t readFRSword(uint16_t recordID, uint8_t wordNumber)
{
    if (readFRSdata(recordID, wordNumber, 1))
    {
        return metaData[0]; // 返回读取的字数据
    }
    return 0;
}

/**
 * @brief 请求读取FRS数据
 * @param recordID FRS记录ID
 * @param readOffset 读取偏移量（以字为单位）
 * @param blockSize 数据块大小（以字为单位）
 */
void frsReadRequest(uint16_t recordID, uint16_t readOffset, uint16_t blockSize)
{
    shtpData[0] = SHTP_REPORT_FRS_READ_REQUEST; // FRS读取请求
    shtpData[1] = 0;                            // 保留字节
    shtpData[2] = recordID & 0xFF;              // 记录ID低字节
    shtpData[3] = recordID >> 8;                // 记录ID高字节
    shtpData[4] = readOffset & 0xFF;            // 偏移量低字节
    shtpData[5] = readOffset >> 8;              // 偏移量高字节
    shtpData[6] = blockSize & 0xFF;             // 块大小低字节
    shtpData[7] = blockSize >> 8;               // 块大小高字节
    sendPacket(CHANNEL_CONTROL, 8);             // 发送到控制通道，长度8字节
}

/**
 * @brief 读取FRS数据
 * @param recordID FRS记录ID
 * @param startLocation 起始位置（字编号）
 * @param wordsToRead 要读取的字数
 * @return 1表示成功，0表示失败
 */
uint8_t readFRSdata(uint16_t recordID, uint8_t startLocation, uint8_t wordsToRead)
{
    frsReadRequest(recordID, startLocation, wordsToRead); // 发送读取请求
    HAL_Delay(10);                                        // 等待响应
    for (uint8_t attempts = 0; attempts < 10; attempts++)
    { // 最多尝试10次
        if (receivePacket() == 1)
        { // 接收数据包
            if (shtpHeader[2] == CHANNEL_CONTROL && shtpData[0] == SHTP_REPORT_FRS_READ_RESPONSE)
            {                                                                     // 检查响应类型
                uint16_t readRecordID = (uint16_t)shtpData[3] << 8 | shtpData[2]; // 读取的记录ID
                if (readRecordID != recordID)
                    continue;                            // 验证记录ID
                uint8_t dataOffset = shtpData[5] & 0x0F; // 数据偏移量
                for (uint8_t i = 0; i < wordsToRead; i++)
                {
                    uint8_t index = 6 + (i * 4); // 数据起始索引
                    if (index + 3 < MAX_PACKET_SIZE)
                    {
                        metaData[i] = ((uint32_t)shtpData[index + 3] << 24) |
                                      ((uint32_t)shtpData[index + 2] << 16) |
                                      ((uint32_t)shtpData[index + 1] << 8) |
                                      shtpData[index]; // 组合4字节为32位字
                    }
                }
                return 1; // 读取成功
            }
        }
        HAL_Delay(5); // 每次尝试间隔5ms
    }
    return 0; // 读取失败
}

/**
 * @brief 接收SHTP数据包
 * @return 1表示成功接收，0表示失败
 */
static uint8_t receivePacket(void)
{
    uint8_t header[4]; // 头部缓冲区
    if (HAL_I2C_Master_Receive(hi2c_bno080, _deviceAddress << 1, header, 4, HAL_MAX_DELAY) != HAL_OK)
    {
        return 0; // 读取头部失败
    }

    uint16_t packetLength = ((uint16_t)header[1] << 8 | header[0]) & ~(1 << 15); // 计算数据包长度
    if (packetLength <= 4)
    {                                  // 如果长度小于等于头部长度，无数据
        memcpy(shtpHeader, header, 4); // 保存头部
        return 1;                      // 返回成功（仅头部）
    }

    uint16_t dataLength = packetLength - 4; // 数据部分长度
    memcpy(shtpHeader, header, 4);          // 保存头部到全局变量

    uint16_t bytesRemaining = dataLength; // 剩余字节数
    uint16_t dataSpot = 0;                // 数据存储位置

    while (bytesRemaining > 0)
    {                                                                                                                         // 分块读取数据
        uint16_t numberOfBytesToRead = (bytesRemaining > (I2C_BUFFER_LENGTH - 4)) ? (I2C_BUFFER_LENGTH - 4) : bytesRemaining; // 计算本次读取字节数
        uint8_t temp[I2C_BUFFER_LENGTH];                                                                                      // 临时缓冲区
        if (HAL_I2C_Master_Receive(hi2c_bno080, _deviceAddress << 1, temp, numberOfBytesToRead + 4, HAL_MAX_DELAY) != HAL_OK)
        {
            return 0; // 读取失败
        }
        for (uint8_t x = 0; x < numberOfBytesToRead; x++)
        {
            if (dataSpot < MAX_PACKET_SIZE)
            {                                       // 防止缓冲区溢出
                shtpData[dataSpot++] = temp[x + 4]; // 跳过头部的4字节，存储数据
            }
        }
        bytesRemaining -= numberOfBytesToRead; // 更新剩余字节数
    }
    return 1; // 接收成功
}

/**
 * @brief 发送SHTP数据包
 * @param channelNumber 通道号（如CHANNEL_CONTROL）
 * @param dataLength 数据长度（不包括头部）
 * @return 1表示发送成功，0表示失败
 */
static uint8_t sendPacket(uint8_t channelNumber, uint8_t dataLength)
{
    uint16_t packetLength = dataLength + 4; // 总长度（头部4字节 + 数据）
    uint8_t packet[packetLength];           // 数据包缓冲区

    packet[0] = packetLength & 0xFF;             // 长度低字节
    packet[1] = (packetLength >> 8) & 0xFF;      // 长度高字节
    packet[2] = channelNumber;                   // 通道号
    packet[3] = sequenceNumber[channelNumber]++; // 序列号（递增）

    for (uint8_t i = 0; i < dataLength; i++)
    {
        packet[4 + i] = shtpData[i]; // 填充数据
    }

    if (HAL_I2C_Master_Transmit(hi2c_bno080, _deviceAddress << 1, packet, packetLength, HAL_MAX_DELAY) != HAL_OK)
    {
        return 0; // 发送失败
    }
    return 1; // 发送成功
}

#define M_PI 3.14159265358979323846
/**
 * @brief 将四元数转换为欧拉角
 * @param quatI 四元数的I分量 (x)
 * @param quatJ 四元数的J分量 (y)
 * @param quatK 四元数的K分量 (z)
 * @param quatReal 四元数的实部 (w)
 * @return 欧拉角结构体（单位：度）
 */
void QuaternionToEulerAngles(float quatI, float quatJ, float quatK, float quatReal, 
                            float *roll, float *pitch, float *yaw)
{
    // 四元数归一化
    float norm = sqrtf(quatI*quatI + quatJ*quatJ + quatK*quatK + quatReal*quatReal);
    quatI /= norm;
    quatJ /= norm;
    quatK /= norm;
    quatReal /= norm;
    
    // 计算欧拉角
    // 俯仰角 (Pitch)
    float sinp = 2.0f * (quatReal * quatJ - quatK * quatI);
    if (fabsf(sinp) >= 1)
        *pitch = (sinp >= 0 ? 1 : -1) * (M_PI / 2) * 180.0f / M_PI; // 奇异点处理
    else
        *pitch = asinf(sinp) * 180.0f / M_PI;
    
    // 横滚角 (Roll)
    float sinr_cosp = 2.0f * (quatReal * quatI + quatJ * quatK);
    float cosr_cosp = 1.0f - 2.0f * (quatI * quatI + quatJ * quatJ);
    *roll = atan2f(sinr_cosp, cosr_cosp) * 180.0f / M_PI;
    
    // 偏航角 (Yaw)
    float siny_cosp = 2.0f * (quatReal * quatK + quatI * quatJ);
    float cosy_cosp = 1.0f - 2.0f * (quatJ * quatJ + quatK * quatK);
    *yaw = atan2f(siny_cosp, cosy_cosp) * 180.0f / M_PI;
}

