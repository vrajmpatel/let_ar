/**
 * @file nrf52840_blinky.ld
 * @brief Linker script for blinky test WITHOUT SoftDevice
 * 
 * This places code at 0x26000 to maintain compatibility with the bootloader
 * which expects the application at that address, but doesn't use SoftDevice.
 */

ENTRY(Reset_Handler)

MEMORY
{
    /* Application Flash - starts at 0x26000 for bootloader compatibility */
    FLASH (rx)  : ORIGIN = 0x00026000, LENGTH = 0xCA000
    
    /* RAM - must start at 0x20003400 to avoid MBR/bootloader reserved area
     * The Adafruit nRF52840 bootloader (with MBR) reserves RAM below 0x20003200.
     * We use 0x20003400 to provide some safety margin.
     * Available RAM: 256KB - 0x3400 = 242.5 KB */
    RAM (rwx)   : ORIGIN = 0x20003400, LENGTH = 0x3CC00
}

__stack_size__ = 0x2000;   /* 8 KB stack */

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(256);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH
    
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH
    
    _sidata = LOADADDR(.data);
    
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH
    
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
    
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        . = . + __stack_size__;
        . = ALIGN(8);
        _estack = .;
    } > RAM
}
