#include "usbd_core.h"
#include "usbd_req.h"
#include "usbd_ioreq.h"
#include "usb_dcd_int.h"
#include "usbd_class_core.h"
#include "usb_user.h"
#include "misc.h"

__IO USB_OTG_DCTL_TypeDef SET_TEST_MODE;

#define USB_PRIO 5
#define USB_SUBPRIO 5

void USBD_Init(USB_OTG_CORE_HANDLE *pdev, USB_OTG_CORE_ID_TypeDef coreID, USBD_DEVICE *pDevice)
{
	init_GPIO_USB();
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);
  
  /* set USB OTG core params */
  DCD_Init(pdev , coreID);
  
  /* Enable Interrupts */
  NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USB_PRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = USB_SUBPRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

uint8_t USBD_SetupStage(USB_OTG_CORE_HANDLE *pdev)
{
  USB_SETUP_REQ req;

  USBD_ParseSetupRequest(pdev , &req);
  
  switch (req.bmRequest & 0x1F) 
  {
  case USB_REQ_RECIPIENT_DEVICE:   
    USBD_StdDevReq (pdev, &req);
    break;
    
  case USB_REQ_RECIPIENT_INTERFACE:     
    USBD_StdItfReq(pdev, &req);
    break;
    
  case USB_REQ_RECIPIENT_ENDPOINT:        
    USBD_StdEPReq(pdev, &req);   
    break;
    
  default:           
    DCD_EP_Stall(pdev , req.bmRequest & 0x80);
    break;
  }  
  return USBD_OK;
}

uint8_t USBD_DataOutStage(USB_OTG_CORE_HANDLE *pdev , uint8_t epnum)
{
  USB_OTG_EP *ep;

  if(epnum == 0) 
  {
    ep = &pdev->dev.out_ep[0];
    if ( pdev->dev.device_state == USB_OTG_EP0_DATA_OUT)
    {
      if(ep->rem_data_len > ep->maxpacket)
      {
        ep->rem_data_len -=  ep->maxpacket;
        
        if(pdev->cfg.dma_enable == 1)
        {
          /* in slave mode this, is handled by the RxSTSQLvl ISR */
          ep->xfer_buff += ep->maxpacket; 
        }        
        USBD_CtlContinueRx (pdev, ep->xfer_buff, MIN(ep->rem_data_len ,ep->maxpacket));
      }
      else
      {
        if(pdev->dev.device_status == USB_OTG_CONFIGURED)
        {
          usbd_class_EP0_RxReady(pdev); 
        }
        USBD_CtlSendStatus(pdev);
      }
    }
  }
  else if(pdev->dev.device_status == USB_OTG_CONFIGURED)
  {
	  usbd_class_DataOut(pdev, epnum);
  } 
  return USBD_OK;
}

uint8_t USBD_DataInStage(USB_OTG_CORE_HANDLE *pdev , uint8_t epnum)
{
  USB_OTG_EP *ep;

  if(epnum == 0) 
  {
    ep = &pdev->dev.in_ep[0];
    if ( pdev->dev.device_state == USB_OTG_EP0_DATA_IN)
    {
      if(ep->rem_data_len > ep->maxpacket)
      {
        ep->rem_data_len -=  ep->maxpacket;
        if(pdev->cfg.dma_enable == 1)
        {
          /* in slave mode this, is handled by the TxFifoEmpty ISR */
          ep->xfer_buff += ep->maxpacket;
        }
        USBD_CtlContinueSendData (pdev, ep->xfer_buff, ep->rem_data_len);
        DCD_EP_PrepareRx (pdev, 0, NULL, 0); /* Start the transfer */
      }
      else
      { /* last packet is MPS multiple, so send ZLP packet */
        if((ep->total_data_len % ep->maxpacket == 0) &&
           (ep->total_data_len >= ep->maxpacket) &&
             (ep->total_data_len < ep->ctl_data_len ))
        {
          
          USBD_CtlContinueSendData(pdev , NULL, 0);
          ep->ctl_data_len = 0;
          
          /* Start the transfer */  
          DCD_EP_PrepareRx (pdev, 0, NULL, 0);
        }
        else
        {     
          USBD_CtlReceiveStatus(pdev);
        }
      }
    }
    if (pdev->dev.test_mode == 1)
    {
      USBD_RunTestMode(pdev); 
      pdev->dev.test_mode = 0;
    }
  }
  else if(pdev->dev.device_status == USB_OTG_CONFIGURED)
  {
    usbd_class_DataIn(pdev, epnum); 
  }

  return USBD_OK;
}

uint8_t  USBD_RunTestMode(USB_OTG_CORE_HANDLE  *pdev) 
{
  USB_OTG_WRITE_REG32(&pdev->regs.DREGS->DCTL, SET_TEST_MODE.d32);
  return USBD_OK;  
}

uint8_t USBD_Reset(USB_OTG_CORE_HANDLE  *pdev)
{  
  DCD_EP_Open(pdev, 0x00, USB_OTG_MAX_EP0_SIZE, EP_TYPE_CTRL);  /* Open EP0 OUT */
  DCD_EP_Open(pdev, 0x80, USB_OTG_MAX_EP0_SIZE, EP_TYPE_CTRL); /* Open EP0 IN */
  pdev->dev.device_status = USB_OTG_DEFAULT;  
  return USBD_OK;
}

uint8_t USBD_Resume(USB_OTG_CORE_HANDLE  *pdev)
{
  pdev->dev.device_status = pdev->dev.device_old_status;  
  pdev->dev.device_status = USB_OTG_CONFIGURED;  
  return USBD_OK;
}

uint8_t USBD_Suspend(USB_OTG_CORE_HANDLE  *pdev)
{
  pdev->dev.device_old_status = pdev->dev.device_status;
  pdev->dev.device_status  = USB_OTG_SUSPENDED;
  return USBD_OK;
}

uint8_t USBD_SOF(USB_OTG_CORE_HANDLE  *pdev)
{
  usbd_class_SOF(pdev); 
  return USBD_OK;
}

USBD_Status USBD_SetCfg(USB_OTG_CORE_HANDLE  *pdev, uint8_t cfgidx)
{
  usbd_class_Init(pdev, cfgidx); 
  return USBD_OK; 
}

USBD_Status USBD_ClrCfg(USB_OTG_CORE_HANDLE  *pdev, uint8_t cfgidx)
{
  usbd_class_DeInit(pdev, cfgidx);   
  return USBD_OK;
}
