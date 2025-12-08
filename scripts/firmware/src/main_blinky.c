/**
 * @file main_blinky.c
 * @brief Minimal blinky test - no SoftDevice, just LED blink
 * 
 * This tests that the basic startup code and vector table are correct.
 * If this works, the issue is in the application code (SoftDevice init, etc.)
 * 
 * Target: Adafruit LED Glasses Driver nRF52840
 * LED Pin: P0.31 (active high)
 */

#include <stdint.h>

/* ==========================================================================
 * GPIO Register Definitions for nRF52840
 * Reference: nRF52840_PS_v1.11.pdf, Section 6.9.2, pages 324-325
 * ==========================================================================
 * P0 Base address: 0x50000000
 * P1 Base address: 0x50000300
 * 
 * Register offsets:
 *   OUT        0x504   Write GPIO port
 *   OUTSET     0x508   Set individual bits in GPIO port
 *   OUTCLR     0x50C   Clear individual bits in GPIO port
 *   DIRSET     0x518   DIR set register
 *   PIN_CNF[n] 0x700 + (n * 4)  Configuration of GPIO pins
 * ========================================================================== */

/* GPIO P0 base address - nRF52840_PS_v1.11.pdf page 324 */
#define GPIO_P0_BASE    0x50000000UL

/* GPIO registers for P0 - nRF52840_PS_v1.11.pdf page 325 */
#define GPIO_P0_OUT        (*(volatile uint32_t *)(GPIO_P0_BASE + 0x504))
#define GPIO_P0_OUTSET     (*(volatile uint32_t *)(GPIO_P0_BASE + 0x508))
#define GPIO_P0_OUTCLR     (*(volatile uint32_t *)(GPIO_P0_BASE + 0x50C))
#define GPIO_P0_DIRSET     (*(volatile uint32_t *)(GPIO_P0_BASE + 0x518))
#define GPIO_P0_PIN_CNF(n) (*(volatile uint32_t *)(GPIO_P0_BASE + 0x700 + ((n) * 4)))

/* LED pin on P0.31 - Adafruit LED Glasses Driver board */
#define LED_PIN         31

/* Simple delay loop */
static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm volatile ("nop");
    }
}

int main(void)
{
    /* Configure LED pin as output
     * PIN_CNF register bits (nRF52840_PS_v1.11.pdf page 348):
     *   Bit 0 (DIR): 1 = Output
     *   Bit 1 (INPUT): 1 = Disconnect input buffer (saves power)
     * Value: 0x03 = Output with input buffer disconnected
     */
    GPIO_P0_PIN_CNF(LED_PIN) = 0x03;
    GPIO_P0_DIRSET = (1UL << LED_PIN);
    
    /* Blink forever */
    while (1) {
        GPIO_P0_OUTSET = (1UL << LED_PIN);   /* LED on */
        delay(500000);
        GPIO_P0_OUTCLR = (1UL << LED_PIN);   /* LED off */
        delay(500000);
    }
    
    return 0;
}

/* Minimal fault handlers */
void HardFault_Handler(void)
{
    while (1) {
        /* Fast blink on fault */
        GPIO_P0_OUTSET = (1UL << LED_PIN);
        delay(50000);
        GPIO_P0_OUTCLR = (1UL << LED_PIN);
        delay(50000);
    }
}

void NMI_Handler(void) { while(1); }
void MemManage_Handler(void) { HardFault_Handler(); }
void BusFault_Handler(void) { HardFault_Handler(); }
void UsageFault_Handler(void) { HardFault_Handler(); }
