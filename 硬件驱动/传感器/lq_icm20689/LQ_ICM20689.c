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
#include "include.h"

#define USE_SOFT_SPI //使用模拟SPI,否则使用硬件SPI
#define PI 3.1415926

float temp_p=0;

//                  单片机    龙邱20689模块
#define SPI_SCK      P25      //SCK
#define SPI_MISO     P23      //单片机MISO接模块--SDO
#define SPI_MOSI     P24      //单片机MOSI接模块--SDI
#define SPI_CS       P22      //CS

#define SPI_SCK_OUT       P25
#define SPI_MISO_IN       P23
#define SPI_MOSI_OUT      P24
#define SPI_CS_OUT        P22
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【作  者】chiusir
【功能说明】读取 ICM20689 的原始数据
【软件版本】V1.0
【最后更新】2020年12月21日 
【函数名】
【返回值】无 0：初始化成功   1：失败
【参数值】无 
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/

void Test_ICM20689(void)
{
  char  txt[16];
  short data aacx,aacy,aacz;	        //加速度传感器原始数据
  short data gyrox,gyroy,gyroz;        //陀螺仪原始数据 
  
  OLED_CLS();                   //LCD清屏
  OLED_P6x8Str(1,0,"LQ20689M Test"); 
  delayms(100);
  if(ICM20689_Init())
  { 		
    OLED_P6x8Str(1,2,"LQ20689M Fail");
    while(1);
  }
  
  while(1)
  {
		//测试原始数值
    ICM_Get_Raw_data(&aacx,&aacy,&aacz,&gyrox,&gyroy,&gyroz);	//得到加速度传感器数据  
		ANO_DT_send_int16(aacx,aacy,aacz,gyrox,gyroy,gyroz,0,0);//发送数据到匿名地面站上位机
		
    sprintf((char*)txt,"angle:%f",atan2((float)aacz,(float)aacx)*180.0/PI); //反正切加速度计得到角度值
    OLED_P6x8Str(0,2,(u8*)txt);

		/*
		Kalman_Filter((float)aacz,(float)(0-gyroy)); //卡尔曼
		sprintf((char*)txt,"g_angle:%f",Angle); 
    OLED_P6x8Str(0,2,(u8*)txt);
    sprintf((char*)txt,"Ang_Y:%f",angle_dot);//X轴加速度值											
    OLED_P6x8Str(0,3,(u8*)txt);
		*/
		/*
    sprintf((char*)txt,"ax:%06d",aacx);	 //X轴加速度值											
    OLED_P6x8Str(0,2,(u8*)txt);
    sprintf((char*)txt,"ay:%06d",aacy);	 //Y轴加速度值
    OLED_P6x8Str(0,3,(u8*)txt);
    sprintf((char*)txt,"az:%06d",aacz);  //Z轴加速度值
    OLED_P6x8Str(0,4,(u8*)txt);
    sprintf((char*)txt,"gx:%06d",gyrox); //X轴角度值
    OLED_P6x8Str(0,5,(u8*)txt);
    sprintf((char*)txt,"gy:%06d",gyroy); //Y轴角度值
    OLED_P6x8Str(0,6,(u8*)txt);
    sprintf((char*)txt,"gz:%06d",gyroz); //Z轴角度值
    OLED_P6x8Str(0,7,(u8*)txt);
		*/
		/*
		//测试简单融合结果
    g_angle = ICM20689_ACC_angle_X_Z();
    ICM20689_GYRO_Y_Ang();
    ICM20689_GYRO_ACCEL_Ang_Y();
    sprintf((char*)txt,"g_angle:%f",g_angle); 
    OLED_P6x8Str(0,2,(u8*)txt);
    sprintf((char*)txt,"Ang_Y:%f",GYRO_Ang_Y);//X轴加速度值											
    OLED_P6x8Str(0,3,(u8*)txt);
    */
    delayms(100);    
		LED_Ctrl(LEDALL, RVS); 
  }  
}

