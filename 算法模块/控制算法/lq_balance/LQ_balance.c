/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【平    台】北京龙邱智能科技STC16核心板
【编    写】chiusir
【E-mail  】chiusir@163.com
【软件版本】V1.1 版权所有，单位使用请先联系授权
【最后更新】2021年1月29日
【相关信息参考下列地址】
【网    站】http://www.lqist.cn
【淘宝店铺】http://longqiu.taobao.com
------------------------------------------------
【IDE】STC16
【Target 】 C251Keil5.57及以上
【SYS PLL】 30MHz使用内部晶振
=================================================================

！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！    本程序由实验室2019级学长编辑优化补充     ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

=================================================================
下载时, 选择时钟 30MHZ (用户可自行修改频率).
STC16F初次下载:先用IRCBND=0，ISP界面设置为24M，
芯片最后一行2038...然后IRCBND=0，下载频率为30M；
芯片最后一行2010...或者IRCBND=1，下载频率为30M；
官方出库批次不同，略有差异，好用为准！！！
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ

龙邱电单车主要控制函数，可以选择MPU6050和ICM20689等6轴模块
由于编译器问题，很多“意外”需要大家注意规避

1、MPU6050使用DMP初始化后可以设置FIFO数据就绪输出中断，通知到单片机采集；
2、ICM20689DMP尚未调试通过，如果使用，需要配合定时中断实现；
3、使用其它陀螺仪，可以任选外部中断和定时中断。

本例程中对单车的控制主要使用的算法：
1、平衡环使用PID；
2、速度环使用的增量式PID；
3、转向换使用的改变中值的方式，即可以简化算，实测稳定效果良好；
4、用户可在此基础上优化算法，实现平衡环、速度环的参数叠加，以达控制目的。

QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
#include "LQ_balance.h"
float d=0.00;//控制平衡的角度中值
float m=0.00;//控制平衡的角度中值
float a=0.00;//控制P参数改变
float c=0.00;//控制I参数改变
float n=0.00;//控制D参数改变
float K1P=0.00;
float K1D=0.00;
float K2P=0.93;              //(实时归一化1.2M   K2P 0.74   K2D  未加)   (实时归一化1.5M   K2P 1.04   K2D  未加)    0.6m   0.53   K2D  未加
float K2D=0.01;
float xc=1.05;                 // 1.2时是1.6    1.2用的差寻的线  1.5时是1.1
float data error=0.00;
float data errorf=0.00;
int16_t ad_max[4] = {50,50,50,50};  // 最大值
int16_t ad_min[4] = {3000,3000,3000,3000};        //最小值
int flag=1,zhidaoshibie=0;
	int flag_zhixian;
// 定义在DMP中，在此引用
extern float data Pitch, Roll; 				  // extern封装库中的俯仰角与翻滚角
extern short data gyro[3], accel[3];    // extern封装库中的陀螺仪XYZ，加速度XYZ变量

short data Velocity =72;					      // 速度，定时周期内为60个脉冲，龙邱带方向512编码器
float data Angle_Balance=0;					      // 陀螺仪DMP获取倾角				
unsigned short data Encoder=0;				    // 编码器的脉冲计数	
short data PWMMotor=0, PWMServo=0;				  // 电机舵机PMW变量中值
short data Motor_Bias=0, Motor_Last_Bias=0, Motor_Integration=0; // 电机所用参数
unsigned char Flag_Stop = 1, Flag_Show=0, Flag_Imu=0;          // 停车，显示，IMU完成标志位


