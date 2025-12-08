/**
 * @file bno085.h
 * @brief BNO085 IMU sensor driver header
 * 
 * Driver interface for the Hillcrest BNO085 9-DOF IMU sensor.
 * Communicates via I2C using the SHTP (Sensor Hub Transport Protocol).
 * 
 * Citations:
 * - Adafruit BNO085 Guide: Sensor specifications and I2C interface
 * - FIRMWARE_DESIGN.md: Hardware overview and initialization sequence
 * - BNO08x Datasheet: SH-2 protocol and register definitions
 */

#ifndef BNO085_H
#define BNO085_H

#include <stdint.h>
#include <stdbool.h>
#include "shtp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * BNO085 Configuration Constants
 * Citation: Adafruit BNO085 Guide: "The default I2C address for the BNO08x is 0x4A"
 ******************************************************************************/
#define BNO085_DEFAULT_ADDR         0x4A    /* Default I2C address */
#define BNO085_ALT_ADDR             0x4B    /* Alternate address (DI high) */

/*******************************************************************************
 * BNO085 Timing Constants
 * Citation: BNO08x Datasheet: Reset and startup timing
 ******************************************************************************/
#define BNO085_RESET_DELAY_MS       100     /* Wait after reset */
#define BNO085_STARTUP_DELAY_MS     300     /* Wait for sensor startup */
#define BNO085_POLL_TIMEOUT_MS      500     /* Timeout waiting for data */

/*******************************************************************************
 * Error Codes
 ******************************************************************************/
#define BNO085_OK                   0       /* Success */
#define BNO085_ERR_I2C              -1      /* I2C communication error */
#define BNO085_ERR_TIMEOUT          -2      /* Operation timeout */
#define BNO085_ERR_NOT_FOUND        -3      /* Device not found */
#define BNO085_ERR_INVALID_DATA     -4      /* Invalid data received */
#define BNO085_ERR_NOT_READY        -5      /* Device not ready */
#define BNO085_ERR_BUFFER_OVERFLOW  -6      /* Buffer too small */
#define BNO085_ERR_INVALID_PARAM    -7      /* Invalid parameter */

/*******************************************************************************
 * Sensor Report Types
 * Citation: FIRMWARE_DESIGN.md Section "BNO085 Report Types Available"
 ******************************************************************************/
typedef enum {
    BNO085_REPORT_NONE              = 0x00,
    BNO085_REPORT_ACCELEROMETER     = 0x01, /* m/s² */
    BNO085_REPORT_GYROSCOPE         = 0x02, /* rad/s */
    BNO085_REPORT_MAGNETOMETER      = 0x03, /* µT */
    BNO085_REPORT_LINEAR_ACCEL      = 0x04, /* m/s² minus gravity */
    BNO085_REPORT_ROTATION_VECTOR   = 0x05, /* Quaternion */
    BNO085_REPORT_GRAVITY           = 0x06, /* m/s² */
    BNO085_REPORT_GAME_ROTATION     = 0x08, /* Quaternion (no mag) */
    BNO085_REPORT_GEOMAG_ROTATION   = 0x09, /* Quaternion (accel+mag) */
    BNO085_REPORT_STEP_COUNTER      = 0x11,
    BNO085_REPORT_STABILITY         = 0x13,
    BNO085_REPORT_ACTIVITY          = 0x1E,
    BNO085_REPORT_ARVR_STABILIZED   = 0x28, /* AR/VR optimized RV */
} bno085_report_type_t;

/*******************************************************************************
 * Sensor Status/Accuracy
 ******************************************************************************/
typedef enum {
    BNO085_ACCURACY_UNRELIABLE  = 0,
    BNO085_ACCURACY_LOW         = 1,
    BNO085_ACCURACY_MEDIUM      = 2,
    BNO085_ACCURACY_HIGH        = 3,
} bno085_accuracy_t;

/*******************************************************************************
 * Stability Classification
 * Citation: Adafruit BNO085 Guide: "Stability Classification"
 ******************************************************************************/
typedef enum {
    BNO085_STABILITY_UNKNOWN    = 0,
    BNO085_STABILITY_ON_TABLE   = 1,
    BNO085_STABILITY_STATIONARY = 2,
    BNO085_STABILITY_STABLE     = 3,
    BNO085_STABILITY_MOTION     = 4,
} bno085_stability_t;

/*******************************************************************************
 * Data Structures
 ******************************************************************************/

/**
 * @brief BNO085 device configuration
 */
