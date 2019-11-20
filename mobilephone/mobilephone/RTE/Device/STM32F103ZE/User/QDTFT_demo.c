/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "delay.h"
#include "Picture.h"
#include "QDTFT_demo.h"
#include <includes.h>

unsigned char Num[10]={0,1,2,3,4,5,6,7,8,9};

//TFTInit_Task
//TFTInit只初始化一次，所以先给它较高的优先级
//可以考虑把它放在LcdDriver.c中
//#define TFTInit_Task_Prio 3
//#define TFTInit_STK_Size 64
//OS_STK TFTInit_Task_STK[TFTInit_STK_Size];
//void TFTInit_Task(void* pdata);

void Redraw_Mainmenu(void)
{

	Lcd_Clear(GRAY0);
	
	Gui_DrawFont_GBK16(16,0,BLUE,GRAY0,"全动电子技术");
	Gui_DrawFont_GBK16(16,20,RED,GRAY0,"液晶测试程序");

	DisplayButtonUp(15,38,113,58); //x1,y1,x2,y2
	Gui_DrawFont_GBK16(16,40,YELLOW,GRAY0,"颜色填充测试");

	DisplayButtonUp(15,68,113,88); //x1,y1,x2,y2
	Gui_DrawFont_GBK16(16,70,BLUE,GRAY0,"文字显示测试");

	DisplayButtonUp(15,98,113,118); //x1,y1,x2,y2
	Gui_DrawFont_GBK16(16,100,RED,GRAY0,"图片显示测试");
//	delay_ms(1500);
	OSTimeDly(150);
}

void Num_Test(void)
{
	u8 i=0;
	Lcd_Clear(GRAY0);
	Gui_DrawFont_GBK16(16,20,RED,GRAY0,"Num Test");
//	delay_ms(1000);
	OSTimeDly(100);
	Lcd_Clear(GRAY0);

	for(i=0;i<10;i++)
	{
	Gui_DrawFont_Num32((i%3)*40,32*(i/3)+5,RED,GRAY0,Num[i+1]);
//	delay_ms(100);
		OSTimeDly(10);
	}
	
}

void Font_Test(void)
{
	Lcd_Clear(GRAY0);
	Gui_DrawFont_GBK16(16,10,BLUE,GRAY0,"文字显示测试");

//	delay_ms(1000);
	OSTimeDly(100);
	Lcd_Clear(GRAY0);
	Gui_DrawFont_GBK16(16,30,YELLOW,GRAY0,"全动电子技术");
	Gui_DrawFont_GBK16(16,50,BLUE,GRAY0,"专注液晶批发");
	Gui_DrawFont_GBK16(16,70,RED,GRAY0, "全程技术支持");
	Gui_DrawFont_GBK16(0,100,BLUE,GRAY0,"Tel:15989313508");
	Gui_DrawFont_GBK16(0,130,RED,GRAY0, "www.qdtech.net");	
//	delay_ms(1800);	
	OSTimeDly(180);
}

void Color_Test(void)
{
	u8 i=1;
	Lcd_Clear(GRAY0);	//清屏函数
	
	Gui_DrawFont_GBK16(20,10,BLUE,GRAY0,"Color Test");
//	delay_ms(200);
	OSTimeDly(20);

	while(i--)
	{
		Lcd_Clear(WHITE);
		Lcd_Clear(BLACK);
		Lcd_Clear(RED);
	  	Lcd_Clear(GREEN);
	  	Lcd_Clear(BLUE);
	}		
}

//取模方式 水平扫描 从左到右 低位在前
void showimage(const unsigned char *p) //显示40*40 QQ图片
{
  	int i,j,k; 
	unsigned char picH,picL;
	Lcd_Clear(WHITE); //清屏  
	
	for(k=0;k<1;k++)		//k决定列数循环次数
	{
	   	for(j=0;j<1;j++)	//j决定行数循环次数
		{	
//			Lcd_SetRegion(40*j,40*k,40*j+39,40*k+39);		//坐标设置
				Lcd_SetRegion(40*j,40*k,40*j+127,40*k+127);
			
//		    for(i=0;i<40*40;i++)		//i的循环次数决定图片分辨率
				for(i=0;i<128*128;i++)
			 {	
			 	picL=*(p+i*2);	//数据低位在前
				picH=*(p+i*2+1);				
				LCD_WriteData_16Bit(picH<<8|picL);  						
			 }	
		 }
	}		
}

//发送32*32的图片，现在只有绿色的拨打图片
void showplay(const unsigned char *p) //显示40*40 QQ图片
{
  	int i,j,k; 
	unsigned char picH,picL;
//	Lcd_Clear(WHITE); //清屏  
	
	for(k=3;k<4;k++)		//k决定列数循环次数
	{
	   	for(j=2;j<3;j++)	//j决定行数循环次数
		{	
//			Lcd_SetRegion(32*j,32*k,32*j+31,32*k+31);		//坐标设置
				Lcd_SetRegion(32*j+16,32*k,32*j+47,32*k+31);
//				Lcd_SetRegion(40*j,40*k,40*j+127,40*k+127);
			
		    for(i=0;i<32*32;i++)		//i的循环次数决定图片分辨率
//				for(i=0;i<128*128;i++)
			 {	
			 	picL=*(p+i*2);	//数据低位在前
				picH=*(p+i*2+1);				
				LCD_WriteData_16Bit(picH<<8|picL);  						
			 }	
		 }
	}		
}

void QDTFT_Test_Demo(void)
{
	Lcd_Init();
	LCD_LED_SET;//通过IO控制背光亮
	
//	Redraw_Mainmenu();//绘制主菜单(部分内容由于分辨率超出物理值可能无法显示)
//	Color_Test();//简单纯色填充测试
//	Num_Test();//数码管字体测试
//	Font_Test();//中英文显示测试		
//	showimage(gImage_qq);//图片显示示例
	
	showimage(gImage_test);
	
//	delay_ms(1200);
	
//	showimage(gImage_lain);
	
//	delay_ms(1200);
	
	OSTimeDly(120);

//	LCD_LED_CLR;//IO控制背光灭	
}

void QDTFT_Test_Demo2(void)
{
		showplay(gImage_play);
}

