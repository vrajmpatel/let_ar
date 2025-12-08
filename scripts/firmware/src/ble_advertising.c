/**
 * @file ble_advertising.c
 * @brief BLE Advertising Module Implementation for S140 SoftDevice
 *
 * This module implements BLE advertising functionality for the S140 SoftDevice.
 *
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x Vol 6 Part B
 */

#include "ble_advertising.h"
#include "ble_stack.h"
#include "nrf_error.h"
#include <string.h>

/* Advertising data buffers (must persist while advertising) */
static uint8_t m_adv_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
static uint8_t m_scan_rsp_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];

/* Advertising handle */
static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;

/* Module state */
static bool m_initialized = false;
static ble_adv_mode_t m_adv_mode = BLE_ADV_MODE_IDLE;
static ble_advertising_config_t m_config;
static ble_advertising_evt_handler_t m_evt_handler = NULL;

/* Advertising data structure for SoftDevice */
static ble_gap_adv_data_t m_adv_data_struct;

/**
 * @brief Build advertising data packet
 *
 * Citation: Bluetooth Core Spec Vol 3, Part C, Section 11 - Advertising Data Format
 * Each field is: [Length] [Type] [Data...]
 */
static uint32_t adv_data_build(void)
{
    uint8_t *p = m_adv_data;
    uint8_t *p_sr = m_scan_rsp_data;
    uint8_t len = 0;
    uint8_t sr_len = 0;
    uint8_t i;
    
    /* Add Flags field (mandatory for connectable advertising)
     * Citation: Bluetooth Core Spec Vol 3, Part C, Section 11.1.3 */
    if (len + 3 <= BLE_GAP_ADV_SET_DATA_SIZE_MAX)
    {
        p[len++] = 2;  /* Length of flags field */
        p[len++] = BLE_GAP_AD_TYPE_FLAGS;
        p[len++] = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    }
    
    /* Add 16-bit Service UUIDs
     * Citation: Bluetooth Core Spec Vol 3, Part C, Section 11.1.1 */
    if (m_config.uuid_count > 0)
    {
        /* Calculate space needed */
        uint8_t uuid_field_len = 1 + (m_config.uuid_count * 2);
        
        if (len + 1 + uuid_field_len <= BLE_GAP_ADV_SET_DATA_SIZE_MAX)
        {
            p[len++] = uuid_field_len;
            p[len++] = BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE;
            
            for (i = 0; i < m_config.uuid_count; i++)
            {
                if (m_config.uuids[i].type == BLE_UUID_TYPE_BLE)
                {
                    /* 16-bit UUID - little endian */
                    p[len++] = (uint8_t)(m_config.uuids[i].uuid & 0xFF);
                    p[len++] = (uint8_t)(m_config.uuids[i].uuid >> 8);
                }
            }
        }
    }
    
    /* Add TX Power Level if requested */
    if (m_config.include_tx_power)
    {
        if (len + 3 <= BLE_GAP_ADV_SET_DATA_SIZE_MAX)
        {
            p[len++] = 2;
            p[len++] = BLE_GAP_AD_TYPE_TX_POWER_LEVEL;
            p[len++] = 0;  /* 0 dBm - actual value set by sd_ble_gap_tx_power_set */
        }
    }
    
    /* Add Appearance if requested */
    if (m_config.include_appearance)
    {
        uint16_t appearance = BLE_APPEARANCE_GENERIC_TAG;
        sd_ble_gap_appearance_get(&appearance);
        
        if (len + 4 <= BLE_GAP_ADV_SET_DATA_SIZE_MAX)
        {
            p[len++] = 3;
            p[len++] = BLE_GAP_AD_TYPE_APPEARANCE;
            p[len++] = (uint8_t)(appearance & 0xFF);
            p[len++] = (uint8_t)(appearance >> 8);
        }
    }
    
    /* Add Manufacturer Specific Data if provided */
    if (m_config.p_manuf_data != NULL && m_config.manuf_data_len > 0)
    {
        uint8_t manuf_field_len = 2 + m_config.manuf_data_len;  /* Company ID + data */
        
        if (len + 1 + manuf_field_len <= BLE_GAP_ADV_SET_DATA_SIZE_MAX)
        {
            p[len++] = manuf_field_len + 1;  /* +1 for type */
            p[len++] = BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA;
            /* Company ID - little endian */
            p[len++] = (uint8_t)(m_config.company_id & 0xFF);
            p[len++] = (uint8_t)(m_config.company_id >> 8);
            memcpy(&p[len], m_config.p_manuf_data, m_config.manuf_data_len);
            len += m_config.manuf_data_len;
        }
    }
    
    /* Add Device Name
     * Citation: Bluetooth Core Spec Vol 3, Part C, Section 11.1.2 */
    if (m_config.include_name)
    {
        uint8_t name[BLE_STACK_DEVICE_NAME_MAX_LEN];
        uint16_t name_len = sizeof(name);
        
        if (sd_ble_gap_device_name_get(name, &name_len) == NRF_SUCCESS && name_len > 0)
        {
            /* Try to fit complete name */
            if (len + 2 + name_len <= BLE_GAP_ADV_SET_DATA_SIZE_MAX)
            {
                p[len++] = name_len + 1;
                p[len++] = BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME;
                memcpy(&p[len], name, name_len);
                len += name_len;
            }
            /* Otherwise, use shortened name */
            else
            {
                uint8_t available = BLE_GAP_ADV_SET_DATA_SIZE_MAX - len - 2;
                if (available > 0)
                {
                    p[len++] = available + 1;
                    p[len++] = BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME;
                    memcpy(&p[len], name, available);
                    len += available;
                }
            }
        }
    }
    else if (m_config.include_name_in_sr)
    {
        /* Put name in scan response instead */
        uint8_t name[BLE_STACK_DEVICE_NAME_MAX_LEN];
        uint16_t name_len = sizeof(name);
        
        if (sd_ble_gap_device_name_get(name, &name_len) == NRF_SUCCESS && name_len > 0)
        {
            if (sr_len + 2 + name_len <= BLE_GAP_ADV_SET_DATA_SIZE_MAX)
            {
                p_sr[sr_len++] = name_len + 1;
                p_sr[sr_len++] = BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME;
                memcpy(&p_sr[sr_len], name, name_len);
                sr_len += name_len;
            }
        }
    }
    
    /* Set up advertising data structure */
    m_adv_data_struct.adv_data.p_data = m_adv_data;
    m_adv_data_struct.adv_data.len = len;
    m_adv_data_struct.scan_rsp_data.p_data = (sr_len > 0) ? m_scan_rsp_data : NULL;
    m_adv_data_struct.scan_rsp_data.len = sr_len;
    
    return NRF_SUCCESS;
}

