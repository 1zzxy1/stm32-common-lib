/*
 * KeyBoard.c
 *
 *  Created on: Apr 28, 2016
 */
#include "KeyBoard.h"

void Key_IO_Init()
{
   GPIO_InitTypeDef GPIO_InitStructure;
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD ,ENABLE);//使能GPIOA、GPIOE时钟

	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_1|GPIO_Pin_3;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	  GPIO_Init(GPIOD, &GPIO_InitStructure);
	

}

/////////////////////////////////////////////  按键输入检测   ////////////////////////////////////////////////////////////////
//功能：5向开关检测按键
//返回值 ： 'U' 'D' 'L' 'R' 'C' 分别代表5向按键的 上、下、左、右、中 5个方向上的某个被按下了 ，没有任何按键按下返回 ' '
//
//     注意：
//         1、需要在头文件中配置按键的引脚，以及按下/松开按键时的电平状态
//         2、按键输入分为不按，短按单击，长按连击三种模式。可以在头文件中把短按配置成按下时返回按键值或松开的时候返回按键值
//            短按的时候在 按下/松开 的时候返回一次按键值（需要头文件配置），短按超过一段时间之后，自动进入连击模式（进入的时间
//            以及连击的速度可以在头文件中配置），连击模式下持续返回按键值
//         3、5个按键是有优先级的，分别为 ： 上 > 下 > 左 > 右 > 中
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t key;
uint8_t KEY_Scan()
{	 
	u8 key_up = 1;
	if(key_up&&(Key_Left == 0||Key_Left == 0||Key_Down == 0||Key_Center == 0||Key_Up == 0||Key_Rignt == 0))
	{
		delay_ms(50);//??? 
		key_up=0;
		
		if(Key_Left == 0)
			return 'L';
			
		if(Key_Down == 0)
			return 'D';
			
		if(Key_Center == 0)
			return 'C';
			
		if(Key_Up == 0)
			return 'U';
			
		if(Key_Rignt == 0)
			return 'R';
		
	}else if((Key_Left==1||Key_Left==1||Key_Down==1||Key_Center==1||Key_Up==0||Key_Rignt==0)) key_up=1; 	    
 	return 0;// ?????
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//配置系统在线调参界面核心函数
uint8_t KeyInput = 0;			//按键的输入
int cursor_x = 0;               //光标的X坐标
int cursor_y = 0; 					//光标的Y坐标
uint8_t state = 0;					//调参状态 0 1 2 对应处于一级菜单 二级菜单 和二级菜单调整参数 三个状态??
int ParameterNo = 0;			//一级菜单参数号
int Parameter2No = 0;			//二级菜单的参数号

//?通过修改这个使能菜单数组，可以设置某个参数号下是否能有二级菜单。1表示有二级菜单，0表示没有
uint8_t en_sec_menu[16] = {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //使能二级菜单
//     ParameterNo: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

//调参界面中：
//上下可以切换选中的参数
//按下左键初始化LCD（在LCD花屏之后初始化可以使小液晶回复正常显示）
//按下中心键，如果在一级菜单中且对应参数号使能了二级菜单的话，会进入到二级菜单中。在二级菜单中，如果光标为 '>' 的时候 按下中心键 会变为 '-'
//这个时候可以通过上、下键修改对应的参数，再按一次中心键 光标重新变回 '>' 参数设置完成，光标又可以通过上、下键选择参数
//如果按下右键，可以自己定义，我的程序里是启动智能车
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ConfigParameterCore()
{

	KeyInput = KEY_Scan();
	//delay_ms(50);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if (KeyInput == 'R')
	{
		LCD_Clear(BLACK);
		delay_ms(100);
	}
	if (KeyInput == 'L')
	{
		LCD_Init();
		// **初始化菜单
		state = 0;
		Parameter2No = 0;
		LCD_Clear(BLACK);
		// **
		delay_ms(100);
	}

	if (state == 0)
	{ 
		// 如果状态在一级菜单
		if (KeyInput == 'U')
		{ 
			// flag向上
			LCD_printf(cursor_x, cursor_y, " ");
			ParameterNo--;
		}
		else if (KeyInput == 'D')
		{
			// flag向下
			LCD_printf(cursor_x, cursor_y, " ");
			ParameterNo++;
		}
	}
	else if (state == 1)
	{ 
		// 如果状态在二级菜单
		if (KeyInput == 'U')
		{ 
			// flag向上
			LCD_printf(cursor_x, cursor_y, " ");
			Parameter2No--;
		}
		else if (KeyInput == 'D')
		{ 
			// flag向上
			LCD_printf(cursor_x, cursor_y, " ");
			Parameter2No++;
		}
	}

	//在这修改  ParameterNo 的范围可以配置哪些一级参数号拥有二级菜单 如果改为 ParameterNo < 15 则所有的一级菜单都拥有二级界面
	//有一些标志位需要经常使用，但是每次进入二级菜单又太麻烦的时候，可以留几个一级界面的位置，直接在一级界面修改标志位，而不进入二级菜单。
	if (state == 0)
	{
		if (ParameterNo > 7)
			ParameterNo = 0;
		else if (ParameterNo < 0)
			ParameterNo = 7;
		if (en_sec_menu[ParameterNo] == 1)
		{
			//更改状态进入二级菜单
			if (KeyInput == 'C')
			{
				state = 1;
				LCD_Clear(BACK_COLOR);
			}
		}
	}
	else if (state == 1)
	{ 
		//二级菜单中
		if (Parameter2No > 6)
			Parameter2No = 0;
		else if (Parameter2No < 0)
			Parameter2No = 6;
		//显示返回键
		LCD_printf(bit2, X(7), "Back");

		if (Parameter2No != 6)
		{ //没有光标选中返回时，按下中心键进入修改参数
			if (KeyInput == 'C')
				state = 2; //更改状态 开始修改参数
		}
		else if (Parameter2No == 6)
		{ //选中了返回
			if (KeyInput == 'C')
			{
				state = 0;
				Parameter2No = 0;
				LCD_Clear(BACK_COLOR);
			} //返回一级菜单 清屏
		}
	}
	else if (state == 2)
	{
		if (KeyInput == 'C')
			state = 1;
	} //修改参数完成后再按一下中心键确定修改
	  //////////////////////////////// 调参界面的操作逻辑设置结束   //////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////// 以下是通过参数号确定光标   ///////////////////////////////////////////////////////////////////////////////////////////////////
	if (state == 0)
	{
		if (ParameterNo == 0)
		{
			cursor_x = X(1);
			cursor_y = X(1);
		}
		else if (ParameterNo == 1)
		{
			cursor_x = X(1);
			cursor_y = X(2);
		}
		else if (ParameterNo == 2)
		{
			cursor_x = X(1);
			cursor_y = X(3);
		}
		else if (ParameterNo == 3)
		{
			cursor_x = X(1);
			cursor_y = X(4);
		}
		else if (ParameterNo == 4)
		{
			cursor_x = X(1);
			cursor_y = X(5);
		}
		else if (ParameterNo == 5)
		{
			cursor_x = X(1);
			cursor_y = X(6);
		}
		else if (ParameterNo == 6)
		{
			cursor_x = X(1);
			cursor_y = X(7);
		}
	}
	else if (state == 1)
	{

		if (Parameter2No == 0)
		{
			cursor_x = 0;
			cursor_y = X(1);
		}
		else if (Parameter2No == 1)
		{
			cursor_x = 0;
			cursor_y = X(2);
		}
		else if (Parameter2No == 2)
		{
			cursor_x = 0;
			cursor_y = X(3);
		}
		else if (Parameter2No == 3)
		{
			cursor_x = 0;
			cursor_y = X(4);
		}
		else if (Parameter2No == 4)
		{
			cursor_x = 0;
			cursor_y = X(5);
		}
		else if (Parameter2No == 5)
		{
			cursor_x = 0;
			cursor_y = X(6);
		}
		else if (Parameter2No == 6)
		{
			cursor_x = 0;
			cursor_y = X(7);
		}
	}

	if (state == 0)
		LCD_printf(cursor_x, cursor_y, ">");
	else if (state == 1)
		LCD_printf(cursor_x, cursor_y, ">");
	else if (state == 2)
		LCD_printf(cursor_x, cursor_y, "*");
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//通过按键修改某个参数的值
//传入参数：parameter_type ptype ：需要修改的这个参数的类型，枚举定义在头文件中
//                                pchar，    该变量为char型
//                                puchar，  该变量为uchar型
//                                pint，      该变量为int型
//                                puint，    该变量为uint型
//                                pfloat，  该变量为float型
//                                pdouble，该变量为double型
//
//         void* ppara：要修改的变量
//
//         double precision ：每次加/减的精度
//
//         uint8_t keyx ：按键的输入值
//
//传出参数：无
//用法实例：ChangeParameter(pdouble,&p,0.05,KeyInput); //修改double类型变量p的值，每次修改0.05（按键输入为'U'/'D'的时候修改）
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ChangeParameterVal(parameter_type ptype, void *ppara, double precision, uint8_t keyx)
{

	if (keyx == 'U')
	{

		switch (ptype)
		{
		case pint8_t:
			*((int8_t *)ppara) += precision;
			break;
		case puint8_t:
			*((uint8_t *)ppara) += precision;
			break;
		case pint16_t:
			*((int16_t *)ppara) += precision;
			break;
		case puint16_t:
			*((uint16_t *)ppara) += precision;
			break;
		case pint32_t:
			*((int32_t *)ppara) += precision;
			break;
		case puint32_t:
			*((uint32_t *)ppara) += precision;
			break;
		case pfloat:
			*((float *)ppara) += precision;
			break;
		case pdouble:
			*((double *)ppara) += precision;
			break;
		}
	}
	else if (keyx == 'D')
	{

		switch (ptype)
		{
		case pint8_t:
			*((int8_t *)ppara) -= precision;
			break;
		case puint8_t:
			*((uint8_t *)ppara) -= precision;
			break;
		case pint16_t:
			*((int16_t *)ppara) -= precision;
			break;
		case puint16_t:
			*((uint16_t *)ppara) -= precision;
			break;
		case pint32_t:
			*((int32_t *)ppara) -= precision;
			break;
		case puint32_t:
			*((uint32_t *)ppara) -= precision;
			break;
		case pfloat:
			*((float *)ppara) -= precision;
			break;
		case pdouble:
			*((double *)ppara) -= precision;
			break;
		}
	}
}

void ChangeFlagVal(uint8_t *flag, uint8_t keyx, uint8_t min, uint8_t max)
{

	if (keyx == 'C')
		(*flag)++;

	if (*flag > max)
		*flag = min;
	else if (*flag < min)
		*flag = max;
}

//清除变量无效的显示
void clean_invalid_char(uint8_t endx)
{

	//	uint8_t i = 0;
	//	for(i = getx();i<=endx;i+=6){

	//		LCD_P6x8char(i,gety(),' ');
	//	}
}

void LCD_ConfigParameter()
{
	ConfigParameterCore(); //调参界面的核心，负责控制光标的位置，各级菜单之间切换
	/////////    显示一级菜单内容   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (state == 0)
	{
		LCD_printf(X(2), 0 * 30, "PID");
		LCD_printf(X(2), 1 * 30, "Camera");
		LCD_printf(X(2), 2 * 30, "Menu3");
		LCD_printf(X(2), 3 * 30, "Menu4");
		LCD_printf(X(2), 4 * 30, "Menu5");
		LCD_printf(X(2), 5 * 30, "Menu6");
		LCD_printf(X(2), 6 * 30, "Menu7");
		
	}

	/////////   显示二级菜单内容    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	else if (state == 1 || state == 2)
	{
		switch (ParameterNo)
		{
		//舵机中值
		case 0:
			//标题
			LCD_printf(X(2), 0 * 30, "Mode");
			//LCD_printf(X(2), 1 * 30, "mode:%s", float2str(mode, 1));

			//			//修改参数
			if (state == 2)
			{
				if (Parameter2No == 1)
				{
					//ChangeParameterVal(puint32_t, &mode, 1, KeyInput);
				}
				else if (Parameter2No == 2)
				{
				}
				else if (Parameter2No == 3)
				{
				}
				else if (Parameter2No == 4)
				{
				}
				break;

			case 1: //舵机PD
					//标题
				LCD_printf(X(2), 0 * 30, "STE");
				//修改参数
//				LCD_printf(X(2), 1 * 30, "pitch:%s", float2str(Angle_pitch, 2));
//				LCD_printf(X(2), 2 * 30, "yaw_L:%s", float2str(Angle_L, 2));
//				LCD_printf(X(2), 3 * 30, "yaw_R:%s", float2str(Angle_R, 2));
				if (state == 2)
				{

					if (Parameter2No == 1)
					{
						//ChangeParameterVal(pfloat, &Angle_pitch, 1, KeyInput);
					}
					else if (Parameter2No == 2)
					{
						//ChangeParameterVal(pfloat, &Angle_L, 1, KeyInput);
					}
					else if (Parameter2No == 3)
					{
						//ChangeParameterVal(pfloat, &Angle_R, 1, KeyInput);
					}
					else if (Parameter2No == 4)
					{
					}
					//STE_Angle_Calation(Angle_pitch, Angle_L, Angle_R);
				}
				break;
			//电机
			case 2:
				//标题
				LCD_printf(X(2), 0 * 30, "Motor");
				//LCD_printf(X(2), 1 * 30, "speed:%s", float2str(Speed, 2));
				//LCD_printf(X(2), 2 * 30, "Dir:%s", float2str(Dir, 2));
				//修改参数

				if (state == 2)
				{
					if (Parameter2No == 1)
					{
						//ChangeParameterVal(puint32_t, &Speed, 500, KeyInput);
					}
					else if (Parameter2No == 2)
					{
						
					}
					else if (Parameter2No == 3)
					{
					}
					else if (Parameter2No == 4)
					{
					}
					else if (Parameter2No == 5)
					{
					}
				}

				break;
			case 3:
				//标题
				LCD_printf(X(2), 0 * 30, "Stepper");
				//显示参数
//				LCD_printf(X(2), 1 * 30, "angle=%s", float2str(angle_stepper, 1));
//				LCD_printf(X(2), 2 * 30, "Freq=%s", float2str(Frequency_stepper, 1));
//				LCD_printf(X(2), 3 * 30, "Dir=%s", float2str(Dir, 1));
				//修改参数
				if (state == 2)
				{
					if (Parameter2No == 1)
					{
						//ChangeParameterVal(pint32_t, &angle_stepper, 100, KeyInput);
					}
					else if (Parameter2No == 2)
					{
						//ChangeParameterVal(pint32_t, &Frequency_stepper, 100, KeyInput);
					}
					else if (Parameter2No == 3)
					{
						//ChangeParameterVal(pint32_t, &Dir, 1, KeyInput);
					}
				}
				//			if(Dir == 1){
				//			  Locate_Rle(angle_stepper,Frequency_stepper,CW);  //绝对定位函数
				//			}
				//			else if(Dir == 2){
				//			  Locate_Rle(angle_stepper,Frequency_stepper,CCW);  //绝对定位函数
				//			}
				break;
			case 4:
				//			Mode = Input_Distance;
				//标题
							LCD_printf(X(2),0,"Input_D");
				//显示参数
				//			LCD_printf(X(2),1*16,"D1=%s   ",float2str(Distance,2));
				//			LCD_printf(X(2),2*16,"D5=%s   ",float2str(Distance,2));
				//			LCD_printf(X(2),3*16,"D10=%s   ",float2str(Distance,2));
				//			LCD_printf(X(2),4*16,"D20=%s   ",float2str(Distance,2));
				//			//修改函数
				//			if(state == 2){
				//					if(Parameter2No == 0){
				//							ChangeParameterVal(pfloat,&Distance,1,KeyInput);
				//					}
				//					else if(Parameter2No == 1){
				//							ChangeParameterVal(pfloat,&Distance,5,KeyInput);
				//					}
				//					else if(Parameter2No == 2){
				//							ChangeParameterVal(pfloat,&Distance,10,KeyInput);
				//					}
				//					else if(Parameter2No == 3){
				//							ChangeParameterVal(pfloat,&Distance,20,KeyInput);
				//					}
				//			}

				break;
			case 6:
				//				LCD_printf(bit2,X(1),"dis");
				//			Mode = Input_DistanceAngle;
				//			//标题
				//			LCD_printf(X(10),0,"Input_DA");
				//			//显示参数
				//			LCD_printf(X(2),1*16,"D1=%s   ",float2str(Distance,2));
				//			LCD_printf(X(2),2*16,"D5=%s   ",float2str(Distance,2));
				//			LCD_printf(X(2),3*16,"D20=%s   ",float2str(Distance,2));
				//
				//			LCD_printf(X(2),5*16,"A1=%s   ",float2str(Angle_yaw,2));
				//			LCD_printf(X(2),6*16,"A5=%s   ",float2str(Angle_yaw,2));
				//			LCD_printf(X(2),7*16,"A20=%s   ",float2str(Angle_yaw,2));
				//			//修改函数
				//			if(state == 2){
				//					if(Parameter2No == 0){
				//							ChangeParameterVal(pfloat,&Distance,1,KeyInput);
				//					}
				//					else if(Parameter2No == 1){
				//							ChangeParameterVal(pfloat,&Distance,5,KeyInput);
				//					}
				//					else if(Parameter2No == 2){
				//							ChangeParameterVal(pfloat,&Distance,20,KeyInput);
				//					}
				//					else if(Parameter2No == 4){
				//							ChangeParameterVal(pfloat,&Angle_yaw,1,KeyInput);
				//					}
				//					else if(Parameter2No == 5){
				//							ChangeParameterVal(pfloat,&Angle_yaw,5,KeyInput);
				//					}
				//					else if(Parameter2No == 6){
				//							ChangeParameterVal(pfloat,&Angle_yaw,20,KeyInput);
				//					}
				//			}

				break;
			}
		}

		//////////////////////  修改没有二级菜单的位置的参数   ////////////////////////////////////////////////////////////////////////
		if (state == 0)
		{
			//没有二级菜单的位置
			if (en_sec_menu[ParameterNo] == 0)
			{

				switch (ParameterNo)
				{
					//					case 8: break;
					//					case 9: ChangeFlagVal(&SIMG,KeyInput,0,1);
					//									if(SIMG == 1){
					//										if(finish_flag_032 == 1){
					//											seekfree_sendimg_032();
					//											finish_flag_032 = 0;
					//										}
					//									}
					//									break;

					//					case 12: ChangeFlagVal(&ChargFlag,KeyInput,0,1);break;
					//					case 13: ChangeFlagVal(&DischargFlag,KeyInput,0,1);break;
				}
			}
			//		//充放电手动控制
			//		if(ChargFlag == 1){	Charge = 0;}
			//		else 	Charge = 1;
			//
			//		if(DischargFlag == 1){	Discharge = 0;}
			//		else	Discharge = 1;
			//		//电压值显示
			//		LCD_printf(X(16),6*16,"Volt=%s   ",float2str(Voltage,2));
			//		//超声波距离显示
			//		UltrasonicRanging();
			//		LCD_printf(X(16),7*16,"Dis=%s   ",float2str(Distance,2));
		}
		//////////////////////  没有二级菜单的位置的参数 结束  ////////////////////////////////////////////////////////////////////////
		if (state != 2)
		{
			//			TIM_SetCompare1(TIM4,STEPD1.MidPWM);
			//			TIM_SetCompare3(TIM4,STEPD2.MidPWM);
		}
	}
}
