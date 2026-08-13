#pragma once

#include "stm32f0xx_hal.h"
#include "platform/stm32/i2c.h"

// clang-format off

#define DEVICE_STRING "Mynd"

#define VIRTUAL_EEPROM_FLASH_SIZE       (4*1024)

#define BOOTLOADER_FLASH_SIZE           (20UL * 1024UL)
#define BOOTLOADER_FLASH_ADDRESS        (FLASH_BASE)

#define MCU_TOTAL_FLASH_SIZE            (128UL * 1024UL)
#define APPLICATION_FLASH_SIZE          (MCU_TOTAL_FLASH_SIZE - BOOTLOADER_FLASH_SIZE - VIRTUAL_EEPROM_FLASH_SIZE)
#define APPLICATION_FLASH_ADDRESS       (BOOTLOADER_FLASH_SIZE + FLASH_BASE)
#define APPLICATION_FLASH_LAST_ADDRESS  (APPLICATION_FLASH_ADDRESS + APPLICATION_FLASH_SIZE - 4)

#define VIRTUAL_EEPROM_FLASH_ADDRESS    (APPLICATION_FLASH_ADDRESS + APPLICATION_FLASH_SIZE)

#define UPDATE_FLASH_SIZE               (APPLICATION_FLASH_SIZE)
#define UPDATE_FLASH_ADDRESS            (APPLICATION_FLASH_ADDRESS)
#define UPDATE_FLASH_LAST_ADDRESS       (APPLICATION_FLASH_LAST_ADDRESS)

/* USB D+ */
#define USB_D_P_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define USB_D_P_GPIO_PIN                    GPIO_PIN_12
#define USB_D_P_GPIO_PORT                   GPIOA
#define USB_D_P_GPIO_MODE                   GPIO_MODE_AF_PP
#define USB_D_P_GPIO_PULL                   GPIO_NOPULL
#define USB_D_P_GPIO_AF                     GPIO_AF2_USB

/* USB D- */
#define USB_D_M_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define USB_D_M_GPIO_PIN                    GPIO_PIN_11
#define USB_D_M_GPIO_PORT                   GPIOA
#define USB_D_M_GPIO_MODE                   GPIO_MODE_AF_PP
#define USB_D_M_GPIO_PULL                   GPIO_NOPULL
#define USB_D_M_GPIO_AF                     GPIO_AF2_USB

// USB PD I2C pins definition
#define USB_PD_I2C_SCL_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define USB_PD_I2C_SCL_GPIO_PIN             GPIO_PIN_6
#define USB_PD_I2C_SCL_GPIO_PORT            GPIOB
#define USB_PD_I2C_SCL_GPIO_MODE            GPIO_MODE_AF_OD
#define USB_PD_I2C_SCL_GPIO_PULL            GPIO_PULLUP
#define USB_PD_I2C_SCL_GPIO_SPEED           GPIO_SPEED_FREQ_HIGH
#define USB_PD_I2C_SCL_GPIO_AF              GPIO_AF1_I2C1

#define USB_PD_I2C_SDA_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define USB_PD_I2C_SDA_GPIO_PIN             GPIO_PIN_7
#define USB_PD_I2C_SDA_GPIO_PORT            GPIOB
#define USB_PD_I2C_SDA_GPIO_MODE            GPIO_MODE_AF_OD
#define USB_PD_I2C_SDA_GPIO_PULL            GPIO_PULLUP
#define USB_PD_I2C_SDA_GPIO_SPEED           GPIO_SPEED_FREQ_HIGH
#define USB_PD_I2C_SDA_GPIO_AF              GPIO_AF1_I2C1

#define USB_PD_I2C                          I2C1
#define USB_PD_I2C_CLK_ENABLE()             __HAL_RCC_I2C1_CLK_ENABLE()
#define USB_PD_I2C_CLK_DISABLE()            __HAL_RCC_I2C1_CLK_DISABLE()
#define USB_PD_I2C_FORCE_RESET()            __HAL_RCC_I2C1_FORCE_RESET()
#define USB_PD_I2C_RELEASE_RESET()          __HAL_RCC_I2C1_RELEASE_RESET()

// Definition for I2Cx's NVIC
#define USB_PD_I2C_IRQn                     I2C1_IRQn
#define USB_PD_I2C_IRQHandler               I2C1_IRQHandler

#define USB_PD_I2C_TIMEOUT_MAX              ((uint32_t) 1000)

