/**
 * @file nrf_sdm.h
 * @brief Nordic SoftDevice Manager (SDM) API for S140
 *
 * This header provides the SoftDevice Manager API for enabling/disabling
 * the S140 SoftDevice and configuring clock sources.
 *
 * Reference: Nordic nRF5 SDK 17.x, S140 SoftDevice Specification v7.x
 *
 * Memory Layout for S140 6.1.1:
 * - SoftDevice Flash: 0x00000000 - 0x00025FFF (152 KB)
 * - Application Flash: 0x00026000 - 0x000FFFFF
 * - SoftDevice RAM: 0x20000000 - 0x20001627
 * - Application RAM: 0x20001628+ (configurable based on BLE config)
 */

#ifndef NRF_SDM_H__
#define NRF_SDM_H__

#include <stdint.h>
#include "nrf_error.h"
#include "nrf_svc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup NRF_SDM_DEFINES SoftDevice Manager Definitions
 * @{ */

/** @brief S140 SoftDevice version */
#define SD_FWID_S140                          0x0101

/** @brief SoftDevice major version */
#define SD_MAJOR_VERSION                      7

/** @brief SoftDevice minor version */
#define SD_MINOR_VERSION                      3

/** @brief SoftDevice bugfix version */
#define SD_BUGFIX_VERSION                     0

/** @} */

/** @defgroup NRF_SDM_CLOCK_DEFINES Clock Source Definitions
 * @{ */

/** @brief Low frequency clock source values
 * Reference: nRF52840 Product Specification - Clock section
 */
typedef enum
{
    NRF_CLOCK_LF_SRC_RC      = 0,  /**< LFCLK RC oscillator (less accurate, no external components) */
    NRF_CLOCK_LF_SRC_XTAL    = 1,  /**< LFCLK 32.768 kHz crystal oscillator (high accuracy) */
    NRF_CLOCK_LF_SRC_SYNTH   = 2,  /**< LFCLK synthesized from HFCLK (requires HFCLK to be running) */
} nrf_clock_lf_src_t;

/** @brief Low frequency clock accuracy values (in ppm)
 * Reference: nRF52840 Product Specification Table 44 (LFXO specification)
 */
typedef enum
{
    NRF_CLOCK_LF_ACCURACY_250_PPM = 0,  /**< Default for RC oscillator */
    NRF_CLOCK_LF_ACCURACY_500_PPM = 1,  /**< 500 ppm */
    NRF_CLOCK_LF_ACCURACY_150_PPM = 2,  /**< 150 ppm */
    NRF_CLOCK_LF_ACCURACY_100_PPM = 3,  /**< 100 ppm */
    NRF_CLOCK_LF_ACCURACY_75_PPM  = 4,  /**< 75 ppm */
    NRF_CLOCK_LF_ACCURACY_50_PPM  = 5,  /**< 50 ppm */
    NRF_CLOCK_LF_ACCURACY_30_PPM  = 6,  /**< 30 ppm */
    NRF_CLOCK_LF_ACCURACY_20_PPM  = 7,  /**< 20 ppm - typical for good crystals */
    NRF_CLOCK_LF_ACCURACY_10_PPM  = 8,  /**< 10 ppm */
    NRF_CLOCK_LF_ACCURACY_5_PPM   = 9,  /**< 5 ppm */
    NRF_CLOCK_LF_ACCURACY_2_PPM   = 10, /**< 2 ppm */
    NRF_CLOCK_LF_ACCURACY_1_PPM   = 11, /**< 1 ppm */
} nrf_clock_lf_accuracy_t;

/** @brief Low frequency clock configuration structure */
typedef struct
{
    uint8_t source;         /**< LF clock source, see @ref nrf_clock_lf_src_t */
    uint8_t rc_ctiv;        /**< RC calibration timer interval (LFCLK RC only): 
                                 Calibration timer interval in units of 250ms.
                                 Range: 1-32 (0.25s - 8s), 0 = no calibration.
                                 Recommended: 16 (4 seconds) */
    uint8_t rc_temp_ctiv;   /**< RC calibration temperature interval (LFCLK RC only):
                                 How often to check temperature for recalibration.
                                 Range: 1-33, recommended: 2.
                                 0 = recalibrate on every rc_ctiv interval.
                                 Must be <= rc_ctiv. */
    uint8_t accuracy;       /**< Clock accuracy, see @ref nrf_clock_lf_accuracy_t */
} nrf_clock_lf_cfg_t;

/** @} */

/** @defgroup NRF_SDM_FAULT Fault Definitions
 * @{ */

/** @brief Fault handler function type
 *
 * @param[in] id     Fault identifier
 * @param[in] pc     Program counter at fault
 * @param[in] info   Additional fault information
 */
typedef void (*nrf_fault_handler_t)(uint32_t id, uint32_t pc, uint32_t info);

/** @brief Fault IDs */
typedef enum
{
    NRF_FAULT_ID_SD_ASSERT     = 0x0001, /**< SoftDevice assertion failed */
    NRF_FAULT_ID_APP_MEMACC    = 0x0002, /**< Application memory access violation */
    NRF_FAULT_ID_SD_RANGE      = 0x0003, /**< SoftDevice detected out of range value */
} nrf_fault_id_t;

/** @} */

/** @defgroup NRF_SDM_API SoftDevice Manager API Functions
 * @{ */

/**
 * @brief Enable the SoftDevice
 *
 * This function enables the SoftDevice and configures the BLE stack.
 * After this call, the SoftDevice controls the RADIO peripheral.
 *
 * @note LFCLK must be configured before calling this function.
 * @note The application interrupt vector table must be placed after the SoftDevice.
 *
 * @param[in] p_clock_lf_cfg  Low frequency clock configuration. Can be NULL to use
 *                            default RC oscillator settings.
 * @param[in] fault_handler   Fault handler function to be called on SoftDevice faults.
 *
 * @retval NRF_SUCCESS                           SoftDevice enabled successfully
 * @retval NRF_ERROR_INVALID_STATE               SoftDevice is already enabled
 * @retval NRF_ERROR_SDM_INCORRECT_INTERRUPT_CONFIGURATION  Invalid interrupt configuration
 * @retval NRF_ERROR_SDM_LFCLK_SOURCE_UNKNOWN    Unknown LFCLK source specified
 */
SVCALL(SD_SOFTDEVICE_ENABLE, uint32_t, sd_softdevice_enable(
    const nrf_clock_lf_cfg_t *p_clock_lf_cfg,
    nrf_fault_handler_t fault_handler));

/**
 * @brief Disable the SoftDevice
 *
 * This function disables the SoftDevice and releases the RADIO peripheral.
 * All BLE connections will be disconnected.
 *
 * @retval NRF_SUCCESS  SoftDevice disabled successfully
 */
SVCALL(SD_SOFTDEVICE_DISABLE, uint32_t, sd_softdevice_disable(void));

/**
 * @brief Check if SoftDevice is enabled
 *
 * @param[out] p_softdevice_enabled  Pointer to store enabled status (1 = enabled, 0 = disabled)
 *
 * @retval NRF_SUCCESS  Status retrieved successfully
 * @retval NRF_ERROR_INVALID_ADDR  Invalid pointer supplied
 */
SVCALL(SD_SOFTDEVICE_IS_ENABLED, uint32_t, sd_softdevice_is_enabled(uint8_t *p_softdevice_enabled));

/**
 * @brief Set the base address of the application's vector table
 *
 * This is required when the application's vector table is not at the default location
 * immediately after the SoftDevice.
 *
 * @param[in] address  Base address of the application's vector table
 *
 * @retval NRF_SUCCESS                    Vector table base set successfully
 * @retval NRF_ERROR_INVALID_ADDR         Invalid address (not aligned or in SoftDevice region)
 * @retval NRF_ERROR_SOFTDEVICE_NOT_ENABLED  SoftDevice not enabled
 */
SVCALL(SD_SOFTDEVICE_VECTOR_TABLE_BASE_SET, uint32_t, sd_softdevice_vector_table_base_set(uint32_t address));

/** @} */

/** @defgroup NRF_SDM_SOC_API SoC Library API Functions
 * @{ */

/**
 * @brief Request the high frequency crystal oscillator (HFXO)
 *
 * The HFXO is required for BLE radio operations. The SoftDevice will
 * start it automatically when needed, but you can request it early
 * for faster radio startup.
 *
 * @retval NRF_SUCCESS  HFXO request initiated
 */
SVCALL(SD_CLOCK_HFCLK_REQUEST, uint32_t, sd_clock_hfclk_request(void));

/**
 * @brief Release the high frequency crystal oscillator
 *
 * @retval NRF_SUCCESS  HFXO release initiated
 */
SVCALL(SD_CLOCK_HFCLK_RELEASE, uint32_t, sd_clock_hfclk_release(void));

/**
 * @brief Check if HFXO is running
 *
 * @param[out] p_is_running  Pointer to store running status (1 = running, 0 = not running)
 *
 * @retval NRF_SUCCESS  Status retrieved successfully
 */
SVCALL(SD_CLOCK_HFCLK_IS_RUNNING, uint32_t, sd_clock_hfclk_is_running(uint32_t *p_is_running));

/**
 * @brief Wait for an event (low power wait)
 *
 * Puts the CPU in low power mode (WFE) and wakes on any enabled event.
 * This is the recommended way to wait for events when SoftDevice is enabled.
 *
 * @retval NRF_SUCCESS  Returned from wait
 */
SVCALL(SD_APP_EVT_WAIT, uint32_t, sd_app_evt_wait(void));

/**
 * @brief Get temperature in 0.25°C units
 *
 * @param[out] p_temp  Pointer to store temperature value
 *
 * @retval NRF_SUCCESS  Temperature retrieved successfully
 */
SVCALL(SD_TEMP_GET, uint32_t, sd_temp_get(int32_t *p_temp));

/**
 * @brief Get random bytes from the SoftDevice's RNG
 *
 * @param[in]  length      Number of random bytes requested
 * @param[out] p_buff      Pointer to buffer for random data
 *
 * @retval NRF_SUCCESS                        Random bytes retrieved
 * @retval NRF_ERROR_SOC_RAND_NOT_ENOUGH_VALUES  Not enough random values available
 */
SVCALL(SD_RAND_APPLICATION_VECTOR_GET, uint32_t, sd_rand_application_vector_get(
    uint8_t *p_buff,
    uint8_t length));

/**
 * @brief Get number of random bytes available
 *
 * @param[out] p_bytes_available  Pointer to store count of available random bytes
 *
 * @retval NRF_SUCCESS  Count retrieved successfully
 */
SVCALL(SD_RAND_APPLICATION_BYTES_AVAILABLE_GET, uint32_t, sd_rand_application_bytes_available_get(
    uint8_t *p_bytes_available));

/**
 * @brief Enable DC/DC converter
 *
 * The DC/DC converter can significantly reduce power consumption when enabled.
 * Requires external inductor on DCC pin.
 *
 * Reference: nRF52840 Product Specification - Power management section
 *
 * @param[in] mode  0 = disable, 1 = enable
 *
 * @retval NRF_SUCCESS  DC/DC mode set successfully
 */
SVCALL(SD_POWER_DCDC_MODE_SET, uint32_t, sd_power_dcdc_mode_set(uint8_t mode));

/**
 * @brief Perform a system reset
 *
 * This function does not return.
 *
 * @retval Does not return
 */
SVCALL(SD_NVIC_SYSTEMRESET, uint32_t, sd_nvic_SystemReset(void));

/**
 * @brief Enable an interrupt via SoftDevice
 *
 * When SoftDevice is enabled, NVIC must be accessed through SoftDevice API.
 *
 * @param[in] IRQn  Interrupt number to enable
 *
 * @retval NRF_SUCCESS                              Interrupt enabled
 * @retval NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE  Interrupt reserved by SoftDevice
 */
SVCALL(SD_NVIC_ENABLEIRQ, uint32_t, sd_nvic_EnableIRQ(int IRQn));

/**
 * @brief Disable an interrupt via SoftDevice
 *
 * @param[in] IRQn  Interrupt number to disable
 *
 * @retval NRF_SUCCESS  Interrupt disabled
 */
SVCALL(SD_NVIC_DISABLEIRQ, uint32_t, sd_nvic_DisableIRQ(int IRQn));

/**
 * @brief Set interrupt priority via SoftDevice
 *
 * @param[in] IRQn      Interrupt number
 * @param[in] priority  Priority value (0-7, where 0,1,4,5 are reserved by SoftDevice)
 *
 * @retval NRF_SUCCESS  Priority set successfully
 * @retval NRF_ERROR_SOC_NVIC_INTERRUPT_PRIORITY_NOT_ALLOWED  Reserved priority level
 */
SVCALL(SD_NVIC_SETPRIORITY, uint32_t, sd_nvic_SetPriority(int IRQn, uint32_t priority));

/**
 * @brief Enter critical region (disable interrupts)
 *
 * @param[out] p_is_nested_critical_region  Pointer to store nested count
 *
 * @retval NRF_SUCCESS  Critical region entered
 */
SVCALL(SD_NVIC_CRITICAL_REGION_ENTER, uint32_t, sd_nvic_critical_region_enter(uint8_t *p_is_nested_critical_region));

/**
 * @brief Exit critical region (restore interrupt state)
 *
 * @param[in] is_nested_critical_region  Nested count from enter call
 *
 * @retval NRF_SUCCESS  Critical region exited
 */
SVCALL(SD_NVIC_CRITICAL_REGION_EXIT, uint32_t, sd_nvic_critical_region_exit(uint8_t is_nested_critical_region));

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NRF_SDM_H__ */
