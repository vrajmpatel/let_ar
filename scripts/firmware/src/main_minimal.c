/**
 * @file main_minimal.c
 * @brief Minimal test firmware - LED blink only (NO SoftDevice)
 * 
 * This is a minimal test to verify that the UF2 flashing works correctly.
 * It does NOT use the SoftDevice - just blinks the LED directly.
 * 
 * If this works, the problem is with SoftDevice initialization.
 * If this crashes too, the problem is with the basic startup code.
 */

#include <stdint.h>

/* GPIO Base Addresses (nRF52840_PS_v1.11.pdf Section 6.9) */
#define GPIO_P0_BASE            0x50000000UL

/* GPIO Register Offsets */
#define GPIO_OUTSET             0x508
#define GPIO_OUTCLR             0x50C
#define GPIO_DIRSET             0x518
#define GPIO_PIN_CNF(n)         (0x700 + ((n) * 4))

/* PIN_CNF values */
#define GPIO_PIN_CNF_DIR_OUTPUT     (1UL << 0)
#define GPIO_PIN_CNF_INPUT_DISCONNECT (1UL << 1)
#define GPIO_PIN_CNF_PULL_DISABLED  (0UL << 2)
#define GPIO_PIN_CNF_DRIVE_S0S1     (0UL << 8)

/* Register access */
#define REG32(addr)             (*(volatile uint32_t *)(addr))

/* LED pin on Adafruit LED Glasses Driver - P0.31 */
#define LED_PIN                 31

/**
 * @brief Simple delay loop
 */
static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm volatile ("nop");
    }
}

/**
 * @brief Main function - just blink LED
 */
int main(void)
{
    /* Configure LED pin as output */
    REG32(GPIO_P0_BASE + GPIO_PIN_CNF(LED_PIN)) = 
        GPIO_PIN_CNF_DIR_OUTPUT |
        GPIO_PIN_CNF_INPUT_DISCONNECT |
        GPIO_PIN_CNF_PULL_DISABLED |
        GPIO_PIN_CNF_DRIVE_S0S1;
    
    /* Set pin as output */
    REG32(GPIO_P0_BASE + GPIO_DIRSET) = (1UL << LED_PIN);
    
    /* Blink LED forever */
    while (1) {
        /* LED ON (active low) - clear pin */
        REG32(GPIO_P0_BASE + GPIO_OUTCLR) = (1UL << LED_PIN);
        delay(500000);  /* ~0.5 second delay */
        
        /* LED OFF - set pin high */
        REG32(GPIO_P0_BASE + GPIO_OUTSET) = (1UL << LED_PIN);
        delay(500000);
        
        /* Flash quickly 3 times to show we're running */
        for (int i = 0; i < 3; i++) {
            REG32(GPIO_P0_BASE + GPIO_OUTCLR) = (1UL << LED_PIN);
            delay(100000);
            REG32(GPIO_P0_BASE + GPIO_OUTSET) = (1UL << LED_PIN);
            delay(100000);
        }
        
        delay(1000000);  /* 1 second pause */
    }
    
    return 0;
}

/**
 * @brief Hard Fault Handler - rapid blink to indicate fault
 */
void HardFault_Handler(void)
{
    /* Configure LED pin */
    REG32(GPIO_P0_BASE + GPIO_PIN_CNF(LED_PIN)) = 
        GPIO_PIN_CNF_DIR_OUTPUT |
        GPIO_PIN_CNF_INPUT_DISCONNECT;
    REG32(GPIO_P0_BASE + GPIO_DIRSET) = (1UL << LED_PIN);
    
    while (1) {
        REG32(GPIO_P0_BASE + GPIO_OUTCLR) = (1UL << LED_PIN);
        for (volatile int i = 0; i < 50000; i++);
        REG32(GPIO_P0_BASE + GPIO_OUTSET) = (1UL << LED_PIN);
        for (volatile int i = 0; i < 50000; i++);
    }
}

void NMI_Handler(void) { HardFault_Handler(); }
void MemManage_Handler(void) { HardFault_Handler(); }
void BusFault_Handler(void) { HardFault_Handler(); }
void UsageFault_Handler(void) { HardFault_Handler(); }