#define USB_PD_I2C_TIMING_100k              0x50332727                  // Hand-tuned for 100kHz
#define USB_PD_I2C_TIMING_300k              0x50330B0B                  // Hand-tuned for 300kHz
#define USB_PD_I2C_TIMING_400k              0x50330808                  // Hand-tuned for 400kHz
#define USB_PD_I2C_TIMING                   (USB_PD_I2C_TIMING_100k)
#define USB_PD_I2C_HANDLE                   &I2C1_Handle

// Shared I2C pins definition
#define SHARED_I2C_SCL_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define SHARED_I2C_SCL_GPIO_PIN             GPIO_PIN_10
#define SHARED_I2C_SCL_GPIO_PORT            GPIOB
#define SHARED_I2C_SCL_GPIO_MODE            GPIO_MODE_AF_OD
#define SHARED_I2C_SCL_GPIO_PULL            GPIO_NOPULL
#define SHARED_I2C_SCL_GPIO_SPEED           GPIO_SPEED_FREQ_HIGH
#define SHARED_I2C_SCL_GPIO_AF              GPIO_AF1_I2C2

#define SHARED_I2C_SDA_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define SHARED_I2C_SDA_GPIO_PIN             GPIO_PIN_11
#define SHARED_I2C_SDA_GPIO_PORT            GPIOB
#define SHARED_I2C_SDA_GPIO_MODE            GPIO_MODE_AF_OD
#define SHARED_I2C_SDA_GPIO_PULL            GPIO_NOPULL
#define SHARED_I2C_SDA_GPIO_SPEED           GPIO_SPEED_FREQ_HIGH
#define SHARED_I2C_SDA_GPIO_AF              GPIO_AF1_I2C2

#define SHARED_I2C                          I2C2
#define SHARED_I2C_CLK_ENABLE()             __HAL_RCC_I2C2_CLK_ENABLE()
#define SHARED_I2C_CLK_DISABLE()            __HAL_RCC_I2C2_CLK_DISABLE()
#define SHARED_I2C_FORCE_RESET()            __HAL_RCC_I2C2_FORCE_RESET()
#define SHARED_I2C_RELEASE_RESET()          __HAL_RCC_I2C2_RELEASE_RESET()

// Definition for I2Cx's NVIC
#define SHARED_I2C_IRQn                     I2C2_IRQn
#define SHARED_I2C_IRQHandler               I2C2_IRQHandler

#define SHARED_I2C_TIMING_100k              0x50332727                  // Hand-tuned for 100kHz
#define SHARED_I2C_TIMING_250k              0x50330D0D                  // Hand-tuned for 250kHz
#define SHARED_I2C_TIMING_300k              0x50330B0B                  // Hand-tuned for 300kHz
#define SHARED_I2C_TIMING_400k              0x50330808                  // Hand-tuned for 400kHz
#define SHARED_I2C_TIMING                   (SHARED_I2C_TIMING_400k)
#define SHARED_I2C_TIMEOUT_MAX              ((uint32_t) 100)
#define SHARED_I2C_HANDLE                   &I2C2_Handle

// ST driver expects 7-bit addresses shifted to the left
#define AW9523B_I2C_ADDRESS                 (0xB6) // 0x5B << 1
#define TAS5825P_I2C_ADDRESS                (0x98) // 0x4C << 1
#define TAS5805M_I2C_ADDRESS                (0x58) // 0x2C << 1
#define TPS25750_I2C_ADDRESS                (0x42) // 0x21 << 1
#define BQ25713_I2C_ADDRESS                 (0xD6) // 0x6B << 1

// Bluetooth UART TX pin
#define BLUETOOTH_UART_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BLUETOOTH_UART_TX_GPIO_PIN          GPIO_PIN_9
#define BLUETOOTH_UART_TX_GPIO_PORT         GPIOA
#define BLUETOOTH_UART_TX_GPIO_MODE         GPIO_MODE_AF_PP
#define BLUETOOTH_UART_TX_GPIO_PULL         GPIO_NOPULL
#define BLUETOOTH_UART_TX_GPIO_SPEED        GPIO_SPEED_FREQ_HIGH
#define BLUETOOTH_UART_TX_GPIO_AF           GPIO_AF1_USART1