float data Zhongzhi = -1.70;					    // 小车平衡角度中值，改大行走向左偏，改小行走向右偏；
float data Zhongzhi1= 0.00;	
float data Zhongzhi2= 0.00;	
#define Balance_Kp   24.5                // 舵机PID参数    1M35    1.2M 26.2    1.3M25.2   1.4M17.2   1.5M17.12
#define Balance_Kd   0.060              // 舵机PID参数 
#define Balance_Ki   0.43            // 舵机PID参数 
//囧囧囧舵机平衡所用的PID系数，需要把宏定义改为变量，改为变量就容易出错囧，偶尔也会正常囧
//float Balance_Kp = 39.5, Balance_Kd = 0.1, Balance_Ki = 0.0026;	// PID参数  囧
#define Motor_Kp     12.0	                // 电机PID参数
#define Motor_Ki     20.0	                // 电机PID参数
#define Turn_Kp      0.10	              // 转向PID参数
#define ceshi     P77
float data Integration=0.00;
// 循迹相关
unsigned char Flag_getTurn = 0; 
extern int left, right,left1, right1;
extern uint16_t data LnowADC[4];
//extern short *ax,*ay,*az,*gx,*gy,*gz;
int h,q=0,z=0,v=0;
int ADC1[20],ADC2[20],ADC3[20],ADC4[20];
//#define L0MAX 2100                       // 右竖直电感最大值：
//#define L1MAX 2840                       // 右水平电感最大值：1780
//#define L2MAX 2000                      // 左水平电感最大值：1630
//#define L3MAX 2100                       // 左竖直电感最大值：
//#define L0MIN 0                        // 右竖直电感-->最小值：107
//#define L1MIN 60                       // 右水平电感-->最小值：104              
//#define L2MIN 130                        // 左水平电感-->最小值：108
//#define L3MIN 0                        // 左竖直电感-->最小值：100

/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void INT2_int (void) interrupt 10  		// 外部中断或者定时中断，5ms
【功  能】最关键的数据处理均在此函数中，通过6050陀螺仪int管脚输出信号，每5ms执行一次
          使用外部中断或者定时器均可。
【参数值】无
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
//void timer0_int(void) interrupt 1  	  // 定时器中断，5ms，与下面外部中断二选一
void INT2_int (void) interrupt 10  		  // 陀螺仪FIFO定时中断，5ms，
{
	int data Servo_PWM=0;						        // 舵机PID
	double last_Zhongzhi1=0,last_Zhongzhi2=0;
	last_Zhongzhi1=Zhongzhi1;       //pd用
	last_Zhongzhi2=Zhongzhi2;       //pd用
//////转向控制，不循迹可以注释掉下面四行///////////////////////////
	Zhongzhi1 = (short)((left - right) <<4)/(left + right);// 根据电感采集的归一化偏差数值计算中值的位置	(差比和)
	Zhongzhi2 = (float)(right1-left1)/10.00;   //差值寻迹
	if((right1-left1)>0)
	Zhongzhi2=Zhongzhi2*xc;
	Zhongzhi=-1.7-(K2P*Zhongzhi2+K2D*(last_Zhongzhi2-Zhongzhi2));   //计算期望的倾斜角度 pd用
//	if(Zhongzhi > 2.0)	Zhongzhi = 2.0;		// 左拐，限幅
//	if(Zhongzhi < -7.0)	Zhongzhi = -7.0;	// 右拐，限幅
	
	error=right1-left1;
	errorf=myabs(error);
	
//丢线停车
//	if(left1>90&&right1>90)
//	Flag_Stop=1;	
//判断直线
	
	
	Encoder = myabs(Read_Encoder(1));		  // 编码器采值	
	
//	UART4_SendInt16(0xF1,bilixishu21);
	//LQ_DMP_Read();// 囧囧囧放这里就卡死，只能放循环里面了，为什么？？？IMU读取大约需要1.7ms囧
 
	Angle_Balance = Roll;    				      // 角度更新
	
	Key_Process();									      // 按键控制，控制发车等
	
	//Velocity = 60 - myabs(left - right) * 0.1;                // 变速控制
  //Velocity = 45 + myabs((int)Angle_Balance) * 1.5;          // 变速控制
	
		  
		
	Servo_PWM = SBB_Get_BalancePID(Angle_Balance, gyro[1]);	    // PID计算单车平衡的PWM数值	
	
	
	if(	Servo_PWM < - Servo_Delta)	  Servo_PWM = - Servo_Delta;// 舵机角度限制
	else if(Servo_PWM > Servo_Delta)  Servo_PWM =   Servo_Delta;// 舵机角度限制
	PWMServo = Servo_Center + Servo_PWM;	                      // 转换为舵机控制PWM 		
	PWMMotor = SBB_Get_MotorPI(Encoder, Velocity) / 15;					// 电机增量式PI控制
	
	if(Angle_Balance > 18 || Angle_Balance < -19)	Flag_Stop = 1;// 摔倒停车判断
	
	if(Flag_Stop == 1)						       // 停车		
	{
		PWMMotor = 0;                      // 电机关闭
		PWMServo = Servo_Center;           // 舵机回中
		Integration = 0;  		// 积分参数归零
	}

	MotorCtrl(PWMMotor - 60,0);	       	 // 电机PWM，加上电机死区
	ServoCtrl(PWMServo);				         // 舵机控制PWM，舵机范围：【约1500±300】	
						
	Flag_Imu = 1;						             // IMU采值标志，在中断结束后采值
	Flag_getTurn++;                      // 转向标志
//	Flag_Show++;						             // OLED信息显示标志
	Led_Flash(10);						           // LED闪烁，用来提示程序是否卡死	
}

