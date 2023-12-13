#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_conf.h"
#include "usb_regs.h"

#define USBD_VID                        0xDEAD //0x1 //0x0483
#define USBD_PID                        0xBEEF //0x1 //0x5740

#define USBD_NAME "STM32F407-based Motor controller"

#define USBD_PRODUCT_HS_STRING          USBD_NAME // "STM32 Virtual ComPort in HS mode"
#define USBD_CONFIGURATION_HS_STRING    (USBD_NAME ": Custom Configuration") // "VCP Config"
#define USBD_INTERFACE_HS_STRING        (USBD_NAME ": Custom Interface") // "VCP Interface"

#define USBD_LANGID_STRING              0x409
#define USBD_MANUFACTURER_STRING        USBD_NAME // "Fullspeed WinUsb Device"
#define USBD_PRODUCT_FS_STRING         	USBD_NAME // "Fullspeed WinUsb Device"
#define USBD_CONFIGURATION_FS_STRING  	(USBD_NAME ": Custom Configuration") //"Fullspeed WinUsb Device: Custom Configuration"
#define USBD_INTERFACE_FS_STRING       	(USBD_NAME ": Custom Interface") //"Fullspeed WinUsb Device: Custom Interface"


USBD_DEVICE USR_desc = {
  USBD_USR_DeviceDescriptor,
  USBD_USR_LangIDStrDescriptor,
  USBD_USR_ManufacturerStrDescriptor,
  USBD_USR_ProductStrDescriptor,
  USBD_USR_SerialStrDescriptor,
  USBD_USR_ConfigStrDescriptor,
  USBD_USR_InterfaceStrDescriptor,
  USBD_USR_OsStrDescriptor,
  USBD_USR_CompatIdDescriptor,
  USBD_USR_PropertiesOsDescriptor,
};



__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC] __ALIGN_END = {
  0x12,                         /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,   /* bDescriptorType */
  0x00,                         /* bcdUSB */
  0x02,
  0xFF,                         /* bDeviceClass */
  0xFF,                         /* bDeviceSubClass */
  0xFF,                         /* bDeviceProtocol */
  USB_OTG_MAX_EP0_SIZE,         /* bMaxPacketSize */
  LOBYTE(USBD_VID),             /* idVendor */
  HIBYTE(USBD_VID),             /* idVendor */
  LOBYTE(USBD_PID),             /* idVendor */
  HIBYTE(USBD_PID),             /* idVendor */
  0x00,                         /* bcdDevice rel. 2.00 */
  0x02,
  USBD_IDX_MFC_STR,             /* Index of manufacturer string */
  USBD_IDX_PRODUCT_STR,         /* Index of product string */
  USBD_IDX_SERIAL_STR,          /* Index of serial number string */
  USBD_CFG_MAX_NUM              /* bNumConfigurations */
};                              /* USB_DeviceDescriptor */


__ALIGN_BEGIN uint8_t USBD_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]
  __ALIGN_END = {
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID] __ALIGN_END = {
  USB_SIZ_STRING_LANGID,
  USB_DESC_TYPE_STRING,
  LOBYTE(USBD_LANGID_STRING),
  HIBYTE(USBD_LANGID_STRING),
};

uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] = {
  USB_SIZ_STRING_SERIAL,
  USB_DESC_TYPE_STRING,
};

__ALIGN_BEGIN uint8_t USBD_StrDesc[USB_MAX_STR_DESC_SIZ] __ALIGN_END;

__ALIGN_BEGIN uint8_t USBD_OsStrDesc[USB_SIZ_OS_STR_DESC] __ALIGN_END = {
		  0x12, /* Length */
		  0x03, /* Type */
		  0x4D, /* Signature: MSFT100 */
		  0x00,
		  0x53,
		  0x00,
		  0x46,
		  0x00,
		  0x54,
		  0x00,
		  0x31,
		  0x00,
		  0x30,
		  0x00,
		  0x30,
		  0x00,
		  MS_VENDOR_CODE, /* MS Vendor Code (unsigned byte) */
		  0x00 /* Pad */
};

