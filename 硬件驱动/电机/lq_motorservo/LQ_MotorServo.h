/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【平    台】北京龙邱智能科技STC16核心板
【编    写】chiusir
【E-mail  】chiusir@163.com
【软件版本】V1.1 版权所有，单位使用请先联系授权
【最后更新】2021年1月25日
【相关信息参考下列地址】
【网    站】http://www.lqist.cn
【淘宝店铺】http://longqiu.taobao.com
------------------------------------------------
【IDE】STC16
【Target 】 C251Keil5.57及以上
【SYS PLL】 30MHz使用内部晶振
=================================================================
STC16相关配套视频：
龙邱科技B站网址：https://space.bilibili.com/95313236
STC16环境下载参考视频： https://www.bilibili.com/video/BV1gy4y1p7T1/
STC16一体板子介绍视频： https://www.bilibili.com/video/BV1Jy4y1e7R4/
=================================================================
下载时, 选择时钟 30MHZ (用户可自行修改频率).
STC16F初次下载:先用IRCBND=0，ISP界面设置为24M，
芯片最后一行2038...然后IRCBND=0，下载频率为30M；
芯片最后一行2010...或者IRCBND=1，下载频率为30M；
官方出库批次不同，略有差异，好用为准！！！
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/

#ifndef __LQ_MS_H_
#define __LQ_MS_H_

#include "include.h"

//电机频率
#define MOTOR_FREQUENCY    12500
#define PWM1_CLOCK         1000000//1M
#define PWM2_CLOCK         10000000 //10M

#define Servo_Delta  260       //舵机左右转动的差值，与舵机型号，拉杆和舵机臂长有关
#define Servo_Center 1690     //舵机直行中值，前轮向左偏移要减小，向右偏移要加大
#define Servo_Left_Max   (Servo_Center+Servo_Delta)      //舵机左转极限值
#define Servo_Right_Min  (Servo_Center-Servo_Delta)      //舵机右转极限值，此值跟舵机放置方式有关，立式

void MotorInit(u16 freq);
void MotorCtrl (int16_t motor1,int16_t motor2);
void TestMotor(void);
void ServoInit(u16 freq);
void ServoCtrl(uint16_t duty);
void TestServo(void);
#endif