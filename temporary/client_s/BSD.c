/*
 * BSD.c
 *
 *	与平台相关的部分
 *
 *		1、发送、接收数据的函数
 *		2、摄像头驱动
 *
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <linux/videodev2.h>

#include "client_interface.h"
#include "camera.h"


/*
 *  私有全局变量
 */
static int client_fd;
char	receive_data[128];
int		LED_status = 0;	/* 模拟led灯 */


/*
 * 私有函数
 */
static int s_BSDSocketInit(int port);
static int s_BSDCameraInit(void);


/*
 * s_BSDSocketInit
 *
 *	初始化socket
 *
 *	输入：服务器端口
 *	返回：小于0，失败；0，成功
 */
static int s_BSDSocketInit(int port)
{
	struct sockaddr_in client_addr;
	int err;


	printf("Initializing socket...\n\n");

	client_addr.sin_port = htons(port);
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd < 0){
		fprintf(stderr, "Socket failed: %d\n\n", client_fd);
		return -2;
	} else {
		printf("Socket OK: %d\n", client_fd);
	}

	err = connect(client_fd, (struct sockaddr*)&client_addr, sizeof(client_addr));
	if(err < 0){
		fprintf(stderr, "Connect failed: %d\n\n", err);
		return -3;
	} else {
		printf("Connect OK: %d\n\n", err);
	}

	return 0;
}


/*
 * s_BSDCameraInit
 *
 *	初始化摄像头
 *
 *	输入：服务器端口
 *	返回：小于0，失败；0，成功
 */
static int s_BSDCameraInit(void)
{
	return CameraInit();
}

/*
 * BSDLowlevelInit
 *
 *	初始化相关项目
 *
 *	输入：服务器端口
 *	返回：小于0，失败；0，成功
 *
 */
int BSDLowlevelInit(int port)
{
	int err;
	
	err = s_BSDSocketInit(port);
	if(err < 0)
	  return err;

	err = s_BSDCameraInit();
	if(err < 0)
	  return err;

	return err;
}


/*
 * BSDSend
 *
 *	发送数据
 *
 *	输入：
 *		data：指向数据流的指针
 *		size：发送数据大小
 *
 */
void BSDSend(void* data, int size)
{
	send(client_fd, data, size, 0);
	//usleep(500);
}


/*
 * BSDReceive
 *
 *	接收数据
 *		将数据循环接收到receive_data[]缓冲区中，
 *		并从中分离出包
 *		（对于终端来说，接收到的包类型只有“控制”一种。。。）
 *
 *	应该单独开一个线程专门接收数据。。。
 */
void BSDReceive(void)
{
	static void *receive_buff = (void*)receive_data;
	int rec_num;

	while(1)
	{
		rec_num = recv(client_fd, receive_buff, 128, 0);
		if(rec_num > 0){
			int n;

			printf("received:\n\t");
			for(n = 0; n < rec_num; n++)
			{
				printf("%4x", receive_data[n] & 0xff);
			}
			printf("\n");
			ProcessPackage((com_pack*)receive_buff);
		} else {
			printf("nothing received...\n");
		}

		sleep(1);
	}
}

/*
 * BSDProcessCtrlPackage
 *
 * 处理接收到的控制包
 *
 * 输入：
 *	ctrl：控制代号（LED）
 *	data：控制内容（LED灯号）
 *
 */
void BSDProcessCtrlPackage(int ctrl, int data)
{
	printf("BSD processing: %d %d\n", ctrl, data);


	switch(ctrl)
	{
		case CTRL_TYPE_GET_LED_STATUS:
			SendAckPackage(ACK_TYPE_LEDS, LED_status);
			break;
			
		case CTRL_TYPE_LED_OFF:
			LED_status &= ~(1L << data);
			break;

		case CTRL_TYPE_LED_ON:
			LED_status |=  (1L << data);
			break;

		case CTRL_TYPE_LED_OFF_ALL:
			LED_status = 0;
			break;

		case CTRL_TYPE_LED_ON_ALL:
			LED_status = -1;
			break;
	}
}


