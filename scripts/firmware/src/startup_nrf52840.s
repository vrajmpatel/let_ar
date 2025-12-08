/**
 * @file startup_nrf52840.s
 * @brief Startup code for nRF52840 with SoftDevice S140
 * 
 * Vector table and reset handler for firmware running with SoftDevice.
 * 
 * SoftDevice Interrupt Forwarding:
 *   When SoftDevice is enabled, certain interrupts are handled by the
 *   SoftDevice and must be forwarded from the application's vector table.
 *   
 *   Citation: Nordic SDK - SoftDevice handlers are located in MBR/SoftDevice
 *   at fixed addresses. The application vector table forwards interrupts
 *   to these handlers using an indirect call through the SoftDevice's
 *   vector table at address 0x00001000.
 *   
 * Reserved by SoftDevice (IRQ priorities 0, 1, 4, 5):
 *   - POWER_CLOCK (0)
 *   - RADIO (1)
 *   - TIMER0 (8)
 *   - RTC0 (11)
 *   - TEMP (12)
 *   - RNG (13)
 *   - ECB (14)
 *   - CCM_AAR (15)
 *   - SWI2/EGU2 (22) - used for SoftDevice event signaling
 *   - SWI5/EGU5 (25) - used for flash operations
 *   
 * Citations:
 * - nRF52840_PS_v1.11.pdf: Interrupt vector table
 * - ARM Cortex-M4 Technical Reference Manual
 * - S140 SoftDevice Specification
 */

    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

/*******************************************************************************
 * Memory addresses
 ******************************************************************************/

/* SoftDevice vector table base address
 * Citation: Nordic SDK - SoftDevice vectors at 0x00001000 */
.equ SOFTDEVICE_VECTOR_TABLE, 0x00001000

/*******************************************************************************
 * Vector Table
 * 
 * Citation: ARM Cortex-M4 TRM - Vector table format
 * The vector table must be naturally aligned to a power of 2 whose value
 * is greater than or equal to 4 times the number of exceptions supported.
 ******************************************************************************/

    .section .isr_vector, "a", %progbits
    .align 8
    .type g_pfnVectors, %object
    .global g_pfnVectors

g_pfnVectors:
    .word _estack                   /* 0: Top of Stack */
    .word Reset_Handler             /* 1: Reset Handler */
    .word NMI_Handler               /* 2: NMI Handler */
    .word HardFault_Handler         /* 3: Hard Fault Handler */
    .word MemManage_Handler         /* 4: MPU Fault Handler */
    .word BusFault_Handler          /* 5: Bus Fault Handler */
    .word UsageFault_Handler        /* 6: Usage Fault Handler */
    .word 0                         /* 7: Reserved */
    .word 0                         /* 8: Reserved */
    .word 0                         /* 9: Reserved */
    .word 0                         /* 10: Reserved */
    .word SVC_Handler               /* 11: SVCall Handler - CRITICAL for SoftDevice */
    .word DebugMon_Handler          /* 12: Debug Monitor Handler */
    .word 0                         /* 13: Reserved */
    .word PendSV_Handler            /* 14: PendSV Handler */
    .word SysTick_Handler           /* 15: SysTick Handler */

    /* External Interrupts - nRF52840 specific
     * Interrupts marked [SD] are reserved by SoftDevice when enabled */
    .word POWER_CLOCK_IRQHandler    /* 0: Power/Clock [SD] */
    .word RADIO_IRQHandler          /* 1: Radio [SD] */
    .word UARTE0_UART0_IRQHandler   /* 2: UART0 */
    .word SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler  /* 3: SPI0/TWI0 */
    .word SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler  /* 4: SPI1/TWI1 */
    .word NFCT_IRQHandler           /* 5: NFCT */
    .word GPIOTE_IRQHandler         /* 6: GPIOTE */
    .word SAADC_IRQHandler          /* 7: SAADC */
    .word TIMER0_IRQHandler         /* 8: Timer 0 [SD] */
    .word TIMER1_IRQHandler         /* 9: Timer 1 */
    .word TIMER2_IRQHandler         /* 10: Timer 2 */
    .word RTC0_IRQHandler           /* 11: RTC 0 [SD] */
    .word TEMP_IRQHandler           /* 12: Temperature [SD] */
    .word RNG_IRQHandler            /* 13: RNG [SD] */
    .word ECB_IRQHandler            /* 14: ECB [SD] */
    .word CCM_AAR_IRQHandler        /* 15: CCM/AAR [SD] */
    .word WDT_IRQHandler            /* 16: Watchdog */
    .word RTC1_IRQHandler           /* 17: RTC 1 */
    .word QDEC_IRQHandler           /* 18: QDEC */
    .word COMP_LPCOMP_IRQHandler    /* 19: COMP/LPCOMP */
    .word SWI0_EGU0_IRQHandler      /* 20: SWI0/EGU0 */
    .word SWI1_EGU1_IRQHandler      /* 21: SWI1/EGU1 */
    .word SWI2_EGU2_IRQHandler      /* 22: SWI2/EGU2 [SD - Events] */
    .word SWI3_EGU3_IRQHandler      /* 23: SWI3/EGU3 */
    .word SWI4_EGU4_IRQHandler      /* 24: SWI4/EGU4 */
    .word SWI5_EGU5_IRQHandler      /* 25: SWI5/EGU5 [SD - Flash] */
    .word TIMER3_IRQHandler         /* 26: Timer 3 */
    .word TIMER4_IRQHandler         /* 27: Timer 4 */
    .word PWM0_IRQHandler           /* 28: PWM 0 */
    .word PDM_IRQHandler            /* 29: PDM */
    .word 0                         /* 30: Reserved */
    .word 0                         /* 31: Reserved */
    .word MWU_IRQHandler            /* 32: MWU */
    .word PWM1_IRQHandler           /* 33: PWM 1 */
    .word PWM2_IRQHandler           /* 34: PWM 2 */
    .word SPIM2_SPIS2_SPI2_IRQHandler /* 35: SPI2 */
    .word RTC2_IRQHandler           /* 36: RTC 2 */
    .word I2S_IRQHandler            /* 37: I2S */
    .word FPU_IRQHandler            /* 38: FPU */
    .word USBD_IRQHandler           /* 39: USBD */
    .word UARTE1_IRQHandler         /* 40: UART1 */
    .word QSPI_IRQHandler           /* 41: QSPI */
    .word CRYPTOCELL_IRQHandler     /* 42: Cryptocell */
    .word 0                         /* 43: Reserved */
    .word 0                         /* 44: Reserved */
    .word PWM3_IRQHandler           /* 45: PWM 3 */
    .word 0                         /* 46: Reserved */
    .word SPIM3_IRQHandler          /* 47: SPI3 */

    .size g_pfnVectors, .-g_pfnVectors

