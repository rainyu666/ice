#ifndef __USBD_DESC_H
#define __USBD_DESC_H

#include "usbd_def.h"

#define DEVICE_ID1 (0x1FFFF7AC)
#define DEVICE_ID2 (0x1FFFF7B0)
#define DEVICE_ID3 (0x1FFFF7B4)

#define USB_SIZ_STRING_SERIAL 0x1A

extern USBD_DescriptorsTypeDef MSC_Desc;

#endif /* __USBD_DESC_H */
