/*
 * BSD.c
 *	��Ӳ��ƽ̨��صĲ���
 *
 *	ͨѶ������ͷ����
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
 * ˽�к궨��
 */
	/* ����1�ķ���DMAͨ�� */
#define		USART_TX_DMA	DMA1_Channel4
	/* ����1�Ľ���DMAͨ�� */
#define		USART_RX_DMA	DMA1_Channel5

	/* ��ʼ�������ݣ����������жϡ��ñ�־λ */
#define		CAMERA_START \
	EXTI->IMR |= EXTI_Line7;\
	EXTI->RTSR |= EXTI_Line7;\
	camera_buffer_info.sign_getphoto = 0;
	/*
	 *	�տ�ʼ�������������жϡ��ܿ��أ�
	 *	���������ж�ʱ�ص������ء������½���
	 *	���½����ж�ʱ�ص��½��ء��ر��ܿ���
	 */
	 
	 /* �����Ĳ��� */
#define CAMERA_WAIT \
	while(camera_buffer_info.sign_getphoto == 0)\
	{\
		OSTimeDly(70);\
	}


/* 
 * ˽�б���
 */
	/* ���ջ����� */
#define 	MAX_BUFFER_SIZE_ 	128
	/* ���ջ�������ҪԤ����4���ֽڣ��Ա����ġ�ѭ����ȡ��ʱ������ */
static u8 receive_data[MAX_BUFFER_SIZE_ + 4];
#define		BUFFER_START_		(&receive_data[0])
#define 	BUFFER_END			(&receive_data[MAX_BUFFER_SIZE_ - 1])
#define		BUFFER_APPEND		(&receive_data[MAX_BUFFER_SIZE_])
/*
 * һЩ�ַ�����ATָ�
 */
	/* �رջ��� */
const char ATE0[] = "ATE0\r\n";
	/* ���ӷ���������ʱ�̶�IP��ַ�Ͷ˿ں� */
const char AT_CIPSTART[] = "AT+CIPSTART=\"TCP\",\"192.168.1.108\",8080\r\n";
	/* �ر����� */
const char AT_CIPCLOSE[] = "AT+CIPCLOSE\r\n";


/*
 * ˽�к���
 */
	/* ��ʼ�������Լ���Ӧ��DMA */
static void s_USARTLowLevelInit(void);
	/* ��ʼ��LED��GPIO */
static void s_GPIOLowLevelInit(void);
	/* ����ESP8266���ػ��� */
static void s_ESP8266LowLevelInit(void);


/*
 * �ײ��ʼ��
 *	�ȳ�ʼ������ͷ���ٳ�ʼ������
 *
 *	��ΪESP8266ģ��Ҫ��5���ʼ���Լ����������������Ϣ�����룩
 *	����ʼ������ͷҪ����ӣ������������˴��ڿ�ͷ������
 *
 *	���أ�
 *		0Ϊ�ɹ���-1ʧ��
 */
int BSDLowlevelInit(void)
{
	/* ��ʼ��ȫ�ֱ��� */
	sign_receive = sign_send = SIGN_WAITING;
	
	/* ��ʼ������ͷ */
	CameraLowLevelInit();
	
	/* ��ʼ��LED */
	s_GPIOLowLevelInit();
	
	if(CameraInit() != 0)
	{
		return -1;
	}
	
	
	/* ��ʼ�������Լ���Ӧ��DMA */
	s_USARTLowLevelInit();
	
	/* ����ESP8266���ػ��� */
	s_ESP8266LowLevelInit();
	
	return 0;
}


/*
 * ����һ������
 *
 *	��DMA���ͣ����ú�DMA�Ժ�ֱ�ӷ��أ������ȴ�
 */
void BSDSend(void *data, int size)
{
	/* �ȴ���һ�ַ��ͽ��� */
	while(USART_TX_DMA->CNDTR != 0);
	
	/* �����ڴ��ַ�����ݴ�С */
	USART_TX_DMA->CCR &= ~0x00000001;
	USART_TX_DMA->CMAR = (u32)data;
	USART_TX_DMA->CNDTR = size;
	
	/* ��ʼ���� */
	USART_TX_DMA->CCR |= 0x00000001;
}


/*
 * ��������
 *
 *	Ҫÿ��һ��ʱ�������һ��
 */
