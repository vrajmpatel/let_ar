/**
 * @file main_test3.c
 * @brief Larger test with more code to test UF2 size limits
 * Uses P1.15 LED (verified working)
 */

#include <stdint.h>

#define GPIO_P1_DIRSET  (*(volatile uint32_t *)(0x50000300UL + 0x518))
#define GPIO_P1_OUTSET  (*(volatile uint32_t *)(0x50000300UL + 0x508))
#define GPIO_P1_OUTCLR  (*(volatile uint32_t *)(0x50000300UL + 0x50C))
#define LED_PIN         15

static void delay(volatile uint32_t count)
{
    while (count--) __asm volatile("nop");
}

/* Add some bulk to make the binary larger */
static const char dummy_data[] = 
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "This is padding data to make the binary larger and test UF2 transfer. "
    "END";

volatile const char *dummy_ptr;

int main(void)
{
    /* Reference the dummy data so it's not optimized out */
    dummy_ptr = dummy_data;
    
    GPIO_P1_DIRSET = (1 << LED_PIN);
    
    while (1) {
        GPIO_P1_OUTCLR = (1 << LED_PIN);  /* LED on */
        delay(300000);
        GPIO_P1_OUTSET = (1 << LED_PIN);  /* LED off */
        delay(300000);
    }
    
    return 0;
}

void HardFault_Handler(void)
{
    GPIO_P1_DIRSET = (1 << LED_PIN);
    while (1) {
        GPIO_P1_OUTCLR = (1 << LED_PIN);
        delay(30000);
        GPIO_P1_OUTSET = (1 << LED_PIN);
        delay(30000);
    }
}

void NMI_Handler(void) { HardFault_Handler(); }
void MemManage_Handler(void) { HardFault_Handler(); }
void BusFault_Handler(void) { HardFault_Handler(); }
void UsageFault_Handler(void) { HardFault_Handler(); }
