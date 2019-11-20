#include "usart.h"
#include "stm32f10x_dma.h"
#include <includes.h>

INT8U length = 0;
INT8U DMA_Rece_Buf[DMA_Rec_Len];	   //DMA接收串口数据缓冲区
INT16U  Usart1_Rec_Cnt=0;             //本帧数据长度	

//注意这里可以使用柔性数组，但是暂不知道stm32是否支持柔性数组，效率怎样？
INT8U  DMA_TX_Buf[DMA_TX_Len] = {0};	   //DMA发送串口数据缓冲区

//按下数字的数量
INT8U TXBuffer_Num = 0;

//ASCII码表，数字的ascii码与实际数字相差48，做换算后也可以用作普通的数字数组，*和#不符合这个规律，但是不常用
const INT8U Ascii_Num[12] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '7', '#'};

//初始化IO 串口1 
//bound:波特率
void uart_init(INT32U bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	  USART_InitTypeDef USART_InitStructure;
	  NVIC_InitTypeDef NVIC_InitStructure;
	  DMA_InitTypeDef DMA_InitStructure;
	
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
		//使能DMA1传输，因为USART1_RX是DMA1的通道5
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
		
 	  USART_DeInit(USART1);  //复位串口1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

   //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能	
	  NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
		USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
		
		USART1->DR = 0x00000000;
		USART1->SR &= 0xffffffbf;
		
		
//    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
		USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
		
		USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);   //使能串口1 DMA接收
    USART_Cmd(USART1, ENABLE);                    //使能串口 
 
		DMA_DeInit(DMA1_Channel5);
		
    //相应的DMA配置
		//暂时只使用RX的DMA，用的是通道5
		DMA_DeInit(DMA1_Channel5);   //将DMA的通道5寄存器重设为缺省值  串口1对应的是DMA通道5
		
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA外设ADC基地址
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_Rece_Buf;  //DMA内存基地址
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设读取发送到内存
		DMA_InitStructure.DMA_BufferSize = DMA_Rec_Len;  //DMA通道的DMA缓存的大小
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
		DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器

		DMA_Cmd(DMA1_Channel5, ENABLE);  //正式驱动DMA传输
//		
//		
//		通道4是TX的DMA
//		DMA_DeInit(DMA1_Channel4);   //将DMA1的通道4寄存器重设为缺省值

//		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA外设基地址
//		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_Rece_Buf;  //DMA内存基地址
//		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
//		DMA_InitStructure.DMA_BufferSize = DMA_TX_Len;  //DMA通道的DMA缓存的大小
//		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
//		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
//		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
//		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
//		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常模式
//		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
//		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
//		DMA_Init(DMA1_Channel4, &DMA_InitStructure);		
}

//重新恢复DMA指针
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
		DMA_Cmd(DMA_CHx, DISABLE );  //关闭USART1 TX DMA1 所指示的通道      
		DMA_SetCurrDataCounter(DMA_CHx,DMA_Rec_Len);//DMA通道的DMA缓存的大小
		DMA_Cmd(DMA_CHx, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
}

//USART1开启一次DMA传输
void USART1_DMA_Send(u16 Trans_Len)
{ 
		USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);  //使能串口1 DMA
		DMA_Cmd(DMA1_Channel4, DISABLE );  //关闭USART1 TX DMA1 所指示的通道      
		DMA_SetCurrDataCounter(DMA1_Channel4,Trans_Len);//DMA通道的DMA缓存的大小
		DMA_Cmd(DMA1_Channel4, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
}

//发送len个字节.
//buf:发送区首地址
//len:发送的字节数
//查询方式进行发送
void Usart1_Send(u8 *buf,INT8U len)
{
		INT8U t;
  	for(t=0;t<len;t++)		//循环发送数据
		{		   
				while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
				USART_SendData(USART1,buf[t]);
		}	 
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
}

//串口中断函数
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
		INT8U i;
		INT32U delay = 5000000;
		OSIntEnter();    
		
		if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)		//发送
//		if(USART_GetFlagStatus(USART1,USART_FLAG_TC)!= RESET)
		//sim900a拨打电话，发送之前都要先发送ATD
//		USART_SendData(USART1, 'A');
//		USART_SendData(USART1, 'T');
//		USART_SendData(USART1, 'D');
		{
				USART_SendData(USART1, DMA_TX_Buf[length++]);
				//串口发送寄存器只能容纳一个数据，如果在一次中断中连续发送多个，
				//后面的数据会把前面的数据“顶”掉
//				USART_SendData(USART1, ';');
				if( length >= 17 )
				{
//						USART_SendData(USART1, ';');
						USART_ClearITPendingBit(USART1, USART_IT_TC);	//清除USART_IT_TC标志着一帧数据发送结束
						USART_ITConfig(USART1, USART_IT_TC, DISABLE);
						length = 0;
				}
		}
		
		if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)		//接收
		{
				USART_ReceiveData(USART1);//读取数据 注意：这句必须要，否则不能够清除中断标志位。
        Usart1_Rec_Cnt = DMA_Rec_Len-DMA_GetCurrDataCounter(DMA1_Channel5);	//算出接本帧数据长度
					
				//数据帧处理
				
				//数据帧处理完成
				
				USART_ClearITPendingBit(USART1, USART_IT_IDLE);         //清除中断标志
        MYDMA_Enable(DMA1_Channel5);                   //恢复DMA指针，等待下一次的接收
		}
		
		OSIntExit();  											 
} 
	