/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【作  者】chiusir
【功能说明】初始化 ICM20689
【软件版本】V1.0
【最后更新】2020年12月21日 
【函数名】
【返回值】无 0：初始化成功   1：失败
【参数值】无 
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
u8 ICM20689_Init(void)
{
  char  txt[10];
  u8 res;
  
  SPI_SoftInit(); 
  delayms(100);  
  res=ICM_Read_Byte(WHO_AM_I);                        //读取ICM20689的ID  
  sprintf(txt,"ID:0x%02X",res);
  OLED_P6x8Str(1,1,txt); 

  if(res!=ICM20689_ID)                                 //器件ID正确
  {    
    //printf("ID=%#X\r\n",res);
    //OLED_P6x8Str(1,4,"ID is fail!");
    return 1;
  }
  else  
  {
    //printf("ICM20689 is OK!\n");  
  }    
  
  ICM_Write_Byte(ICM_PWR_MGMT1_REG,0X80);//复位
  delayms(100);  //延时100ms
  ICM_Write_Byte(ICM_PWR_MGMT1_REG,0X00);//唤醒
  delayms(100);  //延时100ms
  
  ICM_Set_Gyro_Fsr(3);			 //陀螺仪传感器,±2000dps   
  ICM_Set_Accel_Fsr(1);			 //加速度传感器,0:±2g   1:±4g    2:±8g
  ICM_Set_Rate(1000);			   //设置采样率1000Hz
  //ICM_Write_Byte(ICM_CFG_REG,0x02);    //设置数字低通滤波器   98hz
  ICM_Write_Byte(ICM_CFG_REG,4);	       //低通滤波频率，典型值：0x07(3600Hz)此寄存器内决定Internal_Sample_Rate==8K
  ICM_Write_Byte(ICM_INT_EN_REG,0X00);   //关闭所有中断
  ICM_Write_Byte(ICM_FIFO_EN_REG,0X00);	 //关闭FIFO
  ICM_Write_Byte(ICM_USER_CTRL_REG,0X00);//I2C主模式关闭
  ICM_Write_Byte(ICM_PWR_MGMT1_REG,0X01);//设置CLKSEL,PLL X轴为参考
  ICM_Write_Byte(ICM_PWR_MGMT2_REG,0X00);//加速度与陀螺仪都工作 
  
  return 0;
}

