#include "Emm_V5.h"
#include "app_uasrt.h"

/**********************************************************
***	Emm_V5.0步进闭环控制例程
***	编写作者：ZHANGDATOU
***	技术支持：张大头闭环伺服
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/

/**
  * @brief    将当前位置清零
  * @param    huart ：串口句柄
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Reset_CurPos_To_Zero(UART_HandleTypeDef* huart, uint8_t addr)
{
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x0A;                       // 功能码
  cmd[2] =  0x6D;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 发送命令
	HAL_UART_Transmit(huart, (uint8_t *)cmd, 4, EMM_UART_TIMEOUT);
}

/**
  * @brief    解除堵转保护
  * @param    huart ：串口句柄
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Reset_Clog_Pro(UART_HandleTypeDef* huart, uint8_t addr)
{
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x0E;                       // 功能码
  cmd[2] =  0x52;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 4, EMM_UART_TIMEOUT);
}

/**
  * @brief    读取系统参数
  * @param    huart ：串口句柄
  * @param    addr  ：电机地址
  * @param    s     ：系统参数类型
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Read_Sys_Params(UART_HandleTypeDef* huart, uint8_t addr, SysParams_t s)
{
  uint8_t i = 0;
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[i] = addr; ++i;                   // 地址

  switch(s)                             // 功能码
  {
    case S_VER  : cmd[i] = 0x1F; ++i; break;
    case S_RL   : cmd[i] = 0x20; ++i; break;
    case S_PID  : cmd[i] = 0x21; ++i; break;
    case S_VBUS : cmd[i] = 0x24; ++i; break;
    case S_CPHA : cmd[i] = 0x27; ++i; break;
    case S_ENCL : cmd[i] = 0x31; ++i; break;
    case S_TPOS : cmd[i] = 0x33; ++i; break;
    case S_VEL  : cmd[i] = 0x35; ++i; break;
    case S_CPOS : cmd[i] = 0x36; ++i; break;
    case S_PERR : cmd[i] = 0x37; ++i; break;
    case S_FLAG : cmd[i] = 0x3A; ++i; break;
    case S_ORG  : cmd[i] = 0x3B; ++i; break;
    case S_Conf : cmd[i] = 0x42; ++i; cmd[i] = 0x6C; ++i; break;
    case S_State: cmd[i] = 0x43; ++i; cmd[i] = 0x7A; ++i; break;
    default: break;
  }

  cmd[i] = 0x6B; ++i;                   // 校验字节
  
  // 发送命令并获取返回状态
  HAL_UART_Transmit(huart, (uint8_t *)cmd, i, EMM_UART_TIMEOUT);
  
}

/**
  * @brief    修改开环/闭环控制模式
  * @param    huart    ：串口句柄
  * @param    addr     ：电机地址
  * @param    svF      ：是否存储标志，false为不存储，true为存储
  * @param    ctrl_mode：控制模式（对应屏幕上的P_Pul菜单），0是关闭脉冲输入引脚，1是开环模式，2是闭环模式，3是让En端口复用为多圈限位开关输入引脚，Dir端口复用为到位输出高电平功能
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Modify_Ctrl_Mode(UART_HandleTypeDef* huart, uint8_t addr, bool svF, uint8_t ctrl_mode)
{
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x46;                       // 功能码
  cmd[2] =  0x69;                       // 辅助码
  cmd[3] =  svF;                        // 是否存储标志，false为不存储，true为存储
  cmd[4] =  ctrl_mode;                  // 控制模式（对应屏幕上的P_Pul菜单），0是关闭脉冲输入引脚，1是开环模式，2是闭环模式，3是让En端口复用为多圈限位开关输入引脚，Dir端口复用为到位输出高电平功能
  cmd[5] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 6, EMM_UART_TIMEOUT);
}

/**
  * @brief    使能信号控制
  * @param    huart ：串口句柄
  * @param    addr  ：电机地址
  * @param    state ：使能状态     ，true为使能电机，false为关闭电机
  * @param    snF   ：多机同步标志 ，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_En_Control(UART_HandleTypeDef* huart, uint8_t addr, bool state, bool snF)
{
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xF3;                       // 功能码
  cmd[2] =  0xAB;                       // 辅助码
  cmd[3] =  (uint8_t)state;             // 使能状态
  cmd[4] =  snF;                        // 多机同步运动标志
  cmd[5] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 6, EMM_UART_TIMEOUT);
}

/**
  * @brief    速度模式
  * @param    huart：串口句柄
  * @param    addr：电机地址
  * @param    dir ：方向       ，0为CW，其余值为CCW
  * @param    vel ：速度       ，范围0 - 5000RPM
  * @param    acc ：加速度     ，范围0 - 255，注意：0是直接启动
  * @param    snF ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Vel_Control(UART_HandleTypeDef* huart, uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF)
{
  uint8_t cmd[16] = {0};

  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xF6;                       // 功能码
  cmd[2] =  dir;                        // 方向
  cmd[3] =  (uint8_t)(vel >> 8);        // 速度(RPM)高8位字节
  cmd[4] =  (uint8_t)(vel >> 0);        // 速度(RPM)低8位字节
  cmd[5] =  acc;                        // 加速度，注意：0是直接启动
  cmd[6] =  snF;                        // 多机同步运动标志
  cmd[7] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 8, EMM_UART_TIMEOUT);
}

/**
  * @brief    位置模式
  * @param    huart：串口句柄
  * @param    addr：电机地址
  * @param    dir ：方向        ，0为CW，其余值为CCW
  * @param    vel ：速度(RPM)   ，范围0 - 5000RPM
  * @param    acc ：加速度      ，范围0 - 255，注意：0是直接启动
  * @param    clk ：脉冲数      ，范围0- (2^32 - 1)个
  * @param    raF ：相位/绝对标志，false为相对运动，true为绝对值运动
  * @param    snF ：多机同步标志 ，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Pos_Control(UART_HandleTypeDef* huart, uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF)
{
  uint8_t cmd[16] = {0};

  // 装载命令
  cmd[0]  =  addr;                      // 地址
  cmd[1]  =  0xFD;                      // 功能码
  cmd[2]  =  dir;                       // 方向
  cmd[3]  =  (uint8_t)(vel >> 8);       // 速度(RPM)高8位字节
  cmd[4]  =  (uint8_t)(vel >> 0);       // 速度(RPM)低8位字节 
  cmd[5]  =  acc;                       // 加速度，注意：0是直接启动
  cmd[6]  =  (uint8_t)(clk >> 24);      // 脉冲数(bit24 - bit31)
  cmd[7]  =  (uint8_t)(clk >> 16);      // 脉冲数(bit16 - bit23)
  cmd[8]  =  (uint8_t)(clk >> 8);       // 脉冲数(bit8  - bit15)
  cmd[9]  =  (uint8_t)(clk >> 0);       // 脉冲数(bit0  - bit7 )
  cmd[10] =  raF;                       // 相位/绝对标志，false为相对运动，true为绝对值运动
  cmd[11] =  snF;                       // 多机同步运动标志，false为不启用，true为启用
  cmd[12] =  0x6B;                      // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 13, EMM_UART_TIMEOUT);
}

/**
  * @brief    立即停止（所有控制模式都通用）
  * @param    huart ：串口句柄
  * @param    addr  ：电机地址
  * @param    snF   ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Stop_Now(UART_HandleTypeDef* huart, uint8_t addr, bool snF)
{
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xFE;                       // 功能码
  cmd[2] =  0x98;                       // 辅助码
  cmd[3] =  snF;                        // 多机同步运动标志
  cmd[4] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 5, EMM_UART_TIMEOUT);
}

/**
  * @brief    多机同步运动
  * @param    huart ：串口句柄
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Synchronous_motion(UART_HandleTypeDef* huart, uint8_t addr)
{
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0xFF;                       // 功能码
  cmd[2] =  0x66;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 4, EMM_UART_TIMEOUT);
}

/**
  * @brief    设置单圈回零的零点位置
  * @param    huart ：串口句柄
  * @param    addr  ：电机地址
  * @param    svF   ：是否存储标志，false为不存储，true为存储
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Origin_Set_O(UART_HandleTypeDef* huart, uint8_t addr, bool svF)
{
   uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x93;                       // 功能码
  cmd[2] =  0x88;                       // 辅助码
  cmd[3] =  svF;                        // 是否存储标志，false为不存储，true为存储
  cmd[4] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 5, EMM_UART_TIMEOUT);
}

/**
  * @brief    修改回零参数
  * @param    huart  ：串口句柄
  * @param    addr   ：电机地址
  * @param    svF    ：是否存储标志，false为不存储，true为存储
  * @param    o_mode ：回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
  * @param    o_dir  ：回零方向，0为CW，其余值为CCW
  * @param    o_vel  ：回零速度，单位：RPM（转/分钟）
  * @param    o_tm   ：回零超时时间，单位：毫秒
  * @param    sl_vel ：无限位碰撞回零检测转速，单位：RPM（转/分钟）
  * @param    sl_ma  ：无限位碰撞回零检测电流，单位：Ma（毫安）
  * @param    sl_ms ：无限位碰撞回零检测时间，单位：Ms（毫秒）
  * @param    potF   ：上电自动触发回零，false为不使能，true为使能
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Origin_Modify_Params(UART_HandleTypeDef* huart, uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir, uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF)
{
  uint8_t cmd[32] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x4C;                       // 功能码
  cmd[2] =  0xAE;                       // 辅助码
  cmd[3] =  svF;                        // 是否存储标志，false为不存储，true为存储
  cmd[4] =  o_mode;                     // 回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
  cmd[5] =  o_dir;                      // 回零方向
  cmd[6]  =  (uint8_t)(o_vel >> 8);     // 回零速度(RPM)高8位字节
  cmd[7]  =  (uint8_t)(o_vel >> 0);     // 回零速度(RPM)低8位字节 
  cmd[8]  =  (uint8_t)(o_tm >> 24);     // 回零超时时间(bit24 - bit31)
  cmd[9]  =  (uint8_t)(o_tm >> 16);     // 回零超时时间(bit16 - bit23)
  cmd[10] =  (uint8_t)(o_tm >> 8);      // 回零超时时间(bit8  - bit15)
  cmd[11] =  (uint8_t)(o_tm >> 0);      // 回零超时时间(bit0  - bit7 )
  cmd[12] =  (uint8_t)(sl_vel >> 8);    // 无限位碰撞回零检测转速(RPM)高8位字节
  cmd[13] =  (uint8_t)(sl_vel >> 0);    // 无限位碰撞回零检测转速(RPM)低8位字节 
  cmd[14] =  (uint8_t)(sl_ma >> 8);     // 无限位碰撞回零检测电流(Ma)高8位字节
  cmd[15] =  (uint8_t)(sl_ma >> 0);     // 无限位碰撞回零检测电流(Ma)低8位字节 
  cmd[16] =  (uint8_t)(sl_ms >> 8);     // 无限位碰撞回零检测时间(Ms)高8位字节
  cmd[17] =  (uint8_t)(sl_ms >> 0);     // 无限位碰撞回零检测时间(Ms)低8位字节
  cmd[18] =  potF;                      // 上电自动触发回零，false为不使能，true为使能
  cmd[19] =  0x6B;                      // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 20, EMM_UART_TIMEOUT);
}

/**
  * @brief    触发回零
  * @param    huart  ：串口句柄
  * @param    addr   ：电机地址
  * @param    o_mode ：回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
  * @param    snF    ：多机同步标志，false为不启用，true为启用
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Origin_Trigger_Return(UART_HandleTypeDef* huart, uint8_t addr, uint8_t o_mode, bool snF)
{
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x9A;                       // 功能码
  cmd[2] =  o_mode;                     // 回零模式，0为单圈就近回零，1为单圈方向回零，2为多圈无限位碰撞回零，3为多圈有限位开关回零
  cmd[3] =  snF;                        // 多机同步运动标志，false为不启用，true为启用
  cmd[4] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 5, EMM_UART_TIMEOUT);
}

/**
  * @brief    强制中断并退出回零
  * @param    huart ：串口句柄
  * @param    addr  ：电机地址
  * @retval   地址 + 功能码 + 命令状态 + 校验字节
  */
