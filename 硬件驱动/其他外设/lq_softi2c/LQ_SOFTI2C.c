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
#include "LQ_SOFTI2C.h"
#include "include.h"
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】
【功  能】延时函数
【参数值】ms /30M
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void  delayms(unsigned int ms)
{
  unsigned int i;
  do{    
    i=MAIN_Fosc/6000;//大约1毫秒
    while(--i);
  }while(--ms);
}
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void delayus(unsigned int us)
【功  能】延时函数
【参数值】us /30M
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void  delayus(unsigned int us)
{
  unsigned int i;
  do{    
    i=MAIN_Fosc/6000001;//大约1us
    while(--i);
  }while(--us);
}
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void SOFT_IIC_Init(void)
【功  能】初始化I2C对应的接口引脚。
【参数值】无
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void SOFT_IIC_Init(void)
{			
  P2M1 &= 0xCF;
  P2M1 |= 0x00;
  P2M0 &= 0xCF;
  P2M0 |= 0x20;   //SCL设置为推挽输出，SDA设为准双向口
}
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void SOFT_IIC_Start(void)
【功  能】产生IIC起始信号
【参数值】无
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
int SOFT_IIC_Start(void)
{
  //SDA_OUT();     //sda线输出
  SOFT_IIC_SDA=1;
  if(!READ_SDA)return 0;	
  SOFT_IIC_SCL=1;
  delayus(1);
  SOFT_IIC_SDA=0;	//START:when CLK is high,DATA change form high to low 
  delayus(2);	//T(hold,start) > 0.6us
  if(READ_SDA)return 0;	//需要放到延时后面
  SOFT_IIC_SCL=0;	//钳住I2C总线，准备发送或接收数据 
  return 1;
}

/**************************实现函数********************************************
*函数原型:		void SOFT_IIC_Stop(void)
*功　　能:	    //产生IIC停止信号
*******************************************************************************/	  
void SOFT_IIC_Stop(void)
{
  //SDA_OUT();//sda线输出
  SOFT_IIC_SCL=0;
  delayus(1);
  SOFT_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
  delayus(1);
  SOFT_IIC_SCL=1; 
  delayus(1);
  SOFT_IIC_SDA=1;//发送I2C总线结束信号
  delayus(1);							   	
}

/**************************实现函数********************************************
*函数原型:		unsigned char SOFT_IIC_Wait_Ack(void)
*功　　能:	    等待应答信号到来 
//返回值：1，接收应答失败
//        0，接收应答成功
*******************************************************************************/
int SOFT_IIC_Wait_Ack(void)
{
  unsigned char ucErrTime=0;
  //SDA_IN();      //SDA设置为输入  
  //	delay_us(1);
  SOFT_IIC_SDA=1;		// 先释放SDA线
  delayus(2);	   
  SOFT_IIC_SCL=1;
  delayus(1);	 
  while(READ_SDA)
  {
    ucErrTime++; 
    if(ucErrTime>50)
    {
      SOFT_IIC_Stop();
      return 0;
    }
    delayus(1);
  }
  SOFT_IIC_SCL=0;//时钟输出0 	   
  return 1;  
} 

/**************************实现函数********************************************
*函数原型:		void SOFT_IIC_Ack(void)
*功　　能:	    产生ACK应答
*******************************************************************************/
void SOFT_IIC_Ack(void)
{
  SOFT_IIC_SCL=0;
  //SDA_OUT();
  SOFT_IIC_SDA=0;
  delayus(1);
  SOFT_IIC_SCL=1;
  delayus(1);
  SOFT_IIC_SCL=0;
  SOFT_IIC_SDA=1;		//释放SDA线
}

