/***************************************************
 *	cam_cfg.h
 *
 *		���ó���
 *
 ***************************************************/

#ifndef MY_CAMERA_DRIVER_H
#define MY_CAMERA_DRIVER_H

#include <stm32f10x_gpio.h>

/***************************************************
 *                      ����                       *
 ***************************************************/
#define DBG_USART				USART1

#define USART_GPIO				GPIOA
#define USART_TXD_PIN			GPIO_Pin_9
#define USART_RXD_PIN			GPIO_Pin_10


/***************************************************
 *                    ��������С                   *
 ***************************************************/
#define MAX_DATA_BUFFER 30000


/***************************************************
 *                     OV����ͷ                    *
 ***************************************************/
/* ov����ͷ����ʱ���ڲ���Ƶ��
 *	0~0x3f
 *	Խ��Խ��
 */
#define CAMERA_CLK_DIV			0x02

/*
 * ����ʱ�ӷ�Ƶ
 *	1~7f
 *	Խ��Խ��
 */
#define CAMERA_PCLK_DIV			0x3f

/* �����ź��ߣ�SCCB_CLK, SCCB_DATA */
#define SCCB_GPIO				GPIOC

#define SCCB_C_PIN				GPIO_Pin_9
#define SCCB_D_PIN				0x0100/*GPIO_Pin_8*/
/* ����SCCB_D_PIN�����������ã�����ͻ���
 * token is not a valid binary operator in a preprocessor subexpression
 * �Ĵ���
 */

/* 8λ���������� */
#define CAMERA_DATA_GPIO		GPIOB
#define CAMERA_DATA_PINS	\
			GPIO_Pin_8 	|	\
			GPIO_Pin_9 	|	\
			GPIO_Pin_10	|	\
			GPIO_Pin_11	|	\
			GPIO_Pin_12	|	\
			GPIO_Pin_13	|	\
			GPIO_Pin_14	|	\
			GPIO_Pin_15

/***************************************************
 *                       ����                      *
 ***************************************************/
/* LEDָʾ�� */
#define LED_GPIO				GPIOD
#define LED_PIN					GPIO_Pin_2

/* �Ƿ���DMA�����ݣ������ԣ� */
#define USE_DMA
/*
 * DMA����ԭ��
 *	ע�⵽A6����һ�����ù����ǣ�TIM3_CH1��
 *	����TIM3ͨ��1Ϊ���룻����DMA1ͨ��6��ΪTIM3_CH1�Ĵ���
 */

#endif
