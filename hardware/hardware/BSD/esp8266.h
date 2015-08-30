/*
 * esp8266.h
 *	定义ESP8266模块一些常数
 */
 
#ifndef MY_ESP8266_H
#define MY_ESP8266_H

/*
 * 模块返回的字符串
 *		分类处理
 *		对OK、ERROR、+IPD类作反应
 */
 
/* 注意，是小端模式 */

	/* 接收到的句子，以 \r\n 为开头*/
#define		SEGMENT_START		('\r' + ('\n'<<8))
	/* OK字样 */
#define		SEGMENT_OK			('O' + ('K'<<8) + ('\r'<<16) + ('\n'<<24))
	/* 如果是AT+CIPSEND指令，返回“OK”以后，还有“>”号 */
#define		SEGMENT_SEND_READY	('>')
	/* 返回“SEND”字样 */
#define		SEGMENT_SEND		('S' + ('E'<<8) + ('N'<<16) + ('D'<<24))
	/* 返回“+IPD”字样，说明收到了一些东西 */
#define		SEGMENT_GET_STH		('+' + ('I'<<8) + ('P'<<16) + ('D'<<24))


/*
 * 定义各类ESP8266返回符号的标示符
 */
typedef enum tag_sign_return
{
	SIGN_WAITING = 0,			/* 等待回应 */
	SIGN_ERROR,					/* 错误 */
	SIGN_OK,					/* 执行OK */
	SIGN_SEND_READY,			/* 做好发送数据的准备了 */
	SIGN_SEND_OK,				/* 发送成功 */
	SIGN_GET_DATA				/* 收到数据 */
}	en_ReturnSign;

	

#endif
