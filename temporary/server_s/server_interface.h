/*
 * server_interface.h
 *
 *  初始化socket、发送接收数据
 *
 */

#ifndef _MY_SERVER_INTERFACE_H_
#define _MY_SERVER_INTERFACE_H_

#include "display.h"

int ServerInit(int port);
void ServerMainLoop(void);

#endif

