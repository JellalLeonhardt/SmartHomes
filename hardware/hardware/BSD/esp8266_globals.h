/*
 * 声明全局变量
 *	
 *	其实可以用UCOS的“信号量”的，但简单的信号量不如用全局变量来代替
 *
 *	在真正定义全局变量的地方，要先define GLOBALS，再include这个头文件！
 */
 
#ifndef _MY_GLOBALS_H__
#define _MY_GLOBALS_H_

#include "ESP8266.h"

#ifdef GLOBALS
#define EXT_E
#else
#define EXT_E extern
#endif


	/* 发送、接收的全局标志 */
EXT_E en_ReturnSign sign_send, sign_receive;


#endif
