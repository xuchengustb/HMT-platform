#ifndef __UART1_H_
#define __UART1_H_

#include "stm32f10x.h"


// USART Receiver buffer
#define RX_BUFFER_SIZE   64
#define TX_BUFFER_SIZE   32
extern unsigned char rx_buffer[RX_BUFFER_SIZE];
extern unsigned char tx_buffer[TX_BUFFER_SIZE];


typedef struct 
{
  uint16_t volatile Wd_Indx;
  uint16_t volatile Rd_Indx;
  uint16_t Mask;
  uint8_t *pbuf;
}UartBuf;



void UART1NVIC_Configuration(void);
void UART1_init(u32 pclk2,u32 bound);
void UART1_Put_Char(unsigned char DataToSend);
uint8_t Uart1_Put_Char(unsigned char DataToSend);


extern UartBuf UartTxbuf;
extern UartBuf UartRxbuf;
extern uint8_t UartBuf_RD(UartBuf *Ringbuf);
extern uint16_t UartBuf_Cnt(UartBuf *Ringbuf);
extern void UartBuf_WD(UartBuf *Ringbuf,uint8_t DataIn);
#endif