/*******************************************************************************
 * Reset Handler
 ******************************************************************************/

    .section .text.Reset_Handler
    .weak Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    /* Set stack pointer */
    ldr r0, =_estack
    mov sp, r0

    /* Enable FPU (CP10 and CP11 full access)
     * Citation: ARM Cortex-M4 TRM - CPACR register (0xE000ED88)
     * Bits [23:20] must be set to 0xF for full access to CP10 and CP11
     */
    ldr r0, =0xE000ED88
    ldr r1, [r0]
    orr r1, r1, #(0xF << 20)
    str r1, [r0]
    dsb
    isb

    /* Copy .data section from flash to RAM */
    ldr r0, =_sdata       /* Destination start */
    ldr r1, =_edata       /* Destination end */
    ldr r2, =_sidata      /* Source start */
    
    movs r3, #0
    b LoopCopyDataInit

CopyDataInit:
    ldr r4, [r2, r3]
    str r4, [r0, r3]
    adds r3, r3, #4

LoopCopyDataInit:
    adds r4, r0, r3
    cmp r4, r1
    bcc CopyDataInit

    /* Zero fill .bss section */
    ldr r2, =_sbss        /* Start of BSS */
    ldr r4, =_ebss        /* End of BSS */
    movs r3, #0
    b LoopFillZerobss

FillZerobss:
    str r3, [r2]
    adds r2, r2, #4

LoopFillZerobss:
    cmp r2, r4
    bcc FillZerobss

    /* Call system initialization (if defined) */
    bl SystemInit

    /* Call main */
    bl main

    /* If main returns, loop forever */
    b .

    .size Reset_Handler, .-Reset_Handler

/*******************************************************************************
 * System Initialization (weak, can be overridden)
 ******************************************************************************/

    .section .text.SystemInit
    .weak SystemInit
    .type SystemInit, %function
SystemInit:
    bx lr
    .size SystemInit, .-SystemInit

/*******************************************************************************
 * SVC Handler - CRITICAL for SoftDevice
 * 
 * Citation: Nordic SDK - SVC handler must check if the call is for
 * the SoftDevice (SVC number >= 0x10) and forward appropriately.
 * 
 * When SoftDevice is enabled, SVC instructions are used for all API calls.
 * The application must forward SVC calls to the SoftDevice's handler.
 ******************************************************************************/

    .section .text.SVC_Handler
    .weak SVC_Handler
    .type SVC_Handler, %function
