/**
 * @file shtp.h
 * @brief SHTP (Sensor Hub Transport Protocol) definitions for BNO085
 * 
 * Protocol definitions for communicating with the BNO085 IMU sensor
 * using Hillcrest's SH-2 (Sensor Hub 2) firmware over I2C.
 * 
 * Citations:
 * - Adafruit BNO085 Guide: Channel structure and protocol overview
 * - FIRMWARE_DESIGN.md: SHTP protocol details
 * - BNO08x Datasheet: SH-2 protocol reference
 */

#ifndef SHTP_H
#define SHTP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * SHTP Channel Definitions
 * Citation: FIRMWARE_DESIGN.md Section "BNO085 SH-2 Protocol Overview":
 *   - Channel 0: Command
 *   - Channel 1: Executable
 *   - Channel 2: Control (sensor config)
 *   - Channel 3: Input Sensor Reports
 *   - Channel 4: Wake Input Sensor Reports
 *   - Channel 5: Gyro Rotation Vector
 ******************************************************************************/
#define SHTP_CHANNEL_COMMAND        0   /* Command channel */
#define SHTP_CHANNEL_EXECUTABLE     1   /* Executable channel */
#define SHTP_CHANNEL_CONTROL        2   /* Sensor configuration */
#define SHTP_CHANNEL_REPORTS        3   /* Input sensor reports */
#define SHTP_CHANNEL_WAKE_REPORTS   4   /* Wake input sensor reports */
#define SHTP_CHANNEL_GYRO_RV        5   /* Gyro rotation vector */

/*******************************************************************************
 * SHTP Header Structure
 * The SHTP header is 4 bytes:
 *   Byte 0-1: Packet length (little-endian, includes header)
 *   Byte 2:   Channel number
 *   Byte 3:   Sequence number
 ******************************************************************************/
#define SHTP_HEADER_SIZE            4

/* Header byte positions */
#define SHTP_HEADER_LEN_LSB         0
#define SHTP_HEADER_LEN_MSB         1
#define SHTP_HEADER_CHANNEL         2
#define SHTP_HEADER_SEQ             3

/* Bit 15 of length field indicates continuation */
#define SHTP_CONTINUATION_FLAG      0x8000

/*******************************************************************************
 * SH-2 Report IDs
 * Citation: FIRMWARE_DESIGN.md Section "BNO085 Report Types Available":
 *   - 0x01: Accelerometer (m/s²)
 *   - 0x02: Gyroscope (rad/s)
 *   - 0x03: Magnetometer (µT)
 *   - 0x05: Rotation Vector (Quaternion)
 *   - 0x08: Game Rotation Vector
 ******************************************************************************/
/* Sensor Reports */
#define SH2_ACCELEROMETER           0x01    /* Calibrated accelerometer */
#define SH2_GYROSCOPE               0x02    /* Calibrated gyroscope */
#define SH2_MAGNETOMETER            0x03    /* Calibrated magnetometer */
#define SH2_LINEAR_ACCELERATION     0x04    /* Linear acceleration */
#define SH2_ROTATION_VECTOR         0x05    /* Rotation vector (quaternion) */
#define SH2_GRAVITY                 0x06    /* Gravity vector */
#define SH2_GYROSCOPE_UNCALIBRATED  0x07    /* Uncalibrated gyroscope */
#define SH2_GAME_ROTATION_VECTOR    0x08    /* Game rotation vector */
#define SH2_GEOMAGNETIC_ROTATION    0x09    /* Geomagnetic rotation vector */
#define SH2_PRESSURE                0x0A    /* Pressure sensor */
#define SH2_AMBIENT_LIGHT           0x0B    /* Ambient light sensor */
#define SH2_HUMIDITY                0x0C    /* Humidity sensor */
#define SH2_PROXIMITY               0x0D    /* Proximity sensor */
#define SH2_TEMPERATURE             0x0E    /* Temperature sensor */
#define SH2_MAG_UNCALIBRATED        0x0F    /* Uncalibrated magnetometer */
#define SH2_TAP_DETECTOR            0x10    /* Tap detector */
#define SH2_STEP_COUNTER            0x11    /* Step counter */
#define SH2_SIGNIFICANT_MOTION      0x12    /* Significant motion */
#define SH2_STABILITY_CLASSIFIER    0x13    /* Stability classifier */
#define SH2_RAW_ACCELEROMETER       0x14    /* Raw accelerometer */
#define SH2_RAW_GYROSCOPE           0x15    /* Raw gyroscope */
#define SH2_RAW_MAGNETOMETER        0x16    /* Raw magnetometer */
#define SH2_STEP_DETECTOR           0x18    /* Step detector */
#define SH2_SHAKE_DETECTOR          0x19    /* Shake detector */
#define SH2_FLIP_DETECTOR           0x1A    /* Flip detector */
#define SH2_PICKUP_DETECTOR         0x1B    /* Pickup detector */
#define SH2_STABILITY_DETECTOR      0x1C    /* Stability detector */
#define SH2_PERSONAL_ACTIVITY       0x1E    /* Personal activity classifier */
#define SH2_SLEEP_DETECTOR          0x1F    /* Sleep detector */
#define SH2_TILT_DETECTOR           0x20    /* Tilt detector */
#define SH2_POCKET_DETECTOR         0x21    /* Pocket detector */
#define SH2_CIRCLE_DETECTOR         0x22    /* Circle detector */
#define SH2_HEART_RATE              0x23    /* Heart rate monitor */
#define SH2_ARVR_STABILIZED_RV      0x28    /* AR/VR stabilized rotation vector */
#define SH2_ARVR_STABILIZED_GRV     0x29    /* AR/VR stabilized game RV */
#define SH2_GYRO_INTEGRATED_RV      0x2A    /* Gyro-integrated rotation vector */
#define SH2_IZRO_MOTION_REQUEST     0x2B    /* IZRO motion request */