// Bluetooth UART RX pin
#define BLUETOOTH_UART_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define BLUETOOTH_UART_RX_GPIO_PIN          GPIO_PIN_10
#define BLUETOOTH_UART_RX_GPIO_PORT         GPIOA
#define BLUETOOTH_UART_RX_GPIO_MODE         GPIO_MODE_AF_PP
#define BLUETOOTH_UART_RX_GPIO_PULL         GPIO_NOPULL
#define BLUETOOTH_UART_RX_GPIO_SPEED        GPIO_SPEED_FREQ_HIGH
#define BLUETOOTH_UART_RX_GPIO_AF           GPIO_AF1_USART1

#define BLUETOOTH_UART_CLK_ENABLE()         __HAL_RCC_USART1_CLK_ENABLE()
#define BLUETOOTH_UART                      USART1
#define BLUETOOTH_UART_BAUDRATE             115200
#define BLUETOOTH_UART_IRQn                 USART1_IRQn

// Debug UART TX pin
#define DEBUG_UART_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define DEBUG_UART_TX_GPIO_PIN              GPIO_PIN_2
#define DEBUG_UART_TX_GPIO_PORT             GPIOA
#define DEBUG_UART_TX_GPIO_MODE             GPIO_MODE_AF_PP
#define DEBUG_UART_TX_GPIO_PULL             GPIO_NOPULL
#define DEBUG_UART_TX_GPIO_SPEED            GPIO_SPEED_FREQ_HIGH
#define DEBUG_UART_TX_GPIO_AF               GPIO_AF1_USART2

// Debug UART RX pin
#define DEBUG_UART_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define DEBUG_UART_RX_GPIO_PIN              GPIO_PIN_3
#define DEBUG_UART_RX_GPIO_PORT             GPIOA
#define DEBUG_UART_RX_GPIO_MODE             GPIO_MODE_AF_PP
#define DEBUG_UART_RX_GPIO_PULL             GPIO_NOPULL
#define DEBUG_UART_RX_GPIO_SPEED            GPIO_SPEED_FREQ_HIGH
#define DEBUG_UART_RX_GPIO_AF               GPIO_AF1_USART2

#define DEBUG_UART_CLK_ENABLE()             __HAL_RCC_USART2_CLK_ENABLE()
#define DEBUG_UART                          USART2
#define DEBUG_UART_BAUDRATE                 115200
#define DEBUG_UART_IRQn                     USART2_IRQn

// Amps power down pin
#define AMPS_POWER_DOWN_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOC_CLK_ENABLE()
#define AMPS_POWER_DOWN_GPIO_PIN            GPIO_PIN_8
#define AMPS_POWER_DOWN_GPIO_PORT           GPIOC
#define AMPS_POWER_DOWN_GPIO_MODE           GPIO_MODE_OUTPUT_PP
#define AMPS_POWER_DOWN_GPIO_PULL           GPIO_NOPULL
#define AMPS_POWER_DOWN_GPIO_SPEED          GPIO_SPEED_FREQ_LOW

// Tweeter amp fault pin
#define TWEETER_FAULT_INT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define TWEETER_FAULT_INT_GPIO_PIN          GPIO_PIN_9
#define TWEETER_FAULT_INT_GPIO_PORT         GPIOC
#define TWEETER_FAULT_INT_GPIO_MODE         GPIO_MODE_IT_FALLING
#define TWEETER_FAULT_INT_GPIO_PULL         GPIO_PULLUP
#define TWEETER_FAULT_INT_GPIO_SPEED        GPIO_SPEED_FREQ_LOW

// Woofer amp fault pin
#define WOOFER_FAULT_INT_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define WOOFER_FAULT_INT_GPIO_PIN           GPIO_PIN_15
#define WOOFER_FAULT_INT_GPIO_PORT          GPIOA
#define WOOFER_FAULT_INT_GPIO_MODE          GPIO_MODE_IT_FALLING
#define WOOFER_FAULT_INT_GPIO_PULL          GPIO_PULLUP
#define WOOFER_FAULT_INT_GPIO_SPEED         GPIO_SPEED_FREQ_LOW

// Boost converter enable pin
#define BOOST_ENABLE_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define BOOST_ENABLE_GPIO_PIN               GPIO_PIN_8
#define BOOST_ENABLE_GPIO_PORT              GPIOB
#define BOOST_ENABLE_GPIO_MODE              GPIO_MODE_OUTPUT_PP
#define BOOST_ENABLE_GPIO_PULL              GPIO_NOPULL
#define BOOST_ENABLE_GPIO_SPEED             GPIO_SPEED_FREQ_LOW

