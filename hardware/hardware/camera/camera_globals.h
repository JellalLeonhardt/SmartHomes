#ifndef _MY_CAMERA_GLOBALS_H__
#define _MY_CAMERA_GLOBALS_H__

#include "camera.h"

#ifdef GLOBALS
#define 	EXT 
#else
#define		EXT		extern
#endif

/* 记录摄像头数据信息，缓冲区指针、大小，可直接从这里查 */
EXT 	st_cam_buf_info 	camera_buffer_info;

/* JPEG数据缓冲区 */
#define MAX_BUFFER_SIZE		(32 * 1024)
EXT 	char 				JPEGBuff[MAX_BUFFER_SIZE];

/* 
 * 公有的宏定义
 */
	/* 缓冲开头 */
#define BUFFER_START		(&JPEGBuff[0])


#endif