void Emm_V5_Origin_Interrupt(UART_HandleTypeDef* huart, uint8_t addr)
{
  uint8_t cmd[16] = {0};
  
  // 装载命令
  cmd[0] =  addr;                       // 地址
  cmd[1] =  0x9C;                       // 功能码
  cmd[2] =  0x48;                       // 辅助码
  cmd[3] =  0x6B;                       // 校验字节
  
  // 发送命令
  HAL_UART_Transmit(huart, (uint8_t *)cmd, 5, EMM_UART_TIMEOUT);
}


/**
 * @brief    解析电机返回的数据
 * @param    buffer：接收数据缓冲区
 * @param    len   ：接收数据长度
 * @param    resp  ：解析后的数据结构体
 * @retval   0:解析失败，1:解析成功
 */
uint8_t Emm_V5_Parse_Response(uint8_t *buffer, uint8_t len, Emm_V5_Response_t *resp)
{
  uint8_t i;

  // 参数检查
  if (buffer == NULL || resp == NULL || len < 3)
    return 0; // 校验基本参数

  // 清空结构体
  memset(resp, 0, sizeof(Emm_V5_Response_t)); // 初始化结构体

  // 复制原始数据
  resp->addr = buffer[0]; // 保存地址
  resp->func = buffer[1]; // 保存功能码
  resp->data_len = len;   // 保存数据长度
  for (i = 0; i < len && i < 32; i++)
    resp->raw_data[i] = buffer[i]; // 保存原始数据

  // 根据功能码解析数据
  switch (resp->func)
  {
  case 0x1F: // 读取固件版本
    if (len >= 4)
    {
      for (i = 0; i < len - 3 && i < 15; i++)
        resp->version[i] = buffer[2 + i]; // 复制版本信息
      resp->version[i] = '\0';            // 字符串结束
      resp->valid = 1;                    // 数据有效
    }
    break;

  case 0x20: // 读取相电阻和相电感
    if (len >= 6)
    {
      resp->current = (buffer[2] << 8) | buffer[3]; // 读取电阻
      resp->voltage = (buffer[4] << 8) | buffer[5]; // 这里用voltage字段存储电感
      resp->valid = 1;                              // 数据有效
    }
    break;

  case 0x21: // 读取PID参数
    // PID参数解析需根据实际协议格式进行
    if (len >= 8)
    {
      resp->valid = 1; // 数据有效
    }
    break;

  case 0x24: // 读取总线电压
    if (len >= 4)
    {
      resp->voltage = (buffer[2] << 8) | buffer[3]; // 读取电压
      resp->valid = 1;                              // 数据有效
    }
    break;

  case 0x27: // 读取相电流
    if (len >= 4)
    {
      resp->current = (buffer[2] << 8) | buffer[3]; // 读取电流
      resp->valid = 1;                              // 数据有效
    }
    break;

  case 0x31: // 读取编码器值
    if (len >= 6)
    {
      resp->encoder = (buffer[2] << 24) | (buffer[3] << 16) | (buffer[4] << 8) | buffer[5]; // 读取编码器值
      resp->valid = 1;                                                                      // 数据有效
    }
    break;

  case 0x33: // 读取电机状态
    if (len >= 3)
    {
      resp->status = buffer[2]; // 读取状态
      resp->valid = 1;          // 数据有效
    }
    break;

  case 0x35: // 读取实时转速
    if (len >= 5)
    {
      resp->dir = buffer[2];                      // 方向
      resp->speed = (buffer[3] << 8) | buffer[4]; // 转速
      if (resp->dir)
        resp->speed = -resp->speed; // 根据方向设置正负值
      resp->valid = 1;              // 数据有效
    }
    break;

  case 0x36: // 读取实时位置
    if (len >= 7)
    {
      resp->dir = buffer[2];                                                                  // 方向
      // 完整的32位位置值
      uint32_t full_position = (buffer[3] << 24) | (buffer[4] << 16) | (buffer[5] << 8) | buffer[6];
      // 用于角度计算的16位值（一圈内的位置）
      resp->position = full_position % 65536;
      
      // 根据方向设置正负值
      if (resp->dir)
        resp->position = -resp->position;
      
      resp->valid = 1;                                                                        // 数据有效
    }
    break;

  case 0x37: // 读取位置误差
    if (len >= 7)
    {
      int32_t perr = (buffer[2] << 24) | (buffer[3] << 16) | (buffer[4] << 8) | buffer[5]; // 读取位置误差
      resp->position = perr;                                                               // 使用position字段存储误差值
      resp->valid = 1;                                                                     // 数据有效
    }
    break;

  case 0x39: // 读取错误代码
    if (len >= 3)
    {
      resp->error = buffer[2]; // 读取错误代码
      resp->valid = 1;         // 数据有效
    }
    break;

  case 0x3A: // 读取温度/使能标志
    if (len >= 3)
    {
      resp->status = buffer[2]; // 读取状态标志
      resp->valid = 1;          // 数据有效
    }
    break;

  case 0x3B: // 读取回零状态
    if (len >= 3)
    {
      resp->origin_state = buffer[2]; // 读取回零状态
      resp->valid = 1;                // 数据有效
    }
    break;

  case 0x3D: // 读取目标位置
    if (len >= 6)
    {
      resp->target_pos = (buffer[2] << 24) | (buffer[3] << 16) | (buffer[4] << 8) | buffer[5]; // 读取目标位置
      resp->valid = 1;                                                                         // 数据有效
    }
    break;

  case 0x3E: // 读取目标速度
    if (len >= 4)
    {
      resp->target_speed = (buffer[2] << 8) | buffer[3]; // 读取目标速度
      resp->valid = 1;                                   // 数据有效
    }
    break;

  case 0x3F: // 读取加速度
    if (len >= 3)
    {
      resp->acceleration = buffer[2]; // 读取加速度
      resp->valid = 1;                // 数据有效
    }
    break;

  case 0x40: // 读取细分设置
    if (len >= 3)
    {
      resp->subdivision = buffer[2]; // 读取细分设置
      resp->valid = 1;               // 数据有效
    }
    break;

  case 0x41: // 读取控制模式
    if (len >= 3)
    {
      resp->ctrl_mode = buffer[2]; // 读取控制模式
      resp->valid = 1;             // 数据有效
    }
    break;

  case 0x42: // 读取保护设置
    if (len >= 3)
    {
      resp->protection = buffer[2]; // 读取保护设置
      resp->valid = 1;              // 数据有效
    }
    break;

  case 0x43: // 读取PWM占空比
    if (len >= 4)
    {
      resp->pwm_duty = (buffer[2] << 8) | buffer[3]; // 读取PWM占空比
      resp->valid = 1;                               // 数据有效
    }
    break;

  case 0x44: // 读取闭环状态
    if (len >= 3)
    {
      resp->closed_loop_state = buffer[2]; // 读取闭环状态
      resp->valid = 1;                     // 数据有效
    }
    break;

  case 0x45: // 读取编码器校准状态
    if (len >= 3)
    {
      resp->encoder_state = buffer[2]; // 读取编码器校准状态
      resp->valid = 1;                 // 数据有效
    }
    break;

  case 0x46: // 读取多机同步状态
    if (len >= 3)
    {
      resp->sync_state = buffer[2]; // 读取多机同步状态
      resp->valid = 1;              // 数据有效
    }
    break;

  case 0x47: // 读取回零状态
    if (len >= 3)
    {
      resp->origin_state = buffer[2]; // 读取回零状态
      resp->valid = 1;                // 数据有效
    }
    break;

  default:
    // 其他功能码，仅保存原始数据
    resp->valid = 1; // 数据有效(原始数据已保存)
    break;
  }

  return resp->valid; // 返回解析结果
}

