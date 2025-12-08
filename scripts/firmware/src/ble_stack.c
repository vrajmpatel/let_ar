/**
 * @file ble_stack.c
 * @brief BLE Stack Management Implementation for S140 SoftDevice
 *
 * This module implements BLE stack management including GAP configuration,
 * connection management, and event dispatching.
 *
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x
 */

#include "ble_stack.h"
#include "softdevice.h"
#include "nrf_error.h"
#include <string.h>

/* Static state */
static ble_stack_conn_state_t m_conn_state;
static ble_stack_conn_handler_t m_conn_handler = NULL;
static ble_stack_service_handler_t m_service_handlers[BLE_STACK_MAX_SERVICE_HANDLERS];
static uint8_t m_service_handler_count = 0;
static bool m_initialized = false;

/* Default configuration */
static const ble_stack_config_t m_default_config = BLE_STACK_CONFIG_DEFAULT;

/**
 * @brief Initialize connection state
 */
static void conn_state_init(void)
{
    memset(&m_conn_state, 0, sizeof(m_conn_state));
    m_conn_state.conn_handle = BLE_CONN_HANDLE_INVALID;
    m_conn_state.connected = false;
}

/**
 * @brief Handle GAP connected event
 */
static void on_connected(const ble_gap_evt_connected_t *p_evt)
{
    m_conn_state.conn_handle = 0; /* Will be set from event conn_handle */
    m_conn_state.connected = true;
    memcpy(&m_conn_state.peer_addr, &p_evt->peer_addr, sizeof(ble_gap_addr_t));
    memcpy(&m_conn_state.conn_params, &p_evt->conn_params, sizeof(ble_gap_conn_params_t));
    m_conn_state.rssi = 0;
    
    /* Notify connection handler */
    if (m_conn_handler != NULL)
    {
        m_conn_handler(true, &m_conn_state);
    }
}

/**
 * @brief Handle GAP disconnected event
 */
static void on_disconnected(const ble_gap_evt_disconnected_t *p_evt)
{
    (void)p_evt;
    
    /* Store previous state for callback */
    ble_stack_conn_state_t prev_state = m_conn_state;
    
    /* Reset connection state */
    conn_state_init();
    
    /* Notify connection handler */
    if (m_conn_handler != NULL)
    {
        m_conn_handler(false, &prev_state);
    }
}

/**
 * @brief Handle GAP connection parameter update event
 */
static void on_conn_param_update(const ble_gap_evt_conn_param_update_t *p_evt)
{
    memcpy(&m_conn_state.conn_params, &p_evt->conn_params, sizeof(ble_gap_conn_params_t));
}

/**
 * @brief Handle GATTS system attributes missing event
 */
static void on_sys_attr_missing(uint16_t conn_handle)
{
    /* Set system attributes to NULL for a non-bonded connection */
    sd_ble_gatts_sys_attr_set(conn_handle, NULL, 0, 0);
}

/**
 * @brief Handle GATTS exchange MTU request
 */
static void on_exchange_mtu_request(uint16_t conn_handle, uint16_t client_rx_mtu)
{
    /* Reply with our preferred MTU 
     * Citation: Bluetooth Core Spec - MTU exchange allows both sides
     * to agree on a larger MTU than the default 23 bytes */
    uint16_t server_rx_mtu = BLE_GATT_ATT_MTU_DEFAULT;
    
    /* Use larger MTU if configured and client supports it */
    if (client_rx_mtu > server_rx_mtu)
    {
        server_rx_mtu = (client_rx_mtu > BLE_GATT_ATT_MTU_MAX) 
                        ? BLE_GATT_ATT_MTU_MAX 
                        : client_rx_mtu;
    }
    
    sd_ble_gatts_exchange_mtu_reply(conn_handle, server_rx_mtu);
}

