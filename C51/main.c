#include "reg51.h"
/***
  * PIN定义
  * 使用INT3作为压下感应中断触发，收到触发10秒后，使用串口2与CH9328通信
  * 使用串口1与上位机通信
  * 使用串口3与LD3320通信，收到触发后转发给上位机
  
  * 协议
  * 上位机下发
  * 1. 读取第N通道的ADC电压
  * 01 
  * 2. 检测语音信号
  * 02
  * 3. 设置按键触发延时
  * 03
  */

#define COMMAND_LENGTH 3


typedef struct MESSAGE
{
  unsigned char[COMMAND_LENGTH] command_data;
  struct message * next;
} message

message * iterator = NULL;

/***
  * 初始化函数
  */
void init()
{
 
}

void execute(unsigned char command[COMMAND_LENGTH])
{
 
}

/***
  * 主函数
  */
void main()
{
  init();
  while(1)
  {
    if(iterator!=NULL && iterator->next != NULL)
    {
      message * message = iterator->next;
      execute(message);
      iterator = iterator->next;
    }
    else
    {
      //sleep
    }
  }
}