__ALIGN_BEGIN uint8_t USBD_CompatIdDesc[USB_SIZ_COMPAT_ID_DESC] __ALIGN_END = {
  0x28, /* dwLength */
  0x00,
  0x00,
  0x00,
  0x00, /* bcdUSB */
  0x01,
  0x04, /* wIndex */
  0x00,
  0x01, /* bCount */
  0x00, /* Reserved */
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00, /* bFirstInterfaceNumber */
  0x00, /* Reserved */
  0x57, /* "WINUSB" (encoded) */
  0x49,
  0x4E,
  0x55,
  0x53,
  0x42,
  0x00,
  0x00,
  0x00, /* subCompatibleID */
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00, /* Reserved */
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
};

__ALIGN_BEGIN uint8_t USBD_PropertiesOsDescr[USB_SIZ_PROP_OS_DESC] __ALIGN_END = {
  0xCC, /* dwLength */
  0x00,
  0x00,
  0x00,
  0x00, /* bcdVersion */
  0x01,
  0x05, /* wIndex */
  0x00,
  0x02, /* wCount */
  0x00,
  0x84, /* dwSize */
  0x00,
  0x00,
  0x00,
  0x01, /* dwPropertyDataType */
  0x00,
  0x00,
  0x00,
  0x28, /* wPropertyNameLength */
  0x00,
  'D',  /* bPropertyName */
  0x00,
  'e',
  0x00,
  'v',
  0x00,
  'i',
  0x00,
  'c',
  0x00,
  'e',
  0x00,
  'I',
  0x00,
  'n',
  0x00,
  't',
  0x00,
  'e',
  0x00,
  'r',
  0x00,
  'f',
  0x00,
  'a',
  0x00,
  'c',
  0x00,
  'e',
  0x00,
  'G',
  0x00,
  'U',
  0x00,
  'I',
  0x00,
  'D',
  0x00,
  0x00,
  0x00,
  '{', /* dwPropertyDataLength */
  0x00,
  'E',
  0x00,
  'A',
  0x00,
  '0',
  0x00,
  'B',
  0x00,
  'D',
  0x00,
  '5',
  0x00,
  'C',
  0x00,
  '3',
  0x00,
  '-',
  0x00,
  '5',
  0x00,
  '0',
  0x00,
  'F',
  0x00,
  '3',
  0x00,
  '-',
  0x00,
  '4',
  0x00,
  '8',
  0x00,
  '8',
  0x00,
  '8',
  0x00,
  '-',
  0x00,
  '8',
  0x00,
  '4',
  0x00,
  'B',
  0x00,
  '4',
  0x00,
  '-',
  0x00,
  '7',
  0x00,
  '4',
  0x00,
  'E',
  0x00,
  '5',
  0x00,
  '0',
  0x00,
  'E',
  0x00,
  '1',
  0x00,
  '6',
  0x00,
  '4',
  0x00,
  '9',
  0x00,
  'D',
  0x00,
  'B',
  0x00,
  '}',
  0x00,
  0x00,
  0x00,
  0x3E, /* dwSize */
  0x00,
  0x00,
  0x00,
  0x01, /* dwPropertyDataType */
  0x00,
  0x00,
  0x00,
  0x0C, /* wPropertyNameLength */
  0x00,
  'L', /* bPropertyName */
  0x00,
  'a',
  0x00,
  'b',
  0x00,
  'e',
  0x00,
  'l',
  0x00,
  0x00,
  0x00,
  0x24, /* dwPropertyNameLength */
  0x00,
  0x00,
  0x00,
  'W', /* bPropertyData */
  0x00,
  'i',
  0x00,
  'n',
  0x00,
  'U',
  0x00,
  's',
  0x00,
  'b',
  0x00,
  'C',
  0x00,
  'o',
  0x00,
  'm',
  0x00,
  'm',
  0x00,
  ' ',
  0x00,
  'd',
  0x00,
  'e',
  0x00,
  'v',
  0x00,
  'i',
  0x00,
  'c',
  0x00,
  'e',
  0x00,
  0x00,
  0x00,
};

