/*
 * BSD.h
 * 与硬件相关部分
 *
 *	发送、接收数据的函数
 *
 *	电脑上用socket
 */

#ifndef _MY_BSD_H_
#define _MY_BSD_H_

/* 1号终端 */
#define TERMINAL_ID		1


/*
 * 函数
 */
int		BSDLowlevelInit(void);
void	BSDSend(void*, int);
void	BSDReceive(void);

void*	BSDCameraShut(int* size);

void	BSDProcessCtrlPackage(int ctrl, int data);

#endif

