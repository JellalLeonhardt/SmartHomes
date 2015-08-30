/*
 * BSD.c
 *	与硬件平台相关的部分
 *
 *	通讯、摄像头驱动
 */
 
#include <stm32f10x.h>
#include <includes.h>

#include "LED_cfg.h"

#include "client_interface.h"

#include "camera.h"
#include "camera_globals.h"

#include "BSD.h"
#include "esp8266.h"
#define GLOBALS
#include "esp8266_globals.h"

/*
 * 私有宏定义
 */
	/* 串口1的发送DMA通道 */
#define		USART_TX_DMA	DMA1_Channel4
	/* 串口1的接收DMA通道 */
#define		USART_RX_DMA	DMA1_Channel5

	/* 开始接收数据：开上升沿中断、置标志位 */
#define		CAMERA_START \
	EXTI->IMR |= EXTI_Line7;\
	EXTI->RTSR |= EXTI_Line7;\
	camera_buffer_info.sign_getphoto = 0;
	/*
	 *	刚开始，开了上升沿中断、总开关；
	 *	在上升沿中断时关掉上升沿、开启下降沿
	 *	在下降沿中断时关掉下降沿、关闭总开关
	 */
	 
	 /* 阻塞的部分 */
#define CAMERA_WAIT \
	while(camera_buffer_info.sign_getphoto == 0)\
	{\
		OSTimeDly(70);\
	}


/* 
 * 私有变量
 */
	/* 接收缓冲区 */
#define 	MAX_BUFFER_SIZE_ 	128
	/* 接收缓冲区需要预留多4个字节，以备后文“循环读取”时读出界 */
static u8 receive_data[MAX_BUFFER_SIZE_ + 4];
#define		BUFFER_START_		(&receive_data[0])
#define 	BUFFER_END			(&receive_data[MAX_BUFFER_SIZE_ - 1])
#define		BUFFER_APPEND		(&receive_data[MAX_BUFFER_SIZE_])
/*
 * 一些字符串：AT指令集
 */
	/* 关闭回显 */
const char ATE0[] = "ATE0\r\n";
	/* 连接服务器，暂时固定IP地址和端口号 */
const char AT_CIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.1.108\",8080\r\n";
	/* 关闭连接 */
const char AT_CIPCLOSE[] = "AT+CIPCLOSE\r\n";


/*
 * 私有函数
 */
	/* 初始化串口以及对应的DMA */
static void s_USARTLowLevelInit(void);
	/* 初始化LED的GPIO */
static void s_GPIOLowLevelInit(void);
	/* 设置ESP8266：关回显 */
static void s_ESP8266LowLevelInit(void);


/*
 * 底层初始化
 *	先初始化摄像头，再初始化串口
 *
 *	因为ESP8266模块要花5秒初始化自己，串口输出调试信息（乱码）
 *	而初始化摄像头要半分钟，这样就跳过了串口开头的乱码
 *
 *	返回：
 *		0为成功，-1失败
 */
int BSDLowlevelInit(void)
{
	/* 初始化全局变量 */
	sign_receive = sign_send = SIGN_WAITING;
	
	/* 初始化摄像头 */
	CameraLowLevelInit();
	
	/* 初始化LED */
	s_GPIOLowLevelInit();
	
	if(CameraInit() != 0)
	{
		return -1;
	}
	
	
	/* 初始化串口以及对应的DMA */
	s_USARTLowLevelInit();
	
	/* 设置ESP8266：关回显 */
	s_ESP8266LowLevelInit();
	
	return 0;
}


/*
 * 发送一段数据
 *
 *	用DMA发送，设置好DMA以后直接返回，不作等待
 */
void BSDSend(void *data, int size)
{
	/* 等待上一轮发送结束 */
	while(USART_TX_DMA->CNDTR != 0);
	
	/* 设置内存地址、数据大小 */
	USART_TX_DMA->CCR &= ~0x00000001;
	USART_TX_DMA->CMAR = (u32)data;
	USART_TX_DMA->CNDTR = size;
	
	/* 开始发送 */
	USART_TX_DMA->CCR |= 0x00000001;
}


/*
 * 接收数据
 *
 *	要每隔一段时间调用它一次
 */
