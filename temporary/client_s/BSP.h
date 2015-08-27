/*
 * BSP.h
 * 与硬件相关部分
 *
 *	发送、接收数据的函数
 *
 *	电脑上用socket
 */

#ifndef _MY_BSP_H_
#define _MY_BSP_H_

/* 1号终端 */
#define TERMINAL_ID		1


/*
 * 函数
 */
int		BSPLowlevelInit(int);
/*void	BSDSend(void*, int);
void	BSDReceive(void);*/
int BSPRecv(void *buffer);
void BSPSend(void *buffer，int size）;
int BSPCameraShut(void *buffer);

int		BSPProcessCtrlPackage(int ctrl, int data);

#endif

