/**
 * @file bno085.c
 * @brief BNO085 IMU sensor driver implementation
 * 
 * Driver implementation for the Hillcrest BNO085 9-DOF IMU sensor.
 * Communicates via I2C using the SHTP (Sensor Hub Transport Protocol).
 * 
 * Citations:
 * - Adafruit BNO085 Guide: Sensor specifications and I2C interface
 * - FIRMWARE_DESIGN.md: Hardware overview and initialization sequence
 * - BNO08x Datasheet: SH-2 protocol and register definitions
 */

#include "bno085.h"
#include "twim.h"
#include "config.h"
#include "board.h"
#include <string.h>
#include <math.h>

/*******************************************************************************
 * Private Definitions
 ******************************************************************************/

/* External TWIM instance (initialized in board.c) */
extern twim_t g_twim;

/* Static data storage */
static bno085_data_t s_sensor_data;

/*******************************************************************************
 * Private Functions - SHTP Communication
 ******************************************************************************/

/**
 * @brief Send SHTP packet to BNO085
 * @param dev Device handle
 * @param channel SHTP channel
 * @param data Payload data
 * @param len Payload length
 * @return BNO085_OK on success, error code on failure
 */
static int bno085_send_packet(bno085_t *dev, uint8_t channel, 
                              const uint8_t *data, uint16_t len)
{
    uint8_t tx_buffer[256];
    uint16_t packet_len = len + SHTP_HEADER_SIZE;
    int result;
    
    if (packet_len > sizeof(tx_buffer)) {
        return BNO085_ERR_BUFFER_OVERFLOW;
    }
    
    /* Build SHTP header (4 bytes)
     * Citation: FIRMWARE_DESIGN.md:
     *   "Byte 0-1: Packet length (little-endian, includes header)"
     *   "Byte 2: Channel number"
     *   "Byte 3: Sequence number"
     */
    tx_buffer[0] = packet_len & 0xFF;           /* Length LSB */
    tx_buffer[1] = (packet_len >> 8) & 0x7F;    /* Length MSB (bit 15 clear) */
    tx_buffer[2] = channel;                      /* Channel */
    tx_buffer[3] = dev->sequence[channel]++;     /* Sequence (auto-increment) */
    
    /* Copy payload */
    if (len > 0 && data != NULL) {
        memcpy(&tx_buffer[4], data, len);
    }
    
    /* Send via I2C */
    result = twim_write(&g_twim, dev->i2c_addr, tx_buffer, packet_len, true);
    
    if (result < 0) {
        return BNO085_ERR_I2C;
    }
    
    return BNO085_OK;
}

/**
 * @brief Receive SHTP packet from BNO085
 * @param dev Device handle
 * @param timeout_ms Timeout in milliseconds
 * @return Packet length on success, negative error code on failure
 */
static int bno085_receive_packet(bno085_t *dev, uint32_t timeout_ms)
{
    uint8_t header[4];
    uint16_t packet_len;
    uint16_t payload_len;
    int result;
    
    /* Read header first (4 bytes) */
    result = twim_read(&g_twim, dev->i2c_addr, header, 4);
    
    if (result < 0) {
        return BNO085_ERR_I2C;
    }
    
    /* Parse packet length
     * Citation: SHTP protocol - length is 15 bits, bit 15 is continuation flag
     */
    packet_len = header[0] | ((header[1] & 0x7F) << 8);
    
    /* Check for empty packet or invalid length */
    if (packet_len == 0 || packet_len == 0x7FFF) {
        return 0;  /* No data available */
    }
    
    if (packet_len < SHTP_HEADER_SIZE) {
        return BNO085_ERR_INVALID_DATA;
    }
    
    if (packet_len > sizeof(dev->rx_buffer)) {
        return BNO085_ERR_BUFFER_OVERFLOW;
    }
    
    /* Copy header to buffer */
    memcpy(dev->rx_buffer, header, 4);
    
    /* Read remaining data if packet is larger than header */
    payload_len = packet_len - SHTP_HEADER_SIZE;
    if (payload_len > 0) {
        result = twim_read(&g_twim, dev->i2c_addr, 
                          &dev->rx_buffer[4], payload_len);
        
        if (result < 0) {
            return BNO085_ERR_I2C;
        }
    }
    
    dev->rx_len = packet_len;
    
    return packet_len;
}

/**
 * @brief Wait for specific advertisement message
 * @param dev Device handle
 * @param advertisement Expected advertisement type
 * @param timeout_ms Timeout in milliseconds
 * @return BNO085_OK if found, error code otherwise
 */
