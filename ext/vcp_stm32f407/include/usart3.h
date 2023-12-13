#ifndef __USART3_H__
#define __USART3_H__

#include "typedef.h"

#define N_DATA_VALS_USART3 4
extern volatile tF32 txBuff_USART3_aui8[N_DATA_VALS_USART3];


// GPIO B
#define GPIO_PORT_USART3_TX		GPIOB
#define GPIO_PIN_USART3_TX		GPIO_Pin_10
#define GPIO_AFPIN_USART3_TX    GPIO_PinSource10

#define GPIO_PORT_USART3_RX		GPIOB
#define GPIO_PIN_USART3_RX		GPIO_Pin_11
#define GPIO_AFPIN_USART3_RX    GPIO_PinSource11

void init_GPIO_USART3();

void init_USART3();

void send_USART3();
void read_USART3();
bool try_send_data_USART3(tUI8 * data, tUI32 NBytes);

#endif /* __USART3_H__ */


