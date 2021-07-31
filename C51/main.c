#include "stc8g.h"
#include "intrins.h"
#include "stdio.h"

#define COMMAND_LENGTH 6
#define MESSAGE_LENGTH 3

//单片机定义
#define ET4 0x40
#define T4IF 0x04

//消息定义
typedef struct MESSAGE
{
	unsigned char command_data[MESSAGE_LENGTH];
	int done;
  struct message * next;
}message;


//全局变量定义
unsigned char recv_char = 0;
char recv_count = 0;
int time4_count = 0;
unsigned char origin_command[COMMAND_LENGTH];
unsigned char decode_message[MESSAGE_LENGTH];
unsigned char encode_message[MESSAGE_LENGTH];
unsigned char return_data[COMMAND_LENGTH];
message * iterator;

//全局函数声明
void decode(unsigned char *src, unsigned char *dst);
void encode(unsigned char *src, unsigned char *dst);
void add_new_message(unsigned char *msg_data);
void Delay1ms();
void send_key_code(int key_code);

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
}

/***
	* 串口3初始化函数
	*/
void uart_3_init(void)
{
	S3CON = 0x10;		//8位数据,可变波特率
	S3CON &= 0xBF;		//串口3选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0xB4;		//设置定时初始值
	T2H = 0xFF;		//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
}

/***
	* 串口4初始化函数
	*/
void uart_4_init(void)		//115200bps@35.000MHz
{
	S4CON = 0x10;		//8位数据,可变波特率
	S4CON &= 0xBF;		//串口4选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0xB4;		//设置定时初始值
	T2H = 0xFF;		//设置定时初始值
	AUXR |= 0x10;		//定时器2开始计时
}

void timer_4_init()	//100微秒@35MHz
{
	T4L = 0x48;		//设置定时初始值
	T4H = 0x77;		//设置定时初始值
	IE2 = ET4;
}

/***
	* 串口1数据发送
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
	* 串口3数据发送
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
	* 串口4数据发送
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
	* INT0中断，用作放板感应
	*/
void int0_isr() interrupt 0
{
	if(T4T3M == 0x80)	//timer4已经开启
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
	* 定时器4中断，放板后定时触发
	*/
void time4_isr() interrupt 20
{
	if(++time4_count > 400)	//4秒后按下按键
	{
		time4_count = 0;
		//send_key_code(0x29);	//发送Esc按键
		SBUF = 0xAB;
		while(!TI);
		TI=0;
		T4T3M = 0x00;
		AUXINTIF &= ~T4IF;
	}
}

/***
	* 串口1中断，串口1消息接收
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
	* 串口4中断，串口4消息接收
	*	 接收语音模块发送过来的数据，并转发到串口1
	*/
void uart4_isr() interrupt 18
{
	
}

void IapIdle() 
{ 
	IAP_CONTR = 0; //关闭 IAP 功能
	IAP_CMD = 0; //清除命令寄存器
	IAP_TRIG = 0; //清除触发寄存器
	IAP_ADDRH = 0x80; //将地址设置到非 IAP 区域
	IAP_ADDRL = 0; 
} 

char IapRead(int addr) 
{ 
	char dat; 

	IAP_CONTR = 0x80; //使能 IAP
	IAP_TPS = 12; //设置擦除等待参数 12MHz
	IAP_CMD = 1; //设置 IAP 读命令
	IAP_ADDRL = addr; //设置 IAP 低地址
	IAP_ADDRH = addr >> 8; //设置 IAP 高地址
	IAP_TRIG = 0x5a; //写触发命令(0x5a)
	IAP_TRIG = 0xa5; //写触发命令(0xa5)
	_nop_(); 
	dat = IAP_DATA; //读 IAP 数据
	IapIdle(); //关闭 IAP 功能
	return dat; 
} 

/***
	* ADC读取函数
	*	STC8G2K64S4 ADC通道
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
	
	ADCCFG = 0x2f; //设置 ADC 时钟为系统时钟/2/16/16
	switch(channel)
	{
		case 0:
			P1M0 = 0x00; //设置 P1.0 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x80; //使能 ADC 模块
			break;
		case 1:
			P1M0 = 0x00; //设置 P1.1 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x81; //使能 ADC 模块
			break;
		case 2:
			P1M0 = 0x00; //设置 P1.2 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x82; //使能 ADC 模块
			break;
		case 3:
			P1M0 = 0x00; //设置 P1.3 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x83; //使能 ADC 模块
			break;
		case 4:
			P1M0 = 0x00; //设置 P1.4 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x84; //使能 ADC 模块
			break;
		case 5:
			P1M0 = 0x00; //设置 P1.5 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x85; //使能 ADC 模块
			break;
		case 6:
			P1M0 = 0x00; //设置 P1.6 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x86; //使能 ADC 模块
			break;
		case 7:
			P1M0 = 0x00; //设置 P1.7 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x87; //使能 ADC 模块
			break;
		case 8:
			P1M0 = 0x00; //设置 P0.0 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x88; //使能 ADC 模块
			break;
		case 9:
			P1M0 = 0x00; //设置 P0.1 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x89; //使能 ADC 模块
			break;
		case 10:
			P1M0 = 0x00; //设置 P0.2 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x8A; //使能 ADC 模块
			break;
		case 11:
			P1M0 = 0x00; //设置 P0.3 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x8B; //使能 ADC 模块
			break;
		case 12:
			P1M0 = 0x00; //设置 P0.4 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x8C; //使能 ADC 模块
			break;
		case 13:
			P1M0 = 0x00; //设置 P0.5 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x8D; //使能 ADC 模块
			break;
		case 14:
			P1M0 = 0x00; //设置 P0.6 为 ADC 口
			P1M1 = 0x01; 
			ADC_CONTR = 0x8E; //使能 ADC 模块
			break;
		default:
			return;
	}
	ADC_CONTR |= 0x40; //启动 AD 转换
	_nop_(); 
	_nop_(); 
	while (!(ADC_CONTR & 0x20)); //查询 ADC 完成标志
	ADC_CONTR &= ~0x20; //清完成标志
}

/***
	* 延时函数
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

//自定义函数

/***
	* 初始化函数
	*/
void init()
{
	iterator = 0;
	EA = 1;	//所有中断开关
	
	ES = 1;	//串口1中断开关
	
	IT0 = 1;	//INT0 下降沿中断
	EX0 = 1;	//INT0 外部中断开关
	
	uart_1_init();	//串口1初始化
	uart_3_init();	//串口1初始化
	timer_4_init();	//定时器4初始化
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
	Delay1ms();
	send_data_uart_3(key_up, 8);
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
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x02:	//读取单片机信息
			encode_message[0] = *source;
			encode_message[1] = IapRead(0x0001);
			encode_message[2] = IapRead(0x0002);
			encode(encode_message, return_data);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x03:	//读取ADC指定通道电压
			get_ad_value(*(source+1));
			encode_message[0] = *source;
			encode_message[1] = ADC_RES;
			encode_message[2] = ADC_RESL;
			encode(encode_message, return_data);
			send_data_uart_1(return_data, COMMAND_LENGTH);
			break;
		case 0x04:	//识别语音
			break;
		case 0x05:	//发送指定按键
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
	* 延时函数n毫秒
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
	* 加密函数 随机产生3字节，与后面消息内容异或加密
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
	* 解密函数，异或解密
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
	* 消息添加到队列函数
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
	* 生成新消息函数
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
	* 主函数
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