/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void Demo_SBB_Car(void)
【功  能】单车程序初始化及综合演示程序 
【参数值】无
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void Demo_SBB_Car(void)
{
	
	MotorInit(MOTOR_FREQUENCY);          // 初始化PWM 12.5KHZ，
	MotorCtrl(-10, 0);	                 // 电机控制 
	ServoInit(100);   		               // 初始化PWM100HZ驱动 舵机	
	ServoCtrl(Servo_Center);     	       // 舵机中值	
	Timer34EncInit();                    // 编码器接口	
	ADC_Init();                          // ADC初始化  	
	SOFT_IIC_Init();					           // 软件IIC初始化
	LQ_DMP_Init();						           // MPU6050DMP初始化
//	ICM20689_Init();
	delayms(500);						             // DMP初始化需要时间	
  //Timer0_init();                     // 200hz---> 5ms,单车用  与下面外部中断二选一
	GPIO_EXIT_Init();                    // GPIO外部中断2初始化，P3.6，陀螺仪FIFO 200hz--->5ms,单车用
	EnableInterrupts;					           // 允许中断
//	OLED_CLS();   						           // 清屏	
//	OLED_Show();  						           // 显示信息
	while(1)
	{	
		if(Flag_Imu)					             // FIFO数据准备好，每次中断后再读IMU，间隔5ms
		{		
			Flag_Imu = 0;			            	// 清除标志
			LQ_DMP_Read();                   // 读取DMP四元数 在这5ms之内，IMU读取，大约需要1.7ms

		}			
			if(Flag_getTurn >= 1)            // 需要转向
			{
				Flag_getTurn = 0;              // 清除标志
			  
				 LnowADC[0]=ADC_Read_Average(8,10);    	//读取ADC8通道值
         LnowADC[1]=0.00;			//读取ADC9通道值
         LnowADC[2]=0.00; 		//读取ADC10通道值
         LnowADC[3]=ADC_Read_Average(11,10);		//读取ADC11通道值	
				
				
				
				if(flag)
		{
		if (LnowADC[0] < ad_min[0])            ad_min[0] = LnowADC[0];     // ?????
    else if (LnowADC[0] > ad_max[0])       ad_max[0] = LnowADC[0];     // ?????
    if (LnowADC[1] < ad_min[1])            ad_min[1] = LnowADC[1];
    else if (LnowADC[1] > ad_max[1])       ad_max[1] = LnowADC[1]; 
    if (LnowADC[2] < ad_min[2])            ad_min[2] = LnowADC[2];
    else if (LnowADC[2] > ad_max[2])       ad_max[2] = LnowADC[2];
    if (LnowADC[3] < ad_min[3])            ad_min[3] = LnowADC[3];
    else if (LnowADC[3] > ad_max[3])       ad_max[3] = LnowADC[3];	
		}
		
		
				
          left1  = 100.0 -(float)(LnowADC[3] - ad_min[3]) / (ad_max[3] - ad_min[3]) * 100.0;		//电感归一化
				  right1 = 100.0 -(float)(LnowADC[0] - ad_min[0]) / (ad_max[0] - ad_min[0]) * 100.0;		//电感归一化
			  	left  = 100.0 - (float)(LnowADC[1] - ad_min[1]) / (ad_max[1] - ad_min[1]) * 100.0;		//电感归一化
				  right = 100.0 - (float)(LnowADC[2] - ad_min[2]) / (ad_max[2] - ad_min[2]) * 100.0;		//电感归一化
			  	left1 = (left1 < 0) ? 0 : left1;       //限幅
			  	left1 = (left1 > 100) ? 100 : left1;   //限幅
			  	right1 = (right1 < 0) ? 0 : right1;    //限幅
			  	right1 = (right1 > 100) ? 100 : right1;//限幅	
				  left = (left < 0) ? 0 : left;       //限幅
				  left = (left > 100) ? 100 : left;   //限幅
			  	right = (right < 0) ? 0 : right;    //限幅
			  	right = (right > 100) ? 100 : right;//限幅	
			}
			
//		if(Flag_Show >19)//节省资源，理论20次/100ms显示一次
//			{		
//					Flag_Show = 0;
//					OLED_Show();  // 每次需要10ms，不按键不开启	
//			}		
//			
			
		}		
	}


