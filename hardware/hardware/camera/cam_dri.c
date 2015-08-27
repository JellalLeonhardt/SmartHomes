/*
 * cam_dri.c
 *	摄像头驱动
 *
 */
 
#include <stm32f10x.h>
#include <includes.h>

#include "camera.h"
#include "cam_private.h"
#include "ov2640.h"

/* 私有变量 */
#ifdef USE_DMA
#define MAX_BUFFER_SIZE		30000
/* 临时记录DMA配置寄存器 */
u32 MY_DMA1_CH6_CCR_DISABLE, MY_DMA1_CH6_CCR_ENABLE;
char JPEGBuff[MAX_BUFFER_SIZE];
#endif

/*
 *	CameraLowLevelInit()
 *
 *		初始化各类GPIO口：
 *			MCO主时钟输出
 *			SCCB串行控制线
 *			8位并行数据线
 *			指示灯
 *
 *		因为几乎不会失败，设为void类型函数。。。
 */
void CameraLowLevelInit(void)
{
	GPIO_InitTypeDef CameraIO;
	
	CameraIO.GPIO_Speed = GPIO_Speed_50MHz;
	
	
	/* 从MCO引脚输出8MHz XCLK */
	RCC_APB2PeriphClockCmd(
				RCC_APB2Periph_GPIOA |
				RCC_APB2Periph_GPIOB |
				RCC_APB2Periph_GPIOC |
				RCC_APB2Periph_GPIOD |
				RCC_APB2Periph_AFIO, 
				ENABLE);
	
	CameraIO.GPIO_Mode = GPIO_Mode_AF_PP;
	CameraIO.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &CameraIO);
	
	/* 开始输出内部RC振荡器的8MHz MCO时钟 */
	RCC_MCOConfig(RCC_MCO_HSI);
	
	
	/* SCCB的IO口，模拟时序 */
	/* 空闲时置高电平 */
	/* 为避免刚初始化以后突然输出低电平，现在先置高 */
	SCCB_GPIO->ODR |= SCCB_C_PIN | SCCB_D_PIN;
	
	CameraIO.GPIO_Mode = GPIO_Mode_Out_OD;	/* 有上拉电阻 */
	CameraIO.GPIO_Pin = SCCB_C_PIN | SCCB_D_PIN;
	GPIO_Init(SCCB_GPIO, &CameraIO);
	
	
	/* 8位并行数据线 */
	CameraIO.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	CameraIO.GPIO_Pin = CAMERA_DATA_PINS;
	GPIO_Init(CAMERA_DATA_GPIO, &CameraIO);
	
	
	/* 指示灯 */
	CameraIO.GPIO_Mode = GPIO_Mode_Out_PP;
	CameraIO.GPIO_Pin = LED_PIN;
	GPIO_Init(LED_GPIO, &CameraIO);
	LED_OFF;
}
 

/*
 * CameraInit()
 *	初始化摄像头的寄存器
 *	将摄像头设置为JPEG输出
 */
int CameraInit(void)
{
	/* 默认设置320x240的JPEG格式 */
	return ov2640_jpeg_config(JPEG_320x240);
}


/*
 * 开始响应摄像头的数据
 *	初始化、并使能中断，从而响应摄像头的数据
 *	看上去就像摄像头开始工作那样。。。
 */
/*
 *	GPIO配置：
 *		PC7		VSYNC
 *		PA6		PCLK & HREF（线与）
 */
void CameraStart(void)
{
	/* VSYNC */
	{
		EXTI_InitTypeDef VSYNCEXTI;
		
		VSYNCEXTI.EXTI_Line = EXTI_Line7;
		VSYNCEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
		VSYNCEXTI.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
		VSYNCEXTI.EXTI_LineCmd = ENABLE;
		
		EXTI_Init(&VSYNCEXTI);
		
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);
	}
	{
		NVIC_InitTypeDef VSYNCNVIC;
		
		VSYNCNVIC.NVIC_IRQChannel = EXTI9_5_IRQn;
		VSYNCNVIC.NVIC_IRQChannelPreemptionPriority = 0;
		VSYNCNVIC.NVIC_IRQChannelSubPriority = 0;
		VSYNCNVIC.NVIC_IRQChannelCmd = ENABLE;
		
		NVIC_Init(&VSYNCNVIC);
	}
	
#ifndef USE_DMA
	/* 原版程序用了PA11作为外部中断输入，现改为PA6 */
	{
		EXTI_InitTypeDef PCLKEXTI;
		
		PCLKEXTI.EXTI_Line = EXTI_Line6;
		PCLKEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
		PCLKEXTI.EXTI_Trigger = EXTI_Trigger_Falling;
		PCLKEXTI.EXTI_LineCmd = ENABLE;
		
		/* 先初始化对应EXTI的寄存器位 */
		EXTI_Init(&PCLKEXTI);
		/* 然后再失能它，在VSYNC中断中再使能 */
		PCLKEXTI.EXTI_LineCmd = DISABLE;
		EXTI_Init(&PCLKEXTI);
		
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);
	}
	/* 与VSYNC共享外部中断函数 */
	{
	}
