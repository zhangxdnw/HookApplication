#include "stc8g.h"
#include "intrins.h"
#include "stdio.h"

#define COMMAND_LENGTH 6
#define MESSAGE_LENGTH 3

//��Ƭ������
#define ET4 0x40
#define T4IF 0x04

//��Ϣ����
typedef struct MESSAGE
{
	unsigned char command_data[MESSAGE_LENGTH];
	int done;
  struct message * next;
}message;


//ȫ�ֱ�������
unsigned char recv_char = 0;
char recv_count = 0;
int time4_count = 0;
unsigned char origin_command[COMMAND_LENGTH];
unsigned char decode_message[MESSAGE_LENGTH];
unsigned char encode_message[MESSAGE_LENGTH];
unsigned char return_data[COMMAND_LENGTH];
message * iterator;

//ȫ�ֺ�������
void decode(unsigned char *src, unsigned char *dst);
void encode(unsigned char *src, unsigned char *dst);
void add_new_message(unsigned char *msg_data);
void Delay1ms();
void send_key_code(int key_code);

//ϵͳ���庯��

/***
	* ����1��ʼ������
	*/
void uart_1_init(void)	//115200bps@35MHz	
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
	T2L = 0xB4;		//���ö�ʱ��ʼֵ
	T2H = 0xFF;		//���ö�ʱ��ʼֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
}

/***
	* ����3��ʼ������
	*/
void uart_3_init(void)
{
	S3CON = 0x10;		//8λ����,�ɱ䲨����
	S3CON &= 0xBF;		//����3ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
	T2L = 0xB4;		//���ö�ʱ��ʼֵ
	T2H = 0xFF;		//���ö�ʱ��ʼֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
}

/***
	* ����4��ʼ������
	*/
void uart_4_init(void)		//115200bps@35.000MHz
{
	S4CON = 0x10;		//8λ����,�ɱ䲨����
	S4CON &= 0xBF;		//����4ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
	T2L = 0xB4;		//���ö�ʱ��ʼֵ
	T2H = 0xFF;		//���ö�ʱ��ʼֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
}

void timer_4_init()	//100΢��@35MHz
{
	T4L = 0x48;		//���ö�ʱ��ʼֵ
	T4H = 0x77;		//���ö�ʱ��ʼֵ
	IE2 = ET4;
}

/***
	* ����1���ݷ���
	*/
void send_data_uart_1(unsigned char *source, int length)
{
	int i;
	for(i =0;i<length;i++)
	{
		SBUF = *(source+i);
		while(!TI);
		TI = 0;
	}
}

/***
	* ����3���ݷ���
	*/
void send_data_uart_3(unsigned char *source, int length)
{
	int i;
	for(i =0;i<length;i++)
	{
		S3BUF = *(source+i);
		Delay1ms();
	}
}


/***
	* ����4���ݷ���
	*/
void send_data_uart_4(unsigned char *source, int length)
{
	int i;
	for(i =0;i<length;i++)
	{
		S4BUF = *(source+i);
		Delay1ms();
	}
}

/***
	* INT0�жϣ������Ű��Ӧ
	*/
void int0_isr() interrupt 0
{
	if(T4T3M == 0x80)	//timer4�Ѿ�����
	{
		time4_count = 0;
	}
	else
	{
		time4_count = 0;
		T4T3M = 0x80;
	}
}

/***
	* ��ʱ��4�жϣ��Ű��ʱ����
	*/
void time4_isr() interrupt 20
{
	if(++time4_count > 400)	//4����°���
	{
		time4_count = 0;
		//send_key_code(0x29);	//����Esc����
		SBUF = 0xAB;
		while(!TI);
		TI=0;
		T4T3M = 0x00;
		AUXINTIF &= ~T4IF;
	}
}

/***
	* ����1�жϣ�����1��Ϣ����
	*/
void uart1_isr() interrupt 4
{
	if(RI == 1)	//recv
	{
		recv_char = SBUF;
		RI = 0;
		if(recv_count<COMMAND_LENGTH)
		{
			origin_command[recv_count++] = recv_char;
			if(recv_count >= COMMAND_LENGTH)
			{
				decode(origin_command, decode_message);
				//send_data_uart_1(decode_message, MESSAGE_LENGTH);
				add_new_message(decode_message);
				recv_count = 0;
			}
		}
	}
}