void BSDReceive(void)
{
	/* 从头部开始逐字节扫描 */
	static u8* p_read_head = BUFFER_START_;
	/* 直到此时收到信息的尾部 */
	int sign_get_head = 0;
	
	while(p_read_head != BUFFER_START_ + MAX_BUFFER_SIZE_ - USART_RX_DMA->CNDTR)
	{
		/* 将缓冲区头部几个字节复制到尾部去，
		   以便一次读取多个字节时，顺利“循环”读取缓冲区 */
		*(u32*)BUFFER_APPEND = *(u32*)BUFFER_START_;
		
		/* 任何回复都是以 \r\n 为开头的，先要找到这个头 */
		if(!sign_get_head)
		{
			/* 一次取两个字节，提高效率 */
			u16 sh_head = *(u16*)p_read_head;
			if(sh_head == SEGMENT_START)
			{
				/* 做好标志 */
				sign_get_head = 1;
				/* 指向下两个字节 */
				p_read_head += 2;
			}
			else
			{
				/* 指向下一个字节 */
				p_read_head ++;
			}
		}
		else
		{
			/* 已经找到头部了，现在找相应的回复码 */
			/* 一次取4字节，提高效率 */
			u32 i_segment = *(u32*)p_read_head;
			switch(i_segment)
			{
				case SEGMENT_OK:
					/* 返回了“OK”字样 */
					sign_send = SIGN_OK;

					sign_get_head = 0;
					p_read_head += 4;
					break;
				case SEGMENT_SEND:
					/* 返回了“SEND”字样，看看后面是否跟着“OK” */
					p_read_head += 5;
					/* “SEND OK\r\n”一共9个字节，防止读出界 */
					if(p_read_head > BUFFER_END)
						p_read_head -= MAX_BUFFER_SIZE_;
		
					i_segment = *(u32*)p_read_head;
					if(i_segment == SEGMENT_OK)
						sign_send = SIGN_SEND_OK;

					sign_get_head = 0;
					p_read_head += 4;
					break;
				case SEGMENT_GET_STH:
				{
					/* 返回了“+IPD”字样，收到服务器发过来的信息 */
					char ctrl_pack[12];
					int n;

					sign_get_head = 0;
					p_read_head += 8;
				
					for(n = 0; n < 12; n++)
					{
						if(p_read_head > BUFFER_END)
							p_read_head -= MAX_BUFFER_SIZE_;
						
						ctrl_pack[n] = *p_read_head;
						
						p_read_head ++;
					}
					
					ProcessPackage((com_pack*)&ctrl_pack);
					
					break;
				}
				default:
					/* 如果以上的都不符合，看看它是不是 \r\n，若是则可能是下一个字段 */
					if((i_segment & 0xffff) == SEGMENT_START)
						p_read_head += 2;
					else
						sign_get_head = 0;
					break;
			}
		}
		
		/* 需要循环扫描，所以如果出界了，要回头 */
		if(p_read_head > BUFFER_END)
			p_read_head -= MAX_BUFFER_SIZE_;
	}
}


/*
 * 拍一张照片
 *
 *	阻塞函数，直到拍完再返回
 */
void *BSDCameraShut(int* size)
{
	int n;
	char* tmp_p;

	CAMERA_START;
	/* 在这里阻塞 */
	CAMERA_WAIT;
	
	/* 寻找JPEG头（只在前100字节里找） */
	tmp_p = BUFFER_START;
	for(n = 0; n < 100; n++)
	{
		if(*(u16*)tmp_p == 0xd8ff)
			break;
		tmp_p++;
	}
	
	*size = camera_buffer_info.size - n;
	
	return (void*)tmp_p;
}


/*
 * 处理收到的“控制包”
 *
 *	回应一个“回应包”，或者开关GPIO
 *
 *	输入：
 *		ctrl：“控制代号”
 *		data：“控制内容”
 */
void BSDProcessCtrlPackage(int ctrl, int data)
{
	switch(ctrl)
	{
		case CTRL_TYPE_GET_LED_STATUS:
			break;
			
		case CTRL_TYPE_LED_OFF:
			switch(data)
			{
				case 1:
					LED1_OFF;
					break;
				case 2:
					LED2_OFF;
					break;
				case 3:
					LED3_OFF;
					break;
				case 4:
					LED4_OFF;
					break;
			}
			break;

		case CTRL_TYPE_LED_ON:
			switch(data)
			{
				case 1:
					LED1_ON;
					break;
				case 2:
					LED2_ON;
					break;
				case 3:
					LED3_ON;
					break;
				case 4:
					LED4_ON;
					break;
			}
			break;

		case CTRL_TYPE_LED_OFF_ALL:
			LED1_OFF;
			LED2_OFF;
			LED3_OFF;
			LED4_OFF;
			break;

		case CTRL_TYPE_LED_ON_ALL:
			LED1_ON;
			LED2_ON;
			LED3_ON;
			LED4_ON;
			break;
	}
}