/*************************************************************************
*  函数名称：void SPI_SoftInit(void)
*  功能说明：模拟SPI口初始化
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void SPI_SoftInit(void)
{
	#ifdef USE_SPI
			SPI_init();
	#elif defined USE_SOFT_SPI
			P2M1 = 0; P2M0 = 0;  //设置为准双向口
			P2PU =0x3C;          //上拉
	#endif
}
/*************************************************************************
*  函数名称：void SPI_SoftReadWriteNbyte(u8 *lqbuff, u16 len)
*  功能说明：SPI读写数据及长度
*  参数说明：u8 *buf数据指针,u16 len长度
*  函数返回：
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void SPI_SoftReadWriteNbyte(u8 *lqbuff, u16 len)
{
	u8 i;
	
  SPI_CS_OUT = 0;
  SPI_SCK_OUT= 1;
  do
  {
    for(i = 0; i < 8; i++)
    {
      SPI_MOSI_OUT = ((*lqbuff) >= 0x80);
      SPI_SCK_OUT= 0;
      (*lqbuff) = (*lqbuff)<<1;      
      
      SPI_SCK_OUT= 1;
      
      (*lqbuff) |= SPI_MISO_IN;          
    }
    lqbuff++;
  }while(--len);
  SPI_CS_OUT = 1;
}
/*************************************************************************
*  函数名称：void ICM_Read_Len(u8 reg,u8 len,u8 *buf)
*  功能说明：u8 reg起始寄存器,u8 *buf数据指针,u16 len长度
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Read_Len(u8 reg,u8 len,u8 *buf)
{  
#ifdef USE_SOFT_SPI
  buf[0] = reg | 0x80;  
  SPI_SoftReadWriteNbyte(buf, len + 1);  
#else
  SPI_Read_Nbytes(reg|0x80, buf, len);
#endif
}
/*************************************************************************
*  函数名称：void ICM_Get_Gyroscope(short *gx,short *gy,short *gz)
*  功能说明：
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Get_Gyroscope(short *gx,short *gy,short *gz)
{
  u8 buf[7]; 
  ICM_Read_Len(ICM_GYRO_XOUTH_REG,6,buf);
  
  *gx=((u16)buf[1]<<8)|buf[2];  
  *gy=((u16)buf[3]<<8)|buf[4];  
  *gz=((u16)buf[5]<<8)|buf[6];
  
}
/*************************************************************************
*  函数名称：void ICM_Get_Accelerometer(short *ax,short *ay,short *az)
*  功能说明：
*  参数说明：无
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Get_Accelerometer(short *ax,short *ay,short *az)
{
  u8 buf[7];  
  ICM_Read_Len(ICM_ACCEL_XOUTH_REG,6,buf);
  
  *ax=((u16)buf[1]<<8)|buf[2];  
  *ay=((u16)buf[3]<<8)|buf[4];  
  *az=((u16)buf[5]<<8)|buf[6];  
}

/*************************************************************************
*  函数名称：void ICM_Get_Raw_data(short *ax,short *ay,short *az,short *gx,short *gy,short *gz)
*  功能说明：读取加速度陀螺仪数据
*  参数说明：六轴
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Get_Raw_data(short *ax,short *ay,short *az,short *gx,short *gy,short *gz)
{
  u8 buf[16];  
  ICM_Read_Len(ICM_ACCEL_XOUTH_REG,14,buf);
  
  *ax=((u16)buf[1]<<8)|buf[2];  
  *ay=((u16)buf[3]<<8)|buf[4];  
  *az=((u16)buf[5]<<8)|buf[6];
  *gx=((u16)buf[9]<<8)|buf[10];  
  *gy=((u16)buf[11]<<8)|buf[12];  
  *gz=((u16)buf[13]<<8)|buf[14];
}

/*************************************************************************
*  函数名称：void ICM_Write_Byte(u8 reg,u8 value)
*  功能说明：向寄存器写数据
*  参数说明：u8 reg寄存器,u8 value数据
*  函数返回：无
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Write_Byte(u8 reg,u8 value)
{
#ifdef USE_SOFT_SPI
  uint8_t buff[2];  
  buff[0] = reg;          				//先发送寄存器
  buff[1] = value;       				  //再发送数据  
  SPI_SoftReadWriteNbyte(buff, 2);//发送buff里数据，并采集到 buff里  
  
#else
  SPI_CS=0;
  SPI_WriteByte(reg|0x80);
  SPI_WriteByte(value);
  SPI_CS=1;
#endif
}

/*************************************************************************
*  函数名称：void ICM_Write_Byte(u8 reg,u8 value)
*  功能说明：向寄存器写数据
*  参数说明：u8 reg寄存器,
*  函数返回：u8 value数据
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
u8 ICM_Read_Byte(u8 reg)
{  
  u8 buff[2];
#ifdef USE_SOFT_SPI
  buff[0] = reg | 0x80;    //先发送寄存器  
  SPI_SoftReadWriteNbyte(buff, 2);   
  return buff[1];
#else
  SPI_CS=0;
  SPI_WriteByte(reg| 0x80);  
  buff[0]=SPI_ReadByte();
  SPI_CS=1;
  return buff[0];
#endif  

}
/*************************************************************************
*  函数名称：void ICM_Set_Gyro_Fsr(u8 fsr)
*  功能说明：设置陀螺仪传感器满量程范围
*  参数说明：Fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
*  函数返回：
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Set_Gyro_Fsr(u8 fsr)
{
  ICM_Write_Byte(ICM_GYRO_CFG_REG,fsr<<3);//设置陀螺仪满量程范围  
}
/*************************************************************************
*  函数名称：void ICM_Set_Accel_Fsr(u8 fsr)
*  功能说明：设置LQ20689陀螺仪传感器满量程范围
*  参数说明：fsr:0,±2g;1,±4g;2,±8g;3,±16g
*  函数返回：
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Set_Accel_Fsr(u8 fsr)
{
  ICM_Write_Byte(ICM_ACCEL_CFG_REG,fsr<<3);//设置加速度传感器满量程范围  
}
/*************************************************************************
*  函数名称：void ICM_Set_LPF(u16 lpf)
*  功能说明：设置LQ20689数字低通滤波器
*  参数说明：lpf:数字低通滤波频率(Hz)
*  函数返回：
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Set_LPF(u16 lpf)
{
  u8 dat=0;
  if(lpf>=188)dat=1;
  else if(lpf>=98)dat=2;
  else if(lpf>=42)dat=3;
  else if(lpf>=20)dat=4;
  else if(lpf>=10)dat=5;
  else dat=6; 
  ICM_Write_Byte(ICM_CFG_REG,dat);//设置数字低通滤波器  
}
/*************************************************************************
*  函数名称：void ICM_Set_Rate(u16 rate)
*  功能说明：设置LQ20689陀螺仪传感器满量程范围
*  参数说明：rate:4~1000(Hz)
*  函数返回：
*  修改时间：2020年8月10日
*  备    注：   
*************************************************************************/
void ICM_Set_Rate(u16 rate)
{
  u8 dat;
  if(rate>1000)rate=1000;
  if(rate<4)rate=4;
  dat=1000/rate-1;
  ICM_Write_Byte(ICM_SAMPLE_RATE_REG,dat);	//设置数字低通滤波器
  ICM_Set_LPF(rate/2);	//自动设置LPF为采样率的一半
}
/*************************************************************************
*  函数名称：u16 ICM_Get_Temperature(void)
*  功能说明：得到温度值
*  参数说明：
*  函数返回：返回值:温度值(扩大了100倍)
*  修改时间：2020年8月10日
*  备    注：返回值:温度值(扩大了100倍)   
*************************************************************************/
u16 ICM_Get_Temperature(void)
{
  u8 buf[3]; 
  u16 raw;
  float temp;
  ICM_Read_Len(ICM_TEMP_OUTH_REG,2,buf); 
  raw=((u16)buf[0]<<8)|buf[1];  
  temp=21+((double)raw)/333.87;  
  return (u16)temp*100;
}


