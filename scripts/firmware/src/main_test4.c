/**
 * @file main_test4.c
 * @brief Test with board_init - uses P1.15 LED
 */

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

int main(void)
{
    int result;
    
    /* First blink using direct GPIO to confirm we reached main */
    volatile uint32_t *gpio_p1_dirset = (volatile uint32_t *)(0x50000300UL + 0x518);
    volatile uint32_t *gpio_p1_outset = (volatile uint32_t *)(0x50000300UL + 0x508);
    volatile uint32_t *gpio_p1_outclr = (volatile uint32_t *)(0x50000300UL + 0x50C);
    
    *gpio_p1_dirset = (1 << 15);
    
    /* One quick blink to show we got to main */
    *gpio_p1_outclr = (1 << 15);
    for (volatile int i = 0; i < 200000; i++) __asm volatile("nop");
    *gpio_p1_outset = (1 << 15);
    for (volatile int i = 0; i < 200000; i++) __asm volatile("nop");
    
    /* Call board_init */
    result = board_init();
    
    if (result != 0) {
        /* Error - 2 fast blinks repeating */
        while (1) {
            *gpio_p1_outclr = (1 << 15);
            for (volatile int i = 0; i < 50000; i++) __asm volatile("nop");
            *gpio_p1_outset = (1 << 15);
            for (volatile int i = 0; i < 50000; i++) __asm volatile("nop");
            *gpio_p1_outclr = (1 << 15);
            for (volatile int i = 0; i < 50000; i++) __asm volatile("nop");
            *gpio_p1_outset = (1 << 15);
            for (volatile int i = 0; i < 300000; i++) __asm volatile("nop");
        }
    }
    
    /* Success - 3 slow blinks repeating */
    while (1) {
        board_led_on();
        board_delay_ms(300);
        board_led_off();
        board_delay_ms(300);
        board_led_on();
        board_delay_ms(300);
        board_led_off();
        board_delay_ms(300);
        board_led_on();
        board_delay_ms(300);
        board_led_off();
        board_delay_ms(1000);
    }
    
    return 0;
}

void HardFault_Handler(void)
{
    volatile uint32_t *gpio_p1_dirset = (volatile uint32_t *)(0x50000300UL + 0x518);
    volatile uint32_t *gpio_p1_outclr = (volatile uint32_t *)(0x50000300UL + 0x50C);
    volatile uint32_t *gpio_p1_outset = (volatile uint32_t *)(0x50000300UL + 0x508);
    
    *gpio_p1_dirset = (1 << 15);
    
    /* Very fast blink = hard fault */
    while (1) {
        *gpio_p1_outclr = (1 << 15);
        for (volatile int i = 0; i < 20000; i++) __asm volatile("nop");
        *gpio_p1_outset = (1 << 15);
        for (volatile int i = 0; i < 20000; i++) __asm volatile("nop");
    }
}

void NMI_Handler(void) { HardFault_Handler(); }
void MemManage_Handler(void) { HardFault_Handler(); }
void BusFault_Handler(void) { HardFault_Handler(); }
void UsageFault_Handler(void) { HardFault_Handler(); }
