#ifndef _MY_CAMERA_GLOBALS_H__
#define _MY_CAMERA_GLOBALS_H__

#include "camera.h"

#ifdef GLOBALS
#define 	EXT 
#else
#define		EXT		extern
#endif

/* ��¼����ͷ������Ϣ��������ָ�롢��С����ֱ�Ӵ������ */
EXT 	st_cam_buf_info 	camera_buffer_info;

/* JPEG���ݻ����� */
#define MAX_BUFFER_SIZE		(32 * 1024)
EXT 	char 				JPEGBuff[MAX_BUFFER_SIZE];

/* 
 * ���еĺ궨��
 */
	/* ���忪ͷ */
#define BUFFER_START		(&JPEGBuff[0])


#endif
