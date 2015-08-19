/*
 * camera.h
 *	
 *	摄像头驱动
 */

#ifndef _MY_CAMERA_ON_LABTOP_H_
#define _MY_CAMERA_ON_LABTOP_H_

#include "cam_global_var.h"

/*
 * 初始化摄像头
 *
 *	返回：小于0，失败；0,成功
 */
int CameraInit(void);

/*
 * 读取摄像头数据
 *	数据放到 *camera_buffer 所指处
 */
void CameraShut(void);

#endif

