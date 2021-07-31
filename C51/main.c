#include "stc8g.h"
#include "intrins.h"
#include "stdio.h"

#define COMMAND_LENGTH 6
#define MESSAGE_LENGTH 3

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
	ES = 1;	//����1�жϿ���
	EA = 1;	//�����жϿ���
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
	* INT0�жϣ������Ű��Ӧ
	*/
void int0_isr() interrupt 0
{
	
}

/***
	* ��ʱ��4�жϣ��Ű��ʱ����
	*/
void time4_isr() interrupt 20
{
	
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
	*/
void uart4_isr() interrupt 18
{
	
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
	uart_1_init();	//����1��ʼ��
	uart_3_init();	//����1��ʼ��
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
			break;
		case 0x03:	//��ȡADCָ��ͨ����ѹ
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