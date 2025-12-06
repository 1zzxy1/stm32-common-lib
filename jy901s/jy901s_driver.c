/**
 ******************************************************************************
 * @file    JY901S_driver.c
 * @brief   JY901S 陀螺仪驱动库源文件
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

/* Includes ------------------------------------------------------------------*/
#include "jy901s_driver.h"
#include "usart_app.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static int8_t JY901S_ValidateParams(JY901S_t *hwt);
static uint8_t JY901S_CalculateChecksum(uint8_t *data, uint8_t length);
static int8_t JY901S_ParseGyroPacket(JY901S_t *hwt, uint8_t *packet);
static int8_t JY901S_ParseAnglePacket(JY901S_t *hwt, uint8_t *packet);
static int8_t JY901S_ParseAccelPacket(JY901S_t *hwt, uint8_t *packet);
static int8_t JY901S_ParseMagPacket(JY901S_t *hwt, uint8_t *packet);
static float JY901S_ConvertGyroData(uint8_t low, uint8_t high);
static float JY901S_ConvertAngleData(uint8_t low, uint8_t high);
static float JY901S_ConvertAccelData(uint8_t low, uint8_t high);
static float JY901S_ConvertTempData(uint8_t low, uint8_t high);
static float JY901S_ConvertMagData(uint8_t low, uint8_t high);

#if JY901S_WATCHDOG_ENABLE
static void JY901S_RestartUART(JY901S_t* hwt);  // 看门狗重启串口函数
#endif
static int8_t JY901S_SendCommand(JY901S_t *hwt, uint8_t reg_addr, uint16_t data);
static int8_t JY901S_UnlockRegister(JY901S_t *hwt);

/* Exported functions --------------------------------------------------------*/

extern JY901S_t jy901s; 
extern UART_HandleTypeDef huart1;

void jy901s_set(void)
{
	// 检查JY901S是否已使能
	if (!jy901s.enable) {
		my_printf(&huart1, "Error: JY901S not enabled, please call JY901S_Enable first\n");
		return;
	}

	my_printf(&huart1, "Start configuring JY901S...\n");

	//设置波特率为115200
	if (JY901S_SetBaudRate(&jy901s, 6) != 0) {
		my_printf(&huart1, "Error: Baud rate setting failed\n");
		return;
	}
	my_printf(&huart1, "Baud rate set: 115200\n");
	HAL_Delay(500); // 波特率改变后需要较长延时

	//设置输出频率 200Hz 5ms一次
	if (JY901S_SetOutputRate(&jy901s, 11) != 0) {
		my_printf(&huart1, "Error: Output rate setting failed\n");
		return;
	}
	my_printf(&huart1, "Output rate set: 200Hz\n");
	HAL_Delay(200);

	//设置输出内容 可使用位或操作组合多个选项   // Y901S_RSW_ACC|JY901S_RSW_GYRO|JY901S_RSW_ANGLE|JY901S_RSW_MAG
	if (JY901S_SetContent(&jy901s, JY901S_RSW_ANGLE) != 0) {
		my_printf(&huart1, "Error: Output content setting failed\n");
		return;
	}
	my_printf(&huart1, "Output content set: ANGLE\n");
	HAL_Delay(200);

	//设置数字滤波器带宽 较低带宽噪声小但响应慢，较高带宽响应快但噪声大
	if (JY901S_SetBandwidth(&jy901s, JY901S_BANDWIDTH_184HZ) != 0) {
		my_printf(&huart1, "Error: Bandwidth setting failed\n");
		return;
	}
	my_printf(&huart1, "Bandwidth set: 184Hz\n");
	HAL_Delay(200);

	my_printf(&huart1, "JY901S configuration completed!\n");

#if JY901S_WATCHDOG_ENABLE
	// 自动初始化并使能看门狗
	if (JY901S_WatchdogInit(&jy901s, JY901S_WATCHDOG_TIMEOUT) == 0) {
		if (JY901S_WatchdogEnable(&jy901s, 1) == 0) {
			my_printf(&huart1, "JY901S UART2 Watchdog enabled (timeout: %dms)\n", JY901S_WATCHDOG_TIMEOUT);
		} else {
			my_printf(&huart1, "Warning: Watchdog enable failed\n");
		}
	} else {
		my_printf(&huart1, "Warning: Watchdog init failed\n");
	}
#else
	my_printf(&huart1, "JY901S Watchdog disabled by macro\n");
#endif
}

/**
 * @brief JY901S看门狗初始化和使能函数
 * @note 建议在jy901s_set()之后调用
 */