static int bno085_wait_for_advertisement(bno085_t *dev, uint8_t advertisement,
                                         uint32_t timeout_ms)
{
    int result;
    uint32_t elapsed = 0;
    
    while (elapsed < timeout_ms) {
        result = bno085_receive_packet(dev, 100);
        
        if (result > SHTP_HEADER_SIZE) {
            uint8_t channel = dev->rx_buffer[2];
            uint8_t report_id = dev->rx_buffer[4];
            
            /* Check if this is the expected advertisement on channel 0 */
            if (channel == SHTP_CHANNEL_COMMAND && report_id == advertisement) {
                return BNO085_OK;
            }
        }
        
        board_delay_ms(10);
        elapsed += 10;
    }
    
    return BNO085_ERR_TIMEOUT;
}

/**
 * @brief Parse sensor report based on report ID
 * @param dev Device handle
 * @param data Complete sensor data structure to update
 * @return Report ID on success, negative error code on failure
 */
static int bno085_parse_sensor_report(bno085_t *dev, bno085_data_t *data)
{
    uint8_t channel = dev->rx_buffer[2];
    uint8_t *payload = &dev->rx_buffer[SHTP_HEADER_SIZE];
    uint16_t payload_len = dev->rx_len - SHTP_HEADER_SIZE;
    
    /* Only process reports on channel 3 (Input Sensor Reports) */
    if (channel != SHTP_CHANNEL_REPORTS) {
        return 0;
    }
    
    if (payload_len < 5) {
        return BNO085_ERR_INVALID_DATA;
    }
    
    /* First byte of sensor report is the report ID */
    uint8_t report_id = payload[0];
    
    /* Parse based on report type
     * Citation: FIRMWARE_DESIGN.md Section "BNO085 Report Types Available"
     * Citation: Adafruit BNO085 Guide - report data formats
     */
    switch (report_id) {
        case SH2_ROTATION_VECTOR:
        case SH2_GAME_ROTATION_VECTOR:
        case SH2_GEOMAGNETIC_ROTATION: {
            /* Quaternion data format (after common 5-byte header):
             *   Bytes 0-1: i (Q14)
             *   Bytes 2-3: j (Q14)
             *   Bytes 4-5: k (Q14)
             *   Bytes 6-7: real (Q14)
             *   Bytes 8-9: accuracy (Q12) - rotation vector only
             */
            if (payload_len < 14) {
                return BNO085_ERR_INVALID_DATA;
            }
            
            int16_t i = (int16_t)(payload[5] | (payload[6] << 8));
            int16_t j = (int16_t)(payload[7] | (payload[8] << 8));
            int16_t k = (int16_t)(payload[9] | (payload[10] << 8));
            int16_t real = (int16_t)(payload[11] | (payload[12] << 8));
            
            bno085_quaternion_t *quat;
            if (report_id == SH2_GAME_ROTATION_VECTOR) {
                quat = &data->game_rotation;
            } else {
                quat = &data->rotation_vector;
            }
            
            /* Convert from Q14 fixed-point to float
             * Citation: SHTP_Q_ROTATION_VECTOR = 14
             */
            quat->i = SHTP_Q_TO_FLOAT(i, SHTP_Q_ROTATION_VECTOR);
            quat->j = SHTP_Q_TO_FLOAT(j, SHTP_Q_ROTATION_VECTOR);
            quat->k = SHTP_Q_TO_FLOAT(k, SHTP_Q_ROTATION_VECTOR);
            quat->real = SHTP_Q_TO_FLOAT(real, SHTP_Q_ROTATION_VECTOR);
            quat->status = payload[2] & 0x03;  /* Status/accuracy in byte 2 */
            
            /* Parse accuracy for rotation vector (not game rotation) */
            if (report_id == SH2_ROTATION_VECTOR && payload_len >= 16) {
                int16_t acc = (int16_t)(payload[13] | (payload[14] << 8));
                quat->accuracy_rad = SHTP_Q_TO_FLOAT(acc, SHTP_Q_ACCURACY);
            } else {
                quat->accuracy_rad = 0.0f;
            }
            break;
        }
        
        case SH2_ACCELEROMETER:
        case SH2_LINEAR_ACCELERATION:
        case SH2_GRAVITY: {
            /* 3-axis vector format (after common 5-byte header):
             *   Bytes 0-1: X (Q8)
             *   Bytes 2-3: Y (Q8)
             *   Bytes 4-5: Z (Q8)
             */
            if (payload_len < 10) {
                return BNO085_ERR_INVALID_DATA;
            }
            
            int16_t x = (int16_t)(payload[5] | (payload[6] << 8));
            int16_t y = (int16_t)(payload[7] | (payload[8] << 8));
            int16_t z = (int16_t)(payload[9] | (payload[10] << 8));
            
            bno085_vector_t *vec;
            if (report_id == SH2_ACCELEROMETER) {
                vec = &data->accelerometer;
            } else if (report_id == SH2_LINEAR_ACCELERATION) {
                vec = &data->linear_accel;
            } else {
                vec = &data->gravity;
            }
            
            /* Convert from Q8 fixed-point to float (m/s²)
             * Citation: SHTP_Q_ACCELEROMETER = 8
             */
            vec->x = SHTP_Q_TO_FLOAT(x, SHTP_Q_ACCELEROMETER);
            vec->y = SHTP_Q_TO_FLOAT(y, SHTP_Q_ACCELEROMETER);
            vec->z = SHTP_Q_TO_FLOAT(z, SHTP_Q_ACCELEROMETER);
            vec->accuracy = payload[2] & 0x03;
            break;
        }
        
        case SH2_GYROSCOPE: {
            /* Gyroscope format (after common 5-byte header):
             *   Bytes 0-1: X (Q9)
             *   Bytes 2-3: Y (Q9)
             *   Bytes 4-5: Z (Q9)
             */
            if (payload_len < 10) {
                return BNO085_ERR_INVALID_DATA;
            }
            
            int16_t x = (int16_t)(payload[5] | (payload[6] << 8));
            int16_t y = (int16_t)(payload[7] | (payload[8] << 8));
            int16_t z = (int16_t)(payload[9] | (payload[10] << 8));
            
            /* Convert from Q9 fixed-point to float (rad/s)
             * Citation: SHTP_Q_GYROSCOPE = 9
             */
            data->gyroscope.x = SHTP_Q_TO_FLOAT(x, SHTP_Q_GYROSCOPE);
            data->gyroscope.y = SHTP_Q_TO_FLOAT(y, SHTP_Q_GYROSCOPE);
            data->gyroscope.z = SHTP_Q_TO_FLOAT(z, SHTP_Q_GYROSCOPE);
            data->gyroscope.accuracy = payload[2] & 0x03;
            break;
        }
        
        case SH2_MAGNETOMETER: {
            /* Magnetometer format (after common 5-byte header):
             *   Bytes 0-1: X (Q4)
             *   Bytes 2-3: Y (Q4)
             *   Bytes 4-5: Z (Q4)
             */
            if (payload_len < 10) {
                return BNO085_ERR_INVALID_DATA;
            }
            
            int16_t x = (int16_t)(payload[5] | (payload[6] << 8));
            int16_t y = (int16_t)(payload[7] | (payload[8] << 8));
            int16_t z = (int16_t)(payload[9] | (payload[10] << 8));
            
            /* Convert from Q4 fixed-point to float (µT)
             * Citation: SHTP_Q_MAGNETOMETER = 4
             */
            data->magnetometer.x = SHTP_Q_TO_FLOAT(x, SHTP_Q_MAGNETOMETER);
            data->magnetometer.y = SHTP_Q_TO_FLOAT(y, SHTP_Q_MAGNETOMETER);
            data->magnetometer.z = SHTP_Q_TO_FLOAT(z, SHTP_Q_MAGNETOMETER);
            data->magnetometer.accuracy = payload[2] & 0x03;
            break;
        }
        
        case SH2_STEP_COUNTER: {
            /* Step counter format (after common 5-byte header):
             *   Bytes 0-1: Step count (low 16 bits)
             *   Bytes 2-3: Step count (high 16 bits)
             */
            if (payload_len < 9) {
                return BNO085_ERR_INVALID_DATA;
            }
            
            data->step_count = payload[5] | (payload[6] << 8) |
                              (payload[7] << 16) | (payload[8] << 24);
            break;
        }
        
        case SH2_STABILITY_CLASSIFIER: {
            /* Stability classification (after common 5-byte header):
             *   Byte 0: Classification (0-4)
             */
            if (payload_len < 6) {
                return BNO085_ERR_INVALID_DATA;
            }
            
            data->stability = (bno085_stability_t)payload[5];
            break;
        }
        
        default:
            /* Unknown report type - ignore */
            return 0;
    }
    
    data->report_id = report_id;
    return report_id;
}