/**
 * @brief Configure advertising set
 */
static uint32_t adv_set_configure(ble_adv_mode_t mode)
{
    uint32_t err_code;
    ble_gap_adv_params_t adv_params;
    
    memset(&adv_params, 0, sizeof(adv_params));
    
    /* Configure advertising type
     * Citation: Bluetooth Core Spec Vol 6, Part B, Section 4.4.2
     * ADV_IND = Connectable, scannable, undirected */
    adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    adv_params.properties.anonymous = 0;
    
    /* Set advertising interval based on mode */
    switch (mode)
    {
        case BLE_ADV_MODE_FAST:
            adv_params.interval = m_config.fast_interval;
            adv_params.duration = m_config.fast_timeout;
            break;
            
        case BLE_ADV_MODE_SLOW:
            adv_params.interval = m_config.slow_interval;
            adv_params.duration = m_config.slow_timeout;
            break;
            
        default:
            return NRF_ERROR_INVALID_PARAM;
    }
    
    /* Use primary 1M PHY (most compatible) */
    adv_params.primary_phy = BLE_GAP_PHY_1MBPS;
    adv_params.secondary_phy = BLE_GAP_PHY_1MBPS;
    
    /* Filter policy: accept all */
    adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
    
    /* No peer address (undirected) */
    adv_params.p_peer_addr = NULL;
    
    /* Build advertising data */
    err_code = adv_data_build();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Configure advertising set */
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &adv_params, &m_adv_data_struct);
    
    return err_code;
}

