#ifndef __USB_CONF__H__
#define __USB_CONF__H__

#include "usb_conf.h"
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"


#define USB_OTG_FS_CORE
#define RX_FIFO_FS_SIZE                          128
#define TX0_FIFO_FS_SIZE                          64
#define TX1_FIFO_FS_SIZE                         128
#define TX2_FIFO_FS_SIZE                          0
#define TX3_FIFO_FS_SIZE                          0
#define TXH_NP_HS_FIFOSIZ                         96
#define TXH_P_HS_FIFOSIZ                          96

#define __ALIGN_BEGIN
#define __ALIGN_END   

/* __packed keyword used to decrease the data type alignment to 1-byte */
#if defined (__CC_ARM)         /* ARM Compiler */
  #define __packed    __packed
#elif defined (__ICCARM__)     /* IAR Compiler */
  #define __packed    __packed
#elif defined   ( __GNUC__ )   /* GNU Compiler */      
  #ifndef __packed                  
    #define __packed    __attribute__ ((__packed__))
  #endif
#endif /* __CC_ARM */


#endif //__USB_CONF__H__