/*******************************************************************************
 * Public Functions - Initialization
 ******************************************************************************/

int bno085_init(bno085_t *dev)
{
    bno085_config_t config = {
        .i2c_addr = BNO085_DEFAULT_ADDR,
        .int_pin = -1,
        .rst_pin = -1
    };
    
    return bno085_init_config(dev, &config);
}

int bno085_init_config(bno085_t *dev, const bno085_config_t *config)
{
    int result;
    
    if (dev == NULL || config == NULL) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    /* Initialize device handle */
    memset(dev, 0, sizeof(bno085_t));
    dev->i2c_addr = config->i2c_addr;
    dev->int_pin = config->int_pin;
    dev->rst_pin = config->rst_pin;
    
    /* Check if device is present
     * Citation: Adafruit BNO085 Guide: "The default I2C address for the BNO08x is 0x4A"
     */
    if (!twim_device_present(&g_twim, dev->i2c_addr)) {
        return BNO085_ERR_NOT_FOUND;
    }
    
    /* Wait for sensor startup
     * Citation: FIRMWARE_DESIGN.md "Initialization Sequence":
     *   "1. Wait for RESET_COMPLETE advertisement (channel 0)"
     */
    board_delay_ms(BNO085_STARTUP_DELAY_MS);
    
    /* Wait for reset complete advertisement */
    result = bno085_wait_for_advertisement(dev, SH2_RESET_COMPLETE, 500);
    if (result != BNO085_OK) {
        /* If we timeout waiting for reset complete, sensor might already be running.
         * Try to soft reset it. */
        bno085_reset(dev);
        board_delay_ms(BNO085_RESET_DELAY_MS);
        
        result = bno085_wait_for_advertisement(dev, SH2_RESET_COMPLETE, 500);
        if (result != BNO085_OK) {
            return result;
        }
    }
    
    /* Request product ID
     * Citation: FIRMWARE_DESIGN.md "Initialization Sequence":
     *   "2. Send Product ID Request"
     */
    shtp_product_id_t product_id;
    result = bno085_get_product_id(dev, &product_id);
    if (result != BNO085_OK) {
        return result;
    }
    
    /* Store version info */
    dev->sw_version_major = product_id.sw_version_major;
    dev->sw_version_minor = product_id.sw_version_minor;
    dev->sw_version_patch = product_id.sw_version_patch;
    dev->sw_part_number = product_id.sw_part_number;
    dev->sw_build_number = product_id.sw_build_number;
    
    dev->initialized = true;
    
    return BNO085_OK;
}