/***
	* ����4�жϣ�����4��Ϣ����
	*	 ��������ģ�鷢�͹��������ݣ���ת��������1
	*/
void uart4_isr() interrupt 18
{
	
}

void IapIdle() 
{ 
	IAP_CONTR = 0; //�ر� IAP ����
	IAP_CMD = 0; //�������Ĵ���
	IAP_TRIG = 0; //��������Ĵ���
	IAP_ADDRH = 0x80; //����ַ���õ��� IAP ����
	IAP_ADDRL = 0; 
} 

char IapRead(int addr) 
{ 
	char dat; 

	IAP_CONTR = 0x80; //ʹ�� IAP
	IAP_TPS = 12; //���ò����ȴ����� 12MHz
	IAP_CMD = 1; //���� IAP ������
	IAP_ADDRL = addr; //���� IAP �͵�ַ
	IAP_ADDRH = addr >> 8; //���� IAP �ߵ�ַ
	IAP_TRIG = 0x5a; //д��������(0x5a)
	IAP_TRIG = 0xa5; //д��������(0xa5)
	_nop_(); 
	dat = IAP_DATA; //�� IAP ����
	IapIdle(); //�ر� IAP ����
	return dat; 
} 

/***
	* ADC��ȡ����
	*	STC8G2K64S4 ADCͨ��
	*	ADC0	P1.0
	*	ADC1	P1.1
	*	ADC2	P1.2
	*	ADC3	P1.3
	*	ADC4	P1.4
	*	ADC5	P1.5
	*	ADC6	P1.6
	*	ADC7	P1.7
	*	ADC8	P0.0
	*	ADC9	P0.1
	*	ADC10	P0.2
	*	ADC11	P0.3
	*	ADC12	P0.4
	*	ADC13	P0.5
	*	ADC14	P0.6
	*	ADC15	P0.7
	*/
void get_ad_value(int channel)
{
	unsigned int ret;
	
	ADCCFG = 0x2f; //���� ADC ʱ��Ϊϵͳʱ��/2/16/16
	switch(channel)
	{
		case 0:
			P1M0 = 0x00; //���� P1.0 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x80; //ʹ�� ADC ģ��
			break;
		case 1:
			P1M0 = 0x00; //���� P1.1 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x81; //ʹ�� ADC ģ��
			break;
		case 2:
			P1M0 = 0x00; //���� P1.2 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x82; //ʹ�� ADC ģ��
			break;
		case 3:
			P1M0 = 0x00; //���� P1.3 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x83; //ʹ�� ADC ģ��
			break;
		case 4:
			P1M0 = 0x00; //���� P1.4 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x84; //ʹ�� ADC ģ��
			break;
		case 5:
			P1M0 = 0x00; //���� P1.5 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x85; //ʹ�� ADC ģ��
			break;
		case 6:
			P1M0 = 0x00; //���� P1.6 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x86; //ʹ�� ADC ģ��
			break;
		case 7:
			P1M0 = 0x00; //���� P1.7 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x87; //ʹ�� ADC ģ��
			break;
		case 8:
			P1M0 = 0x00; //���� P0.0 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x88; //ʹ�� ADC ģ��
			break;
		case 9:
			P1M0 = 0x00; //���� P0.1 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x89; //ʹ�� ADC ģ��
			break;
		case 10:
			P1M0 = 0x00; //���� P0.2 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x8A; //ʹ�� ADC ģ��
			break;
		case 11:
			P1M0 = 0x00; //���� P0.3 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x8B; //ʹ�� ADC ģ��
			break;
		case 12:
			P1M0 = 0x00; //���� P0.4 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x8C; //ʹ�� ADC ģ��
			break;
		case 13:
			P1M0 = 0x00; //���� P0.5 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x8D; //ʹ�� ADC ģ��
			break;
		case 14:
			P1M0 = 0x00; //���� P0.6 Ϊ ADC ��
			P1M1 = 0x01; 
			ADC_CONTR = 0x8E; //ʹ�� ADC ģ��
			break;
		default:
			return;
	}
	ADC_CONTR |= 0x40; //���� AD ת��
	_nop_(); 
	_nop_(); 
	while (!(ADC_CONTR & 0x20)); //��ѯ ADC ��ɱ�־
	ADC_CONTR &= ~0x20; //����ɱ�־
}

