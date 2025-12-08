/**
 * @file sdk_config.h
 * @brief Nordic SDK configuration for LED Glasses IMU Firmware
 * 
 * Configuration options for Nordic nRF5 SDK components.
 * This is a minimal configuration for bare-metal development with SoftDevice.
 * 
 * Citations:
 * - nRF52840_PS_v1.11.pdf: Hardware specifications
 * - FIRMWARE_DESIGN.md: Design decisions and requirements
 * - Nordic SDK documentation
 */

#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Target Configuration
 ******************************************************************************/

/* Target device
 * Citation: nRF52840_PS_v1.11.pdf - nRF52840 Product Specification
 */
#define NRF52840_XXAA                   1

/*******************************************************************************
 * SoftDevice Configuration
 * Citation: FIRMWARE_DESIGN.md:
 *   "SoftDevice: S140 6.1.1"
 *   "App Start Address: 0x26000"
 ******************************************************************************/

#define SOFTDEVICE_PRESENT              1
#define S140                            1
#define NRF_SD_BLE_API_VERSION          7

/* SoftDevice clock configuration */
#define NRF_SDH_CLOCK_LF_SRC            1   /* LFCLK source: XTAL */
#define NRF_SDH_CLOCK_LF_RC_CTIV        0
#define NRF_SDH_CLOCK_LF_RC_TEMP_CTIV   0
#define NRF_SDH_CLOCK_LF_ACCURACY       7   /* 20 ppm */

/*******************************************************************************
 * BLE Stack Configuration
 * Citation: FIRMWARE_DESIGN.md Section "BLE Configuration"
 ******************************************************************************/

/* Enable BLE stack */
#define NRF_SDH_BLE_ENABLED             1

/* BLE configuration */
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT   1
#define NRF_SDH_BLE_CENTRAL_LINK_COUNT      0
#define NRF_SDH_BLE_TOTAL_LINK_COUNT        1

/* GATT configuration
 * Citation: FIRMWARE_DESIGN.md: "MTU: Up to 247 bytes"
 */
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE   247
#define NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE 1408
#define NRF_SDH_BLE_VS_UUID_COUNT       2   /* Custom service + characteristics */

/* GAP configuration */
#define NRF_SDH_BLE_GAP_DATA_LENGTH     251
#define NRF_SDH_BLE_GAP_EVENT_LENGTH    6

/* Observer priority for BLE events */
#define NRF_SDH_BLE_OBSERVER_PRIO_LEVELS 4

/*******************************************************************************
 * GPIO Configuration
 * Citation: nRF52840_PS_v1.11.pdf Section 6.9 (GPIO)
 ******************************************************************************/

#define GPIOTE_ENABLED                  1
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 4

/*******************************************************************************
 * I2C (TWIM) Configuration
 * Citation: nRF52840_PS_v1.11.pdf Section 6.31 (TWIM)
 * Citation: FIRMWARE_DESIGN.md: "I2C @ 400kHz"
 ******************************************************************************/

#define NRFX_TWIM_ENABLED               1
#define NRFX_TWIM0_ENABLED              1
#define NRFX_TWIM1_ENABLED              0

#define NRFX_TWIM_DEFAULT_CONFIG_FREQUENCY      26738688  /* 400 kHz */
#define NRFX_TWIM_DEFAULT_CONFIG_HOLD_BUS_UNINIT 0

/* Legacy TWI driver (wrapper around NRFX) */
#define TWI_ENABLED                     1
#define TWI0_ENABLED                    1
#define TWI0_USE_EASY_DMA               1
#define TWI1_ENABLED                    0

/*******************************************************************************
 * UART Configuration (Debug)
 * Citation: FIRMWARE_DESIGN.md: Debug configuration
 ******************************************************************************/

#define NRFX_UARTE_ENABLED              1
#define NRFX_UARTE0_ENABLED             1

#define NRFX_UART_ENABLED               0

/* Legacy UART driver */
#define UART_ENABLED                    1
#define UART0_ENABLED                   1
#define UART_DEFAULT_CONFIG_BAUDRATE    30801920  /* 115200 baud */

/*******************************************************************************
 * Timer Configuration
 ******************************************************************************/

#define NRFX_TIMER_ENABLED              1
#define NRFX_TIMER0_ENABLED             0   /* Reserved for SoftDevice */
#define NRFX_TIMER1_ENABLED             1
#define NRFX_TIMER2_ENABLED             1

/* App timer (uses RTC) */
#define APP_TIMER_ENABLED               1
#define APP_TIMER_CONFIG_RTC_FREQUENCY  0   /* 32768 Hz */
#define APP_TIMER_CONFIG_IRQ_PRIORITY   6
#define APP_TIMER_CONFIG_OP_QUEUE_SIZE  10
#define APP_TIMER_CONFIG_USE_SCHEDULER  0

/*******************************************************************************
 * Power Management
 * Citation: nRF52840_PS_v1.11.pdf Section 5.3 (POWER)
 ******************************************************************************/

#define NRFX_POWER_ENABLED              1
#define NRFX_POWER_CONFIG_DEFAULT_DCDCEN 1  /* Enable DC/DC converter */

#define POWER_ENABLED                   1

/*******************************************************************************
 * Clock Configuration
 * Citation: nRF52840_PS_v1.11.pdf Section 5.2 (CLOCK)
 ******************************************************************************/

#define NRFX_CLOCK_ENABLED              1
#define NRFX_CLOCK_CONFIG_LF_SRC        1   /* LFCLK from XTAL */
#define NRFX_CLOCK_CONFIG_IRQ_PRIORITY  6

#define CLOCK_ENABLED                   1

/*******************************************************************************
 * Logging Configuration
 ******************************************************************************/

#define NRF_LOG_ENABLED                 1
#define NRF_LOG_BACKEND_RTT_ENABLED     0
#define NRF_LOG_BACKEND_UART_ENABLED    1
#define NRF_LOG_DEFAULT_LEVEL           3   /* INFO */
#define NRF_LOG_DEFERRED                0   /* Process logs immediately */

/* Log module levels */
#define NRF_SDH_BLE_LOG_LEVEL           3
#define NRF_SDH_LOG_LEVEL               3

/*******************************************************************************
 * FPU Configuration
 * Citation: nRF52840_PS_v1.11.pdf: "FPU" - Floating Point Unit
 ******************************************************************************/

#define NRF_FPU_ENABLED                 1

/*******************************************************************************
 * Crypto Configuration (optional)
 ******************************************************************************/

#define NRF_CRYPTO_ENABLED              0

/*******************************************************************************
 * Scheduler Configuration (optional)
 ******************************************************************************/

#define APP_SCHEDULER_ENABLED           0

/*******************************************************************************
 * FIFO Configuration
 ******************************************************************************/

#define APP_FIFO_ENABLED                1

/*******************************************************************************
 * Section Variables
 ******************************************************************************/

#define NRF_SECTION_ITER_ENABLED        1

/*******************************************************************************
 * Error Handling
 ******************************************************************************/

#define APP_ERROR_ENABLED               1
#define APP_ERROR_CHECK_ENABLED         1

/*******************************************************************************
 * Memory Manager (optional)
 ******************************************************************************/

#define MEM_MANAGER_ENABLED             0

/*******************************************************************************
 * Custom Board Definitions
 * Citation: FIRMWARE_DESIGN.md: LED Glasses Driver pinout
 ******************************************************************************/

/* Override default board pins if needed */
#define BOARD_CUSTOM                    1

#ifdef __cplusplus
}
#endif

#endif /* SDK_CONFIG_H */
