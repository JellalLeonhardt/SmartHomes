/*
 * camera.c
 *	����ͷ����
 *
 */
 
#include <stm32f10x.h>
#include <includes.h>

#include "camera.h"
#include "cam_private.h"
#define GLOBALS
#include "camera_globals.h"
#include "ov2640.h"

/*
 * ȫ�ֱ��� 
 */

/* ��ʱ��¼DMA���üĴ��� */
u32 MY_DMA1_CH6_CCR_DISABLE, MY_DMA1_CH6_CCR_ENABLE;



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
	/* ��ʼ��ȫ�ֱ��� */
	
	/* VSYNC */
	/* VSYNC�����أ���ͼƬ��ʼ���½��أ�����һ��ͼƬ */
	{
		/* �����ȰѸ�ͨ�������ж϶��رգ�����Ҫʱ��ʹ�� */
		EXTI->IMR &= ~EXTI_Line7;
		EXTI->RTSR &= ~EXTI_Line7;	//������
		EXTI->FTSR &= ~EXTI_Line7;	//�½���
		
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);

		/* �Ȳ����жϣ�����Ҫ������ͷ����ʱ�ٿ� */
	}
	{
		NVIC_InitTypeDef VSYNCNVIC;
		
		VSYNCNVIC.NVIC_IRQChannel = EXTI9_5_IRQn;
		VSYNCNVIC.NVIC_IRQChannelPreemptionPriority = 0;
		VSYNCNVIC.NVIC_IRQChannelSubPriority = 0;
		VSYNCNVIC.NVIC_IRQChannelCmd = ENABLE;
		
		NVIC_Init(&VSYNCNVIC);
	}
	
	/* TIM3 Channel 1 */
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
		DATADMA.DMA_Mode = DMA_Mode_Circular;
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
		/* ��EXTI�ж������ٿ�ʼ���� */
	}

	TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}


/*
 * �жϺ���
 *	
 *	VSYNC:
 *		����DMA
 *
 *	������������
 *		DMAѭ���������ݣ�ÿ��VSYNC�źŵ�����һ��ͼƬ������ʱ����
 *		ͼƬ��Ϣ�����������������
 */
void EXTI9_5_IRQHandler()
{
	OSIntEnter();
	
	if((GPIOC->IDR & GPIO_Pin_7) == 0)
	{
		/* VSYNC�½����ж� */
		/* ��DMA */
		DMA1_Channel6->CCR = MY_DMA1_CH6_CCR_DISABLE;
		
		/* ��¼���� */
		camera_buffer_info.size = MAX_BUFFER_SIZE - DMA1_Channel6->CNDTR;

		/* ����DMA */
		DMA1_Channel6->CNDTR = MAX_BUFFER_SIZE;

		/* �ñ�־λ���൱�ڷ��ź����� */
		camera_buffer_info.sign_getphoto = 1;
		
		/* ȡ���½����жϡ������ж� */
		EXTI->FTSR &= ~EXTI_Line7;
		EXTI->EMR &= ~EXTI_Line7;
	}
	else
	{
		/* VSYNC�������ж� */
		/* ��DMA */
		DMA1_Channel6->CCR = MY_DMA1_CH6_CCR_ENABLE;
		
		/* ���½����жϡ����������ж� */
		EXTI->FTSR |= EXTI_Line7;
		EXTI->RTSR &= ~EXTI_Line7;
	}

	/* �����־ */
	EXTI->PR = EXTI_Line7;

	OSIntExit();
}

