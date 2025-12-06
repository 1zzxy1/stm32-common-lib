/**
 ******************************************************************************
 * @file    hwt101_driver.c
 * @brief   HWT101 陀螺仪驱动库源文件
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

/* Includes ------------------------------------------------------------------*/
#include "hwt101_driver.h"
#include "stdio.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static int8_t HWT101_ValidateParams(HWT101_t *hwt);
static uint8_t HWT101_CalculateChecksum(uint8_t *data, uint8_t length);
static int8_t HWT101_ParseGyroPacket(HWT101_t *hwt, uint8_t *packet);
static int8_t HWT101_ParseAnglePacket(HWT101_t *hwt, uint8_t *packet);
static float HWT101_ConvertGyroData(uint8_t low, uint8_t high);
static float HWT101_ConvertAngleData(uint8_t low, uint8_t high);
static int8_t HWT101_SendCommand(HWT101_t *hwt, uint8_t reg_addr, uint16_t data);
static int8_t HWT101_UnlockRegister(HWT101_t *hwt);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 创建HWT101实体
 */
int8_t HWT101_Create(HWT101_t* hwt, UART_HandleTypeDef* huart, uint32_t timeout_ms)
{
    // 参数检查
    if (hwt == NULL || huart == NULL)
    {
        return -1;
    }

    // 超时时间检查
    if (timeout_ms == 0)
    {
        timeout_ms = HWT101_TIMEOUT_MS;  // 使用默认超时时间
    }

    // 初始化硬件配置
    hwt->hw.huart = huart;
    hwt->hw.timeout_ms = timeout_ms;

    // 初始化数据结构
    hwt->data.gyro_z_raw = 0.0f;
    hwt->data.gyro_z = 0.0f;
    hwt->data.yaw = 0.0f;
    hwt->data.version = 0;
    hwt->data.timestamp = 0;
    hwt->data.data_valid = 0;

    // 初始化状态
    hwt->state = HWT101_STATE_IDLE;
    hwt->enable = 1;
    hwt->rx_index = 0;

    // 清空接收缓冲区
    for (uint8_t i = 0; i < HWT101_BUFFER_SIZE; i++)
    {
        hwt->rx_buffer[i] = 0;
    }

    return 0;
}

/**
 * @brief 处理接收数据缓冲区
 */
int8_t HWT101_ProcessBuffer(HWT101_t* hwt, uint8_t* buffer, uint16_t length)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0 || buffer == NULL || length == 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 处理接收到的数据
    for (uint16_t i = 0; i < length; i++)
    {
        uint8_t byte = buffer[i];

        // 状态机处理数据包
        switch (hwt->state)
        {
            case HWT101_STATE_IDLE:
                // 寻找数据包头 0x55
                if (byte == HWT101_HEADER)
                {
                    hwt->rx_buffer[0] = byte;
                    hwt->rx_index = 1;
                    hwt->state = HWT101_STATE_RECEIVING;
                }
                break;

            case HWT101_STATE_RECEIVING:
                // 接收数据包
                hwt->rx_buffer[hwt->rx_index] = byte;
                hwt->rx_index++;

                // 检查是否接收完整数据包
                if (hwt->rx_index >= HWT101_PACKET_SIZE)
                {
                    // 验证校验和
                    uint8_t calculated_checksum = HWT101_CalculateChecksum(hwt->rx_buffer, HWT101_PACKET_SIZE - 1);
                    uint8_t received_checksum = hwt->rx_buffer[HWT101_PACKET_SIZE - 1];

                    if (calculated_checksum == received_checksum)
                    {
                        // 校验和正确，解析数据包
                        uint8_t packet_type = hwt->rx_buffer[1];

                        if (packet_type == HWT101_TYPE_GYRO)
                        {
                            HWT101_ParseGyroPacket(hwt, hwt->rx_buffer);
                        }
                        else if (packet_type == HWT101_TYPE_ANGLE)
                        {
                            HWT101_ParseAnglePacket(hwt, hwt->rx_buffer);
                        }

                        hwt->state = HWT101_STATE_DATA_READY;
                    }
                    else
                    {
                        // 校验和错误
                        hwt->state = HWT101_STATE_ERROR;
                    }

                    // 重置接收状态
                    hwt->rx_index = 0;

                    // 如果状态为错误，下次重新开始寻找包头
                    if (hwt->state == HWT101_STATE_ERROR)
                    {
                        hwt->state = HWT101_STATE_IDLE;
                    }
                }
                break;

            case HWT101_STATE_DATA_READY:
                // 数据已就绪，重新开始寻找下一个数据包
                hwt->state = HWT101_STATE_IDLE;
                // 重新处理当前字节
                i--;
                break;

            case HWT101_STATE_ERROR:
                // 错误状态，重新开始
                hwt->state = HWT101_STATE_IDLE;
                // 重新处理当前字节
                i--;
                break;

            default:
                hwt->state = HWT101_STATE_IDLE;
                break;
        }
    }

    return 0;
}

