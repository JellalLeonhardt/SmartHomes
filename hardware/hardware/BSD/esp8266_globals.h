/*
 * ����ȫ�ֱ���
 *	
 *	��ʵ������UCOS�ġ��ź������ģ����򵥵��ź���������ȫ�ֱ���������
 *
 *	����������ȫ�ֱ����ĵط���Ҫ��define GLOBALS����include���ͷ�ļ���
 */
 
#ifndef _MY_GLOBALS_H__
#define _MY_GLOBALS_H_

#include "ESP8266.h"

#ifdef GLOBALS
#define EXT_E
#else
#define EXT_E extern
#endif


	/* ���͡����յ�ȫ�ֱ�־ */
EXT_E en_ReturnSign sign_send, sign_receive;


#endif
