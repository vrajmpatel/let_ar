/**
 * @file ble_gatt.h
 * @brief BLE Generic Attribute Profile (GATT) Common Definitions for S140 SoftDevice
 *
 * This header provides common GATT definitions shared between GATT client and server.
 *
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x Vol 3 Part G
 */

#ifndef BLE_GATT_H__
#define BLE_GATT_H__

#include <stdint.h>
#include "ble_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BLE_GATT_DEFINES GATT Common Definitions
 * @{ */

/** @brief Default ATT MTU size (23 bytes)
 * Reference: Bluetooth Core Spec Vol 3 Part F Section 3.2.9 */
#define BLE_GATT_ATT_MTU_DEFAULT            23

/** @brief Maximum ATT MTU size supported by S140 (247 bytes) */
#define BLE_GATT_ATT_MTU_MAX                247

/** @brief Handle value used to reference all attributes */
#define BLE_GATT_HANDLE_START               0x0001

/** @brief Invalid handle value */
#define BLE_GATT_HANDLE_END                 0xFFFF

/** @} */

/** @defgroup BLE_GATT_STATUS GATT Status Codes
 * @brief GATT error codes as defined in Bluetooth Core Spec Vol 3 Part F Section 3.4.1.1
 * @{ */

#define BLE_GATT_STATUS_SUCCESS                       0x0000 /**< Success */
#define BLE_GATT_STATUS_UNKNOWN                       0x0001 /**< Unknown or not applicable status */

/* ATT Error codes (0x0100 + ATT error) */
#define BLE_GATT_STATUS_ATTERR_INVALID_HANDLE         0x0101 /**< Invalid handle */
#define BLE_GATT_STATUS_ATTERR_READ_NOT_PERMITTED     0x0102 /**< Read not permitted */
#define BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED    0x0103 /**< Write not permitted */
#define BLE_GATT_STATUS_ATTERR_INVALID_PDU            0x0104 /**< Invalid PDU */
#define BLE_GATT_STATUS_ATTERR_INSUF_AUTHENTICATION   0x0105 /**< Insufficient authentication */
#define BLE_GATT_STATUS_ATTERR_REQUEST_NOT_SUPPORTED  0x0106 /**< Request not supported */
#define BLE_GATT_STATUS_ATTERR_INVALID_OFFSET         0x0107 /**< Invalid offset */
#define BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION    0x0108 /**< Insufficient authorization */
#define BLE_GATT_STATUS_ATTERR_PREPARE_QUEUE_FULL     0x0109 /**< Prepare queue full */
#define BLE_GATT_STATUS_ATTERR_ATTRIBUTE_NOT_FOUND    0x010A /**< Attribute not found */
#define BLE_GATT_STATUS_ATTERR_ATTRIBUTE_NOT_LONG     0x010B /**< Attribute not long */
#define BLE_GATT_STATUS_ATTERR_INSUF_ENC_KEY_SIZE     0x010C /**< Insufficient encryption key size */
#define BLE_GATT_STATUS_ATTERR_INVALID_ATT_VAL_LENGTH 0x010D /**< Invalid attribute value length */
#define BLE_GATT_STATUS_ATTERR_UNLIKELY_ERROR         0x010E /**< Unlikely error */
#define BLE_GATT_STATUS_ATTERR_INSUF_ENCRYPTION       0x010F /**< Insufficient encryption */
#define BLE_GATT_STATUS_ATTERR_UNSUPPORTED_GROUP_TYPE 0x0110 /**< Unsupported group type */
#define BLE_GATT_STATUS_ATTERR_INSUF_RESOURCES        0x0111 /**< Insufficient resources */

/* Application specific error codes (0x0180-0x019F) */
#define BLE_GATT_STATUS_ATTERR_APP_BEGIN              0x0180 /**< Application error start */
#define BLE_GATT_STATUS_ATTERR_APP_END                0x019F /**< Application error end */

/* Common Profile and Service Error Codes (0x01E0-0x01FF) */
#define BLE_GATT_STATUS_ATTERR_CPS_WRITE_REQ_REJECTED 0x01FC /**< Write request rejected */
#define BLE_GATT_STATUS_ATTERR_CPS_CCCD_CONFIG_ERROR  0x01FD /**< CCCD improperly configured */
#define BLE_GATT_STATUS_ATTERR_CPS_PROC_ALR_IN_PROG   0x01FE /**< Procedure already in progress */
#define BLE_GATT_STATUS_ATTERR_CPS_OUT_OF_RANGE       0x01FF /**< Out of range */

/** @} */

/** @defgroup BLE_GATT_OPS GATT Operations
 * @{ */

/** @brief GATT operation types */
typedef enum
{
    BLE_GATT_OP_INVALID        = 0x00, /**< Invalid operation */
    BLE_GATT_OP_WRITE_REQ      = 0x01, /**< Write request */
    BLE_GATT_OP_WRITE_CMD      = 0x02, /**< Write command (no response) */
    BLE_GATT_OP_SIGN_WRITE_CMD = 0x03, /**< Signed write command */
    BLE_GATT_OP_PREP_WRITE_REQ = 0x04, /**< Prepare write request */
    BLE_GATT_OP_EXEC_WRITE_REQ = 0x05, /**< Execute write request */
} ble_gatt_write_op_t;

/** @brief GATT HVX (Handle Value Notification/Indication) types */
typedef enum
{
    BLE_GATT_HVX_INVALID       = 0x00, /**< Invalid */
    BLE_GATT_HVX_NOTIFICATION  = 0x01, /**< Handle Value Notification */
    BLE_GATT_HVX_INDICATION    = 0x02, /**< Handle Value Indication */
} ble_gatt_hvx_type_t;

/** @brief Execute write flags */
typedef enum
{
    BLE_GATT_EXEC_WRITE_FLAG_PREPARED_CANCEL = 0x00, /**< Cancel prepared writes */
    BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE  = 0x01, /**< Execute prepared writes */
} ble_gatt_exec_write_flag_t;

/** @} */

/** @defgroup BLE_GATT_CHAR_PROPS Characteristic Properties
 * @brief Characteristic properties as defined in Bluetooth Core Spec Vol 3 Part G Section 3.3.1.1
 * @{ */

#define BLE_GATT_CHAR_PROPS_BROADCAST         0x01 /**< Broadcasting permitted */
#define BLE_GATT_CHAR_PROPS_READ              0x02 /**< Reading permitted */
#define BLE_GATT_CHAR_PROPS_WRITE_WO_RESP     0x04 /**< Writing without response permitted */
#define BLE_GATT_CHAR_PROPS_WRITE             0x08 /**< Writing with response permitted */
#define BLE_GATT_CHAR_PROPS_NOTIFY            0x10 /**< Notifications permitted */
#define BLE_GATT_CHAR_PROPS_INDICATE          0x20 /**< Indications permitted */
#define BLE_GATT_CHAR_PROPS_AUTH_SIGNED_WR    0x40 /**< Signed writes permitted */
#define BLE_GATT_CHAR_PROPS_EXT_PROPS         0x80 /**< Extended properties present */

/** @brief Characteristic properties structure */
typedef struct
{
    uint8_t broadcast       : 1; /**< Broadcasting permitted */
    uint8_t read            : 1; /**< Reading permitted */
    uint8_t write_wo_resp   : 1; /**< Write without response permitted */
    uint8_t write           : 1; /**< Write with response permitted */
    uint8_t notify          : 1; /**< Notification permitted */
    uint8_t indicate        : 1; /**< Indication permitted */
    uint8_t auth_signed_wr  : 1; /**< Signed write permitted */
} ble_gatt_char_props_t;

/** @brief Extended characteristic properties */
typedef struct
{
    uint8_t reliable_wr : 1; /**< Reliable write permitted */
    uint8_t wr_aux      : 1; /**< Writable auxiliaries */
} ble_gatt_char_ext_props_t;

/** @} */

/** @defgroup BLE_GATT_CPFD Characteristic Presentation Format Descriptor
 * @brief Format values as defined in Bluetooth Core Spec
 * @{ */

/** @brief Characteristic format types */
typedef enum
{
    BLE_GATT_CPF_FORMAT_RFU         = 0x00, /**< Reserved for future use */
    BLE_GATT_CPF_FORMAT_BOOLEAN     = 0x01, /**< Boolean */
    BLE_GATT_CPF_FORMAT_2BIT        = 0x02, /**< Unsigned 2-bit integer */
    BLE_GATT_CPF_FORMAT_NIBBLE      = 0x03, /**< Unsigned 4-bit integer */
    BLE_GATT_CPF_FORMAT_UINT8       = 0x04, /**< Unsigned 8-bit integer */
    BLE_GATT_CPF_FORMAT_UINT12      = 0x05, /**< Unsigned 12-bit integer */
    BLE_GATT_CPF_FORMAT_UINT16      = 0x06, /**< Unsigned 16-bit integer */
    BLE_GATT_CPF_FORMAT_UINT24      = 0x07, /**< Unsigned 24-bit integer */
    BLE_GATT_CPF_FORMAT_UINT32      = 0x08, /**< Unsigned 32-bit integer */
    BLE_GATT_CPF_FORMAT_UINT48      = 0x09, /**< Unsigned 48-bit integer */
    BLE_GATT_CPF_FORMAT_UINT64      = 0x0A, /**< Unsigned 64-bit integer */
    BLE_GATT_CPF_FORMAT_UINT128     = 0x0B, /**< Unsigned 128-bit integer */
    BLE_GATT_CPF_FORMAT_SINT8       = 0x0C, /**< Signed 8-bit integer */
    BLE_GATT_CPF_FORMAT_SINT12      = 0x0D, /**< Signed 12-bit integer */
    BLE_GATT_CPF_FORMAT_SINT16      = 0x0E, /**< Signed 16-bit integer */
    BLE_GATT_CPF_FORMAT_SINT24      = 0x0F, /**< Signed 24-bit integer */
    BLE_GATT_CPF_FORMAT_SINT32      = 0x10, /**< Signed 32-bit integer */
    BLE_GATT_CPF_FORMAT_SINT48      = 0x11, /**< Signed 48-bit integer */
    BLE_GATT_CPF_FORMAT_SINT64      = 0x12, /**< Signed 64-bit integer */
    BLE_GATT_CPF_FORMAT_SINT128     = 0x13, /**< Signed 128-bit integer */
    BLE_GATT_CPF_FORMAT_FLOAT32     = 0x14, /**< IEEE-754 32-bit floating point */
    BLE_GATT_CPF_FORMAT_FLOAT64     = 0x15, /**< IEEE-754 64-bit floating point */
    BLE_GATT_CPF_FORMAT_SFLOAT      = 0x16, /**< IEEE-11073 16-bit SFLOAT */
    BLE_GATT_CPF_FORMAT_FLOAT       = 0x17, /**< IEEE-11073 32-bit FLOAT */
    BLE_GATT_CPF_FORMAT_DUINT16     = 0x18, /**< IEEE-20601 format */
    BLE_GATT_CPF_FORMAT_UTF8S       = 0x19, /**< UTF-8 string */
    BLE_GATT_CPF_FORMAT_UTF16S      = 0x1A, /**< UTF-16 string */
    BLE_GATT_CPF_FORMAT_STRUCT      = 0x1B, /**< Opaque structure */
} ble_gatt_cpf_format_t;

/** @brief Characteristic Presentation Format Descriptor */
typedef struct
{
    uint8_t  format;      /**< Format of the value, see @ref ble_gatt_cpf_format_t */
    int8_t   exponent;    /**< Exponent for integer data types */
    uint16_t unit;        /**< Unit UUID from Bluetooth SIG assigned numbers */
    uint8_t  name_space;  /**< Name space (0x01 = Bluetooth SIG) */
    uint16_t desc;        /**< Description (enumeration from name_space) */
} ble_gatt_char_pf_t;

/** @} */

/** @defgroup BLE_GATT_UNITS GATT Unit UUIDs
 * @brief Selected GATT Unit UUIDs from Bluetooth SIG assigned numbers
 * @{ */

#define BLE_GATT_UNIT_UNITLESS                              0x2700 /**< Unitless */
#define BLE_GATT_UNIT_LENGTH_METRE                          0x2701 /**< Length: metre */
#define BLE_GATT_UNIT_MASS_KILOGRAM                         0x2702 /**< Mass: kilogram */
#define BLE_GATT_UNIT_TIME_SECOND                           0x2703 /**< Time: second */
#define BLE_GATT_UNIT_ELECTRIC_CURRENT_AMPERE               0x2704 /**< Electric current: ampere */
#define BLE_GATT_UNIT_THERMODYNAMIC_TEMPERATURE_KELVIN      0x2705 /**< Temperature: kelvin */
#define BLE_GATT_UNIT_AMOUNT_OF_SUBSTANCE_MOLE              0x2706 /**< Amount: mole */
#define BLE_GATT_UNIT_LUMINOUS_INTENSITY_CANDELA            0x2707 /**< Luminous intensity: candela */
#define BLE_GATT_UNIT_AREA_SQUARE_METRES                    0x2710 /**< Area: square metres */
#define BLE_GATT_UNIT_VOLUME_CUBIC_METRES                   0x2711 /**< Volume: cubic metres */
#define BLE_GATT_UNIT_VELOCITY_METRES_PER_SECOND            0x2712 /**< Velocity: m/s */
#define BLE_GATT_UNIT_ACCELERATION_METRES_PER_SECOND_SQ     0x2713 /**< Acceleration: m/s² */
#define BLE_GATT_UNIT_DENSITY_KILOGRAM_PER_CUBIC_METRE      0x2714 /**< Density: kg/m³ */
#define BLE_GATT_UNIT_FREQUENCY_HERTZ                       0x2722 /**< Frequency: Hz */
#define BLE_GATT_UNIT_FORCE_NEWTON                          0x2723 /**< Force: newton */
#define BLE_GATT_UNIT_PRESSURE_PASCAL                       0x2724 /**< Pressure: pascal */
#define BLE_GATT_UNIT_ENERGY_JOULE                          0x2725 /**< Energy: joule */
#define BLE_GATT_UNIT_POWER_WATT                            0x2726 /**< Power: watt */
#define BLE_GATT_UNIT_ELECTRIC_CHARGE_COULOMB               0x2727 /**< Charge: coulomb */
#define BLE_GATT_UNIT_ELECTRIC_POTENTIAL_VOLTAGE            0x2728 /**< Voltage: volt */
#define BLE_GATT_UNIT_MAGNETIC_FLUX_DENSITY_TESLA           0x272C /**< Magnetic flux density: tesla */
#define BLE_GATT_UNIT_CELSIUS_TEMPERATURE_DEGREE_CELSIUS    0x272F /**< Temperature: °C */
#define BLE_GATT_UNIT_PLANE_ANGLE_DEGREE                    0x2763 /**< Angle: degree */
#define BLE_GATT_UNIT_PLANE_ANGLE_RADIAN                    0x2720 /**< Angle: radian */
#define BLE_GATT_UNIT_ANGULAR_VELOCITY_RADIAN_PER_SECOND    0x2743 /**< Angular velocity: rad/s */
#define BLE_GATT_UNIT_ANGULAR_VELOCITY_REVOLUTION_PER_MIN   0x27A7 /**< Angular velocity: rpm */
#define BLE_GATT_UNIT_PERCENTAGE                            0x27AD /**< Percentage */
#define BLE_GATT_UNIT_PER_MILLE                             0x27AE /**< Per mille */
#define BLE_GATT_UNIT_PERIOD_BEATS_PER_MINUTE               0x27AF /**< Beats per minute */

/** @} */

/** @defgroup BLE_GATT_TIMEOUT GATT Timeout Definitions
 * @{ */

/** @brief GATT transaction timeout in seconds */
#define BLE_GATT_TIMEOUT_SECONDS                30

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BLE_GATT_H__ */
