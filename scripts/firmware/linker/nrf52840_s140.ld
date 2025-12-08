/**
 * @file nrf52840_s140.ld
 * @brief Linker script for nRF52840 with SoftDevice S140 6.1.1
 * 
 * Memory layout for firmware running with Nordic SoftDevice S140.
 * 
 * Citations:
 * - nRF52840_PS_v1.11.pdf: "1 MB flash and 256 KB RAM"
 * - Nordic DevZone: "S140 6.1.1 FLASH_START=0x26000, Minimum RAM Start 0x20001628"
 * - FIRMWARE_DESIGN.md Section "Memory Map":
 *   - SoftDevice: 0x00000000 - 0x00025FFF (152 KB)
 *   - Application: 0x00026000 - 0x000F3FFF (824 KB)
 *   - Bootloader: 0x000F4000 - 0x000FDFFF (40 KB)
 *   - MBR Params: 0x000FE000 - 0x000FEFFF (4 KB)
 *   - Bootloader Settings: 0x000FF000 - 0x000FFFFF (4 KB)
 * 
 * RAM Layout:
 *   - SoftDevice base RAM: 0x20000000 - 0x20001627 (~5.5 KB minimum)
 *   - Additional RAM for BLE config (connections, MTU, etc.)
 *   - Application RAM: starts after SoftDevice allocation
 *   
 * Note: The actual app RAM start is determined at runtime by sd_ble_enable()
 * based on BLE configuration. We use a conservative value here that provides
 * enough headroom for typical configurations.
 */

/* Entry point */
ENTRY(Reset_Handler)

/* Memory regions
 * Citation: Nordic DevZone - S140 6.1.1 memory requirements:
 *   - FLASH_START = 0x26000
 *   - Minimum RAM Start = 0x20001628 (without BLE configuration)
 *   
 * Citation: Adafruit Bluefruit nRF52 Feather Learning Guide:
 *   "App Ram Start must be at least 0x20004180 (provided 0x20003200).
 *    Please update linker file or re-config SoftDevice."
 *   
 * For a typical BLE peripheral configuration with S140 v6.1.1:
 *   - Base SoftDevice RAM: ~5.5 KB (0x1628 bytes)
 *   - BLE stack RAM for connections, ATT table, UUIDs: ~10-12 KB additional
 *   - Minimum required: 0x20004180 (~16.4 KB)
 *   
 * We use 0x20005000 as a safe starting point that accommodates:
 *   - Full SoftDevice RAM requirements
 *   - Headroom for BLE configuration changes (more connections, larger MTU, etc.)
 *   - 20 KB reserved for SoftDevice (0x5000 = 20,480 bytes)
 */
MEMORY
{
    /* Application Flash - after SoftDevice S140 6.1.1 (152 KB)
     * Citation: Nordic DevZone - "FLASH_START=0x26000" */
    FLASH (rx)  : ORIGIN = 0x00026000, LENGTH = 0xCA000   /* 824 KB until bootloader */
    
    /* Application RAM - must start AFTER SoftDevice RAM allocation
     * Citation: Adafruit docs - "App Ram Start must be at least 0x20004180"
     * We use 0x20005000 (20 KB offset) for safety margin.
     * Total nRF52840 RAM: 256 KB (0x40000)
     * Available to app: 256 KB - 0x5000 = 236 KB (0x3B000) */
    RAM (rwx)   : ORIGIN = 0x20005000, LENGTH = 0x3B000
}

/* Stack and heap sizes
 * Citation: ARM Cortex-M4 recommendations
 * - Stack: 8 KB for nested interrupts and BLE callbacks
 * - Heap: 4 KB for dynamic allocation (minimal use recommended)
 */
__stack_size__ = 0x2000;   /* 8 KB stack */
__heap_size__  = 0x1000;   /* 4 KB heap */

/* SoftDevice RAM boundary (for reference by application code)
 * This can be adjusted at runtime by sd_ble_enable() */
__sd_ram_end__ = ORIGIN(RAM);

SECTIONS
{
    /* Vector table must be at the start of application flash
     * Citation: ARM Cortex-M4 TRM - Vector table must be aligned to power of 2 */
    .isr_vector :
    {
        . = ALIGN(256);  /* Align to 256 bytes for Cortex-M4 */
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH
    
    /* Program code and read-only data */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        
        /* C++ constructors/destructors */
        KEEP(*(.init))
        KEEP(*(.fini))
        
        /* Constructor array */
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP(*(.preinit_array*))
        __preinit_array_end = .;
        
        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        __init_array_end = .;
        
        . = ALIGN(4);
        __fini_array_start = .;
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        __fini_array_end = .;
        
        /* Read-only data */
        *(.rodata)
        *(.rodata*)
        
        . = ALIGN(4);
        _etext = .;
    } > FLASH
    
    /* ARM exception handling tables */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH
    
    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH
    
    /* Load address for initialized data */
    _sidata = LOADADDR(.data);
    
    /* Initialized data - copied from flash to RAM at startup */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        *(.ramfunc)         /* Functions placed in RAM for performance */
        *(.ramfunc*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH
    
    /* Uninitialized data (BSS) - zeroed at startup */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > RAM
    
    /* No-init section - preserved across resets
     * Useful for keeping state across soft resets */
    .noinit (NOLOAD) :
    {
        . = ALIGN(4);
        __noinit_start__ = .;
        *(.noinit)
        *(.noinit*)
        . = ALIGN(4);
        __noinit_end__ = .;
    } > RAM
    
    /* Heap section - grows upward */
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        __heap_start__ = .;
        PROVIDE(__HeapBase = .);
        . = . + __heap_size__;
        . = ALIGN(8);
        __heap_end__ = .;
        PROVIDE(__HeapLimit = .);
    } > RAM
    
    /* Stack section - at end of RAM, grows downward
     * Citation: ARM Cortex-M4 TRM - Stack must be 8-byte aligned */
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        __stack_start__ = .;
        . = . + __stack_size__;
        . = ALIGN(8);
        __stack_end__ = .;
        __StackTop = .;
        PROVIDE(__stack = __StackTop);
    } > RAM
    
    /* Initial stack pointer - used by vector table */
    _estack = __StackTop;
    
    /* Sanity checks */
    ASSERT((__stack_end__ <= ORIGIN(RAM) + LENGTH(RAM)), "Error: Not enough RAM for stack")
    ASSERT((__heap_end__ <= __stack_start__), "Error: Heap and stack collision")
}

/* Symbols for startup code */
PROVIDE(_start = Reset_Handler);

/* End of RAM for newlib/sbrk */
PROVIDE(end = _ebss);
PROVIDE(_end = _ebss);

/* SoftDevice memory boundary symbols (for validation)
 * Citation: Adafruit docs - "App Ram Start must be at least 0x20004180" */
PROVIDE(__softdevice_flash_end__ = 0x00026000);
PROVIDE(__softdevice_ram_end__ = 0x20005000);
