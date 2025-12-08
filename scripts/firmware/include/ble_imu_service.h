/**
 * @file ble_imu_service.h
 * @brief Custom BLE GATT Service for IMU data
 * 
 * Defines a custom BLE service for streaming IMU sensor data from the
 * BNO085 to connected BLE central devices.
 * 
 * Citations:
 * - FIRMWARE_DESIGN.md Section "BLE Service Design"
 * - nRF52840_PS_v1.11.pdf: BLE 5.0 features
 * - S140 SoftDevice Specification
 */

#ifndef BLE_IMU_SERVICE_H
#define BLE_IMU_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_gatts.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Service UUID Definitions
 * Citation: FIRMWARE_DESIGN.md:
 *   "Custom IMU Service UUID: 12340000-1234-1234-1234-123456789ABC"
 * 
 * UUID Structure (128-bit, stored little-endian):
 *   Base:    12340000-1234-1234-1234-123456789ABC
 *   Service: 12340000-...
 *   Chars:   12340001-... through 12340005-...
 ******************************************************************************/

/* 128-bit UUID Base (stored in little-endian for SoftDevice) 
 * Citation: Nordic SDK - UUIDs must be in little-endian byte order */
#define BLE_IMU_UUID_BASE { \
    0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12, /* 123456789ABC reversed */ \
    0x34, 0x12,                         /* 1234 */                   \
    0x34, 0x12,                         /* 1234 */                   \
    0x34, 0x12,                         /* 1234 */                   \
    0x00, 0x00,                         /* Short UUID placeholder */ \
    0x34, 0x12                          /* 1234 */                   \
}

/* Service and Characteristic 16-bit UUIDs (relative to base)
 * Citation: FIRMWARE_DESIGN.md Section "BLE Service Design"
 */
#define BLE_IMU_SERVICE_UUID            0x0000  /* IMU Service */
#define BLE_IMU_CHAR_QUATERNION_UUID    0x0001  /* Quaternion data */
#define BLE_IMU_CHAR_ACCEL_UUID         0x0002  /* Accelerometer data */
#define BLE_IMU_CHAR_GYRO_UUID          0x0003  /* Gyroscope data */
#define BLE_IMU_CHAR_RATE_UUID          0x0004  /* Sample rate config */
#define BLE_IMU_CHAR_STATUS_UUID        0x0005  /* Status flags */

/*******************************************************************************
 * Characteristic Data Sizes
 * Citation: FIRMWARE_DESIGN.md Section "BLE Service Design"
 ******************************************************************************/
#define BLE_IMU_QUAT_SIZE           16  /* 4x float32 (i, j, k, real) */
#define BLE_IMU_ACCEL_SIZE          12  /* 3x float32 (x, y, z) m/s² */
#define BLE_IMU_GYRO_SIZE           12  /* 3x float32 (x, y, z) rad/s */
#define BLE_IMU_RATE_SIZE           2   /* uint16 report interval (ms) */
#define BLE_IMU_STATUS_SIZE         1   /* uint8 status flags */

/*******************************************************************************
 * Status Flags
 ******************************************************************************/
#define BLE_IMU_STATUS_SENSOR_OK    (1 << 0)  /* Sensor communication OK */
#define BLE_IMU_STATUS_CALIBRATED   (1 << 1)  /* Sensor calibrated */
#define BLE_IMU_STATUS_STREAMING    (1 << 2)  /* Data streaming active */
#define BLE_IMU_STATUS_ERROR        (1 << 7)  /* Error condition */

/*******************************************************************************
 * Data Structures
 ******************************************************************************/

/**
 * @brief Quaternion data structure for BLE transmission
 * Citation: BNO085 datasheet - Quaternion output format
 */
typedef struct __attribute__((packed)) {
    float i;        /* Quaternion i component */
    float j;        /* Quaternion j component */
    float k;        /* Quaternion k component */
    float real;     /* Quaternion real (w) component */
} ble_imu_quat_t;

/**
 * @brief 3-axis vector data structure for BLE transmission
 */
typedef struct __attribute__((packed)) {
    float x;        /* X-axis value */
    float y;        /* Y-axis value */
    float z;        /* Z-axis value */
} ble_imu_vector_t;

