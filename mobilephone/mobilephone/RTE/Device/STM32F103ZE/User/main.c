#include "stm32f10x_gpio.h"
#include "GPIO_STM32F10x.h"
#include "misc.h"
#include "core_cm3.h"
#include "usart.h"
#include "key.h"
#include <includes.h>
#include "delay.h"
#include "Lcd_Driver.h"
#include "QDTFT_demo.h"
#include "GUI.h"
//#include "Picture.h"


//������������LED������˸
//LED0��ʱ200ms��LED1��ʱ500ms

//LED0�������
#define LED0_Task_Prio 5
#define LED0_STK_Size 64
OS_STK LED0_Task_STK[LED0_STK_Size];
void LED0_Task(void *pdata);

//LED1�������
#define LED1_Task_Prio 6
#define LED1_STK_Size 64
OS_STK LED1_Task_STK[LED1_STK_Size];
void LED1_Task(void *pdata);

//TaskStart���
#define TaskStart_Prio 1
#define TaskStart_Size 64
OS_STK TaskStart_STK[TaskStart_Size];
void TaskStart(void* pdata);

//����һ����Ϣ����
OS_EVENT* myQ;

//����һ������
OS_EVENT* myMBox;

//����һ���ź�
OS_EVENT* mySem;

//����һ�������ʱ��
OS_TMR* myTimer;

//����һ���¼���־��
OS_FLAG_GRP* myFlagGrp;

//����һ���ڴ�������
//�ڴ�������Ҫ�ֲ�����
//�����ͷ�
//OS_MEM* myMem;

//PostQ�������
#define PostQ_Task_Prio 2
#define PostQ_STK_Size 64
OS_STK PostQ_Task_STK[PostQ_STK_Size];
void PostQ_Task(void* pdata);

//PendQ�������
#define PendQ_Task_Prio 3
#define PendQ_STK_Size 64
OS_STK PendQ_Task_STK[PostQ_STK_Size];
void PendQ_Task(void* pdata);

//PostMsg�������
#define PostMsg_Task_Prio 7
#define PostMsg_STK_Size 64
OS_STK PostMsg_Task_STK[PostMsg_STK_Size];
void PostMsg_Task(void* pdata);

//PendMsg�������
#define PendMsg_Task_Prio 8
#define PendMsg_STK_Size 64
OS_STK PendMsg_Task_STK[PendMsg_STK_Size];
void PendMsg_Task(void* pdata);

//PostSem�������
#define PostSem_Task_Prio 9
#define PostSem_STK_Size 64
OS_STK PostSem_Task_STK[PostSem_STK_Size];
void PostSem_Task(void* pdata);

//PendSem�������
#define PendSem_Task_Prio 10
#define PendSem_STK_Size 64
OS_STK PendSem_Task_STK[PendSem_STK_Size];
void PendSem_Task(void* pdata);

//PostFlag�������
#define PostFlag_Task_Prio 11
#define PostFlag_STK_Size 64
OS_STK PostFlag_Task_STK[PostFlag_STK_Size];
void PostFlag_Task(void* pdata);

//PendFlag�������
#define PendFlag_Task_Prio 12
#define PendFlag_STK_Size 64
OS_STK PendFlag_Task_STK[PendFlag_STK_Size];
void PendFlag_Task(void* pdata);

//keyboard scan task
//�ȸ���������һ�����ȼ�����Ƶ��ɨ����������ȼ��ϵ�
#define KeyScan_Task_Prio 32 
#define KeyScan_STK_Size 64
OS_STK KeyScan_Task_STK[KeyScan_STK_Size];
void KeyScan_Task(void* pdata);

//USART_ISR send task
//�����������ȼ�Ӧ�ô���������������ȼ�
//���ڴ��ڷ���������˵������ɨ�������൱��������������
//���Լ���ɨ����������ȼ�Ҫ���ڴ��ڷ�������
#define USARTSend_Task_Prio 40 
#define USARTSend_STK_Size 64
OS_STK USARTSend_Task_STK[USARTSend_STK_Size];
void USARTSend_Task(void* pdata);

//TFTNumber_Task
//TFTNumber_Task��keyscan����������Ӧ�ø��ϵ͵����ȼ�
//����Ϊ���ڷ��������ǲ���绰�����񣬱���ʾ����Ҫ��Ҫ�Ķ�
//���Դ��ڷ�����������ȼ�Ҫ������ʾ����
#define TFTNumber_Task_Prio 44
#define TFTNumber_STK_Size 64
OS_STK TFTNumber_Task_STK[TFTNumber_STK_Size];
void TFTNumber_Task(void* pdata);




