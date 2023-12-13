#include "usbd_class_core.h"
#include "usbd_ioreq.h"
// #include "communication.h"



static void Handle_USBAsynchXfer  (void *pdev);


extern uint8_t USBD_DeviceDesc   [USB_SIZ_DEVICE_DESC];


__ALIGN_BEGIN uint8_t usbd_class_CfgDesc  [USB_CLASS_CONFIG_DESC_SIZ] __ALIGN_END ;


__ALIGN_BEGIN uint8_t usbd_class_OtherCfgDesc  [USB_CLASS_CONFIG_DESC_SIZ] __ALIGN_END ;


__ALIGN_BEGIN static __IO uint32_t  usbd_class_AltSet  __ALIGN_END = 0;


__ALIGN_BEGIN uint8_t USB_Rx_Buffer   [CLASS_DATA_MAX_PACKET_SIZE] __ALIGN_END ;

__ALIGN_BEGIN uint8_t APP_Rx_Buffer   [APP_RX_DATA_SIZE] __ALIGN_END ;


__ALIGN_BEGIN uint8_t CmdBuff[CLASS_CMD_PACKET_SZE] __ALIGN_END ;

uint32_t APP_Rx_ptr_in  = 0;
uint32_t APP_Rx_ptr_out = 0;
uint32_t APP_Rx_length  = 0;

uint8_t  USB_Tx_State = USB_CLASS_IDLE;

static uint32_t classCmd = 0xFF;
static uint32_t classLen = 0;



/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t usbd_class_CfgDesc[USB_CLASS_CONFIG_DESC_SIZ]  __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   								/* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,   	/* bDescriptorType: Configuration */
  USB_CLASS_CONFIG_DESC_SIZ,           	/* wTotalLength: no of returned bytes */
  0x00,
  0x01,   								/* bNumInterfaces: 1 interface */
  0x01,   								/* bConfigurationValue: Configuration value */
  0x00,   								/* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   								/* bmAttributes: self powered */
  250,   								/* MaxPower 500 mA (0x32 = 0 mA?) */

  /*---------------------------------------------------------------------------*/
  /*Interface Descriptor */

  0x09,   							/* bLength: Interface Descriptor size */
  USB_INTERFACE_DESCRIPTOR_TYPE,  	/* bDescriptorType: Interface */ /* Interface descriptor type */
  0x00,   							/* bInterfaceNumber: Number of Interface */
  0x00,   							/* bAlternateSetting: Alternate setting */
  0x04,   							/* bNumEndpoints */
  0xFF, 							/* bInterfaceClass: Vendor specific */
  0xFF, 							/* bInterfaceSubClass: Vendor specific */
  0xFF, 							/* bInterfaceProtocol: ? */
  0x00,   							/* iInterface: */

  /*---------------------------------------------------------------------------*/
  /*Endpoint OUT Descriptor*/

  0x07,   								/* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      	/* bDescriptorType: Endpoint */
  CLASS_OUT_EP,                        	/* bEndpointAddress */
  0x02,                              	/* bmAttributes: Bulk */
  LOBYTE(CLASS_DATA_MAX_PACKET_SIZE),  	/* wMaxPacketSize: */
  HIBYTE(CLASS_DATA_MAX_PACKET_SIZE),
  0,                              		/* bInterval: ignore for Bulk transfer */

  /*---------------------------------------------------------------------------*/
  /*Endpoint OUT2 Descriptor*/

  0x07,   								/* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      	/* bDescriptorType: Endpoint */
  CLASS_OUT_EP2,                        /* bEndpointAddress */
  0x02,                              	/* bmAttributes: Bulk */
  LOBYTE(CLASS_DATA_MAX_PACKET_SIZE),  	/* wMaxPacketSize: */
  HIBYTE(CLASS_DATA_MAX_PACKET_SIZE),
  0,                              		/* bInterval: ignore for Bulk transfer */

  /*---------------------------------------------------------------------------*/
  /*Endpoint IN Descriptor*/

  0x07,   								/* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      	/* bDescriptorType: Endpoint */
  CLASS_IN_EP,                         	/* bEndpointAddress */
  0x02,                              	/* bmAttributes: Bulk */
  LOBYTE(CLASS_DATA_MAX_PACKET_SIZE),  	/* wMaxPacketSize: */
  HIBYTE(CLASS_DATA_MAX_PACKET_SIZE),
  0,                               		/* bInterval: ignore for Bulk transfer */

  /*---------------------------------------------------------------------------*/
  /*Endpoint IN2 Descriptor*/

  0x07,   								/* bLength: Endpoint Descriptor size */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      	/* bDescriptorType: Endpoint */
  CLASS_IN_EP2,                         	/* bEndpointAddress */
  0x02,                              	/* bmAttributes: Bulk */
  LOBYTE(CLASS_DATA_MAX_PACKET_SIZE),  	/* wMaxPacketSize: */
  HIBYTE(CLASS_DATA_MAX_PACKET_SIZE),
  0                               		/* bInterval: ignore for Bulk transfer */
};




