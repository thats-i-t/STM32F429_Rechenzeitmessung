#ifndef __USBD_CORE_H
#define __USBD_CORE_H


#include "usb_dcd.h"
#include "usbd_def.h"
#include "usbd_conf.h"


typedef enum {
  USBD_OK   = 0,
  USBD_BUSY,
  USBD_FAIL,
}USBD_Status;


void USBD_Init(USB_OTG_CORE_HANDLE *pdev, USB_OTG_CORE_ID_TypeDef coreID, USBD_DEVICE *pDevice);
USBD_Status USBD_ClrCfg(USB_OTG_CORE_HANDLE  *pdev, uint8_t cfgidx);
USBD_Status USBD_SetCfg(USB_OTG_CORE_HANDLE  *pdev, uint8_t cfgidx);

uint8_t USBD_SetupStage(USB_OTG_CORE_HANDLE *pdev);
uint8_t USBD_DataOutStage(USB_OTG_CORE_HANDLE *pdev , uint8_t epnum);
uint8_t USBD_DataInStage(USB_OTG_CORE_HANDLE *pdev , uint8_t epnum);
uint8_t USBD_SOF(USB_OTG_CORE_HANDLE  *pdev);
uint8_t USBD_Reset(USB_OTG_CORE_HANDLE  *pdev);
uint8_t USBD_Suspend(USB_OTG_CORE_HANDLE  *pdev);
uint8_t USBD_Resume(USB_OTG_CORE_HANDLE  *pdev);
uint8_t  USBD_RunTestMode (USB_OTG_CORE_HANDLE  *pdev) ;

#endif /* __USBD_CORE_H */
