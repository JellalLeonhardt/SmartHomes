/***********************************************
 *
 *	标题: ov2640.c
 *	作者: 秋阳电子
 *	网址：http://qiuyangdz.taobao.com
 *	日期: 2013/03/15
 *	版本：v1.0
 *	功能: ov2640初始化及相关单片机引脚初始化
 *	说明：
 *
 *************************************************/
#include "stm32f10x.h"
#include "stm32f10x_exti.h"

#include "ov2640.h"
#include "ov2640_private.h"
#include "parameters.h"


/*************************************************
 *
 * ov2640_light_mode_config
 *	ov2640 亮度模式选择
 *
 *	输入参数：亮度模式常量
 *
 **************************************************/
void ov2640_light_mode_config(u8 mode)
{
	switch(mode)
	{
		case AUTO:
			wrOV2640REG(0xff, 0x00);
			wrOV2640REG(0xc7, 0x00);
			break;
		case SUNNY:
			wrOV2640REG(0xff, 0x00);
			wrOV2640REG(0xc7, 0x40);
			wrOV2640REG(0xcc, 0x5e);
			wrOV2640REG(0xcd, 0x41);
			wrOV2640REG(0xce, 0x54);
			break;
		case CLOUDY:
			wrOV2640REG(0xff, 0x00);
			wrOV2640REG(0xc7, 0x40);
			wrOV2640REG(0xcc, 0x65);
			wrOV2640REG(0xcd, 0x41);
			wrOV2640REG(0xce, 0x4f);
			break;
		case OFFICE:
			wrOV2640REG(0xff, 0x00);
			wrOV2640REG(0xc7, 0x40);
			wrOV2640REG(0xcc, 0x52);
			wrOV2640REG(0xcd, 0x41);
			wrOV2640REG(0xce, 0x66);
			break;
		case HOME:
			wrOV2640REG(0xff, 0x00);
			wrOV2640REG(0xc7, 0x40);
			wrOV2640REG(0xcc, 0x42);
			wrOV2640REG(0xcd, 0x3f);
			wrOV2640REG(0xce, 0x71);
			break;
		default:
			wrOV2640REG(0xff, 0x00);
			wrOV2640REG(0xc7, 0x00);
			break;
	}
}


/*************************************************
 *
 * void ov2640_black_white_config(u8 blackwhite)
 *	ov2640 彩色黑白模式设置
 *
 *	输入参数：
 *      0x18 for B&W,
 *      0x40 for Negative,
 *      0x58 for B&W negative,
 *      0x00 for Normal,
 *
 **************************************************/
void ov2640_black_white_config(u8 blackwhite)
{
	wrOV2640REG(0xff, 0x00);
	wrOV2640REG(0x7c, 0x00);
	wrOV2640REG(0x7d, blackwhite);
	wrOV2640REG(0x7c, 0x05);
	wrOV2640REG(0x7d, 0x80);
	wrOV2640REG(0x7d, 0x80);
}


/*************************************************
 *
 * void ov2640_color_saturation(u8 value1, u8 value2)
 *	功能：ov2640 色饱和度设置
 *
 *	输入参数：
 *		value1 = 0x68, value2 = 0x68 for +2,
 *		value1 = 0x58, value2 = 0x58 for +1,
 *		value1 = 0x48, value2 = 0x48 for 0,
 *		value1 = 0x38, value2 = 0x38 fOR -1,
 *		value1 = 0x28, value2 = 0x28 for -2,     
 *
 **************************************************/
void ov2640_color_saturation(u8 value1, u8 value2)
{
	wrOV2640REG(0xff, 0x00);
	wrOV2640REG(0x7c, 0x00);
	wrOV2640REG(0x7d, 0x02);
	wrOV2640REG(0x7c, 0x03);
	wrOV2640REG(0x7d, value1);
	wrOV2640REG(0x7d, value2);
}


/*************************************************
 *
 * void ov2640_color_effectsConfig(u8 value1, u8 value2)
 *	功能：ov2640色彩显示模式设置
 *
 *	输入参数：
 *        value1 = 0x40, value2 = 0xa6 for Antique,
 *        value1 = 0xa0, value2 = 0x40 for Bluish,
 *        value1 = 0x40, value2 = 0x40 for Greenish,
 *        value1 = 0x40, value2 = 0xc0 for Reddish,
 *
 **************************************************/