uint32_t ble_advertising_init(const ble_advertising_config_t *p_config)
{
    /* Use default config if none provided */
    if (p_config == NULL)
    {
        static const ble_advertising_config_t default_config = BLE_ADVERTISING_CONFIG_DEFAULT;
        p_config = &default_config;
    }
    
    /* Copy configuration */
    memcpy(&m_config, p_config, sizeof(m_config));
    
    /* Reset state */
    m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
    m_adv_mode = BLE_ADV_MODE_IDLE;
    m_initialized = true;
    
    /* Register as service handler to receive BLE events */
    ble_stack_service_handler_register(ble_advertising_on_ble_evt);
    
    return NRF_SUCCESS;
}

uint32_t ble_advertising_uuid_add(const ble_uuid_t *p_uuid)
{
    if (p_uuid == NULL)
    {
        return NRF_ERROR_NULL;
    }
    
    if (m_config.uuid_count >= BLE_ADV_MAX_UUIDS)
    {
        return NRF_ERROR_NO_MEM;
    }
    
    memcpy(&m_config.uuids[m_config.uuid_count], p_uuid, sizeof(ble_uuid_t));
    m_config.uuid_count++;
    
    return NRF_SUCCESS;
}

uint32_t ble_advertising_manuf_data_set(uint16_t company_id, const uint8_t *p_data, uint8_t len)
{
    if (len > (BLE_GAP_ADV_SET_DATA_SIZE_MAX - 10))  /* Reserve space for other fields */
    {
        return NRF_ERROR_INVALID_LENGTH;
    }
    
    m_config.company_id = company_id;
    m_config.p_manuf_data = (uint8_t *)p_data;
    m_config.manuf_data_len = len;
    
    return NRF_SUCCESS;
}

uint32_t ble_advertising_start(void)
{
    uint32_t err_code;
    ble_adv_mode_t start_mode;
    
    if (!m_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    if (m_adv_mode != BLE_ADV_MODE_IDLE)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    /* Determine starting mode */
    start_mode = (m_config.fast_timeout > 0) ? BLE_ADV_MODE_FAST : BLE_ADV_MODE_SLOW;
    
    /* Configure advertising set */
    err_code = adv_set_configure(start_mode);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Start advertising */
    err_code = sd_ble_gap_adv_start(m_adv_handle, BLE_CONN_CFG_TAG_DEFAULT);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    m_adv_mode = start_mode;
    
    /* Notify handler */
    if (m_evt_handler != NULL)
    {
        m_evt_handler(BLE_ADV_EVT_STARTED, m_adv_mode);
    }
    
    return NRF_SUCCESS;
}

uint32_t ble_advertising_stop(void)
{
    uint32_t err_code;
    
    if (m_adv_mode == BLE_ADV_MODE_IDLE)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    err_code = sd_ble_gap_adv_stop(m_adv_handle);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE)
    {
        return err_code;
    }
    
    m_adv_mode = BLE_ADV_MODE_IDLE;
    
    /* Notify handler */
    if (m_evt_handler != NULL)
    {
        m_evt_handler(BLE_ADV_EVT_STOPPED, m_adv_mode);
    }
    
    return NRF_SUCCESS;
}

bool ble_advertising_is_active(void)
{
    return (m_adv_mode != BLE_ADV_MODE_IDLE);
}

ble_adv_mode_t ble_advertising_mode_get(void)
{
    return m_adv_mode;
}

void ble_advertising_evt_handler_set(ble_advertising_evt_handler_t handler)
{
    m_evt_handler = handler;
}

