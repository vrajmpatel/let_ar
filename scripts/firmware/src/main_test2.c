/**
 * @file main_test2.c
 * @brief Bare minimum test - no board.c dependencies
 */

#include <stdint.h>

/* Direct GPIO access */
#define GPIO_P0_DIRSET  (*(volatile uint32_t *)(0x50000000UL + 0x518))
#define GPIO_P0_OUTSET  (*(volatile uint32_t *)(0x50000000UL + 0x508))
#define GPIO_P0_OUTCLR  (*(volatile uint32_t *)(0x50000000UL + 0x50C))
#define LED_PIN         13

static void delay(volatile uint32_t count)
{
    while (count--) __asm volatile("nop");
}

int main(void)
{
    GPIO_P0_DIRSET = (1 << LED_PIN);
    
    /* Success pattern: 2 slow blinks */
    while (1) {
        GPIO_P0_OUTCLR = (1 << LED_PIN);  /* LED on */
        delay(300000);
        GPIO_P0_OUTSET = (1 << LED_PIN);  /* LED off */
        delay(300000);
        GPIO_P0_OUTCLR = (1 << LED_PIN);
        delay(300000);
        GPIO_P0_OUTSET = (1 << LED_PIN);
        delay(1000000);
    }
    
    return 0;
}

void HardFault_Handler(void)
{
    GPIO_P0_DIRSET = (1 << LED_PIN);
    while (1) {
        GPIO_P0_OUTCLR = (1 << LED_PIN);
        delay(30000);
        GPIO_P0_OUTSET = (1 << LED_PIN);
        delay(30000);
    }
}

void NMI_Handler(void) { HardFault_Handler(); }
void MemManage_Handler(void) { HardFault_Handler(); }
void BusFault_Handler(void) { HardFault_Handler(); }
void UsageFault_Handler(void) { HardFault_Handler(); }
