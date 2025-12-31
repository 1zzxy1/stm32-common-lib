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
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ

配合匿名地面站上位机

QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
#include <LQ_UART.h>
#include <ANO_DT.h>

//使用匿名4.3上位机协议
/////////////////////////////////////////////////////////////////////////////////////
//数据拆分宏定义，在发送大于1字节的数据类型时，比如int16、float等，需要把数据拆分成单独字节进行发送
#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)	  ) )     /*!< uint32_t 数据拆分 byte0  */
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )     /*!< uint32_t 数据拆分 byte1  */
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )     /*!< uint32_t 数据拆分 byte2  */
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )     /*!< uint32_t 数据拆分 byte3  */

/*!
  * @brief    Send_Data函数是协议中所有发送数据功能使用到的发送函数
  *
  * @param    dataToSend   :   要发送的数据首地址
  * @param    length       :   要发送的数据长度
  *
  * @return   无
  *
  * @note     移植时，用户应根据自身应用的情况，根据使用的通信方式，实现此函数
  *
  * @see      内部调用
  *
  * @date     2019/5/28 星期二
  */
void ANO_DT_Send_Data(unsigned char *dataToSend , unsigned short length)
{

    /**使用串口正常发送数据，大概需要1.5ms*/
	UART4_PutBuff(dataToSend, length);     //可以修改不同的串口发送数据

}
/**  发送数据缓存 */
unsigned char data_to_send[50];

/*!
  * @brief    向上位机发送发送8个int16_t数据
  *
  * @param    data1 - data8  ： 发送给上位机显示波形
  *
  * @return   无
  *
  * @note     无
  *
  * @see      ANO_DT_send_int16(1, 2, 3, 0, 0, 0, 0, 0);
  *
  * @date     2019/5/28 星期二
  */
void ANO_DT_send_int16(short data1, short data2, short data3, short data4, short data5, short data6, short data7, short data8 )
{
  unsigned char  _cnt=0;
	unsigned char  sum = 0, i = 0;
	
  data_to_send[_cnt++] = 0xAA;      //匿名协议帧头  0xAAAA
	data_to_send[_cnt++] = 0xAA;
	data_to_send[_cnt++] = 0xF1;      //使用用户协议帧0xF1
  data_to_send[_cnt++] = 16;        //8个int16_t 长度 16个字节

	data_to_send[_cnt++]=BYTE1(data1);
	data_to_send[_cnt++]=BYTE0(data1);

	data_to_send[_cnt++]=BYTE1(data2);
	data_to_send[_cnt++]=BYTE0(data2);

	data_to_send[_cnt++]=BYTE1(data3);
	data_to_send[_cnt++]=BYTE0(data3);

  data_to_send[_cnt++]=BYTE1(data4);
	data_to_send[_cnt++]=BYTE0(data4);

	data_to_send[_cnt++]=BYTE1(data5);
	data_to_send[_cnt++]=BYTE0(data5);

	data_to_send[_cnt++]=BYTE1(data6);
	data_to_send[_cnt++]=BYTE0(data6);

  data_to_send[_cnt++]=BYTE1(data7);
	data_to_send[_cnt++]=BYTE0(data7);

	data_to_send[_cnt++]=BYTE1(data8);
	data_to_send[_cnt++]=BYTE0(data8);

  sum = 0;
	for(i=0;i<_cnt;i++)
		sum += data_to_send[i];
	data_to_send[_cnt++]=sum;	
	
	ANO_DT_Send_Data(data_to_send ,_cnt);
}
