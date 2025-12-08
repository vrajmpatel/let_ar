/**
 * @file softdevice.c
 * @brief SoftDevice S140 Initialization and Management Implementation
 *
 * This module implements the SoftDevice initialization and management
 * functions for the S140 SoftDevice on nRF52840.
 *
 * Memory Layout (S140 6.1.1):
 *   Citation: Nordic DevZone - "S140 6.1.1 FLASH_START=0x26000, Minimum RAM Start 0x20001628"
 *
 * Initialization Sequence:
 *   1. Configure and enable SoftDevice (sd_softdevice_enable)
 *   2. Set BLE configuration options (sd_ble_cfg_set)
 *   3. Enable BLE stack (sd_ble_enable)
 *
 * Reference: Nordic nRF5 SDK 17.x, S140 SoftDevice Specification
 */

#include "softdevice.h"
#include "nrf52840.h"
#include <string.h>

/* Event buffer size - sized for maximum MTU plus event overhead */
#define BLE_EVT_BUFFER_SIZE     256

/* Static variables */
static bool m_softdevice_enabled = false;
static uint32_t m_app_ram_base = 0;
static ble_evt_handler_t m_evt_handler = NULL;

/* Event buffer (word-aligned for SoftDevice) */
static uint32_t m_evt_buffer[(BLE_EVT_BUFFER_SIZE + 3) / 4];

/* Default configuration */
static const softdevice_config_t m_default_config = SOFTDEVICE_CONFIG_DEFAULT;

/**
 * @brief Internal fault handler - called by SoftDevice on fatal errors
 */
static void sd_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    /* Call application fault handler */
    softdevice_fault_handler(id, pc, info);
}

/**
 * @brief Configure BLE stack before enabling
 */
static uint32_t ble_stack_configure(const softdevice_config_t *p_config, uint32_t *p_ram_base)
{
    uint32_t err_code;
    ble_cfg_t ble_cfg;
    
    /* Start with minimum required RAM base for S140 6.1.1 */
    /* Citation: Nordic DevZone - "Minimum RAM Start 0x20001628" */
    uint32_t ram_base = 0x20001628;
    
    /* Configure GAP connection count */
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.params.gap_role_count.periph_role_count = p_config->periph_conn_count;
    ble_cfg.params.gap_role_count.central_role_count = p_config->central_conn_count;
    ble_cfg.params.gap_role_count.central_sec_count = 0;
    ble_cfg.params.gap_role_count.adv_set_count = 1;
    
    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_base);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_NO_MEM)
    {
        return err_code;
    }
    
    /* Configure GAP connection parameters */
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.params.conn_cfg.conn_cfg_tag = BLE_CONN_CFG_TAG_DEFAULT;
    ble_cfg.params.conn_cfg.params.gap_conn_cfg.conn_count = 
        p_config->periph_conn_count + p_config->central_conn_count;
    ble_cfg.params.conn_cfg.params.gap_conn_cfg.event_length = 6; /* 7.5ms event length */
    
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &ble_cfg, ram_base);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_NO_MEM)
    {
        return err_code;
    }
    
    /* Configure ATT MTU */
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.params.conn_cfg.conn_cfg_tag = BLE_CONN_CFG_TAG_DEFAULT;
    ble_cfg.params.conn_cfg.params.gatt_conn_cfg.att_mtu = p_config->att_mtu;
    
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATT, &ble_cfg, ram_base);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_NO_MEM)
    {
        return err_code;
    }
    
    /* Configure vendor-specific UUID count */
    if (p_config->vs_uuid_count > 0)
    {
        memset(&ble_cfg, 0, sizeof(ble_cfg));
        ble_cfg.params.common_cfg.vs_uuid_count = p_config->vs_uuid_count;
        
        err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_base);
        if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_NO_MEM)
        {
            return err_code;
        }
    }
    
    /* Configure GATTS attribute table size */
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.params.gatts_attr_tab_size.attr_tab_size = p_config->attr_tab_size;
    
    err_code = sd_ble_cfg_set(BLE_GATTS_CFG_ATTR_TAB_SIZE, &ble_cfg, ram_base);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_NO_MEM)
    {
        return err_code;
    }
    
    /* Configure Service Changed characteristic */
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.params.gatts_service_changed.service_changed = p_config->service_changed ? 1 : 0;
    
    err_code = sd_ble_cfg_set(BLE_GATTS_CFG_SERVICE_CHANGED, &ble_cfg, ram_base);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_NO_MEM)
    {
        return err_code;
    }
    
    /* Configure GATTS HVN TX queue */
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.params.conn_cfg.conn_cfg_tag = BLE_CONN_CFG_TAG_DEFAULT;
    ble_cfg.params.conn_cfg.params.gatts_conn_cfg.hvn_tx_queue_size = 4;
    
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATTS, &ble_cfg, ram_base);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_NO_MEM)
    {
        return err_code;
    }
    
    *p_ram_base = ram_base;
    return NRF_SUCCESS;
}

