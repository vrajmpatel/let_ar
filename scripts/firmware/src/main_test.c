/**
 * @file main_test.c
 * @brief Incremental test - board init only, no SoftDevice
 * 
 * Tests board initialization without BLE to narrow down crash location.
 */

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

/* Simple blink to indicate progress */
static void blink_pattern(int count, int delay_ms)
{
    for (int i = 0; i < count; i++) {
        board_led_on();
        board_delay_ms(delay_ms);
        board_led_off();
        board_delay_ms(delay_ms);
    }
    board_delay_ms(500);
}

int main(void)
{
    int result;
    
    /* Step 1: Basic LED toggle to show we got here */
    /* Use raw GPIO first before board_init */
    volatile uint32_t *gpio_p0_dirset = (volatile uint32_t *)(0x50000000UL + 0x518);
    volatile uint32_t *gpio_p0_outset = (volatile uint32_t *)(0x50000000UL + 0x508);
    volatile uint32_t *gpio_p0_outclr = (volatile uint32_t *)(0x50000000UL + 0x50C);
    
    /* Configure P0.13 as output and blink once to show we reached main() */
    *gpio_p0_dirset = (1 << 13);
    *gpio_p0_outclr = (1 << 13);  /* LED on (active low) */
    for (volatile int i = 0; i < 500000; i++) __asm volatile("nop");
    *gpio_p0_outset = (1 << 13);  /* LED off */
    for (volatile int i = 0; i < 500000; i++) __asm volatile("nop");
    
    /* Step 2: Call board_init */
    result = board_init();
    
    if (result != 0) {
        /* Error - fast blink */
        while (1) {
            *gpio_p0_outclr = (1 << 13);
            for (volatile int i = 0; i < 50000; i++) __asm volatile("nop");
            *gpio_p0_outset = (1 << 13);
            for (volatile int i = 0; i < 50000; i++) __asm volatile("nop");
        }
    }
    
    /* Step 3: If we get here, board_init succeeded - slow blink */
    while (1) {
        blink_pattern(3, 200);  /* 3 blinks = success */
    }
    
    return 0;
}

/* Fault handlers */
void HardFault_Handler(void)
{
    volatile uint32_t *gpio_p0_dirset = (volatile uint32_t *)(0x50000000UL + 0x518);
    volatile uint32_t *gpio_p0_outset = (volatile uint32_t *)(0x50000000UL + 0x508);
    volatile uint32_t *gpio_p0_outclr = (volatile uint32_t *)(0x50000000UL + 0x50C);
    
    *gpio_p0_dirset = (1 << 13);
    
    /* Very fast blink = hard fault */
    while (1) {
        *gpio_p0_outclr = (1 << 13);
        for (volatile int i = 0; i < 20000; i++) __asm volatile("nop");
        *gpio_p0_outset = (1 << 13);
        for (volatile int i = 0; i < 20000; i++) __asm volatile("nop");
    }
}

void NMI_Handler(void) { HardFault_Handler(); }
void MemManage_Handler(void) { HardFault_Handler(); }
void BusFault_Handler(void) { HardFault_Handler(); }
void UsageFault_Handler(void) { HardFault_Handler(); }
