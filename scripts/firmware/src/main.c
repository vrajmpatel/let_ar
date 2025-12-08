/**
 * @file main.c
 * @brief Main application entry point for LED Glasses IMU Firmware
 * 
 * Application firmware for the Adafruit LED Glasses Driver nRF52840
 * with BNO085 IMU sensor, streaming orientation data via BLE.
 * 
 * Citations:
 * - FIRMWARE_DESIGN.md: System architecture and design decisions
 * - nRF52840_PS_v1.11.pdf: nRF52840 product specification
 * - Adafruit BNO085 Guide: Sensor configuration
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "config.h"
#include "bno085.h"
#include "twim.h"
#include "shtp.h"

/* BLE Stack Headers */
#include "softdevice.h"
#include "ble_stack.h"
#include "ble_advertising.h"
#include "ble_imu_service.h"
#include "nrf_error.h"

/*******************************************************************************
 * Private Definitions
 ******************************************************************************/

/* Application states */
typedef enum {
    APP_STATE_INIT,
    APP_STATE_SENSOR_SETUP,
    APP_STATE_BLE_INIT,
    APP_STATE_RUNNING,
    APP_STATE_ERROR
} app_state_t;

/* Status LED blink patterns (in ms) */
#define LED_BLINK_INIT      1000    /* Slow blink during init */
#define LED_BLINK_RUNNING   200     /* Fast blink when running */
#define LED_BLINK_ERROR     100     /* Very fast blink on error */

/*******************************************************************************
 * Private Variables
 ******************************************************************************/

static app_state_t s_app_state = APP_STATE_INIT;
static bno085_t s_imu;
static bno085_data_t s_imu_data;
static uint32_t s_led_timer = 0;
static uint32_t s_sensor_timer = 0;
static bool s_sensor_ok = false;

/* BLE service instance */
static ble_imu_service_t s_imu_service;
static bool s_ble_connected = false;

/* Latest sensor readings for BLE notifications */
static ble_imu_quat_t s_quaternion;
static ble_imu_vector_t s_accel;
static ble_imu_vector_t s_gyro;

/*******************************************************************************
 * Private Functions - Error Handling
 ******************************************************************************/

/**
 * @brief Handle fatal error condition
 * @param error Error code
 */
static void app_fatal_error(int error)
{
    (void)error;  /* Could log error code */
    
    s_app_state = APP_STATE_ERROR;
    
    /* Rapid LED blink to indicate error */
    while (1) {
        board_led_toggle();
        board_delay_ms(LED_BLINK_ERROR);
    }
}

/*******************************************************************************
 * Private Functions - Sensor
 ******************************************************************************/

/**
 * @brief Initialize IMU sensor
 * @return 0 on success, error code on failure
 */
static int sensor_init(void)
{
    int result;
    
    /* Initialize BNO085
     * Citation: FIRMWARE_DESIGN.md "Initialization Sequence"
     */
    result = bno085_init(&s_imu);
    if (result != BNO085_OK) {
        return result;
    }
    
    /* Enable rotation vector at 200 Hz
     * Citation: FIRMWARE_DESIGN.md:
     *   "Report Type: Rotation Vector (0x05)"
     *   "Report Interval: 5000 µs (5 ms) = 200 Hz"
     */
    result = bno085_enable_rotation_vector(&s_imu, CONFIG_BNO085_REPORT_RATE_US);
    if (result != BNO085_OK) {
        return result;
    }
    
#if CONFIG_ENABLE_ACCELEROMETER
    /* Enable accelerometer at same rate */
    result = bno085_enable_accelerometer(&s_imu, CONFIG_BNO085_REPORT_RATE_US);
    if (result != BNO085_OK) {
        return result;
    }
#endif

#if CONFIG_ENABLE_GYROSCOPE
    /* Enable gyroscope at same rate */
    result = bno085_enable_gyroscope(&s_imu, CONFIG_BNO085_REPORT_RATE_US);
    if (result != BNO085_OK) {
        return result;
    }
#endif

    s_sensor_ok = true;
    return 0;
}