/**
 * @brief IMU service configuration
 */
typedef struct {
    uint16_t default_rate_ms;       /* Default report rate (ms) */
    bool     auto_notify;           /* Auto-notify on data update */
} ble_imu_config_t;

/**
 * @brief IMU service event types
 */
typedef enum {
    BLE_IMU_EVT_CONNECTED,          /* Client connected */
    BLE_IMU_EVT_DISCONNECTED,       /* Client disconnected */
    BLE_IMU_EVT_QUAT_NOTIFY_EN,     /* Quaternion notifications enabled */
    BLE_IMU_EVT_QUAT_NOTIFY_DIS,    /* Quaternion notifications disabled */
    BLE_IMU_EVT_ACCEL_NOTIFY_EN,    /* Accelerometer notifications enabled */
    BLE_IMU_EVT_ACCEL_NOTIFY_DIS,   /* Accelerometer notifications disabled */
    BLE_IMU_EVT_GYRO_NOTIFY_EN,     /* Gyroscope notifications enabled */
    BLE_IMU_EVT_GYRO_NOTIFY_DIS,    /* Gyroscope notifications disabled */
    BLE_IMU_EVT_STATUS_NOTIFY_EN,   /* Status notifications enabled */
    BLE_IMU_EVT_STATUS_NOTIFY_DIS,  /* Status notifications disabled */
    BLE_IMU_EVT_RATE_WRITE,         /* Sample rate written */
    BLE_IMU_EVT_TX_COMPLETE,        /* Notification TX complete */
} ble_imu_evt_type_t;

/**
 * @brief IMU service event data
 */
typedef struct {
    ble_imu_evt_type_t type;        /* Event type */
    uint16_t           conn_handle; /* Connection handle */
    union {
        uint16_t rate_ms;           /* New sample rate (for RATE_WRITE) */
        uint8_t  tx_count;          /* TX complete count */
    } data;
} ble_imu_evt_t;

/**
 * @brief Event handler callback type
 */
typedef void (*ble_imu_evt_handler_t)(const ble_imu_evt_t *evt);

/**
 * @brief IMU service handle structure
 */
typedef struct {
    uint16_t service_handle;            /* Service handle */
    uint16_t conn_handle;               /* Current connection handle */
    
    /* Characteristic value handles */
    ble_gatts_char_handles_t quat_handles;    /* Quaternion characteristic handles */
    ble_gatts_char_handles_t accel_handles;   /* Accelerometer characteristic handles */
    ble_gatts_char_handles_t gyro_handles;    /* Gyroscope characteristic handles */
    ble_gatts_char_handles_t rate_handles;    /* Sample rate characteristic handles */
    ble_gatts_char_handles_t status_handles;  /* Status characteristic handles */
    
    /* Notification enable flags (from CCCD writes) */
    bool quat_notify_enabled;
    bool accel_notify_enabled;
    bool gyro_notify_enabled;
    bool status_notify_enabled;
    
    /* Configuration */
    uint16_t sample_rate_ms;            /* Current sample rate */
    uint8_t  status_flags;              /* Current status */
    
    /* Event handler */
    ble_imu_evt_handler_t evt_handler;
    
    /* UUID type (vendor specific) */
    uint8_t uuid_type;
} ble_imu_service_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Initialize IMU BLE service
 * 
 * Registers the custom UUID base, adds the service to the GATT server,
 * and adds all characteristics with appropriate properties.
 *
 * @param[in,out] service     Pointer to service handle structure
 * @param[in]     config      Service configuration (NULL for defaults)
 * @param[in]     evt_handler Event callback handler
 * 
 * @retval NRF_SUCCESS             Service initialized successfully
 * @retval NRF_ERROR_INVALID_PARAM Invalid parameter
 * @retval NRF_ERROR_NO_MEM        Insufficient memory for service
 */
uint32_t ble_imu_service_init(ble_imu_service_t *service,
                              const ble_imu_config_t *config,
                              ble_imu_evt_handler_t evt_handler);

/**
 * @brief Handle BLE event (call from BLE event handler)
 * 
 * Handles GAP and GATTS events relevant to the IMU service.
 *
 * @param[in,out] service Pointer to service handle
 * @param[in]     p_ble_evt BLE event from SoftDevice
 */
