/**
 * @file startup_minimal.s
 * @brief Minimal startup code for nRF52840 - NO SoftDevice dependency
 * 
 * This startup code is designed to work WITHOUT the SoftDevice.
 * It places the vector table at 0x26000 (after where SoftDevice would be)
 * so the UF2 bootloader can find it.
 */

    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

/*******************************************************************************
 * Vector Table - placed at 0x26000 (application start)
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
    .word Default_Handler           /* 11: SVCall Handler */
    .word Default_Handler           /* 12: Debug Monitor Handler */
    .word 0                         /* 13: Reserved */
    .word Default_Handler           /* 14: PendSV Handler */
    .word Default_Handler           /* 15: SysTick Handler */

    /* External Interrupts - all go to Default_Handler */
    .rept 48
    .word Default_Handler
    .endr

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

    /* Enable FPU (CP10 and CP11 full access) */
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

    /* Call main */
    bl main

    /* If main returns, loop forever */
    b .

    .size Reset_Handler, .-Reset_Handler

/*******************************************************************************
 * Default Interrupt Handler
 ******************************************************************************/

    .section .text.Default_Handler
    .weak Default_Handler
    .type Default_Handler, %function
Default_Handler:
Infinite_Loop:
    b Infinite_Loop
    .size Default_Handler, .-Default_Handler

/*******************************************************************************
 * Weak aliases for fault handlers
 ******************************************************************************/

    .weak      NMI_Handler
    .thumb_set NMI_Handler, Default_Handler

    .weak      HardFault_Handler
    .thumb_set HardFault_Handler, Default_Handler

    .weak      MemManage_Handler
    .thumb_set MemManage_Handler, Default_Handler

    .weak      BusFault_Handler
    .thumb_set BusFault_Handler, Default_Handler

    .weak      UsageFault_Handler
    .thumb_set UsageFault_Handler, Default_Handler

    .end