// BT Vreg control pin
#define BT_CTRL_VREG_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define BT_CTRL_VREG_GPIO_PIN               GPIO_PIN_14
#define BT_CTRL_VREG_GPIO_PORT              GPIOB
#define BT_CTRL_VREG_GPIO_MODE              GPIO_MODE_OUTPUT_PP
#define BT_CTRL_VREG_GPIO_PULL              GPIO_NOPULL
#define BT_CTRL_VREG_GPIO_SPEED             GPIO_SPEED_FREQ_LOW

// BT 3V3 control pin
#define BT_CTRL_3V3_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOC_CLK_ENABLE()
#define BT_CTRL_3V3_GPIO_PIN                GPIO_PIN_7
#define BT_CTRL_3V3_GPIO_PORT               GPIOC
#define BT_CTRL_3V3_GPIO_MODE               GPIO_MODE_OUTPUT_PP
#define BT_CTRL_3V3_GPIO_PULL               GPIO_NOPULL
#define BT_CTRL_3V3_GPIO_SPEED              GPIO_SPEED_FREQ_LOW

// BT reset pin
#define BT_RESET_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOC_CLK_ENABLE()
#define BT_RESET_GPIO_PIN                   GPIO_PIN_6
#define BT_RESET_GPIO_PORT                  GPIOC
#define BT_RESET_GPIO_MODE                  GPIO_MODE_OUTPUT_PP
#define BT_RESET_GPIO_PULL                  GPIO_NOPULL
#define BT_RESET_GPIO_SPEED                 GPIO_SPEED_FREQ_LOW

// IO expander interrupt pin
#define IO_EXP_INT_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()
#define IO_EXP_INT_GPIO_PIN                 GPIO_PIN_2
#define IO_EXP_INT_GPIO_PORT                GPIOB
#define IO_EXP_INT_GPIO_MODE                GPIO_MODE_IT_FALLING
#define IO_EXP_INT_GPIO_PULL                GPIO_PULLUP
#define IO_EXP_INT_GPIO_SPEED               GPIO_SPEED_FREQ_LOW

// IO expander reset pin
#define IO_EXP_RESET_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define IO_EXP_RESET_GPIO_PIN               GPIO_PIN_8
#define IO_EXP_RESET_GPIO_PORT              GPIOA
#define IO_EXP_RESET_GPIO_MODE              GPIO_MODE_OUTPUT_PP
#define IO_EXP_RESET_GPIO_PULL              GPIO_NOPULL
#define IO_EXP_RESET_GPIO_SPEED             GPIO_SPEED_FREQ_LOW

// Plug detection pin
#define PLUG_DETECTION_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOC_CLK_ENABLE()
#define PLUG_DETECTION_GPIO_PIN             GPIO_PIN_1
#define PLUG_DETECTION_GPIO_PORT            GPIOC
#define PLUG_DETECTION_GPIO_MODE            GPIO_MODE_INPUT
#define PLUG_DETECTION_GPIO_PULL            GPIO_NOPULL
#define PLUG_DETECTION_GPIO_SPEED           GPIO_SPEED_FREQ_LOW

// Power button pin
#define POWER_BUTTON_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define POWER_BUTTON_GPIO_PIN               GPIO_PIN_0
#define POWER_BUTTON_GPIO_PORT              GPIOA
#define POWER_BUTTON_GPIO_MODE              GPIO_MODE_INPUT
#define POWER_BUTTON_GPIO_PULL              GPIO_NOPULL
#define POWER_BUTTON_GPIO_SPEED             GPIO_SPEED_FREQ_LOW

// Power hold pin
#define POWER_HOLD_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOA_CLK_ENABLE()
#define POWER_HOLD_GPIO_PIN                 GPIO_PIN_1
#define POWER_HOLD_GPIO_PORT                GPIOA
#define POWER_HOLD_GPIO_MODE                GPIO_MODE_OUTPUT_PP
#define POWER_HOLD_GPIO_PULL                GPIO_NOPULL
#define POWER_HOLD_GPIO_SPEED               GPIO_SPEED_FREQ_LOW

// USB select 0 pin
#define USB_SELECT0_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define USB_SELECT0_GPIO_PIN                GPIO_PIN_6
#define USB_SELECT0_GPIO_PORT               GPIOA
#define USB_SELECT0_GPIO_MODE               GPIO_MODE_OUTPUT_PP
#define USB_SELECT0_GPIO_PULL               GPIO_NOPULL
#define USB_SELECT0_GPIO_SPEED              GPIO_SPEED_FREQ_LOW

