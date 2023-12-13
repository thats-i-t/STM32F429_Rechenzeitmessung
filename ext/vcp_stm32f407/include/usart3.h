#ifndef __USART3_H__
#define __USART3_H__

#include <stdint.h>

#define N_DATA_VALS_USART3 4
extern volatile uint8_t txBuff_USART3_aui8[N_DATA_VALS_USART3];


void init_GPIO_USART3();

void init_USART3();

void send_USART3();
void read_USART3();
uint8_t try_send_data_USART3(uint8_t * data, uint32_t NBytes);

#endif /* __USART3_H__ */