void BSDReceive(void)
{
	/* ��ͷ����ʼ���ֽ�ɨ�� */
	static u8* p_read_head = BUFFER_START_;
	/* ֱ����ʱ�յ���Ϣ��β�� */
	int sign_get_head = 0;
	
	while(p_read_head != BUFFER_START_ + MAX_BUFFER_SIZE_ - USART_RX_DMA->CNDTR)
	{
		/* ��������ͷ�������ֽڸ��Ƶ�β��ȥ��
		   �Ա�һ�ζ�ȡ����ֽ�ʱ��˳����ѭ������ȡ������ */
		*(u32*)BUFFER_APPEND = *(u32*)BUFFER_START_;
		
		/* �κλظ������� \r\n Ϊ��ͷ�ģ���Ҫ�ҵ����ͷ */
		if(!sign_get_head)
		{
			/* һ��ȡ�����ֽڣ����Ч�� */
			u16 sh_head = *(u16*)p_read_head;
			if(sh_head == SEGMENT_START)
			{
				/* ���ñ�־ */
				sign_get_head = 1;
				/* ָ���������ֽ� */
				p_read_head += 2;
			}
			else
			{
				/* ָ����һ���ֽ� */
				p_read_head ++;
			}
		}
		else
		{
			/* �Ѿ��ҵ�ͷ���ˣ���������Ӧ�Ļظ��� */
			/* һ��ȡ4�ֽڣ����Ч�� */
			u32 i_segment = *(u32*)p_read_head;
			switch(i_segment)
			{
				case SEGMENT_OK:
					/* �����ˡ�OK������ */
					sign_send = SIGN_OK;

					sign_get_head = 0;
					p_read_head += 4;
					break;
				case SEGMENT_SEND:
					/* �����ˡ�SEND�����������������Ƿ���š�OK�� */
					p_read_head += 5;
					/* ��SEND OK\r\n��һ��9���ֽڣ���ֹ������ */
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
					/* �����ˡ�+IPD���������յ�����������������Ϣ */
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
					/* ������ϵĶ������ϣ��������ǲ��� \r\n���������������һ���ֶ� */
					if((i_segment & 0xffff) == SEGMENT_START)
						p_read_head += 2;
					else
						sign_get_head = 0;
					break;
			}
		}
		
		/* ��Ҫѭ��ɨ�裬������������ˣ�Ҫ��ͷ */
		if(p_read_head > BUFFER_END)
			p_read_head -= MAX_BUFFER_SIZE_;
	}
}


/*
 * ��һ����Ƭ
 *
 *	����������ֱ�������ٷ���
 */
void *BSDCameraShut(int* size)
{
	int n;
	char* tmp_p;

	CAMERA_START;
	/* ���������� */
	CAMERA_WAIT;
	
	/* Ѱ��JPEGͷ��ֻ��ǰ100�ֽ����ң� */
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
 * �����յ��ġ����ư���
 *
 *	��Ӧһ������Ӧ���������߿���GPIO
 *
 *	���룺
 *		ctrl�������ƴ��š�
 *		data�����������ݡ�
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
 *	˽�к���
 */
/*
 * ��ʼ������1
 *
 *	��ʼ��GPIO�����ڡ���Ӧ��DMAͨ��
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
		/* ���� */
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
		
		/* ����DMA */
		/* CCR�Ĵ������� */
		USART_TX_DMA->CCR = 0;
		
		/* �������� */
		USART_TX_DMA->CCR = 
			DMA_DIR_PeripheralDST |
			DMA_M2M_Disable |
			DMA_Mode_Normal |
			DMA_Priority_Medium |
		
			DMA_MemoryDataSize_Byte |
			DMA_MemoryInc_Enable |
		
			DMA_PeripheralDataSize_Byte |
			DMA_PeripheralInc_Disable;
		
		/* �����ַ */
		USART_TX_DMA->CPAR = (u32)&USART1->DR;
		
		/* �Ȳ������ڴ��ַ����������С */
		/* �Ȳ�ʹ�ܴ�ͨ��DMA */
		
		
		/* ����DMA */
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
		/* �ڴ��ַ */
		USART_RX_DMA->CMAR = (u32)&receive_data;
		/* �����ַ */
		USART_RX_DMA->CPAR = (u32)&USART1->DR;
		/* ��������С */
		USART_RX_DMA->CNDTR = MAX_BUFFER_SIZE_;
		
		/* ʹ�ܴ�ͨ��DMA */
		USART_RX_DMA->CCR |= 0x00000001;

		/* ʹ�ܴ���DMA���� */
		USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
	}
}


/*
 * ��ʼ��LED��GPIO
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
 * ����ESP8266
 * 
 *	�ػ��ԡ����ӷ�����
 */
static void s_ESP8266LowLevelInit(void)
{
	/* �ػ��� */
	sign_send = SIGN_WAITING;
	/* ���뷢���ַ������Լ���1 */
	BSDSend((void*)ATE0, sizeof(ATE0) - 1);
	/* �ȴ�������Ϣ����־λ�ڱ�Ľ����иı� */
	while(sign_send != SIGN_OK)
	{
		OSTimeDly(100);
	}
	
	/* ���ӷ����� */
	sign_send = SIGN_WAITING;
	BSDSend((void*)AT_CIPSTART, sizeof(AT_CIPSTART) - 1);
	while(sign_send != SIGN_OK)
	{
		OSTimeDly(100);
	}
}
