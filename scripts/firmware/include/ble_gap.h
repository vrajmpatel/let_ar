/**
 * @file ble_gap.h
 * @brief BLE Generic Access Profile (GAP) API for S140 SoftDevice
 *
 * This header provides the GAP API for advertising, scanning, connecting,
 * and managing BLE connections.
 *
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x
 */

#ifndef BLE_GAP_H__
#define BLE_GAP_H__

#include <stdint.h>
#include "nrf_svc.h"
#include "ble_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BLE_GAP_ADDR BLE GAP Address Definitions
 * @{ */

/** @brief GAP address length in bytes */
#define BLE_GAP_ADDR_LEN            6

/** @brief Address types */
typedef enum
{
    BLE_GAP_ADDR_TYPE_PUBLIC                        = 0x00, /**< Public address */
    BLE_GAP_ADDR_TYPE_RANDOM_STATIC                 = 0x01, /**< Random static address */
    BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE     = 0x02, /**< Random private resolvable address */
    BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE = 0x03, /**< Random private non-resolvable address */
    BLE_GAP_ADDR_TYPE_ANONYMOUS                     = 0x7F, /**< Anonymous address (adv only) */
} ble_gap_addr_type_t;

/** @brief GAP Bluetooth device address */
typedef struct
{
    uint8_t addr_id_peer : 1;  /**< Only valid for peer addresses: 1 = address is identity address */
    uint8_t addr_type    : 7;  /**< Address type, see @ref ble_gap_addr_type_t */
    uint8_t addr[BLE_GAP_ADDR_LEN]; /**< 48-bit address, LSB format */
} ble_gap_addr_t;

/** @} */

/** @defgroup BLE_GAP_ADV BLE GAP Advertising Definitions
 * @{ */

/** @brief Maximum advertising data length for legacy advertising
 * Reference: Bluetooth Core Spec 5.0, Vol 6, Part B, Section 2.3.1 */
#define BLE_GAP_ADV_SET_DATA_SIZE_MAX               31

/** @brief Maximum advertising data length for extended advertising */
#define BLE_GAP_ADV_SET_DATA_SIZE_EXTENDED_MAX_SUPPORTED 255

/** @brief Minimum advertising interval (in 0.625ms units) - 20ms */
#define BLE_GAP_ADV_INTERVAL_MIN                    0x0020

/** @brief Maximum advertising interval (in 0.625ms units) - 10.24s */
#define BLE_GAP_ADV_INTERVAL_MAX                    0x4000

/** @brief Default advertising interval for fast connection - 100ms */
#define BLE_GAP_ADV_INTERVAL_DEFAULT                160

/** @brief Advertising types */
typedef enum
{
    BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED      = 0x01, /**< Connectable and scannable undirected (ADV_IND) */
    BLE_GAP_ADV_TYPE_CONNECTABLE_NONSCANNABLE_DIRECTED_HIGH_DUTY_CYCLE = 0x02, /**< Connectable directed high duty cycle */
    BLE_GAP_ADV_TYPE_CONNECTABLE_NONSCANNABLE_DIRECTED     = 0x03, /**< Connectable directed */
    BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED   = 0x04, /**< Non-connectable scannable undirected (ADV_SCAN_IND) */
    BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED = 0x05, /**< Non-connectable non-scannable undirected (ADV_NONCONN_IND) */
    BLE_GAP_ADV_TYPE_EXTENDED_CONNECTABLE_NONSCANNABLE_UNDIRECTED = 0x06, /**< Extended connectable */
    BLE_GAP_ADV_TYPE_EXTENDED_CONNECTABLE_NONSCANNABLE_DIRECTED = 0x07, /**< Extended connectable directed */
    BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_SCANNABLE_UNDIRECTED = 0x08, /**< Extended non-connectable scannable */
    BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_SCANNABLE_DIRECTED = 0x09, /**< Extended non-connectable scannable directed */
    BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED = 0x0A, /**< Extended non-connectable non-scannable */
    BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_NONSCANNABLE_DIRECTED = 0x0B, /**< Extended non-connectable non-scannable directed */
} ble_gap_adv_type_t;

/** @brief Advertising filter policy */
typedef enum
{
    BLE_GAP_ADV_FP_ANY           = 0x00, /**< Allow scan and connect requests from any device */
    BLE_GAP_ADV_FP_FILTER_SCANREQ = 0x01, /**< Filter scan requests with whitelist */
    BLE_GAP_ADV_FP_FILTER_CONNREQ = 0x02, /**< Filter connect requests with whitelist */
    BLE_GAP_ADV_FP_FILTER_BOTH    = 0x03, /**< Filter both scan and connect with whitelist */
} ble_gap_adv_filter_policy_t;

/** @brief Advertising PHY */
typedef enum
{
    BLE_GAP_PHY_AUTO   = 0x00, /**< Automatic PHY selection */
    BLE_GAP_PHY_1MBPS  = 0x01, /**< 1 Mbps PHY */
    BLE_GAP_PHY_2MBPS  = 0x02, /**< 2 Mbps PHY */
    BLE_GAP_PHY_CODED  = 0x04, /**< Coded PHY (long range) */
} ble_gap_phy_t;

/** @brief Primary advertising PHY options for extended advertising */
typedef struct
{
    uint8_t primary_phy; /**< Primary advertising PHY, see @ref ble_gap_phy_t */
    uint8_t secondary_phy; /**< Secondary advertising PHY, see @ref ble_gap_phy_t */
} ble_gap_adv_phys_t;

/** @brief Advertising properties (for extended advertising) */
typedef struct
{
    uint8_t type;                    /**< Advertising type, see @ref ble_gap_adv_type_t */
    uint8_t anonymous        : 1;    /**< Advertise without device address */
    uint8_t include_tx_power : 1;    /**< Include TX power in extended header */
} ble_gap_adv_properties_t;

/** @brief GAP advertising parameters */
typedef struct
{
    ble_gap_adv_properties_t properties;   /**< Advertising properties */
    ble_gap_addr_t const    *p_peer_addr;  /**< For directed advertising: peer address */
    uint32_t                 interval;     /**< Advertising interval in 0.625ms units */
    uint16_t                 duration;     /**< Advertising duration in 10ms units. 0 = infinite */
    uint8_t                  max_adv_evts; /**< Max advertising events. 0 = no limit */
    uint8_t                  channel_mask; /**< Channel mask. Bit 0=Ch37, Bit 1=Ch38, Bit 2=Ch39. 0=all channels */
    uint8_t                  filter_policy; /**< Filter policy, see @ref ble_gap_adv_filter_policy_t */
    uint8_t                  primary_phy;  /**< Primary PHY, see @ref ble_gap_phy_t */
    uint8_t                  secondary_phy; /**< Secondary PHY (for extended), see @ref ble_gap_phy_t */
    uint8_t                  set_id        : 4; /**< Advertising Set Identifier (0-15) */
    uint8_t                  scan_req_notification : 1; /**< Enable scan request notification events */
} ble_gap_adv_params_t;

/** @brief Advertising data buffers */
typedef struct
{
    ble_data_t adv_data;      /**< Advertising data */
    ble_data_t scan_rsp_data; /**< Scan response data */
} ble_gap_adv_data_t;

/** @} */

/** @defgroup BLE_GAP_CONN BLE GAP Connection Definitions
 * @{ */

/** @brief Minimum connection interval (in 1.25ms units) - 7.5ms */
#define BLE_GAP_CP_MIN_CONN_INTVL_MIN       0x0006

/** @brief Maximum connection interval (in 1.25ms units) - 4000ms */
#define BLE_GAP_CP_MIN_CONN_INTVL_MAX       0x0C80

/** @brief Slave latency - no latency */
#define BLE_GAP_CP_SLAVE_LATENCY_MIN        0x0000

/** @brief Maximum slave latency */
#define BLE_GAP_CP_SLAVE_LATENCY_MAX        0x01F3

/** @brief Minimum supervision timeout (in 10ms units) - 100ms */
#define BLE_GAP_CP_CONN_SUP_TIMEOUT_MIN     0x000A

/** @brief Maximum supervision timeout (in 10ms units) - 32000ms */
#define BLE_GAP_CP_CONN_SUP_TIMEOUT_MAX     0x0C80

/** @brief Preferred peripheral connection parameters */
typedef struct
{
    uint16_t min_conn_interval; /**< Min connection interval (1.25ms units) */
    uint16_t max_conn_interval; /**< Max connection interval (1.25ms units) */
    uint16_t slave_latency;     /**< Slave latency */
    uint16_t conn_sup_timeout;  /**< Supervision timeout (10ms units) */
} ble_gap_conn_params_t;

/** @brief GAP connection security modes
 * Reference: Bluetooth Core Spec, Vol 3, Part C, Section 10.2 */
typedef struct
{
    uint8_t sm : 4; /**< Security Mode (1-2) */
    uint8_t lv : 4; /**< Security Level (1-4) */
} ble_gap_conn_sec_mode_t;

/** @brief Connection security status */
typedef struct
{
    ble_gap_conn_sec_mode_t sec_mode; /**< Current security mode and level */
    uint8_t                 encr_key_size; /**< Encryption key size in bytes (7-16) */
} ble_gap_conn_sec_t;

/** @brief Roles in a connection */
typedef enum
{
    BLE_GAP_ROLE_INVALID = 0x00, /**< Invalid role */
    BLE_GAP_ROLE_PERIPH  = 0x01, /**< Peripheral role */
    BLE_GAP_ROLE_CENTRAL = 0x02, /**< Central role */
} ble_gap_role_t;

/** @} */

/** @defgroup BLE_GAP_SEC BLE GAP Security Definitions
 * @{ */

/** @brief Security key length */
#define BLE_GAP_SEC_KEY_LEN         16

/** @brief GAP Security Status Codes
 * Reference: Bluetooth Core Spec Vol 3, Part H, Section 3.5.5 - Pairing Failed
 */
typedef enum
{
    BLE_GAP_SEC_STATUS_SUCCESS                = 0x00, /**< Procedure completed successfully */
    BLE_GAP_SEC_STATUS_PASSKEY_ENTRY_FAILED   = 0x01, /**< Passkey entry failed */
    BLE_GAP_SEC_STATUS_OOB_NOT_AVAILABLE      = 0x02, /**< Out of Band data not available */
    BLE_GAP_SEC_STATUS_AUTH_REQ               = 0x03, /**< Authentication requirements not met */
    BLE_GAP_SEC_STATUS_CONFIRM_VALUE          = 0x04, /**< Confirm value failed */
    BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP       = 0x05, /**< Pairing not supported */
    BLE_GAP_SEC_STATUS_ENC_KEY_SIZE           = 0x06, /**< Encryption key size insufficient */
    BLE_GAP_SEC_STATUS_SMP_CMD_UNSUPPORTED    = 0x07, /**< SMP command not supported */
    BLE_GAP_SEC_STATUS_UNSPECIFIED            = 0x08, /**< Unspecified reason */
    BLE_GAP_SEC_STATUS_REPEATED_ATTEMPTS      = 0x09, /**< Too many pairing attempts */
    BLE_GAP_SEC_STATUS_INVALID_PARAMS         = 0x0A, /**< Invalid parameters */
    BLE_GAP_SEC_STATUS_DHKEY_FAILURE          = 0x0B, /**< DHKey check failure */
    BLE_GAP_SEC_STATUS_NUM_COMP_FAILURE       = 0x0C, /**< Numeric comparison failure */
    BLE_GAP_SEC_STATUS_BR_EDR_IN_PROG         = 0x0D, /**< BR/EDR pairing in progress */
    BLE_GAP_SEC_STATUS_X_TRANS_KEY_DISALLOWED = 0x0E, /**< Cross-transport key derivation not allowed */
} ble_gap_sec_status_t;

/** @brief Security IO capabilities */
typedef enum
{
    BLE_GAP_IO_CAPS_DISPLAY_ONLY      = 0x00, /**< Display only */
    BLE_GAP_IO_CAPS_DISPLAY_YESNO     = 0x01, /**< Display and Yes/No entry */
    BLE_GAP_IO_CAPS_KEYBOARD_ONLY     = 0x02, /**< Keyboard only */
    BLE_GAP_IO_CAPS_NONE              = 0x03, /**< No IO capabilities */
    BLE_GAP_IO_CAPS_KEYBOARD_DISPLAY  = 0x04, /**< Keyboard and display */
} ble_gap_io_caps_t;

/** @brief Security parameters */
typedef struct
{
    uint8_t  bond         : 1; /**< Bonding enabled */
    uint8_t  mitm         : 1; /**< MITM protection required */
    uint8_t  lesc         : 1; /**< LE Secure Connections pairing required */
    uint8_t  keypress     : 1; /**< Keypress notifications enabled */
    uint8_t  io_caps      : 3; /**< IO capabilities, see @ref ble_gap_io_caps_t */
    uint8_t  oob          : 1; /**< Out of band data available */
    uint8_t  min_key_size;     /**< Minimum encryption key size (7-16) */
    uint8_t  max_key_size;     /**< Maximum encryption key size (7-16) */
    struct {
        uint8_t enc  : 1;      /**< Encryption key distribution */
        uint8_t id   : 1;      /**< Identity key distribution */
        uint8_t sign : 1;      /**< Signing key distribution */
        uint8_t link : 1;      /**< Link key derivation */
    } kdist_own;               /**< Key distribution for local device */
    struct {
        uint8_t enc  : 1;      /**< Encryption key distribution */
        uint8_t id   : 1;      /**< Identity key distribution */
        uint8_t sign : 1;      /**< Signing key distribution */
        uint8_t link : 1;      /**< Link key derivation */
    } kdist_peer;              /**< Key distribution for peer device */
} ble_gap_sec_params_t;

/** @} */

/** @defgroup BLE_GAP_DATA_TYPE Advertising Data Types (AD Types)
 * @brief Standard AD Types as defined by Bluetooth SIG
 * Reference: https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/
 * @{ */

#define BLE_GAP_AD_TYPE_FLAGS                               0x01 /**< Flags */
#define BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE   0x02 /**< Partial list of 16-bit UUIDs */
#define BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE         0x03 /**< Complete list of 16-bit UUIDs */
#define BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_MORE_AVAILABLE   0x04 /**< Partial list of 32-bit UUIDs */
#define BLE_GAP_AD_TYPE_32BIT_SERVICE_UUID_COMPLETE         0x05 /**< Complete list of 32-bit UUIDs */
#define BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE  0x06 /**< Partial list of 128-bit UUIDs */
#define BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE        0x07 /**< Complete list of 128-bit UUIDs */
#define BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME                    0x08 /**< Shortened local name */
#define BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME                 0x09 /**< Complete local name */
#define BLE_GAP_AD_TYPE_TX_POWER_LEVEL                      0x0A /**< TX power level */
#define BLE_GAP_AD_TYPE_CLASS_OF_DEVICE                     0x0D /**< Class of device */
#define BLE_GAP_AD_TYPE_SIMPLE_PAIRING_HASH_C               0x0E /**< Simple pairing hash C */
#define BLE_GAP_AD_TYPE_SIMPLE_PAIRING_RANDOMIZER_R         0x0F /**< Simple pairing randomizer R */
#define BLE_GAP_AD_TYPE_SECURITY_MANAGER_TK_VALUE           0x10 /**< Security manager TK value */
#define BLE_GAP_AD_TYPE_SECURITY_MANAGER_OOB_FLAGS          0x11 /**< Security manager OOB flags */
#define BLE_GAP_AD_TYPE_SLAVE_CONNECTION_INTERVAL_RANGE     0x12 /**< Slave connection interval range */
#define BLE_GAP_AD_TYPE_SOLICITED_SERVICE_UUIDS_16BIT       0x14 /**< List of 16-bit solicitation UUIDs */
#define BLE_GAP_AD_TYPE_SOLICITED_SERVICE_UUIDS_128BIT      0x15 /**< List of 128-bit solicitation UUIDs */
#define BLE_GAP_AD_TYPE_SERVICE_DATA                        0x16 /**< Service data - 16-bit UUID */
#define BLE_GAP_AD_TYPE_PUBLIC_TARGET_ADDRESS               0x17 /**< Public target address */
#define BLE_GAP_AD_TYPE_RANDOM_TARGET_ADDRESS               0x18 /**< Random target address */
#define BLE_GAP_AD_TYPE_APPEARANCE                          0x19 /**< Appearance */
#define BLE_GAP_AD_TYPE_ADVERTISING_INTERVAL                0x1A /**< Advertising interval */
#define BLE_GAP_AD_TYPE_LE_BLUETOOTH_DEVICE_ADDRESS         0x1B /**< LE Bluetooth device address */
#define BLE_GAP_AD_TYPE_LE_ROLE                             0x1C /**< LE role */
#define BLE_GAP_AD_TYPE_SIMPLE_PAIRING_HASH_C256            0x1D /**< Simple pairing hash C-256 */
#define BLE_GAP_AD_TYPE_SIMPLE_PAIRING_RANDOMIZER_R256      0x1E /**< Simple pairing randomizer R-256 */
#define BLE_GAP_AD_TYPE_SERVICE_DATA_32BIT_UUID             0x20 /**< Service data - 32-bit UUID */
#define BLE_GAP_AD_TYPE_SERVICE_DATA_128BIT_UUID            0x21 /**< Service data - 128-bit UUID */
#define BLE_GAP_AD_TYPE_LESC_CONFIRMATION_VALUE             0x22 /**< LE SC confirmation value */
#define BLE_GAP_AD_TYPE_LESC_RANDOM_VALUE                   0x23 /**< LE SC random value */
#define BLE_GAP_AD_TYPE_URI                                 0x24 /**< URI */
#define BLE_GAP_AD_TYPE_3D_INFORMATION_DATA                 0x3D /**< 3D information data */
#define BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA          0xFF /**< Manufacturer specific data */

/** @brief Advertising flags bit definitions */
#define BLE_GAP_ADV_FLAG_LE_LIMITED_DISC_MODE               0x01 /**< LE Limited Discoverable Mode */
#define BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE               0x02 /**< LE General Discoverable Mode */
#define BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED               0x04 /**< BR/EDR not supported */
#define BLE_GAP_ADV_FLAG_LE_BR_EDR_CONTROLLER               0x08 /**< LE and BR/EDR Controller */
#define BLE_GAP_ADV_FLAG_LE_BR_EDR_HOST                     0x10 /**< LE and BR/EDR Host */

/** @brief Typical advertising flags for BLE-only devices */
#define BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE         (BLE_GAP_ADV_FLAG_LE_LIMITED_DISC_MODE | BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED)
#define BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE         (BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE | BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED)

/** @} */

/** @defgroup BLE_GAP_DEVICE_NAME Device Name Configuration
 * @{ */

/** @brief Maximum device name length */
#define BLE_GAP_DEVNAME_MAX_LEN             248

/** @brief Default device name length limit */
#define BLE_GAP_DEVNAME_DEFAULT_LEN         31

/** @brief Device name write permission */
typedef enum
{
    BLE_GAP_WRITE_PERM_DISABLE = 0, /**< Write disabled */
    BLE_GAP_WRITE_PERM_OPEN    = 1, /**< Write permitted without authentication */
    BLE_GAP_WRITE_PERM_AUTH    = 2, /**< Write requires authentication */
} ble_gap_write_perm_t;

/** @brief Device name configuration */
typedef struct
{
    ble_gap_conn_sec_mode_t write_perm; /**< Write permissions for device name characteristic */
    uint8_t                 vloc : 2;   /**< Value location: 0=STACK, 1=USER */
    uint8_t                *p_value;    /**< Pointer to device name (if vloc=USER) */
    uint16_t                current_len; /**< Current device name length */
    uint16_t                max_len;    /**< Maximum device name length */
} ble_gap_device_name_cfg_t;

/** @} */

/** @defgroup BLE_GAP_TX_POWER TX Power Definitions
 * @brief TX Power levels supported by nRF52840
 * Reference: nRF52840 Product Specification - RADIO chapter
 * @{ */

#define BLE_GAP_TX_POWER_ROLE_ADV           0 /**< TX power for advertising */
#define BLE_GAP_TX_POWER_ROLE_SCAN_INIT     1 /**< TX power for scanning/initiating */
#define BLE_GAP_TX_POWER_ROLE_CONN          2 /**< TX power for connected */

/** @brief Supported TX power values for nRF52840 (in dBm):
 * -40, -20, -16, -12, -8, -4, 0, +2, +3, +4, +5, +6, +7, +8 */

/** @} */

/** @defgroup BLE_GAP_EVENTS BLE GAP Event Definitions
 * @{ */

/** @brief GAP Event IDs */
typedef enum
{
    BLE_GAP_EVT_CONNECTED                 = 0x10, /**< Connected to peer */
    BLE_GAP_EVT_DISCONNECTED              = 0x11, /**< Disconnected from peer */
    BLE_GAP_EVT_CONN_PARAM_UPDATE         = 0x12, /**< Connection parameters updated */
    BLE_GAP_EVT_SEC_PARAMS_REQUEST        = 0x13, /**< Security parameters request */
    BLE_GAP_EVT_SEC_INFO_REQUEST          = 0x14, /**< Security info request */
    BLE_GAP_EVT_PASSKEY_DISPLAY           = 0x15, /**< Display passkey */
    BLE_GAP_EVT_KEY_PRESSED               = 0x16, /**< Key pressed notification */
    BLE_GAP_EVT_AUTH_KEY_REQUEST          = 0x17, /**< Authentication key request */
    BLE_GAP_EVT_LESC_DHKEY_REQUEST        = 0x18, /**< LE SC DHKEY request */
    BLE_GAP_EVT_AUTH_STATUS               = 0x19, /**< Authentication status */
    BLE_GAP_EVT_CONN_SEC_UPDATE           = 0x1A, /**< Connection security updated */
    BLE_GAP_EVT_TIMEOUT                   = 0x1B, /**< Timeout */
    BLE_GAP_EVT_RSSI_CHANGED              = 0x1C, /**< RSSI changed */
    BLE_GAP_EVT_ADV_REPORT                = 0x1D, /**< Advertising report (scanner) */
    BLE_GAP_EVT_SEC_REQUEST               = 0x1E, /**< Security request from peer */
    BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST = 0x1F, /**< Connection parameter update request */
    BLE_GAP_EVT_SCAN_REQ_REPORT           = 0x20, /**< Scan request report */
    BLE_GAP_EVT_PHY_UPDATE_REQUEST        = 0x21, /**< PHY update request */
    BLE_GAP_EVT_PHY_UPDATE                = 0x22, /**< PHY updated */
    BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST = 0x23, /**< Data length update request */
    BLE_GAP_EVT_DATA_LENGTH_UPDATE        = 0x24, /**< Data length updated */
    BLE_GAP_EVT_QOS_CHANNEL_SURVEY_REPORT = 0x25, /**< QoS channel survey report */
    BLE_GAP_EVT_ADV_SET_TERMINATED        = 0x26, /**< Advertising set terminated */
} ble_gap_evt_id_t;

/** @brief Disconnect reasons (HCI error codes) */
typedef enum
{
    BLE_HCI_STATUS_CODE_SUCCESS                         = 0x00, /**< Success */
    BLE_HCI_STATUS_CODE_UNKNOWN_BTLE_COMMAND            = 0x01, /**< Unknown command */
    BLE_HCI_STATUS_CODE_UNKNOWN_CONNECTION_IDENTIFIER   = 0x02, /**< Unknown connection identifier */
    BLE_HCI_AUTHENTICATION_FAILURE                      = 0x05, /**< Authentication failure */
    BLE_HCI_STATUS_CODE_PIN_OR_KEY_MISSING              = 0x06, /**< PIN or key missing */
    BLE_HCI_MEMORY_CAPACITY_EXCEEDED                    = 0x07, /**< Memory capacity exceeded */
    BLE_HCI_CONNECTION_TIMEOUT                          = 0x08, /**< Connection timeout */
    BLE_HCI_STATUS_CODE_COMMAND_DISALLOWED              = 0x0C, /**< Command disallowed */
    BLE_HCI_STATUS_CODE_INVALID_BTLE_COMMAND_PARAMETERS = 0x12, /**< Invalid parameters */
    BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION           = 0x13, /**< Remote user terminated */
    BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES = 0x14, /**< Remote device low resources */
    BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF     = 0x15, /**< Remote device power off */
    BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION            = 0x16, /**< Local host terminated */
    BLE_HCI_UNSUPPORTED_REMOTE_FEATURE                  = 0x1A, /**< Unsupported remote feature */
    BLE_HCI_STATUS_CODE_INVALID_LMP_PARAMETERS          = 0x1E, /**< Invalid LMP parameters */
    BLE_HCI_STATUS_CODE_UNSPECIFIED_ERROR               = 0x1F, /**< Unspecified error */
    BLE_HCI_STATUS_CODE_LMP_RESPONSE_TIMEOUT            = 0x22, /**< LMP response timeout */
    BLE_HCI_STATUS_CODE_LMP_ERROR_TRANSACTION_COLLISION = 0x23, /**< Transaction collision */
    BLE_HCI_STATUS_CODE_LMP_PDU_NOT_ALLOWED             = 0x24, /**< PDU not allowed */
    BLE_HCI_INSTANT_PASSED                              = 0x28, /**< Instant passed */
    BLE_HCI_PAIRING_WITH_UNIT_KEY_UNSUPPORTED           = 0x29, /**< Unit key not supported */
    BLE_HCI_DIFFERENT_TRANSACTION_COLLISION             = 0x2A, /**< Different transaction collision */
    BLE_HCI_CONTROLLER_BUSY                             = 0x3A, /**< Controller busy */
    BLE_HCI_CONN_INTERVAL_UNACCEPTABLE                  = 0x3B, /**< Connection interval unacceptable */
    BLE_HCI_DIRECTED_ADVERTISER_TIMEOUT                 = 0x3C, /**< Directed advertiser timeout */
    BLE_HCI_CONN_TERMINATED_DUE_TO_MIC_FAILURE          = 0x3D, /**< MIC failure */
    BLE_HCI_CONN_FAILED_TO_BE_ESTABLISHED               = 0x3E, /**< Connection failed */
} ble_hci_status_code_t;

/** @brief GAP Connected event structure */
typedef struct
{
    ble_gap_addr_t        peer_addr;        /**< Peer address */
    uint8_t               role;             /**< Role, see @ref ble_gap_role_t */
    ble_gap_conn_params_t conn_params;      /**< Connection parameters */
    uint8_t               adv_handle;       /**< Advertising handle (peripheral only) */
    ble_data_t            adv_data;         /**< Received advertising data (central only) */
} ble_gap_evt_connected_t;

/** @brief GAP Disconnected event structure */
typedef struct
{
    uint8_t reason; /**< HCI error code, see @ref ble_hci_status_code_t */
} ble_gap_evt_disconnected_t;

/** @brief GAP Connection parameter update event structure */
typedef struct
{
    ble_gap_conn_params_t conn_params; /**< New connection parameters */
} ble_gap_evt_conn_param_update_t;

/** @brief GAP Connection parameter update request event structure */
typedef struct
{
    ble_gap_conn_params_t conn_params; /**< Requested connection parameters */
} ble_gap_evt_conn_param_update_request_t;

/** @brief GAP Timeout event structure */
typedef struct
{
    uint8_t src; /**< Timeout source: 0x01=Advertising, 0x02=Security request */
    union {
        ble_data_t adv_report_buffer; /**< For advertising timeout: unused buffer */
    } params;
} ble_gap_evt_timeout_t;

/** @brief GAP Advertising set terminated event structure */
typedef struct
{
    uint8_t  reason;      /**< Reason: 0x00=Timeout, 0x01=Limit reached, 0x05=Connected */
    uint8_t  adv_handle;  /**< Advertising handle */
    uint16_t conn_handle; /**< Connection handle (if connected) */
    uint8_t  num_completed_adv_events; /**< Number of completed advertising events */
} ble_gap_evt_adv_set_terminated_t;

/** @brief GAP PHY Update event structure */
typedef struct
{
    uint8_t status;   /**< Status of PHY update procedure */
    uint8_t tx_phy;   /**< TX PHY */
    uint8_t rx_phy;   /**< RX PHY */
} ble_gap_evt_phy_update_t;

/** @brief GAP Data Length Update event structure */
typedef struct
{
    uint16_t max_tx_octets;   /**< Maximum TX payload octets */
    uint16_t max_rx_octets;   /**< Maximum RX payload octets */
    uint16_t max_tx_time_us;  /**< Maximum TX time */
    uint16_t max_rx_time_us;  /**< Maximum RX time */
} ble_gap_data_length_params_t;

typedef struct
{
    ble_gap_data_length_params_t effective_params; /**< Effective data length parameters */
} ble_gap_evt_data_length_update_t;

/** @brief GAP Event union */
typedef union
{
    ble_gap_evt_connected_t                  connected;
    ble_gap_evt_disconnected_t               disconnected;
    ble_gap_evt_conn_param_update_t          conn_param_update;
    ble_gap_evt_conn_param_update_request_t  conn_param_update_request;
    ble_gap_evt_timeout_t                    timeout;
    ble_gap_evt_adv_set_terminated_t         adv_set_terminated;
    ble_gap_evt_phy_update_t                 phy_update;
    ble_gap_evt_data_length_update_t         data_length_update;
} ble_gap_evt_params_t;

/** @brief GAP Event structure */
typedef struct
{
    uint16_t            conn_handle; /**< Connection handle */
    ble_gap_evt_params_t params;     /**< Event parameters */
} ble_gap_evt_t;

/** @} */

/** @defgroup BLE_GAP_API BLE GAP API Functions
 * @{ */

/**
 * @brief Set the device address
 *
 * @param[in] p_addr  Pointer to address structure
 *
 * @retval NRF_SUCCESS             Address set successfully
 * @retval NRF_ERROR_INVALID_ADDR  Invalid pointer
 * @retval NRF_ERROR_INVALID_PARAM Invalid address type
 */
SVCALL(SD_BLE_GAP_ADDR_SET, uint32_t, sd_ble_gap_addr_set(const ble_gap_addr_t *p_addr));

/**
 * @brief Get the device address
 *
 * @param[out] p_addr  Pointer to store address
 *
 * @retval NRF_SUCCESS             Address retrieved successfully
 * @retval NRF_ERROR_INVALID_ADDR  Invalid pointer
 */
SVCALL(SD_BLE_GAP_ADDR_GET, uint32_t, sd_ble_gap_addr_get(ble_gap_addr_t *p_addr));

/**
 * @brief Set the device name
 *
 * @param[in] p_write_perm  Write permissions for the characteristic
 * @param[in] p_dev_name    Pointer to device name string (UTF-8)
 * @param[in] len           Length of device name
 *
 * @retval NRF_SUCCESS             Device name set successfully
 * @retval NRF_ERROR_INVALID_ADDR  Invalid pointer
 * @retval NRF_ERROR_INVALID_LENGTH Name too long
 */
SVCALL(SD_BLE_GAP_DEVICE_NAME_SET, uint32_t, sd_ble_gap_device_name_set(
    const ble_gap_conn_sec_mode_t *p_write_perm,
    const uint8_t *p_dev_name,
    uint16_t len));

/**
 * @brief Get the device name
 *
 * @param[out]    p_dev_name  Buffer for device name
 * @param[in,out] p_len       On input: buffer size. On output: name length.
 *
 * @retval NRF_SUCCESS             Device name retrieved successfully
 * @retval NRF_ERROR_DATA_SIZE     Buffer too small
 */
SVCALL(SD_BLE_GAP_DEVICE_NAME_GET, uint32_t, sd_ble_gap_device_name_get(
    uint8_t *p_dev_name,
    uint16_t *p_len));

/**
 * @brief Set the device appearance
 *
 * @param[in] appearance  Appearance value, see @ref BLE_APPEARANCE
 *
 * @retval NRF_SUCCESS  Appearance set successfully
 */
SVCALL(SD_BLE_GAP_APPEARANCE_SET, uint32_t, sd_ble_gap_appearance_set(uint16_t appearance));

/**
 * @brief Get the device appearance
 *
 * @param[out] p_appearance  Pointer to store appearance value
 *
 * @retval NRF_SUCCESS  Appearance retrieved successfully
 */
SVCALL(SD_BLE_GAP_APPEARANCE_GET, uint32_t, sd_ble_gap_appearance_get(uint16_t *p_appearance));

/**
 * @brief Set Peripheral Preferred Connection Parameters (PPCP)
 *
 * @param[in] p_conn_params  Pointer to preferred parameters
 *
 * @retval NRF_SUCCESS             PPCP set successfully
 * @retval NRF_ERROR_INVALID_PARAM Invalid parameters
 */
SVCALL(SD_BLE_GAP_PPCP_SET, uint32_t, sd_ble_gap_ppcp_set(const ble_gap_conn_params_t *p_conn_params));

/**
 * @brief Get Peripheral Preferred Connection Parameters
 *
 * @param[out] p_conn_params  Pointer to store parameters
 *
 * @retval NRF_SUCCESS  PPCP retrieved successfully
 */
SVCALL(SD_BLE_GAP_PPCP_GET, uint32_t, sd_ble_gap_ppcp_get(ble_gap_conn_params_t *p_conn_params));

/**
 * @brief Configure an advertising set
 *
 * @param[in,out] p_adv_handle  Advertising handle. BLE_GAP_ADV_SET_HANDLE_NOT_SET to allocate new.
 * @param[in]     p_adv_params  Advertising parameters
 * @param[in]     p_adv_data    Advertising and scan response data
 *
 * @retval NRF_SUCCESS             Advertising set configured
 * @retval NRF_ERROR_INVALID_STATE Advertising in progress
 * @retval NRF_ERROR_NO_MEM        No memory for advertising set
 */
SVCALL(SD_BLE_GAP_ADV_SET_CONFIGURE, uint32_t, sd_ble_gap_adv_set_configure(
    uint8_t *p_adv_handle,
    const ble_gap_adv_params_t *p_adv_params,
    const ble_gap_adv_data_t *p_adv_data));

/**
 * @brief Start advertising
 *
 * @param[in] adv_handle  Advertising handle from sd_ble_gap_adv_set_configure
 * @param[in] conn_cfg_tag Connection configuration tag
 *
 * @retval NRF_SUCCESS                  Advertising started
 * @retval BLE_ERROR_INVALID_ADV_HANDLE Invalid advertising handle
 * @retval NRF_ERROR_INVALID_STATE      Already advertising
 */
SVCALL(SD_BLE_GAP_ADV_START, uint32_t, sd_ble_gap_adv_start(uint8_t adv_handle, uint8_t conn_cfg_tag));

/**
 * @brief Stop advertising
 *
 * @param[in] adv_handle  Advertising handle to stop
 *
 * @retval NRF_SUCCESS                  Advertising stopped
 * @retval BLE_ERROR_INVALID_ADV_HANDLE Invalid advertising handle
 */
SVCALL(SD_BLE_GAP_ADV_STOP, uint32_t, sd_ble_gap_adv_stop(uint8_t adv_handle));

/**
 * @brief Set TX power for advertising/connection
 *
 * @param[in] role        Role: BLE_GAP_TX_POWER_ROLE_ADV, _SCAN_INIT, or _CONN
 * @param[in] handle      Advertising handle or connection handle
 * @param[in] tx_power    TX power in dBm
 *
 * @retval NRF_SUCCESS             TX power set successfully
 * @retval NRF_ERROR_INVALID_PARAM Invalid TX power value
 */
SVCALL(SD_BLE_GAP_TX_POWER_SET, uint32_t, sd_ble_gap_tx_power_set(
    uint8_t role,
    uint16_t handle,
    int8_t tx_power));

/**
 * @brief Request connection parameter update
 *
 * @param[in] conn_handle   Connection handle
 * @param[in] p_conn_params Requested connection parameters
 *
 * @retval NRF_SUCCESS                      Request sent
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 */
SVCALL(SD_BLE_GAP_CONN_PARAM_UPDATE, uint32_t, sd_ble_gap_conn_param_update(
    uint16_t conn_handle,
    const ble_gap_conn_params_t *p_conn_params));

/**
 * @brief Disconnect a connection
 *
 * @param[in] conn_handle  Connection handle
 * @param[in] hci_status_code HCI status code for disconnect reason
 *
 * @retval NRF_SUCCESS                      Disconnect initiated
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 */
SVCALL(SD_BLE_GAP_DISCONNECT, uint32_t, sd_ble_gap_disconnect(
    uint16_t conn_handle,
    uint8_t hci_status_code));

/**
 * @brief Request PHY update
 *
 * @param[in] conn_handle  Connection handle
 * @param[in] p_gap_phys   Pointer to PHY preference structure
 *
 * @retval NRF_SUCCESS                      Request sent
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 */
typedef struct {
    uint8_t tx_phys; /**< Preferred TX PHYs, see @ref ble_gap_phy_t */
    uint8_t rx_phys; /**< Preferred RX PHYs, see @ref ble_gap_phy_t */
} ble_gap_phys_t;

SVCALL(SD_BLE_GAP_PHY_UPDATE, uint32_t, sd_ble_gap_phy_update(
    uint16_t conn_handle,
    const ble_gap_phys_t *p_gap_phys));

/**
 * @brief Request data length update
 *
 * @param[in]  conn_handle    Connection handle
 * @param[in]  p_dl_params    Requested data length parameters (NULL for defaults)
 * @param[out] p_dl_limitation Data length limitations (can be NULL)
 *
 * @retval NRF_SUCCESS                      Request sent
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 */
typedef struct
{
    uint16_t max_tx_octets; /**< Maximum number of payload octets to send */
    uint16_t max_rx_octets; /**< Maximum number of payload octets to receive */
    uint16_t max_tx_time_us; /**< Maximum TX time in microseconds */
    uint16_t max_rx_time_us; /**< Maximum RX time in microseconds */
} ble_gap_data_length_t;

typedef struct
{
    uint16_t tx_payload_limited_octets; /**< TX payload limited octets */
    uint16_t rx_payload_limited_octets; /**< RX payload limited octets */
    uint16_t tx_rx_time_limited_us;     /**< TX/RX time limited in us */
} ble_gap_data_length_limitation_t;

SVCALL(SD_BLE_GAP_DATA_LENGTH_UPDATE, uint32_t, sd_ble_gap_data_length_update(
    uint16_t conn_handle,
    const ble_gap_data_length_params_t *p_dl_params,
    ble_gap_data_length_limitation_t *p_dl_limitation));

/**
 * @brief Start RSSI measurements
 *
 * @param[in] conn_handle    Connection handle
 * @param[in] threshold_dbm  RSSI threshold for reporting
 * @param[in] skip_count     Number of RSSI samples to skip between reports
 *
 * @retval NRF_SUCCESS                      RSSI started
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 */
SVCALL(SD_BLE_GAP_RSSI_START, uint32_t, sd_ble_gap_rssi_start(
    uint16_t conn_handle,
    uint8_t threshold_dbm,
    uint8_t skip_count));

/**
 * @brief Stop RSSI measurements
 *
 * @param[in] conn_handle  Connection handle
 *
 * @retval NRF_SUCCESS                      RSSI stopped
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 */
SVCALL(SD_BLE_GAP_RSSI_STOP, uint32_t, sd_ble_gap_rssi_stop(uint16_t conn_handle));

/**
 * @brief Get current RSSI
 *
 * @param[in]  conn_handle  Connection handle
 * @param[out] p_rssi       Pointer to store RSSI value
 * @param[out] p_ch_index   Pointer to store channel index (can be NULL)
 *
 * @retval NRF_SUCCESS                      RSSI retrieved
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 */
SVCALL(SD_BLE_GAP_RSSI_GET, uint32_t, sd_ble_gap_rssi_get(
    uint16_t conn_handle,
    int8_t *p_rssi,
    uint8_t *p_ch_index));

/**
 * @brief Reply to security parameters request
 *
 * @param[in] conn_handle   Connection handle
 * @param[in] sec_status    Security status
 * @param[in] p_sec_params  Security parameters
 * @param[in] p_sec_keyset  Key set
 *
 * @retval NRF_SUCCESS  Reply sent
 */
SVCALL(SD_BLE_GAP_SEC_PARAMS_REPLY, uint32_t, sd_ble_gap_sec_params_reply(
    uint16_t conn_handle,
    uint8_t sec_status,
    const ble_gap_sec_params_t *p_sec_params,
    const void *p_sec_keyset));

/**
 * @brief Get connection security level
 *
 * @param[in]  conn_handle  Connection handle
 * @param[out] p_conn_sec   Pointer to store security info
 *
 * @retval NRF_SUCCESS                      Security level retrieved
 * @retval BLE_ERROR_INVALID_CONN_HANDLE    Invalid connection handle
 */
SVCALL(SD_BLE_GAP_CONN_SEC_GET, uint32_t, sd_ble_gap_conn_sec_get(
    uint16_t conn_handle,
    ble_gap_conn_sec_t *p_conn_sec));

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BLE_GAP_H__ */
