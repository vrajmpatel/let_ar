/**
 * @file ble_advertising.h
 * @brief BLE Advertising Module for S140 SoftDevice
 *
 * This module provides easy-to-use advertising functionality including:
 * - Configurable advertising data and scan response
 * - Fast and slow advertising modes
 * - Automatic restart after disconnection
 *
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x
 */

#ifndef BLE_ADVERTISING_H__
#define BLE_ADVERTISING_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_gap.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BLE_ADV_CONFIG Advertising Configuration
 * @{ */

/** @brief Maximum advertising data length (legacy advertising)
 * Citation: Bluetooth Core Spec Vol 6, Part B, Section 2.3.1 */
#define BLE_ADV_DATA_MAX_LEN            31

/** @brief Maximum scan response data length */
#define BLE_ADV_SCAN_RSP_MAX_LEN        31

/** @brief Maximum number of UUIDs to advertise */
#define BLE_ADV_MAX_UUIDS               4

/**
 * @brief Advertising mode enumeration
 */
typedef enum
{
    BLE_ADV_MODE_IDLE,       /**< Advertising is idle (not started) */
    BLE_ADV_MODE_FAST,       /**< Fast advertising (high frequency for quick connection) */
    BLE_ADV_MODE_SLOW,       /**< Slow advertising (low frequency for power saving) */
} ble_adv_mode_t;

/**
 * @brief Advertising configuration structure
 */
typedef struct
{
    /* Advertising intervals (in 0.625ms units)
     * Citation: Bluetooth Core Spec Vol 6, Part B, Section 4.4.2.2
     * Range: 0x0020 (20ms) to 0x4000 (10.24s) */
    uint16_t fast_interval;       /**< Fast advertising interval (default: 40 = 25ms) */
    uint16_t slow_interval;       /**< Slow advertising interval (default: 1600 = 1000ms) */
    
    /* Advertising durations (in 10ms units, 0 = infinite)
     * Citation: Nordic SDK - Duration after which advertising stops automatically */
    uint16_t fast_timeout;        /**< Fast advertising duration (default: 3000 = 30s) */
    uint16_t slow_timeout;        /**< Slow advertising duration (default: 0 = infinite) */
    
    /* Advertising data content */
    bool     include_name;        /**< Include device name in advertising data */
    bool     include_appearance;  /**< Include appearance in advertising data */
    bool     include_tx_power;    /**< Include TX power level in advertising data */
    
    /* UUIDs to advertise */
    ble_uuid_t uuids[BLE_ADV_MAX_UUIDS]; /**< UUIDs to include in advertising data */
    uint8_t    uuid_count;        /**< Number of UUIDs to advertise */
    
    /* Scan response data */
    bool     include_name_in_sr;  /**< Include device name in scan response (if not in adv data) */
    
    /* Manufacturer specific data */
    uint8_t *p_manuf_data;        /**< Pointer to manufacturer data (NULL if not used) */
    uint8_t  manuf_data_len;      /**< Length of manufacturer data */
    uint16_t company_id;          /**< Company identifier for manufacturer data */
    
    /* Auto restart */
    bool     auto_restart;        /**< Automatically restart advertising after disconnect */
    
} ble_advertising_config_t;

/**
 * @brief Default advertising configuration
 *
 * Fast advertising at 100ms for 30 seconds, then slow at 1000ms indefinitely.
 * Includes device name and general discoverable flag.
 */
#define BLE_ADVERTISING_CONFIG_DEFAULT \
{ \
    .fast_interval       = 160,   /* 100ms */ \
    .slow_interval       = 1600,  /* 1000ms */ \
    .fast_timeout        = 3000,  /* 30 seconds */ \
    .slow_timeout        = 0,     /* Infinite */ \
    .include_name        = true, \
    .include_appearance  = false, \
    .include_tx_power    = false, \
    .uuid_count          = 0, \
    .include_name_in_sr  = false, \
    .p_manuf_data        = NULL, \
    .manuf_data_len      = 0, \
    .company_id          = 0xFFFF, \
    .auto_restart        = true, \
}

/**
 * @brief Low power advertising configuration
 *
 * Slow advertising only at 2.5 seconds interval.
 */
#define BLE_ADVERTISING_CONFIG_LOW_POWER \
{ \
    .fast_interval       = 4000,  /* 2.5s */ \
    .slow_interval       = 4000,  /* 2.5s */ \
    .fast_timeout        = 0,     /* Go straight to slow */ \
    .slow_timeout        = 0,     /* Infinite */ \
    .include_name        = true, \
    .include_appearance  = false, \
    .include_tx_power    = false, \
    .uuid_count          = 0, \
    .include_name_in_sr  = false, \
    .p_manuf_data        = NULL, \
    .manuf_data_len      = 0, \
    .company_id          = 0xFFFF, \
    .auto_restart        = true, \
}

/**
 * @brief Fast connection advertising configuration
 *
 * Very fast advertising at 20ms for quick connection.
 */
#define BLE_ADVERTISING_CONFIG_FAST_CONNECT \
{ \
    .fast_interval       = 32,    /* 20ms - minimum */ \
    .slow_interval       = 160,   /* 100ms */ \
    .fast_timeout        = 6000,  /* 60 seconds */ \
    .slow_timeout        = 0,     /* Infinite */ \
    .include_name        = true, \
    .include_appearance  = false, \
    .include_tx_power    = false, \
    .uuid_count          = 0, \
    .include_name_in_sr  = false, \
    .p_manuf_data        = NULL, \
    .manuf_data_len      = 0, \
    .company_id          = 0xFFFF, \
    .auto_restart        = true, \
}

/** @} */

/** @defgroup BLE_ADV_EVENTS Advertising Events
 * @{ */

/**
 * @brief Advertising event types
 */
typedef enum
{
    BLE_ADV_EVT_STARTED,          /**< Advertising started */
    BLE_ADV_EVT_STOPPED,          /**< Advertising stopped */
    BLE_ADV_EVT_FAST_TIMEOUT,     /**< Fast advertising timeout, switching to slow */
    BLE_ADV_EVT_SLOW_TIMEOUT,     /**< Slow advertising timeout */
    BLE_ADV_EVT_CONNECTED,        /**< A central connected */
} ble_adv_evt_t;

/**
 * @brief Advertising event handler callback type
 *
 * @param[in] evt   Advertising event
 * @param[in] mode  Current advertising mode
 */
typedef void (*ble_advertising_evt_handler_t)(ble_adv_evt_t evt, ble_adv_mode_t mode);

/** @} */

/** @defgroup BLE_ADV_API Advertising API Functions
 * @{ */

/**
 * @brief Initialize the advertising module
 *
 * Call this after ble_stack_init() to set up advertising.
 *
 * @param[in] p_config  Pointer to configuration, or NULL for defaults
 *
 * @retval NRF_SUCCESS              Advertising initialized
 * @retval NRF_ERROR_INVALID_STATE  BLE stack not initialized
 * @retval NRF_ERROR_INVALID_PARAM  Invalid configuration
 */
uint32_t ble_advertising_init(const ble_advertising_config_t *p_config);

/**
 * @brief Add a service UUID to advertising data
 *
 * Must be called before ble_advertising_start().
 * Maximum BLE_ADV_MAX_UUIDS UUIDs can be added.
 *
 * @param[in] p_uuid  Pointer to UUID to add
 *
 * @retval NRF_SUCCESS          UUID added
 * @retval NRF_ERROR_NO_MEM     Too many UUIDs
 * @retval NRF_ERROR_NULL       Null pointer
 */
uint32_t ble_advertising_uuid_add(const ble_uuid_t *p_uuid);

/**
 * @brief Set manufacturer specific data
 *
 * Must be called before ble_advertising_start().
 *
 * @param[in] company_id  Company identifier (Bluetooth SIG assigned)
 * @param[in] p_data      Pointer to manufacturer data
 * @param[in] len         Length of manufacturer data
 *
 * @retval NRF_SUCCESS              Data set
 * @retval NRF_ERROR_INVALID_LENGTH Data too long
 */
uint32_t ble_advertising_manuf_data_set(uint16_t company_id, const uint8_t *p_data, uint8_t len);

/**
 * @brief Start advertising
 *
 * Starts advertising in fast mode (if configured with fast_timeout > 0)
 * or slow mode.
 *
 * @retval NRF_SUCCESS              Advertising started
 * @retval NRF_ERROR_INVALID_STATE  Already advertising or not initialized
 */
uint32_t ble_advertising_start(void);

/**
 * @brief Stop advertising
 *
 * @retval NRF_SUCCESS              Advertising stopped
 * @retval NRF_ERROR_INVALID_STATE  Not advertising
 */
uint32_t ble_advertising_stop(void);

/**
 * @brief Check if advertising is active
 *
 * @retval true   Advertising is active
 * @retval false  Advertising is not active
 */
bool ble_advertising_is_active(void);

/**
 * @brief Get current advertising mode
 *
 * @return Current advertising mode
 */
ble_adv_mode_t ble_advertising_mode_get(void);

/**
 * @brief Register advertising event handler
 *
 * @param[in] handler  Event handler function
 */
void ble_advertising_evt_handler_set(ble_advertising_evt_handler_t handler);

/**
 * @brief Handle BLE events for advertising module
 *
 * This is called automatically by ble_stack if the advertising module
 * is registered as a service handler. It handles:
 * - Connection events (stop advertising)
 * - Disconnection events (restart advertising if auto_restart)
 * - Advertising set terminated events (switch modes)
 *
 * @param[in] p_ble_evt  Pointer to BLE event
 */
void ble_advertising_on_ble_evt(const ble_evt_t *p_ble_evt);

/**
 * @brief Update advertising data while advertising
 *
 * Can be used to update manufacturer data or other dynamic content
 * without stopping advertising.
 *
 * @retval NRF_SUCCESS              Advertising data updated
 * @retval NRF_ERROR_INVALID_STATE  Not advertising
 */
uint32_t ble_advertising_data_update(void);

/**
 * @brief Restart advertising after connection
 *
 * Call this if auto_restart is disabled and you want to manually
 * restart advertising after a disconnection.
 *
 * @retval NRF_SUCCESS              Advertising restarted
 * @retval NRF_ERROR_INVALID_STATE  Already advertising
 */
uint32_t ble_advertising_restart(void);

/**
 * @brief Force advertising mode
 *
 * Immediately switch to specified advertising mode.
 *
 * @param[in] mode  Desired advertising mode
 *
 * @retval NRF_SUCCESS              Mode changed
 * @retval NRF_ERROR_INVALID_PARAM  Invalid mode
 */
uint32_t ble_advertising_mode_set(ble_adv_mode_t mode);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BLE_ADVERTISING_H__ */