/*
 *	私有函数
 */
/*
 * 初始化串口1
 *
 *	初始化GPIO，串口、对应的DMA通道
 */
static void s_USARTLowLevelInit(void)
{
	{
		/* GPIO */
		GPIO_InitTypeDef USARTGPIO;
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
		
		USARTGPIO.GPIO_Speed = GPIO_Speed_50MHz;
		
		USARTGPIO.GPIO_Pin=GPIO_Pin_9;				//TXD
		USARTGPIO.GPIO_Mode=GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &USARTGPIO);
		
		USARTGPIO.GPIO_Pin=GPIO_Pin_10;				//RXD
		USARTGPIO.GPIO_Mode=GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &USARTGPIO);
	}
	
	{
		/* 串口 */
		USART_InitTypeDef uart;
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		
		USART_StructInit(&uart);
		uart.USART_BaudRate = 115200;
		
		USART_Init(USART1, &uart);
		USART_Cmd(USART1, ENABLE);
	}
	
	{
		/* DMAs */
		
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
		
		/* 发送DMA */
		/* CCR寄存器清零 */
		USART_TX_DMA->CCR = 0;
		
		/* 具体配置 */
		USART_TX_DMA->CCR = 
			DMA_DIR_PeripheralDST |
			DMA_M2M_Disable |
			DMA_Mode_Normal |
			DMA_Priority_Medium |
		
			DMA_MemoryDataSize_Byte |
			DMA_MemoryInc_Enable |
		
			DMA_PeripheralDataSize_Byte |
			DMA_PeripheralInc_Disable;
		
		/* 外设地址 */
		USART_TX_DMA->CPAR = (u32)&USART1->DR;
		
		/* 先不配置内存地址、缓冲区大小 */
		/* 先不使能此通道DMA */
		
		
		/* 接收DMA */
		USART_RX_DMA->CCR = 0;
		USART_RX_DMA->CCR = 
			DMA_DIR_PeripheralSRC |
			DMA_M2M_Disable |
			DMA_Mode_Circular |
			DMA_Priority_High |
			
			DMA_MemoryDataSize_Byte |
			DMA_MemoryInc_Enable |
			
			DMA_PeripheralDataSize_Byte |
			DMA_PeripheralInc_Disable;
		/* 内存地址 */
		USART_RX_DMA->CMAR = (u32)&receive_data;
		/* 外设地址 */
		USART_RX_DMA->CPAR = (u32)&USART1->DR;
		/* 缓冲区大小 */
		USART_RX_DMA->CNDTR = MAX_BUFFER_SIZE_;
		
		/* 使能此通道DMA */
		USART_RX_DMA->CCR |= 0x00000001;

		/* 使能串口DMA请求 */
		USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
	}
}


/*
 * 初始化LED的GPIO
 */
static void s_GPIOLowLevelInit(void)
{
	GPIO_InitTypeDef LEDIO;
	
	LEDIO.GPIO_Speed = GPIO_Speed_50MHz;
	LEDIO.GPIO_Mode = GPIO_Mode_Out_PP;
	
	LEDIO.GPIO_Pin = LED1_PIN;
	GPIO_Init(LED1_GPIO, &LEDIO);
	
	LEDIO.GPIO_Pin = LED2_PIN;
	GPIO_Init(LED2_GPIO, &LEDIO);
	
	LEDIO.GPIO_Pin = LED3_PIN;
	GPIO_Init(LED3_GPIO, &LEDIO);
	
	LEDIO.GPIO_Pin = LED4_PIN;
	GPIO_Init(LED4_GPIO, &LEDIO);
	
	LED1_ON;
	LED3_ON;
}


/*
 * 设置ESP8266
 * 
 *	关回显、连接服务器
 */
static void s_ESP8266LowLevelInit(void)
{
	/* 关回显 */
	sign_send = SIGN_WAITING;
	/* 不想发空字符，所以减了1 */
	BSDSend((void*)ATE0, sizeof(ATE0) - 1);
	/* 等待发完信息；标志位在别的进程中改变 */
	while(sign_send != SIGN_OK)
	{
		OSTimeDly(100);
	}
	
	/* 连接服务器 */
	sign_send = SIGN_WAITING;
	BSDSend((void*)AT_CIPSTART, sizeof(AT_CIPSTART) - 1);
	while(sign_send != SIGN_OK)
	{
		OSTimeDly(100);
	}
}