void ov2640_color_effectsConfig(u8 value1, u8 value2)
{
	wrOV2640REG(0xff, 0x00);
	wrOV2640REG(0x7c, 0x00);
	wrOV2640REG(0x7d, 0x18);
	wrOV2640REG(0x7c, 0x05);
	wrOV2640REG(0x7d, value1);
	wrOV2640REG(0x7d, value2);
}
/*************************************************
 *
 * void ov2640_contrast_config(u8 value1, u8 value2)
 *	功能：ov2640对比度设置
 *
 *	输入参数：
 *        value1 = 0x28, value2 = 0x0c for Contrast +2,
 *        value1 = 0x24, value2 = 0x16 for Contrast +1,
 *        value1 = 0x20, value2 = 0x20 for Contrast 0,
 *        value1 = 0x1c, value2 = 0x2a for Contrast -1,
 *        value1 = 0x18, value2 = 0x34 for Contrast -2,
 *
 **************************************************/
void ov2640_contrast_config(u8 value1, u8 value2)
{
	wrOV2640REG(0xff, 0x00);
	wrOV2640REG(0x7c, 0x00);
	wrOV2640REG(0x7d, 0x04);
	wrOV2640REG(0x7c, 0x07);
	wrOV2640REG(0x7d, 0x20);
	wrOV2640REG(0x7d, value1);
	wrOV2640REG(0x7d, value2);
	wrOV2640REG(0x7d, 0x06);
}


/*************************************************
 *
 * ov2640_brightness_config(u8 brightness)
 *	功能：ov2640亮度设置
 *
 *	输入参数：
 *        0x40 for Brightness +2,
 *        0x30 for Brightness +1,
 *        0x20 for Brightness 0,
 *        0x10 for Brightness -1,
 *        0x00 for Brightness -2,
 *
 **************************************************/
void ov2640_brightness_config(u8 brightness)
{ 
	wrOV2640REG(0xff, 0x00);
	wrOV2640REG(0x7c, 0x00);
	wrOV2640REG(0x7d, 0x04);
	wrOV2640REG(0x7c, 0x09);
	wrOV2640REG(0x7d, brightness);
	wrOV2640REG(0x7d, 0x00);
}


/*************************************************
 *
 * void ov2640_auto_exposure(u8 level)
 *	功能：ov2640自动曝光等级设置
 *
 *	输入参数：0 - 4
 *
 **************************************************/
void ov2640_auto_exposure(u8 level)
{
	switch(level)
	{
		case 0:
			wrOV2640REG(0xff, 0x01);
			wrOV2640REG(0x24, 0x20);
			wrOV2640REG(0x25, 0x18);
			wrOV2640REG(0x26, 0x60);
			break;
		case 1:
			wrOV2640REG(0xff, 0x01);
			wrOV2640REG(0x24, 0x34);
			wrOV2640REG(0x25, 0x1c);
			wrOV2640REG(0x26, 0x70);
			break;
		case 2:
			wrOV2640REG(0xff, 0x01);
			wrOV2640REG(0x24, 0x3e);
			wrOV2640REG(0x25, 0x38);
			wrOV2640REG(0x26, 0x81);
			break;
		case 3:
			wrOV2640REG(0xff, 0x01);
			wrOV2640REG(0x24, 0x48);
			wrOV2640REG(0x25, 0x40);
			wrOV2640REG(0x26, 0x81);
			break;
		case 4:
			wrOV2640REG(0xff, 0x01);
			wrOV2640REG(0x24, 0x58);
			wrOV2640REG(0x25, 0x50);
			wrOV2640REG(0x26, 0x92);
			break;
		default:
			wrOV2640REG(0xff, 0x01);
			wrOV2640REG(0x24, 0x20);
			wrOV2640REG(0x25, 0x18);
			wrOV2640REG(0x26, 0x60);
			break;
	}
}


/*************************************************
 *
 * 名称：void ov2640_delay(u32 count)
 *	功能：延时
 *	输入参数：延时参数
 *	输出参数：无
 *	返回值：无
 *
 **************************************************/
void ov2640_delay(u32 count)
{
  for(; count != 0; count--);
}
/*************************************************
 *
 * 名称：void ov2640_reset(void)
 *	功能：ov2640复位
 *	输入参数：无
 *	输出参数：无
 *	返回值：  无
 *
 **************************************************/
void ov2640_reset(void)
{
	wrOV2640REG(OV2640_DSP_RA_DLMT, 0x01);
	wrOV2640REG(OV2640_SENSOR_COM7, 0x80);
}