void bno085_deinit(bno085_t *dev)
{
    if (dev != NULL) {
        memset(dev, 0, sizeof(bno085_t));
    }
}

int bno085_reset(bno085_t *dev)
{
    uint8_t reset_cmd[1] = {SH2_EXEC_RESET};
    
    if (dev == NULL) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    /* Send reset command on executable channel */
    return bno085_send_packet(dev, SHTP_CHANNEL_EXECUTABLE, reset_cmd, 1);
}

bool bno085_is_present(bno085_t *dev)
{
    if (dev == NULL) {
        return false;
    }
    
    return twim_device_present(&g_twim, dev->i2c_addr);
}

/*******************************************************************************
 * Public Functions - Configuration
 ******************************************************************************/

int bno085_enable_report(bno085_t *dev, bno085_report_type_t report_type, 
                         uint32_t interval_us)
{
    uint8_t cmd[17];
    
    if (dev == NULL || !dev->initialized) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    /* Build SET_FEATURE_COMMAND
     * Citation: FIRMWARE_DESIGN.md "Report Enable Command (SET_FEATURE_COMMAND = 0xFD)":
     *   Byte 0: Report ID (0xFD)
     *   Byte 1: Feature Report ID
     *   Bytes 2-3: Reserved
     *   Bytes 4-7: Report Interval (microseconds, little-endian)
     *   Bytes 8-11: Batch Interval
     *   Bytes 12-15: Sensor-specific config
     */
    memset(cmd, 0, sizeof(cmd));
    cmd[0] = SH2_CMD_SET_FEATURE;           /* 0xFD */
    cmd[1] = (uint8_t)report_type;          /* Sensor ID */
    cmd[2] = 0;                             /* Flags */
    cmd[3] = 0;                             /* Change sensitivity */
    cmd[4] = (interval_us >> 0) & 0xFF;     /* Report interval LSB */
    cmd[5] = (interval_us >> 8) & 0xFF;
    cmd[6] = (interval_us >> 16) & 0xFF;
    cmd[7] = (interval_us >> 24) & 0xFF;    /* Report interval MSB */
    /* Bytes 8-15: Batch interval and sensor-specific (leave as 0) */
    
    /* Send on control channel */
    int result = bno085_send_packet(dev, SHTP_CHANNEL_CONTROL, cmd, sizeof(cmd));
    
    if (result == BNO085_OK) {
        dev->enabled_reports |= (1UL << report_type);
    }
    
    return result;
}

