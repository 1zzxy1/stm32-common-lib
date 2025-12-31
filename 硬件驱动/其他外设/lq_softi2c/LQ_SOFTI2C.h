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
#ifndef __IOI2C_H
#define __IOI2C_H

#include "LQ_SOFTI2C.h"

//IO方向设置
//#define SDA_IN()  {P2M1 = 1;P2M0 = 0;}
//#define SDA_OUT() {P2M1 = 0;P2M0 = 0;}

//IO操作函数	 
#define SOFT_IIC_SCL    P25      //SCK
#define SOFT_IIC_SDA    P24      //单片机MOSI接模块--SDI
#define READ_SDA   P24      //单片机MOSI接模块--SDI  

//IIC所有操作函数
void SOFT_IIC_Init(void);                //初始化IIC的IO口				 
int  SOFT_IIC_Start(void);				//发送IIC开始信号
void SOFT_IIC_Stop(void);	  			//发送IIC停止信号
void SOFT_IIC_Send_Byte(unsigned char txd);			//IIC发送一个字节
unsigned char   SOFT_IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
int  SOFT_IIC_Wait_Ack(void); 				//IIC等待ACK信号
void SOFT_IIC_Ack(void);					//IIC发送ACK信号
void SOFT_IIC_NAck(void);				//IIC不发送ACK信号

void SOFT_IIC_Write_One_Byte(unsigned char daddr,unsigned char addr,unsigned char dat);
unsigned char SOFT_IIC_Read_One_Byte(unsigned char daddr,unsigned char addr);	 
unsigned char I2C_Readkey(unsigned char I2C_Addr);

unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr);
unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char dat);
unsigned char IICwriteBytes(unsigned char dev, unsigned char reg, unsigned char length, unsigned char* dat);
unsigned char IICwriteBits(unsigned char dev,unsigned char reg,unsigned char bitStart,unsigned char length,unsigned char dat);
unsigned char IICwriteBit(unsigned char dev,unsigned char reg,unsigned char bitNum,unsigned char dat);
unsigned char IICreadBytes(unsigned char dev, unsigned char reg, unsigned char length, unsigned char *dat);

int LQ_I2C_Write(unsigned char addr, unsigned char reg, unsigned int len, unsigned char *dat);
int LQ_I2C_Read(unsigned char addr, unsigned char reg, unsigned int len, unsigned char *buf);
void delayms(unsigned int ms);
void delayus(unsigned int us);

#endif

//------------------End of File----------------------------
