/**
 * @file board.c
 * @brief Adafruit LED Glasses Driver nRF52840 board initialization
 * 
 * Board-level initialization and GPIO functions for the LED Glasses Driver.
 * 
 * Citations:
 * - nRF52840_PS_v1.11.pdf: GPIO and peripheral configuration
 * - Adafruit LED Glasses Guide: Pin definitions
 * 
 * CRITICAL Implementation Notes:
 * 1. GPIO pins for I2C MUST be pre-configured with S0D1 drive strength BEFORE
 *    enabling TWIM peripheral (Citation: nRF52840_PS_v1.11.pdf Section 6.31.6 Table 52)
 * 
 * 2. Memory barriers (__DSB, __ISB) are required after peripheral configuration
 *    to ensure writes complete before enabling (Citation: ARM Cortex-M4 TRM)
 * 
 * 3. The TWIM peripheral PSEL registers must only be configured when TWIM is
 *    disabled (Citation: nRF52840_PS_v1.11.pdf Section 6.31.6)
 */

#include "board.h"
#include "twim.h"
#include "config.h"
#include <stddef.h>

/* Memory barriers for Cortex-M4 */
#define __DSB() __asm volatile ("dsb 0xF" ::: "memory")
#define __ISB() __asm volatile ("isb 0xF" ::: "memory")
#define __NOP() __asm volatile ("nop")

/*******************************************************************************
 * GPIO Register Definitions
 * Citation: nRF52840_PS_v1.11.pdf Section 6.9 (GPIO)
 ******************************************************************************/

/* GPIO Base Addresses */
#define GPIO_P0_BASE            0x50000000UL
#define GPIO_P1_BASE            0x50000300UL

/* GPIO Register Offsets */
#define GPIO_OUT                0x504   /* Write GPIO port */
#define GPIO_OUTSET             0x508   /* Set individual bits in GPIO port */
#define GPIO_OUTCLR             0x50C   /* Clear individual bits in GPIO port */
#define GPIO_IN                 0x510   /* Read GPIO port */
#define GPIO_DIR                0x514   /* Direction of GPIO pins */
#define GPIO_DIRSET             0x518   /* Set direction to output */
#define GPIO_DIRCLR             0x51C   /* Set direction to input */
#define GPIO_PIN_CNF(n)         (0x700 + ((n) * 4))  /* Pin configuration */

/* PIN_CNF Register bit positions */
#define GPIO_PIN_CNF_DIR_POS        0
#define GPIO_PIN_CNF_INPUT_POS      1
#define GPIO_PIN_CNF_PULL_POS       2
#define GPIO_PIN_CNF_DRIVE_POS      8
#define GPIO_PIN_CNF_SENSE_POS      16

/* PIN_CNF Register values */
#define GPIO_PIN_CNF_DIR_INPUT      (0UL << GPIO_PIN_CNF_DIR_POS)
#define GPIO_PIN_CNF_DIR_OUTPUT     (1UL << GPIO_PIN_CNF_DIR_POS)
#define GPIO_PIN_CNF_INPUT_CONNECT  (0UL << GPIO_PIN_CNF_INPUT_POS)
#define GPIO_PIN_CNF_INPUT_DISCONNECT (1UL << GPIO_PIN_CNF_INPUT_POS)
#define GPIO_PIN_CNF_PULL_DISABLED  (0UL << GPIO_PIN_CNF_PULL_POS)
#define GPIO_PIN_CNF_PULL_DOWN      (1UL << GPIO_PIN_CNF_PULL_POS)
#define GPIO_PIN_CNF_PULL_UP        (3UL << GPIO_PIN_CNF_PULL_POS)
#define GPIO_PIN_CNF_DRIVE_S0S1     (0UL << GPIO_PIN_CNF_DRIVE_POS)
#define GPIO_PIN_CNF_DRIVE_H0H1     (3UL << GPIO_PIN_CNF_DRIVE_POS)
#define GPIO_PIN_CNF_DRIVE_S0D1     (6UL << GPIO_PIN_CNF_DRIVE_POS)  /* Standard 0, Disconnect 1 (I2C) */