uint32_t softdevice_init(const softdevice_config_t *p_config)
{
    uint32_t err_code;
    nrf_clock_lf_cfg_t clock_cfg;
    
    /* Use default config if none provided */
    if (p_config == NULL)
    {
        p_config = &m_default_config;
    }
    
    /* Check if already enabled */
    if (m_softdevice_enabled)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    /* Configure low frequency clock */
    clock_cfg.source = p_config->lfclk_source;
    clock_cfg.rc_ctiv = p_config->rc_ctiv;
    clock_cfg.rc_temp_ctiv = p_config->rc_temp_ctiv;
    clock_cfg.accuracy = p_config->lfclk_accuracy;
    
    /* Enable SoftDevice
     * Citation: Nordic SDK - sd_softdevice_enable() must be called first
     * before any other SoftDevice API functions */
    err_code = sd_softdevice_enable(&clock_cfg, sd_fault_handler);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Enable DC/DC converter if requested
     * Citation: nRF52840 Product Specification - DC/DC requires external inductor
     * Reduces current consumption significantly */
    if (p_config->dcdc_enabled)
    {
        err_code = sd_power_dcdc_mode_set(1);
        if (err_code != NRF_SUCCESS)
        {
            sd_softdevice_disable();
            return err_code;
        }
    }
    
    /* Configure BLE stack */
    err_code = ble_stack_configure(p_config, &m_app_ram_base);
    if (err_code != NRF_SUCCESS)
    {
        sd_softdevice_disable();
        return err_code;
    }
    
    /* Enable BLE stack
     * Citation: Nordic SDK - sd_ble_enable() must be called after all
     * sd_ble_cfg_set() calls and before any other BLE API functions */
    err_code = sd_ble_enable(&m_app_ram_base);
    if (err_code != NRF_SUCCESS)
    {
        sd_softdevice_disable();
        return err_code;
    }
    
    m_softdevice_enabled = true;
    
    return NRF_SUCCESS;
}

uint32_t softdevice_disable(void)
{
    uint32_t err_code;
    
    if (!m_softdevice_enabled)
    {
        return NRF_SUCCESS;
    }
    
    err_code = sd_softdevice_disable();
    if (err_code == NRF_SUCCESS)
    {
        m_softdevice_enabled = false;
        m_app_ram_base = 0;
    }
    
    return err_code;
}

bool softdevice_is_enabled(void)
{
    return m_softdevice_enabled;
}

uint32_t softdevice_app_ram_base_get(void)
{
    return m_app_ram_base;
}

