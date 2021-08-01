/***
	*
	* ʹ�ú꾧ʾ�������޸ĵĳ���
	*/
#include "stdlib.h"
#include "config.h"
#include "gpio.h"
#include "UART.h"
#include "delay.h"
#include "Exti.h"
#include "timer.h"
#include "adc.h"


/*************	���س�������	**************/
#define COMMAND_LENGTH 8
#define MESSAGE_LENGTH 4

/*************	���ر�������	**************/
int timer0_count = 0;
u8 encode_message[MESSAGE_LENGTH];
u8 decode_message[MESSAGE_LENGTH];
u8 return_data[COMMAND_LENGTH];
u16	adc_value;

/*************	���غ�������	**************/



/*************  �ⲿ�����ͱ������� *****************/


/******************* IO���ú��� *******************/
void	GPIO_config(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;		//�ṹ����

	//COM1 P3.0 P3.1
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;		//ָ��Ҫ��ʼ����IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//ָ��IO������������ʽ,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P3,&GPIO_InitStructure);	//��ʼ��
	
	//INT0 P3.2
	GPIO_InitStructure.Pin  = GPIO_Pin_2;			//ָ��Ҫ��ʼ����IO, GPIO_Pin_0 ~ GPIO_Pin_7, �����
	GPIO_InitStructure.Mode = GPIO_PullUp;			//ָ��IO������������ʽ,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P3,&GPIO_InitStructure);	//��ʼ��
	
	//COM3 P5.0 P5.1
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1;		//ָ��Ҫ��ʼ����IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//ָ��IO������������ʽ,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P5,&GPIO_InitStructure);	//��ʼ��
	
	//COM4 P5.2 P5.3
	GPIO_InitStructure.Pin  = GPIO_Pin_2 | GPIO_Pin_3;		//ָ��Ҫ��ʼ����IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//ָ��IO������������ʽ,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P5,&GPIO_InitStructure);	//��ʼ��
	
	//ADC 0-14 P1.0~P1.7 P0.0~P0.7
	GPIO_InitStructure.Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;		//ָ��Ҫ��ʼ����IO, GPIO_Pin_0 ~ GPIO_Pin_7
	GPIO_InitStructure.Mode = GPIO_PullUp;	//ָ��IO������������ʽ,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
	GPIO_Inilize(GPIO_P1,&GPIO_InitStructure);	//��ʼ��
	//GPIO_Inilize(GPIO_P0,&GPIO_InitStructure);	//��ʼ��
}

/***************  ���ڳ�ʼ������ *****************/
void	UART_config(void)
{
	COMx_InitDefine		COMx_InitStructure;					//�ṹ����
	
	//COM1
	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//ģʽ,       UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//ʹ�ò�����,   BRT_Timer1, BRT_Timer2 (ע��: ����2�̶�ʹ��BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//������, һ�� 110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//��������,   ENABLE��DISABLE
	COMx_InitStructure.BaudRateDouble = DISABLE;			//�����ʼӱ�, ENABLE��DISABLE
	COMx_InitStructure.UART_Interrupt = ENABLE;				//�ж�����,   ENABLE��DISABLE
	COMx_InitStructure.UART_Polity    = Polity_0;			//ָ���ж����ȼ�(�͵���) Polity_0,Polity_1,Polity_2,Polity_3
	COMx_InitStructure.UART_P_SW      = UART1_SW_P30_P31;	//�л��˿�,   UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
	UART_Configuration(UART1, &COMx_InitStructure);		//��ʼ������1
	
	//COM3
	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//ģʽ,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer3;			//ʹ�ò�����,   BRT_Timer2, BRT_Timer3 (ע��: ����2�̶�ʹ��BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//������,     110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//��������,   ENABLE��DISABLE
	COMx_InitStructure.UART_Interrupt = ENABLE;				//�ж�����,   ENABLE��DISABLE
	COMx_InitStructure.UART_Polity    = Polity_0;			//ָ���ж����ȼ�(�͵���) Polity_0,Polity_1,Polity_2,Polity_3
	COMx_InitStructure.UART_P_SW      = UART3_SW_P50_P51;	//�л��˿�,   UART3_SW_P00_P01,UART3_SW_P50_P51
	UART_Configuration(UART3, &COMx_InitStructure);		//��ʼ������3
	
	//COM4
	COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;		//ģʽ,   UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
	COMx_InitStructure.UART_BRT_Use   = BRT_Timer4;			//ʹ�ò�����,   BRT_Timer2, BRT_Timer4 (ע��: ����2�̶�ʹ��BRT_Timer2)
	COMx_InitStructure.UART_BaudRate  = 115200ul;			//������,     110 ~ 115200
	COMx_InitStructure.UART_RxEnable  = ENABLE;				//��������,   ENABLE��DISABLE
	COMx_InitStructure.UART_Interrupt = ENABLE;				//�ж�����,   ENABLE��DISABLE
	COMx_InitStructure.UART_Polity    = Polity_0;			//ָ���ж����ȼ�(�͵���) Polity_0,Polity_1,Polity_2,Polity_3
	COMx_InitStructure.UART_P_SW      = UART4_SW_P52_P53;	//�л��˿�,   UART4_SW_P02_P03,UART4_SW_P52_P53
	UART_Configuration(UART4, &COMx_InitStructure);
}

