/****************************************************************** 
 * com_format.h
 *
 *	自定义通讯格式：
 *
 *	1、小端模式！！！
 *
 *	2、以数据包为单位
 *
 *	3、一个包分包头/包身
 *		包头：8字节；
 *			1	ID号（0为服务器，其他为终端）
 *			2	包类型
 *			3~4 包长度：short int，包括包头8字节
 *			5~8 xx代号（如，控制代号）：32位int
 *		
 *			包类型：
 *				控制类：0xfe(-2)，仅服务器到终端，包序号暂时置0
 *				数据类：0xfc(-3)，仅终端到服务器，多包发送时填好包序号
 *				回应包：0xfb(-4)，终端回应某些控制类指令，包序号暂时置0
 *				请求包：0xfa(-5)，终端请求某些信息，包序号暂时置0
 *
 *	4、各种包的结构
 *		基本结构：
 *			包头 + 内容
 *
 *		除“数据包”外，其他包的“xx内容”暂限定4个字节（int）
 *
 *	5、因为硬件模块的限制，一个包最大2048字节！！！
 *
 ******************************************************************/

#ifndef _MY_COM_FMT_H_
#define _MY_COM_FMT_H_


/****************************************************************** 
 *                                                                *
 *                           数据结构                             *
 *                                                                *
 ******************************************************************/

/**************************************************
 *                    包头                        *
 **************************************************/
typedef struct tag_mypackhead
{
	signed char      ID;
	signed char      type;
	signed short     size;
	signed short     code;
	unsigned short   checksum;
}com_pack_head;

/**************************************************
 *                     包                         *
 **************************************************/
typedef struct tag_mypack{
	com_pack_head			head;
	union tag_packdata
	{
		signed int		data32;
		signed char		data8[4];
	}
							packdata;

}com_pack;



/****************************************************************** 
 *                                                                *
 *                          定义各种代号                          *
 *                                                                *
 ******************************************************************/

/* 最大字节数 */
#define MAX_PACKAGE_SIZE			2048
/* 最小字节数 */
#define LEAST_PACKAGE_SIZE			12


/* 服务器的ID */
#define ID_SERVER					0


/**************************************************
 *                   包类型                       *
 **************************************************/
#define PACK_TYPE_CTRL				-2
#define PACK_TYPE_DATA				-3
#define PACK_TYPE_ACT				-4
#define PACK_TYPE_REQ				-5


/**************************************************
 *                   控制包                       *
 **************************************************/

                /* “控制代号” */
/* LED control */
	/* 下面2个的“控制内容”为，LED灯的序号 */
#define 	CTRL_TYPE_LED_OFF					0
#define 	CTRL_TYPE_LED_ON					1
	/* 下面3个的“控制内容”，  置0 */
#define 	CTRL_TYPE_LED_OFF_ALL				2
#define 	CTRL_TYPE_LED_ON_ALL				3
#define 	CTRL_TYPE_GET_LED_STATUS			4

/* CAMERA control */
	/* 改变分辨率 */
#define		CTRL_TYPE_CAMERA_CHANGE_SIZE		5
		/* 直接引用ov2640.h的 JPEG_***x*** 的定义 */
#define		CAMERA_SIZE_176X144					0
#define		CAMERA_SIZE_320X240					1
#define		CAMERA_SIZE_352X288					2
#define		CAMERA_SIZE_640X480					3
#define		CAMERA_SIZE_800X600					4
#define		CAMERA_SIZE_1024X768				5

	/* 白平衡 */
#define 	CTRL_TYPE_CAMERA_WHITE_BALANCE		6
		/* 自动白平衡 */
#define		CAMERA_WHITE_BALANCE_AUTO			0x00000000
		/* 阳光 */
#define		CAMERA_WHITE_BALANCE_SUNNY			0x0054415e
		/* 阴天 */
#define		CAMERA_WHITE_BALANCE_CLOUDY			0x004f4165
		/* 办公室 */
#define		CAMERA_WHITE_BALANCE_OFFICE			0x00664152
		/* 家庭 */
#define		CAMERA_WHITE_BALANCE_HOME			0x00713f42

	/* 特殊效果 */
#define		CTRL_TYPE_CAMERA_EFFECTS			7
		/* 普通 */
#define		CAMERA_EFFECTS_NORMAL				0x00808000
		/* 黑白 */
#define		CAMERA_EFFECTS_BLACK_WHITE			0x00808018
		/* 偏蓝 */
#define		CAMERA_EFFECTS_BLUE					0x0040a018
		/* 偏绿 */
#define		CAMERA_EFFECTS_GREEN				0x00404018
		/* 偏红 */
#define		CAMERA_EFFECTS_RED					0x00c04018
		/* 复古 */
#define		CAMERA_EFFECTS_ANTUQUE				0x00a64018
		/* 反色 */
#define		CAMERA_EFFECTS_NEGATUVE				0x00808040
		/* 黑白+反色 */
#define		CAMERA_COLOR_TYPE_B_W_NEGATIVE		0x00808058

	/* 曝光等级 */
#define		CTRL_TYPE_CAMERA_EXPLOSURE			8
#define		CAMERA_EXPLOSURE_0					0x00601820
#define		CAMERA_EXPLOSURE_1					0x00701c34
#define		CAMERA_EXPLOSURE_2					0x0081383e
#define		CAMERA_EXPLOSURE_3					0x00814048
#define		CAMERA_EXPLOSURE_4					0x00925058

	/* 色彩饱和度 */
#define		CTRL_TYPE_CAMERA_SATURATION			9
#define		CAMERA_SATURATION_HIGHEST			0x00006868
#define		CAMERA_SATURATION_HIGH				0x00005858
#define		CAMERA_SATURATION_MEDIUM			0x00004848
#define		CAMERA_SATURATION_LOW				0x00003838
#define		CAMERA_SATURATION_LOWEST			0x00002828

	/* 图片亮度 */
#define		CTRL_TYPE_CAMERA_LIGHTNESS			10
#define		CAMERA_LIGHTNESS_HIGHEST			0x00000040
#define		CAMERA_LIGHTNESS_HIGH				0x00000030
#define		CAMERA_LIGHTNESS_MEDIUM				0x00000020
#define		CAMERA_LIGHTNESS_LOW				0x00000010
#define		CAMERA_LIGHTNESS_LOWEST				0x00000000

	/* 对比度 */
#define		CTRL_TYPE_CAMERA_CONTRAST			11
#define		CAMERA_CONTRAST_HIGNEST				0x00000c28
#define		CAMERA_CONTRAST_HIGH				0x00001624
#define		CAMERA_CONTRAST_MEDIUM				0x00002020
#define		CAMERA_CONTRAST_LOW					0x00002a1c
#define		CAMERA_CONTRAST_LOWEST				0x00003418


/**************************************************
 *                   回应包                       *
 **************************************************/

                /* “回应代号” */
/* LED control */
	/* 下面1个的“回应内容”为，各个LED的开关情况 */
#define ACK_TYPE_LEDS				5


/**************************************************
 *                   数据包                       *
 **************************************************/

/* 多包发送的“结束包”标志 */
#define PACK_END_PACK				-1


/**************************************************
 *                   请求包                       *
 **************************************************/
/*
 * 请求包的内容待定。。。
 * */



/******************************************************************
 *                                                                *
 *                            错误码                              *
 *                                                                *
 ******************************************************************/

#define ERR_COM_NOTHING				0
#define	ERR_COM_NO_SUCH_COMMAND		-1
#define ERR_COM_BROKEN_PACKAGE		-2


#endif


