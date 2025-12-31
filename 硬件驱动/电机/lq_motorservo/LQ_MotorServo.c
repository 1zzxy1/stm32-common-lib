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

电机PWM和舵机PWM相关函数

QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
#include "include.h"

#define USEDRV8701     //使用不同的驱动，定义不同的宏定义
//#define USE7843OR7971//使用不同的驱动，定义不同的宏定义

#define PWM1_0      0x00	//P:P1.0  N:P1.1
#define PWM1_1      0x01	//P:P2.0  N:P2.1
#define PWM1_2      0x02	//P:P6.0  N:P6.1

#define PWM2_0      0x00	//P:P1.2/P5.4  N:P1.3
#define PWM2_1      0x04	//P:P2.2  N:P2.3
#define PWM2_2      0x08	//P:P6.2  N:P6.3

#define PWM3_0      0x00	//P:P1.4  N:P1.5
#define PWM3_1      0x10	//P:P2.4  N:P2.5
#define PWM3_2      0x20	//P:P6.4  N:P6.5

#define PWM4_0      0x00	//P:P1.6  N:P1.7
#define PWM4_1      0x40	//P:P2.6  N:P2.7
#define PWM4_2      0x80	//P:P6.6  N:P6.7
#define PWM4_3      0xC0	//P:P3.4  N:P3.3

#define PWM5_0      0x00	//P2.0
#define PWM5_1      0x01	//P1.7
#define PWM5_2      0x02	//P0.0
#define PWM5_3      0x03	//P7.4

#define PWM6_0      0x00	//P2.1
#define PWM6_1      0x04	//P5.4
#define PWM6_2      0x08	//P0.1
#define PWM6_3      0x0C	//P7.5

#define PWM7_0      0x00	//P2.2
#define PWM7_1      0x10	//P3.3
#define PWM7_2      0x20	//P0.2
#define PWM7_3      0x30	//P7.6

#define PWM8_0      0x00	//P2.3
#define PWM8_1      0x40	//P3.4
#define PWM8_2      0x80	//P0.3
#define PWM8_3      0xC0	//P7.7

#define ENO5P       0x01
#define ENO6P       0x04
#define ENO7P       0x10
#define ENO8P       0x40
#define ENO1P       0x01
#define ENO1N       0x02
#define ENO2P       0x04
#define ENO2N       0x08
#define ENO3P       0x10
#define ENO3N       0x20
#define ENO4P       0x40
#define ENO4N       0x80

int32 RAllPulse;
int16 ECPULSE1;
int16 ECPULSE2;
volatile uint8_t Game_Over = 0; // 小车完成全部任务，停车
int16 ServoDuty = Servo_Center;
int16 MotorDuty1 = 500;      		// 电机驱动占空比数值

int32 NowTime = 0;
uint16 BatVolt = 0;           	// 电池电压采集

