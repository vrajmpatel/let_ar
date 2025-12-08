/**
 * @file ble.h
 * @brief Main BLE API Header for S140 SoftDevice
 *
 * This header includes all BLE-related headers and provides the main BLE API
 * for enabling the stack, handling events, and managing UUIDs.
 *
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x
 */

#ifndef BLE_H__
#define BLE_H__

#include <stdint.h>
#include "nrf_svc.h"
#include "nrf_error.h"
#include "ble_types.h"
#include "ble_gap.h"
#include "ble_gatt.h"
#include "ble_gatts.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BLE_API_VERSION BLE API Version
 * @{ */

#define BLE_API_VERSION                 7   /**< BLE API version */

/** @} */

/** @defgroup BLE_COMMON_CFG BLE Common Configuration
 * @{ */

/** @brief Configuration tag for default configuration */
#define BLE_CONN_CFG_TAG_DEFAULT        1

/** @brief Maximum number of connections */
#define BLE_CONN_CFG_GAP_MAX_CONN       1

/** @brief Configuration IDs for sd_ble_cfg_set */
typedef enum
{
    BLE_CONN_CFG_GAP       = 0x20, /**< GAP connection configuration */
    BLE_CONN_CFG_GATTC     = 0x21, /**< GATT Client connection configuration */
    BLE_CONN_CFG_GATTS     = 0x22, /**< GATT Server connection configuration */
    BLE_CONN_CFG_GATT      = 0x23, /**< GATT connection configuration */
    BLE_CONN_CFG_L2CAP     = 0x24, /**< L2CAP connection configuration */
    BLE_COMMON_CFG_VS_UUID = 0x01, /**< Vendor-specific UUID configuration */
    BLE_GAP_CFG_ROLE_COUNT = 0x40, /**< GAP role count configuration */
    BLE_GAP_CFG_DEVICE_NAME = 0x41, /**< Device name configuration */
    BLE_GAP_CFG_PPCP_INCL_CONFIG = 0x42, /**< PPCP include configuration */
    BLE_GAP_CFG_CAR_INCL_CONFIG = 0x43, /**< CAR include configuration */
    BLE_GATTS_CFG_SERVICE_CHANGED = 0x50, /**< Service changed configuration */
    BLE_GATTS_CFG_ATTR_TAB_SIZE = 0x51, /**< Attribute table size configuration */
} ble_cfg_id_t;

/** @} */

/** @defgroup BLE_COMMON_STRUCTURES BLE Common Structures
 * @{ */

/**
 * @brief BLE connection configuration - GAP
 */
typedef struct
{
    uint8_t  conn_count;     /**< Number of connections */
    uint16_t event_length;   /**< Event length in 1.25ms units */
} ble_gap_conn_cfg_t;

/**
 * @brief BLE connection configuration - GATTC
 */
typedef struct
{
    uint8_t write_cmd_tx_queue_size; /**< Write command TX queue size */
} ble_gattc_conn_cfg_t;

/**
 * @brief BLE connection configuration - GATTS
 */
typedef struct
{
    uint8_t hvn_tx_queue_size; /**< HVN TX queue size */
} ble_gatts_conn_cfg_t;

/**
 * @brief BLE connection configuration - GATT
 */
typedef struct
{
    uint16_t att_mtu; /**< ATT MTU size */
} ble_gatt_conn_cfg_t;

/**
 * @brief BLE connection configuration union
 */
typedef union
{
    ble_gap_conn_cfg_t   gap_conn_cfg;   /**< GAP connection configuration */
    ble_gattc_conn_cfg_t gattc_conn_cfg; /**< GATTC connection configuration */
    ble_gatts_conn_cfg_t gatts_conn_cfg; /**< GATTS connection configuration */
    ble_gatt_conn_cfg_t  gatt_conn_cfg;  /**< GATT connection configuration */
} ble_conn_cfg_params_t;

/**
 * @brief BLE connection configuration
 */
typedef struct
{
    uint8_t              conn_cfg_tag; /**< Connection configuration tag */
    ble_conn_cfg_params_t params;      /**< Configuration parameters */
} ble_conn_cfg_t;

/**
 * @brief GAP role count configuration
 */
typedef struct
{
    uint8_t periph_role_count;  /**< Maximum peripheral roles */
    uint8_t central_role_count; /**< Maximum central roles */
    uint8_t central_sec_count;  /**< Maximum secure central connections */
    uint8_t qos_channel_survey_role_available : 1; /**< QoS channel survey role */
    uint8_t adv_set_count;      /**< Number of advertising sets */
} ble_gap_cfg_role_count_t;

/**
 * @brief Device name configuration
 */
typedef struct
{
    ble_gap_conn_sec_mode_t write_perm; /**< Write permissions */
    uint8_t                 vloc : 2;   /**< Value location */
    uint8_t                *p_value;    /**< Pointer to name (if vloc=USER) */
    uint16_t                current_len; /**< Current length */
    uint16_t                max_len;    /**< Maximum length */
} ble_gap_cfg_device_name_t;

/**
 * @brief Vendor-specific UUID count configuration
 */
typedef struct
{
    uint8_t vs_uuid_count; /**< Number of vendor-specific UUIDs */
} ble_common_cfg_vs_uuid_t;

/**
 * @brief GATTS attribute table size configuration
 */
typedef struct
{
    uint32_t attr_tab_size; /**< Attribute table size in bytes */
} ble_gatts_cfg_attr_tab_size_t;

/**
 * @brief Service changed configuration
 */
typedef struct
{
    uint8_t service_changed : 1; /**< Service changed characteristic included */
} ble_gatts_cfg_service_changed_t;

/**
 * @brief BLE configuration union
 */
typedef union
{
    ble_conn_cfg_t                conn_cfg;              /**< Connection configuration */
    ble_common_cfg_vs_uuid_t      common_cfg;            /**< VS UUID configuration */
    ble_gap_cfg_role_count_t      gap_role_count;        /**< GAP role count */
    ble_gap_cfg_device_name_t     gap_device_name;       /**< Device name */
    ble_gatts_cfg_attr_tab_size_t gatts_attr_tab_size;   /**< GATTS attr table size */
    ble_gatts_cfg_service_changed_t gatts_service_changed; /**< Service changed */
} ble_cfg_params_t;

/**
 * @brief BLE configuration structure
 */
typedef struct
{
    ble_cfg_params_t params; /**< Configuration parameters */
} ble_cfg_t;

/** @} */

/** @defgroup BLE_EVENTS BLE Event Definitions
 * @{ */

/** @brief BLE Event IDs base values */
#define BLE_EVT_BASE                    0x01
#define BLE_GAP_EVT_BASE                0x10
#define BLE_GATTC_EVT_BASE              0x30
#define BLE_GATTS_EVT_BASE              0x50
#define BLE_L2CAP_EVT_BASE              0x70

/** @brief Common BLE Event IDs */
typedef enum
{
    BLE_EVT_USER_MEM_REQUEST  = BLE_EVT_BASE,     /**< User memory request */
    BLE_EVT_USER_MEM_RELEASE  = BLE_EVT_BASE + 1, /**< User memory release */
} ble_common_evt_id_t;

/**
 * @brief User memory request event structure
 */
typedef struct
{
    uint8_t type; /**< Memory type: 0 = ATT, 1 = L2CAP */
} ble_common_evt_user_mem_request_t;

/**
 * @brief User memory release event structure
 */
typedef struct
{
    uint8_t  type;         /**< Memory type */
    uint8_t *mem_block_ptr; /**< Pointer to released memory */
    uint16_t mem_block_len; /**< Length of released memory */
} ble_common_evt_user_mem_release_t;

/**
 * @brief Common BLE event union
 */
typedef union
{
    ble_common_evt_user_mem_request_t user_mem_request; /**< User memory request */
    ble_common_evt_user_mem_release_t user_mem_release; /**< User memory release */
} ble_common_evt_t;

/**
 * @brief Main BLE event structure
 *
 * All BLE events from the SoftDevice are delivered in this format.
 */
typedef struct
{
    ble_evt_hdr_t header; /**< Event header */
    union
    {
        ble_common_evt_t common_evt; /**< Common events */
        ble_gap_evt_t    gap_evt;    /**< GAP events */
        ble_gatts_evt_t  gatts_evt;  /**< GATTS events */
        /* Note: GATTC and L2CAP events omitted for brevity */
    } evt; /**< Event data */
} ble_evt_t;

/** @brief Recommended buffer size for BLE events */
#define BLE_EVT_LEN_MAX(ATT_MTU)  (offsetof(ble_evt_t, evt.gatts_evt.params.write.data) + (ATT_MTU))

/** @} */

/** @defgroup BLE_VERSION BLE Version Information
 * @{ */

/**
 * @brief BLE version information structure
 */
typedef struct
{
    uint8_t  version_number;    /**< Link Layer version number */
    uint16_t company_id;        /**< Company identifier */
    uint16_t subversion_number; /**< Link Layer subversion */
} ble_version_t;

/** @} */

/** @defgroup BLE_API BLE Common API Functions
 * @{ */

/**
 * @brief Enable the BLE stack
 *
 * This function enables the BLE stack and must be called after sd_softdevice_enable()
 * and after all sd_ble_cfg_set() calls.
 *
 * @param[in,out] p_app_ram_base  Pointer to application RAM base. On input: desired base.
 *                                On output: required base (may be higher if more RAM needed).
 *
 * @retval NRF_SUCCESS               BLE enabled successfully
 * @retval NRF_ERROR_INVALID_STATE   SoftDevice not enabled
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 * @retval NRF_ERROR_NO_MEM          Not enough RAM for BLE configuration
 */
SVCALL(SD_BLE_ENABLE, uint32_t, sd_ble_enable(uint32_t *p_app_ram_base));

/**
 * @brief Set BLE configuration
 *
 * Must be called before sd_ble_enable(). Multiple calls can be made to set
 * different configuration options.
 *
 * @param[in] cfg_id   Configuration ID, see @ref ble_cfg_id_t
 * @param[in] p_cfg    Pointer to configuration structure
 * @param[in] app_ram_base  Application RAM base address
 *
 * @retval NRF_SUCCESS               Configuration set successfully
 * @retval NRF_ERROR_INVALID_STATE   BLE already enabled
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM   Invalid configuration
 * @retval NRF_ERROR_NO_MEM          Configuration requires more RAM than available
 */
SVCALL(SD_BLE_CFG_SET, uint32_t, sd_ble_cfg_set(
    uint32_t cfg_id,
    const ble_cfg_t *p_cfg,
    uint32_t app_ram_base));

/**
 * @brief Get a BLE event from the pending events queue
 *
 * Call this when an event is signaled (typically via SWI2 interrupt).
 *
 * @param[out]    p_dest    Pointer to buffer for event data
 * @param[in,out] p_len     On input: buffer size. On output: event size.
 *
 * @retval NRF_SUCCESS               Event retrieved successfully
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 * @retval NRF_ERROR_NOT_FOUND       No events pending
 * @retval NRF_ERROR_DATA_SIZE       Buffer too small (required size in p_len)
 */
SVCALL(SD_BLE_EVT_GET, uint32_t, sd_ble_evt_get(uint8_t *p_dest, uint16_t *p_len));

/**
 * @brief Add a vendor-specific UUID base
 *
 * Register a 128-bit UUID base to use vendor-specific UUIDs in services/characteristics.
 * After registration, use the returned type with 16-bit UUIDs.
 *
 * @param[in]  p_vs_uuid  Pointer to 128-bit UUID base (little-endian)
 * @param[out] p_uuid_type Pointer to store UUID type for use with ble_uuid_t
 *
 * @retval NRF_SUCCESS               UUID registered successfully
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 * @retval NRF_ERROR_NO_MEM          UUID table full
 */
SVCALL(SD_BLE_UUID_VS_ADD, uint32_t, sd_ble_uuid_vs_add(
    const ble_uuid128_t *p_vs_uuid,
    uint8_t *p_uuid_type));

/**
 * @brief Decode a UUID from raw bytes
 *
 * @param[in]  uuid_le_len  Length of UUID data (2 or 16 bytes)
 * @param[in]  p_uuid_le    Pointer to UUID data (little-endian)
 * @param[out] p_uuid       Pointer to decoded UUID structure
 *
 * @retval NRF_SUCCESS               UUID decoded successfully
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 * @retval NRF_ERROR_INVALID_LENGTH  Invalid UUID length
 * @retval NRF_ERROR_NOT_FOUND       UUID base not registered
 */
SVCALL(SD_BLE_UUID_DECODE, uint32_t, sd_ble_uuid_decode(
    uint8_t uuid_le_len,
    const uint8_t *p_uuid_le,
    ble_uuid_t *p_uuid));

/**
 * @brief Encode a UUID to raw bytes
 *
 * @param[in]  p_uuid       Pointer to UUID structure
 * @param[out] p_uuid_le_len Pointer to store length of encoded UUID
 * @param[out] p_uuid_le    Buffer for encoded UUID (must be at least 16 bytes)
 *
 * @retval NRF_SUCCESS               UUID encoded successfully
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM   Invalid UUID type
 */
SVCALL(SD_BLE_UUID_ENCODE, uint32_t, sd_ble_uuid_encode(
    const ble_uuid_t *p_uuid,
    uint8_t *p_uuid_le_len,
    uint8_t *p_uuid_le));

/**
 * @brief Get BLE version information
 *
 * @param[out] p_version  Pointer to store version information
 *
 * @retval NRF_SUCCESS               Version retrieved successfully
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 */
SVCALL(SD_BLE_VERSION_GET, uint32_t, sd_ble_version_get(ble_version_t *p_version));

/**
 * @brief Reply to user memory request
 *
 * Call this in response to BLE_EVT_USER_MEM_REQUEST.
 *
 * @param[in] conn_handle  Connection handle
 * @param[in] p_block      Pointer to memory block (NULL to decline)
 *
 * @retval NRF_SUCCESS                      Reply sent successfully
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 * @retval NRF_ERROR_INVALID_STATE          No pending memory request
 */
typedef struct
{
    uint8_t *p_mem;  /**< Pointer to memory block */
    uint16_t len;    /**< Length of memory block */
} ble_user_mem_block_t;

SVCALL(SD_BLE_USER_MEM_REPLY, uint32_t, sd_ble_user_mem_reply(
    uint16_t conn_handle,
    const ble_user_mem_block_t *p_block));

/**
 * @brief Set a BLE option
 *
 * @param[in] opt_id  Option ID
 * @param[in] p_opt   Pointer to option value
 *
 * @retval NRF_SUCCESS               Option set successfully
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM   Invalid option
 */
SVCALL(SD_BLE_OPT_SET, uint32_t, sd_ble_opt_set(uint32_t opt_id, const void *p_opt));

/**
 * @brief Get a BLE option
 *
 * @param[in]  opt_id  Option ID
 * @param[out] p_opt   Pointer to store option value
 *
 * @retval NRF_SUCCESS               Option retrieved successfully
 * @retval NRF_ERROR_INVALID_ADDR    Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM   Invalid option
 */
SVCALL(SD_BLE_OPT_GET, uint32_t, sd_ble_opt_get(uint32_t opt_id, void *p_opt));

/** @} */

/** @defgroup BLE_HELPERS Helper Macros and Functions
 * @{ */

/**
 * @brief Set security mode to open (no security)
 */
#define BLE_GAP_CONN_SEC_MODE_SET_OPEN(ptr) \
    do { (ptr)->sm = 1; (ptr)->lv = 1; } while(0)

/**
 * @brief Set security mode to no access
 */
#define BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(ptr) \
    do { (ptr)->sm = 0; (ptr)->lv = 0; } while(0)

/**
 * @brief Set security mode to require encryption
 */
#define BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(ptr) \
    do { (ptr)->sm = 1; (ptr)->lv = 2; } while(0)

/**
 * @brief Set security mode to require encryption with MITM
 */
#define BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(ptr) \
    do { (ptr)->sm = 1; (ptr)->lv = 3; } while(0)

/**
 * @brief Set security mode to require LESC encryption
 */
#define BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM(ptr) \
    do { (ptr)->sm = 1; (ptr)->lv = 4; } while(0)

/**
 * @brief Set security mode to require signing
 */
#define BLE_GAP_CONN_SEC_MODE_SET_SIGNED_NO_MITM(ptr) \
    do { (ptr)->sm = 2; (ptr)->lv = 1; } while(0)

/**
 * @brief Set security mode to require signing with MITM
 */
#define BLE_GAP_CONN_SEC_MODE_SET_SIGNED_WITH_MITM(ptr) \
    do { (ptr)->sm = 2; (ptr)->lv = 2; } while(0)

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BLE_H__ */
