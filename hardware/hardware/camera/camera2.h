/*
 * camera.h
 *	摄像头驱动的函数接口
 *
 */

#ifndef MY_CAMERA_H_
#define MY_CAMERA_H_

#include "cam_cfg.h"


/*
 * 数据结构
 */
	/* 摄像头缓冲区信息 */
typedef struct tag_camera_buffer
{
	/*
	 *	 原理：
	 *		DMA每次循环把数据搬到一个很大的缓冲区里面，
	 *		每搬完一张图片，都会记录以下信息
	 */
	
	/* 记录可供输出的图片数据指针 */
	void 	*p_picture;

	/* 大小 */
	/* 因为DMA是循环搬运的，有时图像数据
	 * 一半在尾，另一半在头；此值留供判断 
	 */
	int		size_head;
	int		size_end;
	
} st_cam_buf_info;


/*
 * 函数接口 
 */

/*
 * 底层初始化
 *	初始化各个GPIO
 */
void CameraLowLevelInit(void);


/*
 * 初始化摄像头
 *	将摄像头设置成JPEG格式的输出
 *
 *	返回：0为成功；非0，返回出错的指令号
 *
 *	摄像头随即开始工作；但单片机未必开始读数
 */
int CameraInit(void);


/*
 * 开始响应摄像头的数据
 *	初始化、并使能中断，从而响应摄像头的数据
 */
void CameraStart(void);


/*
 * 宏定义
 */
/* LED */
#define 	LED_ON				LED_GPIO->ODR &= ~(LED_PIN);
#define		LED_OFF				LED_GPIO->ODR |= LED_PIN;

#endif
