#ifndef __USART_H
#define __USART_H

#include "os_cpu.h"

#define DMA_Rec_Len 200
#define DMA_TX_Len 200

extern INT8U  DMA_TX_Buf[DMA_TX_Len];
extern INT8U TXBuffer_Num;
extern const INT8U Ascii_Num[12];

void uart_init(INT32U bound);
//void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx);
void Usart1_Send(INT8U *buf,INT8U len);

extern void Usart1_Send(INT8U *buf,INT8U len);

#endif