/*******************************************************************************
 * SH-2 Command IDs
 * Citation: FIRMWARE_DESIGN.md: "SET_FEATURE_COMMAND = 0xFD"
 ******************************************************************************/
/* Control commands (Channel 2) */
#define SH2_CMD_SET_FEATURE         0xFD    /* Enable sensor report */
#define SH2_CMD_GET_FEATURE_REQ     0xFE    /* Get feature request */
#define SH2_CMD_GET_FEATURE_RESP    0xFC    /* Get feature response */
#define SH2_CMD_PRODUCT_ID_REQ      0xF9    /* Product ID request */
#define SH2_CMD_PRODUCT_ID_RESP     0xF8    /* Product ID response */
#define SH2_CMD_FRS_WRITE_REQ       0xF7    /* FRS write request */
#define SH2_CMD_FRS_WRITE_DATA      0xF6    /* FRS write data */
#define SH2_CMD_FRS_WRITE_RESP      0xF5    /* FRS write response */
#define SH2_CMD_FRS_READ_REQ        0xF4    /* FRS read request */
#define SH2_CMD_FRS_READ_RESP       0xF3    /* FRS read response */
#define SH2_CMD_COMMAND_REQ         0xF2    /* Command request */
#define SH2_CMD_COMMAND_RESP        0xF1    /* Command response */

/* Executable commands (Channel 1) */
#define SH2_EXEC_RESET              0x01    /* Reset command */
#define SH2_EXEC_ON                 0x02    /* On command */
#define SH2_EXEC_SLEEP              0x03    /* Sleep command */

/* Advertisement messages (Channel 0) */
#define SH2_SHTP_ADVERTISEMENT      0x00    /* SHTP advertisement */
#define SH2_RESET_COMPLETE          0x01    /* Reset complete */

/*******************************************************************************
 * SET_FEATURE_COMMAND Structure (SH2_CMD_SET_FEATURE = 0xFD)
 * Citation: FIRMWARE_DESIGN.md Section "Report Enable Command":
 *   Byte 0:     Report ID (0xFD)
 *   Byte 1:     Feature Report ID (e.g., 0x05 for Rotation Vector)
 *   Bytes 2-3:  Reserved
 *   Bytes 4-7:  Report Interval (microseconds, little-endian)
 *   Bytes 8-11: Batch Interval
 *   Bytes 12-15: Sensor-specific config
 ******************************************************************************/
#define SET_FEATURE_CMD_SIZE        17

/* SET_FEATURE_COMMAND byte offsets */
#define SET_FEATURE_REPORT_ID       0   /* Command ID (0xFD) */
#define SET_FEATURE_SENSOR_ID       1   /* Sensor to configure */
#define SET_FEATURE_FLAGS           2   /* Feature flags */
#define SET_FEATURE_CHANGE_SENS     3   /* Change sensitivity */
#define SET_FEATURE_INTERVAL_LSB    4   /* Report interval [31:0] */
#define SET_FEATURE_BATCH_LSB       8   /* Batch interval [31:0] */
#define SET_FEATURE_SPECIFIC        12  /* Sensor-specific config [31:0] */

/*******************************************************************************
 * SHTP Data Structures
 ******************************************************************************/

/**
 * @brief SHTP packet header
 */
typedef struct {
    uint16_t length;        /* Packet length including header */
    uint8_t  channel;       /* Channel number */
    uint8_t  sequence;      /* Sequence number */
} shtp_header_t;

/**
 * @brief SHTP packet structure
 */
typedef struct {
    shtp_header_t header;
    uint8_t      *data;     /* Pointer to payload data */
    uint16_t      data_len; /* Payload length (length - 4) */
} shtp_packet_t;

/**
 * @brief Sensor report configuration
 */
typedef struct {
    uint8_t  sensor_id;             /* Sensor report ID */
    uint32_t report_interval_us;    /* Report interval in microseconds */
    uint32_t batch_interval_us;     /* Batch interval in microseconds */
    uint32_t sensor_specific;       /* Sensor-specific configuration */
} shtp_sensor_config_t;

/**
 * @brief Product ID response structure
 */