void softdevice_evt_process(void)
{
    uint32_t err_code;
    uint16_t evt_len;
    
    if (!m_softdevice_enabled)
    {
        return;
    }
    
    /* Process all pending events */
    while (1)
    {
        evt_len = sizeof(m_evt_buffer);
        err_code = sd_ble_evt_get((uint8_t *)m_evt_buffer, &evt_len);
        
        if (err_code == NRF_ERROR_NOT_FOUND)
        {
            /* No more events */
            break;
        }
        
        if (err_code != NRF_SUCCESS)
        {
            /* Error getting event - stop processing */
            break;
        }
        
        /* Dispatch event to handler */
        if (m_evt_handler != NULL)
        {
            m_evt_handler((const ble_evt_t *)m_evt_buffer);
        }
    }
}

void softdevice_ble_evt_handler_set(ble_evt_handler_t handler)
{
    m_evt_handler = handler;
}

/**
 * @brief Default SoftDevice fault handler (weak)
 *
 * Can be overridden by application.
 */
__attribute__((weak)) void softdevice_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    (void)id;
    (void)pc;
    (void)info;
    
    /* Default action: perform system reset */
    sd_nvic_SystemReset();
}

uint32_t softdevice_hfclk_request(void)
{
    if (!m_softdevice_enabled)
    {
        return NRF_ERROR_SOFTDEVICE_NOT_ENABLED;
    }
    return sd_clock_hfclk_request();
}

uint32_t softdevice_hfclk_release(void)
{
    if (!m_softdevice_enabled)
    {
        return NRF_ERROR_SOFTDEVICE_NOT_ENABLED;
    }
    return sd_clock_hfclk_release();
}

bool softdevice_hfclk_is_running(void)
{
    uint32_t is_running = 0;
    
    if (!m_softdevice_enabled)
    {
        return false;
    }
    
    sd_clock_hfclk_is_running(&is_running);
    return (is_running != 0);
}

void softdevice_wait_for_event(void)
{
    if (m_softdevice_enabled)
    {
        sd_app_evt_wait();
    }
    else
    {
        /* Use WFE directly when SoftDevice is disabled */
        __WFE();
    }
}

uint32_t softdevice_rand_get(uint8_t *p_buff, uint8_t length)
{
    if (!m_softdevice_enabled)
    {
        return NRF_ERROR_SOFTDEVICE_NOT_ENABLED;
    }
    return sd_rand_application_vector_get(p_buff, length);
}

uint32_t softdevice_temp_get(float *p_temp_degc)
{
    uint32_t err_code;
    int32_t temp_raw;
    
    if (!m_softdevice_enabled)
    {
        return NRF_ERROR_SOFTDEVICE_NOT_ENABLED;
    }
    
    if (p_temp_degc == NULL)
    {
        return NRF_ERROR_NULL;
    }
    
    /* sd_temp_get returns temperature in 0.25°C units */
    err_code = sd_temp_get(&temp_raw);
    if (err_code == NRF_SUCCESS)
    {
        *p_temp_degc = (float)temp_raw * 0.25f;
    }
    
    return err_code;
}

void softdevice_critical_region_enter(uint8_t *p_nested)
{
    if (m_softdevice_enabled)
    {
        sd_nvic_critical_region_enter(p_nested);
    }
    else
    {
        /* Disable interrupts directly */
        __disable_irq();
        *p_nested = 0;
    }
}

void softdevice_critical_region_exit(uint8_t nested)
{
    if (m_softdevice_enabled)
    {
        sd_nvic_critical_region_exit(nested);
    }
    else
    {
        /* Re-enable interrupts directly */
        __enable_irq();
    }
}

/**
 * @brief Application error handler (weak)
 *
 * Called by APP_ERROR_CHECK macro. Can be overridden by application.
 */
__attribute__((weak)) void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t *p_file_name)
{
    (void)error_code;
    (void)line_num;
    (void)p_file_name;
    
    /* Default action: infinite loop (for debugging) */
    __disable_irq();
    while (1)
    {
        /* Trapped in error handler - connect debugger to inspect */
        __NOP();
    }
}
