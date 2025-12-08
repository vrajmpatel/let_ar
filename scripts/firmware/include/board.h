/**
 * @file board.h
 * @brief Adafruit LED Glasses Driver nRF52840 board definitions
 * 
 * Pin definitions and board-specific constants for the LED Glasses Driver board.
 * 
 * @note These pins are derived from Adafruit's LED Glasses Driver pinout documentation.
 *       The STEMMA QT connector uses board.SCL and board.SDA for I2C.
 * 
 * Citations:
 * - nRF52840_PS_v1.11.pdf: "ARM Cortex-M4F @ 64MHz, 1 MB flash and 256 KB RAM"
 * - Adafruit LED Glasses Guide: "STEMMA QT connector labeled I2C on the board"
 */

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * MCU Configuration
 * Citation: nRF52840_PS_v1.11.pdf - "Arm® Cortex®-M4 32-bit processor with FPU, 64 MHz"
 ******************************************************************************/
#define BOARD_MCU_FREQUENCY_HZ      64000000UL  /* 64 MHz CPU clock */
#define BOARD_FLASH_SIZE            (1024 * 1024) /* 1 MB Flash */
#define BOARD_RAM_SIZE              (256 * 1024)  /* 256 KB RAM */

/*******************************************************************************
 * SoftDevice Configuration
 * Citation: Device INFO_UF2.TXT: "SoftDevice: S140 6.1.1"
 * Citation: FIRMWARE_DESIGN.md: "App Start Address: 0x26000"
 ******************************************************************************/
#define SOFTDEVICE_S140             1
#define SOFTDEVICE_APP_START_ADDR   0x00026000UL  /* Application start after S140 */
#define SOFTDEVICE_RAM_START        0x20000000UL

/*******************************************************************************
 * I2C (STEMMA QT) Pin Configuration
 * Citation: Adafruit LED Glasses Guide: "STEMMA QT connector labeled I2C"
 * Citation: CircuitPython: "board.SCL and board.SDA"
 * 
 * @note These are the default STEMMA QT I2C pins on the LED Glasses Driver.
 *       Actual pin numbers may need verification from Adafruit's pinout diagram.
 ******************************************************************************/
#define BOARD_I2C_SCL_PIN           27  /* P0.27 - STEMMA QT SCL */
#define BOARD_I2C_SDA_PIN           26  /* P0.26 - STEMMA QT SDA */
#define BOARD_I2C_SCL_PORT          0   /* Port 0 */
#define BOARD_I2C_SDA_PORT          0   /* Port 0 */

/*******************************************************************************
 * BNO085 IMU Configuration
 * Citation: Adafruit BNO085 Guide: "The default I2C address for the BNO08x is 0x4A"
 * Citation: Adafruit BNO085 Guide: "RST- Reset, Active Low. Pull low to GND to reset"
 * Citation: Adafruit BNO085 Guide: "INT - Interrupt/Data Ready-Active Low pin"
 ******************************************************************************/
#define BNO085_I2C_ADDR             0x4A    /* Default I2C address */
#define BNO085_I2C_ADDR_ALT         0x4B    /* Alternate address (DI pin high) */

/* Optional INT and RST pins if connected via additional wiring */
#define BNO085_INT_PIN              0xFF    /* Not connected via STEMMA QT */
#define BNO085_RST_PIN              0xFF    /* Not connected via STEMMA QT */

/*******************************************************************************
 * LED Configuration (on-board indicator LED)
 * Pin: P0.31 - Red LED on Adafruit LED Glasses Driver board
 ******************************************************************************/
#define BOARD_LED_PIN               31      /* P0.31 - LED */
#define BOARD_LED_PORT              0

/*******************************************************************************
 * Button Configuration
 ******************************************************************************/
#define BOARD_BUTTON_PIN            7       /* P0.07 - User button (if present) */
#define BOARD_BUTTON_PORT           0
#define BOARD_BUTTON_ACTIVE_LOW     1       /* Button pulls to GND when pressed */

/*******************************************************************************
 * UART Debug Configuration (optional)
 ******************************************************************************/
#define BOARD_UART_TX_PIN           6       /* P0.06 */
#define BOARD_UART_RX_PIN           8       /* P0.08 */
#define BOARD_UART_BAUDRATE         115200

/*******************************************************************************
 * UF2 Bootloader Configuration
 * Citation: Device INFO_UF2.TXT: "UF2 Bootloader 0.8.0"
 * Citation: Device INFO_UF2.TXT: "Board-ID: nRF52840-LedGlasses-revA"
 ******************************************************************************/
#define UF2_FAMILY_ID               0xADA52840UL
#define UF2_BOARD_ID                "nRF52840-LedGlasses-revA"

/*******************************************************************************
 * GPIO Helper Macros
 * Citation: nRF52840_PS_v1.11.pdf: GPIO pin configuration
 ******************************************************************************/
#define GPIO_PIN(port, pin)         ((((port) & 0x01) << 5) | ((pin) & 0x1F))
#define GPIO_PORT(gpio)             (((gpio) >> 5) & 0x01)
#define GPIO_PIN_NUM(gpio)          ((gpio) & 0x1F)

/*******************************************************************************
 * Board Initialization Function Prototypes
 ******************************************************************************/

/**
 * @brief Initialize board-specific hardware
 * @return 0 on success, negative error code on failure
 */
int board_init(void);

/**
 * @brief Configure GPIO pin as output
 * @param port GPIO port (0 or 1)
 * @param pin Pin number (0-31)
 */
void board_gpio_output(uint8_t port, uint8_t pin);

/**
 * @brief Configure GPIO pin as input
 * @param port GPIO port (0 or 1)  
 * @param pin Pin number (0-31)
 * @param pull Pull configuration (0=none, 1=pulldown, 3=pullup)
 */
void board_gpio_input(uint8_t port, uint8_t pin, uint8_t pull);

/**
 * @brief Set GPIO pin high
 * @param port GPIO port (0 or 1)
 * @param pin Pin number (0-31)
 */
void board_gpio_set(uint8_t port, uint8_t pin);

/**
 * @brief Set GPIO pin low
 * @param port GPIO port (0 or 1)
 * @param pin Pin number (0-31)
 */
void board_gpio_clear(uint8_t port, uint8_t pin);

/**
 * @brief Toggle GPIO pin
 * @param port GPIO port (0 or 1)
 * @param pin Pin number (0-31)
 */
void board_gpio_toggle(uint8_t port, uint8_t pin);

/**
 * @brief Read GPIO pin state
 * @param port GPIO port (0 or 1)
 * @param pin Pin number (0-31)
 * @return Pin state (0 or 1)
 */
uint8_t board_gpio_read(uint8_t port, uint8_t pin);

/**
 * @brief Turn on board LED
 */
void board_led_on(void);

/**
 * @brief Turn off board LED
 */
void board_led_off(void);

/**
 * @brief Toggle board LED
 */
void board_led_toggle(void);

/**
 * @brief Simple delay in milliseconds
 * @param ms Delay in milliseconds
 */
void board_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
