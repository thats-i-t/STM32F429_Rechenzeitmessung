#include "usbd_req.h"
#include "usbd_ioreq.h"
#include "usbd_desc.h"
#include "usbd_class_core.h"

#include "usb.h"

extern __IO USB_OTG_DCTL_TypeDef SET_TEST_MODE;

__ALIGN_BEGIN uint32_t USBD_ep_status __ALIGN_END  = 0;
__ALIGN_BEGIN uint32_t  USBD_default_cfg __ALIGN_END  = 0;
__ALIGN_BEGIN uint32_t  USBD_cfg_status __ALIGN_END  = 0;

extern uint32_t APP_Rx_ptr_in;
extern uint32_t APP_Rx_ptr_out;


static void USBD_GetDescriptor(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_GetCompatIdDescriptor(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_GetPropertiesOsDescriptor(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_SetAddress(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_SetConfig(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_GetConfig(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_GetStatus(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_SetFeature(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_ClrFeature(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static uint8_t USBD_GetLen(uint8_t *buf);


USBD_Status  USBD_StdDevReq (USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req)
{
  USBD_Status ret = USBD_OK;

  if(req->bmRequest == 0xC0 && req->bRequest == MS_VENDOR_CODE){
	  USBD_GetCompatIdDescriptor(pdev, req);
	  return ret;
  }
  if(req->bmRequest == 0xC1 && req->bRequest == MS_VENDOR_CODE){
	  USBD_GetPropertiesOsDescriptor(pdev, req);
	  return ret;
  }
  if(req->bmRequest == 0x40 && req->bRequest == 11){
      USBD_CtlSendData (pdev,NULL,0); // send ACK = empty packet
	  return ret;
  }

  switch (req->bRequest)
  {
  case USB_REQ_GET_DESCRIPTOR:

    USBD_GetDescriptor (pdev, req) ;
    break;

  case USB_REQ_SET_ADDRESS:
    // usbDevAddress = req->wValue;
    USBD_SetAddress(pdev, req);
    break;

  case USB_REQ_SET_CONFIGURATION:
    USBD_SetConfig (pdev , req);
    break;

  case USB_REQ_GET_CONFIGURATION:
    USBD_GetConfig (pdev , req);
    break;

  case USB_REQ_GET_STATUS:
    USBD_GetStatus (pdev , req);
    break;


  case USB_REQ_SET_FEATURE:
    USBD_SetFeature (pdev , req);
    break;

  case USB_REQ_CLEAR_FEATURE:
    USBD_ClrFeature (pdev , req);
    break;

  default:
    USBD_CtlError(pdev , req);
    break;
  }

  return ret;
}

USBD_Status  USBD_StdItfReq (USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req)
{
  USBD_Status ret = USBD_OK;

  switch (pdev->dev.device_status)
  {
  case USB_OTG_CONFIGURED:

    if (LOBYTE(req->wIndex) <= USBD_ITF_MAX_NUM)
    {
      usbd_class_Setup (pdev, req);

      if((req->wLength == 0)&& (ret == USBD_OK))
      {
         USBD_CtlSendStatus(pdev);
      }
    }
    else
    {
       USBD_CtlError(pdev , req);
    }
    break;

  default:
     USBD_CtlError(pdev , req);
    break;
  }
  return ret;
}

USBD_Status  USBD_StdEPReq (USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req)
{

  uint8_t   ep_addr;
  USBD_Status ret = USBD_OK;

  ep_addr  = LOBYTE(req->wIndex);

  /* Check the class specific requests before going to standard request */
  if ((req->bmRequest & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_CLASS)
  {
    usbd_class_Setup(pdev, req);
    return ret;
  }

  switch (req->bRequest)
  {
  case USB_REQ_SET_FEATURE :

    switch (pdev->dev.device_status)
    {
    case USB_OTG_ADDRESSED:
      if ((ep_addr != 0x00) && (ep_addr != 0x80))
      {
        DCD_EP_Stall(pdev , ep_addr);
      }
      break;

    case USB_OTG_CONFIGURED:
      if (req->wValue == USB_FEATURE_EP_HALT)
      {
        if ((ep_addr != 0x00) && (ep_addr != 0x80))
        {
          DCD_EP_Stall(pdev , ep_addr);

        }
      }
      usbd_class_Setup(pdev, req);
      USBD_CtlSendStatus(pdev);

      break;

    default:
      USBD_CtlError(pdev , req);
      break;
    }
    break;

  case USB_REQ_CLEAR_FEATURE :

    switch (pdev->dev.device_status)
    {
    case USB_OTG_ADDRESSED:
      if ((ep_addr != 0x00) && (ep_addr != 0x80))
      {
        DCD_EP_Stall(pdev , ep_addr);
      }
      break;

    case USB_OTG_CONFIGURED:
      if (req->wValue == USB_FEATURE_EP_HALT)
      {
        if ((ep_addr != 0x00) && (ep_addr != 0x80))
        {
          DCD_EP_ClrStall(pdev , ep_addr);
          usbd_class_Setup(pdev, req);
        }
        USBD_CtlSendStatus(pdev);
      }
      break;

    default:
       USBD_CtlError(pdev , req);
      break;
    }
    break;

  case USB_REQ_GET_STATUS:
    switch (pdev->dev.device_status)
    {
    case USB_OTG_ADDRESSED:
      if ((ep_addr != 0x00) && (ep_addr != 0x80))
      {
        DCD_EP_Stall(pdev , ep_addr);
      }
      break;

    case USB_OTG_CONFIGURED:


      if ((ep_addr & 0x80)== 0x80)
      {
        if(pdev->dev.in_ep[ep_addr & 0x7F].is_stall)
        {
          USBD_ep_status = 0x0001;
        }
        else
        {
          USBD_ep_status = 0x0000;
        }
      }
      else if ((ep_addr & 0x80)== 0x00)
      {
        if(pdev->dev.out_ep[ep_addr].is_stall)
        {
          USBD_ep_status = 0x0001;
        }

        else
        {
          USBD_ep_status = 0x0000;
        }
      }

      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_ep_status,
                        2);
      break;

    default:
       USBD_CtlError(pdev , req);
      break;
    }
    break;

  default:
    break;
  }
  return ret;
}

static void USBD_GetDescriptor(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req)
{
  uint16_t len;
  uint8_t *pbuf;
  len = req->wLength ;

  switch (req->wValue >> 8)
  {
  case USB_DESC_TYPE_DEVICE:
    pbuf = USBD_USR_DeviceDescriptor(pdev->cfg.speed, &len);
    break;

  case USB_DESC_TYPE_CONFIGURATION:
    pbuf   = USBD_class_GetCfgDesc(pdev->cfg.speed, &len);
    pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
    pdev->dev.pConfig_descriptor = pbuf;
    break;

  case USB_DESC_TYPE_STRING:
    switch ((uint8_t)(req->wValue))
    {
    case USBD_IDX_LANGID_STR:
     pbuf = USBD_USR_LangIDStrDescriptor(pdev->cfg.speed, &len);
      break;

    case USBD_IDX_MFC_STR:
      pbuf = USBD_USR_ManufacturerStrDescriptor(pdev->cfg.speed, &len);
      break;

    case USBD_IDX_PRODUCT_STR:
      pbuf = USBD_USR_ProductStrDescriptor(pdev->cfg.speed, &len);
      break;

    case USBD_IDX_SERIAL_STR:
      pbuf = USBD_USR_SerialStrDescriptor(pdev->cfg.speed, &len);
      break;

    case USBD_IDX_CONFIG_STR:
      pbuf = USBD_USR_ConfigStrDescriptor(pdev->cfg.speed, &len);
      break;

    case USBD_IDX_INTERFACE_STR:
      pbuf = USBD_USR_InterfaceStrDescriptor(pdev->cfg.speed, &len);
      break;
    case USBD_IDX_OS_STR:
    	pbuf = USBD_USR_OsStrDescriptor(pdev->cfg.speed, &len);
    	break;
    default:
       USBD_CtlError(pdev , req);
      return;
    }
    break;
  case USB_DESC_TYPE_DEVICE_QUALIFIER:
      USBD_CtlError(pdev , req);
      return;

  case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
      USBD_CtlError(pdev , req);
      return;
  default:
     USBD_CtlError(pdev , req);
    return;
  }

  if((len != 0)&& (req->wLength != 0))
  {
    len = MIN(len , req->wLength);
    USBD_CtlSendData (pdev, pbuf, len);
  }

}

static void USBD_GetCompatIdDescriptor(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req)
{
	uint16_t len;
	uint8_t *pbuf;
	pbuf = USBD_USR_CompatIdDescriptor(pdev->cfg.speed, &len);
	len = MIN(len , req->wLength);
	USBD_CtlSendData (pdev,pbuf,len);
}

static void USBD_GetPropertiesOsDescriptor(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req)
{
	uint16_t len;
	uint8_t *pbuf;
	pbuf = USBD_USR_PropertiesOsDescriptor(pdev->cfg.speed, &len);
	len = MIN(len , req->wLength);
	USBD_CtlSendData (pdev,pbuf,len);
}

static void USBD_SetAddress(USB_OTG_CORE_HANDLE  *pdev,
                            USB_SETUP_REQ *req)
{
  uint8_t  dev_addr;

  if ((req->wIndex == 0) && (req->wLength == 0))
  {
    dev_addr = (uint8_t)(req->wValue) & 0x7F;

    if (pdev->dev.device_status == USB_OTG_CONFIGURED)
    {
      USBD_CtlError(pdev , req);
    }
    else
    {
      pdev->dev.device_address = dev_addr;
      DCD_EP_SetAddress(pdev, dev_addr);
      USBD_CtlSendStatus(pdev);

      if (dev_addr != 0)
      {
        pdev->dev.device_status  = USB_OTG_ADDRESSED;
      }
      else
      {
        pdev->dev.device_status  = USB_OTG_DEFAULT;
      }
    }
  }
  else
  {
     USBD_CtlError(pdev , req);
  }
}

static void USBD_SetConfig(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req)
{

  static uint8_t  cfgidx;

  cfgidx = (uint8_t)(req->wValue);

  if (cfgidx > USBD_CFG_MAX_NUM )
  {
     USBD_CtlError(pdev , req);
  }
  else
  {
    switch (pdev->dev.device_status)
    {
    case USB_OTG_ADDRESSED:
      if (cfgidx)
      {
        pdev->dev.device_config = cfgidx;
        pdev->dev.device_status = USB_OTG_CONFIGURED;
        USBD_SetCfg(pdev , cfgidx);
        USBD_CtlSendStatus(pdev);
      }
      else
      {
         USBD_CtlSendStatus(pdev);
      }
      break;

    case USB_OTG_CONFIGURED:
      if (cfgidx == 0)
      {
        pdev->dev.device_status = USB_OTG_ADDRESSED;
        pdev->dev.device_config = cfgidx;
        USBD_ClrCfg(pdev , cfgidx);
        USBD_CtlSendStatus(pdev);

      }
      else  if (cfgidx != pdev->dev.device_config)
      {
        /* Clear old configuration */
        USBD_ClrCfg(pdev , pdev->dev.device_config);

        /* set new configuration */
        pdev->dev.device_config = cfgidx;
        USBD_SetCfg(pdev , cfgidx);
        USBD_CtlSendStatus(pdev);
      }
      else
      {
        USBD_CtlSendStatus(pdev);
      }
      break;

    default:
       USBD_CtlError(pdev , req);
      break;
    }
  }
}

static void USBD_GetConfig(USB_OTG_CORE_HANDLE  *pdev,
                           USB_SETUP_REQ *req)
{

  if (req->wLength != 1)
  {
     USBD_CtlError(pdev , req);
  }
  else
  {
    switch (pdev->dev.device_status )
    {
    case USB_OTG_ADDRESSED:

      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_default_cfg,
                        1);
      break;

    case USB_OTG_CONFIGURED:

      USBD_CtlSendData (pdev,
                        &pdev->dev.device_config,
                        1);
      break;

    default:
       USBD_CtlError(pdev , req);
      break;
    }
  }
}

static void USBD_GetStatus(USB_OTG_CORE_HANDLE  *pdev,
                           USB_SETUP_REQ *req)
{


  switch (pdev->dev.device_status)
  {
  	  case USB_OTG_ADDRESSED:
  	  case USB_OTG_CONFIGURED:

#ifdef USBD_SELF_POWERED
    USBD_cfg_status = USB_CONFIG_SELF_POWERED;
#else
    USBD_cfg_status = 0x00;
#endif

		if (pdev->dev.DevRemoteWakeup)
		{
		  USBD_cfg_status |= USB_CONFIG_REMOTE_WAKEUP;
		}

		USBD_CtlSendData(pdev, (uint8_t *)&USBD_cfg_status, 2);
    break;

  default :
    USBD_CtlError(pdev , req);
    break;
  }
}

static void USBD_SetFeature(USB_OTG_CORE_HANDLE  *pdev,
                            USB_SETUP_REQ *req)
{

  USB_OTG_DCTL_TypeDef     dctl;
  uint8_t test_mode = 0;

  if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
  {
    pdev->dev.DevRemoteWakeup = 1;
    usbd_class_Setup(pdev, req);
    USBD_CtlSendStatus(pdev);
  }

  else if ((req->wValue == USB_FEATURE_TEST_MODE) &&
           ((req->wIndex & 0xFF) == 0))
  {
    dctl.d32 = USB_OTG_READ_REG32(&pdev->regs.DREGS->DCTL);

    test_mode = req->wIndex >> 8;
    switch (test_mode)
    {
    case 1: /* TEST_J */
      dctl.b.tstctl = 1;
      break;

    case 2: /* TEST_K */
      dctl.b.tstctl = 2;
      break;

    case 3: /* TEST_SE0_NAK */
      dctl.b.tstctl = 3;
      break;

    case 4: /* TEST_PACKET */
      dctl.b.tstctl = 4;
      break;

    case 5: /* TEST_FORCE_ENABLE */
      dctl.b.tstctl = 5;
      break;

    default :
      dctl.b.tstctl = 1;
      break;
    }
    SET_TEST_MODE = dctl;
    pdev->dev.test_mode = 1;
    USBD_CtlSendStatus(pdev);
  }
}

static void USBD_ClrFeature(USB_OTG_CORE_HANDLE  *pdev,
                            USB_SETUP_REQ *req)
{
  switch (pdev->dev.device_status)
  {
  case USB_OTG_ADDRESSED:
  case USB_OTG_CONFIGURED:
    if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
    {
      pdev->dev.DevRemoteWakeup = 0;
      usbd_class_Setup(pdev, req);
      USBD_CtlSendStatus(pdev);
    }
    break;

  default :
     USBD_CtlError(pdev , req);
    break;
  }
}

void USBD_ParseSetupRequest( USB_OTG_CORE_HANDLE  *pdev,
                            USB_SETUP_REQ *req)
{
  req->bmRequest     = *(uint8_t *)  (pdev->dev.setup_packet);
  req->bRequest      = *(uint8_t *)  (pdev->dev.setup_packet +  1);
  req->wValue        = SWAPBYTE      (pdev->dev.setup_packet +  2);
  req->wIndex        = SWAPBYTE      (pdev->dev.setup_packet +  4);
  req->wLength       = SWAPBYTE      (pdev->dev.setup_packet +  6);

  pdev->dev.in_ep[0].ctl_data_len = req->wLength  ;
  pdev->dev.device_state = USB_OTG_EP0_SETUP;
}

void USBD_CtlError( USB_OTG_CORE_HANDLE  *pdev,
                            USB_SETUP_REQ *req)
{

  DCD_EP_Stall(pdev , 0x80);
  DCD_EP_Stall(pdev , 0);
  USB_OTG_EP0_OutStart(pdev);
}

void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len)
{
  uint8_t idx = 0;

  if (desc != NULL)
  {
    *len =  USBD_GetLen(desc) * 2 + 2;
    unicode[idx++] = *len;
    unicode[idx++] =  USB_DESC_TYPE_STRING;

    while (*desc != '\0')
    {
      unicode[idx++] = *desc++;
      unicode[idx++] =  0x00;
    }
  }
}

static uint8_t USBD_GetLen(uint8_t *buf)
{
    uint8_t  len = 0;

    while (*buf != '\0')
    {
        len++;
        buf++;
    }

    return len;
}
