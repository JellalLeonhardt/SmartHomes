/*
 * esp8266.h
 *	����ESP8266ģ��һЩ����
 */
 
#ifndef MY_ESP8266_H
#define MY_ESP8266_H

/*
 * ģ�鷵�ص��ַ���
 *		���ദ��
 *		��OK��ERROR��+IPD������Ӧ
 */
 
/* ע�⣬��С��ģʽ */

	/* ���յ��ľ��ӣ��� \r\n Ϊ��ͷ*/
#define		SEGMENT_START		('\r' + ('\n'<<8))
	/* OK���� */
#define		SEGMENT_OK			('O' + ('K'<<8) + ('\r'<<16) + ('\n'<<24))
	/* �����AT+CIPSENDָ����ء�OK���Ժ󣬻��С�>���� */
#define		SEGMENT_SEND_READY	('>')
	/* ���ء�SEND������ */
#define		SEGMENT_SEND		('S' + ('E'<<8) + ('N'<<16) + ('D'<<24))
	/* ���ء�+IPD��������˵���յ���һЩ���� */
#define		SEGMENT_GET_STH		('+' + ('I'<<8) + ('P'<<16) + ('D'<<24))


/*
 * �������ESP8266���ط��ŵı�ʾ��
 */
typedef enum tag_sign_return
{
	SIGN_WAITING = 0,			/* �ȴ���Ӧ */
	SIGN_ERROR,					/* ���� */
	SIGN_OK,					/* ִ��OK */
	SIGN_SEND_READY,			/* ���÷������ݵ�׼���� */
	SIGN_SEND_OK,				/* ���ͳɹ� */
	SIGN_GET_DATA				/* �յ����� */
}	en_ReturnSign;

	

#endif
