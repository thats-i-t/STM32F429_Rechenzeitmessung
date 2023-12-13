#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#include "usb_conf.h"

#define USBD_ITF_MAX_NUM           1
#define USBD_CFG_MAX_NUM           1
#define USB_MAX_STR_DESC_SIZ       64 
#define USBD_EP0_MAX_PACKET_SIZE   64

/* Class layer parameters */
#define CLASS_IN_EP                       0x81  /* EP1 for data IN */
#define CLASS_IN_EP2                       0x82  /* EP2 for data IN */
#define CLASS_OUT_EP                      0x01  /* EP1 for data OUT */
#define CLASS_OUT_EP2                      0x02  /* EP1 for data OUT */

/* Endpoints parameters*/
 #define CLASS_DATA_MAX_PACKET_SIZE       64   /* Endpoint IN & OUT Packet size */
 #define CLASS_CMD_PACKET_SZE             8    /* Control Endpoint Packet size */

 #define CLASS_IN_FRAME_INTERVAL          5    /* Number of frames between IN transfers */
 #define APP_RX_DATA_SIZE               32768  //40960 // 2048  // Total size of IN buffer:
                                               // APP_RX_DATA_SIZE*8/MAX_BAUDARATE*1000 should be > CDC_IN_FRAME_INTERVAL

#endif //__USBD_CONF__H__
