/**
 * @file main_gpio_test.c
 * @brief Test with just GPIO from board.c - no TWIM
 */

#include <stdint.h>

/* From board.h */
#define BOARD_LED_PIN  31
#define BOARD_LED_PORT 0

/* GPIO definitions */
#define GPIO_P0_BASE    0x50000000UL
#define GPIO_REG(base, offset) (*(volatile uint32_t *)((base) + (offset)))
#define GPIO_OUTSET     0x508
#define GPIO_OUTCLR     0x50C
#define GPIO_DIRSET     0x518
#define GPIO_PIN_CNF(n) (0x700 + ((n) * 4))

void board_gpio_output(uint8_t port, uint8_t pin)
{
    uint32_t base = (port == 0) ? GPIO_P0_BASE : 0x50000300UL;
    GPIO_REG(base, GPIO_PIN_CNF(pin)) = 1 | (1 << 1);  /* Output, disconnect input */
}

void board_gpio_set(uint8_t port, uint8_t pin)
{
    uint32_t base = (port == 0) ? GPIO_P0_BASE : 0x50000300UL;
    GPIO_REG(base, GPIO_OUTSET) = (1UL << pin);
}

void board_gpio_clear(uint8_t port, uint8_t pin)
{
    uint32_t base = (port == 0) ? GPIO_P0_BASE : 0x50000300UL;
    GPIO_REG(base, GPIO_OUTCLR) = (1UL << pin);
}

void board_led_on(void)
{
    /* LED active low */
    board_gpio_clear(BOARD_LED_PORT, BOARD_LED_PIN);
}

void board_led_off(void)
{
    board_gpio_set(BOARD_LED_PORT, BOARD_LED_PIN);
}

void board_delay_ms(uint32_t ms)
{
    while (ms--) {
        for (volatile uint32_t i = 0; i < 16000; i++) {
            __asm volatile ("nop");
        }
    }
}

int board_init(void)
{
    board_gpio_output(BOARD_LED_PORT, BOARD_LED_PIN);
    board_led_off();
    return 0;
}

int main(void)
{
    board_init();
    
    while (1) {
        board_led_on();
        board_delay_ms(500);
        board_led_off();
        board_delay_ms(500);
    }
    
    return 0;
}

void HardFault_Handler(void)
{
    GPIO_REG(GPIO_P0_BASE, GPIO_PIN_CNF(31)) = 1;
    GPIO_REG(GPIO_P0_BASE, GPIO_DIRSET) = (1UL << 31);
    while (1) {
        GPIO_REG(GPIO_P0_BASE, GPIO_OUTSET) = (1UL << 31);
        for (volatile int i = 0; i < 20000; i++) __asm volatile("nop");
        GPIO_REG(GPIO_P0_BASE, GPIO_OUTCLR) = (1UL << 31);
        for (volatile int i = 0; i < 20000; i++) __asm volatile("nop");
    }
}

void NMI_Handler(void) { HardFault_Handler(); }
void MemManage_Handler(void) { HardFault_Handler(); }
void BusFault_Handler(void) { HardFault_Handler(); }
void UsageFault_Handler(void) { HardFault_Handler(); }