/**************************实现函数********************************************
*函数原型:		void SOFT_IIC_NAck(void)
*功　　能:	    产生NACK应答
*******************************************************************************/	    
void SOFT_IIC_NAck(void)
{
  SOFT_IIC_SCL=0;
  //SDA_OUT();
  delayus(1);
  SOFT_IIC_SDA=1;
  delayus(1);
  SOFT_IIC_SCL=1;
  delayus(2);
  SOFT_IIC_SCL=0;
}
/**************************实现函数********************************************
*函数原型:		void SOFT_IIC_Send_Byte(unsigned char txd)
*功　　能:	    IIC发送一个字节
*******************************************************************************/		  
void SOFT_IIC_Send_Byte(unsigned char txd)
{                        
  unsigned char t;   
  //SDA_OUT(); 	    
  SOFT_IIC_SCL=0;//拉低时钟开始数据传输
  delayus(1);
  for(t=0;t<8;t++)
  {            
    SOFT_IIC_SDA=(txd&0x80)>>7;
    delayus(1);   
    SOFT_IIC_SCL=1;
    txd<<=1; 	  
    delayus(2); 
    SOFT_IIC_SCL=0;	
    delayus(1);  
  }
  SOFT_IIC_SDA = 1;  // 发送完拉高释放SDA
  delayus(1);
} 	 
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】bool i2cWrite(unsigned char addr, unsigned char reg, unsigned char dat)
【功  能】向addr设备的reg地址写len字节数据*dat，与龙邱DMP库接口，必须一致
【参数值】unsigned char addr  设备地址
【参数值】unsigned char reg   要写入的寄存器
【参数值】unsigned int  len   数据长度
【参数值】unsigned char *dat  数据指针
【返回值】0：成功   1：失败 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
int LQ_I2C_Write(unsigned char addr, unsigned char reg, unsigned int len, unsigned char *dat)
{
  unsigned int data i = 0;		// 连这个也要加data
  unsigned int data length = len;
  if (!SOFT_IIC_Start())
    return 1;
  SOFT_IIC_Send_Byte(addr << 1);
  if (!SOFT_IIC_Wait_Ack()) 
  {
    SOFT_IIC_Stop();
    return 2;
  }
  SOFT_IIC_Send_Byte(reg);
  SOFT_IIC_Wait_Ack();
  for (i = 0; i < length; i++) 
  {
    SOFT_IIC_Send_Byte(dat[i]);
    if (!SOFT_IIC_Wait_Ack()) {
      SOFT_IIC_Stop();
      return 3;
    }
    else
      ;
  }
  SOFT_IIC_Stop();
  return 0;
}
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】int LQ_I2C_Read(unsigned char addr, unsigned char reg, unsigned int len, unsigned char *buf)
【功  能】向addr设备的reg地址取len字节数据存放在*buf，与龙邱DMP库接口，必须一致
【参数值】unsigned char addr  设备地址
【参数值】unsigned char reg   要写入的寄存器
【参数值】unsigned int  len   数据长度
【参数值】unsigned char *buf  数据指针
【返回值】0：成功   1：失败  
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
int LQ_I2C_Read(unsigned char addr, unsigned char reg, unsigned int len, unsigned char *buf)
{
  unsigned int data length = len;
  if (SOFT_IIC_Start() == 0)
    return 1;
  SOFT_IIC_Send_Byte((unsigned char)(addr << 1));
  if (SOFT_IIC_Wait_Ack() == 0) 
  {
    SOFT_IIC_Stop();
    return 2;
  }
  SOFT_IIC_Send_Byte(reg);
  SOFT_IIC_Wait_Ack();
  delayus(2);					//应该需要一定延时。
  while(SOFT_IIC_Start() == 0);
  SOFT_IIC_Send_Byte((unsigned char)((addr << 1)+1));
  SOFT_IIC_Wait_Ack();
  while (length) 
  {
    if (length == 1)
      *buf = SOFT_IIC_Read_Byte(0);
    else
      *buf = SOFT_IIC_Read_Byte(1);
    buf++;
    length--;
  }
  SOFT_IIC_Stop();
  return 0;
}


/**************************实现函数********************************************
*函数原型:		unsigned char SOFT_IIC_Read_Byte(unsigned char ack)
*功　　能:	    //读1个字节，ack=1时，发送ACK，ack=0，发送nACK 
*******************************************************************************/  
unsigned char SOFT_IIC_Read_Byte(unsigned char ack)
{
  unsigned char i,receive=0;
  //SDA_IN();//SDA设置为输入
  for(i=0;i<8;i++ )
  {
    SOFT_IIC_SCL=0; 
    delayus(2);
    SOFT_IIC_SCL=1;
    receive<<=1;
    if(READ_SDA)receive++;   
    delayus(2); 
  }					 
  if(ack)
    SOFT_IIC_Ack(); //发送ACK 
  else
    SOFT_IIC_NAck();//发送nACK  
  return receive;
}

/**************************实现函数********************************************
*函数原型:		unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
*功　　能:	    读取指定设备 指定寄存器的一个值
输入	I2C_Addr  目标设备地址
addr	   寄存器地址
返回   读出来的值
*******************************************************************************/ 
unsigned char I2C_ReadOneByte(unsigned char I2C_Addr,unsigned char addr)
{
  unsigned char res=0;
  
  SOFT_IIC_Start();	
  SOFT_IIC_Send_Byte(I2C_Addr);	   //发送写命令
  res++;
  SOFT_IIC_Wait_Ack();
  SOFT_IIC_Send_Byte(addr); res++;  //发送地址
  SOFT_IIC_Wait_Ack();	  
  //SOFT_IIC_Stop();//产生一个停止条件	
  SOFT_IIC_Start();
  SOFT_IIC_Send_Byte((unsigned char)(I2C_Addr+1)); res++;          //进入接收模式			   
  SOFT_IIC_Wait_Ack();
  res=SOFT_IIC_Read_Byte(0);	   
  SOFT_IIC_Stop();//产生一个停止条件
  
  return res;
}


