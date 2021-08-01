/***
	*
	* 使用宏晶示例代码修改的程序
	*/
#include "stdlib.h"
#include "config.h"
#include "gpio.h"
#include "UART.h"
#include "delay.h"
#include "Exti.h"
#include "timer.h"
#include "adc.h"


/*************	本地常量声明	**************/
#define COMMAND_LENGTH 8
#define MESSAGE_LENGTH 4

/*************	本地变量声明	**************/
int timer0_count = 0;
u8 encode_message[MESSAGE_LENGTH];
u8 decode_message[MESSAGE_LENGTH];
u8 return_data[COMMAND_LENGTH];
u16	adc_value;

/*************	本地函数声明	**************/



/*************  外部函数和变量声明 *****************/


/******************* IO配置函数 *******************/
void	GPIO_config(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义

	//COM1 P3.0 P3.1
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;		//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P3,&GPIO_InitStructure);	//初始化
	
	//INT0 P3.2
	GPIO_InitStructure.Pin  = GPIO_Pin_2;			//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7, 或操作
	GPIO_InitStructure.Mode = GPIO_PullUp;			//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P3,&GPIO_InitStructure);	//初始化
	
	//COM3 P5.0 P5.1
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;		//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P5,&GPIO_InitStructure);	//初始化
	
	//COM4 P5.2 P5.3
	GPIO_InitStructure.Pin  = GPIO_Pin_2 | GPIO_Pin_3;		//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P5,&GPIO_InitStructure);	//初始化
	
	//ADC 0-14 P1.0~P1.7 P0.0~P0.7
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;		//指定要初始化的IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P1,&GPIO_InitStructure);	//初始化
	//GPIO_Inilize(GPIO_P0,&GPIO_InitStructure);	//初始化
}

/***************  串口初始化函数 *****************/
void	UART_config(void)
{
	COMx_InitDefine		COMx_InitStructure;					//结构定义
	
	//COM1
	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//模式,       UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//使用波特率,   BRT_Timer1, BRT_Timer2 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率, 一般 110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
	COMx_InitStructure.BaudRateDouble = DISABLE;			//波特率加倍, ENABLE或DISABLE
	COMx_InitStructure.UART_Interrupt = ENABLE;				//中断允许,   ENABLE或DISABLE
	COMx_InitStructure.UART_Polity    = Polity_0;			//指定中断优先级(低到高) Polity_0,Polity_1,Polity_2,Polity_3
	COMx_InitStructure.UART_P_SW      = UART1_SW_P30_P31;	//切换端口,   UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
	UART_Configuration(UART1, &COMx_InitStructure);		//初始化串口1
	
	//COM3
	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//模式,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer3;			//使用波特率,   BRT_Timer2, BRT_Timer3 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率,     110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
	COMx_InitStructure.UART_Interrupt = ENABLE;				//中断允许,   ENABLE或DISABLE
	COMx_InitStructure.UART_Polity    = Polity_0;			//指定中断优先级(低到高) Polity_0,Polity_1,Polity_2,Polity_3
	COMx_InitStructure.UART_P_SW      = UART3_SW_P50_P51;	//切换端口,   UART3_SW_P00_P01,UART3_SW_P50_P51
	UART_Configuration(UART3, &COMx_InitStructure);		//初始化串口3
	
	//COM4
	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//模式,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer4;			//使用波特率,   BRT_Timer2, BRT_Timer4 (注意: 串口2固定使用BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//波特率,     110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//接收允许,   ENABLE或DISABLE
	COMx_InitStructure.UART_Interrupt = ENABLE;				//中断允许,   ENABLE或DISABLE
	COMx_InitStructure.UART_Polity    = Polity_0;			//指定中断优先级(低到高) Polity_0,Polity_1,Polity_2,Polity_3
	COMx_InitStructure.UART_P_SW      = UART4_SW_P52_P53;	//切换端口,   UART4_SW_P02_P03,UART4_SW_P52_P53
	UART_Configuration(UART4, &COMx_InitStructure);
}

