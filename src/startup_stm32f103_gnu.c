/*
 *    \file startup_stm32f103_gnu.c
 *
 * Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 * This file is part of LispDoor.
 *
 *     LispDoor is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     LispDoor is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with LispDoor.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyrights and
 * permission notices:
 *
 *    Copyright (C) Quantum Leaps, LLC. All rights reserved.
 *
 *    This program is open source software: you can redistribute it and/or
 *    modify it under the terms of the following MIT License (MIT).
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a
 *    copy of this software and associated documentation files (the "Software"),
 *    to deal in the Software without restriction, including without limitation
 *    the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *    and/or sell copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *    DEALINGS IN THE SOFTWARE.
 *
 *    Contact information:
 *    http://www.state-machine.com
 *    mailto:info@state-machine.com
 *
 *    Copyright (c) 2011 - 2014 ARM LIMITED
 *
 *   All rights reserved.
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of ARM nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 */

/* start and end of stack defined in the linker script ---------------------*/
extern int __stack_start__;
extern int __stack_end__;

/* Weak prototypes for error handlers --------------------------------------*/
/**
 * \note
 * The function assert_failed defined at the end of this file defines
 * the error/assertion handling policy for the application and might
 * need to be customized for each project. This function is defined in
 * assembly to avoid accessing the stack, which might be corrupted by
 * the time assert_failed is called.
 */
__attribute__((naked, noreturn)) void assert_failed(char const *module,
                                                    int loc);

/* Function prototypes -----------------------------------------------------*/
void Default_Handler(void); /* Default empty handler */
void Reset_Handler(void);   /* Reset Handler */
void SystemInit(void);      /* CMSIS system initialization */

/*----------------------------------------------------------------------------
 * weak aliases for each Exception handler to the Default_Handler.
 * Any function with the same name will override these definitions.
 */
/* Cortex-M Processor fault exceptions... */
void NMI_Handler(void) __attribute__((weak));
void HardFault_Handler(void) __attribute__((weak));
void MemManage_Handler(void) __attribute__((weak));
void BusFault_Handler(void) __attribute__((weak));
void UsageFault_Handler(void) __attribute__((weak));

/* Cortex-M Processor non-fault exceptions... */
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* external interrupts...   */
void WWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TAMPER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel6_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel7_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void ADC1_2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USB_HP_CAN1_TX_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void USB_LP_CAN1_RX0_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_IRQHandler(void)
    __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USBWakeUp_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/*..........................................................................*/
__attribute__((section(".isr_vector"))) int const g_pfnVectors[] = {
    (int)&__stack_end__,      /* Top of Stack                    */
    (int)&Reset_Handler,      /* Reset Handler                   */
    (int)&NMI_Handler,        /* NMI Handler                     */
    (int)&HardFault_Handler,  /* Hard Fault Handler              */
    (int)&MemManage_Handler,  /* The MPU fault handler           */
    (int)&BusFault_Handler,   /* The bus fault handler           */
    (int)&UsageFault_Handler, /* The usage fault handler         */
    0,                        /* Reserved                        */
    0,                        /* Reserved                        */
    0,                        /* Reserved                        */
    0,                        /* Reserved                        */
    (int)&SVC_Handler,        /* SVCall handler                  */
    (int)&DebugMon_Handler,   /* Debug monitor handler           */
    0,                        /* Reserved                        */
    (int)&PendSV_Handler,     /* The PendSV handler              */
    (int)&SysTick_Handler,    /* The SysTick handler             */
};

/* reset handler -----------------------------------------------------------*/
void Reset_Handler(void) {
  extern int main(void);
  extern int __libc_init_array(void);
  extern unsigned __data_start;      /* start of .data in the linker script */
  extern unsigned __data_end__;      /* end of .data in the linker script */
  extern unsigned const __data_load; /* initialization values for .data  */
  extern unsigned __bss_start__;     /* start of .bss in the linker script */
  extern unsigned __bss_end__;       /* end of .bss in the linker script */
  extern void software_init_hook(void) __attribute__((weak));

  unsigned const *src;
  unsigned *dst;

  SystemInit(); /* CMSIS system initialization */

  /* copy the data segment initializers from flash to RAM... */
  src = &__data_load;
  for (dst = &__data_start; dst < &__data_end__; ++dst, ++src) {
    *dst = *src;
  }

  /* zero fill the .bss segment in RAM... */
  for (dst = &__bss_start__; dst < &__bss_end__; ++dst) {
    *dst = 0;
  }

  /* init hook provided? */
  if (&software_init_hook != (void (*)(void))(0)) {
    /* give control to the RTOS */
    software_init_hook(); /* this will also call __libc_init_array */
  } else {
    /* call all static constructors in C++ (harmless in C programs) */
    __libc_init_array();
    (void)main(); /* application's entry point; should never return! */
  }

  /* the previous code should not return, but assert just in case... */
  assert_failed("Reset_Handler", __LINE__);
}

/* fault exception handlers ------------------------------------------------*/
__attribute__((naked)) void NMI_Handler(void);
void NMI_Handler(void) {
  __asm volatile(
      "    ldr r0,=str_nmi\n\t"
      "    mov r1,#1\n\t"
      "    b assert_failed\n\t"
      "str_nmi: .asciz \"NMI\"\n\t"
      ".align 2\n\t");
}
/*..........................................................................*/
__attribute__((naked)) void MemManage_Handler(void);
void MemManage_Handler(void) {
  __asm volatile(
      "    ldr r0,=str_mem\n\t"
      "    mov r1,#1\n\t"
      "    b assert_failed\n\t"
      "str_mem: .asciz \"MemManage\"\n\t"
      ".align 2\n\t");
}
/*..........................................................................*/
__attribute__((naked)) void HardFault_Handler(void);
/* void HardFault_Handler(void) { */
/*   __asm volatile( */
/*       "    ldr r0,=str_hrd\n\t" */
/*       "    mov r1,#1\n\t" */
/*       "    b assert_failed\n\t" */
/*       "str_hrd: .asciz \"HardFault\"\n\t" */
/*       ".align 2\n\t"); */
/* } */

void HardFault_Handler() {
  // Hijack the process stack pointer to make backtrace work
  __asm volatile(
      "    mrs r1,psp\n\t"
      "    mov sp,r1\n\t"
      ".align 2\n\t");
}
/*..........................................................................*/
__attribute__((naked)) void BusFault_Handler(void);
void BusFault_Handler(void) {
  __asm volatile(
      "    ldr r0,=str_bus\n\t"
      "    mov r1,#1\n\t"
      "    b assert_failed\n\t"
      "str_bus: .asciz \"BusFault\"\n\t"
      ".align 2\n\t");
}
/*..........................................................................*/
__attribute__((naked)) void UsageFault_Handler(void);
void UsageFault_Handler(void) {
  __asm volatile(
      "    ldr r0,=str_usage\n\t"
      "    mov r1,#1\n\t"
      "    b assert_failed\n\t"
      "str_usage: .asciz \"UsageFault\"\n\t"
      ".align 2\n\t");
}
/*..........................................................................*/
__attribute__((naked)) void Default_Handler(void);
void Default_Handler(void) {
  __asm volatile(
      "    ldr r0,=str_dflt\n\t"
      "    mov r1,#1\n\t"
      "    b assert_failed\n\t"
      "str_dflt: .asciz \"Default\"\n\t"
      ".align 2\n\t");
}
/*..........................................................................*/
// void _init(void) { /* dummy */
//}
/*..........................................................................*/
// void _fini(void) { /* dummy */
//}

/****** End Of File *********************************************************/
