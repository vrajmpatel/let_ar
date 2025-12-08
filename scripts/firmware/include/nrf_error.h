/**
 * @file nrf_error.h
 * @brief Nordic SDK Error Codes for S140 SoftDevice
 *
 * Error codes are based on Nordic SDK 17.x conventions.
 * Reference: Nordic nRF5 SDK documentation
 */

#ifndef NRF_ERROR_H__
#define NRF_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup NRF_ERRORS_BASE Error Codes Base numbers
 * @{ */
#define NRF_ERROR_BASE_NUM      (0x0)       /**< Global error base */
#define NRF_ERROR_SDM_BASE_NUM  (0x1000)    /**< SDM error base */
#define NRF_ERROR_SOC_BASE_NUM  (0x2000)    /**< SoC error base */
#define NRF_ERROR_STK_BASE_NUM  (0x3000)    /**< STK error base */
/** @} */

/** @defgroup NRF_ERROR_CODES Global Error Codes
 * @{ */
#define NRF_SUCCESS                           (NRF_ERROR_BASE_NUM + 0)  /**< Successful command */
#define NRF_ERROR_SVC_HANDLER_MISSING         (NRF_ERROR_BASE_NUM + 1)  /**< SVC handler is missing */
#define NRF_ERROR_SOFTDEVICE_NOT_ENABLED      (NRF_ERROR_BASE_NUM + 2)  /**< SoftDevice has not been enabled */
#define NRF_ERROR_INTERNAL                    (NRF_ERROR_BASE_NUM + 3)  /**< Internal Error */
#define NRF_ERROR_NO_MEM                      (NRF_ERROR_BASE_NUM + 4)  /**< No Memory for operation */
#define NRF_ERROR_NOT_FOUND                   (NRF_ERROR_BASE_NUM + 5)  /**< Not found */
#define NRF_ERROR_NOT_SUPPORTED               (NRF_ERROR_BASE_NUM + 6)  /**< Not supported */
#define NRF_ERROR_INVALID_PARAM               (NRF_ERROR_BASE_NUM + 7)  /**< Invalid Parameter */
#define NRF_ERROR_INVALID_STATE               (NRF_ERROR_BASE_NUM + 8)  /**< Invalid state, operation disallowed in this state */
#define NRF_ERROR_INVALID_LENGTH              (NRF_ERROR_BASE_NUM + 9)  /**< Invalid Length */
#define NRF_ERROR_INVALID_FLAGS               (NRF_ERROR_BASE_NUM + 10) /**< Invalid Flags */
#define NRF_ERROR_INVALID_DATA                (NRF_ERROR_BASE_NUM + 11) /**< Invalid Data */
#define NRF_ERROR_DATA_SIZE                   (NRF_ERROR_BASE_NUM + 12) /**< Invalid Data size */
#define NRF_ERROR_TIMEOUT                     (NRF_ERROR_BASE_NUM + 13) /**< Operation timed out */
#define NRF_ERROR_NULL                        (NRF_ERROR_BASE_NUM + 14) /**< Null Pointer */
#define NRF_ERROR_FORBIDDEN                   (NRF_ERROR_BASE_NUM + 15) /**< Forbidden Operation */
#define NRF_ERROR_INVALID_ADDR                (NRF_ERROR_BASE_NUM + 16) /**< Bad Memory Address */
#define NRF_ERROR_BUSY                        (NRF_ERROR_BASE_NUM + 17) /**< Busy */
#define NRF_ERROR_CONN_COUNT                  (NRF_ERROR_BASE_NUM + 18) /**< Maximum connection count exceeded */
#define NRF_ERROR_RESOURCES                   (NRF_ERROR_BASE_NUM + 19) /**< Not enough resources for operation */
/** @} */

/** @defgroup NRF_ERROR_SDM_CODES SoftDevice Manager Error Codes
 * @{ */
#define NRF_ERROR_SDM_LFCLK_SOURCE_UNKNOWN    (NRF_ERROR_SDM_BASE_NUM + 0)  /**< Unknown LFCLK source */
#define NRF_ERROR_SDM_INCORRECT_INTERRUPT_CONFIGURATION (NRF_ERROR_SDM_BASE_NUM + 1) /**< Incorrect interrupt configuration */
#define NRF_ERROR_SDM_INCORRECT_CLENR0        (NRF_ERROR_SDM_BASE_NUM + 2)  /**< Incorrect CLENR0 */
/** @} */

/** @defgroup NRF_ERROR_SOC_CODES SoC Library Error Codes
 * @{ */
#define NRF_ERROR_SOC_MUTEX_ALREADY_TAKEN     (NRF_ERROR_SOC_BASE_NUM + 0)  /**< Mutex already taken */
#define NRF_ERROR_SOC_NVIC_INTERRUPT_NOT_AVAILABLE (NRF_ERROR_SOC_BASE_NUM + 1) /**< NVIC interrupt not available */
#define NRF_ERROR_SOC_NVIC_INTERRUPT_PRIORITY_NOT_ALLOWED (NRF_ERROR_SOC_BASE_NUM + 2) /**< NVIC interrupt priority not allowed */
#define NRF_ERROR_SOC_NVIC_SHOULD_NOT_RETURN  (NRF_ERROR_SOC_BASE_NUM + 3)  /**< NVIC should not return */
#define NRF_ERROR_SOC_POWER_MODE_UNKNOWN      (NRF_ERROR_SOC_BASE_NUM + 4)  /**< Power mode unknown */
#define NRF_ERROR_SOC_POWER_POF_THRESHOLD_UNKNOWN (NRF_ERROR_SOC_BASE_NUM + 5) /**< Power POF threshold unknown */
#define NRF_ERROR_SOC_POWER_OFF_SHOULD_NOT_RETURN (NRF_ERROR_SOC_BASE_NUM + 6) /**< Power off should not return */
#define NRF_ERROR_SOC_RAND_NOT_ENOUGH_VALUES  (NRF_ERROR_SOC_BASE_NUM + 7)  /**< RAND not enough values */
#define NRF_ERROR_SOC_PPI_INVALID_CHANNEL     (NRF_ERROR_SOC_BASE_NUM + 8)  /**< Invalid PPI Channel */
#define NRF_ERROR_SOC_PPI_INVALID_GROUP       (NRF_ERROR_SOC_BASE_NUM + 9)  /**< Invalid PPI Group */
/** @} */

/** @defgroup BLE_ERROR_CODES BLE Error Codes
 * @{ */
#define BLE_ERROR_NOT_ENABLED                 (NRF_ERROR_STK_BASE_NUM + 1)  /**< BLE not enabled */
#define BLE_ERROR_INVALID_CONN_HANDLE         (NRF_ERROR_STK_BASE_NUM + 2)  /**< Invalid connection handle */
#define BLE_ERROR_INVALID_ATTR_HANDLE         (NRF_ERROR_STK_BASE_NUM + 3)  /**< Invalid attribute handle */
#define BLE_ERROR_INVALID_ADV_HANDLE          (NRF_ERROR_STK_BASE_NUM + 4)  /**< Invalid advertising handle */
#define BLE_ERROR_INVALID_ROLE                (NRF_ERROR_STK_BASE_NUM + 5)  /**< Invalid role */
#define BLE_ERROR_BLOCKED_BY_OTHER_LINKS      (NRF_ERROR_STK_BASE_NUM + 6)  /**< Blocked by other links */
/** @} */

/** @brief Macro for checking error codes and returning on error */
#define APP_ERROR_CHECK(err_code) \
    do { \
        if ((err_code) != NRF_SUCCESS) { \
            app_error_handler((err_code), __LINE__, (uint8_t*)__FILE__); \
        } \
    } while (0)

/** @brief Macro for checking error codes without handler call */
#define VERIFY_SUCCESS(err_code) \
    do { \
        if ((err_code) != NRF_SUCCESS) { \
            return (err_code); \
        } \
    } while (0)

/**
 * @brief Application error handler function prototype
 * @param[in] error_code  Error code supplied to the handler
 * @param[in] line_num    Line number where the handler is called
 * @param[in] p_file_name Pointer to the file name
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t *p_file_name);

#ifdef __cplusplus
}
#endif

#endif /* NRF_ERROR_H__ */
