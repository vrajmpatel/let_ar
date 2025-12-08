/**
 * @file nrf52840.h
 * @brief nRF52840 peripheral register definitions
 * 
 * This file contains memory-mapped register definitions for the nRF52840 SoC.
 * All addresses and bit definitions are derived from nRF52840_PS_v1.11.pdf.
 * 
 * Citation: nRF52840 Product Specification v1.11
 */

#ifndef NRF52840_H
#define NRF52840_H

#include <stdint.h>

/* ============================================================================
 * Core Memory Map (nRF52840_PS_v1.11.pdf Section 4.2)
 * ============================================================================ */
#define FLASH_BASE          0x00000000UL    /* Flash memory start */
#define RAM_BASE            0x20000000UL    /* SRAM start */
#define PERIPH_BASE         0x40000000UL    /* Peripheral registers start */

/* Flash and RAM sizes (nRF52840_PS_v1.11.pdf Features) */
#define FLASH_SIZE          (1024 * 1024)   /* 1 MB Flash */
#define RAM_SIZE            (256 * 1024)    /* 256 KB RAM */

/* S140 SoftDevice v6.1.1 memory layout 
 * Citation: Bootloader INFO_UF2.TXT - "SoftDevice: S140 6.1.1"
 * S140 occupies 0x00000 to 0x25FFF (152KB) */
#define SOFTDEVICE_SIZE     0x26000         /* 152 KB reserved for SoftDevice */
#define APP_CODE_BASE       0x26000         /* Application start address */

/* ============================================================================
 * TWIM - I2C Master with EasyDMA (nRF52840_PS_v1.11.pdf Section 6.31)
 * Citation: "TWIM0 Base address 0x40003000, TWIM1 Base address 0x40004000"
 * ============================================================================ */
#define TWIM0_BASE          0x40003000UL
#define TWIM1_BASE          0x40004000UL

/* TWIM Register Offsets (nRF52840_PS_v1.11.pdf Section 6.31.7) */
#define TWIM_TASKS_STARTRX  0x000   /* Start TWI receive sequence */
#define TWIM_TASKS_STARTTX  0x008   /* Start TWI transmit sequence */
#define TWIM_TASKS_STOP     0x014   /* Stop TWI transaction */
#define TWIM_TASKS_SUSPEND  0x01C   /* Suspend TWI transaction */
#define TWIM_TASKS_RESUME   0x020   /* Resume TWI transaction */

#define TWIM_EVENTS_STOPPED     0x104   /* TWI stopped */
#define TWIM_EVENTS_ERROR       0x124   /* TWI error */
#define TWIM_EVENTS_SUSPENDED   0x148   /* SUSPEND task issued */
#define TWIM_EVENTS_RXSTARTED   0x14C   /* Receive sequence started */
#define TWIM_EVENTS_TXSTARTED   0x150   /* Transmit sequence started */
#define TWIM_EVENTS_LASTRX      0x15C   /* Last byte received */
#define TWIM_EVENTS_LASTTX      0x160   /* Last byte transmitted */

#define TWIM_SHORTS         0x200   /* Shortcuts */
#define TWIM_INTEN          0x300   /* Interrupt enable */
#define TWIM_INTENSET       0x304   /* Enable interrupt */
#define TWIM_INTENCLR       0x308   /* Disable interrupt */
#define TWIM_ERRORSRC       0x4C4   /* Error source */
#define TWIM_ENABLE         0x500   /* Enable TWIM */
#define TWIM_PSEL_SCL       0x508   /* Pin select for SCL */
#define TWIM_PSEL_SDA       0x50C   /* Pin select for SDA */
#define TWIM_FREQUENCY      0x524   /* TWI frequency */
#define TWIM_RXD_PTR        0x534   /* RXD data pointer */
#define TWIM_RXD_MAXCNT     0x538   /* Max RXD buffer size */
#define TWIM_RXD_AMOUNT     0x53C   /* Bytes transferred in RXD */
#define TWIM_RXD_LIST       0x540   /* EasyDMA list type */
#define TWIM_TXD_PTR        0x544   /* TXD data pointer */
#define TWIM_TXD_MAXCNT     0x548   /* Max TXD buffer size */
#define TWIM_TXD_AMOUNT     0x54C   /* Bytes transferred in TXD */
#define TWIM_TXD_LIST       0x550   /* EasyDMA list type */
#define TWIM_ADDRESS        0x588   /* TWI slave address */

/* TWIM_ENABLE values (nRF52840_PS_v1.11.pdf Section 6.31.7.18)
 * Citation: "Disabled=0, Enabled=6 Enable TWIM" */
#define TWIM_ENABLE_DISABLED    0
#define TWIM_ENABLE_ENABLED     6

/* TWIM_FREQUENCY values (nRF52840_PS_v1.11.pdf Section 6.31.7.21)
 * Citation: "Supported baud rates: 100, 250, 400 kbps" */
