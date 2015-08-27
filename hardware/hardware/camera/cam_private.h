/*
 *	cam_private.h
 *		摄像头驱动专用
 */

#ifndef MY_CAMERA_PRIVATE_H
#define MY_CAMERA_PRIVATE_H

#include "cam_cfg.h"

/* 为方便模拟SCCB，作此宏定义 */
#define 	SIO_D_SET 			SCCB_GPIO->ODR |= SCCB_D_PIN;
#define 	SIO_D_CLR 			SCCB_GPIO->ODR &= ~SCCB_D_PIN;
#define 	SIO_C_SET 			SCCB_GPIO->ODR |= SCCB_C_PIN;
#define 	SIO_C_CLR 			SCCB_GPIO->ODR &= ~SCCB_C_PIN;

#define 	SIO_D_STATE 		(SCCB_GPIO->IDR & SCCB_D_PIN)


/* 快速转换SCCB_D端口的状态：输入<==>输出 */
#if (SCCB_D_PIN & 0xff != 0)
#define 	SCCB_D_R			(SCCB_GPIO->CRL)
#else
#define 	SCCB_D_R			(SCCB_GPIO->CRH)
#endif

#if (SCCB_D_PIN == 0x0001) || (SCCB_D_PIN == 0x0100)
#define		SCCB_GPIO_R_MASK	0x0000000f
#elif (SCCB_D_PIN == 0x0002) || (SCCB_D_PIN == 0x0200)
#define		SCCB_GPIO_R_MASK	0x000000f0
#elif (SCCB_D_PIN == 0x0004) || (SCCB_D_PIN == 0x0400)
#define		SCCB_GPIO_R_MASK	0x00000f00
#elif (SCCB_D_PIN == 0x0008) || (SCCB_D_PIN == 0x0800)
#define		SCCB_GPIO_R_MASK	0x0000f000
#elif (SCCB_D_PIN == 0x0010) || (SCCB_D_PIN == 0x1000)
#define		SCCB_GPIO_R_MASK	0x000f0000
#elif (SCCB_D_PIN == 0x0020) || (SCCB_D_PIN == 0x2000)
#define		SCCB_GPIO_R_MASK	0x00f00000
#elif (SCCB_D_PIN == 0x0040) || (SCCB_D_PIN == 0x4000)
#define		SCCB_GPIO_R_MASK	0x0f000000
#elif (SCCB_D_PIN == 0x8002) || (SCCB_D_PIN == 0x8000)
#define		SCCB_GPIO_R_MASK	0xf0000000
#endif

/* GPIO_Mode_IN_FLOATING */
#define		SCCB_D_INPUT_MASK	(0x44444444 & SCCB_GPIO_R_MASK)
/* GPIO_Mode_OUT_OD */
#define		SCCB_D_OUTPUT_MASK	(0x77777777 & SCCB_GPIO_R_MASK)

#define		SCCB_D_INPUT		SCCB_D_R = (SCCB_D_R & ~SCCB_GPIO_R_MASK) | SCCB_D_INPUT_MASK;
#define		SCCB_D_OUTPUT		SCCB_D_R = (SCCB_D_R & ~SCCB_GPIO_R_MASK) | SCCB_D_OUTPUT_MASK;

#endif
