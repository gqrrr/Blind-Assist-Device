#include "stm32f10x.h"
#include <stdio.h>

//串口1初始化--PA9 TX,PA10 RX
void USART1_Init(void);
//初始化板载LED
void LED_Init(void);
//串口1中断服务函数
void USART1_IRQHandler(void);

int main(void)
{
	LED_Init();
	USART1_Init();
	while(1)
	{
	}
}

void USART1_Init(void)
{
	//初始化引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
	
	//PA9
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;//复用推挽输出：复用-由USART1驱动，推挽-输出1拉高 输出0拉低
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//PA10
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;//输入浮空：使引脚电平由外部信号决定
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//初始化串口
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	
	USART_InitTypeDef USART_InitStruct;

  USART_InitStruct.USART_BaudRate=115200;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_StopBits=USART_StopBits_1;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;
	
	USART_Init(USART1,&USART_InitStruct);
	
	//开启串口接收中断
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	
	//配置中断优先级
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=1;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	//使能串口
	USART_Cmd(USART1,ENABLE);
	
}

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
}

void USART1_IRQHandler(void)
{
	//判断是否接收到数据
	if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==SET)
	{
		//读取数据
		uint8_t data=USART_ReceiveData(USART1);
		//如果数据为1，点亮
		if(data=='1')
		{
			GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_RESET);
		}
		else if(data=='0')
		{
			GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_SET);
		}
		//清除中断标志位
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}
