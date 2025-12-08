/**
 * @file ble_types.h
 * @brief BLE Common Type Definitions for S140 SoftDevice
 *
 * This header defines common types used throughout the BLE API.
 * Reference: Nordic nRF5 SDK 17.x, Bluetooth Core Specification 5.x
 */

#ifndef BLE_TYPES_H__
#define BLE_TYPES_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup BLE_TYPES_DEFINES BLE Type Definitions
 * @{ */

/** @brief Invalid connection handle value */
#define BLE_CONN_HANDLE_INVALID         0xFFFF

/** @brief Handle for all connections (broadcast) */
#define BLE_CONN_HANDLE_ALL             0xFFFE

/** @brief Invalid GATT handle value */
#define BLE_GATT_HANDLE_INVALID         0x0000

/** @brief Invalid advertising handle value */
#define BLE_GAP_ADV_SET_HANDLE_NOT_SET  0xFF

/** @brief Maximum number of vendor specific UUIDs */
#define BLE_UUID_VS_MAX_COUNT           10

/** @brief Invalid UUID type */
#define BLE_UUID_TYPE_UNKNOWN           0x00

/** @brief 16-bit Bluetooth SIG UUID type */
#define BLE_UUID_TYPE_BLE               0x01

/** @brief Vendor specific UUID type (base starts at 2) */
#define BLE_UUID_TYPE_VENDOR_BEGIN      0x02

/** @} */

/** @defgroup BLE_COMMON_TYPES Common BLE Types
 * @{ */

/**
 * @brief UUID structure
 *
 * For 16-bit Bluetooth SIG UUIDs, use BLE_UUID_TYPE_BLE as type.
 * For 128-bit vendor-specific UUIDs, first register the base UUID
 * using sd_ble_uuid_vs_add(), then use the returned type.
 */
typedef struct
{
    uint16_t uuid; /**< 16-bit UUID value or octets 12-13 of 128-bit UUID */
    uint8_t  type; /**< UUID type, see @ref BLE_UUID_TYPE_UNKNOWN and @ref BLE_UUID_TYPE_BLE */
} ble_uuid_t;

/**
 * @brief 128-bit UUID structure
 *
 * Used for registering vendor-specific UUID bases with the SoftDevice.
 */
typedef struct
{
    uint8_t uuid128[16]; /**< 128-bit UUID value, little-endian */
} ble_uuid128_t;

/**
 * @brief Data structure for variable length data
 */
typedef struct
{
    uint8_t  *p_data; /**< Pointer to data buffer */
    uint16_t len;     /**< Length of data in bytes */
} ble_data_t;

/** @} */

/** @defgroup BLE_EVENT_TYPES BLE Event Base Types
 * @{ */

/** @brief Common BLE event header */
typedef struct
{
    uint16_t evt_id;    /**< Event identifier */
    uint16_t evt_len;   /**< Event length in bytes (including header) */
} ble_evt_hdr_t;

/** @} */

/** @defgroup BLE_UUID_STANDARD Standard Bluetooth SIG UUIDs
 * @brief Standard 16-bit UUIDs as defined by Bluetooth SIG
 * Reference: https://www.bluetooth.com/specifications/assigned-numbers/
 * @{ */

/* GATT Service UUIDs */
#define BLE_UUID_GAP                                0x1800 /**< Generic Access Profile */
#define BLE_UUID_GATT                               0x1801 /**< Generic Attribute Profile */
#define BLE_UUID_DEVICE_INFORMATION_SERVICE         0x180A /**< Device Information Service */
#define BLE_UUID_BATTERY_SERVICE                    0x180F /**< Battery Service */
#define BLE_UUID_HEART_RATE_SERVICE                 0x180D /**< Heart Rate Service */
#define BLE_UUID_HEALTH_THERMOMETER_SERVICE         0x1809 /**< Health Thermometer Service */
#define BLE_UUID_CURRENT_TIME_SERVICE               0x1805 /**< Current Time Service */
#define BLE_UUID_RUNNING_SPEED_AND_CADENCE_SERVICE  0x1814 /**< Running Speed and Cadence */
#define BLE_UUID_CYCLING_SPEED_AND_CADENCE_SERVICE  0x1816 /**< Cycling Speed and Cadence */
#define BLE_UUID_ENVIRONMENTAL_SENSING_SERVICE      0x181A /**< Environmental Sensing */
#define BLE_UUID_AUTOMATION_IO_SERVICE              0x1815 /**< Automation IO */

/* GATT Characteristic UUIDs */
#define BLE_UUID_DEVICE_NAME_CHAR                   0x2A00 /**< Device Name */
#define BLE_UUID_APPEARANCE_CHAR                    0x2A01 /**< Appearance */
#define BLE_UUID_PPCP_CHAR                          0x2A04 /**< Peripheral Preferred Connection Parameters */
#define BLE_UUID_SERVICE_CHANGED_CHAR               0x2A05 /**< Service Changed */
#define BLE_UUID_SYSTEM_ID_CHAR                     0x2A23 /**< System ID */
#define BLE_UUID_MODEL_NUMBER_STRING_CHAR           0x2A24 /**< Model Number String */
#define BLE_UUID_SERIAL_NUMBER_STRING_CHAR          0x2A25 /**< Serial Number String */
#define BLE_UUID_FIRMWARE_REVISION_STRING_CHAR      0x2A26 /**< Firmware Revision String */
#define BLE_UUID_HARDWARE_REVISION_STRING_CHAR      0x2A27 /**< Hardware Revision String */
#define BLE_UUID_SOFTWARE_REVISION_STRING_CHAR      0x2A28 /**< Software Revision String */
#define BLE_UUID_MANUFACTURER_NAME_STRING_CHAR      0x2A29 /**< Manufacturer Name String */
#define BLE_UUID_PNP_ID_CHAR                        0x2A50 /**< PnP ID */
#define BLE_UUID_BATTERY_LEVEL_CHAR                 0x2A19 /**< Battery Level */
#define BLE_UUID_TEMPERATURE_CHAR                   0x2A6E /**< Temperature */
#define BLE_UUID_HUMIDITY_CHAR                      0x2A6F /**< Humidity */
#define BLE_UUID_PRESSURE_CHAR                      0x2A6D /**< Pressure */

/* GATT Descriptor UUIDs */
#define BLE_UUID_CHAR_EXTENDED_PROPERTIES           0x2900 /**< Characteristic Extended Properties */
#define BLE_UUID_CHAR_USER_DESCRIPTION              0x2901 /**< Characteristic User Description */
#define BLE_UUID_CCCD                               0x2902 /**< Client Characteristic Configuration Descriptor */
#define BLE_UUID_SCCD                               0x2903 /**< Server Characteristic Configuration Descriptor */
#define BLE_UUID_CHAR_PRESENTATION_FORMAT           0x2904 /**< Characteristic Presentation Format */
#define BLE_UUID_CHAR_AGGREGATE_FORMAT              0x2905 /**< Characteristic Aggregate Format */
#define BLE_UUID_VALID_RANGE                        0x2906 /**< Valid Range */
#define BLE_UUID_REPORT_REFERENCE                   0x2908 /**< Report Reference */

/** @} */

/** @defgroup BLE_APPEARANCE Bluetooth Appearance Values
 * @brief Standard appearance values as defined by Bluetooth SIG
 * Reference: https://www.bluetooth.com/specifications/assigned-numbers/
 * @{ */

#define BLE_APPEARANCE_UNKNOWN                      0     /**< Unknown */
#define BLE_APPEARANCE_GENERIC_PHONE                64    /**< Generic Phone */
#define BLE_APPEARANCE_GENERIC_COMPUTER             128   /**< Generic Computer */
#define BLE_APPEARANCE_GENERIC_WATCH                192   /**< Generic Watch */
#define BLE_APPEARANCE_WATCH_SPORTS_WATCH           193   /**< Watch: Sports Watch */
#define BLE_APPEARANCE_GENERIC_CLOCK                256   /**< Generic Clock */
#define BLE_APPEARANCE_GENERIC_DISPLAY              320   /**< Generic Display */
#define BLE_APPEARANCE_GENERIC_REMOTE_CONTROL       384   /**< Generic Remote Control */
#define BLE_APPEARANCE_GENERIC_EYE_GLASSES          448   /**< Generic Eye-glasses */
#define BLE_APPEARANCE_GENERIC_TAG                  512   /**< Generic Tag */
#define BLE_APPEARANCE_GENERIC_KEYRING              576   /**< Generic Keyring */
#define BLE_APPEARANCE_GENERIC_MEDIA_PLAYER         640   /**< Generic Media Player */
#define BLE_APPEARANCE_GENERIC_BARCODE_SCANNER      704   /**< Generic Barcode Scanner */
#define BLE_APPEARANCE_GENERIC_THERMOMETER          768   /**< Generic Thermometer */
#define BLE_APPEARANCE_GENERIC_HEART_RATE_SENSOR    832   /**< Generic Heart Rate Sensor */
#define BLE_APPEARANCE_GENERIC_BLOOD_PRESSURE       896   /**< Generic Blood Pressure */
#define BLE_APPEARANCE_GENERIC_HID                  960   /**< Generic HID */
#define BLE_APPEARANCE_HID_KEYBOARD                 961   /**< HID Keyboard */
#define BLE_APPEARANCE_HID_MOUSE                    962   /**< HID Mouse */
#define BLE_APPEARANCE_HID_JOYSTICK                 963   /**< HID Joystick */
#define BLE_APPEARANCE_HID_GAMEPAD                  964   /**< HID Gamepad */
#define BLE_APPEARANCE_GENERIC_GLUCOSE_METER        1024  /**< Generic Glucose Meter */
#define BLE_APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR 1088 /**< Generic Running Walking Sensor */
#define BLE_APPEARANCE_RUNNING_WALKING_IN_SHOE      1089  /**< Running Walking Sensor: In-Shoe */
#define BLE_APPEARANCE_RUNNING_WALKING_ON_SHOE      1090  /**< Running Walking Sensor: On-Shoe */
#define BLE_APPEARANCE_RUNNING_WALKING_ON_HIP       1091  /**< Running Walking Sensor: On-Hip */
#define BLE_APPEARANCE_GENERIC_CYCLING              1152  /**< Generic Cycling */
#define BLE_APPEARANCE_CYCLING_COMPUTER             1153  /**< Cycling: Cycling Computer */
#define BLE_APPEARANCE_CYCLING_SPEED_SENSOR         1154  /**< Cycling: Speed Sensor */
#define BLE_APPEARANCE_CYCLING_CADENCE_SENSOR       1155  /**< Cycling: Cadence Sensor */
#define BLE_APPEARANCE_CYCLING_POWER_SENSOR         1156  /**< Cycling: Power Sensor */
#define BLE_APPEARANCE_CYCLING_SPEED_CADENCE_SENSOR 1157  /**< Cycling: Speed and Cadence Sensor */
#define BLE_APPEARANCE_GENERIC_OUTDOOR_SPORTS       5184  /**< Generic Outdoor Sports Activity */

/** @} */

/** @defgroup BLE_CONN_CFG Connection Configuration
 * @{ */

/** @brief Default MTU size (ATT_MTU) */
#define BLE_GATT_ATT_MTU_DEFAULT        23

/** @brief Maximum MTU size */
#define BLE_GATT_ATT_MTU_MAX            247

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BLE_TYPES_H__ */