typedef struct {
    uint8_t  reset_cause;       /* Reset cause */
    uint8_t  sw_version_major;  /* Software version major */
    uint8_t  sw_version_minor;  /* Software version minor */
    uint32_t sw_part_number;    /* Software part number */
    uint32_t sw_build_number;   /* Software build number */
    uint16_t sw_version_patch;  /* Software version patch */
} shtp_product_id_t;

/**
 * @brief Rotation vector data (quaternion)
 * Citation: Adafruit BNO085 Guide: "Four-point quaternion output"
 */
typedef struct {
    float i;        /* Quaternion i component */
    float j;        /* Quaternion j component */
    float k;        /* Quaternion k component */
    float real;     /* Quaternion real component */
    float accuracy; /* Estimated accuracy (radians) */
} shtp_rotation_vector_t;

/**
 * @brief 3-axis sensor data structure
 * Citation: Adafruit BNO085 Guide: "Three axes of acceleration/rotation"
 */
typedef struct {
    float x;        /* X-axis value */
    float y;        /* Y-axis value */
    float z;        /* Z-axis value */
    uint8_t status; /* Sensor status/accuracy */
} shtp_vector3_t;

/*******************************************************************************
 * SHTP Helper Macros
 ******************************************************************************/

/**
 * @brief Build 32-bit little-endian value from bytes
 */
#define SHTP_LE32(b0, b1, b2, b3) \
    ((uint32_t)(b0) | ((uint32_t)(b1) << 8) | \
     ((uint32_t)(b2) << 16) | ((uint32_t)(b3) << 24))

/**
 * @brief Build 16-bit little-endian value from bytes
 */
#define SHTP_LE16(b0, b1) \
    ((uint16_t)(b0) | ((uint16_t)(b1) << 8))

/**
 * @brief Extract byte from 32-bit value
 */
#define SHTP_BYTE(val, n) ((uint8_t)(((val) >> ((n) * 8)) & 0xFF))

/**
 * @brief Q-point conversion for sensor data
 * BNO085 reports use fixed-point Q notation
 */
#define SHTP_Q_TO_FLOAT(val, q) ((float)(val) / (float)(1 << (q)))

/* Q-point values for different sensor types */
#define SHTP_Q_ROTATION_VECTOR  14  /* Quaternion Q14 */
#define SHTP_Q_ACCELEROMETER    8   /* m/s² Q8 */
#define SHTP_Q_GYROSCOPE        9   /* rad/s Q9 */
#define SHTP_Q_MAGNETOMETER     4   /* µT Q4 */
#define SHTP_Q_ACCURACY         12  /* Accuracy Q12 */

/*******************************************************************************
 * SHTP Function Prototypes
 ******************************************************************************/

/**
 * @brief Initialize SHTP protocol layer
 * @return 0 on success, negative error code on failure
 */
int shtp_init(void);

/**
 * @brief Reset SHTP protocol state
 */
void shtp_reset(void);

/**
 * @brief Build SHTP packet header
 * @param header Pointer to header structure
 * @param channel Channel number
 * @param length Total packet length
 */
void shtp_build_header(shtp_header_t *header, uint8_t channel, uint16_t length);

/**
 * @brief Parse SHTP packet header from raw bytes
 * @param raw Raw byte buffer (at least 4 bytes)
 * @param header Output header structure
 * @return true if valid header, false otherwise
 */
bool shtp_parse_header(const uint8_t *raw, shtp_header_t *header);

/**
 * @brief Build SET_FEATURE_COMMAND for sensor configuration
 * @param buffer Output buffer (at least SET_FEATURE_CMD_SIZE bytes)
 * @param config Sensor configuration
 * @return Number of bytes written
 */
int shtp_build_set_feature(uint8_t *buffer, const shtp_sensor_config_t *config);

/**
 * @brief Build product ID request
 * @param buffer Output buffer
 * @return Number of bytes written
 */
int shtp_build_product_id_request(uint8_t *buffer);

/**
 * @brief Parse product ID response
 * @param data Response data
 * @param len Data length
 * @param product_id Output product ID structure
 * @return 0 on success, negative error code on failure
 */
int shtp_parse_product_id(const uint8_t *data, uint16_t len, shtp_product_id_t *product_id);

/**
 * @brief Parse rotation vector report
 * @param data Report data
 * @param len Data length
 * @param rv Output rotation vector structure
 * @return 0 on success, negative error code on failure
 */
int shtp_parse_rotation_vector(const uint8_t *data, uint16_t len, shtp_rotation_vector_t *rv);

/**
 * @brief Parse accelerometer report
 * @param data Report data
 * @param len Data length
 * @param accel Output vector structure
 * @return 0 on success, negative error code on failure
 */
int shtp_parse_accelerometer(const uint8_t *data, uint16_t len, shtp_vector3_t *accel);

/**
 * @brief Parse gyroscope report
 * @param data Report data
 * @param len Data length
 * @param gyro Output vector structure
 * @return 0 on success, negative error code on failure
 */
int shtp_parse_gyroscope(const uint8_t *data, uint16_t len, shtp_vector3_t *gyro);

#ifdef __cplusplus
}
#endif

#endif /* SHTP_H */