uint32_t ble_stack_init(const ble_stack_config_t *p_config)
{
    uint32_t err_code;
    ble_gap_conn_sec_mode_t sec_mode;
    ble_gap_conn_params_t gap_conn_params;
    
    /* Use default config if none provided */
    if (p_config == NULL)
    {
        p_config = &m_default_config;
    }
    
    /* Check SoftDevice is enabled */
    if (!softdevice_is_enabled())
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    /* Initialize state */
    conn_state_init();
    m_service_handler_count = 0;
    memset(m_service_handlers, 0, sizeof(m_service_handlers));
    
    /* Set security mode to open (no security required)
     * Citation: Bluetooth Core Spec - Security Mode 1, Level 1 = no security */
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    /* Set device name */
    if (p_config->device_name != NULL)
    {
        err_code = sd_ble_gap_device_name_set(
            &sec_mode,
            (const uint8_t *)p_config->device_name,
            strlen(p_config->device_name));
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    
    /* Set appearance */
    err_code = sd_ble_gap_appearance_set(p_config->appearance);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Set Peripheral Preferred Connection Parameters (PPCP)
     * Citation: Bluetooth Core Spec Vol 3, Part C, Section 12.3
     * These are advertised to the central as preferred values */
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));
    
    /* Use provided values or defaults */
    gap_conn_params.min_conn_interval = (p_config->min_conn_interval > 0) 
                                        ? p_config->min_conn_interval 
                                        : 24;  /* 30ms default */
    gap_conn_params.max_conn_interval = (p_config->max_conn_interval > 0) 
                                        ? p_config->max_conn_interval 
                                        : 60;  /* 75ms default */
    gap_conn_params.slave_latency = p_config->slave_latency;
    gap_conn_params.conn_sup_timeout = (p_config->conn_sup_timeout > 0) 
                                       ? p_config->conn_sup_timeout 
                                       : 400; /* 4s default */
    
    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Register as BLE event handler */
    softdevice_ble_evt_handler_set(ble_stack_evt_handler);
    
    m_initialized = true;
    
    return NRF_SUCCESS;
}

const ble_stack_conn_state_t *ble_stack_conn_state_get(void)
{
    return &m_conn_state;
}

bool ble_stack_is_connected(void)
{
    return m_conn_state.connected;
}

uint16_t ble_stack_conn_handle_get(void)
{
    return m_conn_state.conn_handle;
}

uint32_t ble_stack_disconnect(void)
{
    if (!m_conn_state.connected)
    {
        return BLE_ERROR_INVALID_CONN_HANDLE;
    }
    
    /* Disconnect with reason: Remote User Terminated Connection
     * Citation: Bluetooth Core Spec Vol 2, Part D, Section 1.3 - HCI Error Codes */
    return sd_ble_gap_disconnect(m_conn_state.conn_handle, 
                                  BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
}

uint32_t ble_stack_conn_param_update(uint16_t min_interval, uint16_t max_interval,
                                      uint16_t latency, uint16_t timeout)
{
    ble_gap_conn_params_t params;
    
    if (!m_conn_state.connected)
    {
        return BLE_ERROR_INVALID_CONN_HANDLE;
    }
    
    params.min_conn_interval = min_interval;
    params.max_conn_interval = max_interval;
    params.slave_latency = latency;
    params.conn_sup_timeout = timeout;
    
    return sd_ble_gap_conn_param_update(m_conn_state.conn_handle, &params);
}

uint32_t ble_stack_tx_power_set(int8_t tx_power)
{
    /* Set TX power for advertising */
    uint32_t err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, 0, tx_power);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Set TX power for connection if connected */
    if (m_conn_state.connected)
    {
        err_code = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_CONN, 
                                           m_conn_state.conn_handle, 
                                           tx_power);
    }
    
    return err_code;
}

uint32_t ble_stack_rssi_start(void)
{
    if (!m_conn_state.connected)
    {
        return BLE_ERROR_INVALID_CONN_HANDLE;
    }
    
    /* Start RSSI measurements with:
     * - threshold: 0 dBm (report all changes)
     * - skip_count: 10 (report every 10th measurement)
     */
    return sd_ble_gap_rssi_start(m_conn_state.conn_handle, 0, 10);
}

uint32_t ble_stack_rssi_get(int8_t *p_rssi)
{
    if (p_rssi == NULL)
    {
        return NRF_ERROR_INVALID_ADDR;
    }
    
    if (!m_conn_state.connected)
    {
        return BLE_ERROR_INVALID_CONN_HANDLE;
    }
    
    return sd_ble_gap_rssi_get(m_conn_state.conn_handle, p_rssi, NULL);
}

