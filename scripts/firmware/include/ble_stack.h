/**
 * @file ble_stack.h
 * @brief BLE Stack Management Module for S140 SoftDevice
 *
 * This module provides high-level management of the BLE stack including:
 * - GAP configuration (device name, appearance, connection parameters)
 * - Connection management
 * - Event dispatching to registered services
 *
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x
 */

#ifndef BLE_STACK_H__
#define BLE_STACK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_gap.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BLE_STACK_CONFIG BLE Stack Configuration
 * @{ */

/** @brief Maximum device name length */
#define BLE_STACK_DEVICE_NAME_MAX_LEN       32

/** @brief Maximum number of service event handlers */
#define BLE_STACK_MAX_SERVICE_HANDLERS      8

/**
 * @brief BLE stack configuration structure
 */
typedef struct
{
    /* Device information */
    const char *device_name;        /**< Device name (UTF-8, null-terminated) */
    uint16_t    appearance;         /**< Device appearance, see @ref BLE_APPEARANCE */
    
    /* Connection parameters
     * Citation: Bluetooth Core Spec Vol 6, Part B, Section 4.5.1
     * - Interval: 7.5ms to 4000ms in 1.25ms units
     * - Slave latency: 0 to 499
     * - Supervision timeout: 100ms to 32000ms in 10ms units
     */
    uint16_t min_conn_interval;     /**< Min connection interval (1.25ms units), use 0 for default */
    uint16_t max_conn_interval;     /**< Max connection interval (1.25ms units), use 0 for default */
    uint16_t slave_latency;         /**< Slave latency */
    uint16_t conn_sup_timeout;      /**< Connection supervision timeout (10ms units) */
    
    /* TX Power
     * Citation: nRF52840 Product Specification - Supported values:
     * -40, -20, -16, -12, -8, -4, 0, +2, +3, +4, +5, +6, +7, +8 dBm
     */
    int8_t tx_power;                /**< TX power in dBm */
    
} ble_stack_config_t;

/**
 * @brief Default BLE stack configuration
 *
 * Uses common defaults suitable for most applications:
 * - Connection interval: 30-75ms (good balance of latency and power)
 * - Slave latency: 0 (responsive)
 * - Supervision timeout: 4 seconds
 * - TX power: 0 dBm (good range, moderate power)
 */
#define BLE_STACK_CONFIG_DEFAULT \
{ \
    .device_name       = "LET-AR IMU", \
    .appearance        = BLE_APPEARANCE_GENERIC_TAG, \
    .min_conn_interval = 24,   /* 30ms */ \
    .max_conn_interval = 60,   /* 75ms */ \
    .slave_latency     = 0, \
    .conn_sup_timeout  = 400,  /* 4 seconds */ \
    .tx_power          = 0, \
}

/**
 * @brief Low power configuration (longer intervals, higher latency)
 */
#define BLE_STACK_CONFIG_LOW_POWER \
{ \
    .device_name       = "LET-AR IMU", \
    .appearance        = BLE_APPEARANCE_GENERIC_TAG, \
    .min_conn_interval = 80,   /* 100ms */ \
    .max_conn_interval = 160,  /* 200ms */ \
    .slave_latency     = 4, \
    .conn_sup_timeout  = 600,  /* 6 seconds */ \
    .tx_power          = -4, \
}

/**
 * @brief High throughput configuration (short intervals, no latency)
 */
#define BLE_STACK_CONFIG_HIGH_THROUGHPUT \
{ \
    .device_name       = "LET-AR IMU", \
    .appearance        = BLE_APPEARANCE_GENERIC_TAG, \
    .min_conn_interval = 6,    /* 7.5ms - minimum allowed */ \
    .max_conn_interval = 12,   /* 15ms */ \
    .slave_latency     = 0, \
    .conn_sup_timeout  = 200,  /* 2 seconds */ \
    .tx_power          = 4, \
}

/** @} */

/** @defgroup BLE_STACK_CONN Connection State
 * @{ */

/**
 * @brief Connection state structure
 */
typedef struct
{
    uint16_t            conn_handle;    /**< Connection handle (BLE_CONN_HANDLE_INVALID if disconnected) */
    bool                connected;      /**< Connection status */
    ble_gap_addr_t      peer_addr;      /**< Peer device address */
    ble_gap_conn_params_t conn_params;  /**< Current connection parameters */
    int8_t              rssi;           /**< Last RSSI measurement */
} ble_stack_conn_state_t;

/** @} */

/** @defgroup BLE_STACK_HANDLERS Event Handlers
 * @{ */

/**
 * @brief Connection event handler callback type
 *
 * @param[in] connected  True if connected, false if disconnected
 * @param[in] p_state    Pointer to connection state
 */
typedef void (*ble_stack_conn_handler_t)(bool connected, const ble_stack_conn_state_t *p_state);

/**
 * @brief Service event handler callback type
 *
 * Services register this callback to receive BLE events for processing.
 *
 * @param[in] p_ble_evt  Pointer to BLE event
 */
typedef void (*ble_stack_service_handler_t)(const ble_evt_t *p_ble_evt);

/** @} */

/** @defgroup BLE_STACK_API BLE Stack API Functions
 * @{ */

/**
 * @brief Initialize the BLE stack
 *
 * This function must be called after softdevice_init() and before
 * any other BLE stack functions. It configures:
 * - GAP parameters (device name, appearance)
 * - Preferred connection parameters
 * - TX power
 *
 * @param[in] p_config  Pointer to configuration, or NULL for defaults
 *
 * @retval NRF_SUCCESS              BLE stack initialized successfully
 * @retval NRF_ERROR_INVALID_STATE  SoftDevice not enabled
 * @retval NRF_ERROR_INVALID_PARAM  Invalid configuration parameter
 */
uint32_t ble_stack_init(const ble_stack_config_t *p_config);

/**
 * @brief Get current connection state
 *
 * @return Pointer to connection state structure (always valid)
 */
const ble_stack_conn_state_t *ble_stack_conn_state_get(void);

/**
 * @brief Check if connected
 *
 * @retval true   A central is connected
 * @retval false  No connection
 */
bool ble_stack_is_connected(void);

/**
 * @brief Get current connection handle
 *
 * @return Connection handle, or BLE_CONN_HANDLE_INVALID if not connected
 */
uint16_t ble_stack_conn_handle_get(void);

/**
 * @brief Disconnect from central
 *
 * @retval NRF_SUCCESS                  Disconnect initiated
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Not connected
 */
uint32_t ble_stack_disconnect(void);

/**
 * @brief Request connection parameter update
 *
 * Requests the central to update connection parameters.
 *
 * @param[in] min_interval  Minimum connection interval (1.25ms units)
 * @param[in] max_interval  Maximum connection interval (1.25ms units)
 * @param[in] latency       Slave latency
 * @param[in] timeout       Supervision timeout (10ms units)
 *
 * @retval NRF_SUCCESS                  Request sent
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Not connected
 * @retval NRF_ERROR_INVALID_PARAM      Invalid parameters
 */
uint32_t ble_stack_conn_param_update(uint16_t min_interval, uint16_t max_interval,
                                      uint16_t latency, uint16_t timeout);

/**
 * @brief Set TX power
 *
 * @param[in] tx_power  TX power in dBm (-40 to +8)
 *
 * @retval NRF_SUCCESS              TX power set
 * @retval NRF_ERROR_INVALID_PARAM  Invalid TX power value
 */
uint32_t ble_stack_tx_power_set(int8_t tx_power);

/**
 * @brief Start RSSI measurements
 *
 * RSSI will be updated automatically on connection events.
 *
 * @retval NRF_SUCCESS                  RSSI started
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Not connected
 */
uint32_t ble_stack_rssi_start(void);

/**
 * @brief Get current RSSI
 *
 * @param[out] p_rssi  Pointer to store RSSI value (dBm)
 *
 * @retval NRF_SUCCESS                  RSSI retrieved
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Not connected
 * @retval NRF_ERROR_INVALID_ADDR       Invalid pointer
 */
uint32_t ble_stack_rssi_get(int8_t *p_rssi);

/**
 * @brief Register a connection event handler
 *
 * The handler is called when a connection is established or lost.
 *
 * @param[in] handler  Connection event handler function
 */
void ble_stack_conn_handler_set(ble_stack_conn_handler_t handler);

/**
 * @brief Register a service event handler
 *
 * Services should register their event handlers to receive BLE events.
 * Maximum BLE_STACK_MAX_SERVICE_HANDLERS handlers can be registered.
 *
 * @param[in] handler  Service event handler function
 *
 * @retval NRF_SUCCESS          Handler registered
 * @retval NRF_ERROR_NO_MEM     Too many handlers registered
 * @retval NRF_ERROR_NULL       Null handler
 */
uint32_t ble_stack_service_handler_register(ble_stack_service_handler_t handler);

/**
 * @brief BLE event handler (called by main event loop)
 *
 * This function dispatches BLE events to all registered handlers.
 * It should be registered with softdevice_ble_evt_handler_set().
 *
 * @param[in] p_ble_evt  Pointer to BLE event
 */
void ble_stack_evt_handler(const ble_evt_t *p_ble_evt);

/**
 * @brief Request PHY update to 2M PHY
 *
 * Requests higher throughput PHY if supported by central.
 *
 * @retval NRF_SUCCESS                  Request sent
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Not connected
 */
uint32_t ble_stack_phy_update_2m(void);

/**
 * @brief Request data length update
 *
 * Requests longer data packets for higher throughput.
 *
 * @retval NRF_SUCCESS                  Request sent
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Not connected
 */
uint32_t ble_stack_data_length_update(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BLE_STACK_H__ */
