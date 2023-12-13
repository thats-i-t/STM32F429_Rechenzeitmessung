/* Notes: */
// Pins A9, A10, A11, A12 are used for USB OTG FS


/*
*************************************************************************************************
* Includes
*************************************************************************************************
*/

#include <string.h>
#include <stdlib.h>

// #include "usbd_cdc_core.h"
#include "usb_conf.h"
#include "usbd_desc.h"
// #include "usbd_cdc_vcp.h"

#include "std_function_lib.h"
#include "communication.h"
#include "usb.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "usbd_class_core.h"
#include "usb_dcd_int.h"
#include "usbd_conf.h"
#include "usbd_class_core.h"
#include "typedef.h"

extern uint8_t  APP_Rx_Buffer []; /* Write CDC received data in this buffer. These data will be sent over USB IN endpoint in the CDC core functions. */
extern uint32_t APP_Rx_ptr_in;    /* Increment this pointer or roll it back to start address when writing received data in the buffer APP_Rx_Buffer. */


uint16_t usb_send_mode = 5;
int16_t usbDevAddress = -1;

__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev __ALIGN_END;


void OTG_FS_IRQHandler(void)
{
	USBD_OTG_ISR_Handler(&USB_OTG_dev);
}

void init_usb(){
	USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc);
}

void reinit_usb(){
	USB_OTG_dev.dev.connection_status = 0;   
	init_usb();
}

bool try_send_data_USB(tUI8 * txBuff_aui8, uint16_t numBytesTotal)
{	
	bool success = FALSE;

	if(USB_Tx_State == USB_CLASS_IDLE)
	{
		if(numBytesTotal > USB_BUFF_SIZE_IN_BYTES) numBytesTotal = USB_BUFF_SIZE_IN_BYTES;

		if(APP_Rx_ptr_in + numBytesTotal > APP_RX_DATA_SIZE){
			memcpy(&APP_Rx_Buffer[APP_Rx_ptr_in], txBuff_aui8, (APP_RX_DATA_SIZE-APP_Rx_ptr_in)*sizeof(tUI8));
			memcpy(&APP_Rx_Buffer[0], &txBuff_aui8[APP_RX_DATA_SIZE-APP_Rx_ptr_in], (numBytesTotal-(APP_RX_DATA_SIZE-APP_Rx_ptr_in))*sizeof(tUI8));
			APP_Rx_ptr_in = numBytesTotal-(APP_RX_DATA_SIZE-APP_Rx_ptr_in);
		}
		else{
			memcpy(&APP_Rx_Buffer[APP_Rx_ptr_in], txBuff_aui8, numBytesTotal*sizeof(tUI8));
			APP_Rx_ptr_in += numBytesTotal;
		}
		
		success = TRUE;
	}
	return success;
}