#else
	
	{
		TIM_TimeBaseInitTypeDef TIMIRQ;
		TIM_ICInitTypeDef timicirq;
		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

		TIM_TimeBaseStructInit(&TIMIRQ);
		TIM_TimeBaseInit(TIM3, &TIMIRQ);

		timicirq.TIM_Channel = TIM_Channel_1;
		timicirq.TIM_ICPolarity = TIM_ICPolarity_Rising;
		timicirq.TIM_ICPrescaler = TIM_ICPSC_DIV1;
		timicirq.TIM_ICSelection = TIM_ICSelection_DirectTI;
		timicirq.TIM_ICFilter = 0;
		TIM_ICInit(TIM3, &timicirq);
		
//		TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
	}
	{
		DMA_InitTypeDef DATADMA;
		
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		DATADMA.DMA_DIR = DMA_DIR_PeripheralSRC;
		DATADMA.DMA_BufferSize = MAX_BUFFER_SIZE;
		DATADMA.DMA_M2M = DMA_M2M_Disable;
		DATADMA.DMA_Mode = DMA_Mode_Normal;
		DATADMA.DMA_Priority = DMA_Priority_High;
		
		DATADMA.DMA_MemoryBaseAddr = (u32)JPEGBuff;
		DATADMA.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DATADMA.DMA_MemoryInc = DMA_MemoryInc_Enable;
		
		DATADMA.DMA_PeripheralBaseAddr = (u32)((u8*)&(GPIOB->IDR) + 1);
		DATADMA.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DATADMA.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		
		DMA_Init(DMA1_Channel6, &DATADMA);
		
		/* 记录此时寄存器的读数 */
		MY_DMA1_CH6_CCR_DISABLE = DMA1_Channel6->CCR;
		MY_DMA1_CH6_CCR_ENABLE = MY_DMA1_CH6_CCR_DISABLE | 0x00000001;
		
//		DMA_Cmd(DMA1_Channel6, ENABLE);
		/* 在VSYNC上升沿中断再打开DMA */
	}

	TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
#endif
}

#ifndef USE_DMA
/*
 * 中断函数
 *	
 *	VSYNC:
 *		控制PCLK中断
 *	PCLK:
 *		把数据都丢上串口
 */
void EXTI9_5_IRQHandler()
{
	if((EXTI->PR & EXTI_Line6) != 0)
	{
		/* PCLK中断，发送数据 */
		u8 cam_in = *((u8*)&(GPIOB->IDR) + 1);

		USART_SendData(USART1, cam_in);
//		while (!(USART1->SR & USART_FLAG_TXE));
		
		/* 清除标志 */
		EXTI->PR = EXTI_Line6;
	}
	
	if((EXTI->PR & EXTI_Line7) != 0)
	{
		/* VSYNC中断 */
		if((GPIOC->IDR & EXTI_Line7) != 0)
		{
			/* VSYNC上升沿，使能PCLK中断 */
			EXTI->IMR |= EXTI_Line6;
		}
		else
		{
			/* VSYNC下降沿，失能PCLK中断 */
			EXTI->IMR &= ~EXTI_Line6;
		}
		
		/* 清除标志 */
		EXTI->PR = EXTI_Line7;
	}
}
#else
/*
 * 中断函数
 *	
 *	VSYNC:
 *		控制DMA
 */
int sign_send_photo = 0;
int photo_size;

void EXTI9_5_IRQHandler()
{
	static u32 sign_sending = 1;
	
	OSIntEnter();
	/* VSYNC中断 */
	if(!sign_send_photo)
	{
		if((GPIOC->IDR & EXTI_Line7) == 0 && !sign_sending)
		{
			/* VSYNC下降沿，失能DMA，并且发送数据 */
			DMA1_Channel6->CCR = MY_DMA1_CH6_CCR_DISABLE;

			photo_size = MAX_BUFFER_SIZE - DMA1_Channel6->CNDTR;

			sign_sending = 1;
			sign_send_photo = 1;
			
			DMA1_Channel6->CNDTR = MAX_BUFFER_SIZE;
		}
		else
		{
			sign_sending = 0;
			/* VSYNC上升沿，使能DMA */
			DMA1_Channel6->CCR = MY_DMA1_CH6_CCR_ENABLE;
		}
	}
	
	/* 清除标志 */
	EXTI->PR = EXTI_Line7;
	OSIntExit();
}
#endif