/* Initialize the CDC interface
  *  cfgidx: Configuration index
  * */
uint8_t  usbd_class_Init (void *pdev, uint8_t cfgidx)
{
  uint8_t *pbuf;

  DCD_EP_Open(pdev, CLASS_IN_EP, CLASS_DATA_IN_PACKET_SIZE, USB_OTG_EP_BULK); // EP IN 1
  DCD_EP_Open(pdev, CLASS_IN_EP2, CLASS_DATA_IN_PACKET_SIZE, USB_OTG_EP_BULK); // EP IN 2
  DCD_EP_Open(pdev, CLASS_OUT_EP, CLASS_DATA_OUT_PACKET_SIZE, USB_OTG_EP_BULK); // EPN OUT 1
  DCD_EP_Open(pdev, CLASS_OUT_EP2, CLASS_DATA_OUT_PACKET_SIZE, USB_OTG_EP_BULK); // EP OUT 2

  pbuf = (uint8_t *)USBD_DeviceDesc;
  pbuf[4] = DEVICE_CLASS_NR;
  pbuf[5] = DEVICE_SUBCLASS_NR;

  /* Prepare Out endpoints to receive next packet */
  DCD_EP_PrepareRx(pdev, CLASS_OUT_EP, (uint8_t*)(USB_Rx_Buffer), CLASS_DATA_OUT_PACKET_SIZE);
  DCD_EP_PrepareRx(pdev, CLASS_OUT_EP2, (uint8_t*)(USB_Rx_Buffer), CLASS_DATA_OUT_PACKET_SIZE);

  return USBD_OK;
}

uint8_t  usbd_class_DeInit (void *pdev, uint8_t cfgidx)
{
  DCD_EP_Close(pdev, CLASS_IN_EP);
  DCD_EP_Close(pdev, CLASS_IN_EP2);
  DCD_EP_Close(pdev, CLASS_OUT_EP);
  DCD_EP_Close(pdev, CLASS_OUT_EP2);
  return USBD_OK;
}

uint8_t  usbd_class_Setup (void  *pdev, USB_SETUP_REQ *req)
{
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  /* Vendor Requests --------------------------------*/
  case USB_REQ_TYPE_VENDOR :
	  switch (req->bRequest)
	  {
	  	  case MS_VENDOR_CODE:
	  		USBD_StdDevReq (pdev, req);
			return USBD_OK;

	      default:
			USBD_CtlError (pdev, req);
			return USBD_FAIL;
	  }
      return USBD_OK;
    /* Class Requests -------------------------------*/
  case USB_REQ_TYPE_CLASS :

      /* Check if the request is a data setup packet */
      if (req->wLength)
      {
        /* Check if the request is Device-to-Host */
        if (req->bmRequest & 0x80)
        {
          /* Send the data to the host */
          USBD_CtlSendData (pdev, CmdBuff, req->wLength);
        }
        else /* Host-to-Device request */
        {
          /* Set the value of the current command to be processed */
          classCmd = req->bRequest;
          classLen = req->wLength;

          /* Prepare the reception of the buffer over EP0
          Next step: the received data will be managed in usbd_class_EP0_TxSent()
          function. */
          USBD_CtlPrepareRx (pdev, CmdBuff, req->wLength);
        }
      }

      return USBD_OK;

    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL;

    /* Standard Requests -------------------------------*/
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
      USBD_CtlError(pdev, req);
      return USBD_FAIL;

    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData(pdev, (uint8_t *)&usbd_class_AltSet, 1);
      break;

    case USB_REQ_SET_INTERFACE :
      if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
      {
        usbd_class_AltSet = (uint8_t)(req->wValue);
      }
      else
      {
        /* Call the error management function (command will be nacked */
        USBD_CtlError (pdev, req);
      }
      break;
    }
  }
  return USBD_OK;
}

/* Data received on control endpoint */
uint8_t  usbd_class_EP0_RxReady (void  *pdev)
{
  /* Reset the command variable to default value */
  classCmd = 0; //NO_CMD;
  return USBD_OK;
}