/* Register access macros */
#define GPIO_REG(base, offset)      (*(volatile uint32_t *)((base) + (offset)))

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/* Global TWIM instance for I2C communication */
twim_t g_twim;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief Get GPIO base address for port
 * @param port Port number (0 or 1)
 * @return Base address
 */
static uint32_t gpio_base(uint8_t port)
{
    return (port == 0) ? GPIO_P0_BASE : GPIO_P1_BASE;
}

/**
 * @brief Simple busy-wait delay
 * @param cycles Number of loop iterations
 */
static void delay_cycles(volatile uint32_t cycles)
{
    while (cycles--) {
        __asm volatile ("nop");
    }
}

/*******************************************************************************
 * Public Functions - GPIO
 ******************************************************************************/

void board_gpio_output(uint8_t port, uint8_t pin)
{
    uint32_t base = gpio_base(port);
    
    /* Configure pin as output with standard drive
     * Citation: nRF52840_PS_v1.11.pdf Section 6.9.2
     */
    GPIO_REG(base, GPIO_PIN_CNF(pin)) = GPIO_PIN_CNF_DIR_OUTPUT |
                                        GPIO_PIN_CNF_INPUT_DISCONNECT |
                                        GPIO_PIN_CNF_PULL_DISABLED |
                                        GPIO_PIN_CNF_DRIVE_S0S1;
}

void board_gpio_input(uint8_t port, uint8_t pin, uint8_t pull)
{
    uint32_t base = gpio_base(port);
    uint32_t pull_config;
    
    switch (pull) {
        case 1:  pull_config = GPIO_PIN_CNF_PULL_DOWN; break;
        case 3:  pull_config = GPIO_PIN_CNF_PULL_UP; break;
        default: pull_config = GPIO_PIN_CNF_PULL_DISABLED; break;
    }
    
    GPIO_REG(base, GPIO_PIN_CNF(pin)) = GPIO_PIN_CNF_DIR_INPUT |
                                        GPIO_PIN_CNF_INPUT_CONNECT |
                                        pull_config |
                                        GPIO_PIN_CNF_DRIVE_S0S1;
}

void board_gpio_set(uint8_t port, uint8_t pin)
{
    GPIO_REG(gpio_base(port), GPIO_OUTSET) = (1UL << pin);
}

void board_gpio_clear(uint8_t port, uint8_t pin)
{
    GPIO_REG(gpio_base(port), GPIO_OUTCLR) = (1UL << pin);
}

void board_gpio_toggle(uint8_t port, uint8_t pin)
{
    uint32_t base = gpio_base(port);
    uint32_t current = GPIO_REG(base, GPIO_OUT);
    
    if (current & (1UL << pin)) {
        GPIO_REG(base, GPIO_OUTCLR) = (1UL << pin);
    } else {
        GPIO_REG(base, GPIO_OUTSET) = (1UL << pin);
    }
}

uint8_t board_gpio_read(uint8_t port, uint8_t pin)
{
    uint32_t in = GPIO_REG(gpio_base(port), GPIO_IN);
    return (in & (1UL << pin)) ? 1 : 0;
}

/*******************************************************************************
 * Public Functions - LED
 ******************************************************************************/

void board_led_on(void)
{
    /* LED is typically active low on nRF52 boards */
    board_gpio_clear(BOARD_LED_PORT, BOARD_LED_PIN);
}

void board_led_off(void)
{
    board_gpio_set(BOARD_LED_PORT, BOARD_LED_PIN);
}

void board_led_toggle(void)
{
    board_gpio_toggle(BOARD_LED_PORT, BOARD_LED_PIN);
}

/*******************************************************************************
 * Public Functions - Delay
 ******************************************************************************/

void board_delay_ms(uint32_t ms)
{
    /* Approximate delay assuming 64 MHz clock
     * This is a rough estimate - for accurate timing use a timer
     * Citation: nRF52840_PS_v1.11.pdf: "64 MHz" CPU clock
     */
    while (ms--) {
        delay_cycles(16000);  /* ~1ms at 64MHz with loop overhead */
    }
}

/*******************************************************************************
 * Public Functions - Board Initialization
 ******************************************************************************/

