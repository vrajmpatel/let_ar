/**
 * @file nrf_svc.h
 * @brief Nordic SDK Supervisor Call (SVC) Macros for S140 SoftDevice
 *
 * Provides macros for making SVC calls to the SoftDevice.
 * S140 SoftDevice uses SVC instructions to transfer control from application to SoftDevice.
 *
 * Reference: Nordic nRF5 SDK, ARM Cortex-M4 Technical Reference Manual
 */

#ifndef NRF_SVC_H__
#define NRF_SVC_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup SVC_NUM SoftDevice SVC Numbers
 * @brief SVC numbers for S140 SoftDevice API calls
 *
 * SoftDevice Manager (SDM): 0x10-0x1F
 * SoC Library: 0x20-0x2F
 * BLE: 0x60-0xBF
 *
 * @{ */

/* SoftDevice Manager SVC numbers */
#define SD_SOFTDEVICE_ENABLE                  0x10
#define SD_SOFTDEVICE_DISABLE                 0x11
#define SD_SOFTDEVICE_IS_ENABLED              0x12
#define SD_SOFTDEVICE_VECTOR_TABLE_BASE_SET   0x13

/* SoC Library SVC numbers */
#define SD_PPI_CHANNEL_ENABLE_GET             0x20
#define SD_PPI_CHANNEL_ENABLE_SET             0x21
#define SD_PPI_CHANNEL_ENABLE_CLR             0x22
#define SD_PPI_CHANNEL_ASSIGN                 0x23
#define SD_PPI_GROUP_TASK_ENABLE              0x24
#define SD_PPI_GROUP_TASK_DISABLE             0x25
#define SD_PPI_GROUP_ASSIGN                   0x26
#define SD_PPI_GROUP_GET                      0x27
#define SD_FLASH_PAGE_ERASE                   0x28
#define SD_FLASH_WRITE                        0x29
#define SD_PROTECTED_REGISTER_WRITE           0x2A
#define SD_MUTEX_NEW                          0x2B
#define SD_MUTEX_ACQUIRE                      0x2C
#define SD_MUTEX_RELEASE                      0x2D
#define SD_RAND_APPLICATION_POOL_CAPACITY_GET 0x2E
#define SD_RAND_APPLICATION_BYTES_AVAILABLE_GET 0x2F
#define SD_RAND_APPLICATION_VECTOR_GET        0x30
#define SD_POWER_MODE_SET                     0x31
#define SD_POWER_SYSTEM_OFF                   0x32
#define SD_POWER_RESET_REASON_GET             0x33
#define SD_POWER_RESET_REASON_CLR             0x34
#define SD_POWER_POF_ENABLE                   0x35
#define SD_POWER_POF_THRESHOLD_SET            0x36
#define SD_POWER_POF_THRESHOLD_GET            0x37
#define SD_POWER_RAM_POWER_SET                0x38
#define SD_POWER_RAM_POWER_CLR                0x39
#define SD_POWER_RAM_POWER_GET                0x3A
#define SD_POWER_GPREGRET_SET                 0x3B
#define SD_POWER_GPREGRET_CLR                 0x3C
#define SD_POWER_GPREGRET_GET                 0x3D
#define SD_POWER_DCDC_MODE_SET                0x3E
#define SD_APP_EVT_WAIT                       0x3F
#define SD_CLOCK_HFCLK_REQUEST                0x40
#define SD_CLOCK_HFCLK_RELEASE                0x41
#define SD_CLOCK_HFCLK_IS_RUNNING             0x42
#define SD_NVIC_ENABLEIRQ                     0x43
#define SD_NVIC_DISABLEIRQ                    0x44
#define SD_NVIC_GETPENDINGIRQ                 0x45
#define SD_NVIC_SETPENDINGIRQ                 0x46
#define SD_NVIC_CLEARPENDINGIRQ               0x47
#define SD_NVIC_SETPRIORITY                   0x48
#define SD_NVIC_GETPRIORITY                   0x49
#define SD_NVIC_SYSTEMRESET                   0x4A
#define SD_NVIC_CRITICAL_REGION_ENTER         0x4B
#define SD_NVIC_CRITICAL_REGION_EXIT          0x4C
#define SD_ECB_BLOCK_ENCRYPT                  0x4D
#define SD_ECB_BLOCKS_ENCRYPT                 0x4E
#define SD_RADIO_NOTIFICATION_CFG_SET         0x4F
#define SD_RADIO_SESSION_OPEN                 0x50
#define SD_RADIO_SESSION_CLOSE                0x51
#define SD_RADIO_REQUEST                      0x52
#define SD_EVT_GET                            0x53
#define SD_TEMP_GET                           0x54
#define SD_POWER_USBPWRRDY_ENABLE             0x55
#define SD_POWER_USBDETECTED_ENABLE           0x56
#define SD_POWER_USBREMOVED_ENABLE            0x57
#define SD_POWER_USBREGSTATUS_GET             0x58

/* BLE SVC numbers */
#define SD_BLE_ENABLE                         0x60
#define SD_BLE_EVT_GET                        0x61
#define SD_BLE_UUID_VS_ADD                    0x62
#define SD_BLE_UUID_DECODE                    0x63
#define SD_BLE_UUID_ENCODE                    0x64
#define SD_BLE_VERSION_GET                    0x65
#define SD_BLE_USER_MEM_REPLY                 0x66
#define SD_BLE_OPT_SET                        0x67
#define SD_BLE_OPT_GET                        0x68
#define SD_BLE_CFG_SET                        0x69
#define SD_BLE_UUID_VS_REMOVE                 0x6A

/* BLE GAP SVC numbers */
#define SD_BLE_GAP_ADDR_SET                   0x6C
#define SD_BLE_GAP_ADDR_GET                   0x6D
#define SD_BLE_GAP_WHITELIST_SET              0x6E
#define SD_BLE_GAP_DEVICE_IDENTITIES_SET      0x6F
#define SD_BLE_GAP_PRIVACY_SET                0x70
#define SD_BLE_GAP_PRIVACY_GET                0x71
#define SD_BLE_GAP_ADV_SET_CONFIGURE          0x72
#define SD_BLE_GAP_ADV_START                  0x73
#define SD_BLE_GAP_ADV_STOP                   0x74
#define SD_BLE_GAP_CONN_PARAM_UPDATE          0x75
#define SD_BLE_GAP_DISCONNECT                 0x76
#define SD_BLE_GAP_TX_POWER_SET               0x77
#define SD_BLE_GAP_APPEARANCE_SET             0x78
#define SD_BLE_GAP_APPEARANCE_GET             0x79
#define SD_BLE_GAP_PPCP_SET                   0x7A
#define SD_BLE_GAP_PPCP_GET                   0x7B
#define SD_BLE_GAP_DEVICE_NAME_SET            0x7C
#define SD_BLE_GAP_DEVICE_NAME_GET            0x7D
#define SD_BLE_GAP_AUTHENTICATE               0x7E
#define SD_BLE_GAP_SEC_PARAMS_REPLY           0x7F
#define SD_BLE_GAP_AUTH_KEY_REPLY             0x80
#define SD_BLE_GAP_LESC_DHKEY_REPLY           0x81
#define SD_BLE_GAP_KEYPRESS_NOTIFY            0x82
#define SD_BLE_GAP_LESC_OOB_DATA_GET          0x83
#define SD_BLE_GAP_LESC_OOB_DATA_SET          0x84
#define SD_BLE_GAP_ENCRYPT                    0x85
#define SD_BLE_GAP_SEC_INFO_REPLY             0x86
#define SD_BLE_GAP_CONN_SEC_GET               0x87
#define SD_BLE_GAP_RSSI_START                 0x88
#define SD_BLE_GAP_RSSI_STOP                  0x89
#define SD_BLE_GAP_SCAN_START                 0x8A
#define SD_BLE_GAP_SCAN_STOP                  0x8B
#define SD_BLE_GAP_CONNECT                    0x8C
#define SD_BLE_GAP_CONNECT_CANCEL             0x8D
#define SD_BLE_GAP_RSSI_GET                   0x8E
#define SD_BLE_GAP_PHY_UPDATE                 0x8F
#define SD_BLE_GAP_PHY_REQUEST                0x90
#define SD_BLE_GAP_DATA_LENGTH_UPDATE         0x91
#define SD_BLE_GAP_QOS_CHANNEL_SURVEY_START   0x92
#define SD_BLE_GAP_QOS_CHANNEL_SURVEY_STOP    0x93
#define SD_BLE_GAP_ADV_ADDR_GET               0x94
#define SD_BLE_GAP_NEXT_CONN_EVT_COUNTER_GET  0x95
#define SD_BLE_GAP_CONN_EVT_TRIGGER_START     0x96
#define SD_BLE_GAP_CONN_EVT_TRIGGER_STOP      0x97

/* BLE GATTC SVC numbers */
#define SD_BLE_GATTC_PRIMARY_SERVICES_DISCOVER 0x98
#define SD_BLE_GATTC_RELATIONSHIPS_DISCOVER   0x99
#define SD_BLE_GATTC_CHARACTERISTICS_DISCOVER 0x9A
#define SD_BLE_GATTC_DESCRIPTORS_DISCOVER     0x9B
#define SD_BLE_GATTC_ATTR_INFO_DISCOVER       0x9C
#define SD_BLE_GATTC_CHAR_VALUE_BY_UUID_READ  0x9D
#define SD_BLE_GATTC_READ                     0x9E
#define SD_BLE_GATTC_CHAR_VALUES_READ         0x9F
#define SD_BLE_GATTC_WRITE                    0xA0
#define SD_BLE_GATTC_HV_CONFIRM               0xA1
#define SD_BLE_GATTC_EXCHANGE_MTU_REQUEST     0xA2

/* BLE GATTS SVC numbers */
#define SD_BLE_GATTS_SERVICE_ADD              0xA3
#define SD_BLE_GATTS_INCLUDE_ADD              0xA4
#define SD_BLE_GATTS_CHARACTERISTIC_ADD       0xA5
#define SD_BLE_GATTS_DESCRIPTOR_ADD           0xA6
#define SD_BLE_GATTS_VALUE_SET                0xA7
#define SD_BLE_GATTS_VALUE_GET                0xA8
#define SD_BLE_GATTS_HVX                      0xA9
#define SD_BLE_GATTS_SERVICE_CHANGED          0xAA
#define SD_BLE_GATTS_RW_AUTHORIZE_REPLY       0xAB
#define SD_BLE_GATTS_SYS_ATTR_SET             0xAC
#define SD_BLE_GATTS_SYS_ATTR_GET             0xAD
#define SD_BLE_GATTS_INITIAL_USER_HANDLE_GET  0xAE
#define SD_BLE_GATTS_ATTR_GET                 0xAF
#define SD_BLE_GATTS_EXCHANGE_MTU_REPLY       0xB0

/* BLE L2CAP SVC numbers */
#define SD_BLE_L2CAP_CH_SETUP                 0xB1
#define SD_BLE_L2CAP_CH_RELEASE               0xB2
#define SD_BLE_L2CAP_CH_RX                    0xB3
#define SD_BLE_L2CAP_CH_TX                    0xB4
#define SD_BLE_L2CAP_CH_FLOW_CONTROL          0xB5

/** @} */

/**
 * @brief Macro for calling SoftDevice functions with SVC
 *
 * This macro uses inline assembly to execute an SVC instruction.
 * The SVC number determines which SoftDevice function is called.
 *
 * @param[in] number  SVC number
 */
#ifdef __GNUC__
#define SVCALL(number, return_type, signature) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wreturn-type\"") \
    __attribute__((naked, unused)) static return_type signature \
    { \
        __asm volatile ( \
            "svc %[svc_num]\n\t" \
            "bx lr\n\t" \
            : : [svc_num] "i" (number) \
        ); \
    } \
    _Pragma("GCC diagnostic pop")
#else
#error "Unsupported compiler"
#endif

/**
 * @brief Alternative SVC call macro for functions returning void
 */
#define SVCALL_VOID(number, signature) \
    __attribute__((naked, unused)) static void signature \
    { \
        __asm volatile ( \
            "svc %0\n" \
            "bx lr\n" \
            : : "I" ((uint8_t)(number)) \
        ); \
    }

#ifdef __cplusplus
}
#endif

#endif /* NRF_SVC_H__ */
