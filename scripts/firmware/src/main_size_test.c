/**
 * @file main_size_test.c
 * @brief Blinky with padding to test size threshold
 * Uses same code as working blinky but with data padding
 */

#include <stdint.h>

#define GPIO_P0_BASE    0x50000000UL
#define GPIO_P0_OUTSET  (*(volatile uint32_t *)(GPIO_P0_BASE + 0x508))
#define GPIO_P0_OUTCLR  (*(volatile uint32_t *)(GPIO_P0_BASE + 0x50C))
#define GPIO_P0_DIRSET  (*(volatile uint32_t *)(GPIO_P0_BASE + 0x518))
#define GPIO_P0_PIN_CNF(n) (*(volatile uint32_t *)(GPIO_P0_BASE + 0x700 + ((n) * 4)))

#define LED_PIN         31

/* Add padding to make binary ~5KB */
static const char padding1[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0001_";
static const char padding2[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0002_";
static const char padding3[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0003_";
static const char padding4[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0004_";
static const char padding5[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0005_";
static const char padding6[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0006_";
static const char padding7[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0007_";
static const char padding8[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0008_";
static const char padding9[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0009_";
static const char padding10[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0010_";
static const char padding11[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0011_";
static const char padding12[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0012_";
static const char padding13[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0013_";
static const char padding14[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0014_";
static const char padding15[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0015_";
static const char padding16[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0016_";
static const char padding17[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0017_";
static const char padding18[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0018_";
static const char padding19[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0019_";
static const char padding20[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0020_";
static const char padding21[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0021_";
static const char padding22[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0022_";
static const char padding23[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0023_";
static const char padding24[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0024_";
static const char padding25[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0025_";
static const char padding26[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0026_";
static const char padding27[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0027_";
static const char padding28[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0028_";
static const char padding29[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0029_";
static const char padding30[] = "PADDING_DATA_TO_INCREASE_BINARY_SIZE_0030_";

volatile const char *dummy;

static void delay(volatile uint32_t count)
{
    while (count--) __asm volatile ("nop");
}

int main(void)
{
    /* Reference padding to prevent optimization */
    dummy = padding1; dummy = padding2; dummy = padding3; dummy = padding4; dummy = padding5;
    dummy = padding6; dummy = padding7; dummy = padding8; dummy = padding9; dummy = padding10;
    dummy = padding11; dummy = padding12; dummy = padding13; dummy = padding14; dummy = padding15;
    dummy = padding16; dummy = padding17; dummy = padding18; dummy = padding19; dummy = padding20;
    dummy = padding21; dummy = padding22; dummy = padding23; dummy = padding24; dummy = padding25;
    dummy = padding26; dummy = padding27; dummy = padding28; dummy = padding29; dummy = padding30;
    
    GPIO_P0_PIN_CNF(LED_PIN) = 0x03;
    GPIO_P0_DIRSET = (1UL << LED_PIN);
    
    while (1) {
        GPIO_P0_OUTSET = (1UL << LED_PIN);
        delay(500000);
        GPIO_P0_OUTCLR = (1UL << LED_PIN);
        delay(500000);
    }
    
    return 0;
}

void HardFault_Handler(void)
{
    GPIO_P0_PIN_CNF(LED_PIN) = 0x03;
    GPIO_P0_DIRSET = (1UL << LED_PIN);
    while (1) {
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