/**
 * @brief 获取HWT101状态
 */
HWT101_State_t HWT101_GetState(HWT101_t* hwt)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return HWT101_STATE_ERROR;
    }

    return hwt->state;
}

/**
 * @brief 使能/失能HWT101
 */
int8_t HWT101_Enable(HWT101_t* hwt, uint8_t enable)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    hwt->enable = enable;

    // 如果失能，清空数据和状态
    if (!enable)
    {
        hwt->data.gyro_z_raw = 0.0f;
        hwt->data.gyro_z = 0.0f;
        hwt->data.yaw = 0.0f;
        hwt->data.version = 0;
        hwt->data.timestamp = 0;
        hwt->data.data_valid = 0;
        hwt->state = HWT101_STATE_IDLE;
        hwt->rx_index = 0;
    }

    return 0;
}

/**
 * @brief 获取角速度Z
 */
float HWT101_GetGyroZ(HWT101_t* hwt)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return 0.0f;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return 0.0f;
    }

    // 检查数据有效性
    if (!hwt->data.data_valid)
    {
        return 0.0f;
    }

    return hwt->data.gyro_z;
}

/**
 * @brief 获取偏航角
 */
float HWT101_GetYaw(HWT101_t* hwt)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return 0.0f;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return 0.0f;
    }

    // 检查数据有效性
    if (!hwt->data.data_valid)
    {
        return 0.0f;
    }

    return hwt->data.yaw;
}

/**
 * @brief 获取完整数据
 */
HWT101_Data_t* HWT101_GetData(HWT101_t* hwt)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return NULL;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return NULL;
    }

    // 检查数据有效性
    if (!hwt->data.data_valid)
    {
        return NULL;
    }

    return &(hwt->data);
}

/**
 * @brief 保存配置
 */
int8_t HWT101_SaveConfig(HWT101_t* hwt)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 发送保存命令：FF AA 00 00 00
    printf("\xFF\xAA\x00\x00\x00");

    // 延时处理
    HAL_Delay(100);

    return 0;
}

/**
 * @brief 设置波特率
 */
