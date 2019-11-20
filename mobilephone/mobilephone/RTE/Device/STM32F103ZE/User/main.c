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


//创建两个任务，LED交替闪烁
//LED0延时200ms，LED1延时500ms

//LED0任务相关
#define LED0_Task_Prio 5
#define LED0_STK_Size 64
OS_STK LED0_Task_STK[LED0_STK_Size];
void LED0_Task(void *pdata);

//LED1任务相关
#define LED1_Task_Prio 6
#define LED1_STK_Size 64
OS_STK LED1_Task_STK[LED1_STK_Size];
void LED1_Task(void *pdata);

//TaskStart相关
#define TaskStart_Prio 1
#define TaskStart_Size 64
OS_STK TaskStart_STK[TaskStart_Size];
void TaskStart(void* pdata);

//定义一个消息队列
OS_EVENT* myQ;

//定义一个邮箱
OS_EVENT* myMBox;

//定义一个信号
OS_EVENT* mySem;

//定义一个软件定时器
OS_TMR* myTimer;

//定义一个事件标志组
OS_FLAG_GRP* myFlagGrp;

//定义一个内存管理变量
//内存管理变量要局部定义
//方便释放
//OS_MEM* myMem;

//PostQ任务相关
#define PostQ_Task_Prio 2
#define PostQ_STK_Size 64
OS_STK PostQ_Task_STK[PostQ_STK_Size];
void PostQ_Task(void* pdata);

//PendQ任务相关
#define PendQ_Task_Prio 3
#define PendQ_STK_Size 64
OS_STK PendQ_Task_STK[PostQ_STK_Size];
void PendQ_Task(void* pdata);

//PostMsg任务相关
#define PostMsg_Task_Prio 7
#define PostMsg_STK_Size 64
OS_STK PostMsg_Task_STK[PostMsg_STK_Size];
void PostMsg_Task(void* pdata);

//PendMsg任务相关
#define PendMsg_Task_Prio 8
#define PendMsg_STK_Size 64
OS_STK PendMsg_Task_STK[PendMsg_STK_Size];
void PendMsg_Task(void* pdata);

//PostSem任务相关
#define PostSem_Task_Prio 9
#define PostSem_STK_Size 64
OS_STK PostSem_Task_STK[PostSem_STK_Size];
void PostSem_Task(void* pdata);

//PendSem任务相关
#define PendSem_Task_Prio 10
#define PendSem_STK_Size 64
OS_STK PendSem_Task_STK[PendSem_STK_Size];
void PendSem_Task(void* pdata);

//PostFlag任务相关
#define PostFlag_Task_Prio 11
#define PostFlag_STK_Size 64
OS_STK PostFlag_Task_STK[PostFlag_STK_Size];
void PostFlag_Task(void* pdata);

//PendFlag任务相关
#define PendFlag_Task_Prio 12
#define PendFlag_STK_Size 64
OS_STK PendFlag_Task_STK[PendFlag_STK_Size];
void PendFlag_Task(void* pdata);

//keyboard scan task
//先给键盘随便搞一个优先级，不频繁扫描的任务优先级较低
#define KeyScan_Task_Prio 32 
#define KeyScan_STK_Size 64
OS_STK KeyScan_Task_STK[KeyScan_STK_Size];
void KeyScan_Task(void* pdata);

//USART_ISR send task
//上游任务优先级应该大于下游任务的优先级
//对于串口发送任务来说，键盘扫描任务相当于它的上游任务
//所以键盘扫描任务的优先级要高于串口发送任务
#define USARTSend_Task_Prio 40 
#define USARTSend_STK_Size 64
OS_STK USARTSend_Task_STK[USARTSend_STK_Size];
void USARTSend_Task(void* pdata);

//TFTNumber_Task
//TFTNumber_Task是keyscan的下游任务，应该给较低的优先级
//又因为串口发送任务是拨打电话的任务，比显示任务要重要的多
//所以串口发送任务的优先级要高于显示任务
#define TFTNumber_Task_Prio 44
#define TFTNumber_STK_Size 64
OS_STK TFTNumber_Task_STK[TFTNumber_STK_Size];
void TFTNumber_Task(void* pdata);




//软件定时器1回调函数
void MyCallback (OS_TMR *ptmr, void *p_arg);

//void delay_ms(u16 time)
//{    
//   u16 i=0;  
//   while(time--)
//   {
//      i=12000;  //自己定义
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
		
//		采用的是按键然后打开串口发送中断，然后在串口发送中断中将数据发送出去
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

//		采用的是查询的方式来进行串口发送
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
				//在myMem里申请一个内存地址
				pblk = OSMemGet(pmyMem, &geterr);
				if( pblk == (void*)0 )
				{
						OSTaskDel(OS_PRIO_SELF);
						return;
				}
				
				if( geterr == OS_ERR_NONE )
				{
						//申请成功，LED0闪
						GPIOB->ODR ^= GPIO_Pin_5;
				}
				
				//延时后准备释放该内存地址
				OSTimeDly((INT32U)pdata);
			
				puterr = OSMemPut(pmyMem, pblk);
				
				if( puterr ==  OS_ERR_NONE )
				{
						//释放成功，LED1闪
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
		
		//100ms+1s，周期定为5的话就是500ms
//		myTimer = OSTmrCreate(0, 10, OS_TMR_OPT_PERIODIC, (OS_TMR_CALLBACK)MyCallback, (void*)0, (INT8U*)"tmr1", &tmrerr);
//		OSTmrStart(myTimer, &tmrserr);

//		sem1 = OSSemCreate(0);
//		OSSemPend(sem1, 0, &err);
		
		//TFT的初始化函数放在这里
		Lcd_Init();
		//显示函数，现在不进行刷新，只静态的显示一张图片
		QDTFT_Test_Demo();
	
		OSTaskDel(OS_PRIO_SELF);
}

