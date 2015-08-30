/*
 * camera.h
 *	����ͷ�����ĺ����ӿ�
 *
 */

#ifndef MY_CAMERA_H_
#define MY_CAMERA_H_

#include "cam_cfg.h"


/*
 * ���ݽṹ
 */
	/* ����ͷ��������Ϣ */
typedef struct tag_camera_buffer
{
	/* ��־λ */
	int sign_getphoto;
	
	/* ���� */
	int size;
	
} st_cam_buf_info;


/*
 * �����ӿ� 
 */

/*
 * �ײ��ʼ��
 *	��ʼ������GPIO
 */
void CameraLowLevelInit(void);


/*
 * ��ʼ������ͷ
 *	������ͷ���ó�JPEG��ʽ�����
 *
 *	���أ�0Ϊ�ɹ�����0�����س����ָ���
 *
 *	����ͷ�漴��ʼ����������Ƭ��δ�ؿ�ʼ����
 */
int CameraInit(void);


/*
 * ��ʼ��Ӧ����ͷ������
 *	��ʼ������ʹ���жϣ��Ӷ���Ӧ����ͷ������
 */
void CameraStart(void);


/*
 * �궨��
 */
/* LED */
#define 	LED_ON				LED_GPIO->ODR &= ~(LED_PIN);
#define		LED_OFF				LED_GPIO->ODR |= LED_PIN;


#endif