u16 PWM1_Duty;
u16 PWM2_Duty;
u16 PWM3_Duty;
u16 PWM4_Duty;
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void MotorInit(u16 freq)
【功  能】电机PWM初始化
【参数值】int Encoder 编码器脉采集的冲数
【参数值】int Target  期望脉冲数
【返回值】电机PWM 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
【调用举例】驱动电机  TIM4_PWM_Init(1000-1);  //1KHZ周期
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
#ifdef USE7843OR7971
void MotorInit(u16 freq)
{
  P_SW2 |= 0x80;
  PWM2_PSCRL = (MAIN_Fosc/PWM2_CLOCK)-1;   	//主频为22M,PWM分频到10M范围 
  PWM2_CCER1 = 0x00;												//写 CCMRx 前必须先清零 CCxE 关闭通道
  PWM2_CCER2 = 0x00;
  PWM2_CCMR1 = 0x60; 												//通道模式配置
  PWM2_CCMR2 = 0x60;
  PWM2_CCMR3 = 0x60;
  PWM2_CCMR4 = 0x60;
  PWM2_CCER1 = 0x33; 												//配置通道输出使能和极性
  PWM2_CCER2 = 0x33;
  
  PWM2_ARRH  = (u8)((PWM2_CLOCK/freq)>>8);  //设置周期时间，从0计数到设定数值
  PWM2_ARRL  = (u8)(PWM2_CLOCK/freq); //  
  
  PWM2_CCR1H = 0;
  PWM2_CCR1L = 20; 	//设置占空比时间
  PWM2_CCR2H = 0;
  PWM2_CCR2L = 20; 	//设置占空比时间	
  PWM2_CCR3H = 0;
  PWM2_CCR3L = 20; 	//设置占空比时间
  PWM2_CCR4H = 0;	
  PWM2_CCR4L = 20; 	//设置占空比时间
  
  PWM2_ENO = 0x00;
  PWM2_ENO |= ENO5P; //使能输出
  PWM2_ENO |= ENO6P; //使能输出 
  PWM2_ENO |= ENO7P; //使能输出
  PWM2_ENO |= ENO8P; //使能输出 
  PWM2_PS = 0x00;    //高级 PWM 通道输出脚选择位
  PWM2_PS |= PWM5_3; //选择 PWM5_3 通道
  PWM2_PS |= PWM6_3; //选择 PWM6_3 通道 //高级 PWM 通道 4P 输出脚选择位, 0x00:P1.6, 0x40:P2.6, 0x80:P6.6, 0xC0:P3.4
  PWM2_PS |= PWM7_3;
  PWM2_PS |= PWM8_3;
  PWM2_BKR = 0x80;   //使能主输出
  PWM2_CR1 |= 0x01;  //开始计时
  P_SW2 &= 0x7f;
}
#elif defined USEDRV8701
void MotorInit(u16 freq)
{
  P_SW2 |= 0x80;
  PWM2_PSCRL = (MAIN_Fosc/PWM2_CLOCK)-1;   //主频为22M,PWM分频到10M范围 
  PWM2_CCER1 = 0x00; //写 CCMRx 前必须先清零 CCxE 关闭通道
  PWM2_CCER2 = 0x00;
  PWM2_CCMR1 = 0x60; //通道模式配置
  PWM2_CCMR2 = 0x60;
  PWM2_CCMR3 = 0x60;
  PWM2_CCMR4 = 0x60;
  PWM2_CCER1 = 0x55; //配置通道输出使能和极性
  PWM2_CCER2 = 0x55;
  
  PWM2_ARRH  = (u8)((PWM2_CLOCK/freq)>>8);  //设置周期时间，从0计数到设定数值
  PWM2_ARRL  = (u8)(PWM2_CLOCK/freq); //  
  
  PWM2_CCR1H = 100;
  PWM2_CCR1L = 200; 	//设置占空比时间
  PWM2_CCR2H = 100;
  PWM2_CCR2L = 200; 	//设置占空比时间	
  
  PWM2_ENO = 0x00;
  PWM2_ENO |= ENO5P; 	//使能输出
  //PWM2_ENO |= ENO6P;//使能输出 
  PWM2_ENO |= ENO7P; 	//使能输出
  //PWM2_ENO |= ENO8P;//使能输出 
  PWM2_PS = 0x00;   	//高级 PWM 通道输出脚选择位
  PWM2_PS |= PWM5_3;	//选择 PWM5_3 通道
  //PWM2_PS |= PWM6_3;//选择 PWM6_3 通道 //高级 PWM 通道 4P 输出脚选择位, 0x00:P1.6, 0x40:P2.6, 0x80:P6.6, 0xC0:P3.4
  PWM2_PS |= PWM7_3;
  //PWM2_PS |= PWM8_3;
  PWM2_BKR = 0x80;		//使能主输出
  PWM2_CR1 |= 0x01;  	//开始计时
  
  P7M1 = 0;   
  P7M0 = 0xA0;				//P75,P77设置为推挽输出，强上拉，要加限流电阻
  P7PU |= 0xA0;
  
  P_SW2 &= 0x7f;
}
#endif
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void MotorCtrl(float motor1, float motor2)
【功  能】电机控制函数
【参数值】motor1   ： 电机1占空比
【参数值】motor2   ： 电机2占空比
【返回值】电机PWM 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
【调用举例】 MotorCtrl (1000,-2000) //两个电机一正一反转动
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
#ifdef  USE7843OR7971
void MotorCtrl (int16_t motor1,int16_t motor2)
{
  if (motor1 < 0)
  {	 
		motor1=0-motor1;
    P_SW2 |= 0x80;
    PWM2_CCR1H = 0; //设置占空比时间
    PWM2_CCR1L = 0;
    PWM2_CCR2H = (u8)(motor1 >> 8); //设置占空比时间
    PWM2_CCR2L = (u8)(motor1);
    P_SW2 &= 0x7f;
  }
  else
  {
    P_SW2 |= 0x80;
    PWM2_CCR1H = (u8)(motor1 >> 8); //设置占空比时间
    PWM2_CCR1L = (u8)(motor1);
    PWM2_CCR2H = 0; //设置占空比时间
    PWM2_CCR2L = 0;
    P_SW2 &= 0x7f;
  }  
  if (motor2 < 0)
  {	 
		motor2=0-motor2;
    P_SW2 |= 0x80;
    PWM2_CCR3H = 0; //设置占空比时间
    PWM2_CCR3L = 0;
    PWM2_CCR4H = (u8)(motor2 >> 8); //设置占空比时间
    PWM2_CCR4L = (u8)(motor2);
    P_SW2 &= 0x7f;
  }
  else
  {
    P_SW2 |= 0x80;
    PWM2_CCR3H = (u8)(motor2 >> 8); //设置占空比时间
    PWM2_CCR3L = (u8)(motor2);
    PWM2_CCR4H = 0; //设置占空比时间
    PWM2_CCR4L = 0;
    P_SW2 &= 0x7f;
  }  
}
#elif defined USEDRV8701
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void MotorCtrl(float motor1, float motor2)
【功  能】电机控制函数
【参数值】motor1   ： 电机1占空比
【参数值】motor2   ： 电机2占空比
【返回值】电机PWM 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
【调用举例】 MotorCtrl (1000,-2000) //两个电机一正一反转动
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void MotorCtrl (int16_t motor1,int16_t motor2)
{
  if (motor1 < 0)
  {	 
		motor1=0-motor1;
    P_SW2 |= 0x80;
    PWM2_CCR1H = (u8)(motor1 >> 8); //设置占空比时间
    PWM2_CCR1L = (u8)(motor1);
    P75=1;
    
    P_SW2 &= 0x7f;
  }
  else
  {
    P_SW2 |= 0x80;
    PWM2_CCR1H = (u8)(motor1 >> 8); //设置占空比时间
    PWM2_CCR1L = (u8)(motor1);
    P75=0;
    P_SW2 &= 0x7f;
  }  
  if (motor2 < 0)
  {	 
		motor2=0-motor2;
    //P_SW2 |= 0x80;
    PWM2_CCR3H = (u8)(motor2 >> 8); //设置占空比时间
    PWM2_CCR3L = (u8)(motor2);
    P77=1;
    P_SW2 &= 0x7f;
  }
  else
  {
    P_SW2 |= 0x80;
    PWM2_CCR3H = (u8)(motor2 >> 8); //设置占空比时间
    PWM2_CCR3L = (u8)(motor2);
    P77=0;
    P_SW2 &= 0x7f;
  }  
}
#endif
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】TestMotor(void)
【功  能】测试标定输出PWM控制电机
【参数值】无
【参数值】无
【返回值】无
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
【调用举例】
【注意事项】注意，一定要对电机输出进行限制
使用龙邱母板测试电机及驱动流程：
1.先使用万用表测量电池电压，务必保证电池电压在7V以上，否则无力不反应！
2.接好母板到驱动板的信号线及电源线；
3.接好驱动板到电机的导线；
4.烧写程序并运行，确定电机能正常转动后，开启驱动板电源开关；
5.按键K0/K1确定电机转动速度及方向；
6.如果出现疯转，按下K1键返回低速模式，或者直接关闭驱动板电源！
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void TestMotor (void)
{
  short duty = 20;
  char txt[16];
  OLED_Init(); 
  OLED_CLS();   						// 清屏
  OLED_P6x8Str(2, 0, "LQ Motor Test");
  MotorInit(MOTOR_FREQUENCY);    		// 电机初始化
  while (1)
  {
    if (KEY_Read(KEY0) == 0)      	// 按下KEY0键，占空比减小
    {
      if (duty > -PWM2_CLOCK/MOTOR_FREQUENCY)	// 满占空比为10M/12.5k=800
        duty -= 30;
    }
    if (KEY_Read(KEY2) == 0)      	// 按下KEY2键，占空比加大
    {
      if (duty < PWM2_CLOCK/MOTOR_FREQUENCY)  // 满占空比为10M/12.5k=800
        duty += 40;
    }
    if (KEY_Read(KEY1) == 0)      	// 按下KEY1键，占空比中值
    {
      duty = 20;
    }
    
    MotorCtrl(duty,0);
    
    sprintf(txt, "PWM: %05d;", duty);
    OLED_P6x8Str(0, 5, txt);  		// 字符串显示
    UART1_PutStr(txt);
    
    LED_Ctrl(LED0, RVS);       		// 电平翻转,LED闪烁
    delayms(200);              		// 延时等待
  }
}
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void ServoInit(u16 freq)
【功  能】舵机PWM初始化
【参数值】u16 freq,期望频率，范围0--10000
【参数值】无
【返回值】无
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
【调用举例】驱动1个舵机 ServoInit(100);  //100HZ周期
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void ServoInit(u16 freq)
{
  P_SW2 |= 0x80;	
  //PWM1_PSCRL = 16; //分频范围  fcn_cnt=fck_psc/(PSCR[15:0]+1)
  PWM1_PSCRL = MAIN_Fosc/PWM1_CLOCK-1;      //主频为30M,PWM分频到1M范围  
  PWM1_CCER2 = 0x00; //写 CCMRx 前必须先清零 CCxE 关闭通道
  PWM1_CCMR4 = 0x60; //设置 PWM4 模式1 输出
  PWM1_CCER2 = 0x55; //使能 CC4E 通道
  
  PWM1_ARRH  = (u8)((PWM1_CLOCK/freq)>>8);  //设置周期时间，从0计数到设定数值
  PWM1_ARRL  = (u8)(PWM1_CLOCK/freq);       //
  
  PWM1_CCR4H = (u8)((Servo_Center)>>8);     //舵机中值
  PWM1_CCR4L = (u8)(Servo_Center);          //
  
  PWM1_ENO |= 0x80; //使能 PWM4N 输出
  PWM1_PS  |= 0x00; //高级 PWM 通道 4N 输出脚选择位, 0x00:P1.7, 0x40:P2.7, 0x80:P6.7, 0xC0:P3.3
  
  PWM1_BKR = 0x80;  //使能主输出
  PWM1_CR1 |= 0x01; //开始计时
  P_SW2 &= 0x7f;		
}
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void ServoCtrl (uint16_t pwm_value)
【功  能】舵机转角函数，由于小车拉杆范围限制，较小
【参数值】short duty，占空比，范围0--10000；
【参数值】int Target  期望脉冲数
【返回值】电机PWM 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
【调用举例】ServoCtrl (1500);//舵机中值
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void ServoCtrl(uint16_t pwm_value)
{
  /*
  if (duty >= Servo_Left_Max)                  //限制幅值
  duty = Servo_Left_Max;
    else if (duty <= Servo_Right_Min)          //限制幅值
  duty = Servo_Right_Min;
  */
  P_SW2 |= 0x80;
  PWM1_CCR4H = (u8)(pwm_value >> 8); //设置占空比时间
  PWM1_CCR4L = (u8)(pwm_value);
  P_SW2 &= 0x7f;
}
/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】Test_Servo(void)
【功  能】舵机PWM初始化，测试标定输出PWM控制SD5/S3010舵机
【参数值】无
【参数值】无
【返回值】无
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
【调用举例】
【注意事项】注意，一定要对舵机打角进行限制
使用龙邱母板测试舵机流程：
1.先使用万用表测量电池电压，务必保证电池电压在7V以上，否则无力不反应！
2.然后确定舵机供电电压，SD5舵机用5V供电，S3010用6-7V供电，SD012舵机用5V供电！！！
3.把舵机的舵盘去掉，让舵机可以自由转动；
4.烧写程序并运行，让舵机转动到中值附近；如果没反应重复1-2步，或者调整舵机的PWM频率计占空比，能受控为准；
5.舵机受控后用手轻转，舵机会吱吱响，对抗转动，此时可以装上舵盘；
6.按键K0/K1确定舵机的左右转动极限，并记下来，作为后续限幅防止舵机堵转烧毁！
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void TestServo (void)
{
  char txt[16] = "X:";
  u16 duty = Servo_Center;
  
  OLED_CLS();   				// 清屏
  OLED_P6x8Str(2, 0, "LQ Servo Test");
  ServoInit(50);				// 舵机初始化，频率为50Hz
  ServoCtrl(Servo_Center); 	// 舵机中值
  while (1)
  {
    if (!KEY_Read(KEY0))
    {
      if (duty > 100)  	// 防止duty超
      {
        duty -= 10;     // 标定的时候，可以把步长改小点，比如10
      }
    }
    if (!KEY_Read(KEY1))
    {
      duty = Servo_Center;
    }
    if (!KEY_Read(KEY2))
    {
      duty += 10;
    }
    ServoCtrl(duty); 
    sprintf(txt, "Servo:%05d ", duty);
    OLED_P6x8Str(1, 2, txt); 	// 显示
    LED_Ctrl(LED0, RVS);   		// LED闪烁;
    delayms(100);        
  }
}


//