void MyCallback (OS_TMR *ptmr, void *p_arg)
{
		GPIOB->ODR ^= GPIO_Pin_5;
}

//独立键盘周期扫描任务，优先级为32
//模拟的是电话的拨出键，就是绿色的那个键
//用二值信号量来进行与串口发送中断任务的同步
//void KeyScan_Task(void* pdata)
//{
//		
//		//创建信号量的时候
//		mySem = OSSemCreate(0);
//		
//		while(1)
//		{
//				//扫描周期暂定200ms
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

//矩阵键盘周期扫描任务，优先级为32
//模拟的是电话的数字按键和#与*键
//用二值信号量来进行与串口发送中断任务的同步
//暂时不考虑加临界区，给串口写数据速度较慢，不怕打断
void KeyScan_Task(void* pdata)
{
		//这里只能用邮箱来进行按键值传递
		int i = 0;
//		mySem = OSSemCreate(0);
	
		myMBox = OSMboxCreate((void*)0);
	
		while(1)
		{
				i = key();
				if( (i!=-1)&&(i!=12) )	//键值应该改成-1，明早再改
				{
						GPIOB->ODR ^= GPIO_Pin_5;
//						DMA_TX_Buf[TXBuffer_Num] = i;	//不能赋值，不然会乱码，因为发送的是字符的值
						if( TXBuffer_Num == 0 )
						{
								Lcd_Clear(0xffff);
								DMA_TX_Buf[0] = 'A';
								DMA_TX_Buf[1] = 'T';
								DMA_TX_Buf[2] = 'D';
						}
						//先给串口发字符
						DMA_TX_Buf[TXBuffer_Num+3] = Ascii_Num[i];	//键值改了这里也要改
						
						//按照设备依赖性任务的划分，还是应该把显示任务单独安排一个任务，post一个sem
						//这个任务并不进行等待和周期性调动，先这样处理
//						Gui_DrawFont_Num32((TXBuffer_Num%3)*40+3,32*(TXBuffer_Num/3),0xf800,0xEF7D,Num[i]);
						
						//这里如果想让显示任务先运行，就需要给显示任务更高的优先级
						//这样显示任务才能打断当前的键盘显示任务
						OSMboxPost(myMBox, &i);
						
						TXBuffer_Num++;
						if( TXBuffer_Num>= 11 )
						{
								DMA_TX_Buf[14] = ';';
								DMA_TX_Buf[15] = '\r';
								DMA_TX_Buf[16] = '\n';
								TXBuffer_Num = 0;
								//DMA_TX_Buf要清零？
//								GPIOB->ODR ^= GPIO_Pin_5;
							
								GPIOE->ODR ^= GPIO_Pin_5;
								//不应该post，应该直接开中断
								USART_ITConfig(USART1, USART_IT_TC, ENABLE);
							
//								OSSemPost(mySem);
						}
				}
				
				//需要把消息i传递出去，#键我们现在用来作为取消拨打键
				//但是现在只是把显示功能取消掉了，串口发送还没能取消，再想办法
				if( 12 == i )
				{
						TXBuffer_Num = 0;
						QDTFT_Test_Demo();
				}
				
				OSTimeDly(10);	//扫描周期暂定100ms
		}
}

//串口发送任务，优先级为40
//这个任务设计的有问题，这个任务只进行了开启串口中断这一个操作，还浪费了一个信号量
//实际应该在按键那里打开串口中断，这样减少一个信号量的使用
//然后新建一个任务，在串口发信号量，在主程序的任务中处理数据
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
				//现在测试可以使用pend sem的方式来做，但是实际上这里不止要判断按键信号
				//还要对sim900a的返回信息进行判断
				//例如发送ATD+电话号码，发送成功后会返回一个OK，然后进入通话状态
			
				//比如最开始建立sim900a的通讯的时候，单片机给sim900a发送一个ATD;指令，
				//sim900a必须要返回一个OK以后，才能证明sim900a成功与单片机进行了链接
				OSSemPend(mySem, 0, &err);
				GPIOE->ODR ^= GPIO_Pin_5;
//				TXBuffer_Num = 0;			//按下拨打键以后清空TXBuffer，这种通讯方式或存在不妥，暂时先这样
				USART_ITConfig(USART1, USART_IT_TC, ENABLE);
		}
}

//TFT显示任务数字
void TFTNumber_Task(void* pdata)
{
		INT8U err;
		int rec;
		
		while(1)
		{
				//这里实际应用时不能让msg一直等待，即OSMboxPend(myMBox, 0, &err)
				//这个函数的第二个参数不能为0，应该规定一个具体的等待时间
				//这个TFTNumber不能单纯的只对键盘的数字做出反应，也要进行所有显示的刷新工作
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