void ble_stack_conn_handler_set(ble_stack_conn_handler_t handler)
{
    m_conn_handler = handler;
}

uint32_t ble_stack_service_handler_register(ble_stack_service_handler_t handler)
{
    if (handler == NULL)
    {
        return NRF_ERROR_NULL;
    }
    
    if (m_service_handler_count >= BLE_STACK_MAX_SERVICE_HANDLERS)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    m_service_handlers[m_service_handler_count++] = handler;
    return NRF_SUCCESS;
}

void ble_stack_evt_handler(const ble_evt_t *p_ble_evt)
{
    uint16_t conn_handle;
    uint8_t i;
    
    if (p_ble_evt == NULL)
    {
        return;
    }
    
    /* Extract connection handle based on event type */
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        case BLE_GAP_EVT_DISCONNECTED:
        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
        case BLE_GAP_EVT_PHY_UPDATE:
        case BLE_GAP_EVT_DATA_LENGTH_UPDATE:
            conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
            
        case BLE_GATTS_EVT_WRITE:
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        case BLE_GATTS_EVT_HVC:
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
            break;
            
        default:
            conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
    }
    
    /* Handle core GAP/GATTS events */
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_state.conn_handle = conn_handle;
            on_connected(&p_ble_evt->evt.gap_evt.params.connected);
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnected(&p_ble_evt->evt.gap_evt.params.disconnected);
            break;
            
        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
            on_conn_param_update(&p_ble_evt->evt.gap_evt.params.conn_param_update);
            break;
            
        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            /* Accept the connection parameter update request from central */
            sd_ble_gap_conn_param_update(
                conn_handle, 
                &p_ble_evt->evt.gap_evt.params.conn_param_update_request.conn_params);
            break;
            
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            /* Accept the PHY update request
             * Citation: Bluetooth Core Spec Vol 6, Part B, Section 4.6.2
             * PHY update allows using 2M PHY for higher throughput */
            ble_gap_phys_t phys = {
                .tx_phys = BLE_GAP_PHY_AUTO,
                .rx_phys = BLE_GAP_PHY_AUTO,
            };
            sd_ble_gap_phy_update(conn_handle, &phys);
            break;
        }
            
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            /* Reject pairing request (no bonding implemented)
             * Citation: Bluetooth Core Spec Vol 3, Part H - Security Manager Protocol */
            sd_ble_gap_sec_params_reply(conn_handle, 
                                        BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, 
                                        NULL, NULL);
            break;
            
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            on_sys_attr_missing(conn_handle);
            break;
            
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            on_exchange_mtu_request(conn_handle, 
                                   p_ble_evt->evt.gatts_evt.params.exchange_mtu_request.client_rx_mtu);
            break;
            
        default:
            break;
    }
    
    /* Dispatch to registered service handlers */
    for (i = 0; i < m_service_handler_count; i++)
    {
        if (m_service_handlers[i] != NULL)
        {
            m_service_handlers[i](p_ble_evt);
        }
    }
}

uint32_t ble_stack_phy_update_2m(void)
{
    ble_gap_phys_t phys;
    
    if (!m_conn_state.connected)
    {
        return BLE_ERROR_INVALID_CONN_HANDLE;
    }
    
    /* Request 2M PHY for both TX and RX
     * Citation: Bluetooth Core Spec Vol 6, Part B, Section 4.6
     * 2M PHY provides double the data rate of 1M PHY */
    phys.tx_phys = BLE_GAP_PHY_2MBPS;
    phys.rx_phys = BLE_GAP_PHY_2MBPS;
    
    return sd_ble_gap_phy_update(m_conn_state.conn_handle, &phys);
}

uint32_t ble_stack_data_length_update(void)
{
    if (!m_conn_state.connected)
    {
        return BLE_ERROR_INVALID_CONN_HANDLE;
    }
    
    /* Request maximum data length
     * Citation: Bluetooth Core Spec Vol 6, Part B, Section 4.5.10
     * Data Length Extension (DLE) allows up to 251 bytes per packet */
    return sd_ble_gap_data_length_update(m_conn_state.conn_handle, NULL, NULL);
}