void ble_imu_service_on_ble_evt(ble_imu_service_t *service, const ble_evt_t *p_ble_evt);

/**
 * @brief Send quaternion data notification
 * 
 * @param[in] service Pointer to service handle
 * @param[in] quat    Quaternion data to send
 * 
 * @retval NRF_SUCCESS             Notification sent/queued
 * @retval NRF_ERROR_INVALID_STATE Not connected or notifications disabled
 * @retval NRF_ERROR_RESOURCES     TX buffer full
 */
uint32_t ble_imu_notify_quaternion(ble_imu_service_t *service, 
                                   const ble_imu_quat_t *quat);

/**
 * @brief Send accelerometer data notification
 * 
 * @param[in] service Pointer to service handle
 * @param[in] accel   Accelerometer data to send
 * 
 * @retval NRF_SUCCESS             Notification sent/queued
 * @retval NRF_ERROR_INVALID_STATE Not connected or notifications disabled
 * @retval NRF_ERROR_RESOURCES     TX buffer full
 */
uint32_t ble_imu_notify_accelerometer(ble_imu_service_t *service,
                                      const ble_imu_vector_t *accel);

/**
 * @brief Send gyroscope data notification
 * 
 * @param[in] service Pointer to service handle
 * @param[in] gyro    Gyroscope data to send
 * 
 * @retval NRF_SUCCESS             Notification sent/queued
 * @retval NRF_ERROR_INVALID_STATE Not connected or notifications disabled
 * @retval NRF_ERROR_RESOURCES     TX buffer full
 */
uint32_t ble_imu_notify_gyroscope(ble_imu_service_t *service,
                                  const ble_imu_vector_t *gyro);

/**
 * @brief Send status notification
 * 
 * @param[in] service Pointer to service handle
 * @param[in] status  Status flags
 * 
 * @retval NRF_SUCCESS             Notification sent/queued
 * @retval NRF_ERROR_INVALID_STATE Not connected or notifications disabled
 */
uint32_t ble_imu_notify_status(ble_imu_service_t *service, uint8_t status);

/**
 * @brief Update status flags and optionally notify
 * 
 * Updates the status characteristic value and sends a notification
 * if enabled and connected.
 *
 * @param[in,out] service Pointer to service handle
 * @param[in]     status  New status flags
 * 
 * @retval NRF_SUCCESS Status updated
 */
uint32_t ble_imu_update_status(ble_imu_service_t *service, uint8_t status);

/**
 * @brief Get current sample rate
 * 
 * @param[in] service Pointer to service handle
 * @return Current sample rate in milliseconds
 */
uint16_t ble_imu_get_sample_rate(const ble_imu_service_t *service);

/**
 * @brief Set sample rate
 * 
 * Updates the rate characteristic value in the GATT database.
 *
 * @param[in,out] service  Pointer to service handle
 * @param[in]     rate_ms  New sample rate in milliseconds (1-1000)
 * 
 * @retval NRF_SUCCESS             Rate updated
 * @retval NRF_ERROR_INVALID_PARAM Rate out of range
 */
uint32_t ble_imu_set_sample_rate(ble_imu_service_t *service, uint16_t rate_ms);

/**
 * @brief Check if any notifications are enabled
 * 
 * @param[in] service Pointer to service handle
 * @return true if any notifications enabled
 */
bool ble_imu_notifications_enabled(const ble_imu_service_t *service);

/**
 * @brief Check if connected to a central device
 * 
 * @param[in] service Pointer to service handle
 * @return true if connected
 */
bool ble_imu_is_connected(const ble_imu_service_t *service);

/**
 * @brief Get the service UUID for advertising
 * 
 * Returns the service UUID in a format suitable for adding to advertising data.
 *
 * @param[in]  service Pointer to initialized service handle
 * @param[out] p_uuid  Pointer to store UUID
 * 
 * @retval NRF_SUCCESS           UUID retrieved
 * @retval NRF_ERROR_INVALID_STATE Service not initialized
 */
uint32_t ble_imu_service_uuid_get(const ble_imu_service_t *service, ble_uuid_t *p_uuid);

#ifdef __cplusplus
}
#endif

#endif /* BLE_IMU_SERVICE_H */