/*************************************************
 *
 * void ov2640_jpeg_config(image_format_typedef image_format)
 *	功能：ov2640对应分辨率下JPEG输出模式设置
 *
 *	输入参数：
 *        JPEG_176x144 =  0x00,	    
 *        JPEG_320x240 =  0x01,	    
 *        JPEG_352x288 =  0x02,	    
 *        JPEG_640x480 =  0x03,	   
 *        JPEG_800x600 =  0x04,	   
 *        JPEG_1024x768 = 0x05,	    
 *
 **************************************************/
int ov2640_jpeg_config(image_format_typedef image_format)
{
	u32 i;

	ov2640_reset();
	ov2640_delay(0x500000);

	/* initialize ov2640 */
	for(i = 0; i < (sizeof(ov2640_jpeg_init) / 2); i++)
	{
		if(wrOV2640REG(ov2640_jpeg_init[i][0], ov2640_jpeg_init[i][1]) < 0)
			return i;
		ov2640_delay(0x1000);
	}

	/* set to output YUV422 */
	for(i = 0; i < (sizeof(ov2640_yuv422) / 2); i++)
	{
		if(wrOV2640REG(ov2640_yuv422[i][0], ov2640_yuv422[i][1]) < 0)
			return i;
		ov2640_delay(0x1000);
	}

	wrOV2640REG(0xff, 0x01);
	wrOV2640REG(0x15, 0x00);

	/* set to output jpeg */
	for(i = 0; i < (sizeof(ov2640_jpeg) / 2); i++)
	{
		if(wrOV2640REG(ov2640_jpeg[i][0], ov2640_jpeg[i][1]) < 0)
			return i;
		ov2640_delay(0x1000);
	}

	ov2640_delay(0x500000);

	switch(image_format)
	{
		case JPEG_176x144:
			for(i = 0; i < (sizeof(ov2640_176x144_jpeg) / 2); i++)
			{
				if(wrOV2640REG(ov2640_176x144_jpeg[i][0], ov2640_176x144_jpeg[i][1]) < 0)
					return i;
			} 
			break;
		case JPEG_320x240:
			for(i = 0; i < (sizeof(ov2640_320x240_jpeg) / 2); i++)
			{
				if(wrOV2640REG(ov2640_320x240_jpeg[i][0], ov2640_320x240_jpeg[i][1]) < 0)
					return i;
				ov2640_delay(0x1000);
			}
			break;
		case JPEG_352x288:
			for(i = 0; i < (sizeof(ov2640_352x288_jpeg) / 2); i++)
			{
				if(wrOV2640REG(ov2640_352x288_jpeg[i][0], ov2640_352x288_jpeg[i][1]) < 0)
					return i;
				ov2640_delay(0x1000);
			}
			break;
		case JPEG_640x480:
			for(i  =0; i < (sizeof(ov2640_640x480_jpeg) / 2); i++)
			{
				if(wrOV2640REG(ov2640_800x600_jpeg[i][0], ov2640_640x480_jpeg[i][1]) < 0)
					return i;
				ov2640_delay(0x1000);
			}
			break;
		case JPEG_800x600:
			for(i = 0; i < (sizeof(ov2640_800x600_jpeg) / 2); i++)
			{
				if(wrOV2640REG(ov2640_800x600_jpeg[i][0], ov2640_800x600_jpeg[i][1]) < 0)
					return i;
				ov2640_delay(0x1000);
			}
			break;
		case JPEG_1024x768:
			for(i = 0; i < (sizeof(ov2640_1024x768_jpeg) / 2); i++)
			{
				if(wrOV2640REG(ov2640_1024x768_jpeg[i][0], ov2640_1024x768_jpeg[i][1]) < 0)
					return i;
				ov2640_delay(0x1000);
			}
			break;
		default:
			for(i = 0; i < (sizeof(ov2640_320x240_jpeg) / 2); i++)
			{
				if(wrOV2640REG(ov2640_320x240_jpeg[i][0], ov2640_320x240_jpeg[i][1]) < 0)
					return i;
				ov2640_delay(0x1000);
			}
			break;
	}
	return 0;
}
void delay_us(int n)
{
	u16 i;
	
	while(n--){
		i = 10;
		while(i--);
	}
}
void ov2640_interrupt_enable(void)
{
  /* 使能像素同步中断 */
   EXTI->IMR |= EXTI_Line0;				
   EXTI->EMR |= EXTI_Line0;
}
void ov2640_interrupt_disable(void)
{
  /* 关闭像素同步中断 */
   EXTI->IMR &= ~EXTI_Line0;				
   EXTI->EMR &= ~EXTI_Line0; 
}