typedef struct {
    uint8_t i2c_addr;           /* I2C address (0x4A or 0x4B) */
    int8_t  int_pin;            /* Interrupt pin (-1 if not used) */
    int8_t  rst_pin;            /* Reset pin (-1 if not used) */
} bno085_config_t;

/**
 * @brief BNO085 device handle
 */
typedef struct {
    uint8_t i2c_addr;           /* I2C device address */
    int8_t  int_pin;            /* Interrupt pin */
    int8_t  rst_pin;            /* Reset pin */
    bool    initialized;        /* Device initialized flag */
    uint8_t sequence[6];        /* SHTP sequence numbers per channel */
    
    /* Product information */
    uint8_t  sw_version_major;
    uint8_t  sw_version_minor;
    uint16_t sw_version_patch;
    uint32_t sw_part_number;
    uint32_t sw_build_number;
    
    /* Receive buffer */
    uint8_t  rx_buffer[512];
    uint16_t rx_len;
    
    /* Enabled reports bitmask */
    uint32_t enabled_reports;
} bno085_t;

/**
 * @brief Quaternion data (rotation vector)
 * Citation: Adafruit BNO085 Guide: "Four-point quaternion output"
 */
typedef struct {
    float i;                    /* i component */
    float j;                    /* j component */
    float k;                    /* k component */
    float real;                 /* Real (w) component */
    float accuracy_rad;         /* Estimated accuracy (radians) */
    uint8_t status;             /* Accuracy status */
} bno085_quaternion_t;

/**
 * @brief 3-axis vector data
 * Citation: Adafruit BNO085 Guide: "Three axes of acceleration/rotation"
 */
typedef struct {
    float x;                    /* X-axis */
    float y;                    /* Y-axis */
    float z;                    /* Z-axis */
    uint8_t accuracy;           /* Accuracy status (0-3) */
} bno085_vector_t;

/**
 * @brief Complete IMU data structure
 */
typedef struct {
    bno085_quaternion_t rotation_vector;    /* Fused orientation */
    bno085_quaternion_t game_rotation;      /* No magnetometer */
    bno085_vector_t     accelerometer;      /* m/s² */
    bno085_vector_t     gyroscope;          /* rad/s */
    bno085_vector_t     magnetometer;       /* µT */
    bno085_vector_t     linear_accel;       /* m/s² minus gravity */
    bno085_vector_t     gravity;            /* Gravity vector */
    uint32_t            step_count;         /* Step counter */
    bno085_stability_t  stability;          /* Stability classification */
    uint32_t            timestamp_us;       /* Sensor timestamp */
    uint8_t             report_id;          /* Most recent report ID */
} bno085_data_t;

/*******************************************************************************
 * Initialization Functions
 ******************************************************************************/

/**
 * @brief Initialize BNO085 driver with default configuration
 * @param dev Pointer to device handle
 * @return BNO085_OK on success, error code on failure
 * 
 * Citation: FIRMWARE_DESIGN.md "Initialization Sequence":
 *   1. Wait for RESET_COMPLETE advertisement (channel 0)
 *   2. Send Product ID Request
 *   3. Enable desired sensor reports
 */
int bno085_init(bno085_t *dev);

/**
 * @brief Initialize BNO085 with custom configuration
 * @param dev Pointer to device handle
 * @param config Configuration parameters
 * @return BNO085_OK on success, error code on failure
 */
int bno085_init_config(bno085_t *dev, const bno085_config_t *config);

/**
 * @brief Deinitialize BNO085 driver
 * @param dev Pointer to device handle
 */
void bno085_deinit(bno085_t *dev);

/**
 * @brief Software reset the BNO085
 * @param dev Pointer to device handle
 * @return BNO085_OK on success, error code on failure
 */
int bno085_reset(bno085_t *dev);

/**
 * @brief Check if BNO085 is present and responding
 * @param dev Pointer to device handle
 * @return true if device is present, false otherwise
 */
bool bno085_is_present(bno085_t *dev);

/*******************************************************************************
 * Configuration Functions
 ******************************************************************************/

/**
 * @brief Enable a sensor report at specified rate
 * @param dev Pointer to device handle
 * @param report_type Report type to enable
 * @param interval_us Report interval in microseconds
 * @return BNO085_OK on success, error code on failure
 * 
 * Citation: FIRMWARE_DESIGN.md:
 *   "Report Interval: 5000 µs (5 ms) = 200 Hz"
 */
int bno085_enable_report(bno085_t *dev, bno085_report_type_t report_type, 
                         uint32_t interval_us);

