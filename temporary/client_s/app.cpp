// uCOS_Demo.cpp : 定义控制台应用程序的入口点。 
#include "stdafx.h" 
#include "SystemConfig.h" 
#include "BSP.h"
#include "format.h"

#define STD_1K_STKSIZE 512

void recvTask(void *p_arg);
void sendTask(void *p_arg);

/* 定义堆栈*/ 

OS_STK recvTask_Stk[STD_1K_STKSIZE];
OS_STK sendTask_Stk[STD_1K_STKSIZE / 4];
 
int _tmain(int argc, _TCHAR* argv[]) 
{ 
	OSInit();  /* 系统初始化*/ 
	OSStatInit();  /* 统计任务初始化*/ 
 
	OSTaskCreate(recvTask, (void *)0, &recvTask_Stk[STD_1K_STKSIZE - 1], 2);
	OSTaskCreate(sendTask, (void *)0, &sendTask_Stk[STD_1K_STKSIZE - 1], 3);
 
	OSStart();  /* 开始任务调度*/ 
 
	return 0; 
} 
 
void recvTask(void *p_arg) 
{ 
	com_pack recv_data;

	while (1) {
		BSPRecv(&recv_data);
		
		if (recv_data.head.ID != TERMINAL_ID)
			continue;

		switch (recv_data.head.code) {
		case CTRL_TYPE_LED_OFF:
			break;
		case CTRL_TYPE_LED_ON:
			break;
		case CTRL_TYPE_LED_OFF_ALL:
			break;
		case CTRL_TYPE_LED_ON_ALL:
			break;
		case CTRL_TYPE_GET_LED_STATUS:
			//TODO

			recv_data.head.type = PACK_TYPE_ACT;
			recv_data.head.code = ACK_TYPE_LEDS;
			BSPSend(&recv_data, sizeof(recv_data));
			break;
		default:
			break;
		}

		OSTimeDlyHMSM(0,0,1,0);  /* 任务调度*/ 
	} 
	p_arg = p_arg;
}

void sendTask(void *p_arg)
{
	INT32S len = 0;
	INT16U counter = 0;
	com_pack pack;

	pack.head.ID = TERMINAL_ID;
	pack.head.type = PACK_TYPE_DATA;

	while (1) {
		pack.packdata.data_p = BSPCameraShut(&len);

		//split the data to many packages
		counter = 0;
		while (len > MAX_PACKAGE_SIZE - sizeof(com_pack_head)) {
			pack.head.size = MAX_PACKAGE_SIZE;
			pack.head.code = counter++;
			
			BSPSend(&pack, 8);
			BSPSend(pack.packdata.data_p, MAX_PACKAGE_SIZE - 8);

			pack.packdata.data_p += MAX_PACKAGE_SIZE - 8;
			len -= MAX_PACKAGE_SIZE - 8;
		}

		//send the last package
		pack.head.size = len + 8;
		pack.head.code = PACK_END_PACK;
		BSPSend(&pack, 8);
		BSPSend(pack.packdata.data_p, len);

		OSTimeDlyHMSM(0, 0, 1, 0);
	}
	p_arg = p_arg;
}