int bno085_disable_report(bno085_t *dev, bno085_report_type_t report_type)
{
    int result = bno085_enable_report(dev, report_type, 0);
    
    if (result == BNO085_OK) {
        dev->enabled_reports &= ~(1UL << report_type);
    }
    
    return result;
}

/*******************************************************************************
 * Public Functions - Data Reading
 ******************************************************************************/

bool bno085_data_available(bno085_t *dev)
{
    if (dev == NULL || !dev->initialized) {
        return false;
    }
    
    /* If INT pin is configured, check it */
    if (dev->int_pin >= 0) {
        /* INT is active low */
        return board_gpio_read(0, dev->int_pin) == 0;
    }
    
    /* Otherwise, always return true (need to poll) */
    return true;
}

int bno085_poll(bno085_t *dev, bno085_data_t *data)
{
    int result;
    
    if (dev == NULL || !dev->initialized) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    /* Try to receive a packet */
    result = bno085_receive_packet(dev, 0);
    
    if (result <= 0) {
        return result;  /* No data or error */
    }
    
    /* Parse the packet */
    if (data != NULL) {
        return bno085_parse_sensor_report(dev, data);
    } else {
        return bno085_parse_sensor_report(dev, &s_sensor_data);
    }
}

int bno085_get_rotation_vector(bno085_t *dev, bno085_quaternion_t *quat)
{
    bno085_data_t data;
    int result;
    int report;
    int attempts = 10;
    
    if (dev == NULL || quat == NULL) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    /* Poll until we get a rotation vector report */
    while (attempts-- > 0) {
        result = bno085_poll(dev, &data);
        
        if (result < 0) {
            return result;
        }
        
        report = result;
        if (report == SH2_ROTATION_VECTOR || report == SH2_GAME_ROTATION_VECTOR) {
            if (report == SH2_GAME_ROTATION_VECTOR) {
                *quat = data.game_rotation;
            } else {
                *quat = data.rotation_vector;
            }
            return BNO085_OK;
        }
        
        board_delay_ms(1);
    }
    
    return BNO085_ERR_TIMEOUT;
}

int bno085_get_accelerometer(bno085_t *dev, bno085_vector_t *accel)
{
    bno085_data_t data;
    int result;
    int attempts = 10;
    
    if (dev == NULL || accel == NULL) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    while (attempts-- > 0) {
        result = bno085_poll(dev, &data);
        
        if (result < 0) {
            return result;
        }
        
        if (result == SH2_ACCELEROMETER) {
            *accel = data.accelerometer;
            return BNO085_OK;
        }
        
        board_delay_ms(1);
    }
    
    return BNO085_ERR_TIMEOUT;
}

int bno085_get_gyroscope(bno085_t *dev, bno085_vector_t *gyro)
{
    bno085_data_t data;
    int result;
    int attempts = 10;
    
    if (dev == NULL || gyro == NULL) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    while (attempts-- > 0) {
        result = bno085_poll(dev, &data);
        
        if (result < 0) {
            return result;
        }
        
        if (result == SH2_GYROSCOPE) {
            *gyro = data.gyroscope;
            return BNO085_OK;
        }
        
        board_delay_ms(1);
    }
    
    return BNO085_ERR_TIMEOUT;
}

int bno085_get_all_data(bno085_t *dev, bno085_data_t *data)
{
    if (dev == NULL || data == NULL) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    /* Return cached data - caller should call bno085_poll() first */
    memcpy(data, &s_sensor_data, sizeof(bno085_data_t));
    
    return BNO085_OK;
}

/*******************************************************************************
 * Public Functions - Utility
 ******************************************************************************/