/******************** INT���� ********************/
void	Exti_config(void)
{
	EXTI_InitTypeDef	Exti_InitStructure;							//�ṹ����

	Exti_InitStructure.EXTI_Interrupt = ENABLE;				//�ж�ʹ��,   ENABLE��DISABLE
	Exti_InitStructure.EXTI_Mode      = EXT_MODE_Fall;//�ж�ģʽ,   EXT_MODE_RiseFall,EXT_MODE_Fall
	Exti_InitStructure.EXTI_Polity    = Polity_0;			//ָ���ж����ȼ�(�͵���) Polity_0,Polity_1,Polity_2,Polity_3
	Ext_Inilize(EXT_INT0,&Exti_InitStructure);				//��ʼ��
}

/************************ ��ʱ������ ****************************/
void	Timer_config(void)
{
	TIM_InitTypeDef		TIM_InitStructure;						//�ṹ����
	TIM_InitStructure.TIM_Mode      = TIM_16BitAutoReload;	//ָ������ģʽ,   TIM_16BitAutoReload,TIM_16Bit,TIM_8BitAutoReload,TIM_16BitAutoReloadNoMask
	TIM_InitStructure.TIM_Polity    = Polity_0;			//ָ���ж����ȼ�(�͵���) Polity_0,Polity_1,Polity_2,Polity_3
	TIM_InitStructure.TIM_Interrupt = ENABLE;					//�ж��Ƿ�����,   ENABLE��DISABLE
	TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T;		//ָ��ʱ��Դ,     TIM_CLOCK_1T,TIM_CLOCK_12T,TIM_CLOCK_Ext
	TIM_InitStructure.TIM_ClkOut    = DISABLE;				//�Ƿ������������, ENABLE��DISABLE
	TIM_InitStructure.TIM_Value     = 65536UL - (MAIN_Fosc / (50*12));		//��ֵ,
	TIM_InitStructure.TIM_Run       = DISABLE;					//�Ƿ��ʼ����������ʱ��, ENABLE��DISABLE
	Timer_Inilize(Timer0,&TIM_InitStructure);					//��ʼ��Timer0
}