/**
 * @brief Poll sensor and update data
 */
static void sensor_poll(void)
{
    int report;
    
    if (!s_sensor_ok) {
        return;
    }
    
    /* Poll for sensor data */
    report = bno085_poll(&s_imu, &s_imu_data);
    
    if (report < 0) {
        /* Error polling sensor */
        return;
    }
    
    /* Update cached data based on report type */
    switch (report) {
        case SH2_ROTATION_VECTOR:
        case SH2_GAME_ROTATION_VECTOR:
            s_quaternion.i = s_imu_data.rotation_vector.i;
            s_quaternion.j = s_imu_data.rotation_vector.j;
            s_quaternion.k = s_imu_data.rotation_vector.k;
            s_quaternion.real = s_imu_data.rotation_vector.real;
            break;
            
        case SH2_ACCELEROMETER:
            s_accel.x = s_imu_data.accelerometer.x;
            s_accel.y = s_imu_data.accelerometer.y;
            s_accel.z = s_imu_data.accelerometer.z;
            break;
            
        case SH2_GYROSCOPE:
            s_gyro.x = s_imu_data.gyroscope.x;
            s_gyro.y = s_imu_data.gyroscope.y;
            s_gyro.z = s_imu_data.gyroscope.z;
            break;
            
        default:
            break;
    }
}

/*******************************************************************************
 * Private Functions - BLE
 ******************************************************************************/

/**
 * @brief BLE connection event handler
 * 
 * Called when a BLE central connects or disconnects.
 * 
 * @param connected True if connected, false if disconnected
 * @param p_state   Connection state information
 */
static void ble_conn_evt_handler(bool connected, const ble_stack_conn_state_t *p_state)
{
    (void)p_state;
    
    s_ble_connected = connected;
    
    if (connected) {
        /* Connection established
         * Citation: FIRMWARE_DESIGN.md - "Configure 2 Mbps PHY for throughput"
         * Request 2M PHY for higher throughput with IMU data */
        ble_stack_phy_update_2m();
        ble_stack_data_length_update();
    } else {
        /* Connection lost - advertising will auto-restart if configured */
    }
}

/**
 * @brief BLE advertising event handler
 * 
 * @param evt   Advertising event type
 * @param mode  Current advertising mode
 */
static void ble_adv_evt_handler(ble_adv_evt_t evt, ble_adv_mode_t mode)
{
    (void)mode;
    
    switch (evt) {
        case BLE_ADV_EVT_STARTED:
            /* Advertising started */
            break;
            
        case BLE_ADV_EVT_CONNECTED:
            /* Connection established via advertising */
            break;
            
        case BLE_ADV_EVT_FAST_TIMEOUT:
            /* Fast advertising timed out, switching to slow */
            break;
            
        default:
            break;
    }
}

/**
 * @brief BLE IMU service event handler
 * 
 * Handles events from the custom IMU GATT service.
 * 
 * @param evt Event data
 */