// USB select 1 pin
#define USB_SELECT1_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOC_CLK_ENABLE()
#define USB_SELECT1_GPIO_PIN                GPIO_PIN_10
#define USB_SELECT1_GPIO_PORT               GPIOC
#define USB_SELECT1_GPIO_MODE               GPIO_MODE_OUTPUT_PP
#define USB_SELECT1_GPIO_PULL               GPIO_NOPULL
#define USB_SELECT1_GPIO_SPEED              GPIO_SPEED_FREQ_LOW

// Auto ON detection pin
#define AUTO_ON_DETECT_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
#define AUTO_ON_DETECT_GPIO_PIN             GPIO_PIN_5
#define AUTO_ON_DETECT_GPIO_PORT            GPIOA
#define AUTO_ON_DETECT_GPIO_MODE            GPIO_MODE_ANALOG
#define AUTO_ON_DETECT_GPIO_PULL            GPIO_NOPULL
#define AUTO_ON_DETECT_ADC_CHANNEL          ADC_CHANNEL_5 // PA5 -> ADC_IN5

// ISnsRef pin
#define ISNS_REF_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOA_CLK_ENABLE()
#define ISNS_REF_GPIO_PIN                   GPIO_PIN_7
#define ISNS_REF_GPIO_PORT                  GPIOA
#define ISNS_REF_GPIO_MODE                  GPIO_MODE_ANALOG
#define ISNS_REF_GPIO_PULL                  GPIO_NOPULL
#define ISNS_REF_ADC_CHANNEL                ADC_CHANNEL_7 // PA7 -> ADC_IN7

// ISens pin
#define ISENS_GPIO_CLK_ENABLE()             __HAL_RCC_GPIOB_CLK_ENABLE()
#define ISENS_GPIO_PIN                      GPIO_PIN_0
#define ISENS_GPIO_PORT                     GPIOB
#define ISENS_GPIO_MODE                     GPIO_MODE_ANALOG
#define ISENS_GPIO_PULL                     GPIO_NOPULL
#define ISENS_ADC_CHANNEL                   ADC_CHANNEL_8 // PB0 -> ADC_IN8

// BT version pin
#define BT_VER_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOB_CLK_ENABLE()
#define BT_VER_GPIO_PIN                     GPIO_PIN_1
#define BT_VER_GPIO_PORT                    GPIOB
#define BT_VER_GPIO_MODE                    GPIO_MODE_ANALOG
#define BT_VER_GPIO_PULL                    GPIO_NOPULL
#define BT_VER_ADC_CHANNEL                  ADC_CHANNEL_9 // PB1 -> ADC_IN9

// PSys pin
#define PSYS_GPIO_CLK_ENABLE()              __HAL_RCC_GPIOC_CLK_ENABLE()
#define PSYS_GPIO_PIN                       GPIO_PIN_0
#define PSYS_GPIO_PORT                      GPIOC
#define PSYS_GPIO_MODE                      GPIO_MODE_ANALOG
#define PSYS_GPIO_PULL                      GPIO_NOPULL
#define PSYS_ADC_CHANNEL                    ADC_CHANNEL_10 // PC0 -> ADC_IN10

// BAT_NTC2 pin
#define BAT_NTC2_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOC_CLK_ENABLE()
#define BAT_NTC2_GPIO_PIN                   GPIO_PIN_4
#define BAT_NTC2_GPIO_PORT                  GPIOC
#define BAT_NTC2_GPIO_MODE                  GPIO_MODE_ANALOG
#define BAT_NTC2_GPIO_PULL                  GPIO_NOPULL
#define BAT_NTC2_ADC_CHANNEL                ADC_CHANNEL_14 // PC4 -> ADC_IN14

// Amp version pin
#define AMP_VER_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOC_CLK_ENABLE()
#define AMP_VER_GPIO_PIN                    GPIO_PIN_5
#define AMP_VER_GPIO_PORT                   GPIOC
#define AMP_VER_GPIO_MODE                   GPIO_MODE_ANALOG
#define AMP_VER_GPIO_PULL                   GPIO_NOPULL
#define AMP_VER_ADC_CHANNEL                 ADC_CHANNEL_15 // PC5 -> ADC_IN15

// clang-format on