/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】int SBB_Get_BalancePID(float Angle,float Gyro)
【功  能】舵机打角控制PID
【参数值】float Angle,电单车左右倾角
【参数值】float Gyro 电单车左右角速度
【返回值】小车平衡，舵机转向PID 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
int SBB_Get_BalancePID(float Angle,float Gyro)
{  
	float  Bias=0.00;
	int SBB_BalancePID; 
  m=Zhongzhi+d;
	Bias= Angle - m;     // 求出平衡的角度中值和此时横滚角的偏差
	
	Integration += Bias;         // 积分

	if(Integration<-300)      Integration=-300;	//限幅
	else if(Integration>300)  Integration= 300;	//限幅

	//===计算平衡控制的舵机PWM  PID控制 kp是P系数 ki式I系数 kd是D系数 
	SBB_BalancePID = (Balance_Kp+a )* Bias + (Balance_Ki+c)*Integration + (Balance_Kd+n)*Gyro;   
	return SBB_BalancePID;
}

/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】int SBB_Get_MotorPI (int Encoder,int Target)
【功  能】电机控制增量式PI
【参数值】int Encoder 编码器脉采集的冲数
【参数值】int Target  期望脉冲数
【返回值】电机PWM 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
int SBB_Get_MotorPI (int Encoder,int Target)
{ 	
	static int Pwm;

	Motor_Bias = Encoder - Target;					 // 计算偏差
	Pwm += Motor_Kp * (Motor_Bias - Motor_Last_Bias) + Motor_Ki * Motor_Bias; // 增量式PI控制器	
	if(Pwm > 7000) Pwm = 7000;               // 限幅
	else if(Pwm < -7000)Pwm = -7000;         // 限幅
	Motor_Last_Bias = Motor_Bias;	           // 保存上一次偏差 
	return Pwm;                         		 // 增量输出
}

