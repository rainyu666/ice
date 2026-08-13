#ifndef __USBD_CONF_H
#define __USBD_CONF_H

#include "stm32f0xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
/* Common Config */
#define USBD_MAX_NUM_INTERFACES               1
#define USBD_MAX_NUM_CONFIGURATION            1
#define USBD_MAX_STR_DESC_SIZ                 0x100
#define USBD_SUPPORT_USER_STRING              0 
#define USBD_SELF_POWERED                     1
#define USBD_DEBUG_LEVEL                      0

/* MSC Class Config */
#define MSC_MEDIA_PACKET                      512
// clang-format on

/* For footprint reasons and since only one allocation is handled in the MSC class
   driver, the malloc/free is changed into a static allocation method */

void *USBD_static_malloc(uint32_t size);
void  USBD_static_free(void *p);

// clang-format off
#define MAX_STATIC_ALLOC_SIZE     200 /*MSC Class Driver Structure size*/

#define USBD_malloc               (uint32_t *)USBD_static_malloc
#define USBD_free                 USBD_static_free
#define USBD_memset               /* Not used */
#define USBD_memcpy               /* Not used */

    
/* DEBUG macros */  
#if (USBD_DEBUG_LEVEL > 0)
#define  USBD_UsrLog(...)   printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_UsrLog(...)   
#endif                            
                            
#if (USBD_DEBUG_LEVEL > 1)

#define  USBD_ErrLog(...)   printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_ErrLog(...)   
#endif 
                                                        
#if (USBD_DEBUG_LEVEL > 2)                         
#define  USBD_DbgLog(...)   printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBD_DbgLog(...)                         
#endif
// clang-format on

#endif /* __USBD_CONF_H */
