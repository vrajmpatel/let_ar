/**
 * @file ble_gatts.h
 * @brief BLE GATT Server (GATTS) API for S140 SoftDevice
 *
 * This header provides the GATT Server API for adding services, characteristics,
 * and descriptors, as well as handling GATT server events.
 *
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x Vol 3 Part G
 */

#ifndef BLE_GATTS_H__
#define BLE_GATTS_H__

#include <stdint.h>
#include "nrf_svc.h"
#include "ble_types.h"
#include "ble_gatt.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BLE_GATTS_DEFINES GATT Server Definitions
 * @{ */

/** @brief Maximum number of characteristics per service (limited by RAM allocation) */
#define BLE_GATTS_CHAR_MAX_COUNT            20

/** @brief Value is located in stack memory */
#define BLE_GATTS_VLOC_STACK                0x00

/** @brief Value is located in user memory */
#define BLE_GATTS_VLOC_USER                 0x01

/** @brief Invalid attribute handle */
#define BLE_GATTS_HANDLE_INVALID            0x0000

/** @brief Service handle range */
#define BLE_GATTS_SRVC_TYPE_PRIMARY         0x00 /**< Primary service */
#define BLE_GATTS_SRVC_TYPE_SECONDARY       0x01 /**< Secondary service */

/** @} */

/** @defgroup BLE_GATTS_AUTHORIZE GATT Server Authorization Types
 * @{ */

/** @brief Authorization types */
typedef enum
{
    BLE_GATTS_AUTHORIZE_TYPE_INVALID = 0x00, /**< Invalid type */
    BLE_GATTS_AUTHORIZE_TYPE_READ    = 0x01, /**< Read authorization */
    BLE_GATTS_AUTHORIZE_TYPE_WRITE   = 0x02, /**< Write authorization */
} ble_gatts_authorize_type_t;

/** @} */

/** @defgroup BLE_GATTS_ATTR GATT Server Attribute Structures
 * @{ */

/**
 * @brief Attribute metadata
 *
 * This structure defines the permissions and properties of an attribute.
 */
typedef struct
{
    ble_gap_conn_sec_mode_t read_perm;    /**< Read permissions */
    ble_gap_conn_sec_mode_t write_perm;   /**< Write permissions */
    uint8_t                 vlen       :1; /**< Variable length attribute */
    uint8_t                 vloc       :2; /**< Value location: BLE_GATTS_VLOC_STACK or BLE_GATTS_VLOC_USER */
    uint8_t                 rd_auth    :1; /**< Read authorization required */
    uint8_t                 wr_auth    :1; /**< Write authorization required */
} ble_gatts_attr_md_t;

/**
 * @brief GATT attribute structure
 *
 * Defines a single attribute including its UUID, value, and metadata.
 */
typedef struct
{
    ble_uuid_t const        *p_uuid;      /**< Pointer to attribute UUID */
    ble_gatts_attr_md_t const *p_attr_md; /**< Pointer to attribute metadata */
    uint16_t                init_len;     /**< Initial attribute value length */
    uint16_t                init_offs;    /**< Initial attribute value offset */
    uint16_t                max_len;      /**< Maximum attribute value length */
    uint8_t                 *p_value;     /**< Pointer to attribute value (if vloc=USER) */
} ble_gatts_attr_t;

/**
 * @brief GATT characteristic metadata
 *
 * Defines the properties and descriptors of a characteristic.
 */
typedef struct
{
    ble_gatt_char_props_t     char_props;     /**< Characteristic properties */
    ble_gatt_char_ext_props_t char_ext_props; /**< Extended properties */
    uint8_t const            *p_char_user_desc; /**< User description string */
    uint16_t                  char_user_desc_max_size; /**< Max user description size */
    uint16_t                  char_user_desc_size; /**< Current user description size */
    ble_gatt_char_pf_t const *p_char_pf;      /**< Presentation format */
    ble_gatts_attr_md_t const *p_user_desc_md; /**< User description metadata */
    ble_gatts_attr_md_t const *p_cccd_md;     /**< CCCD metadata */
    ble_gatts_attr_md_t const *p_sccd_md;     /**< SCCD metadata */
} ble_gatts_char_md_t;

/**
 * @brief GATT characteristic handles
 *
 * Returned when a characteristic is added, containing all related handles.
 */
typedef struct
{
    uint16_t value_handle;     /**< Handle of the characteristic value */
    uint16_t user_desc_handle; /**< Handle of the user description descriptor */
    uint16_t cccd_handle;      /**< Handle of the CCCD (Client Characteristic Configuration Descriptor) */
    uint16_t sccd_handle;      /**< Handle of the SCCD (Server Characteristic Configuration Descriptor) */
} ble_gatts_char_handles_t;

/**
 * @brief GATT Handle Value Notification/Indication parameters
 */
typedef struct
{
    uint16_t  handle;  /**< Handle of the attribute value */
    uint8_t   type;    /**< Type: BLE_GATT_HVX_NOTIFICATION or BLE_GATT_HVX_INDICATION */
    uint16_t  offset;  /**< Offset in the value (for long attributes) */
    uint16_t *p_len;   /**< Pointer to length of data. Updated with actual sent length */
    uint8_t  *p_data;  /**< Pointer to data to send */
} ble_gatts_hvx_params_t;

/**
 * @brief GATT Value structure for get/set operations
 */
typedef struct
{
    uint16_t len;      /**< Length of value */
    uint16_t offset;   /**< Offset in value */
    uint8_t *p_value;  /**< Pointer to value data */
} ble_gatts_value_t;

/**
 * @brief Read authorization reply parameters
 */
typedef struct
{
    uint16_t gatt_status; /**< GATT status code */
    uint8_t  update : 1;  /**< Update attribute value before reply */
    uint16_t offset;      /**< Offset in the attribute value */
    uint16_t len;         /**< Length of data to return */
    uint8_t *p_data;      /**< Pointer to data to return */
} ble_gatts_rw_authorize_reply_params_read_t;

/**
 * @brief Write authorization reply parameters
 */
typedef struct
{
    uint16_t gatt_status; /**< GATT status code */
} ble_gatts_rw_authorize_reply_params_write_t;

/**
 * @brief Authorization reply parameters
 */
typedef struct
{
    uint8_t type; /**< Authorization type, see @ref ble_gatts_authorize_type_t */
    union
    {
        ble_gatts_rw_authorize_reply_params_read_t  read;  /**< Read authorization reply */
        ble_gatts_rw_authorize_reply_params_write_t write; /**< Write authorization reply */
    } params; /**< Reply parameters */
} ble_gatts_rw_authorize_reply_params_t;

/** @} */

/** @defgroup BLE_GATTS_EVENTS GATT Server Events
 * @{ */

/** @brief GATT Server Event IDs */
typedef enum
{
    BLE_GATTS_EVT_WRITE               = 0x50, /**< Write operation performed */
    BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST = 0x51, /**< Read/Write authorization request */
    BLE_GATTS_EVT_SYS_ATTR_MISSING    = 0x52, /**< System attributes missing */
    BLE_GATTS_EVT_HVC                 = 0x53, /**< Handle Value Confirmation (indication ACK) */
    BLE_GATTS_EVT_SC_CONFIRM          = 0x54, /**< Service Changed confirmation */
    BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST = 0x55, /**< MTU exchange request */
    BLE_GATTS_EVT_TIMEOUT             = 0x56, /**< GATT timeout */
    BLE_GATTS_EVT_HVN_TX_COMPLETE     = 0x57, /**< Handle Value Notification TX complete */
} ble_gatts_evt_id_t;

/**
 * @brief Write event context
 */
typedef struct
{
    uint16_t handle;   /**< Attribute handle */
    ble_uuid_t uuid;   /**< Attribute UUID */
    uint8_t  op;       /**< Write operation, see @ref ble_gatt_write_op_t */
    uint8_t  auth_required; /**< Authorization required */
    uint16_t offset;   /**< Offset in attribute value */
    uint16_t len;      /**< Length of data written */
    uint8_t  data[];   /**< Written data (variable length) */
} ble_gatts_evt_write_t;

/**
 * @brief Read authorization request context
 */
typedef struct
{
    uint16_t   handle; /**< Attribute handle being read */
    ble_uuid_t uuid;   /**< Attribute UUID */
    uint16_t   offset; /**< Offset in attribute value */
} ble_gatts_evt_read_t;

/**
 * @brief Authorization request event structure
 */
typedef struct
{
    uint8_t type; /**< Type: BLE_GATTS_AUTHORIZE_TYPE_READ or BLE_GATTS_AUTHORIZE_TYPE_WRITE */
    union
    {
        ble_gatts_evt_read_t  read;  /**< Read authorization context */
        ble_gatts_evt_write_t write; /**< Write authorization context */
    } request; /**< Authorization request details */
} ble_gatts_evt_rw_authorize_request_t;

/**
 * @brief System attributes missing event structure
 */
typedef struct
{
    uint8_t hint; /**< Hint: 0 = no hint */
} ble_gatts_evt_sys_attr_missing_t;

/**
 * @brief Handle Value Confirmation event structure
 */
typedef struct
{
    uint16_t handle; /**< Handle of the attribute that was indicated */
} ble_gatts_evt_hvc_t;

/**
 * @brief MTU exchange request event structure
 */
typedef struct
{
    uint16_t client_rx_mtu; /**< Client's RX MTU size */
} ble_gatts_evt_exchange_mtu_request_t;

/**
 * @brief GATT Server timeout event structure
 */
typedef struct
{
    uint8_t src; /**< Timeout source */
} ble_gatts_evt_timeout_t;

/**
 * @brief Handle Value Notification TX complete event structure
 */
typedef struct
{
    uint8_t count; /**< Number of notifications transmitted */
} ble_gatts_evt_hvn_tx_complete_t;

/**
 * @brief GATT Server event union
 */
typedef union
{
    ble_gatts_evt_write_t               write;                 /**< Write event */
    ble_gatts_evt_rw_authorize_request_t authorize_request;    /**< Authorization request */
    ble_gatts_evt_sys_attr_missing_t    sys_attr_missing;      /**< System attributes missing */
    ble_gatts_evt_hvc_t                 hvc;                   /**< Handle Value Confirmation */
    ble_gatts_evt_exchange_mtu_request_t exchange_mtu_request; /**< MTU exchange request */
    ble_gatts_evt_timeout_t             timeout;               /**< Timeout */
    ble_gatts_evt_hvn_tx_complete_t     hvn_tx_complete;       /**< HVN TX complete */
} ble_gatts_evt_params_t;

/**
 * @brief GATT Server event structure
 */
typedef struct
{
    uint16_t              conn_handle; /**< Connection handle */
    ble_gatts_evt_params_t params;     /**< Event parameters */
} ble_gatts_evt_t;

/** @} */

/** @defgroup BLE_GATTS_API GATT Server API Functions
 * @{ */

/**
 * @brief Add a service to the GATT server
 *
 * @param[in]  type         Service type: BLE_GATTS_SRVC_TYPE_PRIMARY or BLE_GATTS_SRVC_TYPE_SECONDARY
 * @param[in]  p_uuid       Pointer to service UUID
 * @param[out] p_handle     Pointer to store service handle
 *
 * @retval NRF_SUCCESS              Service added successfully
 * @retval NRF_ERROR_INVALID_ADDR   Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM  Invalid service type
 * @retval NRF_ERROR_FORBIDDEN      Service cannot be added (stack limitation)
 * @retval NRF_ERROR_NO_MEM         Not enough memory
 */
SVCALL(SD_BLE_GATTS_SERVICE_ADD, uint32_t, sd_ble_gatts_service_add(
    uint8_t type,
    const ble_uuid_t *p_uuid,
    uint16_t *p_handle));

/**
 * @brief Add a characteristic to a service
 *
 * @param[in]  service_handle  Handle of the service to add characteristic to
 * @param[in]  p_char_md       Pointer to characteristic metadata
 * @param[in]  p_attr_char_value Pointer to characteristic value attribute
 * @param[out] p_handles       Pointer to store characteristic handles
 *
 * @retval NRF_SUCCESS              Characteristic added successfully
 * @retval NRF_ERROR_INVALID_ADDR   Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM  Invalid parameter
 * @retval NRF_ERROR_INVALID_STATE  Invalid state (e.g., connections exist)
 * @retval NRF_ERROR_FORBIDDEN      Characteristic cannot be added
 * @retval NRF_ERROR_NO_MEM         Not enough memory
 */
SVCALL(SD_BLE_GATTS_CHARACTERISTIC_ADD, uint32_t, sd_ble_gatts_characteristic_add(
    uint16_t service_handle,
    const ble_gatts_char_md_t *p_char_md,
    const ble_gatts_attr_t *p_attr_char_value,
    ble_gatts_char_handles_t *p_handles));

/**
 * @brief Add a descriptor to a characteristic
 *
 * @param[in]  char_handle  Handle of the characteristic value
 * @param[in]  p_attr       Pointer to descriptor attribute
 * @param[out] p_handle     Pointer to store descriptor handle
 *
 * @retval NRF_SUCCESS              Descriptor added successfully
 * @retval NRF_ERROR_INVALID_ADDR   Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM  Invalid parameter
 * @retval NRF_ERROR_INVALID_STATE  Invalid state
 * @retval NRF_ERROR_FORBIDDEN      Descriptor cannot be added
 * @retval NRF_ERROR_NO_MEM         Not enough memory
 */
SVCALL(SD_BLE_GATTS_DESCRIPTOR_ADD, uint32_t, sd_ble_gatts_descriptor_add(
    uint16_t char_handle,
    const ble_gatts_attr_t *p_attr,
    uint16_t *p_handle));

/**
 * @brief Add an include declaration to a service
 *
 * @param[in]  service_handle      Handle of the service
 * @param[in]  inc_srvc_handle     Handle of the service to include
 * @param[out] p_include_handle    Pointer to store include declaration handle
 *
 * @retval NRF_SUCCESS              Include added successfully
 * @retval NRF_ERROR_INVALID_ADDR   Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM  Invalid parameter
 * @retval NRF_ERROR_INVALID_STATE  Invalid state
 * @retval NRF_ERROR_NO_MEM         Not enough memory
 */
SVCALL(SD_BLE_GATTS_INCLUDE_ADD, uint32_t, sd_ble_gatts_include_add(
    uint16_t service_handle,
    uint16_t inc_srvc_handle,
    uint16_t *p_include_handle));

/**
 * @brief Set an attribute value
 *
 * @param[in]     conn_handle  Connection handle (BLE_CONN_HANDLE_INVALID for default)
 * @param[in]     handle       Attribute handle
 * @param[in,out] p_value      Pointer to value structure
 *
 * @retval NRF_SUCCESS                  Value set successfully
 * @retval NRF_ERROR_INVALID_ADDR       Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM      Invalid handle
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle
 * @retval NRF_ERROR_DATA_SIZE          Value too large
 */
SVCALL(SD_BLE_GATTS_VALUE_SET, uint32_t, sd_ble_gatts_value_set(
    uint16_t conn_handle,
    uint16_t handle,
    ble_gatts_value_t *p_value));

/**
 * @brief Get an attribute value
 *
 * @param[in]     conn_handle  Connection handle (BLE_CONN_HANDLE_INVALID for default)
 * @param[in]     handle       Attribute handle
 * @param[in,out] p_value      Pointer to value structure (len is updated)
 *
 * @retval NRF_SUCCESS                  Value retrieved successfully
 * @retval NRF_ERROR_INVALID_ADDR       Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM      Invalid handle
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle
 */
SVCALL(SD_BLE_GATTS_VALUE_GET, uint32_t, sd_ble_gatts_value_get(
    uint16_t conn_handle,
    uint16_t handle,
    ble_gatts_value_t *p_value));

/**
 * @brief Send a Handle Value Notification or Indication
 *
 * For notifications, the function returns when the notification is queued.
 * For indications, use BLE_GATTS_EVT_HVC to confirm receipt.
 *
 * @param[in]     conn_handle  Connection handle
 * @param[in,out] p_hvx_params Pointer to HVX parameters (p_len updated with actual length)
 *
 * @retval NRF_SUCCESS                  HVX queued successfully
 * @retval NRF_ERROR_INVALID_ADDR       Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM      Invalid handle or type
 * @retval NRF_ERROR_INVALID_STATE      CCCD not configured for notifications/indications
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle
 * @retval NRF_ERROR_BUSY               Indication pending (only one at a time)
 * @retval NRF_ERROR_RESOURCES          Too many notifications queued
 * @retval BLE_ERROR_GATTS_SYS_ATTR_MISSING System attributes not set
 */
SVCALL(SD_BLE_GATTS_HVX, uint32_t, sd_ble_gatts_hvx(
    uint16_t conn_handle,
    ble_gatts_hvx_params_t *p_hvx_params));

/**
 * @brief Indicate that a Service Changed has occurred
 *
 * @param[in] conn_handle      Connection handle
 * @param[in] start_handle     Start of affected handle range
 * @param[in] end_handle       End of affected handle range
 *
 * @retval NRF_SUCCESS                  Service changed indication sent
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle
 * @retval NRF_ERROR_INVALID_PARAM      Invalid handle range
 */
SVCALL(SD_BLE_GATTS_SERVICE_CHANGED, uint32_t, sd_ble_gatts_service_changed(
    uint16_t conn_handle,
    uint16_t start_handle,
    uint16_t end_handle));

/**
 * @brief Reply to a Read/Write authorization request
 *
 * Call this in response to a BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST event.
 *
 * @param[in] conn_handle  Connection handle
 * @param[in] p_params     Pointer to reply parameters
 *
 * @retval NRF_SUCCESS                  Reply sent successfully
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle
 * @retval NRF_ERROR_INVALID_ADDR       Invalid pointer
 * @retval NRF_ERROR_INVALID_STATE      No pending authorization request
 */
SVCALL(SD_BLE_GATTS_RW_AUTHORIZE_REPLY, uint32_t, sd_ble_gatts_rw_authorize_reply(
    uint16_t conn_handle,
    const ble_gatts_rw_authorize_reply_params_t *p_params));

/**
 * @brief Set system attribute data
 *
 * Must be called after a connection is established if system attributes
 * were stored (for bonded devices). Call with NULL data for new devices.
 *
 * @param[in] conn_handle    Connection handle
 * @param[in] p_sys_attr_data Pointer to system attribute data (NULL for default)
 * @param[in] len            Length of system attribute data
 * @param[in] flags          Flags (0 = all attributes)
 *
 * @retval NRF_SUCCESS                  System attributes set successfully
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle
 * @retval NRF_ERROR_INVALID_ADDR       Invalid pointer
 * @retval NRF_ERROR_INVALID_DATA       Invalid data format
 */
SVCALL(SD_BLE_GATTS_SYS_ATTR_SET, uint32_t, sd_ble_gatts_sys_attr_set(
    uint16_t conn_handle,
    const uint8_t *p_sys_attr_data,
    uint16_t len,
    uint32_t flags));

/**
 * @brief Get system attribute data
 *
 * Used to retrieve system attributes for storage (bonding).
 *
 * @param[in]     conn_handle    Connection handle
 * @param[out]    p_sys_attr_data Buffer for system attribute data
 * @param[in,out] p_len          On input: buffer size. On output: data length.
 * @param[in]     flags          Flags (0 = all attributes)
 *
 * @retval NRF_SUCCESS                  System attributes retrieved successfully
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle
 * @retval NRF_ERROR_INVALID_ADDR       Invalid pointer
 * @retval NRF_ERROR_DATA_SIZE          Buffer too small
 */
SVCALL(SD_BLE_GATTS_SYS_ATTR_GET, uint32_t, sd_ble_gatts_sys_attr_get(
    uint16_t conn_handle,
    uint8_t *p_sys_attr_data,
    uint16_t *p_len,
    uint32_t flags));

/**
 * @brief Reply to an MTU exchange request
 *
 * Call this in response to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event.
 *
 * @param[in] conn_handle   Connection handle
 * @param[in] server_rx_mtu Server's RX MTU
 *
 * @retval NRF_SUCCESS                  Reply sent successfully
 * @retval BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle
 * @retval NRF_ERROR_INVALID_STATE      No pending MTU request
 */
SVCALL(SD_BLE_GATTS_EXCHANGE_MTU_REPLY, uint32_t, sd_ble_gatts_exchange_mtu_reply(
    uint16_t conn_handle,
    uint16_t server_rx_mtu));

/**
 * @brief Get attribute information
 *
 * @param[in]  handle    Attribute handle
 * @param[out] p_uuid    Pointer to store UUID
 * @param[out] p_md      Pointer to store metadata (can be NULL)
 *
 * @retval NRF_SUCCESS              Attribute info retrieved
 * @retval NRF_ERROR_INVALID_ADDR   Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM  Invalid handle
 */
SVCALL(SD_BLE_GATTS_ATTR_GET, uint32_t, sd_ble_gatts_attr_get(
    uint16_t handle,
    ble_uuid_t *p_uuid,
    ble_gatts_attr_md_t *p_md));

/**
 * @brief Get the first user handle
 *
 * Returns the first attribute handle in the application region
 * (after SoftDevice reserved handles).
 *
 * @param[out] p_handle  Pointer to store first handle
 *
 * @retval NRF_SUCCESS              Handle retrieved
 * @retval NRF_ERROR_INVALID_ADDR   Invalid pointer
 */
SVCALL(SD_BLE_GATTS_INITIAL_USER_HANDLE_GET, uint32_t, sd_ble_gatts_initial_user_handle_get(uint16_t *p_handle));

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BLE_GATTS_H__ */