void jy901s_watchdog_init(void)
{
	// 检查JY901S是否已使能
	if (!jy901s.enable) {
		my_printf(&huart1, "Error: JY901S not enabled, please call JY901S_Enable first\n");
		return;
	}

	// 初始化看门狗，设置3秒超时
	if (JY901S_WatchdogInit(&jy901s, 3000) != 0) {
		my_printf(&huart1, "Error: Watchdog initialization failed\n");
		return;
	}

	// 使能看门狗
	if (JY901S_WatchdogEnable(&jy901s, 1) != 0) {
		my_printf(&huart1, "Error: Watchdog enable failed\n");
		return;
	}

	my_printf(&huart1, "JY901S UART2 Watchdog enabled (timeout: 3000ms)\n");
}

/**
 * @brief JY901S完整校准函数
 * @note 按顺序校准加速度、磁场传感器，提高测量精度
 */
void jy901s_calibration(void)
{
	// 检查JY901S是否已使能
	if (!jy901s.enable) {
		my_printf(&huart1, "Error: JY901S not enabled, please call JY901S_Enable first\n");
		return;
	}

	my_printf(&huart1, "=== JY901S Sensor Calibration Start ===\n");

	// 1. 加速度校准
	my_printf(&huart1, "\n1. Starting accelerometer calibration...\n");
	my_printf(&huart1, "Please place device horizontally and keep it still!\n");
	my_printf(&huart1, "Calibration will start in 3 seconds...\n");

	// 倒计时
	for (int i = 3; i > 0; i--) {
		my_printf(&huart1, "%d...\n", i);
		HAL_Delay(1000);
	}

	// 开始加速度校准
	if (JY901S_StartAccCalibration(&jy901s) != 0) {
		my_printf(&huart1, "Error: Accelerometer calibration start failed\n");
		return;
	}
	my_printf(&huart1, "Accelerometer calibration in progress, keep device still...\n");

	// 校准时间（通常需要几秒钟）
	HAL_Delay(5000);

	// 停止加速度校准
	if (JY901S_StopAccCalibration(&jy901s) != 0) {
		my_printf(&huart1, "Error: Accelerometer calibration stop failed\n");
		return;
	}
	my_printf(&huart1, "Accelerometer calibration completed!\n");

	// 2. 磁场校准
	my_printf(&huart1, "\n2. Starting magnetometer calibration...\n");
	my_printf(&huart1, "Please prepare to slowly rotate device in all directions!\n");
	my_printf(&huart1, "Calibration will start in 3 seconds...\n");

	// 倒计时
	for (int i = 3; i > 0; i--) {
		my_printf(&huart1, "%d...\n", i);
		HAL_Delay(1000);
	}

	// 开始磁场校准
	if (JY901S_StartMagCalibration(&jy901s) != 0) {
		my_printf(&huart1, "Error: Magnetometer calibration start failed\n");
		return;
	}
	my_printf(&huart1, "Magnetometer calibration in progress, slowly rotate device in all directions...\n");

	// 磁场校准时间（需要较长时间，让传感器感受各个方向的磁场）
	for (int i = 30; i > 0; i--) {
		my_printf(&huart1, "Time remaining: %d seconds, continue rotating device...\n", i);
		HAL_Delay(1000);
	}

	// 停止磁场校准
	if (JY901S_StopMagCalibration(&jy901s) != 0) {
		my_printf(&huart1, "Error: Magnetometer calibration stop failed\n");
		return;
	}
	my_printf(&huart1, "Magnetometer calibration completed!\n");

	my_printf(&huart1, "\n=== All sensor calibration completed! ===\n");
	my_printf(&huart1, "Recommend restarting device to ensure calibration parameters take effect.\n");
}


/**
 * @brief 创建JY901S实体
 */
