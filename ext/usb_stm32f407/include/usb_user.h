#ifndef __USB_H__
#define __USB_H__

#include "usbd_conf.h"
#include "communication.h"
#include "tasks.h"
#include "gpio.h"
#include "typedef.h"

/*
*************************************************************************************************
* Defines
*************************************************************************************************
*/

#define USB_BUFF_SIZE_IN_BYTES TX_BYTES_PER_MSG


#if USB_BUFF_SIZE_IN_BYTES > APP_RX_DATA_SIZE
#error USB_BUFF_SIZE must be smaller than or equal to  APP_RX_DATA_SIZE
#endif


/*
*************************************************************************************************
* Variables for export
*************************************************************************************************
*/
extern int16_t usbDevAddress;

/*
*************************************************************************************************
* Function declarations for export
*************************************************************************************************
*/

void init_usb(void);
void reinit_usb(void);
bool try_send_data_USB(tUI8 * txBuff_aui8, uint16_t numBytesData);


#endif /* __USB_H__ */