#define TWIM_FREQUENCY_K100     0x01980000UL    /* 100 kbps */
#define TWIM_FREQUENCY_K250     0x04000000UL    /* 250 kbps */
#define TWIM_FREQUENCY_K400     0x06400000UL    /* 400 kbps */

/* TWIM ERRORSRC bits (nRF52840_PS_v1.11.pdf Section 6.31.7.17) */
#define TWIM_ERRORSRC_OVERRUN   (1 << 0)    /* Overrun error */
#define TWIM_ERRORSRC_ANACK     (1 << 1)    /* NACK after address */
#define TWIM_ERRORSRC_DNACK     (1 << 2)    /* NACK after data byte */

/* TWIM SHORTS bits */
#define TWIM_SHORTS_LASTTX_STARTRX  (1 << 7)
#define TWIM_SHORTS_LASTTX_SUSPEND  (1 << 8)
#define TWIM_SHORTS_LASTTX_STOP     (1 << 9)
#define TWIM_SHORTS_LASTRX_STARTTX  (1 << 10)
#define TWIM_SHORTS_LASTRX_STOP     (1 << 12)

/* ============================================================================
 * GPIO Configuration (nRF52840_PS_v1.11.pdf Section 6.8)
 * ============================================================================ */
#define GPIO_P0_BASE        0x50000000UL
#define GPIO_P1_BASE        0x50000300UL

/* GPIO Register Offsets */
#define GPIO_OUT            0x504   /* Write GPIO port */
#define GPIO_OUTSET         0x508   /* Set individual bits in GPIO port */
#define GPIO_OUTCLR         0x50C   /* Clear individual bits in GPIO port */
#define GPIO_IN             0x510   /* Read GPIO port */
#define GPIO_DIR            0x514   /* Direction of GPIO pins */
#define GPIO_DIRSET         0x518   /* DIR set register */
#define GPIO_DIRCLR         0x51C   /* DIR clear register */
#define GPIO_PIN_CNF(n)     (0x700 + ((n) * 4))  /* Pin configuration */

/* PIN_CNF bits (nRF52840_PS_v1.11.pdf Section 6.8.2)
 * Citation: "S0D1 drive strength for I2C" */
#define GPIO_PIN_CNF_DIR_INPUT      (0 << 0)
#define GPIO_PIN_CNF_DIR_OUTPUT     (1 << 0)
#define GPIO_PIN_CNF_INPUT_CONNECT  (0 << 1)
#define GPIO_PIN_CNF_INPUT_DISCONNECT (1 << 1)
#define GPIO_PIN_CNF_PULL_DISABLED  (0 << 2)
#define GPIO_PIN_CNF_PULL_DOWN      (1 << 2)
#define GPIO_PIN_CNF_PULL_UP        (3 << 2)
#define GPIO_PIN_CNF_DRIVE_S0S1     (0 << 8)    /* Standard 0, Standard 1 */
#define GPIO_PIN_CNF_DRIVE_H0S1     (1 << 8)    /* High drive 0, Standard 1 */
#define GPIO_PIN_CNF_DRIVE_S0H1     (2 << 8)    /* Standard 0, High drive 1 */
#define GPIO_PIN_CNF_DRIVE_H0H1     (3 << 8)    /* High drive 0, High drive 1 */
#define GPIO_PIN_CNF_DRIVE_D0S1     (4 << 8)    /* Disconnect 0, Standard 1 */
#define GPIO_PIN_CNF_DRIVE_D0H1     (5 << 8)    /* Disconnect 0, High drive 1 */
#define GPIO_PIN_CNF_DRIVE_S0D1     (6 << 8)    /* Standard 0, Disconnect 1 - I2C */
#define GPIO_PIN_CNF_DRIVE_H0D1     (7 << 8)    /* High drive 0, Disconnect 1 */

/* PSEL register format - bit 31 controls connection
 * Citation: "CONNECT: Disconnected=1, Connected=0" */
#define PSEL_PIN(port, pin)     (((port) << 5) | (pin))
#define PSEL_CONNECT            (0 << 31)
#define PSEL_DISCONNECT         (1 << 31)

/* ============================================================================
 * CLOCK (nRF52840_PS_v1.11.pdf Section 6.5)
 * ============================================================================ */
#define CLOCK_BASE          0x40000000UL

#define CLOCK_TASKS_HFCLKSTART  0x000
#define CLOCK_TASKS_HFCLKSTOP   0x004
#define CLOCK_TASKS_LFCLKSTART  0x008
#define CLOCK_TASKS_LFCLKSTOP   0x00C
#define CLOCK_EVENTS_HFCLKSTARTED 0x100
#define CLOCK_EVENTS_LFCLKSTARTED 0x104
#define CLOCK_HFCLKSTAT         0x40C
#define CLOCK_LFCLKSTAT         0x418
#define CLOCK_LFCLKSRC          0x518

/* LFCLKSRC values */
#define CLOCK_LFCLKSRC_RC       0   /* Internal RC oscillator */
#define CLOCK_LFCLKSRC_XTAL     1   /* External 32.768 kHz crystal */
#define CLOCK_LFCLKSRC_SYNTH    2   /* Synthesized from HFCLK */

