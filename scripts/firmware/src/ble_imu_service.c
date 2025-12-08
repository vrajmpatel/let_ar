/**
 * @file ble_imu_service.c
 * @brief Custom BLE GATT Service for IMU data - Full S140 Implementation
 * 
 * BLE service implementation for streaming IMU sensor data from the
 * BNO085 to connected BLE central devices using S140 SoftDevice.
 * 
 * Citations:
 * - FIRMWARE_DESIGN.md Section "BLE Service Design"
 * - nRF52840_PS_v1.11.pdf: BLE 5.0 features
 * - S140 SoftDevice Specification
 */

#include "ble_imu_service.h"
#include "ble_stack.h"
#include "nrf_error.h"
#include <string.h>

/*******************************************************************************
 * Private Definitions
 ******************************************************************************/

/* Default sample rate in milliseconds */
#define DEFAULT_SAMPLE_RATE_MS      10

/* 128-bit UUID base for the IMU service */
static const ble_uuid128_t m_uuid_base = { BLE_IMU_UUID_BASE };

/* Static pointer to service instance for event handler callback
 * This is needed because ble_stack_service_handler_t only passes the event,
 * not a context pointer. The service instance pointer is set during init.
 */
static ble_imu_service_t *mp_service_instance = NULL;

/*******************************************************************************
 * Private Helper Functions
 ******************************************************************************/

/**
 * @brief Add a characteristic with notification support
 *
 * @param[in]  service       Service structure
 * @param[in]  uuid          16-bit UUID for the characteristic
 * @param[in]  p_init_value  Initial value (can be NULL)
 * @param[in]  value_len     Length of the characteristic value
 * @param[in]  can_notify    True if characteristic supports notifications
 * @param[in]  can_write     True if characteristic is writable
 * @param[out] p_handles     Pointer to store characteristic handles
 */
static uint32_t char_add(ble_imu_service_t *service,
                         uint16_t uuid,
                         const uint8_t *p_init_value,
                         uint16_t value_len,
                         bool can_notify,
                         bool can_write,
                         ble_gatts_char_handles_t *p_handles)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          char_uuid;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_md_t cccd_md;
    
    /* Initialize characteristic metadata */
    memset(&char_md, 0, sizeof(char_md));
    
    /* Set characteristic properties 
     * Citation: Bluetooth Core Spec Vol 3, Part G, Section 3.3.1.1 */
    char_md.char_props.read = 1;  /* All our characteristics are readable */
    char_md.char_props.notify = can_notify ? 1 : 0;
    char_md.char_props.write = can_write ? 1 : 0;
    
    /* Configure CCCD for notification characteristics 
     * Citation: Bluetooth Core Spec Vol 3, Part G, Section 3.3.3.3 */
    if (can_notify)
    {
        memset(&cccd_md, 0, sizeof(cccd_md));
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
        cccd_md.vloc = BLE_GATTS_VLOC_STACK;  /* CCCD value stored in stack */
        char_md.p_cccd_md = &cccd_md;
    }
    
    /* Set up attribute metadata */
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    
    if (can_write)
    {
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    }
    else
    {
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    }
    
    attr_md.vloc = BLE_GATTS_VLOC_STACK;  /* Value stored in SoftDevice */
    attr_md.vlen = 0;  /* Fixed length attribute */
    
    /* Set up characteristic UUID */
    char_uuid.type = service->uuid_type;
    char_uuid.uuid = uuid;
    
    /* Set up attribute value */
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid = &char_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = (p_init_value != NULL) ? value_len : 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = value_len;
    attr_char_value.p_value = (uint8_t *)p_init_value;
    
    /* Add characteristic to service 
     * Citation: Nordic SDK - sd_ble_gatts_characteristic_add() */
    return sd_ble_gatts_characteristic_add(service->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           p_handles);
}

/**
 * @brief Handle write events to characteristics
 */
