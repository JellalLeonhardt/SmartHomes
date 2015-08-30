/*
 * ov2640_private.h
 *	ģ��SCCCB����
 *	ȫ������˽�к�����ֱ����ͷ�ļ�������������
 */

#ifndef _MY_OV2640_PRIVATE_H_
#define _MY_OV2640_PRIVATE_H_

#include <stm32f10x.h>
#include "cam_private.h"

/****************************************************
 *                                                  *
 *                    �궨��                        *
 *                                                  *
 ****************************************************/
 /* OV2640�ġ�����ַ���͡�д��ַ�� */
#define RDADD 0x61
#define WRADD 0x60

/****************************************************
 *                                                  *
 *                   ˽�к���                       *
 *                                                  *
 ****************************************************/
/* дOV2640�ļĴ��� */
static 	int		wrOV2640REG(u8 regID, u8 regDAT);
/* ��OV2640�ļĴ��� */
static 	int		rdOV2640REG(u8 regID);

/* �����Ƕ�д�Ĵ����������õ��Ӻ��� */
	/* ��ʱʵ�ֵ���ʱ������ר������ģ��ʱ�򣬲���ucos��ʱ */
static 	void 	delay_us(int n);
	/* ����SCCB��ʼʱ�� */
static 	void 	SCCBStart(void);
	/* ����SCCB����ʱ�� */
static 	void 	SCCBStop(void);
	/* ����һ���ֽ� */
static 	int 	SCCBSend(u8 data);
	/* ����һ���ֽ� */
static 	u8 		SCCBReceive(void);
	/* ���޷�Ӧ��ʱ�� */
static	void	noAck(void);

/*************************************************
 *
 * wrOV2640REG(u8 regID, u8 regDAT)
 *	дOV2640�Ĵ���
 *
 *	���룺
 *		regID		�Ĵ�����ַ
 *		regDAT		Ҫд������
 *
 *	���أ�С��0ʧ��
 *
 *************************************************/
static int wrOV2640REG(u8 regID, u8 regDAT)
{
	/* ���Ϳ�ʼʱ�� */
	SCCBStart();
	
	/* ��������ͷ��д����ַ */
	if(0==SCCBSend(WRADD))
	{
		SCCBStop();
		return(-1);
	}
	delay_us(100);
	
	/* ���ͼĴ�����ַ */
	if(0==SCCBSend(regID))
	{
		SCCBStop();
		return(-1);
	}
	delay_us(100);
	
	/* ����Ҫд������ */
	if(0==SCCBSend(regDAT))
	{
		SCCBStop();
		return(-1);
	}
	
    SCCBStop();
	return 0;
}

/*************************************************
 *
 * rdOV2640(u8 regID, u8 regDAT)
 *	��OV2640�Ĵ���
 *
 *	���룺
 *		regID		�Ĵ�����ַ
 *
 *	���أ�С��0ʧ��
 *
 *************************************************/
static int rdOV2640REG(u8 regID)
{
	u8 tmp;

	/* ���Ϳ�ʼʱ�� */
	SCCBStart();
	
	/* ��Ҫ��������ͷ��д����ַ�� */
    if(0==SCCBSend(WRADD))
	{
		SCCBStop();
		return(-1);
	}
	delay_us(100);
	
	/* �ٷ�Ҫ���ļĴ����ĵ�ַ */
	if(0==SCCBSend(regID))
	{
		SCCBStop();
		return(-1);
	}
	delay_us(100);
    
	/* Ȼ���ٷ�����ͷ��������ַ�� */
	SCCBStart();
	if(0==SCCBSend(RDADD))
	{
		SCCBStop();
		return(-1);
	}
	
	/* Ȼ�����˳����ȡ���� */
	delay_us(100);
	tmp = SCCBReceive();
	noAck();
	SCCBStop();
	return tmp;
}

/*************************************************
 *
 * void SCCBStart(void);
 *	����SCCB��ʼʱ�� 
 *
 *************************************************/
static void SCCBStart(void)
{
	SIO_D_SET;
	delay_us(100);

	SIO_C_SET;
	delay_us(100);
 
	SIO_D_CLR;
	delay_us(100);

	SIO_C_CLR;
	delay_us(100);
}


/*************************************************
 *
 * void SCCBStop(void);
 *	����SCCB��ʼʱ�� 
 *
 *************************************************/
void SCCBStop(void)
{
	SIO_D_CLR;
	delay_us(100);
 
	SIO_C_SET;
	delay_us(100);
  
	SIO_D_SET;
	delay_us(100);
}


/*************************************************
 *
 * SCCBSend(u8 m_data)
 *	����һ���ֽ�
 *
 *	���룺
 *		m_data		Ҫ��������
 *
 *	���أ�0��ʧ�ܣ�1���ɹ�
 *
 *************************************************/
static int SCCBSend(u8 m_data)
{
	unsigned char j,tem;

	for(j=0;j<8;j++)
	{
		if((m_data<<j)&0x80)
		{
			SIO_D_SET;
		} else {
			SIO_D_CLR;
		}
		delay_us(100);
		SIO_C_SET;
		delay_us(100);
		SIO_C_CLR;
		delay_us(100);
	}
	delay_us(100);
    
	SCCB_D_INPUT;/* ��Ϊ���� */
	delay_us(100);
	SIO_C_SET;
	delay_us(100);
	if(SIO_D_STATE)
	{
		tem=0;   //SDA=1????,??0
	} else {
		tem=1;   //SDA=0????,??1
	}
	SIO_C_CLR;
	delay_us(100);    
	SCCB_D_OUTPUT;/* ������ */

    return(tem);  
}


/*************************************************
 *
 * u8 SCCBReceive(u8 m_data)
 *	����һ���ֽ�
 *
 *	���أ��յ����ֽ�
 *
 *************************************************/
static u8 SCCBReceive(void)
{
	u8 read, j;
	
	read = 0;
	
	SCCB_D_INPUT;
	delay_us(100);
	for(j = 0; j < 8; j++)
	{
		SIO_C_SET;
		delay_us(100);
		if(SIO_D_STATE)
			read = (read << 1) + 1;
		else
			read <<= 1;
		SIO_C_CLR;
		delay_us(100);
	}
	
	return read;
}


/*************************************************
 *
 * void noAck(void);
 *	����SCCB������Ӧ��ʱ�� 
 *
 *************************************************/
void noAck(void)
{
    
    SIO_D_SET;
    delay_us(100);
    
    SIO_C_SET;
    delay_us(100);
    
    SIO_C_CLR;
    delay_us(100);
    
    SIO_D_CLR;
    delay_us(100);

}

#endif
