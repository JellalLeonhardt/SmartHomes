#ifndef _MY_LED_CFG_H_
#define _MY_LED_CFG_H_

#define		LED1_GPIO	GPIOA
#define 	LED1_PIN	GPIO_Pin_2
#define		LED1_ON		LED1_GPIO->ODR |= LED1_PIN;
#define		LED1_OFF	LED1_GPIO->ODR &= ~LED1_PIN;

#define		LED2_GPIO	GPIOA
#define 	LED2_PIN	GPIO_Pin_0
#define		LED2_ON		LED2_GPIO->ODR |= LED2_PIN;
#define		LED2_OFF	LED2_GPIO->ODR &= ~LED2_PIN;

#define		LED3_GPIO	GPIOC
#define 	LED3_PIN	GPIO_Pin_2
#define		LED3_ON		LED3_GPIO->ODR |= LED3_PIN;
#define		LED3_OFF	LED3_GPIO->ODR &= ~LED3_PIN;

#define		LED4_GPIO	GPIOA
#define 	LED4_PIN	GPIO_Pin_0
#define		LED4_ON		LED4_GPIO->ODR |= LED4_PIN;
#define		LED4_OFF	LED4_GPIO->ODR &= ~LED4_PIN;


#endif
