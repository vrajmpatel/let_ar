/**
 * @file softdevice.h
 * @brief SoftDevice S140 Initialization and Management Module
 *
 * This module provides high-level functions for initializing and managing
 * the S140 SoftDevice on nRF52840.
 *
 * Memory Layout (S140 6.1.1):
 *   Citation: Nordic DevZone - "S140 6.1.1 FLASH_START=0x26000, Minimum RAM Start 0x20001628"
 *
 * - SoftDevice Flash: 0x00000000 - 0x00025FFF (152 KB)
 * - Application Flash: 0x00026000 - 0x000FFFFF
 * - SoftDevice RAM: 0x20000000 - 0x20001627 (~5.5 KB base)
 * - Application RAM: 0x20001628+ (depends on BLE configuration)
 */

#ifndef SOFTDEVICE_H__
#define SOFTDEVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_error.h"
#include "nrf_sdm.h"
#include "ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SOFTDEVICE_CONFIG SoftDevice Configuration
 * @{ */

/**
 * @brief SoftDevice configuration structure
 *
 * Use this to configure the SoftDevice before initialization.
 */
typedef struct
{
    /* Clock configuration */
    nrf_clock_lf_src_t      lfclk_source;       /**< Low frequency clock source */
    nrf_clock_lf_accuracy_t lfclk_accuracy;     /**< Clock accuracy (for XTAL) */
    uint8_t                 rc_ctiv;            /**< RC calibration interval (for RC) */
    uint8_t                 rc_temp_ctiv;       /**< RC temp calibration interval */
    
    /* BLE configuration */
    uint8_t                 periph_conn_count;  /**< Number of peripheral connections */
    uint8_t                 central_conn_count; /**< Number of central connections */
    uint16_t                att_mtu;            /**< ATT MTU size (23-247) */
    uint8_t                 vs_uuid_count;      /**< Number of vendor-specific UUIDs */
    uint16_t                attr_tab_size;      /**< GATT attribute table size in bytes */
    
    /* Features */
    bool                    service_changed;    /**< Include Service Changed characteristic */
    bool                    dcdc_enabled;       /**< Enable DC/DC converter */
    
} softdevice_config_t;

/**
 * @brief Default SoftDevice configuration
 *
 * Uses external 32.768 kHz crystal, 1 peripheral connection,
 * default MTU, suitable for most applications.
 */
#define SOFTDEVICE_CONFIG_DEFAULT \
{ \
    .lfclk_source      = NRF_CLOCK_LF_SRC_XTAL, \
    .lfclk_accuracy    = NRF_CLOCK_LF_ACCURACY_20_PPM, \
    .rc_ctiv           = 0, \
    .rc_temp_ctiv      = 0, \
    .periph_conn_count = 1, \
    .central_conn_count = 0, \
    .att_mtu           = BLE_GATT_ATT_MTU_DEFAULT, \
    .vs_uuid_count     = 2, \
    .attr_tab_size     = 1408, \
    .service_changed   = false, \
    .dcdc_enabled      = false, \
}

/**
 * @brief SoftDevice configuration for RC oscillator (no external crystal)
 */
#define SOFTDEVICE_CONFIG_RC_CLOCK \
{ \
    .lfclk_source      = NRF_CLOCK_LF_SRC_RC, \
    .lfclk_accuracy    = NRF_CLOCK_LF_ACCURACY_250_PPM, \
    .rc_ctiv           = 16, \
    .rc_temp_ctiv      = 2, \
    .periph_conn_count = 1, \
    .central_conn_count = 0, \
    .att_mtu           = BLE_GATT_ATT_MTU_DEFAULT, \
    .vs_uuid_count     = 2, \
    .attr_tab_size     = 1408, \
    .service_changed   = false, \
    .dcdc_enabled      = false, \
}

/** @} */

/** @defgroup SOFTDEVICE_API SoftDevice API Functions
 * @{ */

/**
 * @brief Initialize and enable the SoftDevice
 *
 * This function performs the following:
 * 1. Configures the low frequency clock
 * 2. Enables the SoftDevice with the specified clock configuration
 * 3. Configures the BLE stack (connections, MTU, etc.)
 * 4. Enables the BLE stack
 *
 * After this function returns successfully, the SoftDevice is running
 * and ready to accept BLE API calls.
 *
 * @param[in] p_config  Pointer to configuration structure, or NULL for defaults
 *
 * @retval NRF_SUCCESS                          SoftDevice enabled successfully
 * @retval NRF_ERROR_INVALID_STATE              SoftDevice already enabled
 * @retval NRF_ERROR_SDM_INCORRECT_INTERRUPT_CONFIGURATION  Invalid interrupt config
 * @retval NRF_ERROR_SDM_LFCLK_SOURCE_UNKNOWN   Unknown LFCLK source
 * @retval NRF_ERROR_NO_MEM                     Not enough RAM for configuration
 */
uint32_t softdevice_init(const softdevice_config_t *p_config);

/**
 * @brief Disable the SoftDevice
 *
 * Disables the SoftDevice and releases the RADIO peripheral.
 * All BLE connections will be disconnected.
 *
 * @retval NRF_SUCCESS  SoftDevice disabled successfully
 */
uint32_t softdevice_disable(void);

/**
 * @brief Check if SoftDevice is enabled
 *
 * @retval true   SoftDevice is enabled
 * @retval false  SoftDevice is disabled
 */
bool softdevice_is_enabled(void);

/**
 * @brief Get the application RAM base address
 *
 * Returns the start address of RAM available to the application
 * after SoftDevice and BLE stack RAM allocation.
 *
 * @return Application RAM base address
 */
uint32_t softdevice_app_ram_base_get(void);

/**
 * @brief Process SoftDevice events
 *
 * Call this function regularly (typically from main loop) to process
 * pending SoftDevice events. This will dispatch events to the registered
 * event handler.
 */
void softdevice_evt_process(void);

/**
 * @brief BLE event handler callback type
 *
 * @param[in] p_ble_evt  Pointer to BLE event
 */
typedef void (*ble_evt_handler_t)(const ble_evt_t *p_ble_evt);

/**
 * @brief Register a BLE event handler
 *
 * The handler will be called for all BLE events from the SoftDevice.
 *
 * @param[in] handler  Event handler function
 */
void softdevice_ble_evt_handler_set(ble_evt_handler_t handler);

/**
 * @brief SoftDevice fault handler (weak, can be overridden)
 *
 * Called when the SoftDevice encounters a fatal error.
 * Default implementation performs a system reset.
 *
 * @param[in] id    Fault identifier
 * @param[in] pc    Program counter at fault
 * @param[in] info  Additional fault information
 */
void softdevice_fault_handler(uint32_t id, uint32_t pc, uint32_t info);

/**
 * @brief Request high frequency clock (HFXO)
 *
 * @retval NRF_SUCCESS  HFXO request initiated
 */
uint32_t softdevice_hfclk_request(void);

/**
 * @brief Release high frequency clock
 *
 * @retval NRF_SUCCESS  HFXO release initiated
 */
uint32_t softdevice_hfclk_release(void);

/**
 * @brief Check if HFXO is running
 *
 * @retval true   HFXO is running
 * @retval false  HFXO is not running
 */
bool softdevice_hfclk_is_running(void);

/**
 * @brief Wait for event (low power)
 *
 * Puts the CPU in low power mode until an event occurs.
 * This is the recommended way to wait when SoftDevice is enabled.
 */
void softdevice_wait_for_event(void);

/**
 * @brief Get random bytes from SoftDevice RNG
 *
 * @param[out] p_buff   Buffer for random data
 * @param[in]  length   Number of random bytes to get
 *
 * @retval NRF_SUCCESS                        Random bytes retrieved
 * @retval NRF_ERROR_SOC_RAND_NOT_ENOUGH_VALUES Not enough random values available
 */
uint32_t softdevice_rand_get(uint8_t *p_buff, uint8_t length);

/**
 * @brief Get chip temperature
 *
 * @param[out] p_temp_degc  Pointer to store temperature in degrees Celsius
 *
 * @retval NRF_SUCCESS  Temperature retrieved
 */
uint32_t softdevice_temp_get(float *p_temp_degc);

/** @} */

/** @defgroup SOFTDEVICE_CRITICAL Critical Sections
 * @{ */

/**
 * @brief Enter critical section
 *
 * Disables interrupts in a way compatible with the SoftDevice.
 *
 * @param[out] p_nested  Pointer to store nesting flag
 */
void softdevice_critical_region_enter(uint8_t *p_nested);

/**
 * @brief Exit critical section
 *
 * Restores interrupt state.
 *
 * @param[in] nested  Nesting flag from enter call
 */
void softdevice_critical_region_exit(uint8_t nested);

/**
 * @brief Critical section macro
 */
#define SOFTDEVICE_CRITICAL_SECTION(code) \
    do { \
        uint8_t __nested; \
        softdevice_critical_region_enter(&__nested); \
        { code } \
        softdevice_critical_region_exit(__nested); \
    } while (0)

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* SOFTDEVICE_H__ */
