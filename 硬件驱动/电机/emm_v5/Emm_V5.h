#ifndef __EMM_V5_H
#define __EMM_V5_H

#include "usart.h"
#include "stdbool.h"
#include "string.h"

#define   EMM_UART_TIMEOUT  1000  /* 串口发送超时时间(ms) */

/**********************************************************
***	Emm_V5.0步进闭环控制例程
***	编写作者：ZHANGDATOU
***	技术支持：张大头闭环伺服
***	淘宝店铺：https://zhangdatou.taobao.com
***	CSDN博客：http s://blog.csdn.net/zhangdatou666
***	qq交流群：262438510
**********************************************************/

#define		ABS(x)		((x) > 0 ? (x) : -(x)) 

typedef enum {
	S_VER   = 0,			/* 读取固件版本和对应的硬件版本 */
	S_RL    = 1,			/* 读取读取相电阻和相电感 */
	S_PID   = 2,			/* 读取PID参数 */
	S_VBUS  = 3,			/* 读取总线电压 */
	S_CPHA  = 5,			/* 读取相电流 */
	S_ENCL  = 7,			/* 读取经过线性化校准后的编码器值 */
	S_TPOS  = 8,			/* 读取电机目标位置角度 */
	S_VEL   = 9,			/* 读取电机实时转速 */
	S_CPOS  = 10,			/* 读取电机实时位置角度 */
	S_PERR  = 11,			/* 读取电机位置误差角度 */
	S_FLAG  = 13,			/* 读取使能/到位/堵转状态标志位 */
	S_Conf  = 14,			/* 读取驱动参数 */
	S_State = 15,			/* 读取系统状态参数 */
	S_ORG   = 16,     /* 读取正在回零/回零失败状态标志位 */
}SysParams_t;

/* 电机返回数据结构 */
typedef struct {
  uint8_t  addr;                  /* 电机地址 */
  uint8_t  func;                  /* 功能码 */
  uint8_t  dir;                   /* 方向，0为CW，1为CCW */
  int16_t  speed;                 /* 实时速度(RPM) */
  int32_t  position;              /* 实时位置(脉冲数) */
  uint8_t  status;                /* 电机状态 */
  uint8_t  error;                 /* 错误代码 */
  int32_t  encoder;               /* 编码器位置 */
  int16_t  temperature;           /* 温度 */
  uint16_t voltage;               /* 母线电压(V) */
  uint16_t current;               /* 相电流(mA) */
  int32_t  target_pos;            /* 目标位置 */
  uint16_t target_speed;          /* 目标速度 */
  uint8_t  acceleration;          /* 加速度 */
  uint8_t  subdivision;           /* 细分设置 */
  uint8_t  ctrl_mode;             /* 控制模式 */
  uint8_t  protection;            /* 保护设置 */
  uint16_t pwm_duty;              /* PWM占空比 */
  uint8_t  closed_loop_state;     /* 闭环状态 */
  uint8_t  encoder_state;         /* 编码器校准状态 */
  uint8_t  sync_state;            /* 多机同步状态 */
  uint8_t  origin_state;          /* 回零状态 */
  char     version[16];           /* 固件版本 */
  uint8_t  raw_data[32];          /* 原始数据 */
  uint8_t  data_len;              /* 数据长度 */
  uint8_t  valid;                 /* 数据有效标志 */
} Emm_V5_Response_t;


/**********************************************************
*** 注意：每个函数的参数的具体说明，请查阅对应函数的注释说明
**********************************************************/
void Emm_V5_Reset_CurPos_To_Zero(UART_HandleTypeDef* huart, uint8_t addr); // 将当前位置清零
void Emm_V5_Reset_Clog_Pro(UART_HandleTypeDef* huart, uint8_t addr); // 解除堵转保护
void Emm_V5_Read_Sys_Params(UART_HandleTypeDef* huart, uint8_t addr, SysParams_t s); // 读取参数
void Emm_V5_Modify_Ctrl_Mode(UART_HandleTypeDef* huart, uint8_t addr, bool svF, uint8_t ctrl_mode); // 发送命令修改开环/闭环控制模式
void Emm_V5_En_Control(UART_HandleTypeDef* huart, uint8_t addr, bool state, bool snF); // 电机使能控制
void Emm_V5_Vel_Control(UART_HandleTypeDef* huart, uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF); // 速度模式控制
void Emm_V5_Pos_Control(UART_HandleTypeDef* huart, uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF); // 位置模式控制
void Emm_V5_Stop_Now(UART_HandleTypeDef* huart, uint8_t addr, bool snF); // 让电机立即停止运动
void Emm_V5_Synchronous_motion(UART_HandleTypeDef* huart, uint8_t addr); // 触发多机同步开始运动
void Emm_V5_Origin_Set_O(UART_HandleTypeDef* huart, uint8_t addr, bool svF); // 设置挡圈回零的零点位置
void Emm_V5_Origin_Modify_Params(UART_HandleTypeDef* huart, uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir, uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF); // 修改回零参数
void Emm_V5_Origin_Trigger_Return(UART_HandleTypeDef* huart, uint8_t addr, uint8_t o_mode, bool snF); // 发送命令触发回零
void Emm_V5_Origin_Interrupt(UART_HandleTypeDef* huart, uint8_t addr); // 强制中断并退出回零
uint8_t Emm_V5_Parse_Response(uint8_t *buffer, uint8_t len, Emm_V5_Response_t *resp);

#endif