/******************* AD���ú��� *******************/
void	ADC_config(void)
{
	ADC_InitTypeDef		ADC_InitStructure;		//�ṹ����
	ADC_InitStructure.ADC_SMPduty   = 31;		//ADC ģ���źŲ���ʱ�����, 0~31��ע�⣺ SMPDUTY һ����������С�� 10��
	ADC_InitStructure.ADC_CsSetup   = 0;		//ADC ͨ��ѡ��ʱ����� 0(Ĭ��),1
	ADC_InitStructure.ADC_CsHold    = 1;		//ADC ͨ��ѡ�񱣳�ʱ����� 0,1(Ĭ��),2,3
	ADC_InitStructure.ADC_Speed     = ADC_SPEED_2X1T;		//���� ADC ����ʱ��Ƶ��	ADC_SPEED_2X1T~ADC_SPEED_2X16T
	ADC_InitStructure.ADC_Power     = ENABLE;				//ADC��������/�ر�	ENABLE,DISABLE
	ADC_InitStructure.ADC_AdjResult = ADC_RIGHT_JUSTIFIED;	//ADC�������,	ADC_LEFT_JUSTIFIED,ADC_RIGHT_JUSTIFIED
	ADC_InitStructure.ADC_Polity    = Polity_0;			//ָ���ж����ȼ�(�͵���) Polity_0,Polity_1,Polity_2,Polity_3
	ADC_InitStructure.ADC_Interrupt = DISABLE;			//�ж�����	ENABLE,DISABLE
	ADC_Inilize(&ADC_InitStructure);					//��ʼ��
	ADC_PowerControl(ENABLE);							//������ADC��Դ��������, ENABLE��DISABLE
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

/********************* Timer0�жϺ���************************/
void timer0_int (void) interrupt TIMER0_VECTOR
{
	if(timer0_count++ >= 65 * 40)		//����ֵ65���100ms
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
	* ���Ͱ�����Ϣ��������
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

/********************* ����1����ִ�к���************************/
void execute(u8 *source)
{
	decode(source, decode_message);
	switch(decode_message[0])
	{
		case 0x01:	//���֣�ԭ���ݷ���
			encode_message[0] = decode_message[0];
			encode_message[1] = decode_message[1];
			encode_message[2] = decode_message[2];
			encode_message[3] = decode_message[3];
			encode(encode_message, return_data, *source);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x02:	//��ȡ��Ƭ����Ϣ
			encode_message[0] = decode_message[0];
			encode_message[1] = 0x00;
			encode_message[2] = 0x00;
			encode_message[3] = 0x00;
			encode(encode_message, return_data, *source);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x03:	//��ȡADCָ��ͨ����ѹ
			ADC_PowerControl(ENABLE);							//������ADC��Դ��������, ENABLE��DISABLE
			adc_value = Get_ADCResult(decode_message[1]);
			encode_message[0] = decode_message[0];
			encode_message[1] = decode_message[1];
			encode_message[2] = adc_value>>8;
			encode_message[3] = (u8)adc_value;
			//send_data_uart_1(encode_message, MESSAGE_LENGTH);
			encode(encode_message, return_data, *source);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			ADC_PowerControl(DISABLE);							//������ADC��Դ��������, ENABLE��DISABLE
			break;
		case 0x04:	//ʶ������
			//TODO ��ʼʶ������
			break;
		case 0x05:	//����ָ������
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

/********************* ����4����ִ�к���************************/
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
		if(COM1.RX_TimeOut > 0)		//��ʱ����
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
		if(COM3.RX_TimeOut > 0)		//��ʱ����
		{
			if(--COM3.RX_TimeOut == 0)
			{
				if(COM3.RX_Cnt > 0)
				{
					for(i=0; i<COM3.RX_Cnt; i++)	TX3_write2buff(RX3_Buffer[i]);	//�յ�������ԭ������
				}
				COM3.RX_Cnt = 0;
			}
		}
		if(COM4.RX_TimeOut > 0)		//��ʱ����
		{
			if(--COM4.RX_TimeOut == 0)
			{
				if(COM4.RX_Cnt > 0)
				{
					//TODO ����4���ݷ��ظ���λ��
					send_speech_recognition(0x00);
				}
				COM4.RX_Cnt = 0;
			}
		}
		if(!INT0)	//�ж�INT0����
		{
			TX1_write2buff(0xAA);
			timer0_count=0;
			Timer0_Run();
		}
	}
}