int8_t JY901S_Create(JY901S_t* hwt, UART_HandleTypeDef* huart, uint32_t timeout_ms)
{
    // 参数检查
    if (hwt == NULL || huart == NULL)
    {
        return -1;
    }

    // 超时时间检查
    if (timeout_ms == 0)
    {
        timeout_ms = JY901S_TIMEOUT_MS;  // 使用默认超时时间
    }

    // 初始化硬件配置
    hwt->hw.huart = huart;
    hwt->hw.timeout_ms = timeout_ms;

    // 初始化数据结构
    hwt->data.gyro_z = 0.0f;
    hwt->data.yaw = 0.0f;
		hwt->data.gyro_x = 0.0f;
    hwt->data.roll= 0.0f;
		hwt->data.gyro_y = 0.0f;
    hwt->data.pitch = 0.0f;
    hwt->data.version = 0;
    hwt->data.timestamp = 0;
    hwt->data.data_valid = 0;

    // 初始化新增传感器数据字段
    hwt->data.acc_x = 0.0f;
    hwt->data.acc_y = 0.0f;
    hwt->data.acc_z = 0.0f;
    hwt->data.mag_x = 0.0f;
    hwt->data.mag_y = 0.0f;
    hwt->data.mag_z = 0.0f;
    hwt->data.temperature = 0.0f;

    // 初始化状态
    hwt->state = JY901S_STATE_IDLE;
    hwt->enable = 1;
    hwt->rx_index = 0;

    // 清空接收缓冲区
    for (uint8_t i = 0; i < JY901S_BUFFER_SIZE; i++)
    {
        hwt->rx_buffer[i] = 0;
    }

#if JY901S_WATCHDOG_ENABLE
    // 初始化看门狗参数（默认不使能）
    hwt->last_data_time = HAL_GetTick();
    hwt->watchdog_timeout = JY901S_WATCHDOG_TIMEOUT;  // 使用宏定义的超时时间
    hwt->watchdog_enable = 0;      // 默认不使能
    hwt->restart_count = 0;
#endif

    return 0;
}

/**
 * @brief 处理接收数据缓冲区
 */
int8_t JY901S_ProcessBuffer(JY901S_t* hwt, uint8_t* buffer, uint16_t length)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0 || buffer == NULL || length == 0)
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
            case JY901S_STATE_IDLE:
                // 寻找数据包头 0x55
                if (byte == JY901S_HEADER)
                {
                    hwt->rx_buffer[0] = byte;
                    hwt->rx_index = 1;
                    hwt->state = JY901S_STATE_RECEIVING;
                }
                break;

            case JY901S_STATE_RECEIVING:
                // 接收数据包
                hwt->rx_buffer[hwt->rx_index] = byte;
                hwt->rx_index++;

                // 检查是否接收完整数据包
                if (hwt->rx_index >= JY901S_PACKET_SIZE)
                {
                    // 验证校验和
                    uint8_t calculated_checksum = JY901S_CalculateChecksum(hwt->rx_buffer, JY901S_PACKET_SIZE - 1);
                    uint8_t received_checksum = hwt->rx_buffer[JY901S_PACKET_SIZE - 1];

                    if (calculated_checksum == received_checksum)
                    {
                        // 校验和正确，解析数据包
                        uint8_t packet_type = hwt->rx_buffer[1];

                        if (packet_type == JY901S_TYPE_GYRO)//
                        {
                            JY901S_ParseGyroPacket(hwt, hwt->rx_buffer);
                        }
                        else if (packet_type == JY901S_TYPE_ANGLE)
                        {
                            JY901S_ParseAnglePacket(hwt, hwt->rx_buffer);
                        }
                        else if (packet_type == JY901S_TYPE_ACCEL)
                        {
                            JY901S_ParseAccelPacket(hwt, hwt->rx_buffer);
                        }
                        else if (packet_type == JY901S_TYPE_MAG)
                        {
                            JY901S_ParseMagPacket(hwt, hwt->rx_buffer);
                        }

                        hwt->state = JY901S_STATE_DATA_READY;

                        // 数据解析成功，喂狗
                        JY901S_WatchdogFeed(hwt);
                    }
                    else
                    {
                        // 校验和错误
                        hwt->state = JY901S_STATE_ERROR;
                    }

                    // 重置接收状态
                    hwt->rx_index = 0;

                    // 如果状态为错误，下次重新开始寻找包头
                    if (hwt->state == JY901S_STATE_ERROR)
                    {
                        hwt->state = JY901S_STATE_IDLE;
                    }
                }
                break;

            case JY901S_STATE_DATA_READY:
                // 数据已就绪，重新开始寻找下一个数据包
                hwt->state = JY901S_STATE_IDLE;
                // 重新处理当前字节
                i--;
                break;

            case JY901S_STATE_ERROR:
                // 错误状态，重新开始
                hwt->state = JY901S_STATE_IDLE;
                // 重新处理当前字节
                i--;
                break;

            default:
                hwt->state = JY901S_STATE_IDLE;
                break;
        }
    }

    return 0;
}

/**
 * @brief 获取JY901S状态
 */
JY901S_State_t JY901S_GetState(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return JY901S_STATE_ERROR;
    }

    return hwt->state;
}

/**
 * @brief 使能/失能JY901S
 */
