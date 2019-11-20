#include "key.h"
#include "stm32f10x_gpio.h"
#include "GPIO_STM32F10x.h"
#include "stm32f10x_rcc.h"
#include <includes.h>

//#include "delay.h" 

void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;														
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;  
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;														
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
} 

//�Ѽ�*��Ϊ11����#��Ϊ12����ʱ���Գɹ�������Ӧ�û�����Щ���⣬������
int key(void)  //ʵ�־�����̡�����ֵΪKeyVal���������ļ�ֵ���˼�ֵ���û��Լ����塣
{
	int KeyVal = -1;	 //keyValΪ��󷵻صļ�ֵ��
	GPIO_SetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3);//GPIOA0,1,2,3Ϊ��,��������ߵ�ƽ	
	if((GPIOA->IDR & 0x00e0)==0x0000)  //�����PA0,1,2,3ȫΪ0����û�м����¡���ʱ������ֵΪ-1.
		return -1;
	else
		{
			OSTimeDly(2);//ʱ��Ϊ10����ʱ20ms
		if((GPIOA->IDR & 0x00e0)==0x0000)  //�������ʱ10ms��PE0,2,4,6��ȫΪ0����ղ����ŵĵ�λ�仯�Ƕ���������.
		return -1;
		}
	  
//		GPIOA->ODR &= (0xff00|0x00e8);
		GPIOA->ODR |= 0x000f;
		GPIOA->ODR &= 0xffe8;
//		GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);
	switch(GPIOA->IDR & 0x00e0)		                //��PE0,2,4,6��ֵ�����жϣ��������ͬ�ļ�ֵ��
	{
		case 0x0080: KeyVal=6;	break;
		case 0x0040: KeyVal=4;	break;
		case 0x0020: KeyVal=5;	break;
	}
		while((GPIOA->IDR & 0x00e0)!=0x0000);	//���ּ��
//		delay_us(1);
		GPIOA->ODR |= 0x000f;
		GPIOA->ODR &= 0xffe4;
	switch(GPIOA->IDR & 0x00e0)		                //��PE0,2,4,6��ֵ�����жϣ��������ͬ�ļ�ֵ��
	{
		case 0x0080: KeyVal=9;	break;
		case 0x0040: KeyVal=7;	break;
		case 0x0020: KeyVal=8;	break;
	}
		while((GPIOA->IDR & 0x00e0)!=0x0000);		//���ּ��
//		delay_us(1);
		GPIOA->ODR |= 0x000f;
		GPIOA->ODR &= 0x00e2;
		
	switch(GPIOA->IDR & 0x00e0)		                //��PE0,2,4,6��ֵ�����жϣ��������ͬ�ļ�ֵ��
	{
		//�������Ҫ��
		case 0x0080: KeyVal=12;	break;
//		case 0x0040: KeyVal=11;	break;	//ԭ���̵�7�����ˣ�ֻ������*��������
		case 0x0040: KeyVal=7;	break;
		case 0x0020: KeyVal=0;	break;	//0
	}
		while((GPIOA->IDR & 0x00e0)!=0x0000);		//���ּ��
//		delay_us(1);
	GPIOA->ODR |= 0x000f;
	GPIOA->ODR &= 0x00e1;
		
	switch(GPIOA->IDR & 0x00e0)		               //��PE0,2,4,6��ֵ�����жϣ��������ͬ�ļ�ֵ��
	{
		case 0x0080: KeyVal=3;	break;
		case 0x0040: KeyVal=1;	break;
		case 0x0020: KeyVal=2;	break;
	}
	while((GPIOA->IDR & 0x00e0)!=0x0000);			//���ּ��
//		delay_us(1);
	
	return KeyVal;		
}


