//�����ʱ��1�ص�����
void MyCallback (OS_TMR *ptmr, void *p_arg);

//void delay_ms(u16 time)
//{    
//   u16 i=0;  
//   while(time--)
//   {
//      i=12000;  //�Լ�����
//      while(i--) ;    
//   }
//}

int main(void)
{
		INT32U delay = 5000000;
		INT8U t;
		INT8U len = 5;
		INT8U buffer[5] = {48, 49, 50, 51, 52};
		GPIO_InitTypeDef GPIO_InitStruct;
		NVIC_InitTypeDef NVIC_InitStruct;
	
		GPIO_PortClock(GPIOB, 1);
		GPIO_Init(GPIOB, &GPIO_InitStruct);
		GPIO_PinConfigure(GPIOB, 5, GPIO_OUT_PUSH_PULL, GPIO_MODE_OUT50MHZ);
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	
		GPIO_PortClock(GPIOE, 1);
		GPIO_Init(GPIOE, &GPIO_InitStruct);
		GPIO_PinConfigure(GPIOE, 5, GPIO_OUT_PUSH_PULL, GPIO_MODE_OUT50MHZ);
		GPIO_ResetBits(GPIOE, GPIO_Pin_5);
		
		
		KEY_Init();
		
//		while(1)
//		{
//				t = key();
//			
//				delay = 4000000;
//				delay = 3000000;
//		};
	
		NVIC_Init(&NVIC_InitStruct);
		SysTick_Config(90000);
		SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	
//		delay_init(72);
	
		uart_init(9600);
		
//		while(1)
//		{
//				QDTFT_Test_Demo();
//		}
		
		
		
		OSInit();	
		
//		���õ��ǰ���Ȼ��򿪴��ڷ����жϣ�Ȼ���ڴ��ڷ����ж��н����ݷ��ͳ�ȥ
//		while(1) 
//		{
//				if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
//				{
//						delay_ms(200);
//						if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
//						{
//								USART_ITConfig(USART1, USART_IT_TC, ENABLE);
//								GPIOB->ODR ^= GPIO_Pin_5;
//						}
//				}
//		}

//		���õ��ǲ�ѯ�ķ�ʽ�����д��ڷ���
//		while(1)
//		{
////				USART_SendData(USART1, 27);
//				for(t=0; t<len; t++)
//				{	
//						USART_SendData(USART1, buffer[t]);
//						while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
//						GPIOB->ODR ^= GPIO_Pin_5;
//				}
//				while(delay--); 
//				delay = 5000000;
//		}
	
		OSTaskCreate(TaskStart, 0, &TaskStart_STK[TaskStart_Size-1], TaskStart_Prio);
		OSTaskCreate(KeyScan_Task, 0, &KeyScan_Task_STK[KeyScan_STK_Size-1], KeyScan_Task_Prio);
		OSTaskCreate(TFTNumber_Task, 0, &TFTNumber_Task_STK[TFTNumber_STK_Size-1], TFTNumber_Task_Prio);
//		OSTaskCreate(USARTSend_Task, 0, &USARTSend_Task_STK[USARTSend_STK_Size-1], USARTSend_Task_Prio);
//		OSTaskCreate(PostFlag_Task, (void*)50, &PostFlag_Task_STK[PostFlag_STK_Size-1], PostFlag_Task_Prio);
//		OSTaskCreate(PendFlag_Task, 0, &PendFlag_Task_STK[PendFlag_STK_Size-1], PendFlag_Task_Prio);
//		OSTaskCreate(PostSem_Task, (void*)50, &PostSem_Task_STK[PostSem_STK_Size-1], PostSem_Task_Prio);
//		OSTaskCreate(PendSem_Task, 0, &PendSem_Task_STK[PendSem_STK_Size-1], PendSem_Task_Prio);
//		OSTaskCreate(PostQ_Task, (void*)50, &PostQ_Task_STK[PostQ_STK_Size-1], PostQ_Task_Prio);
//		OSTaskCreate(PendQ_Task, 0, &PendQ_Task_STK[PendQ_STK_Size-1], PendQ_Task_Prio);
//		OSTaskCreate(LED0_Task, (void*)80, &LED0_Task_STK[LED0_STK_Size-1], LED0_Task_Prio);
//		OSTaskCreate(LED1_Task, (void*)30, &LED1_Task_STK[LED0_STK_Size-1], LED1_Task_Prio);
//		OSTaskCreate(PostMsg_Task, (void*)50, &PostMsg_Task_STK[PostMsg_STK_Size-1], PostMsg_Task_Prio);
//		OSTaskCreate(PendMsg_Task, 0, &PendMsg_Task_STK[PendMsg_STK_Size-1], PendMsg_Task_Prio);
		OSStart();
		
		return 0;
}