static void on_write(ble_imu_service_t *service, const ble_gatts_evt_write_t *p_evt)
{
    ble_imu_evt_t evt;
    
    /* Check for CCCD writes (notification enable/disable)
     * Citation: Bluetooth Core Spec Vol 3, Part G, Section 3.3.3.3
     * CCCD value: 0x0000 = disabled, 0x0001 = notifications enabled */
    
    /* Quaternion CCCD */
    if (p_evt->handle == service->quat_handles.cccd_handle && p_evt->len == 2)
    {
        bool enabled = (p_evt->data[0] & 0x01) != 0;
        service->quat_notify_enabled = enabled;
        
        if (service->evt_handler != NULL)
        {
            evt.type = enabled ? BLE_IMU_EVT_QUAT_NOTIFY_EN : BLE_IMU_EVT_QUAT_NOTIFY_DIS;
            evt.conn_handle = service->conn_handle;
            service->evt_handler(&evt);
        }
    }
    /* Accelerometer CCCD */
    else if (p_evt->handle == service->accel_handles.cccd_handle && p_evt->len == 2)
    {
        bool enabled = (p_evt->data[0] & 0x01) != 0;
        service->accel_notify_enabled = enabled;
        
        if (service->evt_handler != NULL)
        {
            evt.type = enabled ? BLE_IMU_EVT_ACCEL_NOTIFY_EN : BLE_IMU_EVT_ACCEL_NOTIFY_DIS;
            evt.conn_handle = service->conn_handle;
            service->evt_handler(&evt);
        }
    }
    /* Gyroscope CCCD */
    else if (p_evt->handle == service->gyro_handles.cccd_handle && p_evt->len == 2)
    {
        bool enabled = (p_evt->data[0] & 0x01) != 0;
        service->gyro_notify_enabled = enabled;
        
        if (service->evt_handler != NULL)
        {
            evt.type = enabled ? BLE_IMU_EVT_GYRO_NOTIFY_EN : BLE_IMU_EVT_GYRO_NOTIFY_DIS;
            evt.conn_handle = service->conn_handle;
            service->evt_handler(&evt);
        }
    }
    /* Status CCCD */
    else if (p_evt->handle == service->status_handles.cccd_handle && p_evt->len == 2)
    {
        bool enabled = (p_evt->data[0] & 0x01) != 0;
        service->status_notify_enabled = enabled;
        
        if (service->evt_handler != NULL)
        {
            evt.type = enabled ? BLE_IMU_EVT_STATUS_NOTIFY_EN : BLE_IMU_EVT_STATUS_NOTIFY_DIS;
            evt.conn_handle = service->conn_handle;
            service->evt_handler(&evt);
        }
    }
    /* Sample rate write */
    else if (p_evt->handle == service->rate_handles.value_handle && p_evt->len == 2)
    {
        uint16_t new_rate = (uint16_t)p_evt->data[0] | ((uint16_t)p_evt->data[1] << 8);
        
        /* Validate rate (1-1000 ms) */
        if (new_rate >= 1 && new_rate <= 1000)
        {
            service->sample_rate_ms = new_rate;
            
            if (service->evt_handler != NULL)
            {
                evt.type = BLE_IMU_EVT_RATE_WRITE;
                evt.conn_handle = service->conn_handle;
                evt.data.rate_ms = new_rate;
                service->evt_handler(&evt);
            }
        }
    }
}

/**
 * @brief Static wrapper for BLE event handling
 * 
 * This wrapper is registered with ble_stack_service_handler_register() and
 * forwards events to the actual service instance stored in mp_service_instance.
 */
static void imu_service_ble_evt_wrapper(const ble_evt_t *p_ble_evt)
{
    if (mp_service_instance != NULL)
    {
        ble_imu_service_on_ble_evt(mp_service_instance, p_ble_evt);
    }
}

/**
 * @brief Send a notification for a characteristic
 */
static uint32_t notify_send(uint16_t conn_handle, uint16_t value_handle,
                            const uint8_t *p_data, uint16_t len)
{
    ble_gatts_hvx_params_t hvx_params;
    uint16_t hvx_len = len;
    
    memset(&hvx_params, 0, sizeof(hvx_params));
    hvx_params.handle = value_handle;
    hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len = &hvx_len;
    hvx_params.p_data = (uint8_t *)p_data;
    
    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}

/*******************************************************************************
 * Public Functions - Initialization
 ******************************************************************************/

