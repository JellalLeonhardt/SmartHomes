/*
 * ov2640_private.h
 *	模拟SCCCB总线
 *	全部都是私有函数，直接在头文件中声明、定义
 */

#ifndef _MY_OV2640_PRIVATE_H_
#define _MY_OV2640_PRIVATE_H_

#include <stm32f10x.h>
#include "cam_private.h"

/****************************************************
 *                                                  *
 *                    宏定义                        *
 *                                                  *
 ****************************************************/
 /* OV2640的“读地址”和“写地址” */
#define RDADD 0x61
#define WRADD 0x60

/****************************************************
 *                                                  *
 *                   私有函数                       *
 *                                                  *
 ****************************************************/
/* 写OV2640的寄存器 */
static 	int		wrOV2640REG(u8 regID, u8 regDAT);
/* 读OV2640的寄存器 */
static 	int		rdOV2640REG(u8 regID);

/* 以下是读写寄存器函数调用的子函数 */
	/* 临时实现的延时函数，专门用来模拟时序，不用ucos延时 */
static 	void 	delay_us(int n);
	/* 发送SCCB开始时序 */
static 	void 	SCCBStart(void);
	/* 发送SCCB结束时序 */
static 	void 	SCCBStop(void);
	/* 发送一个字节 */
static 	int 	SCCBSend(u8 data);
	/* 接收一个字节 */
static 	u8 		SCCBReceive(void);
	/* “无反应”时序 */
static	void	noAck(void);

/*************************************************
 *
 * wrOV2640REG(u8 regID, u8 regDAT)
 *	写OV2640寄存器
 *
 *	输入：
 *		regID		寄存器地址
 *		regDAT		要写的数据
 *
 *	返回：小于0失败
 *
 *************************************************/
static int wrOV2640REG(u8 regID, u8 regDAT)
{
	/* 发送开始时序 */
	SCCBStart();
	
	/* 发送摄像头“写”地址 */
	if(0==SCCBSend(WRADD))
	{
		SCCBStop();
		return(-1);
	}
	delay_us(100);
	
	/* 发送寄存器地址 */
	if(0==SCCBSend(regID))
	{
		SCCBStop();
		return(-1);
	}
	delay_us(100);
	
	/* 发送要写的数据 */
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
 *	读OV2640寄存器
 *
 *	输入：
 *		regID		寄存器地址
 *
 *	返回：小于0失败
 *
 *************************************************/
static int rdOV2640REG(u8 regID)
{
	u8 tmp;

	/* 发送开始时序 */
	SCCBStart();
	
	/* 先要发送摄像头“写”地址！ */
    if(0==SCCBSend(WRADD))
	{
		SCCBStop();
		return(-1);
	}
	delay_us(100);
	
	/* 再发要读的寄存器的地址 */
	if(0==SCCBSend(regID))
	{
		SCCBStop();
		return(-1);
	}
	delay_us(100);
    
	/* 然后再发摄像头“读”地址！ */
	SCCBStart();
	if(0==SCCBSend(RDADD))
	{
		SCCBStop();
		return(-1);
	}
	
	/* 然后才能顺利读取数据 */
	delay_us(100);
	tmp = SCCBReceive();
	noAck();
	SCCBStop();
	return tmp;
}

/*************************************************
 *
 * void SCCBStart(void);
 *	发送SCCB开始时序 
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
 *	发送SCCB开始时序 
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
 *	发送一个字节
 *
 *	输入：
 *		m_data		要发的数据
 *
 *	返回：0：失败；1：成功
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
    
	SCCB_D_INPUT;/* 设为输入 */
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
	SCCB_D_OUTPUT;/* 设回输出 */

    return(tem);  
}


/*************************************************
 *
 * u8 SCCBReceive(u8 m_data)
 *	接收一个字节
 *
 *	返回：收到的字节
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
 *	发送SCCB“无响应”时序 
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