/* ============================================================================
 * TIMER (nRF52840_PS_v1.11.pdf Section 6.30)
 * ============================================================================ */
#define TIMER0_BASE         0x40008000UL
#define TIMER1_BASE         0x40009000UL
#define TIMER2_BASE         0x4000A000UL
#define TIMER3_BASE         0x4001A000UL
#define TIMER4_BASE         0x4001B000UL

/* TIMER Register Offsets */
#define TIMER_TASKS_START   0x000
#define TIMER_TASKS_STOP    0x004
#define TIMER_TASKS_COUNT   0x008
#define TIMER_TASKS_CLEAR   0x00C
#define TIMER_TASKS_CAPTURE(n) (0x040 + ((n) * 4))
#define TIMER_EVENTS_COMPARE(n) (0x140 + ((n) * 4))
#define TIMER_SHORTS        0x200
#define TIMER_INTENSET      0x304
#define TIMER_INTENCLR      0x308
#define TIMER_MODE          0x504
#define TIMER_BITMODE       0x508
#define TIMER_PRESCALER     0x510
#define TIMER_CC(n)         (0x540 + ((n) * 4))

/* TIMER MODE values */
#define TIMER_MODE_TIMER    0
#define TIMER_MODE_COUNTER  1

/* TIMER BITMODE values */
#define TIMER_BITMODE_16    0
#define TIMER_BITMODE_08    1
#define TIMER_BITMODE_24    2
#define TIMER_BITMODE_32    3

/* ============================================================================
 * NVIC - Nested Vectored Interrupt Controller (ARM Cortex-M4)
 * ============================================================================ */
#define NVIC_BASE           0xE000E100UL
#define NVIC_ISER(n)        (NVIC_BASE + 0x000 + ((n) * 4))  /* Interrupt Set Enable */
#define NVIC_ICER(n)        (NVIC_BASE + 0x080 + ((n) * 4))  /* Interrupt Clear Enable */
#define NVIC_ISPR(n)        (NVIC_BASE + 0x100 + ((n) * 4))  /* Interrupt Set Pending */
#define NVIC_ICPR(n)        (NVIC_BASE + 0x180 + ((n) * 4))  /* Interrupt Clear Pending */
#define NVIC_IPR(n)         (NVIC_BASE + 0x300 + ((n) * 4))  /* Interrupt Priority */

/* System Control Block */
#define SCB_BASE            0xE000ED00UL
#define SCB_VTOR            (SCB_BASE + 0x08)   /* Vector Table Offset Register */
#define SCB_AIRCR           (SCB_BASE + 0x0C)   /* Application Interrupt/Reset Control */

/* SysTick */
#define SYSTICK_BASE        0xE000E010UL
#define SYSTICK_CSR         (SYSTICK_BASE + 0x00)   /* Control and Status */
#define SYSTICK_RVR         (SYSTICK_BASE + 0x04)   /* Reload Value */
#define SYSTICK_CVR         (SYSTICK_BASE + 0x08)   /* Current Value */

/* ============================================================================
 * Peripheral Interrupt Numbers (for NVIC)
 * ============================================================================ */
#define TWIM0_TWIS0_IRQn    3
#define TWIM1_TWIS1_IRQn    4
#define TIMER0_IRQn         8
#define TIMER1_IRQn         9
#define TIMER2_IRQn         10

/* ============================================================================
 * Helper Macros for Register Access
 * ============================================================================ */
#define REG32(addr)         (*(volatile uint32_t *)(addr))
#define REG16(addr)         (*(volatile uint16_t *)(addr))
#define REG8(addr)          (*(volatile uint8_t *)(addr))

/* Data Synchronization Barrier - ensure all memory accesses complete */
#define DSB()               __asm__ volatile ("dsb" ::: "memory")

/* Instruction Synchronization Barrier */
#define ISB()               __asm__ volatile ("isb" ::: "memory")

/* Wait for interrupt */
#define WFI()               __asm__ volatile ("wfi")

/* Wait for event */
#define WFE()               __asm__ volatile ("wfe")

/* Send event */
#define SEV()               __asm__ volatile ("sev")

/* Enable interrupts */
#define ENABLE_IRQ()        __asm__ volatile ("cpsie i" ::: "memory")

/* Disable interrupts */
#define DISABLE_IRQ()       __asm__ volatile ("cpsid i" ::: "memory")

/* CMSIS-compatible intrinsic functions */
#ifndef __NOP
#define __NOP()             __asm__ volatile ("nop")
#endif

#ifndef __WFE
#define __WFE()             __asm__ volatile ("wfe")
#endif

#ifndef __disable_irq
#define __disable_irq()     __asm__ volatile ("cpsid i" ::: "memory")
#endif

#ifndef __enable_irq
#define __enable_irq()      __asm__ volatile ("cpsie i" ::: "memory")
#endif

#endif /* NRF52840_H */
