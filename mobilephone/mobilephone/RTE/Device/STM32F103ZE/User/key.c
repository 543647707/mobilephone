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

//把键*定为11，键#定为12，暂时调试成功，但是应该还是有些问题，先这样
int key(void)  //实现矩阵键盘。返回值为KeyVal，各按键的键值，此键值由用户自己定义。
{
	int KeyVal = -1;	 //keyVal为最后返回的键值。
	GPIO_SetBits(GPIOA,GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3);//GPIOA0,1,2,3为高,各行输出高电平	
	if((GPIOA->IDR & 0x00e0)==0x0000)  //如果，PA0,1,2,3全为0，则没有键按下。此时，返回值为-1.
		return -1;
	else
		{
			OSTimeDly(2);//时基为10，延时20ms
		if((GPIOA->IDR & 0x00e0)==0x0000)  //如果，延时10ms后，PE0,2,4,6又全为0，则刚才引脚的电位变化是抖动产生的.
		return -1;
		}
	  
//		GPIOA->ODR &= (0xff00|0x00e8);
		GPIOA->ODR |= 0x000f;
		GPIOA->ODR &= 0xffe8;
//		GPIO_ResetBits(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);
	switch(GPIOA->IDR & 0x00e0)		                //对PE0,2,4,6的值进行判断，以输出不同的键值。
	{
		case 0x0080: KeyVal=6;	break;
		case 0x0040: KeyVal=4;	break;
		case 0x0020: KeyVal=5;	break;
	}
		while((GPIOA->IDR & 0x00e0)!=0x0000);	//松手检测
//		delay_us(1);
		GPIOA->ODR |= 0x000f;
		GPIOA->ODR &= 0xffe4;
	switch(GPIOA->IDR & 0x00e0)		                //对PE0,2,4,6的值进行判断，以输出不同的键值。
	{
		case 0x0080: KeyVal=9;	break;
		case 0x0040: KeyVal=7;	break;
		case 0x0020: KeyVal=8;	break;
	}
		while((GPIOA->IDR & 0x00e0)!=0x0000);		//松手检测
//		delay_us(1);
		GPIOA->ODR |= 0x000f;
		GPIOA->ODR &= 0x00e2;
		
	switch(GPIOA->IDR & 0x00e0)		                //对PE0,2,4,6的值进行判断，以输出不同的键值。
	{
		//这里可能要改
		case 0x0080: KeyVal=12;	break;
//		case 0x0040: KeyVal=11;	break;	//原键盘的7键坏了，只能先用*键来代替
		case 0x0040: KeyVal=7;	break;
		case 0x0020: KeyVal=0;	break;	//0
	}
		while((GPIOA->IDR & 0x00e0)!=0x0000);		//松手检测
//		delay_us(1);
	GPIOA->ODR |= 0x000f;
	GPIOA->ODR &= 0x00e1;
		
	switch(GPIOA->IDR & 0x00e0)		               //对PE0,2,4,6的值进行判断，以输出不同的键值。
	{
		case 0x0080: KeyVal=3;	break;
		case 0x0040: KeyVal=1;	break;
		case 0x0020: KeyVal=2;	break;
	}
	while((GPIOA->IDR & 0x00e0)!=0x0000);			//松手检测
//		delay_us(1);
	
	return KeyVal;		
}


