/******************** INT配置 ********************/
void	Exti_config(void)
{
	EXTI_InitTypeDef	Exti_InitStructure;							//结构定义

	Exti_InitStructure.EXTI_Interrupt = ENABLE;				//中断使能,   ENABLE或DISABLE
	Exti_InitStructure.EXTI_Mode      = EXT_MODE_Fall;//中断模式,   EXT_MODE_RiseFall,EXT_MODE_Fall
	Exti_InitStructure.EXTI_Polity    = Polity_0;			//指定中断优先级(低到高) Polity_0,Polity_1,Polity_2,Polity_3
	Ext_Inilize(EXT_INT0,&Exti_InitStructure);				//初始化
}

/************************ 定时器配置 ****************************/
void	Timer_config(void)
{
	TIM_InitTypeDef		TIM_InitStructure;						//结构定义
	TIM_InitStructure.TIM_Mode      = TIM_16BitAutoReload;	//指定工作模式,   TIM_16BitAutoReload,TIM_16Bit,TIM_8BitAutoReload,TIM_16BitAutoReloadNoMask
	TIM_InitStructure.TIM_Polity    = Polity_0;			//指定中断优先级(低到高) Polity_0,Polity_1,Polity_2,Polity_3
	TIM_InitStructure.TIM_Interrupt = ENABLE;					//中断是否允许,   ENABLE或DISABLE
	TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T;		//指定时钟源,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
	TIM_InitStructure.TIM_ClkOut    = DISABLE;				//是否输出高速脉冲, ENABLE或DISABLE
	TIM_InitStructure.TIM_Value     = 65536UL - (MAIN_Fosc / (50*12));		//初值,
	TIM_InitStructure.TIM_Run       = DISABLE;					//是否初始化后启动定时器, ENABLE或DISABLE
	Timer_Inilize(Timer0,&TIM_InitStructure);					//初始化Timer0
}

/******************* AD配置函数 *******************/
void	ADC_config(void)
{
	ADC_InitTypeDef		ADC_InitStructure;		//结构定义
	ADC_InitStructure.ADC_SMPduty   = 31;		//ADC 模拟信号采样时间控制, 0~31（注意： SMPDUTY 一定不能设置小于 10）
	ADC_InitStructure.ADC_CsSetup   = 0;		//ADC 通道选择时间控制 0(默认),1
	ADC_InitStructure.ADC_CsHold    = 1;		//ADC 通道选择保持时间控制 0,1(默认),2,3
	ADC_InitStructure.ADC_Speed     = ADC_SPEED_2X1T;		//设置 ADC 工作时钟频率	ADC_SPEED_2X1T~ADC_SPEED_2X16T
	ADC_InitStructure.ADC_Power     = ENABLE;				//ADC功率允许/关闭	ENABLE,DISABLE
	ADC_InitStructure.ADC_AdjResult = ADC_RIGHT_JUSTIFIED;	//ADC结果调整,	ADC_LEFT_JUSTIFIED,ADC_RIGHT_JUSTIFIED
	ADC_InitStructure.ADC_Polity    = Polity_0;			//指定中断优先级(低到高) Polity_0,Polity_1,Polity_2,Polity_3
	ADC_InitStructure.ADC_Interrupt = DISABLE;			//中断允许	ENABLE,DISABLE
	ADC_Inilize(&ADC_InitStructure);					//初始化
	ADC_PowerControl(ENABLE);							//单独的ADC电源操作函数, ENABLE或DISABLE
}

void send_data_uart_1(unsigned char *buf, u8 length)
{
	u8 i = 0;
	for(i=0; i<length; i++)
		TX1_write2buff(*(buf + i));
}

void send_data_uart_3(unsigned char *buf, u8 length)
{
	u8 i = 0;
	for(i=0; i<length; i++)
		TX3_write2buff(*(buf + i));
}

void send_data_uart_4(unsigned char *buf, u8 length)
{
	u8 i = 0;
	for(i=0; i<length; i++)
		TX4_write2buff(*(buf + i));
}

/********************* Timer0中断函数************************/
void timer0_int (void) interrupt TIMER0_VECTOR
{
	if(timer0_count++ >= 65 * 40)		//经验值65大概100ms
	{
		timer0_count = 0;
		Timer0_Stop();
	}
}

