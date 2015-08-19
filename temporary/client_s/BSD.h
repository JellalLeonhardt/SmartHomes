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
int		BSDLowlevelInit(int);
void	BSDSend(void*, int);
void	BSDReceive(void);

int		BSDProcessCtrlPackage(int ctrl, int data);

#endif