/* Data sent on non-control IN endpoint */
uint8_t  usbd_class_DataIn (void *pdev, uint8_t epnum)
{
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;

  if (USB_Tx_State == USB_CLASS_BUSY)
  {
    if (APP_Rx_length == 0)
    {
      USB_Tx_State = USB_CLASS_IDLE;
    }
    else
    {
      if (APP_Rx_length > CLASS_DATA_IN_PACKET_SIZE){
        USB_Tx_ptr = APP_Rx_ptr_out;
        USB_Tx_length = CLASS_DATA_IN_PACKET_SIZE;

        APP_Rx_ptr_out += CLASS_DATA_IN_PACKET_SIZE;
        APP_Rx_length -= CLASS_DATA_IN_PACKET_SIZE;
      }
      else
      {
        USB_Tx_ptr = APP_Rx_ptr_out;
        USB_Tx_length = APP_Rx_length;

        APP_Rx_ptr_out += APP_Rx_length;
        APP_Rx_length = 0;
        if(USB_Tx_length == CLASS_DATA_IN_PACKET_SIZE)
        {
          USB_Tx_State = USB_CLASS_ZLP;
        }
      }

      /* Prepare the available data buffer to be sent on IN endpoint */
      DCD_EP_Tx (pdev,
                 CLASS_IN_EP,
                 (uint8_t*)&APP_Rx_Buffer[USB_Tx_ptr],
                 USB_Tx_length);
      return USBD_OK;
    }
  }

  /* Avoid any asynchronous transfer during ZLP */
  if (USB_Tx_State == USB_CLASS_ZLP)
  {
    /*Send ZLP to indicate the end of the current transfer */
    DCD_EP_Tx (pdev,
               CLASS_IN_EP,
               NULL,
               0);

    USB_Tx_State = USB_CLASS_IDLE;
  }
  return USBD_OK;
}

/* Data received on non-control Out endpoint */
uint8_t  usbd_class_DataOut (void *pdev, uint8_t epnum)
{
  uint16_t USB_Rx_Cnt;

  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;

  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the application Xfer */
  process_received_data((char *)USB_Rx_Buffer, USB_Rx_Cnt);

  /* Prepare Out endpoint to receive next packet */
  if(epnum == 2){
	  DCD_EP_PrepareRx(pdev, CLASS_OUT_EP2, (uint8_t*)(USB_Rx_Buffer), CLASS_DATA_OUT_PACKET_SIZE);
  }
  else{
	  DCD_EP_PrepareRx(pdev, CLASS_OUT_EP, (uint8_t*)(USB_Rx_Buffer), CLASS_DATA_OUT_PACKET_SIZE);
  }

  return USBD_OK;
}

uint8_t  usbd_class_SOF (void *pdev)
{
  static uint32_t FrameCount = 0;

  if (FrameCount++ == CLASS_IN_FRAME_INTERVAL)
  {
    /* Reset the frame counter */
    FrameCount = 0;

    /* Check the data to be sent through IN pipe */
    Handle_USBAsynchXfer(pdev);
  }

  return USBD_OK;
}


static void Handle_USBAsynchXfer (void *pdev)
{
	uint16_t USB_Tx_ptr;
	uint16_t USB_Tx_length;

	if(USB_Tx_State == USB_CLASS_IDLE)
	{
		/* If out-pointer points to last element, that roll it back */
		if (APP_Rx_ptr_out == APP_RX_DATA_SIZE)
		{
			APP_Rx_ptr_out = 0;
		}

		/* If pointers point to same element, all bytes were sent already (or overflow occurred) */
		if(APP_Rx_ptr_out == APP_Rx_ptr_in)
		{
			USB_Tx_State = USB_CLASS_IDLE;
			return;
		}

		/* Calculate the number of bytes that were not send yet
		 * (and which one should send now) */
		if(APP_Rx_ptr_out > APP_Rx_ptr_in) // rollback of the in-pointer (and no rollback of the out-pointer) occurred
		{
			APP_Rx_length = APP_RX_DATA_SIZE - APP_Rx_ptr_out;
		}
		else
		{
			APP_Rx_length = APP_Rx_ptr_in - APP_Rx_ptr_out;
		}
		/* If the number of bytes to be sent is larger than CLASS_DATA_IN_PACKET_SIZE (typically = 64)
		 * than sent a part of the bytes this time and the other part(s) the next time */
		if (APP_Rx_length > CLASS_DATA_IN_PACKET_SIZE)
		{
			USB_Tx_ptr = APP_Rx_ptr_out;
			USB_Tx_length = CLASS_DATA_IN_PACKET_SIZE;

			APP_Rx_ptr_out += CLASS_DATA_IN_PACKET_SIZE;
			APP_Rx_length -= CLASS_DATA_IN_PACKET_SIZE;
			USB_Tx_State = USB_CLASS_BUSY;
		}
		else /* if number of bytes to be sent is smaller than or equal to CLASS_DATA_IN_PACKET_SIZE (=64) */
		{
			USB_Tx_ptr = APP_Rx_ptr_out;
			USB_Tx_length = APP_Rx_length;

			APP_Rx_ptr_out += APP_Rx_length;
			APP_Rx_length = 0;
			if(USB_Tx_length == CLASS_DATA_IN_PACKET_SIZE)
			{
				USB_Tx_State = USB_CLASS_ZLP;
			}
			else
			{
				USB_Tx_State = USB_CLASS_BUSY;
			}
		}
		DCD_EP_Tx(pdev, CLASS_IN_EP, (uint8_t*)&APP_Rx_Buffer[USB_Tx_ptr], USB_Tx_length);
	}
}

uint8_t  *USBD_class_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_class_CfgDesc);
  return usbd_class_CfgDesc;
}
