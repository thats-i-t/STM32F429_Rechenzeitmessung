#ifndef __USB_CLASS_CORE_H__
#define __USB_CLASS_CORE_H__

#include "stm32f4xx.h"
#include "usb_conf.h"
#include "usbd_desc.h"

#define USB_CLASS_CONFIG_DESC_SIZ                46
#define DEVICE_CLASS_NR                        0xFF
#define DEVICE_SUBCLASS_NR                     0xFF

#define CLASS_DATA_IN_PACKET_SIZE               CLASS_DATA_MAX_PACKET_SIZE
#define CLASS_DATA_OUT_PACKET_SIZE              CLASS_DATA_MAX_PACKET_SIZE

#define USB_CLASS_IDLE         0
#define USB_CLASS_BUSY         1
#define USB_CLASS_ZLP          2

extern uint8_t  USB_Tx_State;

uint8_t  usbd_class_Init        (void  *pdev, uint8_t cfgidx);
uint8_t  usbd_class_DeInit      (void  *pdev, uint8_t cfgidx);
uint8_t  usbd_class_Setup       (void  *pdev, USB_SETUP_REQ *req);
uint8_t  usbd_class_EP0_RxReady  (void *pdev);
uint8_t  usbd_class_DataIn      (void *pdev, uint8_t epnum);
uint8_t  usbd_class_DataOut     (void *pdev, uint8_t epnum);
uint8_t  usbd_class_SOF         (void *pdev);
uint8_t  *USBD_class_GetCfgDesc (uint8_t speed, uint16_t *length);

#endif /* __USB_CLASS_CORE_H__ */
