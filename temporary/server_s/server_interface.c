/*
 * server_interface.c
 *
 *  初始化socket、发送接收数据
 *
 */

#include "server_interface.h"
#include "format.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>


/*
 * 私有全局变量
 */
int server_fd = 0;
int connect_fd = 0;

#define MAX_BUFFER_SIZE 220000
char receive_buff[MAX_BUFFER_SIZE] = { 0 };
#define RECEIVE_BUFFER_END &receive_buff[MAX_BUFFER_SIZE - 1]

/*
 * 私有函数
 */
void s_ProcessBuffer(void *buffer);
void s_ProcessPackage(com_pack *buffer);
void s_ProcessData(void *pack);


/*
 * ServerInit
 *
 *	socket初始化
 *
 *	输入：要监听的端口号
 *	返回：0为成功
 */
int ServerInit(int port)
{
	int err;
	struct sockaddr_in server_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd < 0){
		fprintf(stderr, "Socket failed: %d\n\n", server_fd);
		return -1;
	} else {
		printf("Socket OK: %d\n\n", server_fd);
	}


	server_addr.sin_family =		AF_INET;
	server_addr.sin_addr.s_addr =	htonl(INADDR_ANY);
	server_addr.sin_port =			htons(port);

	err = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(err < 0){
		fprintf(stderr, "Bind failed: %d\n\n", err);
		return -2;
	} else {
		printf("Bind port %d ok...\n\n", port);
	}


	err = listen(server_fd, 200);
	if(err < 0){
		fprintf(stderr, "Listen error: %d\n\n", err);
		return -3;
	} else {
		printf("%d  listening...\n\n", err);
	}

	return 0;
}


/*
 * ServerMainLoop
 *
 *	循环处理收发数据
 *
 *		不会返回了。。。
 */
void ServerMainLoop(void)
{
	int readnum = 0;
	int count = 0;
	char *rcve_p = receive_buff,
		 *prcs_p = receive_buff;

	while(1)
	{
		int remain_buff;


		connect_fd = accept(server_fd, NULL, NULL);
		if(connect_fd < 0){
			fprintf(stderr, "Accept failed: %d\n\n", connect_fd);
			exit(-1);
		} else {
			printf("Accept OK: %d\n\n", connect_fd);
		}

		remain_buff = MAX_BUFFER_SIZE - count;
		do
		{
			int curr_pack_size = 0;

			/* 循环处理信息包 */
			readnum = recv(
						connect_fd, 
						rcve_p,
						remain_buff,
						0
						  );

			count += readnum;
			rcve_p += readnum;
			remain_buff = MAX_BUFFER_SIZE - count;

			printf("Received %d bytes... %d\n", readnum, count);

			if(count >= LEAST_PACKAGE_SIZE)
			{
				curr_pack_size = ((com_pack*)prcs_p)->head.size;

				printf("\tProcess it: pack size:%d\n", curr_pack_size);
    
				while(count >= curr_pack_size)
				{
					s_ProcessBuffer(prcs_p);
    
					/* 通讯以包为单位，
					 * 所以每次prcs_p都会指向另一个包
					 * */
					prcs_p += curr_pack_size;

					count -= curr_pack_size;
					if(count < LEAST_PACKAGE_SIZE)
					  break;

					curr_pack_size = ((com_pack*)prcs_p)->head.size;
				}
			}

			/* 每次都要把剩余的字节搬回原点 */
			memcpy(receive_buff, prcs_p, count);
			prcs_p = receive_buff;
			rcve_p = receive_buff + count;
		}while(readnum > 0);
	}
}


/*
 * 私有函数
 */

/*
 * s_ProcessBuffer
 *
 *	处理缓冲区的数据（缓冲区内可能不止一个包）
 */
void s_ProcessBuffer(void* buffer)
{
	com_pack *temp = (com_pack*)buffer;

	printf("\tsize: %d\n", temp->head.size);

	s_ProcessPackage(temp);
}

/*
 * s_ProcessPackage
 *
 *	分类处理数据包
 */
void s_ProcessPackage(com_pack *buffer)
{
	switch(buffer->head.type)
	{
		case PACK_TYPE_DATA:
			s_ProcessData(buffer);
			break;
	}
}


/*
 * s_ProcessData
 *
 *	处理收到的数据：当作图像数据
 */
void s_ProcessData(void *pack)
{
	DisplayWriteData(pack + 8, ((com_pack*)pack)->head.size - 8);

	/* 判断是否结束包 */
	if(((com_pack*)pack)->head.code == PACK_END_PACK)
	{
		printf("\t\tDisplay\n");
		/* 发完一张图片，然后再显示 */
		DisplayDraw();
	}
}

