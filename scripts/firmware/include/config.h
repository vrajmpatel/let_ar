/**
 * @file config.h
 * @brief Application configuration for LED Glasses IMU Firmware
 * 
 * Central configuration file for all firmware parameters including
 * I2C settings, BNO085 sensor configuration, and BLE parameters.
 * 
 * Citations:
 * - nRF52840_PS_v1.11.pdf: TWIM frequency and configuration
 * - Adafruit BNO085 Guide: Sensor configuration and report types
 * - FIRMWARE_DESIGN.md: Design decisions and specifications
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Firmware Version
 ******************************************************************************/
#define FW_VERSION_MAJOR        1
#define FW_VERSION_MINOR        0
#define FW_VERSION_PATCH        0
#define FW_VERSION_STRING       "1.0.0"

/*******************************************************************************
 * I2C (TWIM) Configuration
 * Citation: nRF52840_PS_v1.11.pdf Section 6.31
 *   - TWIM0 Base: 0x40003000
 *   - TWIM1 Base: 0x40004000
 *   - Supported baud rates: 100, 250, 400 kbps
 *   - FREQUENCY register offset: 0x524
 *     K100 = 0x01980000
 *     K250 = 0x04000000
 *     K400 = 0x06400000
 * Citation: Adafruit BNO085 Guide: "i2c = busio.I2C(board.SCL, board.SDA, frequency=400000)"
 ******************************************************************************/
#define CONFIG_TWIM_INSTANCE        0           /* Use TWIM0 */
#define CONFIG_I2C_FREQUENCY        400000      /* 400 kHz (Fast Mode) */

/* TWIM Frequency register values from nRF52840_PS_v1.11.pdf Section 6.31.7.21 */
#define TWIM_FREQ_K100              0x01980000UL
#define TWIM_FREQ_K250              0x04000000UL
#define TWIM_FREQ_K400              0x06400000UL

#define CONFIG_I2C_TIMEOUT_MS       100         /* Transaction timeout */
#define CONFIG_I2C_RETRY_COUNT      3           /* Retry on NACK */

/*******************************************************************************
 * BNO085 Sensor Configuration
 * Citation: Adafruit BNO085 Guide: "The default I2C address for the BNO08x is 0x4A"
 * Citation: FIRMWARE_DESIGN.md: "Report Interval: 5000 µs (5 ms) = 200 Hz"
 ******************************************************************************/
#define CONFIG_BNO085_I2C_ADDR      0x4A        /* Default I2C address */
#define CONFIG_BNO085_REPORT_RATE_US 5000       /* 5ms = 200 Hz */
#define CONFIG_BNO085_REPORT_RATE_MS 5          /* 5ms = 200 Hz */

/* Report Types - Citation: FIRMWARE_DESIGN.md Section "BNO085 Report Types" */
#define CONFIG_ENABLE_ROTATION_VECTOR   1       /* Primary output */
#define CONFIG_ENABLE_ACCELEROMETER     1       /* Raw accel data */
#define CONFIG_ENABLE_GYROSCOPE         1       /* Raw gyro data */
#define CONFIG_ENABLE_MAGNETOMETER      0       /* Disabled by default */
#define CONFIG_ENABLE_GAME_ROTATION     0       /* No magnetometer fusion */

/*******************************************************************************
 * BLE Configuration
 * Citation: nRF52840_PS_v1.11.pdf: "Bluetooth 5 – 2 Mbps, 1 Mbps, 500 kbps, 125 kbps"
 * Citation: FIRMWARE_DESIGN.md: "SoftDevice: S140 6.1.1"
 ******************************************************************************/
#define CONFIG_BLE_DEVICE_NAME          "IMU_Glasses"
#define CONFIG_BLE_MANUFACTURER_NAME    "Adafruit"

/* Connection parameters */
#define CONFIG_BLE_MIN_CONN_INTERVAL    6       /* 7.5 ms (units of 1.25 ms) */
#define CONFIG_BLE_MAX_CONN_INTERVAL    12      /* 15 ms */
#define CONFIG_BLE_SLAVE_LATENCY        0       /* No latency for responsiveness */
#define CONFIG_BLE_CONN_SUP_TIMEOUT     400     /* 4 seconds (units of 10 ms) */

/* PHY preference - Citation: "2 Mbps PHY for throughput" */
#define CONFIG_BLE_PREFERRED_PHY        2       /* 2 Mbps PHY */

/* MTU size - Citation: "MTU: Up to 247 bytes" */
#define CONFIG_BLE_GATT_MTU             247     /* Maximum BLE 5.0 MTU */

/* TX Power - Citation: "-20 to +8 dBm TX power" */
#define CONFIG_BLE_TX_POWER             4       /* +4 dBm */

/*******************************************************************************
 * IMU Service UUIDs
 * Citation: FIRMWARE_DESIGN.md: "Custom IMU Service UUID: 12340000-1234-..."
 ******************************************************************************/
/* Base UUID: 12340000-1234-1234-1234-123456789ABC */
#define IMU_SERVICE_UUID_BASE   {0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12, 0x34, 0x12, \
                                 0x34, 0x12, 0x34, 0x12, 0x00, 0x00, 0x34, 0x12}

#define IMU_SERVICE_UUID        0x0000  /* Service UUID short */
#define IMU_CHAR_QUAT_UUID      0x0001  /* Quaternion characteristic */
#define IMU_CHAR_ACCEL_UUID     0x0002  /* Accelerometer characteristic */
#define IMU_CHAR_GYRO_UUID      0x0003  /* Gyroscope characteristic */
#define IMU_CHAR_RATE_UUID      0x0004  /* Sample rate characteristic */
#define IMU_CHAR_STATUS_UUID    0x0005  /* Status characteristic */

/*******************************************************************************
 * Debug Configuration
 ******************************************************************************/
#define CONFIG_DEBUG_UART           1       /* Enable UART debug output */
#define CONFIG_DEBUG_RTT            0       /* Disable SEGGER RTT */
#define CONFIG_DEBUG_LEVEL          2       /* 0=off, 1=error, 2=warn, 3=info, 4=debug */

/*******************************************************************************
 * Timing Configuration
 ******************************************************************************/
#define CONFIG_MAIN_LOOP_DELAY_MS   1       /* Main loop minimum delay */
#define CONFIG_LED_BLINK_INTERVAL   500     /* Status LED blink interval (ms) */

/*******************************************************************************
 * Buffer Sizes
 ******************************************************************************/
#define CONFIG_SHTP_RX_BUFFER_SIZE  512     /* SHTP receive buffer */
#define CONFIG_SHTP_TX_BUFFER_SIZE  256     /* SHTP transmit buffer */
#define CONFIG_BLE_TX_BUFFER_SIZE   256     /* BLE notification buffer */

/*******************************************************************************
 * Error Handling
 ******************************************************************************/
#define CONFIG_ASSERT_ENABLED       1       /* Enable runtime assertions */
#define CONFIG_WATCHDOG_ENABLED     1       /* Enable hardware watchdog */
#define CONFIG_WATCHDOG_TIMEOUT_MS  5000    /* 5 second watchdog timeout */

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