/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void OLED_Show(void)
【功  能】OLED信息显示
【参数值】无
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/ 
void OLED_Show(void)
{
	char txt[16];
	short Vbat=0;
	//=============第一行显示标题=====================
	sprintf(txt,"left1:%03d right1:%03d", left1, right1);	
	OLED_P6x8Str(0,0,(u8*)txt);
	//=============第二行显示舵机PID系数============== 
	sprintf(txt,"P:%.2fI:%.3fD:%.4f",Balance_Kp+a, Balance_Ki+c,Balance_Kd+n);
	OLED_P6x8Str(0,1,(u8*)txt);			
	//=============第三行显示编码器和目标速度信息=====
	sprintf(txt,"K2P:%.2f  Vt:%04d ", K2P, Velocity);
	OLED_P6x8Str(0,2,(u8*)txt);	
	//=============第四行显示舵机和电机PWM============
	sprintf(txt,"errorf:%.1f  Mot:%04d ", errorf, PWMMotor);
	OLED_P6x8Str(0,3,(u8*)txt);
	//=============第五行显示电压和中值===============
	Vbat = ADC_Read(6) * 4 / 5;  //10K和1K电阻分压 Voltage*3.3/4095*100*11	
	sprintf(txt,"PW:%02d.%02dV  %0.2f ", Vbat/100, Vbat%100, m);
	OLED_P6x8Str(0,4,(u8*)txt);
	//=============第六行显示角度=====================
	sprintf(txt,"Ang:%.1f Pitch:%.1f  ", Angle_Balance, Pitch);
	OLED_P6x8Str(0,5,(u8*)txt);
	//=============第6行显示读取电感值================
	sprintf(txt,"AD1:%04d   AD2:%04d ", LnowADC[1], LnowADC[2]);
	OLED_P6x8Str(0,6,(u8*)txt);
	//=============第7行显示转向值====================
	sprintf(txt,"AD0:%04d   AD3:%04d ", LnowADC[0], LnowADC[3]);
	OLED_P6x8Str(0,7,(u8*)txt);
}

/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】short myabs(short x)
【功  能】求绝对值函数
【参数值】正负数据
【返回值】正数 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
short myabs(short x)
{
	if(x > 0)
		return x;
	else
		return -x;
}

/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void Key_Process(void)
【功  能】标定，发车控制，单按启动，再按停止
【参数值】无
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void Key_Process(void)
{	
 
		if (KEY_Read(KEY0) == 0)      //按下KEY0键，占空比减小
        {
					K2P+=0.1;
						delayms(100);
        }   
  if (KEY_Read(KEY1) == 0)      //按下KEY2键，占空比加大
        {
				
           K2P-=0.05;
					delayms(100);
        } 
	if (KEY_Read(KEY2) == 0)
	{
		      flag=0;
					delayms(100);
	}
	if(KEY_Read(KEY3) == 0)           // 单击K3(走起，启停键)读取		
	{ 
		Flag_Stop =! Flag_Stop;   			// 单击走起，再击停车		
	}  
	else if(KEY_Read_Comb() == KEY02DOWN) // 组合键读取
	{
		Zhongzhi = Angle_Balance;				// 同时按下K0 K2键标定初始值
	}
	//囧囧囧用于实时调整舵机系数，需要把宏定义改为变量，改为变量就容易出错囧
	//if(KEY_Read(KEY0) == 0)		   Balance_Kp = Balance_Kp - 0.001;
	//else if(KEY_Read(KEY2) == 0) Balance_Kp = Balance_Kp + 0.001;
}

/*LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL
【函数名】void UART4_SendInt16(unsigned char Function, int outData)
【功  能】匿名上位机用户协议，使用时占用MCU资源较大，跑车时不使用
【参数值】unsigned char Function, 功能字符
【参数值】int outData 数据
【返回值】无 
【作  者】chiusir
【最后更新】2021年1月22日 
【软件版本】V1.0
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
void UART4_SendInt16(unsigned char Function, int outData)
{
	unsigned char sum;
	unsigned char outData_H = ((unsigned short)outData)>>8;
	unsigned char outData_L = (unsigned char)outData;
	S4BUF = (0xAA);   // 头
	delayus(90);
	S4BUF = (0xAA);   // 头
	delayus(90);
	S4BUF = Function; // 功能
	delayus(90);
	S4BUF = (0x02);   // 数据为两个字节
	delayus(90);
	S4BUF = outData_H;// 数据高8位
	delayus(90);
	S4BUF = outData_L;// 数据低8位
	delayus(90);
	sum = 0xAA | 0xAA | 0xF1 | 0x02 | outData_H | outData_L;// 校验值
	S4BUF = sum;
	delayus(90);
}