void ble_advertising_on_ble_evt(const ble_evt_t *p_ble_evt)
{
    uint32_t err_code;
    
    if (p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            /* Connection established - advertising stopped automatically */
            m_adv_mode = BLE_ADV_MODE_IDLE;
            
            if (m_evt_handler != NULL)
            {
                m_evt_handler(BLE_ADV_EVT_CONNECTED, m_adv_mode);
            }
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            /* Connection lost - restart advertising if enabled */
            if (m_config.auto_restart && m_initialized)
            {
                err_code = ble_advertising_start();
                (void)err_code;  /* Ignore error - best effort restart */
            }
            break;
            
        case BLE_GAP_EVT_ADV_SET_TERMINATED:
            /* Advertising duration timeout */
            if (p_ble_evt->evt.gap_evt.params.adv_set_terminated.reason == 0x00)
            {
                /* Timeout - switch to slow mode if in fast mode */
                if (m_adv_mode == BLE_ADV_MODE_FAST)
                {
                    if (m_evt_handler != NULL)
                    {
                        m_evt_handler(BLE_ADV_EVT_FAST_TIMEOUT, m_adv_mode);
                    }
                    
                    /* Switch to slow advertising */
                    err_code = adv_set_configure(BLE_ADV_MODE_SLOW);
                    if (err_code == NRF_SUCCESS)
                    {
                        err_code = sd_ble_gap_adv_start(m_adv_handle, BLE_CONN_CFG_TAG_DEFAULT);
                        if (err_code == NRF_SUCCESS)
                        {
                            m_adv_mode = BLE_ADV_MODE_SLOW;
                        }
                    }
                }
                else if (m_adv_mode == BLE_ADV_MODE_SLOW)
                {
                    /* Slow timeout */
                    m_adv_mode = BLE_ADV_MODE_IDLE;
                    
                    if (m_evt_handler != NULL)
                    {
                        m_evt_handler(BLE_ADV_EVT_SLOW_TIMEOUT, m_adv_mode);
                    }
                }
            }
            else if (p_ble_evt->evt.gap_evt.params.adv_set_terminated.reason == 0x05)
            {
                /* Connected - handled by BLE_GAP_EVT_CONNECTED */
            }
            break;
            
        default:
            break;
    }
}

uint32_t ble_advertising_data_update(void)
{
    uint32_t err_code;
    
    if (m_adv_mode == BLE_ADV_MODE_IDLE)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    /* Rebuild advertising data */
    err_code = adv_data_build();
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Update advertising set (can update while advertising) */
    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, NULL, &m_adv_data_struct);
    
    return err_code;
}

uint32_t ble_advertising_restart(void)
{
    if (m_adv_mode != BLE_ADV_MODE_IDLE)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    return ble_advertising_start();
}

uint32_t ble_advertising_mode_set(ble_adv_mode_t mode)
{
    uint32_t err_code;
    
    if (mode == BLE_ADV_MODE_IDLE)
    {
        return ble_advertising_stop();
    }
    
    if (mode != BLE_ADV_MODE_FAST && mode != BLE_ADV_MODE_SLOW)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    /* Stop current advertising if active */
    if (m_adv_mode != BLE_ADV_MODE_IDLE)
    {
        sd_ble_gap_adv_stop(m_adv_handle);
    }
    
    /* Configure and start new mode */
    err_code = adv_set_configure(mode);
    if (err_code != NRF_SUCCESS)
    {
        m_adv_mode = BLE_ADV_MODE_IDLE;
        return err_code;
    }
    
    err_code = sd_ble_gap_adv_start(m_adv_handle, BLE_CONN_CFG_TAG_DEFAULT);
    if (err_code != NRF_SUCCESS)
    {
        m_adv_mode = BLE_ADV_MODE_IDLE;
        return err_code;
    }
    
    m_adv_mode = mode;
    
    return NRF_SUCCESS;
}
