/***************************************************
 *	cam_cfg.h
 *
 *		配置常数
 *
 ***************************************************/

#ifndef MY_CAMERA_DRIVER_H
#define MY_CAMERA_DRIVER_H

#include <stm32f10x_gpio.h>

/***************************************************
 *                      串口                       *
 ***************************************************/
#define DBG_USART				USART1

#define USART_GPIO				GPIOA
#define USART_TXD_PIN			GPIO_Pin_9
#define USART_RXD_PIN			GPIO_Pin_10


/***************************************************
 *                    缓冲区大小                   *
 ***************************************************/
#define MAX_DATA_BUFFER 30000


/***************************************************
 *                     OV摄像头                    *
 ***************************************************/
/* ov摄像头的主时钟内部分频数
 *	0~0x3f
 *	越大越慢
 */
#define CAMERA_CLK_DIV			0x02

/*
 * 像素时钟分频
 *	1~7f
 *	越大越慢
 */
#define CAMERA_PCLK_DIV			0x3f

/* 两根信号线：SCCB_CLK, SCCB_DATA */
#define SCCB_GPIO				GPIOC

#define SCCB_C_PIN				GPIO_Pin_9
#define SCCB_D_PIN				0x0100/*GPIO_Pin_8*/
/* 上面SCCB_D_PIN必须这样设置，否则就会有
 * token is not a valid binary operator in a preprocessor subexpression
 * 的错误
 */

/* 8位并行数据线 */
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
 *                       其他                      *
 ***************************************************/
/* LED指示灯 */
#define LED_GPIO				GPIOD
#define LED_PIN					GPIO_Pin_2

/* 是否用DMA传数据（试验性） */
#define USE_DMA
/*
 * DMA传输原理：
 *	注意到A6引脚一个复用功能是，TIM3_CH1，
 *	配置TIM3通道1为输入；配置DMA1通道6，为TIM3_CH1的触发
 */

#endif