int8_t JY901S_Enable(JY901S_t* hwt, uint8_t enable)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    hwt->enable = enable;

    // 如果失能，清空数据和状态
    if (!enable)
    {
				hwt->data.gyro_z = 0.0f;
				hwt->data.yaw = 0.0f;
				hwt->data.gyro_x = 0.0f;
				hwt->data.roll= 0.0f;
				hwt->data.gyro_y = 0.0f;
				hwt->data.pitch = 0.0f;

        hwt->data.version = 0;
        hwt->data.timestamp = 0;
        hwt->data.data_valid = 0;

        // 清空新增传感器数据字段
        hwt->data.acc_x = 0.0f;
        hwt->data.acc_y = 0.0f;
        hwt->data.acc_z = 0.0f;
        hwt->data.mag_x = 0.0f;
        hwt->data.mag_y = 0.0f;
        hwt->data.mag_z = 0.0f;
        hwt->data.temperature = 0.0f;

        hwt->state = JY901S_STATE_IDLE;
        hwt->rx_index = 0;
    }

    return 0;
}

/**
 * @brief 获取角速度Z
 */
float JY901S_GetGyroZ(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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
 * @brief 获取角速度Z
 */
float JY901S_GetGyroX(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.gyro_x;
}
/**
 * @brief 获取角速度Z
 */
float JY901S_GetGyroY(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.gyro_y;
}

/**
 * @brief 获取偏航角
 */
float JY901S_GetYaw(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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
 * @brief 获取偏航角
 */
float JY901S_GetRoll(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.roll;
}
/**
 * @brief 获取偏航角
 */
float JY901S_GetPitch(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.pitch;
}

/**
 * @brief 获取完整数据
 */
JY901S_Data_t* JY901S_GetData(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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
 * @brief 获取加速度X
 */
float JY901S_GetAccX(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.acc_x;
}

/**
 * @brief 获取加速度Y
 */
float JY901S_GetAccY(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.acc_y;
}

/**
 * @brief 获取加速度Z
 */
float JY901S_GetAccZ(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.acc_z;
}

/**
 * @brief 获取温度
 */
float JY901S_GetTemperature(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.temperature;
}

/**
 * @brief 获取磁场X
 */
float JY901S_GetMagX(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.mag_x;
}

/**
 * @brief 获取磁场Y
 */
float JY901S_GetMagY(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.mag_y;
}

/**
 * @brief 获取磁场Z
 */
float JY901S_GetMagZ(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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

    return hwt->data.mag_z;
}

/**
 * @brief 保存配置
 */
int8_t JY901S_SaveConfig(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 发送保存命令：FF AA 00 00 00
    my_printf(hwt->hw.huart, "\xFF\xAA\x00\x00\x00");

    // 延时处理
    HAL_Delay(100);

    return 0;
}

/**
 * @brief 设置波特率
 */
int8_t JY901S_SetBaudRate(JY901S_t* hwt, uint8_t baud_code)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 写入波特率寄存器(0x04)
    if (JY901S_SendCommand(hwt, JY901S_REG_BAUD, (uint16_t)baud_code) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 额外延时，确保配置生效
    HAL_Delay(200);

    return 0;
}

///**
// * @brief 设置输出内容
// * 
// */
//int8_t JY901S_SetRSW(JY901S_t* hwt, uint16_t  function)
//{
//    // 参数检查
//    if (JY901S_ValidateParams(hwt) != 0)
//    {
//        return -1;
//    }

//    // 检查是否使能
//    if (!hwt->enable)
//    {
//        return -1;
//    }

//    // 波特率代码范围检查：1-7
//    if (baud_code < 1 || baud_code > 7)
//    {
//        return -1;
//    }

//    // 解锁寄存器
//    if (JY901S_UnlockRegister(hwt) != 0)
//    {
//        return -1;
//    }

//    // 写入波特率寄存器(0x04)
//    if (JY901S_SendCommand(hwt, JY901S_REG_BAUD, (uint16_t)baud_code) != 0)
//    {
//        return -1;
//    }

//    // 保存配置
//    if (JY901S_SaveConfig(hwt) != 0)
//    {
//        return -1;
//    }

//    // 额外延时，确保配置生效
//    HAL_Delay(200);

//    return 0;
//}


/**
 * @brief 设置输出频率
 */
int8_t JY901S_SetOutputRate(JY901S_t* hwt, uint8_t rate_code)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
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
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 写入输出频率寄存器(0x03)
    if (JY901S_SendCommand(hwt, JY901S_REG_RRATE, (uint16_t)rate_code) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 额外延时，确保配置生效
    HAL_Delay(200);

    return 0;
}

/**
 * @brief 设置输出内容
 */
int8_t JY901S_SetContent(JY901S_t* hwt, uint16_t content)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 输出内容范围检查：有效位为0x01-0xFF
    if (content == 0 || content > 0xFF)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 延时确保解锁生效
    HAL_Delay(20);

    // 写入RSW寄存器(0x02)
    if (JY901S_SendCommand(hwt, JY901S_REG_RSW, content) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 额外延时，确保配置生效
    HAL_Delay(200);

    return 0;
}

/**
 * @brief 设置数字滤波器带宽
 */
int8_t JY901S_SetBandwidth(JY901S_t* hwt, uint8_t bandwidth)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 带宽范围检查：有效值为0-6
    if (bandwidth > JY901S_BANDWIDTH_5HZ)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 延时确保解锁生效
    HAL_Delay(20);

    // 写入BANDWIDTH寄存器(0x1F)
    if (JY901S_SendCommand(hwt, JY901S_REG_BANDWIDTH, bandwidth) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 额外延时，确保配置生效
    HAL_Delay(200);

    return 0;
}

/**
 * @brief 开始加速度校准
 */
int8_t JY901S_StartAccCalibration(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 延时确保解锁生效
    HAL_Delay(20);

    // 写入CALSW寄存器(0x01)，数据为0x0001（加速度校准模式）
    if (JY901S_SendCommand(hwt, JY901S_REG_CALSW, JY901S_CALSW_CALGYROACC) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 延时500ms确保校准开始
    HAL_Delay(500);

    return 0;
}

/**
 * @brief 停止加速度校准
 */
int8_t JY901S_StopAccCalibration(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 延时确保解锁生效
    HAL_Delay(20);

    // 写入CALSW寄存器(0x01)，数据为0x0000（正常模式）
    if (JY901S_SendCommand(hwt, JY901S_REG_CALSW, JY901S_CALSW_NORMAL) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 延时确保配置生效
    HAL_Delay(200);

    return 0;
}

/**
 * @brief 开始磁场校准
 */
int8_t JY901S_StartMagCalibration(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 延时确保解锁生效
    HAL_Delay(20);

    // 写入CALSW寄存器(0x01)，数据为0x0007（磁场校准模式）
    if (JY901S_SendCommand(hwt, JY901S_REG_CALSW, JY901S_CALSW_CALMAGMM) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 延时500ms确保校准开始
    HAL_Delay(500);

    return 0;
}

/**
 * @brief 停止磁场校准
 */
int8_t JY901S_StopMagCalibration(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 延时确保解锁生效
    HAL_Delay(20);

    // 写入CALSW寄存器(0x01)，数据为0x0000（正常模式）
    if (JY901S_SendCommand(hwt, JY901S_REG_CALSW, JY901S_CALSW_NORMAL) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 延时确保配置生效
    HAL_Delay(200);

    return 0;
}

/**
 * @brief 自动检测JY901S当前使用的波特率
 */
uint32_t JY901S_AutoScanBaudRate(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return 0;
    }

    // 定义波特率数组（参考官方例程）
    const uint32_t baud_rates[] = {4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    const uint8_t baud_count = sizeof(baud_rates) / sizeof(baud_rates[0]);

    // 保存原始波特率配置
    uint32_t original_baudrate = hwt->hw.huart->Init.BaudRate;

    // 遍历波特率数组
    for (uint8_t i = 0; i < baud_count; i++)
    {
        // 重新初始化串口为当前测试波特率
        hwt->hw.huart->Init.BaudRate = baud_rates[i];
        if (HAL_UART_Init(hwt->hw.huart) != HAL_OK)
        {
            continue; // 初始化失败，尝试下一个波特率
        }

        // 重试机制
        for (uint8_t retry = 0; retry < 2; retry++)
        {
            // 清空接收缓冲区
            hwt->rx_index = 0;
            hwt->data.data_valid = 0;

            // 发送读取寄存器命令（读取AX寄存器）
            if (JY901S_ReadRegister(hwt, 0x34) == 0) // AX寄存器地址
            {
                // 等待响应
                HAL_Delay(100);

                // 检查是否收到有效数据
                if (hwt->data.data_valid != 0)
                {
                    // 找到正确的波特率
                    return baud_rates[i];
                }
            }
        }
    }

    // 恢复原始波特率配置
    hwt->hw.huart->Init.BaudRate = original_baudrate;
    HAL_UART_Init(hwt->hw.huart);

    // 未找到有效波特率
    return 0;
}

/**
 * @brief 读取寄存器
 */
int8_t JY901S_ReadRegister(JY901S_t* hwt, uint8_t reg_addr)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 发送读取寄存器命令：0xFF 0xAA 0x27 + 寄存器地址低字节 + 寄存器地址高字节
    // 参考官方例程WitReadReg的NORMAL协议实现
    my_printf(hwt->hw.huart, "\xFF\xAA\x27%c%c", reg_addr, 0x00);

    return 0;
}

/**
 * @brief 开始手动校准
 */
int8_t JY901S_StartManualCalibration(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 发送进入手动校准命令：FF AA A6 01 00
    if (JY901S_SendCommand(hwt, JY901S_REG_MANUALCALI, 0x0001) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
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
int8_t JY901S_StopManualCalibration(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 发送退出手动校准命令：FF AA A6 04 00
    if (JY901S_SendCommand(hwt, JY901S_REG_MANUALCALI, 0x0004) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
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
int8_t JY901S_ResetYaw(JY901S_t* hwt)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 检查是否使能
    if (!hwt->enable)
    {
        return -1;
    }

    // 解锁寄存器
    if (JY901S_UnlockRegister(hwt) != 0)
    {
        return -1;
    }

    // 发送Z轴角度归零命令：FF AA 76 00 00
    if (JY901S_SendCommand(hwt, JY901S_REG_CALIYAW, 0x0000) != 0)
    {
        return -1;
    }

    // 保存配置
    if (JY901S_SaveConfig(hwt) != 0)
    {
        return -1;
    }

    // 归零过程延时，确保命令生效
    HAL_Delay(500);

    return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 验证JY901S参数有效性
 * @param hwt: JY901S实体指针
 * @retval 0: 有效, -1: 无效
 */
static int8_t JY901S_ValidateParams(JY901S_t *hwt)
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
static uint8_t JY901S_CalculateChecksum(uint8_t *data, uint8_t length)
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
 * @param hwt: JY901S实体指针
 * @param packet: 数据包缓冲区
 * @retval 0: 成功, -1: 失败
 */
static int8_t JY901S_ParseGyroPacket(JY901S_t *hwt, uint8_t *packet)
{
    // 角速度数据包格式：0x55 0x52 0x00 0x00 RWzL RWzH WzL WzH 0x00 0x00 SUM
    //                   0x55	0x52 WxL	WxH	 WyL	WyH	 WzL WzH VolL	VolH SUM
    // 原始角速度Z：packet[4](低) + packet[5](高)
    // 校准角速度Z：packet[6](低) + packet[7](高)
		
    hwt->data.gyro_x = JY901S_ConvertGyroData(packet[2], packet[3]);
	
    hwt->data.gyro_y = JY901S_ConvertGyroData(packet[4], packet[5]);
	
    hwt->data.gyro_z = JY901S_ConvertGyroData(packet[6], packet[7]);
		
	  
    // 更新时间戳和有效标志
    hwt->data.timestamp = HAL_GetTick();
    hwt->data.data_valid = 1;

    return 0;
}

/**
 * @brief 解析角度数据包
 * @param hwt: JY901S实体指针
 * @param packet: 数据包缓冲区
 * @retval 0: 成功, -1: 失败
 */
static int8_t JY901S_ParseAnglePacket(JY901S_t *hwt, uint8_t *packet)
{
    // 角度数据包格式：0x55 0x53 0x00  0x00  0x00   0x00   YawL YawH VL VH SUM
	  //                 0x55	0x53 RollL RollH PitchL	PitchH YawL	YawH VL	VH SUM
    // 偏航角Z：packet[6](低) + packet[7](高)
    // 版本号：packet[8](低) + packet[9](高)
		
		hwt->data.roll = JY901S_ConvertAngleData(packet[2], packet[3]);
	
		hwt->data.pitch = JY901S_ConvertAngleData(packet[4], packet[5]);
	
    hwt->data.yaw = JY901S_ConvertAngleData(packet[6], packet[7]);
	
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
static float JY901S_ConvertGyroData(uint8_t low, uint8_t high)
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
static float JY901S_ConvertAngleData(uint8_t low, uint8_t high)
{
    // 按照协议文档：偏航角=((high<<8)|low)/32768*180°
    // 注意：需要转换为有符号16位整数
    int16_t raw_data = (int16_t)((high << 8) | low);
    float angle_value = (float)raw_data / 32768.0f * 180.0f;

    return angle_value;
}

/**
 * @brief 发送命令到JY901S
 * @param hwt: JY901S实体指针
 * @param reg_addr: 寄存器地址
 * @param data: 16位数据
 * @retval 0: 成功, -1: 失败
 */
static int8_t JY901S_SendCommand(JY901S_t *hwt, uint8_t reg_addr, uint16_t data)
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
    my_printf(hwt->hw.huart, "\xFF\xAA%c%c%c", reg_addr, data_low, data_high);

    // 延时处理
    HAL_Delay(100);

    return 0;
}

/**
 * @brief 解锁寄存器
 * @param hwt: JY901S实体指针
 * @retval 0: 成功, -1: 失败
 */
static int8_t JY901S_UnlockRegister(JY901S_t *hwt)
{
    // 参数检查
    if (hwt == NULL)
    {
        return -1;
    }

    // 发送解锁序列：FF AA 69 88 B5
    my_printf(hwt->hw.huart, "\xFF\xAA\x69\x88\xB5");

    // 延时处理
    HAL_Delay(100);

    return 0;
}

/**
 * @brief 解析加速度数据包
 * @param hwt: JY901S实体指针
 * @param packet: 数据包缓冲区
 * @retval 0: 成功, -1: 失败
 */
static int8_t JY901S_ParseAccelPacket(JY901S_t *hwt, uint8_t *packet)
{
    // 加速度数据包格式：0x55 0x51 AxL AxH AyL AyH AzL AzH TempL TempH SUM
    // 加速度X：packet[2](低) + packet[3](高)
    // 加速度Y：packet[4](低) + packet[5](高)
    // 加速度Z：packet[6](低) + packet[7](高)
    // 温度：packet[8](低) + packet[9](高)

    hwt->data.acc_x = JY901S_ConvertAccelData(packet[2], packet[3]);
    hwt->data.acc_y = JY901S_ConvertAccelData(packet[4], packet[5]);
    hwt->data.acc_z = JY901S_ConvertAccelData(packet[6], packet[7]);
    hwt->data.temperature = JY901S_ConvertTempData(packet[8], packet[9]);

    // 更新时间戳和有效标志
    hwt->data.timestamp = HAL_GetTick();
    hwt->data.data_valid = 1;

    return 0;
}

/**
 * @brief 转换加速度数据
 * @param low: 低字节
 * @param high: 高字节
 * @retval 加速度值 (g)
 */
static float JY901S_ConvertAccelData(uint8_t low, uint8_t high)
{
    // 按照协议文档：加速度=((high<<8)|low)/32768*16g
    // 注意：需要转换为有符号16位整数
    int16_t raw_data = (int16_t)((high << 8) | low);
    float accel_value = (float)raw_data / 32768.0f * 16.0f;

    return accel_value;
}

/**
 * @brief 转换温度数据
 * @param low: 低字节
 * @param high: 高字节
 * @retval 温度值 (°C)
 */
static float JY901S_ConvertTempData(uint8_t low, uint8_t high)
{
    // 按照协议文档：温度=((high<<8)|low)/340+36.25°C
    // 注意：需要转换为有符号16位整数
    int16_t raw_data = (int16_t)((high << 8) | low);
    float temp_value = (float)raw_data / 340.0f + 36.25f;

    return temp_value;
}

/**
 * @brief 解析磁场数据包
 * @param hwt: JY901S实体指针
 * @param packet: 数据包缓冲区
 * @retval 0: 成功, -1: 失败
 */
static int8_t JY901S_ParseMagPacket(JY901S_t *hwt, uint8_t *packet)
{
    // 磁场数据包格式：0x55 0x54 HxL HxH HyL HyH HzL HzH TempL TempH SUM
    // 磁场X：packet[2](低) + packet[3](高)
    // 磁场Y：packet[4](低) + packet[5](高)
    // 磁场Z：packet[6](低) + packet[7](高)
    // 温度：packet[8](低) + packet[9](高)

    hwt->data.mag_x = JY901S_ConvertMagData(packet[2], packet[3]);
    hwt->data.mag_y = JY901S_ConvertMagData(packet[4], packet[5]);
    hwt->data.mag_z = JY901S_ConvertMagData(packet[6], packet[7]);
    hwt->data.temperature = JY901S_ConvertTempData(packet[8], packet[9]);

    // 更新时间戳和有效标志
    hwt->data.timestamp = HAL_GetTick();
    hwt->data.data_valid = 1;

    return 0;
}

/**
 * @brief 转换磁场数据
 * @param low: 低字节
 * @param high: 高字节
 * @retval 磁场值 (mGauss)
 */
static float JY901S_ConvertMagData(uint8_t low, uint8_t high)
{
    // 按照协议文档：磁场数据直接使用原始值，单位mGauss
    // 注意：需要转换为有符号16位整数
    int16_t raw_data = (int16_t)((high << 8) | low);
    float mag_value = (float)raw_data;

    return mag_value;
}

// ==================== 串口看门狗功能实现 ====================

#if JY901S_WATCHDOG_ENABLE

/**
 * @brief 初始化JY901S串口看门狗
 */
int8_t JY901S_WatchdogInit(JY901S_t* hwt, uint32_t timeout_ms)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    // 超时时间检查（建议1000-10000ms）
    if (timeout_ms < 500 || timeout_ms > 30000)
    {
        return -1;
    }

    // 初始化看门狗参数
    hwt->watchdog_timeout = timeout_ms;
    hwt->last_data_time = HAL_GetTick();
    hwt->watchdog_enable = 0;  // 默认不使能，需要手动开启
    hwt->restart_count = 0;

    return 0;
}

/**
 * @brief 使能/失能JY901S串口看门狗
 */
int8_t JY901S_WatchdogEnable(JY901S_t* hwt, uint8_t enable)
{
    // 参数检查
    if (JY901S_ValidateParams(hwt) != 0)
    {
        return -1;
    }

    hwt->watchdog_enable = enable ? 1 : 0;

    // 使能时重置时间戳
    if (hwt->watchdog_enable)
    {
        hwt->last_data_time = HAL_GetTick();
    }

    return 0;
}

/**
 * @brief JY901S串口看门狗检查函数
 */
void JY901S_WatchdogCheck(JY901S_t* hwt)
{
    // 参数检查
    if (hwt == NULL || !hwt->watchdog_enable)
    {
        return;
    }

    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed_time;

    // 处理时间戳溢出情况
    if (current_time >= hwt->last_data_time)
    {
        elapsed_time = current_time - hwt->last_data_time;
    }
    else
    {
        // 时间戳溢出处理
        elapsed_time = (0xFFFFFFFF - hwt->last_data_time) + current_time + 1;
    }

    // 检查是否超时
    if (elapsed_time > hwt->watchdog_timeout)
    {
        // 串口超时，执行重启
        JY901S_RestartUART(hwt);

        // 更新统计信息
        hwt->restart_count++;
        hwt->last_data_time = current_time;

        // 输出调试信息
        my_printf(hwt->hw.huart, "UART2 Watchdog: Timeout detected, restarting UART (Count: %lu)\n", hwt->restart_count);
    }
}

/**
 * @brief 喂狗函数 - 更新数据接收时间戳
 */
void JY901S_WatchdogFeed(JY901S_t* hwt)
{
    // 参数检查
    if (hwt == NULL)
    {
        return;
    }

    // 更新最后数据接收时间
    hwt->last_data_time = HAL_GetTick();
}

/**
 * @brief 获取串口重启次数
 */
uint32_t JY901S_GetRestartCount(JY901S_t* hwt)
{
    // 参数检查
    if (hwt == NULL)
    {
        return 0;
    }

    return hwt->restart_count;
}

/**
 * @brief 重启串口2
 */
static void JY901S_RestartUART(JY901S_t* hwt)
{
    // 参数检查
    if (hwt == NULL || hwt->hw.huart == NULL)
    {
        return;
    }

    // 停止串口接收
    HAL_UART_AbortReceive(hwt->hw.huart);

    // 反初始化串口
    HAL_UART_DeInit(hwt->hw.huart);

    // 短暂延时
    HAL_Delay(10);

    // 重新初始化串口
    if (HAL_UART_Init(hwt->hw.huart) == HAL_OK)
    {
        // 重新启动DMA接收
        HAL_UART_Receive_DMA(hwt->hw.huart, hwt->rx_buffer, sizeof(hwt->rx_buffer));

        // 清空接收缓冲区
        hwt->rx_index = 0;
        memset(hwt->rx_buffer, 0, sizeof(hwt->rx_buffer));

        // 重置数据有效标志
        hwt->data.data_valid = 0;
    }
    else
    {
        // 重新初始化失败，输出错误信息
        my_printf(hwt->hw.huart, "UART2 Watchdog: UART restart failed!\n");
    }
}

#else // JY901S_WATCHDOG_ENABLE == 0

// 看门狗功能被禁用时的空实现
int8_t JY901S_WatchdogInit(JY901S_t* hwt, uint32_t timeout_ms) { return 0; }
int8_t JY901S_WatchdogEnable(JY901S_t* hwt, uint8_t enable) { return 0; }
void JY901S_WatchdogCheck(JY901S_t* hwt) { }
void JY901S_WatchdogFeed(JY901S_t* hwt) { }
uint32_t JY901S_GetRestartCount(JY901S_t* hwt) { return 0; }

#endif // JY901S_WATCHDOG_ENABLE