void PostQ_Task(void* pdata)
{
		INT8U err;
		INT8U i = 34;
		void* myq[6];
		OS_Q_DATA myQData;
	
		myQ = OSQCreate(myq, 6);
	
		if( myQ == (OS_EVENT*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		while(1)
		{
//				err = OSQPost(myQ, (void*)"Hello!");
//				OSQPost(myQ, (void*)"Hello!");
				GPIOB->ODR ^= GPIO_Pin_5;
				err = OSQPost(myQ, &i);
				if( err == OS_ERR_NONE )
				{
						OSQQuery(myQ, &myQData);
				}
				OSTimeDly((INT32U)pdata);
		}
}

void PendQ_Task(void* pdata)
{
//		char* prec;
		INT8U rec;
		INT8U err;
		
		rec = rec;
	
		if( myQ == (OS_EVENT*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		while(1)
		{
//				prec = (char*)OSQPend(myQ, 0, &err);
				rec = *(INT32U*)OSQPend(myQ, 0, &err);
				if( err == OS_ERR_NONE )
				{
						if( 34 == rec )
						{
								GPIOE->ODR ^= GPIO_Pin_5;
						}
				}
		}
}

void PostMsg_Task(void* pdata)
{
		INT32U i;
		OS_MBOX_DATA myMBoxData;
		
		i = 23;
	
		myMBox = OSMboxCreate((void*)0);
		
		if( myMBox == (OS_EVENT*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		
		while(1)
		{
				GPIOB->ODR ^= GPIO_Pin_5;
				OSMboxPost(myMBox, &i);
				OSMboxQuery(myMBox, &myMBoxData);
				OSTimeDly((INT32U)pdata);
		}
}

void PendMsg_Task(void* pdata)
{
		INT8U rec;
		INT8U err;
		if( myMBox == (OS_EVENT*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		
		while(1)
		{
				rec = *(INT32U*)OSMboxPend(myMBox, 0, &err);
				if( err == OS_ERR_NONE )
				{
						if( 23 == rec )
						{
								GPIOE->ODR ^= GPIO_Pin_5;
						}
				}
		}
}

void PostSem_Task(void* pdata)
{
		OS_SEM_DATA mySemData;
		mySem = OSSemCreate(0);
		
		if( mySem == (OS_EVENT*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		
		while(1)
		{
				GPIOB->ODR ^= GPIO_Pin_5;
				OSSemPost(mySem);
				OSSemQuery(mySem, &mySemData);
				OSTimeDly((INT32U)pdata);
		}
}

void PendSem_Task(void* pdata)
{
		INT8U err;
	
		if( mySem == (OS_EVENT*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		
		while(1)
		{
				OSSemPend(mySem, 0, &err);
				GPIOE->ODR ^= GPIO_Pin_5;
		}
			
}

void PostFlag_Task(void* pdata)
{
		INT8U err;
		INT8U posterr;
		OS_FLAGS flagrec;
	
		myFlagGrp = OSFlagCreate(0, &err);
		if( myFlagGrp == (OS_FLAG_GRP*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		
		while(1)
		{
				OSTimeDly((INT32U)pdata);
				flagrec = OSFlagPost(myFlagGrp, 0x02, OS_FLAG_SET, &posterr);
				if( 0x02 == flagrec )
				{
						GPIOB->ODR ^= GPIO_Pin_5;
				}
		}
}

void PendFlag_Task(void* pdata)
{
		INT8U err;
		OS_FLAGS flagrec;
		
		if( myFlagGrp == (OS_FLAG_GRP*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		
		while(1)
		{
				flagrec = OSFlagPend(myFlagGrp, 0x02, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, 0, &err);
				if( 0x02 == flagrec )
				{
						GPIOE->ODR ^= GPIO_Pin_5;
				}
		}
}

void LED0_Task(void* pdata)
{
		OS_MEM* pmyMem;
		INT8U err;
		INT8U geterr;
		INT8U puterr;
		INT8U myMem[3][20];
		void* pblk;
	
		//create a Mem
		pmyMem = OSMemCreate(myMem, 3, 20, &err);
	
		if( pmyMem == (OS_MEM*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
	
		while(1)
		{		
				//��myMem������һ���ڴ��ַ
				pblk = OSMemGet(pmyMem, &geterr);
				if( pblk == (void*)0 )
				{
						OSTaskDel(OS_PRIO_SELF);
						return;
				}
				
				if( geterr == OS_ERR_NONE )
				{
						//����ɹ���LED0��
						GPIOB->ODR ^= GPIO_Pin_5;
				}
				
				//��ʱ��׼���ͷŸ��ڴ��ַ
				OSTimeDly((INT32U)pdata);
			
				puterr = OSMemPut(pmyMem, pblk);
				
				if( puterr ==  OS_ERR_NONE )
				{
						//�ͷųɹ���LED1��
						GPIOE->ODR ^= GPIO_Pin_5;
				}
				else
				{
						OSTaskDel(OS_PRIO_SELF);
						return;
				}
				
//				GPIOB->ODR ^= GPIO_Pin_5;
//				OSTimeDly((INT32U)pdata);
		};
}

void LED1_Task(void* pdata)
{
		while(1)
		{
				GPIOE->ODR ^= GPIO_Pin_5;
				OSTimeDly((INT32U)pdata);
		};
}

void TaskStart(void* pdata)
{
		INT8U err;
		INT8U tmrerr;
		INT8U tmrserr;
		OS_EVENT* sem1;
		
		OSStatInit();
		
		//100ms+1s�����ڶ�Ϊ5�Ļ�����500ms
//		myTimer = OSTmrCreate(0, 10, OS_TMR_OPT_PERIODIC, (OS_TMR_CALLBACK)MyCallback, (void*)0, (INT8U*)"tmr1", &tmrerr);
//		OSTmrStart(myTimer, &tmrserr);

//		sem1 = OSSemCreate(0);
//		OSSemPend(sem1, 0, &err);
		
		//TFT�ĳ�ʼ��������������
		Lcd_Init();
		//��ʾ���������ڲ�����ˢ�£�ֻ��̬����ʾһ��ͼƬ
		QDTFT_Test_Demo();
	
		OSTaskDel(OS_PRIO_SELF);
}

void MyCallback (OS_TMR *ptmr, void *p_arg)
{
		GPIOB->ODR ^= GPIO_Pin_5;
}

//������������ɨ���������ȼ�Ϊ32
//ģ����ǵ绰�Ĳ�������������ɫ���Ǹ���
//�ö�ֵ�ź����������봮�ڷ����ж������ͬ��
//void KeyScan_Task(void* pdata)
//{
//		
//		//�����ź�����ʱ��
//		mySem = OSSemCreate(0);
//		
//		while(1)
//		{
//				//ɨ�������ݶ�200ms
//				OSTimeDly(20);
//				if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0)
//				{
//						continue;
//				}
//				if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 1)
//				{
//						GPIOB->ODR ^= GPIO_Pin_5;
//						OSSemPost(mySem);
//				}
//				while(1)
//				{
//						OSTimeDly(20);
//						if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0)
//						{
//								break;
//						}
//				}
//		}
//}

//�����������ɨ���������ȼ�Ϊ32
//ģ����ǵ绰�����ְ�����#��*��
//�ö�ֵ�ź����������봮�ڷ����ж������ͬ��
//��ʱ�����Ǽ��ٽ�����������д�����ٶȽ��������´��
void KeyScan_Task(void* pdata)
{
		//����ֻ�������������а���ֵ����
		int i = 0;
//		mySem = OSSemCreate(0);
	
		myMBox = OSMboxCreate((void*)0);
	
		while(1)
		{
				i = key();
				if( (i!=-1)&&(i!=12) )	//��ֵӦ�øĳ�-1�������ٸ�
				{
						GPIOB->ODR ^= GPIO_Pin_5;
//						DMA_TX_Buf[TXBuffer_Num] = i;	//���ܸ�ֵ����Ȼ�����룬��Ϊ���͵����ַ���ֵ
						if( TXBuffer_Num == 0 )
						{
								Lcd_Clear(0xffff);
								DMA_TX_Buf[0] = 'A';
								DMA_TX_Buf[1] = 'T';
								DMA_TX_Buf[2] = 'D';
						}
						//�ȸ����ڷ��ַ�
						DMA_TX_Buf[TXBuffer_Num+3] = Ascii_Num[i];	//��ֵ��������ҲҪ��
						
						//�����豸����������Ļ��֣�����Ӧ�ð���ʾ���񵥶�����һ������postһ��sem
						//������񲢲����еȴ��������Ե���������������
//						Gui_DrawFont_Num32((TXBuffer_Num%3)*40+3,32*(TXBuffer_Num/3),0xf800,0xEF7D,Num[i]);
						
						//�������������ʾ���������У�����Ҫ����ʾ������ߵ����ȼ�
						//������ʾ������ܴ�ϵ�ǰ�ļ�����ʾ����
						OSMboxPost(myMBox, &i);
						
						TXBuffer_Num++;
						if( TXBuffer_Num>= 11 )
						{
								DMA_TX_Buf[14] = ';';
								DMA_TX_Buf[15] = '\r';
								DMA_TX_Buf[16] = '\n';
								TXBuffer_Num = 0;
								//DMA_TX_BufҪ���㣿
//								GPIOB->ODR ^= GPIO_Pin_5;
							
								GPIOE->ODR ^= GPIO_Pin_5;
								//��Ӧ��post��Ӧ��ֱ�ӿ��ж�
								USART_ITConfig(USART1, USART_IT_TC, ENABLE);
							
//								OSSemPost(mySem);
						}
				}
				
				//��Ҫ����Ϣi���ݳ�ȥ��#����������������Ϊȡ�������
				//��������ֻ�ǰ���ʾ����ȡ�����ˣ����ڷ��ͻ�û��ȡ��������취
				if( 12 == i )
				{
						TXBuffer_Num = 0;
						QDTFT_Test_Demo();
				}
				
				OSTimeDly(10);	//ɨ�������ݶ�100ms
		}
}

//���ڷ����������ȼ�Ϊ40
//���������Ƶ������⣬�������ֻ�����˿��������ж���һ�����������˷���һ���ź���
//ʵ��Ӧ���ڰ�������򿪴����жϣ���������һ���ź�����ʹ��
//Ȼ���½�һ�������ڴ��ڷ��ź�������������������д�������
void USARTSend_Task(void* pdata)
{
		INT8U err;
		
		if( mySem == (OS_EVENT*)0 )
		{
				OSTaskDel(OS_PRIO_SELF);
				return;
		}
		
		while(1)
		{
				//���ڲ��Կ���ʹ��pend sem�ķ�ʽ����������ʵ�������ﲻֹҪ�жϰ����ź�
				//��Ҫ��sim900a�ķ�����Ϣ�����ж�
				//���緢��ATD+�绰���룬���ͳɹ���᷵��һ��OK��Ȼ�����ͨ��״̬
			
				//�����ʼ����sim900a��ͨѶ��ʱ�򣬵�Ƭ����sim900a����һ��ATD;ָ�
				//sim900a����Ҫ����һ��OK�Ժ󣬲���֤��sim900a�ɹ��뵥Ƭ������������
				OSSemPend(mySem, 0, &err);
				GPIOE->ODR ^= GPIO_Pin_5;
//				TXBuffer_Num = 0;			//���²�����Ժ����TXBuffer������ͨѶ��ʽ����ڲ��ף���ʱ������
				USART_ITConfig(USART1, USART_IT_TC, ENABLE);
		}
}

//TFT��ʾ��������
void TFTNumber_Task(void* pdata)
{
		INT8U err;
		int rec;
		
		while(1)
		{
				//����ʵ��Ӧ��ʱ������msgһֱ�ȴ�����OSMboxPend(myMBox, 0, &err)
				//��������ĵڶ�����������Ϊ0��Ӧ�ù涨һ������ĵȴ�ʱ��
				//���TFTNumber���ܵ�����ֻ�Լ��̵�����������Ӧ��ҲҪ����������ʾ��ˢ�¹���
				rec = *(int*)OSMboxPend(myMBox, 0, &err);
				if( err == OS_ERR_NONE )
				{
						if( rec != -1 )
						{
//								GPIOE->ODR ^= GPIO_Pin_5;
								if( TXBuffer_Num == 0 )
								{
										Gui_DrawFont_Num32((10%3)*40+3,(10/3)*32,0xf800,0xEF7D,rec);
										QDTFT_Test_Demo2();
								}
								else
								{
//										Gui_DrawFont_Num32(((TXBuffer_Num-1)%3)*40+3,((TXBuffer_Num-1)/3)*32,0xf800,0xEF7D,Num[rec]);
										Gui_DrawFont_Num32(((TXBuffer_Num-1)%3)*40+3,((TXBuffer_Num-1)/3)*32,0xf800,0xEF7D,rec);
//										Gui_DrawFont_Num32(((TXBuffer_Num-1)%3)*40+3,((TXBuffer_Num-1)/3)*32,0xf800,0xEF7D, 0);
								}
								
						}
				}
				
		}
}

void SysTick_Handler(void)
{
		OSIntEnter();
		OSTimeTick();  
		OSIntExit();  
}