int bno085_get_product_id(bno085_t *dev, shtp_product_id_t *product_id)
{
    uint8_t request[2] = {SH2_CMD_PRODUCT_ID_REQ, 0};
    int result;
    int attempts = 10;
    
    if (dev == NULL || product_id == NULL) {
        return BNO085_ERR_INVALID_PARAM;
    }
    
    /* Send product ID request on control channel */
    result = bno085_send_packet(dev, SHTP_CHANNEL_CONTROL, request, sizeof(request));
    if (result != BNO085_OK) {
        return result;
    }
    
    /* Wait for response */
    while (attempts-- > 0) {
        board_delay_ms(10);
        
        result = bno085_receive_packet(dev, 100);
        if (result <= SHTP_HEADER_SIZE) {
            continue;
        }
        
        /* Check for product ID response */
        uint8_t channel = dev->rx_buffer[2];
        uint8_t report_id = dev->rx_buffer[4];
        
        if (channel == SHTP_CHANNEL_CONTROL && report_id == SH2_CMD_PRODUCT_ID_RESP) {
            uint8_t *data = &dev->rx_buffer[4];
            
            product_id->reset_cause = data[1];
            product_id->sw_version_major = data[2];
            product_id->sw_version_minor = data[3];
            product_id->sw_part_number = data[4] | (data[5] << 8) |
                                         (data[6] << 16) | (data[7] << 24);
            product_id->sw_build_number = data[8] | (data[9] << 8) |
                                          (data[10] << 16) | (data[11] << 24);
            product_id->sw_version_patch = data[12] | (data[13] << 8);
            
            return BNO085_OK;
        }
    }
    
    return BNO085_ERR_TIMEOUT;
}

void bno085_quat_to_euler(const bno085_quaternion_t *quat, 
                          float *roll, float *pitch, float *yaw)
{
    if (quat == NULL || roll == NULL || pitch == NULL || yaw == NULL) {
        return;
    }
    
    float qi = quat->i;
    float qj = quat->j;
    float qk = quat->k;
    float qr = quat->real;
    
    /* Roll (x-axis rotation) */
    float sinr_cosp = 2.0f * (qr * qi + qj * qk);
    float cosr_cosp = 1.0f - 2.0f * (qi * qi + qj * qj);
    *roll = atan2f(sinr_cosp, cosr_cosp);
    
    /* Pitch (y-axis rotation) */
    float sinp = 2.0f * (qr * qj - qk * qi);
    if (fabsf(sinp) >= 1.0f) {
        *pitch = copysignf(3.14159265f / 2.0f, sinp);  /* Use 90 degrees if out of range */
    } else {
        *pitch = asinf(sinp);
    }
    
    /* Yaw (z-axis rotation) */
    float siny_cosp = 2.0f * (qr * qk + qi * qj);
    float cosy_cosp = 1.0f - 2.0f * (qj * qj + qk * qk);
    *yaw = atan2f(siny_cosp, cosy_cosp);
}

const char* bno085_report_name(bno085_report_type_t report_type)
{
    switch (report_type) {
        case BNO085_REPORT_ACCELEROMETER:     return "Accelerometer";
        case BNO085_REPORT_GYROSCOPE:         return "Gyroscope";
        case BNO085_REPORT_MAGNETOMETER:      return "Magnetometer";
        case BNO085_REPORT_LINEAR_ACCEL:      return "Linear Acceleration";
        case BNO085_REPORT_ROTATION_VECTOR:   return "Rotation Vector";
        case BNO085_REPORT_GRAVITY:           return "Gravity";
        case BNO085_REPORT_GAME_ROTATION:     return "Game Rotation Vector";
        case BNO085_REPORT_GEOMAG_ROTATION:   return "Geomagnetic Rotation";
        case BNO085_REPORT_STEP_COUNTER:      return "Step Counter";
        case BNO085_REPORT_STABILITY:         return "Stability";
        case BNO085_REPORT_ACTIVITY:          return "Activity";
        default:                              return "Unknown";
    }
}

const char* bno085_error_name(int error)
{
    switch (error) {
        case BNO085_OK:                  return "OK";
        case BNO085_ERR_I2C:             return "I2C error";
        case BNO085_ERR_TIMEOUT:         return "Timeout";
        case BNO085_ERR_NOT_FOUND:       return "Device not found";
        case BNO085_ERR_INVALID_DATA:    return "Invalid data";
        case BNO085_ERR_NOT_READY:       return "Not ready";
        case BNO085_ERR_BUFFER_OVERFLOW: return "Buffer overflow";
        case BNO085_ERR_INVALID_PARAM:   return "Invalid parameter";
        default:                         return "Unknown error";
    }
}