void encode(u8 *src, u8 *dst, u8 seed)
{
	u8 i;
	srand(seed);
	for(i = 0; i < MESSAGE_LENGTH; i++)
	{
		*(dst + i) = rand();
		*(dst + i + MESSAGE_LENGTH) = *(dst + i) ^ *(src + i);
	}
}

void decode(u8 *src, u8 *dst)
{
	u8 i;
	for(i = 0; i < MESSAGE_LENGTH; i++)
	{
		*(dst + i) = *(src + i) ^ *(src + i + MESSAGE_LENGTH);
	}
}

/***
	* 发送按键消息给按键板
	*/
void send_key_code(int key_code)
{
	unsigned char key_down[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned char key_up[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	key_down[2] = key_code;
	send_data_uart_3(key_down, 8);
	delay_ms(1);
	send_data_uart_3(key_up, 8);
}

/********************* 串口1命令执行函数************************/
void execute(u8 *source)
{
	decode(source, decode_message);
	switch(decode_message[0])
	{
		case 0x01:	//握手，原数据返回
			encode_message[0] = decode_message[0];
			encode_message[1] = decode_message[1];
			encode_message[2] = decode_message[2];
			encode_message[3] = decode_message[3];
			encode(encode_message, return_data, *source);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x02:	//读取单片机信息
			encode_message[0] = decode_message[0];
			encode_message[1] = 0x00;
			encode_message[2] = 0x00;
			encode_message[3] = 0x00;
			encode(encode_message, return_data, *source);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x03:	//读取ADC指定通道电压
			ADC_PowerControl(ENABLE);							//单独的ADC电源操作函数, ENABLE或DISABLE
			adc_value = Get_ADCResult(decode_message[1]);
			encode_message[0] = decode_message[0];
			encode_message[1] = decode_message[1];
			encode_message[2] = adc_value>>8;
			encode_message[3] = (u8)adc_value;
			//send_data_uart_1(encode_message, MESSAGE_LENGTH);
			encode(encode_message, return_data, *source);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			ADC_PowerControl(DISABLE);							//单独的ADC电源操作函数, ENABLE或DISABLE
			break;
		case 0x04:	//识别语音
			//TODO 开始识别语音
			break;
		case 0x05:	//发送指定按键
			send_key_code(decode_message[1]);
			encode_message[0] = decode_message[0];
			encode_message[1] = decode_message[1];
			encode_message[2] = 0x00;
			encode_message[3] = 0x00;
			encode(encode_message, return_data, *source);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
	}
}

/********************* 串口4命令执行函数************************/
void send_speech_recognition(u8 result)
{
	encode_message[0] = 0x04;
	encode_message[1] = result;
	encode_message[2] = 0x00;
	encode_message[3] = 0x00;
	encode(encode_message, return_data, result);
	send_data_uart_1(return_data, COMMAND_LENGTH);
}

/**********************************************/
void main(void)
{
	u8	i;

	GPIO_config();
	UART_config();
	Exti_config();
	Timer_config();
	ADC_config();
	EA = 1;

	while (1)
	{
		delay_ms(1);
		if(COM1.RX_TimeOut > 0)		//超时计数
		{
			if(--COM1.RX_TimeOut == 0)
			{
				if(COM1.RX_Cnt > 0)
				{
					execute(RX1_Buffer);
				}
				COM1.RX_Cnt = 0;
			}
		}
		if(COM3.RX_TimeOut > 0)		//超时计数
		{
			if(--COM3.RX_TimeOut == 0)
			{
				if(COM3.RX_Cnt > 0)
				{
					for(i=0; i<COM3.RX_Cnt; i++)	TX3_write2buff(RX3_Buffer[i]);	//收到的数据原样返回
				}
				COM3.RX_Cnt = 0;
			}
		}
		if(COM4.RX_TimeOut > 0)		//超时计数
		{
			if(--COM4.RX_TimeOut == 0)
			{
				if(COM4.RX_Cnt > 0)
				{
					//TODO 串口4数据返回给上位机
					send_speech_recognition(0x00);
				}
				COM4.RX_Cnt = 0;
			}
		}
		if(!INT0)	//中断INT0产生
		{
			TX1_write2buff(0xAA);
			timer0_count=0;
			Timer0_Run();
		}
	}
}