static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);
static void Get_SerialNum(void);

uint8_t *USBD_USR_DeviceDescriptor(uint8_t speed, uint16_t * length)
{
  *length = sizeof(USBD_DeviceDesc);
  return (uint8_t *) USBD_DeviceDesc;
}

uint8_t *USBD_USR_LangIDStrDescriptor(uint8_t speed, uint16_t * length)
{
  *length = sizeof(USBD_LangIDDesc);
  return (uint8_t *) USBD_LangIDDesc;
}

uint8_t *USBD_USR_ProductStrDescriptor(uint8_t speed, uint16_t * length)
{
  if (speed == 0)
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_PRODUCT_HS_STRING, USBD_StrDesc,
                   length);
  }
  else
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_PRODUCT_FS_STRING, USBD_StrDesc,
                   length);
  }
  return USBD_StrDesc;
}

uint8_t *USBD_USR_ManufacturerStrDescriptor(uint8_t speed, uint16_t * length)
{
  USBD_GetString((uint8_t *) (uint8_t *) USBD_MANUFACTURER_STRING, USBD_StrDesc,
                 length);
  return USBD_StrDesc;
}

uint8_t *USBD_USR_SerialStrDescriptor(uint8_t speed, uint16_t * length)
{
  *length = USB_SIZ_STRING_SERIAL;  
  Get_SerialNum(); /* Update the serial number string descriptor with the data from the unique ID */
  return (uint8_t *) USBD_StringSerial;
}

uint8_t *USBD_USR_ConfigStrDescriptor(uint8_t speed, uint16_t * length)
{
  if (speed == USB_OTG_SPEED_HIGH)
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_CONFIGURATION_HS_STRING,
                   USBD_StrDesc, length);
  }
  else
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_CONFIGURATION_FS_STRING,
                   USBD_StrDesc, length);
  }
  return USBD_StrDesc;
}

uint8_t *USBD_USR_InterfaceStrDescriptor(uint8_t speed, uint16_t * length)
{
  if (speed == 0)
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_INTERFACE_HS_STRING,
                   USBD_StrDesc, length);
  }
  else
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_INTERFACE_FS_STRING,
                   USBD_StrDesc, length);
  }
  return USBD_StrDesc;
}

static void Get_SerialNum(void)
{
  uint32_t deviceserial0, deviceserial1, deviceserial2;

  deviceserial0 = *(uint32_t *) DEVICE_ID1;
  deviceserial1 = *(uint32_t *) DEVICE_ID2;
  deviceserial2 = *(uint32_t *) DEVICE_ID3;

  deviceserial0 += deviceserial2;

  if (deviceserial0 != 0)
  {
    IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
    IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
  }
}

static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len)
{
  uint8_t idx = 0;

  for (idx = 0; idx < len; idx++)
  {
    if (((value >> 28)) < 0xA)
    {
      pbuf[2 * idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2 * idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4;

    pbuf[2 * idx + 1] = 0;
  }
}

uint8_t *USBD_USR_OsStrDescriptor(uint8_t speed, uint16_t * length)
{
  *length = sizeof(USBD_OsStrDesc);
  return (uint8_t *) USBD_OsStrDesc;
}

uint8_t *USBD_USR_CompatIdDescriptor(uint8_t speed, uint16_t * length)
{
  *length = sizeof(USBD_CompatIdDesc);
  return (uint8_t *) USBD_CompatIdDesc;
}

uint8_t *USBD_USR_PropertiesOsDescriptor(uint8_t speed, uint16_t * length)
{
  *length = sizeof(USBD_PropertiesOsDescr);
  return (uint8_t *) USBD_PropertiesOsDescr;
}
