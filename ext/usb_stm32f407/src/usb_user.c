/* Notes: */
// Pins A9, A10, A11, A12 are used for USB OTG FS


/*
*************************************************************************************************
* Includes
*************************************************************************************************
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "stm32f4xx_gpio.h"

// #include "usbd_cdc_core.h"
#include "usb_conf.h"
#include "usbd_desc.h"
// #include "usbd_cdc_vcp.h"

// #include "std_function_lib.h"
// #include "communication.h"
#include "usb_user.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "usbd_class_core.h"
#include "usb_dcd_int.h"
#include "usbd_conf.h"
#include "usbd_class_core.h"


#define GPIO_PORT_USB_DP		GPIOA
#define GPIO_PIN_USB_DP			GPIO_Pin_12
#define GPIO_AFPIN_USB_DP		GPIO_PinSource12

#define GPIO_PORT_USB_DM		GPIOA
#define GPIO_PIN_USB_DM			GPIO_Pin_11
#define GPIO_AFPIN_USB_DM		GPIO_PinSource11

#define GPIO_PORT_USB_VBUS_FS		GPIOA
#define GPIO_PIN_USB_VBUS_FS			GPIO_Pin_9

#define GPIO_PORT_USB_OTG_FS_ID		GPIOA
#define GPIO_PIN_USB_OTG_FS_ID			GPIO_Pin_10
#define GPIO_AFPIN_USB_OTG_FS_ID		GPIO_PinSource10


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

uint8_t send_data_USB(uint8_t * txBuff_aui8, uint16_t numBytesTotal)
{	
	uint8_t success = 0;

	if(USB_Tx_State == USB_CLASS_IDLE)
	{
		if(numBytesTotal > USB_BUFF_SIZE_IN_BYTES) numBytesTotal = USB_BUFF_SIZE_IN_BYTES;

		if(APP_Rx_ptr_in + numBytesTotal > APP_RX_DATA_SIZE){
			memcpy(&APP_Rx_Buffer[APP_Rx_ptr_in], txBuff_aui8, (APP_RX_DATA_SIZE-APP_Rx_ptr_in)*sizeof(uint8_t));
			memcpy(&APP_Rx_Buffer[0], &txBuff_aui8[APP_RX_DATA_SIZE-APP_Rx_ptr_in], (numBytesTotal-(APP_RX_DATA_SIZE-APP_Rx_ptr_in))*sizeof(uint8_t));
			APP_Rx_ptr_in = numBytesTotal-(APP_RX_DATA_SIZE-APP_Rx_ptr_in);
		}
		else{
			memcpy(&APP_Rx_Buffer[APP_Rx_ptr_in], txBuff_aui8, numBytesTotal*sizeof(uint8_t));
			APP_Rx_ptr_in += numBytesTotal;
		}

		success = 1;
	}
	return success;
}

void init_GPIO_USB()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_USB_DM;
  GPIO_Init(GPIO_PORT_USB_DM, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN_USB_DP;
  GPIO_Init(GPIO_PORT_USB_DP, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIO_PORT_USB_DM,GPIO_AFPIN_USB_DM,GPIO_AF_OTG_FS);
  GPIO_PinAFConfig(GPIO_PORT_USB_DP,GPIO_AFPIN_USB_DP,GPIO_AF_OTG_FS);

//   /* Configure VBUS Pin */
//   GPIO_InitStructure.GPIO_Pin = GPIO_PIN_USB_VBUS_FS;
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//   GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//   GPIO_Init(GPIO_PORT_USB_VBUS_FS, &GPIO_InitStructure);

//   /* Configure ID pin */
//   GPIO_InitStructure.GPIO_Pin =  GPIO_PIN_USB_OTG_FS_ID;
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//   GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//   GPIO_Init(GPIO_PORT_USB_OTG_FS_ID, &GPIO_InitStructure);
//   GPIO_PinAFConfig(GPIO_PORT_USB_OTG_FS_ID, GPIO_AFPIN_USB_OTG_FS_ID, GPIO_AF_OTG2_FS);

//   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
//   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, ENABLE);
}