int board_init(void)
{
    int result;
    
    /* Configure LED pin as output (off) */
    board_gpio_output(BOARD_LED_PORT, BOARD_LED_PIN);
    board_led_off();
    
    /* Configure button pin as input with pull-up (if present) */
    board_gpio_input(BOARD_BUTTON_PORT, BOARD_BUTTON_PIN, 3);  /* Pull-up */
    
    /* =========================================================================
     * CRITICAL: I2C Pin Configuration - MUST be done BEFORE enabling TWIM
     * =========================================================================
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.6 Table 52:
     *   "TWI master signal: Direction=Input, Drive strength=S0D1"
     *   "To secure correct signal levels on the pins used by the TWI master 
     *    when the system is in OFF mode, and when the TWI master is disabled,
     *    these pins must be configured in the GPIO peripheral"
     * 
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.6:
     *   "PSEL.SCL, PSEL.SDA must only be configured when the TWI master is disabled"
     * 
     * The S0D1 drive mode means:
     *   - S0: Standard '0', disconnect '1' (open-drain)
     *   - This allows the I2C pull-up resistors to pull the line high
     *   - The nRF52840 can only drive the line low
     */
    uint32_t scl_base = gpio_base(BOARD_I2C_SCL_PORT);
    uint32_t sda_base = gpio_base(BOARD_I2C_SDA_PORT);
    
    /* First, ensure pins are set HIGH (released) before configuring
     * This prevents glitches on the I2C bus during configuration
     */
    GPIO_REG(scl_base, GPIO_OUTSET) = (1UL << BOARD_I2C_SCL_PIN);
    GPIO_REG(sda_base, GPIO_OUTSET) = (1UL << BOARD_I2C_SDA_PIN);
    
    /* Configure SCL pin for I2C
     * Citation: nRF52840_PS_v1.11.pdf Section 6.9.2 PIN_CNF register
     */
    GPIO_REG(scl_base, GPIO_PIN_CNF(BOARD_I2C_SCL_PIN)) = 
        GPIO_PIN_CNF_DIR_INPUT |
        GPIO_PIN_CNF_INPUT_CONNECT |
        GPIO_PIN_CNF_PULL_DISABLED |  /* External pull-ups on BNO085 breakout */
        GPIO_PIN_CNF_DRIVE_S0D1;
    
    /* Configure SDA pin for I2C */
    GPIO_REG(sda_base, GPIO_PIN_CNF(BOARD_I2C_SDA_PIN)) = 
        GPIO_PIN_CNF_DIR_INPUT |
        GPIO_PIN_CNF_INPUT_CONNECT |
        GPIO_PIN_CNF_PULL_DISABLED |
        GPIO_PIN_CNF_DRIVE_S0D1;
    
    /* Memory barrier to ensure GPIO configuration completes before TWIM init
     * Citation: ARM Cortex-M4 TRM - Required for peripheral configuration ordering
     */
    __DSB();
    __ISB();
    
    /* Small delay to allow I2C lines to stabilize at high level
     * This ensures the bus is in a known state before TWIM takes control
     */
    for (volatile int i = 0; i < 1000; i++) {
        __NOP();
    }
    
    /* Initialize TWIM (I2C)
     * Citation: FIRMWARE_DESIGN.md:
     *   "I2C @ 400kHz (STEMMA QT)"
     *   "TWIM0 Base: 0x40003000"
     * Citation: nRF52840_PS_v1.11.pdf Section 6.31.7.21:
     *   "K400: 0x06400000 - 400 kbps"
     * 
     * Note: twim_init() will configure PSEL registers while TWIM is disabled,
     * as required by the datasheet.
     */
    twim_config_t twim_config = {
        .scl_pin = BOARD_I2C_SCL_PIN,
        .scl_port = BOARD_I2C_SCL_PORT,
        .sda_pin = BOARD_I2C_SDA_PIN,
        .sda_port = BOARD_I2C_SDA_PORT,
        .frequency = TWIM_FREQ_400K
    };
    
    result = twim_init(&g_twim, CONFIG_TWIM_INSTANCE, &twim_config);
    if (result != TWIM_OK) {
        return result;
    }
    
    return 0;
}
