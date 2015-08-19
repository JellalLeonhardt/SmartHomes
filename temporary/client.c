/*
 * client.c
 *
 *	模拟一个硬件终端：
 *		1号终端
 *		大约一秒发送一张JPEG格式的图片
 *		模拟了几个LED灯，对服务器发过来的指令有反应
 *
 */

#include "client_interface.h"
#include "BSD.h"
#include "camera.h"

#include <pthread.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>

/*
 * 全局变量
 */
pthread_t receive_fd;
pthread_t camera_fd;

/* 
 * 函数声明
 */
/* 一个线程，每隔一秒拍一张照片并发送 */
void thread_CameraShutAndSend(void);


/*
 * main
 *  
 *  使用多线程
 *		“接收数据”和“发送数据”为单独的线程
 *
 */
int main(int argc, char *argv[]){
	int port;
	int err;


	if(argc != 2){
		fprintf(stderr, "unknow port\n\tusage: client xxxx\n\n");
		return -1;
	}

	sscanf(argv[1], "%d", &port);
	err = BSDLowlevelInit(port);
	if(err < 0){
		fprintf(stderr, "Failed to initialize socket...\n\n");
		return -2;
	} else {
		printf("Socket initialized...\n\n");
	}

	if(0 != pthread_create(&receive_fd, NULL, (void*)BSDReceive, NULL))
	{
		fprintf(stderr, "Failed to create thread for 'receive'...\n\n");
		return -3;
	} else {
		printf("Create thread for 'receive'...\n\n");
	}

	if(0 != pthread_create(&camera_fd, NULL, (void*)thread_CameraShutAndSend, NULL))
	{
		fprintf(stderr, "Failed to create thread for 'camera'...\n\n");
		return -3;
	} else {
		printf("Create thread for 'camera'...\n\n");
	}


	pthread_join(receive_fd, NULL);
	/* Will not return... */

	return 0;

}


/*
 * thread_CameraShutAndSend
 *
 *	隔一秒获取一张照片，并发送
 *	使用多线程
 */
void thread_CameraShutAndSend(void)
{
	while(1)
	{
		CameraShut();

		/* 因为电脑摄像头申请到缓冲区非常大，
		 * 不把camera_buffer_size那么大的数据全部发完，
		 * 只发大约210k，其实也足够了 */
		SendDataPackage(camera_buffer, 210000);

		sleep(1);
	}
}