/***
	* ��ʱ����
	*/
void Delay1ms()		//@35MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	i = 46;
	j = 113;
	do
	{
		while (--j);
	} while (--i);
}

//�Զ��庯��

/***
	* ��ʼ������
	*/
void init()
{
	iterator = 0;
	EA = 1;	//�����жϿ���
	
	ES = 1;	//����1�жϿ���
	
	IT0 = 1;	//INT0 �½����ж�
	EX0 = 1;	//INT0 �ⲿ�жϿ���
	
	uart_1_init();	//����1��ʼ��
	uart_3_init();	//����1��ʼ��
	timer_4_init();	//��ʱ��4��ʼ��
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
	Delay1ms();
	send_data_uart_3(key_up, 8);
}

/***
	* ����ִ�к���
	*/
void execute(unsigned char *source)
{
	switch(*source)
	{
		case 0x01:	//���֣�ԭ���ݷ���
			encode_message[0] = *source;
			encode_message[1] = *(source+1);
			encode_message[2] = *(source+2);
			encode(encode_message, return_data);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x02:	//��ȡ��Ƭ����Ϣ
			encode_message[0] = *source;
			encode_message[1] = IapRead(0x0001);
			encode_message[2] = IapRead(0x0002);
			encode(encode_message, return_data);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x03:	//��ȡADCָ��ͨ����ѹ
			get_ad_value(*(source+1));
			encode_message[0] = *source;
			encode_message[1] = ADC_RES;
			encode_message[2] = ADC_RESL;
			encode(encode_message, return_data);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x04:	//ʶ������
			break;
		case 0x05:	//����ָ������
			send_key_code(*(source+1));
			encode_message[0] = *source;
			encode_message[1] = 0x00;
			encode_message[2] = 0x00;
			encode(encode_message, return_data);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
	}
}

/***
	* ��ʱ����n����
	*/
void delay_n_ms(int n)
{
	int i = 0;
	for(i;i<n;i++)
	{
		Delay1ms();
	}
}

/***
	* ���ܺ��� �������3�ֽڣ��������Ϣ����������
	*/
void encode(unsigned char *src, unsigned char *dst)
{
	int index = 0;
	for(index; index < MESSAGE_LENGTH; index++)
	{
		unsigned char random_data = rand()>>8;
		*(dst+index) = random_data;
		*(dst+index + MESSAGE_LENGTH) = * (src + index) ^ *(dst+index);
	}
}

/***
	* ���ܺ�����������
	*/
void decode(unsigned char *src, unsigned char *dst)
{
	int index = 0;
	for(index; index < MESSAGE_LENGTH; index++)
	{
		*(dst+index) = *(src+index) ^ * (src+index+MESSAGE_LENGTH);
	}
}

/***
	* ��Ϣ��ӵ����к���
	*/
void add_message_to_list(message msg)
{
	if(iterator == 0)
	{
		iterator = &msg;
	}
	else
	{
		message *tmp = iterator;
		while(tmp->next != 0)
		{
			tmp = tmp->next;
		}
		tmp->next = &msg;
	}
}

/***
	* ��������Ϣ����
	*/
void add_new_message(unsigned char * msg_data)
{
	message msg;
	int i;
	for(i = 0; i < MESSAGE_LENGTH; i++)
	{
		msg.command_data[i] = *(msg_data + i);
	}
	msg.next = 0;
	msg.done = 0;
	add_message_to_list(msg);
}

/***
	* ������
	*/
void main()
{
	init();

	while(1)
	{
		if(iterator != 0)
		{
			if(iterator->done == 0)
			{
				execute(iterator->command_data);
				iterator->done = 1;
				iterator = iterator->next;
			}
		}
	}
}