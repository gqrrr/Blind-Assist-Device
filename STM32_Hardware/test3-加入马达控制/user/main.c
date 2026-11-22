#include "stm32f10x.h"

volatile uint16_t capture_start=0;//上升沿时间
volatile uint16_t capture_end=0;//下降沿时间
volatile uint8_t is_captured=0;//完成标志位
volatile float distance_cm=0;//测得的距离
volatile uint8_t pc_danger_flag=0;//记录电脑摄像头传递的数据

//延迟函数
void App_Delay(uint32_t us);
//初始化USART1模块
void USART1_Init(void);
//超声波与定时器引脚初始化
void HCSR04_Init(void);
//超声波测距
void Trigger(void);
//马达控制引脚初始化--初始化PB5
void Motor_Init(void);
//定时器2中断函数
void TIM2_IRQHandler(void);
//串口1中断函数
void USART1_IRQHandler(void);

int main(void)
{
	USART1_Init();
	HCSR04_Init();
	Motor_Init();
	
	while(1)
	{
		Trigger();
		App_Delay(60000);
		//危险判定
		uint8_t is_danger=0;
		//条件一--超声波距离过近
		if(is_captured==1)
		{
			distance_cm=(float)(capture_end-capture_start)*0.017;
			if(distance_cm>0&&distance_cm<100.0)
			{
				is_danger=1;
			}
		}
		//条件二--摄像头识别过近
		if(pc_danger_flag==1)
		{
			is_danger=1;
		}
		//执行操作
		if(is_danger==1)
		{
			GPIO_WriteBit(GPIOB,GPIO_Pin_5,Bit_SET);
		}
		else
		{
			GPIO_WriteBit(GPIOB,GPIO_Pin_5,Bit_RESET);
		}
	}
}

//延迟函数
void App_Delay(uint32_t us)
{
	while(us--)
	{
		for(uint16_t i=0;i<8;i++);
	}
}

//初始化USART1模块
void USART1_Init(void)
{
	//初始化引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	//PA9
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//PA10
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//初始化USART1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate=115200;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Tx |USART_Mode_Rx;
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
	//闭合总开关
	USART_Cmd(USART1,ENABLE);
}

//超声波与定时器引脚初始化
void HCSR04_Init(void)
{
	//初始化PA1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//初始化PA0-Trig
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//初始化时基单元
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period=65535;
	TIM_TimeBaseInitStruct.TIM_Prescaler=71;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
	//配置通道二为输入捕获
	TIM_ICInitTypeDef TIM_ICInitStruct;
	TIM_ICInitStruct.TIM_Channel=TIM_Channel_2;
	TIM_ICInitStruct.TIM_ICFilter=0x03;
	TIM_ICInitStruct.TIM_ICPolarity=TIM_ICPolarity_Rising;
	TIM_ICInitStruct.TIM_ICPrescaler=TIM_ICPSC_DIV1;
	TIM_ICInitStruct.TIM_ICSelection=TIM_ICSelection_DirectTI;
	TIM_ICInit(TIM2,&TIM_ICInitStruct);
	//开启TIM2捕获中断
	TIM_ITConfig(TIM2,TIM_IT_CC2,ENABLE);
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	//使能定时器
	TIM_Cmd(TIM2,ENABLE);
}

//超声波测距
void Trigger(void)
{
	is_captured=0;
	GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);
	App_Delay(15);
	GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_RESET);
}

//马达控制引脚初始化--初始化PB5
void Motor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	//默认低电平--不震动
	GPIO_WriteBit(GPIOB,GPIO_Pin_5,Bit_RESET);
}

//定时器2中断函数
void TIM2_IRQHandler(void)
{
	if(TIM_GetFlagStatus(TIM2,TIM_IT_CC2)==SET)
	{
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)==1)
		{
			capture_start=TIM_GetCapture2(TIM2);
			TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Falling);
		}
		else
		{
			capture_end=TIM_GetCapture2(TIM2);
			TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Rising);
			is_captured=1;
		}
		TIM_ClearITPendingBit(TIM2,TIM_IT_CC2);
	}
}
	
//串口1中断函数
void USART1_IRQHandler(void)
{
	if(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==SET)
	{
		uint8_t data=USART_ReceiveData(USART1);
		if(data=='1')
		{
			pc_danger_flag=1;
		}
		else if(data=='0')
		{
			pc_danger_flag=0;
		}
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}


