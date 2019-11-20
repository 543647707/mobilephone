#include "usart.h"
#include "stm32f10x_dma.h"
#include <includes.h>

INT8U length = 0;
INT8U DMA_Rece_Buf[DMA_Rec_Len];	   //DMA���մ������ݻ�����
INT16U  Usart1_Rec_Cnt=0;             //��֡���ݳ���	

//ע���������ʹ���������飬�����ݲ�֪��stm32�Ƿ�֧���������飬Ч��������
INT8U  DMA_TX_Buf[DMA_TX_Len] = {0};	   //DMA���ʹ������ݻ�����

//�������ֵ�����
INT8U TXBuffer_Num = 0;

//ASCII������ֵ�ascii����ʵ���������48���������Ҳ����������ͨ���������飬*��#������������ɣ����ǲ�����
const INT8U Ascii_Num[12] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '7', '#'};

//��ʼ��IO ����1 
//bound:������
void uart_init(INT32U bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	  USART_InitTypeDef USART_InitStructure;
	  NVIC_InitTypeDef NVIC_InitStructure;
	  DMA_InitTypeDef DMA_InitStructure;
	
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
		//ʹ��DMA1���䣬��ΪUSART1_RX��DMA1��ͨ��5
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
		
 	  USART_DeInit(USART1);  //��λ����1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

   //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��	
	  NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
		USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
		USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
		
		USART1->DR = 0x00000000;
		USART1->SR &= 0xffffffbf;
		
		
//    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
		USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
		
		USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);   //ʹ�ܴ���1 DMA����
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
 
		DMA_DeInit(DMA1_Channel5);
		
    //��Ӧ��DMA����
		//��ʱֻʹ��RX��DMA���õ���ͨ��5
		DMA_DeInit(DMA1_Channel5);   //��DMA��ͨ��5�Ĵ�������Ϊȱʡֵ  ����1��Ӧ����DMAͨ��5
		
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA����ADC����ַ
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_Rece_Buf;  //DMA�ڴ����ַ
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴������ȡ���͵��ڴ�
		DMA_InitStructure.DMA_BufferSize = DMA_Rec_Len;  //DMAͨ����DMA����Ĵ�С
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
		DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���

		DMA_Cmd(DMA1_Channel5, ENABLE);  //��ʽ����DMA����
//		
//		
//		ͨ��4��TX��DMA
//		DMA_DeInit(DMA1_Channel4);   //��DMA1��ͨ��4�Ĵ�������Ϊȱʡֵ

//		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA�������ַ
//		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DMA_Rece_Buf;  //DMA�ڴ����ַ
//		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
//		DMA_InitStructure.DMA_BufferSize = DMA_TX_Len;  //DMAͨ����DMA����Ĵ�С
//		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
//		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
//		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
//		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
//		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //����������ģʽ
//		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
//		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
//		DMA_Init(DMA1_Channel4, &DMA_InitStructure);		
}

//���»ָ�DMAָ��
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
		DMA_Cmd(DMA_CHx, DISABLE );  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��      
		DMA_SetCurrDataCounter(DMA_CHx,DMA_Rec_Len);//DMAͨ����DMA����Ĵ�С
		DMA_Cmd(DMA_CHx, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� 
}

//USART1����һ��DMA����
void USART1_DMA_Send(u16 Trans_Len)
{ 
		USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);  //ʹ�ܴ���1 DMA
		DMA_Cmd(DMA1_Channel4, DISABLE );  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��      
		DMA_SetCurrDataCounter(DMA1_Channel4,Trans_Len);//DMAͨ����DMA����Ĵ�С
		DMA_Cmd(DMA1_Channel4, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� 
}

//����len���ֽ�.
//buf:�������׵�ַ
//len:���͵��ֽ���
//��ѯ��ʽ���з���
void Usart1_Send(u8 *buf,INT8U len)
{
		INT8U t;
  	for(t=0;t<len;t++)		//ѭ����������
		{		   
				while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
				USART_SendData(USART1,buf[t]);
		}	 
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);		
}

//�����жϺ���
void USART1_IRQHandler(void)                	//����1�жϷ������
{
		INT8U i;
		INT32U delay = 5000000;
		OSIntEnter();    
		
		if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)		//����
//		if(USART_GetFlagStatus(USART1,USART_FLAG_TC)!= RESET)
		//sim900a����绰������֮ǰ��Ҫ�ȷ���ATD
//		USART_SendData(USART1, 'A');
//		USART_SendData(USART1, 'T');
//		USART_SendData(USART1, 'D');
		{
				USART_SendData(USART1, DMA_TX_Buf[length++]);
				//���ڷ��ͼĴ���ֻ������һ�����ݣ������һ���ж����������Ͷ����
				//��������ݻ��ǰ������ݡ�������
//				USART_SendData(USART1, ';');
				if( length >= 17 )
				{
//						USART_SendData(USART1, ';');
						USART_ClearITPendingBit(USART1, USART_IT_TC);	//���USART_IT_TC��־��һ֡���ݷ��ͽ���
						USART_ITConfig(USART1, USART_IT_TC, DISABLE);
						length = 0;
				}
		}
		
		if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)		//����
		{
				USART_ReceiveData(USART1);//��ȡ���� ע�⣺������Ҫ�������ܹ�����жϱ�־λ��
        Usart1_Rec_Cnt = DMA_Rec_Len-DMA_GetCurrDataCounter(DMA1_Channel5);	//����ӱ�֡���ݳ���
					
				//����֡����
				
				//����֡�������
				
				USART_ClearITPendingBit(USART1, USART_IT_IDLE);         //����жϱ�־
        MYDMA_Enable(DMA1_Channel5);                   //�ָ�DMAָ�룬�ȴ���һ�εĽ���
		}
		
		OSIntExit();  											 
} 
	

