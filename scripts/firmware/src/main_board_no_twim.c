/**
 * @file main_board_no_twim.c
 * @brief Test with real board.c but NO twim initialization
 */

#include <stdint.h>
#include "board.h"

/* Override board_init to skip TWIM */
int board_init_no_twim(void);

int main(void)
{
    board_init_no_twim();
    
    while (1) {
        board_led_on();
        board_delay_ms(500);
        board_led_off();
        board_delay_ms(500);
    }
    
    return 0;
}

/* Custom board init that skips TWIM */
int board_init_no_twim(void)
{
    /* Configure LED pin as output (off) */
    board_gpio_output(BOARD_LED_PORT, BOARD_LED_PIN);
    board_led_off();
    
    /* Configure button pin as input with pull-up (if present) */
    board_gpio_input(BOARD_BUTTON_PORT, BOARD_BUTTON_PIN, 3);
    
    /* Skip TWIM init entirely */
    return 0;
}

void HardFault_Handler(void)
{
    volatile uint32_t *gpio_p0_dirset = (volatile uint32_t *)(0x50000000UL + 0x518);
    volatile uint32_t *gpio_p0_outset = (volatile uint32_t *)(0x50000000UL + 0x508);
    volatile uint32_t *gpio_p0_outclr = (volatile uint32_t *)(0x50000000UL + 0x50C);
    
    *gpio_p0_dirset = (1UL << 31);
    while (1) {
        *gpio_p0_outset = (1UL << 31);
        for (volatile int i = 0; i < 20000; i++) __asm volatile("nop");
        *gpio_p0_outclr = (1UL << 31);
        for (volatile int i = 0; i < 20000; i++) __asm volatile("nop");
    }
}

void NMI_Handler(void) { HardFault_Handler(); }
void MemManage_Handler(void) { HardFault_Handler(); }
void BusFault_Handler(void) { HardFault_Handler(); }
void UsageFault_Handler(void) { HardFault_Handler(); }
