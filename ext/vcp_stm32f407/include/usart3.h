#ifndef __USART3_H__
#define __USART3_H__

#include <stdint.h>

#include "usb.h"

#define N_DATA_VALS_TX_USART3 LEN_TX_FRAME
#define TX_BYTES_PER_MSG N_DATA_VALS_TX_USART3 // a.t.m. it will send always the whole buffer

#define DMA_USART3_PRIO 5
#define DMA_USART3_SUBPRIO 5

extern volatile uint8_t txBuff_USART3_aui8[N_DATA_VALS_TX_USART3];


void init_GPIO_USART3();

void init_USART3();

void send_USART3();
void read_USART3();
uint8_t try_send_data_USART3(uint8_t * data, uint32_t NBytes);

#endif /* __USART3_H__ */


