/****************************************************************
 * client_interface.c
 *
 *	发送、接收、处理“包”信息的函数
 ****************************************************************/

#include "BSD.h"
#include "format.h"

#include <stdio.h>

/****************************************************************
 * SendDataPackage
 *
 *	发送数据包
 *
 * 参数：
 *  data指向数据的指针
 *	size为裸数据的大小！从1开始计数
 *
 ****************************************************************/
int SendDataPackage(void* data, int size)
{
	com_pack package;
	int		package_number = 0;

	/* 看看要不要分多包发送 */
	while(size > 0)
	{
		/* 填写包信息 */
		package.head.ID =		TERMINAL_ID;
		package.head.type =		PACK_TYPE_DATA;
		if(size > (MAX_PACKAGE_SIZE - 8)){
			/* 此时应该分多包发送 */
			package.head.size = MAX_PACKAGE_SIZE;
			/* 数据包的序号 */
			package.head.code = package_number;
		} else {
			/* 不用分多包发送了 */
			package.head.size = size + 8;
			/* 填入“结束包”代号 */
			package.head.code = PACK_END_PACK;
		}

		printf("%d\n", package.head.size);

		/* 先发包头 */
		BSDSend((void*)&package, 8);
		/* 然后是包身 */
		BSDSend(data, package.head.size - 8);

		size -= (package.head.size - 8);
		data += (package.head.size - 8);

		package_number ++;	/* 序号加1 */
	}
	printf("One picture...\n");

	return 0;
}

/****************************************************************
 * SendAckPackage
 *
 *	发送回应包
 *
 * 参数：
 *  ack_code:回应代号
 *  ack:回应内容
 *
 ****************************************************************/
int SendAckPackage(int ack_code, int ack)
{
	com_pack package;

	package.head.ID =		TERMINAL_ID;
	package.head.type =		PACK_TYPE_ACT;
	package.head.size =		12;	/* 包头8字节 + 内容4字节 */
	package.head.code =		ack_code;

	package.packdata.data32 = ack;

	BSDSend((char*)&package, 12);

	return 0;
}


/****************************************************************
 * SendReqPackage
 *
 *	发送请求包
 *
 * 参数：
 *  req_code:回应代号
 *  req:回应内容
 *
 ****************************************************************/
int SendReqPackage(int req_code, int req)
{
	com_pack package;

	package.head.ID =		TERMINAL_ID;
	package.head.type =		PACK_TYPE_ACT;
	package.head.size =		12;	/* 包头8字节 + 内容4字节 */
	package.head.code =		req_code;

	package.packdata.data32 = req;

	BSDSend((char*)&package, 12);

	return 0;
}

/****************************************************************
 * ProcessPackage
 *
 *	处理信息（终端）
 *
 * 参数：
 *	pack：指向某数据流的指针————一段连续的内存空间，将按包的格式处理
 *
 ***************************************************************/
int ProcessPackage(com_pack *pack)
{
	if(pack->head.ID != TERMINAL_ID){
		printf("not this ID:%d\n\n", pack->head.ID);
		return ERR_COM_NOTHING;
	}

	printf("ID: %d  size: %d  type: %d  code: %d\n",
				pack->head.ID,
				pack->head.size,
				pack->head.type,
				pack->head.code
		  );

	switch(pack->head.type)
	{
		case PACK_TYPE_CTRL:
			printf("processing: control type\n\n");
			BSDProcessCtrlPackage(pack->head.code, pack->packdata.data32);
			return ERR_COM_NOTHING;

		default:
			printf("no such type:%d\n\n", pack->head.type);
			return ERR_COM_NO_SUCH_COMMAND;
	}
}



