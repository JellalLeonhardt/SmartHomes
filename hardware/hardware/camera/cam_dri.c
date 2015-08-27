/*
 * cam_dri.c
 *	����ͷ����
 *
 */
 
#include <stm32f10x.h>
#include <includes.h>

#include "camera.h"
#include "cam_private.h"
#include "ov2640.h"

/* ˽�б��� */
#ifdef USE_DMA
#define MAX_BUFFER_SIZE		30000
/* ��ʱ��¼DMA���üĴ��� */
u32 MY_DMA1_CH6_CCR_DISABLE, MY_DMA1_CH6_CCR_ENABLE;
char JPEGBuff[MAX_BUFFER_SIZE];
#endif

/*
 *	CameraLowLevelInit()
 *
 *		��ʼ������GPIO�ڣ�
 *			MCO��ʱ�����
 *			SCCB���п�����
 *			8λ����������
 *			ָʾ��
 *
 *		��Ϊ��������ʧ�ܣ���Ϊvoid���ͺ���������
 */
void CameraLowLevelInit(void)
{
	GPIO_InitTypeDef CameraIO;
	
	CameraIO.GPIO_Speed = GPIO_Speed_50MHz;
	
	
	/* ��MCO�������8MHz XCLK */
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
	
	/* ��ʼ����ڲ�RC������8MHz MCOʱ�� */
	RCC_MCOConfig(RCC_MCO_HSI);
	
	
	/* SCCB��IO�ڣ�ģ��ʱ�� */
	/* ����ʱ�øߵ�ƽ */
	/* Ϊ����ճ�ʼ���Ժ�ͻȻ����͵�ƽ���������ø� */
	SCCB_GPIO->ODR |= SCCB_C_PIN | SCCB_D_PIN;
	
	CameraIO.GPIO_Mode = GPIO_Mode_Out_OD;	/* ���������� */
	CameraIO.GPIO_Pin = SCCB_C_PIN | SCCB_D_PIN;
	GPIO_Init(SCCB_GPIO, &CameraIO);
	
	
	/* 8λ���������� */
	CameraIO.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	CameraIO.GPIO_Pin = CAMERA_DATA_PINS;
	GPIO_Init(CAMERA_DATA_GPIO, &CameraIO);
	
	
	/* ָʾ�� */
	CameraIO.GPIO_Mode = GPIO_Mode_Out_PP;
	CameraIO.GPIO_Pin = LED_PIN;
	GPIO_Init(LED_GPIO, &CameraIO);
	LED_OFF;
}
 

/*
 * CameraInit()
 *	��ʼ������ͷ�ļĴ���
 *	������ͷ����ΪJPEG���
 */
int CameraInit(void)
{
	/* Ĭ������320x240��JPEG��ʽ */
	return ov2640_jpeg_config(JPEG_320x240);
}


/*
 * ��ʼ��Ӧ����ͷ������
 *	��ʼ������ʹ���жϣ��Ӷ���Ӧ����ͷ������
 *	����ȥ��������ͷ��ʼ��������������
 */
/*
 *	GPIO���ã�
 *		PC7		VSYNC
 *		PA6		PCLK & HREF�����룩
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
	/* ԭ���������PA11��Ϊ�ⲿ�ж����룬�ָ�ΪPA6 */
	{
		EXTI_InitTypeDef PCLKEXTI;
		
		PCLKEXTI.EXTI_Line = EXTI_Line6;
		PCLKEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
		PCLKEXTI.EXTI_Trigger = EXTI_Trigger_Falling;
		PCLKEXTI.EXTI_LineCmd = ENABLE;
		
		/* �ȳ�ʼ����ӦEXTI�ļĴ���λ */
		EXTI_Init(&PCLKEXTI);
		/* Ȼ����ʧ��������VSYNC�ж�����ʹ�� */
		PCLKEXTI.EXTI_LineCmd = DISABLE;
		EXTI_Init(&PCLKEXTI);
		
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);
	}
	/* ��VSYNC�����ⲿ�жϺ��� */
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
		
		/* ��¼��ʱ�Ĵ����Ķ��� */
		MY_DMA1_CH6_CCR_DISABLE = DMA1_Channel6->CCR;
		MY_DMA1_CH6_CCR_ENABLE = MY_DMA1_CH6_CCR_DISABLE | 0x00000001;
		
//		DMA_Cmd(DMA1_Channel6, ENABLE);
		/* ��VSYNC�������ж��ٴ�DMA */
	}

	TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
#endif
}

#ifndef USE_DMA
/*
 * �жϺ���
 *	
 *	VSYNC:
 *		����PCLK�ж�
 *	PCLK:
 *		�����ݶ����ϴ���
 */
void EXTI9_5_IRQHandler()
{
	if((EXTI->PR & EXTI_Line6) != 0)
	{
		/* PCLK�жϣ��������� */
		u8 cam_in = *((u8*)&(GPIOB->IDR) + 1);

		USART_SendData(USART1, cam_in);
//		while (!(USART1->SR & USART_FLAG_TXE));
		
		/* �����־ */
		EXTI->PR = EXTI_Line6;
	}
	
	if((EXTI->PR & EXTI_Line7) != 0)
	{
		/* VSYNC�ж� */
		if((GPIOC->IDR & EXTI_Line7) != 0)
		{
			/* VSYNC�����أ�ʹ��PCLK�ж� */
			EXTI->IMR |= EXTI_Line6;
		}
		else
		{
			/* VSYNC�½��أ�ʧ��PCLK�ж� */
			EXTI->IMR &= ~EXTI_Line6;
		}
		
		/* �����־ */
		EXTI->PR = EXTI_Line7;
	}
}
#else
/*
 * �жϺ���
 *	
 *	VSYNC:
 *		����DMA
 */
int sign_send_photo = 0;
int photo_size;

void EXTI9_5_IRQHandler()
{
	static u32 sign_sending = 1;
	
	OSIntEnter();
	/* VSYNC�ж� */
	if(!sign_send_photo)
	{
		if((GPIOC->IDR & EXTI_Line7) == 0 && !sign_sending)
		{
			/* VSYNC�½��أ�ʧ��DMA�����ҷ������� */
			DMA1_Channel6->CCR = MY_DMA1_CH6_CCR_DISABLE;

			photo_size = MAX_BUFFER_SIZE - DMA1_Channel6->CNDTR;

			sign_sending = 1;
			sign_send_photo = 1;
			
			DMA1_Channel6->CNDTR = MAX_BUFFER_SIZE;
		}
		else
		{
			sign_sending = 0;
			/* VSYNC�����أ�ʹ��DMA */
			DMA1_Channel6->CCR = MY_DMA1_CH6_CCR_ENABLE;
		}
	}
	
	/* �����־ */
	EXTI->PR = EXTI_Line7;
	OSIntExit();
}
#endif

