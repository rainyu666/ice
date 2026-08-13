#pragma once

#define USBD_PID 0x003F

#define USBD_PRODUCT_FS_STRING "Teufel MYND"

// clang-format off
#define STORAGE_INQUIRYDATA_CONFIG \
    /* 36 */ \
    /* LUN 0 */ \
    0x00, 0x80, 0x02, 0x02, (STANDARD_INQUIRY_DATA_LEN - 5), \
    0x00, 0x00, 0x00, 'T',  'e', \
    'u',  'f',  'e',  'l',  ' ', \
    ' ', /* Manufacturer: 8 bytes  */ \
    'M',  'Y',  'N',  'D',  ' ', \
    ' ',  ' ',  ' ', /* Product     : 16 Bytes */ \
    ' ',  ' ',  ' ',  ' ',  ' ', \
    ' ',  ' ',  ' ',  '1',  '.', \
    '0',  '0', /* Version     : 4 Bytes  */
// clang-format on
