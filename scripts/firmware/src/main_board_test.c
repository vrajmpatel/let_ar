/**
 * @file main_board_test.c
 * @brief Test with just board_init - no BLE, no sensors
 * Uses the correct LED pin (P0.31) from board.h
 */

#include <stdint.h>
#include "board.h"

int main(void)
{
    int result;
    
    /* Initialize board (GPIO, LED, TWIM) */
    result = board_init();
    
    if (result != 0) {
        /* Error - fast blink using direct GPIO */
        volatile uint32_t *gpio_p0_dirset = (volatile uint32_t *)(0x50000000UL + 0x518);
        volatile uint32_t *gpio_p0_outset = (volatile uint32_t *)(0x50000000UL + 0x508);
        volatile uint32_t *gpio_p0_outclr = (volatile uint32_t *)(0x50000000UL + 0x50C);
        
        *gpio_p0_dirset = (1UL << 31);
        while (1) {
            *gpio_p0_outset = (1UL << 31);
            for (volatile int i = 0; i < 50000; i++) __asm volatile("nop");
            *gpio_p0_outclr = (1UL << 31);
            for (volatile int i = 0; i < 50000; i++) __asm volatile("nop");
        }
    }
    
    /* Success - slow blink using board functions */
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
    volatile uint32_t *gpio_p0_dirset = (volatile uint32_t *)(0x50000000UL + 0x518);
    volatile uint32_t *gpio_p0_outset = (volatile uint32_t *)(0x50000000UL + 0x508);
    volatile uint32_t *gpio_p0_outclr = (volatile uint32_t *)(0x50000000UL + 0x50C);
    
    *gpio_p0_dirset = (1UL << 31);
    /* Very fast blink = hard fault */
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