SVC_Handler:
    /* Get the SVC number from the stacked PC
     * Citation: ARM Cortex-M4 TRM - On exception entry, LR contains
     * special EXC_RETURN value indicating which stack was used */
    
    /* Check EXC_RETURN to determine which stack pointer to use */
    tst lr, #4
    ite eq
    mrseq r0, msp          /* Main stack pointer */
    mrsne r0, psp          /* Process stack pointer */
    
    /* Get stacked PC (offset 24 from stack frame start) */
    ldr r1, [r0, #24]
    
    /* Get SVC instruction (2 bytes before stacked PC) */
    ldrb r0, [r1, #-2]
    
    /* SoftDevice SVC numbers are >= 0x10
     * Citation: Nordic SDK - Application SVCs are < 0x10 */
    cmp r0, #0x10
    bhs forward_to_sd
    
    /* Application SVC - handle here or use default */
    b Default_Handler

forward_to_sd:
    /* Forward to SoftDevice SVC handler
     * Citation: Nordic SDK - SoftDevice SVC handler is at 
     * SOFTDEVICE_VECTOR_TABLE + (11 * 4) = 0x0000102C */
    ldr r0, =SOFTDEVICE_VECTOR_TABLE
    ldr r0, [r0, #(11*4)]   /* Vector table offset for SVC (entry 11) */
    bx r0
    
    .size SVC_Handler, .-SVC_Handler

/*******************************************************************************
 * SWI2/EGU2 Handler - SoftDevice Event Handler
 * 
 * Citation: Nordic SDK - SoftDevice uses SWI2/EGU2 to signal that
 * events are ready. This interrupt should wake the application
 * from low-power sleep to process BLE events.
 ******************************************************************************/

    .section .text.SWI2_EGU2_IRQHandler
    .weak SWI2_EGU2_IRQHandler
    .type SWI2_EGU2_IRQHandler, %function
SWI2_EGU2_IRQHandler:
    /* This is triggered by SoftDevice when BLE events are pending.
     * The actual event processing is done by calling sd_ble_evt_get()
     * in the main loop, not in this ISR. */
    
    /* Simply return - the main loop will pick up the event */
    bx lr
    
    .size SWI2_EGU2_IRQHandler, .-SWI2_EGU2_IRQHandler

/*******************************************************************************
 * Default Interrupt Handler (weak)
 ******************************************************************************/

    .section .text.Default_Handler
    .weak Default_Handler
    .type Default_Handler, %function
Default_Handler:
Infinite_Loop:
    b Infinite_Loop
    .size Default_Handler, .-Default_Handler

/*******************************************************************************
 * Hard Fault Handler with debugging support
 ******************************************************************************/

    .section .text.HardFault_Handler
    .weak HardFault_Handler
    .type HardFault_Handler, %function
HardFault_Handler:
    /* Get the appropriate stack pointer */
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    
    /* Store fault information for debugging */
    /* R0 now points to the stack frame with:
     * R0, R1, R2, R3, R12, LR, PC, xPSR */
    
    /* Call C handler if defined */
    bl HardFault_Handler_C
    
    /* If C handler returns, loop forever */
    b .
    
    .size HardFault_Handler, .-HardFault_Handler

/*******************************************************************************
 * Weak aliases for all handlers
 ******************************************************************************/

    .weak      NMI_Handler
    .thumb_set NMI_Handler, Default_Handler

    /* Note: HardFault_Handler defined above, not aliased */

    .weak      MemManage_Handler
    .thumb_set MemManage_Handler, Default_Handler

    .weak      BusFault_Handler
    .thumb_set BusFault_Handler, Default_Handler

    .weak      UsageFault_Handler
    .thumb_set UsageFault_Handler, Default_Handler

    /* Note: SVC_Handler defined above for SoftDevice forwarding */

    .weak      DebugMon_Handler
    .thumb_set DebugMon_Handler, Default_Handler

    .weak      PendSV_Handler
    .thumb_set PendSV_Handler, Default_Handler

    .weak      SysTick_Handler
    .thumb_set SysTick_Handler, Default_Handler

    /* External interrupt handlers */
    .weak      POWER_CLOCK_IRQHandler
    .thumb_set POWER_CLOCK_IRQHandler, Default_Handler

    .weak      RADIO_IRQHandler
    .thumb_set RADIO_IRQHandler, Default_Handler

    .weak      UARTE0_UART0_IRQHandler
    .thumb_set UARTE0_UART0_IRQHandler, Default_Handler

    .weak      SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler
    .thumb_set SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler, Default_Handler

    .weak      SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler
    .thumb_set SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler, Default_Handler

    .weak      NFCT_IRQHandler
    .thumb_set NFCT_IRQHandler, Default_Handler

    .weak      GPIOTE_IRQHandler
    .thumb_set GPIOTE_IRQHandler, Default_Handler

    .weak      SAADC_IRQHandler
    .thumb_set SAADC_IRQHandler, Default_Handler

    .weak      TIMER0_IRQHandler
    .thumb_set TIMER0_IRQHandler, Default_Handler

    .weak      TIMER1_IRQHandler
    .thumb_set TIMER1_IRQHandler, Default_Handler

    .weak      TIMER2_IRQHandler
    .thumb_set TIMER2_IRQHandler, Default_Handler

    .weak      RTC0_IRQHandler
    .thumb_set RTC0_IRQHandler, Default_Handler

    .weak      TEMP_IRQHandler
    .thumb_set TEMP_IRQHandler, Default_Handler

    .weak      RNG_IRQHandler
    .thumb_set RNG_IRQHandler, Default_Handler

    .weak      ECB_IRQHandler
    .thumb_set ECB_IRQHandler, Default_Handler

    .weak      CCM_AAR_IRQHandler
    .thumb_set CCM_AAR_IRQHandler, Default_Handler

    .weak      WDT_IRQHandler
    .thumb_set WDT_IRQHandler, Default_Handler

    .weak      RTC1_IRQHandler
    .thumb_set RTC1_IRQHandler, Default_Handler

    .weak      QDEC_IRQHandler
    .thumb_set QDEC_IRQHandler, Default_Handler

    .weak      COMP_LPCOMP_IRQHandler
    .thumb_set COMP_LPCOMP_IRQHandler, Default_Handler

    .weak      SWI0_EGU0_IRQHandler
    .thumb_set SWI0_EGU0_IRQHandler, Default_Handler

    .weak      SWI1_EGU1_IRQHandler
    .thumb_set SWI1_EGU1_IRQHandler, Default_Handler

    /* Note: SWI2_EGU2_IRQHandler defined above for SoftDevice events */

    .weak      SWI3_EGU3_IRQHandler
    .thumb_set SWI3_EGU3_IRQHandler, Default_Handler

    .weak      SWI4_EGU4_IRQHandler
    .thumb_set SWI4_EGU4_IRQHandler, Default_Handler

    .weak      SWI5_EGU5_IRQHandler
    .thumb_set SWI5_EGU5_IRQHandler, Default_Handler

    .weak      TIMER3_IRQHandler
    .thumb_set TIMER3_IRQHandler, Default_Handler

    .weak      TIMER4_IRQHandler
    .thumb_set TIMER4_IRQHandler, Default_Handler

    .weak      PWM0_IRQHandler
    .thumb_set PWM0_IRQHandler, Default_Handler

    .weak      PDM_IRQHandler
    .thumb_set PDM_IRQHandler, Default_Handler

    .weak      MWU_IRQHandler
    .thumb_set MWU_IRQHandler, Default_Handler

    .weak      PWM1_IRQHandler
    .thumb_set PWM1_IRQHandler, Default_Handler

    .weak      PWM2_IRQHandler
    .thumb_set PWM2_IRQHandler, Default_Handler

    .weak      SPIM2_SPIS2_SPI2_IRQHandler
    .thumb_set SPIM2_SPIS2_SPI2_IRQHandler, Default_Handler

    .weak      RTC2_IRQHandler
    .thumb_set RTC2_IRQHandler, Default_Handler

    .weak      I2S_IRQHandler
    .thumb_set I2S_IRQHandler, Default_Handler

    .weak      FPU_IRQHandler
    .thumb_set FPU_IRQHandler, Default_Handler

    .weak      USBD_IRQHandler
    .thumb_set USBD_IRQHandler, Default_Handler

    .weak      UARTE1_IRQHandler
    .thumb_set UARTE1_IRQHandler, Default_Handler

    .weak      QSPI_IRQHandler
    .thumb_set QSPI_IRQHandler, Default_Handler

    .weak      CRYPTOCELL_IRQHandler
    .thumb_set CRYPTOCELL_IRQHandler, Default_Handler

    .weak      PWM3_IRQHandler
    .thumb_set PWM3_IRQHandler, Default_Handler

    .weak      SPIM3_IRQHandler
    .thumb_set SPIM3_IRQHandler, Default_Handler

    /* Weak C handler for hard faults */
    .weak      HardFault_Handler_C
    .thumb_set HardFault_Handler_C, Default_Handler

    .end
