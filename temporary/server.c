/*
 * server.c
 *
 *	服务器的TCP端，作为参考
 */

#include "server_interface.h"
#include "display.h"

#include <stdio.h>
#include <gtk/gtk.h>
#include <pthread.h>

pthread_t server_id;

int main(int argc, char *argv[])
{
	int port;
	
	if(argc != 2)
	{
		fprintf(stderr, "Unknow port.\n\tUsage: server xxxx\n\n");
		return -1;
	}

	gtk_init(&argc, &argv);

	DisplayLowLevelInit();

	sscanf(argv[1], "%d", &port);
	ServerInit(port);

	if(0 == pthread_create(&server_id, NULL, (void*)ServerMainLoop, NULL))
	  printf("Create thread for server..\n");

	gtk_main();
	/* Will not return */

	return 0;
}

