/*
 * display.h
 *	gtk+图形界面
 *	初始化、画图等等
 */

#ifndef _MY_DISPLAY_H_
#define _MY_DISPLAY_H_

int DisplayLowLevelInit(void);
void DisplayWriteData(void *buffer, int size);
void DisplayDraw(void);

#endif