uint32_t ble_imu_service_init(ble_imu_service_t *service,
                              const ble_imu_config_t *config,
                              ble_imu_evt_handler_t evt_handler)
{
    uint32_t err_code;
    ble_uuid_t service_uuid;
    uint8_t init_status = 0;
    uint16_t init_rate;
    
    if (service == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    /* Initialize service structure */
    memset(service, 0, sizeof(ble_imu_service_t));
    service->conn_handle = BLE_CONN_HANDLE_INVALID;
    service->evt_handler = evt_handler;
    
    /* Set default configuration */
    if (config != NULL)
    {
        service->sample_rate_ms = config->default_rate_ms;
    }
    else
    {
        service->sample_rate_ms = DEFAULT_SAMPLE_RATE_MS;
    }
    init_rate = service->sample_rate_ms;
    
    /* Register vendor-specific UUID base
     * Citation: Nordic SDK - sd_ble_uuid_vs_add() registers a 128-bit UUID base */
    err_code = sd_ble_uuid_vs_add(&m_uuid_base, &service->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Add service to GATT server
     * Citation: Nordic SDK - sd_ble_gatts_service_add() adds a primary service */
    service_uuid.type = service->uuid_type;
    service_uuid.uuid = BLE_IMU_SERVICE_UUID;
    
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &service->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Add Quaternion characteristic (Read, Notify)
     * Citation: FIRMWARE_DESIGN.md - "Quaternion (0x0001) - 16 bytes, notify" */
    err_code = char_add(service, BLE_IMU_CHAR_QUATERNION_UUID,
                        NULL, BLE_IMU_QUAT_SIZE,
                        true, false,
                        &service->quat_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Add Accelerometer characteristic (Read, Notify)
     * Citation: FIRMWARE_DESIGN.md - "Accelerometer (0x0002) - 12 bytes, notify" */
    err_code = char_add(service, BLE_IMU_CHAR_ACCEL_UUID,
                        NULL, BLE_IMU_ACCEL_SIZE,
                        true, false,
                        &service->accel_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Add Gyroscope characteristic (Read, Notify)
     * Citation: FIRMWARE_DESIGN.md - "Gyroscope (0x0003) - 12 bytes, notify" */
    err_code = char_add(service, BLE_IMU_CHAR_GYRO_UUID,
                        NULL, BLE_IMU_GYRO_SIZE,
                        true, false,
                        &service->gyro_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Add Sample Rate characteristic (Read, Write)
     * Citation: FIRMWARE_DESIGN.md - "Sample Rate (0x0004) - 2 bytes, read/write" */
    err_code = char_add(service, BLE_IMU_CHAR_RATE_UUID,
                        (const uint8_t *)&init_rate, BLE_IMU_RATE_SIZE,
                        false, true,
                        &service->rate_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Add Status characteristic (Read, Notify)
     * Citation: FIRMWARE_DESIGN.md - "Status (0x0005) - 1 byte, notify/read" */
    err_code = char_add(service, BLE_IMU_CHAR_STATUS_UUID,
                        &init_status, BLE_IMU_STATUS_SIZE,
                        true, false,
                        &service->status_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Store service instance pointer for event handling wrapper */
    mp_service_instance = service;
    
    /* Register with BLE stack for event handling
     * We use the static wrapper that forwards to ble_imu_service_on_ble_evt
     * with the service instance pointer. */
    err_code = ble_stack_service_handler_register(imu_service_ble_evt_wrapper);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

void ble_imu_service_on_ble_evt(ble_imu_service_t *service, const ble_evt_t *p_ble_evt)
{
    ble_imu_evt_t evt;
    
    if (service == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            
            /* Reset notification flags on new connection */
            service->quat_notify_enabled = false;
            service->accel_notify_enabled = false;
            service->gyro_notify_enabled = false;
            service->status_notify_enabled = false;
            
            if (service->evt_handler != NULL)
            {
                evt.type = BLE_IMU_EVT_CONNECTED;
                evt.conn_handle = service->conn_handle;
                service->evt_handler(&evt);
            }
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            if (service->evt_handler != NULL)
            {
                evt.type = BLE_IMU_EVT_DISCONNECTED;
                evt.conn_handle = service->conn_handle;
                service->evt_handler(&evt);
            }
            
            service->conn_handle = BLE_CONN_HANDLE_INVALID;
            
            /* Disable all notifications */
            service->quat_notify_enabled = false;
            service->accel_notify_enabled = false;
            service->gyro_notify_enabled = false;
            service->status_notify_enabled = false;
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(service, &p_ble_evt->evt.gatts_evt.params.write);
            break;
            
        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            if (service->evt_handler != NULL)
            {
                evt.type = BLE_IMU_EVT_TX_COMPLETE;
                evt.conn_handle = service->conn_handle;
                evt.data.tx_count = p_ble_evt->evt.gatts_evt.params.hvn_tx_complete.count;
                service->evt_handler(&evt);
            }
            break;
            
        default:
            break;
    }
}

/*******************************************************************************
 * Public Functions - Notifications
 ******************************************************************************/

uint32_t ble_imu_notify_quaternion(ble_imu_service_t *service, 
                                   const ble_imu_quat_t *quat)
{
    if (service == NULL || quat == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    if (service->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    if (!service->quat_notify_enabled)
    {
        return NRF_SUCCESS;  /* Silently succeed if notifications disabled */
    }
    
    return notify_send(service->conn_handle,
                       service->quat_handles.value_handle,
                       (const uint8_t *)quat,
                       BLE_IMU_QUAT_SIZE);
}

uint32_t ble_imu_notify_accelerometer(ble_imu_service_t *service,
                                      const ble_imu_vector_t *accel)
{
    if (service == NULL || accel == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    if (service->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    if (!service->accel_notify_enabled)
    {
        return NRF_SUCCESS;
    }
    
    return notify_send(service->conn_handle,
                       service->accel_handles.value_handle,
                       (const uint8_t *)accel,
                       BLE_IMU_ACCEL_SIZE);
}

uint32_t ble_imu_notify_gyroscope(ble_imu_service_t *service,
                                  const ble_imu_vector_t *gyro)
{
    if (service == NULL || gyro == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    if (service->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    if (!service->gyro_notify_enabled)
    {
        return NRF_SUCCESS;
    }
    
    return notify_send(service->conn_handle,
                       service->gyro_handles.value_handle,
                       (const uint8_t *)gyro,
                       BLE_IMU_GYRO_SIZE);
}

uint32_t ble_imu_notify_status(ble_imu_service_t *service, uint8_t status)
{
    if (service == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    if (service->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    /* Update stored status */
    service->status_flags = status;
    
    if (!service->status_notify_enabled)
    {
        return NRF_SUCCESS;
    }
    
    return notify_send(service->conn_handle,
                       service->status_handles.value_handle,
                       &status,
                       BLE_IMU_STATUS_SIZE);
}

/*******************************************************************************
 * Public Functions - Configuration
 ******************************************************************************/

uint32_t ble_imu_update_status(ble_imu_service_t *service, uint8_t status)
{
    uint32_t err_code;
    ble_gatts_value_t gatts_value;
    
    if (service == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    service->status_flags = status;
    
    /* Update characteristic value in GATT database */
    memset(&gatts_value, 0, sizeof(gatts_value));
    gatts_value.len = BLE_IMU_STATUS_SIZE;
    gatts_value.offset = 0;
    gatts_value.p_value = &status;
    
    err_code = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID,
                                      service->status_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    /* Send notification if enabled and connected */
    if (service->conn_handle != BLE_CONN_HANDLE_INVALID &&
        service->status_notify_enabled)
    {
        return ble_imu_notify_status(service, status);
    }
    
    return NRF_SUCCESS;
}

uint16_t ble_imu_get_sample_rate(const ble_imu_service_t *service)
{
    if (service == NULL)
    {
        return 0;
    }
    
    return service->sample_rate_ms;
}

uint32_t ble_imu_set_sample_rate(ble_imu_service_t *service, uint16_t rate_ms)
{
    uint32_t err_code;
    ble_gatts_value_t gatts_value;
    
    if (service == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    /* Validate rate (1-1000 ms) */
    if (rate_ms < 1 || rate_ms > 1000)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    service->sample_rate_ms = rate_ms;
    
    /* Update characteristic value in GATT database */
    memset(&gatts_value, 0, sizeof(gatts_value));
    gatts_value.len = BLE_IMU_RATE_SIZE;
    gatts_value.offset = 0;
    gatts_value.p_value = (uint8_t *)&rate_ms;
    
    err_code = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID,
                                      service->rate_handles.value_handle,
                                      &gatts_value);
    
    return err_code;
}

/*******************************************************************************
 * Public Functions - Status Queries
 ******************************************************************************/

bool ble_imu_notifications_enabled(const ble_imu_service_t *service)
{
    if (service == NULL)
    {
        return false;
    }
    
    return service->quat_notify_enabled ||
           service->accel_notify_enabled ||
           service->gyro_notify_enabled ||
           service->status_notify_enabled;
}

bool ble_imu_is_connected(const ble_imu_service_t *service)
{
    if (service == NULL)
    {
        return false;
    }
    
    return service->conn_handle != BLE_CONN_HANDLE_INVALID;
}

uint32_t ble_imu_service_uuid_get(const ble_imu_service_t *service, ble_uuid_t *p_uuid)
{
    if (service == NULL || p_uuid == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    
    if (service->uuid_type == BLE_UUID_TYPE_UNKNOWN)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    p_uuid->type = service->uuid_type;
    p_uuid->uuid = BLE_IMU_SERVICE_UUID;
    
    return NRF_SUCCESS;
}
