/*
 * com_interface.h
 *
 *	与硬件无关的通讯接口
 *
 *		处理“包”信息
 */

#ifndef _MY_COM_INTERFACE_H_
#define _MY_COM_INTERFACE_H_

#include "format.h"

/*
 * functions
 */

/* 发送“回应包” */
int SendAckPackage(int ack_code, int ack);

/* 打包并发送“数据包” */
int SendDataPackage(void* data, int size);

/* 发送“请求包” */
int SendReqPackage(int req_code, int req);

/* 处理包信息(终端) */
int ProcessPackage(com_pack* pack);

#endif

