///**
// * @file bno08x_hardware_reset_example.c
// * @brief BNO08X硬件复位功能使用示例
// * @note 展示如何使用BNO080_HardwareReset()函数进行硬件复位
// */

//#include "bno08x_hal.h"
//#include "mydefine.h"

//extern I2C_HandleTypeDef hi2c1;
//extern UART_HandleTypeDef huart1;

///**
// * @brief BNO08X硬件复位示例函数
// * @note 演示硬件复位的完整流程
// */
//void bno08x_hardware_reset_example(void)
//{
//    my_printf(&huart1, "=== BNO08X硬件复位示例 ===\n");
//    
//    // 1. 初始化I2C通信
//    BNO080_Init(&hi2c1, BNO080_DEFAULT_ADDRESS);
//    my_printf(&huart1, "I2C通信初始化完成\n");
//    
//    // 2. 执行硬件复位
//    my_printf(&huart1, "开始硬件复位...\n");
//    int8_t reset_result = BNO080_HardwareReset();
//    
//    if (reset_result == 0) {
//        my_printf(&huart1, "✓ 硬件复位成功！\n");
//        my_printf(&huart1, "  - nRESET引脚控制正常\n");
//        my_printf(&huart1, "  - 传感器启动完成\n");
//        my_printf(&huart1, "  - 收到Product ID Response\n");
//        
//        // 3. 获取复位原因（可选）
//        uint8_t reset_reason = resetReason();
//        my_printf(&huart1, "复位原因代码: 0x%02X\n", reset_reason);
//        
//        // 4. 配置传感器（示例）
//        my_printf(&huart1, "配置传感器功能...\n");
//        enableRotationVector(100); // 启用旋转向量，100ms间隔
//        my_printf(&huart1, "✓ 旋转向量已启用\n");
//        
//    } else {
//        my_printf(&huart1, "✗ 硬件复位失败！\n");
//        my_printf(&huart1, "可能原因：\n");
//        my_printf(&huart1, "  - nRESET引脚连接问题\n");
//        my_printf(&huart1, "  - I2C通信异常\n");
//        my_printf(&huart1, "  - 传感器硬件故障\n");
//        my_printf(&huart1, "  - 超时（1秒内未收到响应）\n");
//        
//        // 备用方案：尝试软件复位
//        my_printf(&huart1, "尝试软件复位作为备用方案...\n");
//        softReset();
//        HAL_Delay(100);
//        my_printf(&huart1, "软件复位完成\n");
//    }
//    
//    my_printf(&huart1, "=== 硬件复位示例结束 ===\n\n");
//}

///**
// * @brief BNO08X传感器异常恢复示例
// * @note 演示在传感器异常时如何使用硬件复位进行恢复
// */
//void bno08x_error_recovery_example(void)
//{
//    my_printf(&huart1, "=== BNO08X异常恢复示例 ===\n");
//    
//    // 模拟传感器异常检测
//    my_printf(&huart1, "检测到传感器异常，开始恢复流程...\n");
//    
//    // 1. 尝试硬件复位恢复
//    my_printf(&huart1, "步骤1: 执行硬件复位\n");
//    if (BNO080_HardwareReset() == 0) {
//        my_printf(&huart1, "✓ 硬件复位成功，传感器已恢复\n");
//        
//        // 重新配置传感器
//        my_printf(&huart1, "步骤2: 重新配置传感器\n");
//        enableRotationVector(100);
//        my_printf(&huart1, "✓ 传感器配置完成\n");
//        
//    } else {
//        my_printf(&huart1, "✗ 硬件复位失败\n");
//        
//        // 2. 备用恢复方案
//        my_printf(&huart1, "步骤2: 尝试软件复位\n");
//        softReset();
//        HAL_Delay(100);
//        
//        // 3. 检查恢复状态
//        if (dataAvailable()) {
//            my_printf(&huart1, "✓ 软件复位成功，传感器部分恢复\n");
//        } else {
//            my_printf(&huart1, "✗ 所有恢复方案失败，需要检查硬件\n");
//        }
//    }
//    
//    my_printf(&huart1, "=== 异常恢复示例结束 ===\n\n");
//}

///**
// * @brief 硬件复位时序测试
// * @note 测试不同复位时序的效果
// */
//void bno08x_reset_timing_test(void)
//{
//    my_printf(&huart1, "=== BNO08X复位时序测试 ===\n");
//    
//    // 测试1: 标准时序（20ms复位 + 100ms等待）
//    my_printf(&huart1, "测试1: 标准时序\n");
//    HAL_GPIO_WritePin(BNO_RST_GPIO_Port, BNO_RST_Pin, GPIO_PIN_RESET);
//    HAL_Delay(20);
//    HAL_GPIO_WritePin(BNO_RST_GPIO_Port, BNO_RST_Pin, GPIO_PIN_SET);
//    HAL_Delay(100);
//    
//    uint32_t start_time = HAL_GetTick();
//    uint8_t response_received = 0;
//    
//    while ((HAL_GetTick() - start_time) < 1000) {
//        if (receivePacket() == 1) {
//            if (shtpData[0] == SHTP_REPORT_PRODUCT_ID_RESPONSE) {
//                response_received = 1;
//                break;
//            }
//        }
//        HAL_Delay(10);
//    }
//    
//    uint32_t response_time = HAL_GetTick() - start_time;
//    
//    if (response_received) {
//        my_printf(&huart1, "✓ 标准时序成功，响应时间: %lu ms\n", response_time);
//    } else {
//        my_printf(&huart1, "✗ 标准时序失败\n");
//    }
//    
//    my_printf(&huart1, "=== 复位时序测试结束 ===\n\n");
//}