//**********************************************************************************************************************************************************
void icm20689_reset()
{
  ICM_Write_Byte(ICM_PWR_MGMT1_REG,0X80); 	//复位
  delayms(100);
  ICM_Write_Byte(ICM_PWR_MGMT1_REG, 0x00);	//解除休眠状态
}

//***************反正切Y轴角度*************************//
/*
 * 函数名：ICM20689_ACC_angle_X_Z
 * 描述  ：加速度计反正切角度
 * 输入  ：无
 * 输出  ：角度值
 * 调用  ：内部调用
 */
/*
float ICM20689_ACC_angle_X_Z(void)                       
{
	short a_x,a_y,a_z;
  ICM_Get_Accelerometer((short)&a_x,(short)&a_y,(short)&a_z);
  return (atan2((float)a_z,(float)a_x)*(180.0/PI));   //反正切加速度计得到角度值
}
*/
//**************积分Y轴角度*************************//
/*
 * 函数名：ICM20689_GYRO_Y_Ang(void)
 * 描述  ：积分Y轴角度
 * 输入  ：无
 * 输出  ：无
 * 调用  ：内部调用
 */
 /*
void ICM20689_GYRO_Y_Ang(void)
{
	short g_x,g_y,g_z;
  ICM_Get_Gyroscope((u16)&g_x,(u16)&g_y,&g_z);   //获取Y轴陀螺仪值
  GYRO_Speed_Y= ((float)g_y)*0.00032;
  GYRO_Ang_Y+=GYRO_Speed_Y;  //积分角度
}
*/
//***************加速度计和陀螺仪数据融合*****************************//
/*
void ICM20689_GYRO_ACCEL_Ang_Y(void)
{	
  ICM20689_GYRO_Y_Ang();
  ACCEL_Ang_Y=ICM20689_ACC_angle_X_Z();
  temp_p=(ACCEL_Ang_Y-GYRO_Ang_Y)*0.025;//融合系数  //0.025
  GYRO_Ang_Y=GYRO_Ang_Y+temp_p;  
  ACC_GYRO_Ang_Y=GYRO_Ang_Y;
}
*/