int8_t HWT101_SetBaudRate(HWT101_t* hwt, uint8_t baud_code)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 波特率代码范围检查：1-7
    if (baud_code < 1 || baud_code > 7)
    {
        return -1;
    }

    // 解锁寄存器
    if (HWT101_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 写入波特率寄存器(0x04)
    if (HWT101_SendCommand(hwt, HWT101_REG_BAUD, (uint16_t)baud_code) != 0)
    {
        return -1;
    }

    // 保存配置
    if (HWT101_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 额外延时，确保配置生效
    HAL_Delay(200);

    return 0;
}

/**
 * @brief 设置输出频率
 */
int8_t HWT101_SetOutputRate(HWT101_t* hwt, uint8_t rate_code)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 输出频率代码范围检查：1-13，但跳过10
    if (rate_code < 1 || rate_code > 13 || rate_code == 10)
    {
        return -1;
    }

    // 解锁寄存器
    if (HWT101_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 写入输出频率寄存器(0x03)
    if (HWT101_SendCommand(hwt, HWT101_REG_RRATE, (uint16_t)rate_code) != 0)
    {
        return -1;
    }

    // 保存配置
    if (HWT101_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 额外延时，确保配置生效
    HAL_Delay(200);

    return 0;
}

/**
 * @brief 开始手动校准
 */
int8_t HWT101_StartManualCalibration(HWT101_t* hwt)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (HWT101_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 发送进入手动校准命令：FF AA A6 01 00
    if (HWT101_SendCommand(hwt, HWT101_REG_MANUALCALI, 0x0001) != 0)
    {
        return -1;
    }

    // 保存配置
    if (HWT101_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 校准过程延时，确保命令生效
    HAL_Delay(500);

    return 0;
}

/**
 * @brief 停止手动校准
 */
int8_t HWT101_StopManualCalibration(HWT101_t* hwt)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (HWT101_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 发送退出手动校准命令：FF AA A6 04 00
    if (HWT101_SendCommand(hwt, HWT101_REG_MANUALCALI, 0x0004) != 0)
    {
        return -1;
    }

    // 保存配置
    if (HWT101_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 校准过程延时，确保命令生效
    HAL_Delay(500);

    return 0;
}

/**
 * @brief Z轴角度归零
 */
int8_t HWT101_ResetYaw(HWT101_t* hwt)
{
    // 参数检查
    if (HWT101_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (HWT101_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 发送Z轴角度归零命令：FF AA 76 00 00
    if (HWT101_SendCommand(hwt, HWT101_REG_CALIYAW, 0x0000) != 0)
    {
        return -1;
    }

    // 保存配置
    if (HWT101_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 归零过程延时，确保命令生效
    HAL_Delay(500);

    return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 验证HWT101参数有效性
 * @param hwt: HWT101实体指针
 * @retval 0: 有效, -1: 无效
 */
static int8_t HWT101_ValidateParams(HWT101_t *hwt)
{
    if (hwt == NULL ||
        hwt->hw.huart == NULL)
    {
        return -1;
    }

    return 0;
}

/**
 * @brief 计算校验和
 * @param data: 数据缓冲区
 * @param length: 数据长度
 * @retval 校验和值
 */
static uint8_t HWT101_CalculateChecksum(uint8_t *data, uint8_t length)
{
    uint8_t checksum = 0;

    for (uint8_t i = 0; i < length; i++)
    {
        checksum += data[i];
    }

    return checksum;
}

/**
 * @brief 解析角速度数据包
 * @param hwt: HWT101实体指针
 * @param packet: 数据包缓冲区
 * @retval 0: 成功, -1: 失败
 */
static int8_t HWT101_ParseGyroPacket(HWT101_t *hwt, uint8_t *packet)
{
    // 角速度数据包格式：0x55 0x52 0x00 0x00 RWzL RWzH WzL WzH 0x00 0x00 SUM
    // 原始角速度Z：packet[4](低) + packet[5](高)
    // 校准角速度Z：packet[6](低) + packet[7](高)

    hwt->data.gyro_z_raw = HWT101_ConvertGyroData(packet[4], packet[5]);
    hwt->data.gyro_z = HWT101_ConvertGyroData(packet[6], packet[7]);

    // 更新时间戳和有效标志
    hwt->data.timestamp = HAL_GetTick();
    hwt->data.data_valid = 1;

    return 0;
}

/**
 * @brief 解析角度数据包
 * @param hwt: HWT101实体指针
 * @param packet: 数据包缓冲区
 * @retval 0: 成功, -1: 失败
 */
static int8_t HWT101_ParseAnglePacket(HWT101_t *hwt, uint8_t *packet)
{
    // 角度数据包格式：0x55 0x53 0x00 0x00 0x00 0x00 YawL YawH VL VH SUM
    // 偏航角Z：packet[6](低) + packet[7](高)
    // 版本号：packet[8](低) + packet[9](高)

    hwt->data.yaw = HWT101_ConvertAngleData(packet[6], packet[7]);
    hwt->data.version = (uint16_t)((packet[9] << 8) | packet[8]);

    // 更新时间戳和有效标志
    hwt->data.timestamp = HAL_GetTick();
    hwt->data.data_valid = 1;

    return 0;
}

/**
 * @brief 转换角速度数据
 * @param low: 低字节
 * @param high: 高字节
 * @retval 角速度值 (°/s)
 */
static float HWT101_ConvertGyroData(uint8_t low, uint8_t high)
{
    // 按照协议文档：角速度=((high<<8)|low)/32768*2000°/s
    // 注意：需要转换为有符号16位整数
    int16_t raw_data = (int16_t)((high << 8) | low);
    float gyro_value = (float)raw_data / 32768.0f * 2000.0f;

    return gyro_value;
}

/**
 * @brief 转换角度数据
 * @param low: 低字节
 * @param high: 高字节
 * @retval 角度值 (°)
 */
static float HWT101_ConvertAngleData(uint8_t low, uint8_t high)
{
    // 按照协议文档：偏航角=((high<<8)|low)/32768*180°
    // 注意：需要转换为有符号16位整数
    int16_t raw_data = (int16_t)((high << 8) | low);
    float angle_value = (float)raw_data / 32768.0f * 180.0f;

    return angle_value;
}

/**
 * @brief 发送命令到HWT101
 * @param hwt: HWT101实体指针
 * @param reg_addr: 寄存器地址
 * @param data: 16位数据
 * @retval 0: 成功, -1: 失败
 */
static int8_t HWT101_SendCommand(HWT101_t *hwt, uint8_t reg_addr, uint16_t data)
{
    // 参数检查
    if (hwt == NULL)
    {
        return -1;
    }

    // 提取数据的低字节和高字节
    uint8_t data_low = (uint8_t)(data & 0xFF);
    uint8_t data_high = (uint8_t)((data >> 8) & 0xFF);

    // 发送5字节命令：0xFF 0xAA + 寄存器地址 + 数据低字节 + 数据高字节
    printf("\xFF\xAA%c%c%c", reg_addr, data_low, data_high);

    // 延时处理
    HAL_Delay(100);

    return 0;
}

/**
 * @brief 解锁寄存器
 * @param hwt: HWT101实体指针
 * @retval 0: 成功, -1: 失败
 */
static int8_t HWT101_UnlockRegister(HWT101_t *hwt)
{
    // 参数检查
    if (hwt == NULL)
    {
        return -1;
    }

    // 发送解锁序列：FF AA 69 88 B5
    printf("\xFF\xAA\x69\x88\xB5");

    // 延时处理
    HAL_Delay(100);

    return 0;
}
