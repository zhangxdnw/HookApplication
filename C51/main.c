#include "stc8g.h"
#include "intrins.h"
#include "stdio.h"

#define COMMAND_LENGTH 6
#define MESSAGE_LENGTH 3

typedef struct MESSAGE
{
	unsigned char command_data[MESSAGE_LENGTH];
	int done;
  struct message * next;
}message;

//全局变量定义
unsigned char recv_char = 0;
char recv_count = 0;
unsigned char origin_command[COMMAND_LENGTH];
unsigned char decode_message[MESSAGE_LENGTH];
unsigned char encode_message[MESSAGE_LENGTH];
unsigned char return_data[COMMAND_LENGTH];
message * iterator;

//
void decode(unsigned char *src, unsigned char *dst);
void encode(unsigned char *src, unsigned char *dst);
void add_new_message(unsigned char *msg_data);

//系统定义函数

/***
	* 串口1初始化函数
	*/
void uart_1_init(void)	//115200bps@35MHz	
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0xB4;		//设置定时初始值
	T2H = 0xFF;		//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
	ES = 1;	//串口1中断开关
	EA = 1;	//所有中断开关
}

void send_data(unsigned char *source, int length)
{
	int i;
	for(i =0;i<length;i++)
	{
		SBUF = *(source+i);
		while(!TI);
		TI = 0;
	}
}

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
				//send_data(decode_message, MESSAGE_LENGTH);
				add_new_message(decode_message);
				recv_count = 0;
			}
		}
	}
}

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

//自定义函数


/***
	* 初始化函数
	*/
void init()
{
	iterator = 0;
	uart_1_init();	//串口1初始化
}

/***
	* 命令执行函数
	*/
void execute(unsigned char *source)
{
	switch(*source)
	{
		case 0x01:	//握手，原数据返回
			encode_message[0] = *source;
			encode_message[1] = *(source+1);
			encode_message[2] = *(source+2);
			encode(encode_message, return_data);
			send_data(return_data, COMMAND_LENGTH);
			break;
		case 0x02:	//读取单片机信息
			break;
		case 0x03:	//读取ADC指定通道电压
			break;
		case 0x04:	//识别语音
			break;
		case 0x05:	//发送指定按键
			break;
	}
}

void delay_n_ms(int n)
{
	int i = 0;
	for(i;i<n;i++)
	{
		Delay1ms();
	}
}

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

void decode(unsigned char *src, unsigned char *dst)
{
	int index = 0;
	for(index; index < MESSAGE_LENGTH; index++)
	{
		*(dst+index) = *(src+index) ^ * (src+index+MESSAGE_LENGTH);
	}
}

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

void main()
{
	init();
	
	while(1)
	{
		delay_n_ms(10);
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
