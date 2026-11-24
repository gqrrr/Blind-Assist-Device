#include "stm32f10x.h"
#include <string.h>

//初始化USART2
void USART2_Init(void);
//向串口2发送单个字节
void USART2_SendByte(uint8_t data);
//SYBN6288打包发送函数
void SYN6288_Speak(char *text);
//延迟函数
void App_Delay(uint32_t us);

int main(void)
{
	USART2_Init();
	App_Delay(10000);
	while(1)
	{
		SYN6288_Speak("[v1]前方有障碍 请注意安全");
		App_Delay(6000000);
	}
}

void App_Delay(uint32_t us)
{
	while(us--)
	{
		for(uint16_t i=0;i<8;i++);
	}
}

void USART2_Init(void)
{
	//初始化引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	//PA2
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//PA3
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//初始化USART2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate=9600;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Tx;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_StopBits=USART_StopBits_1;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART2,&USART_InitStruct);
	//使能
	USART_Cmd(USART2,ENABLE);
}

void USART2_SendByte(uint8_t data)
{
	USART_SendData(USART2,data);
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET);
}

void SYN6288_Speak(char *text)
{
	uint8_t frame[300];//最大发送100字节
	uint8_t len=strlen(text);//求文本长度
	uint8_t i;
	uint8_t checksum=0;//异或校验和
	//构造帧头和命令
	frame[0]=0xFD;
	frame[1]=(len+3)>>8;//数据区长度高字节
	frame[2]=(len+3)&0xFF;//数据区长度低字节
	frame[3]=0x01;//命令字：合成播放
	frame[4]=0x00;//背景音乐
	//把文字加进去-GB2312编码（默认）
	for(i=0;i<len;i++)
	{
		frame[5+i]=text[i];
	}
	//计算异或校验
	for(i=0;i<(len+5);i++)
	{
		checksum ^=frame[i];
	}
	frame[5+len]=checksum;//最后一位放校验和
	//通过串口发送给SYN6288
	for(i=0;i<(len+6);i++)
	{
		USART2_SendByte(frame[i]);
  }
}