/**
 * @brief Disable a sensor report
 * @param dev Pointer to device handle
 * @param report_type Report type to disable
 * @return BNO085_OK on success, error code on failure
 */
int bno085_disable_report(bno085_t *dev, bno085_report_type_t report_type);

/**
 * @brief Enable rotation vector (quaternion) report
 * @param dev Pointer to device handle
 * @param interval_us Report interval in microseconds
 * @return BNO085_OK on success, error code on failure
 */
static inline int bno085_enable_rotation_vector(bno085_t *dev, uint32_t interval_us) {
    return bno085_enable_report(dev, BNO085_REPORT_ROTATION_VECTOR, interval_us);
}

/**
 * @brief Enable accelerometer report
 * @param dev Pointer to device handle
 * @param interval_us Report interval in microseconds
 * @return BNO085_OK on success, error code on failure
 */
static inline int bno085_enable_accelerometer(bno085_t *dev, uint32_t interval_us) {
    return bno085_enable_report(dev, BNO085_REPORT_ACCELEROMETER, interval_us);
}

/**
 * @brief Enable gyroscope report
 * @param dev Pointer to device handle
 * @param interval_us Report interval in microseconds
 * @return BNO085_OK on success, error code on failure
 */
static inline int bno085_enable_gyroscope(bno085_t *dev, uint32_t interval_us) {
    return bno085_enable_report(dev, BNO085_REPORT_GYROSCOPE, interval_us);
}

/**
 * @brief Enable game rotation vector (no magnetometer)
 * @param dev Pointer to device handle
 * @param interval_us Report interval in microseconds
 * @return BNO085_OK on success, error code on failure
 */
static inline int bno085_enable_game_rotation(bno085_t *dev, uint32_t interval_us) {
    return bno085_enable_report(dev, BNO085_REPORT_GAME_ROTATION, interval_us);
}

/*******************************************************************************
 * Data Reading Functions
 ******************************************************************************/

/**
 * @brief Check if data is available (interrupt pin or poll)
 * @param dev Pointer to device handle
 * @return true if data is available, false otherwise
 */
bool bno085_data_available(bno085_t *dev);

/**
 * @brief Poll for and process a single sensor report
 * @param dev Pointer to device handle
 * @param data Output data structure (can be NULL to just poll)
 * @return Report ID received, or negative error code
 */
int bno085_poll(bno085_t *dev, bno085_data_t *data);

/**
 * @brief Get the latest rotation vector (quaternion)
 * @param dev Pointer to device handle
 * @param quat Output quaternion structure
 * @return BNO085_OK on success, error code on failure
 */
int bno085_get_rotation_vector(bno085_t *dev, bno085_quaternion_t *quat);

/**
 * @brief Get the latest accelerometer data
 * @param dev Pointer to device handle
 * @param accel Output vector structure
 * @return BNO085_OK on success, error code on failure
 */
int bno085_get_accelerometer(bno085_t *dev, bno085_vector_t *accel);

/**
 * @brief Get the latest gyroscope data
 * @param dev Pointer to device handle
 * @param gyro Output vector structure
 * @return BNO085_OK on success, error code on failure
 */
int bno085_get_gyroscope(bno085_t *dev, bno085_vector_t *gyro);

/**
 * @brief Get all available sensor data
 * @param dev Pointer to device handle
 * @param data Output data structure
 * @return BNO085_OK on success, error code on failure
 */
int bno085_get_all_data(bno085_t *dev, bno085_data_t *data);

/*******************************************************************************
 * Utility Functions
 ******************************************************************************/

/**
 * @brief Get product ID and version information
 * @param dev Pointer to device handle
 * @param product_id Output product ID structure
 * @return BNO085_OK on success, error code on failure
 */
int bno085_get_product_id(bno085_t *dev, shtp_product_id_t *product_id);

/**
 * @brief Convert quaternion to Euler angles (roll, pitch, yaw)
 * @param quat Input quaternion
 * @param roll Output roll angle (radians)
 * @param pitch Output pitch angle (radians)
 * @param yaw Output yaw angle (radians)
 */
void bno085_quat_to_euler(const bno085_quaternion_t *quat, 
                          float *roll, float *pitch, float *yaw);

/**
 * @brief Get string name for report type
 * @param report_type Report type enum
 * @return String name of report type
 */
const char* bno085_report_name(bno085_report_type_t report_type);

/**
 * @brief Get string name for error code
 * @param error Error code
 * @return String description of error
 */
const char* bno085_error_name(int error);

#ifdef __cplusplus
}
#endif

#endif /* BNO085_H */