/**************************实现函数********************************************
*函数原型:		unsigned char IICreadBytes(unsigned char dev, unsigned char reg, unsigned char length, unsigned char *dat)
*功　　能:	    读取指定设备 指定寄存器的 length个值
输入	dev  目标设备地址
reg	  寄存器地址
length 要读的字节数
*dat  读出的数据将要存放的指针
返回   读出来的字节数量
*******************************************************************************/ 
unsigned char IICreadBytes(unsigned char dev, unsigned char reg, unsigned char length, unsigned char *dat)
{
  unsigned char count = 0;
  
  SOFT_IIC_Start();
  SOFT_IIC_Send_Byte(dev);	   //发送写命令
  SOFT_IIC_Wait_Ack();
  SOFT_IIC_Send_Byte(reg);   //发送地址
  SOFT_IIC_Wait_Ack();	  
  SOFT_IIC_Start();
  SOFT_IIC_Send_Byte((unsigned char)(dev+1));  //进入接收模式	
  SOFT_IIC_Wait_Ack();
  
  for(count=0;count<length;count++){
    
    if(count!=length-1)dat[count]=SOFT_IIC_Read_Byte(1);  //带ACK的读数据
    else  dat[count]=SOFT_IIC_Read_Byte(0);	 //最后一个字节NACK
  }
  SOFT_IIC_Stop();//产生一个停止条件
  return count;
}

/**************************实现函数********************************************
*函数原型:		unsigned char IICwriteBytes(unsigned char dev, unsigned char reg, unsigned char length, unsigned char* dat)
*功　　能:	    将多个字节写入指定设备 指定寄存器
输入	dev  目标设备地址
reg	  寄存器地址
length 要写的字节数
*dat  将要写的数据的首地址
返回   返回是否成功
*******************************************************************************/ 
unsigned char IICwriteBytes(unsigned char dev, unsigned char reg, unsigned char length, unsigned char* dat){
  
  unsigned char count = 0;
  SOFT_IIC_Start();
  SOFT_IIC_Send_Byte(dev);	   //发送写命令
  SOFT_IIC_Wait_Ack();
  SOFT_IIC_Send_Byte(reg);   //发送地址
  SOFT_IIC_Wait_Ack();	  
  for(count=0;count<length;count++){
    SOFT_IIC_Send_Byte(dat[count]); 
    SOFT_IIC_Wait_Ack(); 
  }
  SOFT_IIC_Stop();//产生一个停止条件
  
  return 1; //status == 0;
}

/**************************实现函数********************************************
*函数原型:		unsigned char IICreadByte(unsigned char dev, unsigned char reg, unsigned char *dat)
*功　　能:	    读取指定设备 指定寄存器的一个值
输入	dev  目标设备地址
reg	   寄存器地址
*dat  读出的数据将要存放的地址
返回   1
*******************************************************************************/ 
unsigned char IICreadByte(unsigned char dev, unsigned char reg, unsigned char *dat)
{
  *dat=I2C_ReadOneByte(dev, reg);
  return 1;
}

/**************************实现函数********************************************
*函数原型:		unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char dat)
*功　　能:	    写入指定设备 指定寄存器一个字节
输入	dev  目标设备地址
reg	   寄存器地址
dat  将要写入的字节
返回   1
*******************************************************************************/ 
unsigned char IICwriteByte(unsigned char dev, unsigned char reg, unsigned char dat)
{
  return IICwriteBytes(dev, reg, 1, &dat);
}

/**************************实现函数********************************************
*函数原型:		unsigned char IICwriteBits(unsigned char dev,unsigned char reg,unsigned char bitStart,unsigned char length,unsigned char dat)
*功　　能:	    读 修改 写 指定设备 指定寄存器一个字节 中的多个位
输入	dev  目标设备地址
reg	   寄存器地址
bitStart  目标字节的起始位
length   位长度
dat    存放改变目标字节位的值
返回   成功 为1 
失败为0
*******************************************************************************/ 
unsigned char IICwriteBits(unsigned char dev,unsigned char reg,unsigned char bitStart,unsigned char length,unsigned char dat)
{
  
  unsigned char b;
  if (IICreadByte(dev, reg, &b) != 0) {
    unsigned char mask = (0xFF << (bitStart + 1)) | 0xFF >> ((8 - bitStart) + length - 1);
    dat <<= (8 - length);
    dat >>= (7 - bitStart);
    b &= mask;
    b |= dat;
    return IICwriteByte(dev, reg, b);
  } else {
    return 0;
  }
}

/**************************实现函数********************************************
*函数原型:		unsigned char IICwriteBit(unsigned char dev, unsigned char reg, unsigned char bitNum, unsigned char dat)
*功　　能:	    读 修改 写 指定设备 指定寄存器一个字节 中的1个位
输入	dev  目标设备地址
reg	   寄存器地址
bitNum  要修改目标字节的bitNum位
dat  为0 时，目标位将被清0 否则将被置位
返回   成功 为1 
失败为0
*******************************************************************************/ 
unsigned char IICwriteBit(unsigned char dev, unsigned char reg, unsigned char bitNum, unsigned char dat)
{
  unsigned char b;
  IICreadByte(dev, reg, &b);
  b = (dat != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
  return IICwriteByte(dev, reg, b);
}

//------------------End of File----------------------------