static void ble_imu_evt_handler(const ble_imu_evt_t *evt)
{
    switch (evt->type) {
        case BLE_IMU_EVT_CONNECTED:
            /* Client connected - could adjust sensor rate */
            break;
            
        case BLE_IMU_EVT_DISCONNECTED:
            /* Client disconnected */
            break;
            
        case BLE_IMU_EVT_QUAT_NOTIFY_EN:
            /* Start streaming quaternion data */
            break;
            
        case BLE_IMU_EVT_RATE_WRITE:
            /* Adjust sensor report rate
             * Citation: FIRMWARE_DESIGN.md - "Report Interval: 5000 µs (5 ms) = 200 Hz" */
            if (s_sensor_ok && evt->data.rate_ms >= 1) {
                uint32_t interval_us = (uint32_t)evt->data.rate_ms * 1000;
                bno085_enable_rotation_vector(&s_imu, interval_us);
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief Initialize BLE stack and services
 * @return 0 on success, error code on failure
 * 
 * Initialization sequence for S140 SoftDevice:
 * Citation: Nordic SDK - SoftDevice initialization sequence:
 *   1. sd_softdevice_enable() - Enable SoftDevice with clock config
 *   2. sd_ble_cfg_set() - Configure BLE parameters (called by softdevice_init)
 *   3. sd_ble_enable() - Enable BLE stack (called by softdevice_init)
 *   4. Configure GAP (device name, appearance, connection params)
 *   5. Initialize GATT services
 *   6. Configure and start advertising
 * 
 * Citation: FIRMWARE_DESIGN.md:
 *   "SoftDevice: S140 6.1.1"
 *   "App Start Address: 0x26000"
 *   "MTU: Up to 247 bytes"
 */
static int ble_init(void)
{
    uint32_t err_code;
    
    /*
     * Step 1: Initialize and enable SoftDevice S140
     * Citation: FIRMWARE_DESIGN.md Section "SoftDevice Configuration":
     *   - LFCLK source: XTAL (32.768 kHz crystal on LED Glasses board)
     *   - 1 peripheral connection
     *   - MTU: 247 bytes for IMU data throughput
     *   - 2 vendor-specific UUIDs (service + characteristics)
     */
    softdevice_config_t sd_config = {
        .lfclk_source      = NRF_CLOCK_LF_SRC_XTAL,
        .lfclk_accuracy    = NRF_CLOCK_LF_ACCURACY_20_PPM,
        .rc_ctiv           = 0,
        .rc_temp_ctiv      = 0,
        .periph_conn_count = 1,
        .central_conn_count = 0,
        .att_mtu           = 247,  /* Citation: FIRMWARE_DESIGN.md "MTU: Up to 247 bytes" */
        .vs_uuid_count     = 2,    /* Service UUID + characteristics */
        .attr_tab_size     = 1408, /* GATT attribute table size */
        .service_changed   = false,
        .dcdc_enabled      = false, /* LED Glasses board may not have DC/DC inductor */
    };
    
    err_code = softdevice_init(&sd_config);
    if (err_code != NRF_SUCCESS) {
        return (int)err_code;
    }
    
    /*
     * Step 2: Initialize BLE stack (GAP configuration)
     * Citation: FIRMWARE_DESIGN.md Section "BLE Configuration":
     *   - Device name: "LET-AR IMU"
     *   - Connection interval: 7.5-15ms for high throughput
     *   - TX power: 0 dBm (good range, moderate power)
     */
    ble_stack_config_t stack_config = {
        .device_name       = "LET-AR IMU",
        .appearance        = BLE_APPEARANCE_GENERIC_TAG,
        .min_conn_interval = 6,    /* 7.5ms - minimum for high throughput */
        .max_conn_interval = 12,   /* 15ms */
        .slave_latency     = 0,    /* No latency for real-time IMU data */
        .conn_sup_timeout  = 400,  /* 4 seconds */
        .tx_power          = 0,    /* 0 dBm */
    };
    
    err_code = ble_stack_init(&stack_config);
    if (err_code != NRF_SUCCESS) {
        return (int)err_code;
    }
    
    /* Register connection event handler */
    ble_stack_conn_handler_set(ble_conn_evt_handler);
    
    /*
     * Step 3: Initialize custom IMU GATT service
     * Citation: FIRMWARE_DESIGN.md Section "BLE Service Design":
     *   - Custom IMU Service UUID: 12340000-1234-1234-1234-123456789ABC
     *   - Characteristics: Quaternion, Accelerometer, Gyroscope, Sample Rate, Status
     */
    ble_imu_config_t imu_config = {
        .default_rate_ms = 5,  /* 200 Hz = 5ms, Citation: FIRMWARE_DESIGN.md */
    };
    
    err_code = ble_imu_service_init(&s_imu_service, &imu_config, ble_imu_evt_handler);
    if (err_code != NRF_SUCCESS) {
        return (int)err_code;
    }
    
    /*
     * Step 4: Initialize advertising
     * Citation: FIRMWARE_DESIGN.md - BLE 5.0 advertising
     *   - Fast advertising: 100ms interval for quick connection
     *   - Include device name and service UUID
     *   - Auto-restart after disconnection
     */
    ble_advertising_config_t adv_config = {
        .fast_interval       = 160,   /* 100ms (160 * 0.625ms) */
        .slow_interval       = 1600,  /* 1000ms */
        .fast_timeout        = 3000,  /* 30 seconds */
        .slow_timeout        = 0,     /* Infinite */
        .include_name        = true,
        .include_appearance  = false,
        .include_tx_power    = false,
        .uuid_count          = 0,     /* Will be added below */
        .include_name_in_sr  = false,
        .p_manuf_data        = NULL,
        .manuf_data_len      = 0,
        .company_id          = 0xFFFF,
        .auto_restart        = true,  /* Auto-restart after disconnect */
    };
    
    err_code = ble_advertising_init(&adv_config);
    if (err_code != NRF_SUCCESS) {
        return (int)err_code;
    }
    
    /* Add IMU service UUID to advertising data */
    ble_uuid_t imu_uuid = {
        .uuid = BLE_IMU_SERVICE_UUID,
        .type = s_imu_service.uuid_type,
    };
    ble_advertising_uuid_add(&imu_uuid);
    
    /* Register advertising event handler */
    ble_advertising_evt_handler_set(ble_adv_evt_handler);
    
    /*
     * Step 5: Start advertising
     * Citation: FIRMWARE_DESIGN.md - "Start advertising" phase
     */
    err_code = ble_advertising_start();
    if (err_code != NRF_SUCCESS) {
        return (int)err_code;
    }
    
    return 0;
}

/*******************************************************************************
 * Private Functions - Main Loop
 ******************************************************************************/

/**
 * @brief Update status LED based on application state
 */
static void led_update(void)
{
    uint32_t blink_rate;
    
    switch (s_app_state) {
        case APP_STATE_INIT:
        case APP_STATE_SENSOR_SETUP:
        case APP_STATE_BLE_INIT:
            blink_rate = LED_BLINK_INIT;
            break;
            
        case APP_STATE_RUNNING:
            blink_rate = LED_BLINK_RUNNING;
            break;
            
        case APP_STATE_ERROR:
        default:
            blink_rate = LED_BLINK_ERROR;
            break;
    }
    
    /* Simple timer-based blink */
    s_led_timer += CONFIG_MAIN_LOOP_DELAY_MS;
    if (s_led_timer >= blink_rate) {
        s_led_timer = 0;
        board_led_toggle();
    }
}

/**
 * @brief Send BLE notifications for IMU data
 * 
 * Citation: FIRMWARE_DESIGN.md - "BLE Service Design":
 *   - Notify quaternion, accelerometer, gyroscope data to connected clients
 *   - Only send when notifications are enabled and client is connected
 */
static void ble_notify_imu_data(void)
{
    uint32_t err_code;
    
    /* Only send notifications if connected */
    if (!s_ble_connected) {
        return;
    }
    
    /* Send quaternion notification (primary data) */
    err_code = ble_imu_notify_quaternion(&s_imu_service, &s_quaternion);
    if (err_code != NRF_SUCCESS && err_code != NRF_ERROR_INVALID_STATE) {
        /* NRF_ERROR_INVALID_STATE means notifications not enabled - that's OK */
        /* Other errors might indicate buffer full, etc. */
    }
    
#if CONFIG_ENABLE_ACCELEROMETER
    /* Send accelerometer notification */
    err_code = ble_imu_notify_accelerometer(&s_imu_service, &s_accel);
    (void)err_code;  /* Ignore errors - best effort */
#endif

#if CONFIG_ENABLE_GYROSCOPE
    /* Send gyroscope notification */
    err_code = ble_imu_notify_gyroscope(&s_imu_service, &s_gyro);
    (void)err_code;  /* Ignore errors - best effort */
#endif
}

/**
 * @brief Main application loop iteration
 * 
 * Citation: Nordic SDK - SoftDevice event processing:
 *   - Call sd_app_evt_wait() to enter low-power sleep
 *   - SoftDevice will wake on BLE events or interrupts
 *   - Process events, then poll sensors and send notifications
 * 
 * Citation: FIRMWARE_DESIGN.md - "Main Loop":
 *   1. Wait for SoftDevice event (low power)
 *   2. Process any pending BLE events
 *   3. Poll sensor data
 *   4. Send BLE notifications
 *   5. Update LED status
 */
static void app_main_loop(void)
{
    /*
     * Wait for SoftDevice event (puts CPU in low-power mode)
     * Citation: nRF52840_PS_v1.11.pdf Section "Power Management":
     *   - sd_app_evt_wait() enters System ON Idle mode
     *   - CPU wakes on any interrupt (BLE events, timers, etc.)
     *   - Reduces power consumption significantly
     * 
     * softdevice_wait_for_event() wraps sd_app_evt_wait() with proper
     * handling for when SoftDevice is disabled.
     */
    softdevice_wait_for_event();
    
    /*
     * Process SoftDevice BLE events
     * Citation: Nordic SDK - Event handling:
     *   - sd_ble_evt_get() retrieves pending BLE events
     *   - Events dispatched to stack handlers (GAP, GATT, etc.)
     *   - Must be called in a loop until no more events
     */
    softdevice_evt_process();
    
    /* Poll sensor data from BNO085 */
    sensor_poll();
    
    /* Send BLE notifications if enabled */
    ble_notify_imu_data();
    
    /* Update status LED */
    led_update();
}

/*******************************************************************************
 * Main Entry Point
 ******************************************************************************/

/**
 * @brief Main function - firmware entry point
 * @return Never returns
 * 
 * Citation: FIRMWARE_DESIGN.md "Development Phases":
 *   Phase 1: Project Setup
 *   Phase 2: I2C Driver (TWIM)
 *   Phase 3: BNO085 Driver
 *   Phase 4: BLE Service
 *   Phase 5: Integration & Test
 */
int main(void)
{
    int result;
    
    /* ========== Phase 1: Board Initialization ========== */
    s_app_state = APP_STATE_INIT;
    
    result = board_init();
    if (result != 0) {
        app_fatal_error(result);
    }
    
    /* Indicate startup with LED */
    board_led_on();
    board_delay_ms(500);
    board_led_off();
    
    /* ========== Phase 2-3: Sensor Initialization ========== */
    s_app_state = APP_STATE_SENSOR_SETUP;
    
    result = sensor_init();
    if (result != 0) {
        /* Sensor init failed - continue anyway for debugging */
        s_sensor_ok = false;
    }
    
    /* ========== Phase 4: BLE Initialization ========== */
    s_app_state = APP_STATE_BLE_INIT;
    
    result = ble_init();
    if (result != 0) {
        app_fatal_error(result);
    }
    
    /* ========== Phase 5: Main Loop ========== */
    s_app_state = APP_STATE_RUNNING;
    
    while (1) {
        app_main_loop();
    }
    
    /* Never reached */
    return 0;
}

/*******************************************************************************
 * Interrupt Handlers
 ******************************************************************************/

/**
 * @brief Hard Fault Handler
 */
void HardFault_Handler(void)
{
    /* Toggle LED rapidly to indicate hard fault */
    while (1) {
        board_led_toggle();
        for (volatile int i = 0; i < 100000; i++) {
            __asm volatile ("nop");
        }
    }
}

/**
 * @brief NMI Handler
 */
void NMI_Handler(void)
{
    /* Nothing to do */
}

/**
 * @brief Memory Management Fault Handler
 */
void MemManage_Handler(void)
{
    HardFault_Handler();
}

/**
 * @brief Bus Fault Handler
 */
void BusFault_Handler(void)
{
    HardFault_Handler();
}

/**
 * @brief Usage Fault Handler
 */
void UsageFault_Handler(void)
{
    HardFault_Handler();
}
