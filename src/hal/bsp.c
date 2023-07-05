/*
 *    \file bsp.c
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
 * This file incorporates work covered by the following copyright and
 * permission notice:
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

#include "hal/bsp.h"

#include "hal/qassert.h"
#include "lispdoor/memorylayout.h"
#include "stm32f1xx_hal.h" /* TODO: USE CMSIS */

#define BootRAM (int)0xF108F85F

/*******************************************************************************
 *  Clock Definitions
 *******************************************************************************/
uint32_t SystemCoreClock =
    72000000U; /*!< System Clock Frequency (Core Clock) */
static UART_HandleTypeDef huart1;

const uint8_t AHBPrescTable[16U] = {0, 0, 0, 0, 0, 0, 0, 0,
                                    1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8U] = {0, 0, 0, 0, 1, 2, 3, 4};

void USART1_IRQHandler();

void IrqOn() {
  /* __HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE); */
  __enable_irq();
}
void IrqOff() { __disable_irq(); }
void BspInit() {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();
  SystemClock_Config();
  GPIO_Init();

  /* To not be optimized */
  HAL_UART_Receive_IT(&huart1, &terminal_buffer[terminal_buffer_insert_index],
                      1);
  USART1_IRQHandler();
  /* __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE); */
  __enable_irq();
}
/*  */

/* end of stack defined in the linker script ---------------------*/
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
/* __attribute__((naked, noreturn)) */ Q_NORETURN assert_failed(
    char const *module, int loc);

/* Function prototypes -----------------------------------------------------*/
void Default_Handler(void); /* Default empty handler */
void Reset_Handler(void);   /* Reset Handler */
void SystemInit(void);      /* CMSIS system initialization */

/*----------------------------------------------------------------------------
 * weak aliases for each Exception handler to the Default_Handler.
 * Any function with the same name will override these definitions.
 */
/* Cortex-M Processor fault exceptions... */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);

/* Cortex-M Processor non-fault exceptions... */
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/* external interrupts...   */
void WWDG_IRQHandler(void);
void PVD_IRQHandler(void);
void TAMPER_IRQHandler(void);
void RTC_IRQHandler(void);
void FLASH_IRQHandler(void);
void RCC_IRQHandler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);
void ADC1_2_IRQHandler(void);
void USB_HP_CAN1_TX_IRQHandler(void);
void USB_LP_CAN1_RX0_IRQHandler(void);
void CAN1_RX1_IRQHandler(void);
void CAN1_SCE_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void TIM1_BRK_IRQHandler(void);
void TIM1_UP_IRQHandler(void);
void TIM1_TRG_COM_IRQHandler(void);
void TIM1_CC_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void I2C2_EV_IRQHandler(void);
void I2C2_ER_IRQHandler(void);
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void RTC_Alarm_IRQHandler(void);
void USBWakeUp_IRQHandler(void);
__attribute__((section(".my_vector"))) int vector_table[] = {
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

    /*IRQ handlers... */
    (int)&WWDG_IRQHandler,            /* The SysTick handler             */
    (int)&PVD_IRQHandler,             /* The SysTick handler             */
    (int)&TAMPER_IRQHandler,          /* The SysTick handler             */
    (int)&RTC_IRQHandler,             /* The SysTick handler             */
    (int)&FLASH_IRQHandler,           /* The SysTick handler             */
    (int)&RCC_IRQHandler,             /* The SysTick handler             */
    (int)&EXTI0_IRQHandler,           /* The SysTick handler             */
    (int)&EXTI1_IRQHandler,           /* The SysTick handler             */
    (int)&EXTI2_IRQHandler,           /* The SysTick handler             */
    (int)&EXTI3_IRQHandler,           /* The SysTick handler             */
    (int)&EXTI4_IRQHandler,           /* The SysTick handler             */
    (int)&DMA1_Channel1_IRQHandler,   /* The SysTick handler             */
    (int)&DMA1_Channel2_IRQHandler,   /* The SysTick handler             */
    (int)&DMA1_Channel3_IRQHandler,   /* The SysTick handler             */
    (int)&DMA1_Channel4_IRQHandler,   /* The SysTick handler             */
    (int)&DMA1_Channel5_IRQHandler,   /* The SysTick handler             */
    (int)&DMA1_Channel6_IRQHandler,   /* The SysTick handler             */
    (int)&DMA1_Channel7_IRQHandler,   /* The SysTick handler             */
    (int)&ADC1_2_IRQHandler,          /* The SysTick handler             */
    (int)&USB_HP_CAN1_TX_IRQHandler,  /* The SysTick handler             */
    (int)&USB_LP_CAN1_RX0_IRQHandler, /* The SysTick handler             */
    (int)&CAN1_RX1_IRQHandler,        /* The SysTick handler             */
    (int)&CAN1_SCE_IRQHandler,        /* The SysTick handler             */
    (int)&EXTI9_5_IRQHandler,         /* The SysTick handler             */
    (int)&TIM1_BRK_IRQHandler,        /* The SysTick handler             */
    (int)&TIM1_UP_IRQHandler,         /* The SysTick handler             */
    (int)&TIM1_TRG_COM_IRQHandler,    /* The SysTick handler             */
    (int)&TIM1_CC_IRQHandler,         /* The SysTick handler             */
    (int)&TIM2_IRQHandler,            /* The SysTick handler             */
    (int)&TIM3_IRQHandler,            /* The SysTick handler             */
    (int)&TIM4_IRQHandler,            /* The SysTick handler             */
    (int)&I2C1_EV_IRQHandler,         /* The SysTick handler             */
    (int)&I2C1_ER_IRQHandler,         /* The SysTick handler             */
    (int)&I2C2_EV_IRQHandler,         /* The SysTick handler             */
    (int)&I2C2_ER_IRQHandler,         /* The SysTick handler             */
    (int)&SPI1_IRQHandler,            /* The SysTick handler             */
    (int)&SPI2_IRQHandler,            /* The SysTick handler             */
    (int)&USART1_IRQHandler,          /* The SysTick handler             */
    (int)&USART2_IRQHandler,          /* The SysTick handler             */
    (int)&USART3_IRQHandler,          /* The SysTick handler             */
    (int)&EXTI15_10_IRQHandler,       /* The SysTick handler             */
    (int)&RTC_Alarm_IRQHandler,       /* The SysTick handler             */
    (int)&USBWakeUp_IRQHandler,       /* The SysTick handler             */
    0,                                /* Reserved                        */
    0,                                /* Reserved                        */
    0,                                /* Reserved                        */
    0,                                /* Reserved                        */
    0,                                /* Reserved                        */
    0,                                /* Reserved                        */
    0,                                /* Reserved                        */
    BootRAM                           /* @0x108. This is for boot in RAM
                                            mode for STM32F10x Medium
                                                        Density devices. */
};
/**
 * @brief  Setup the microcontroller system
 *         Initialize the Embedded Flash Interface, the PLL and update the
 *         SystemCoreClock variable.
 * @note   This function should be used only after reset.
 * @param  None
 * @retval None
 */
void SystemInit(void) {
  /* Reset the RCC clock configuration to the default reset state(for debug
   * purpose) */
  /* Set HSION bit */
  RCC->CR |= RCC_HSI_ON;
  /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
  RCC->CFGR &= 0xF8FF0000U;
  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= 0xFEF6FFFFU;
  /* Reset HSEBYP bit */
  RCC->CR &= 0xFFFBFFFFU;
  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
  RCC->CFGR &= 0xFF80FFFFU;
  /* Disable all interrupts and clear pending bits  */
  RCC->CIR = 0x009F0000U;

  /* Vector Table Relocation in Internal SRAM. */
  SCB->VTOR = (uint32_t)vector_table;
}

/**
 * @brief  Update SystemCoreClock variable according to Clock Register Values.
 *         The SystemCoreClock variable contains the core clock (HCLK), it can
 *         be used by the user application to setup the SysTick timer or
 * configure other parameters.
 *
 * @note   Each time the core clock (HCLK) changes, this function must be called
 *         to update SystemCoreClock variable value. Otherwise, any
 * configuration based on this variable will be incorrect.
 *
 * @note   - The system frequency computed by this function is not the real
 *           frequency in the chip. It is calculated based on the predefined
 *           constant and the selected clock source:
 *
 *           - If SYSCLK source is HSI, SystemCoreClock will contain the
 * HSI_VALUE(*)
 *
 *           - If SYSCLK source is HSE, SystemCoreClock will contain the
 * HSE_VALUE(**)
 *
 *           - If SYSCLK source is PLL, SystemCoreClock will contain the
 * HSE_VALUE(**) or HSI_VALUE(*) multiplied by the PLL factors.
 *
 *         (*) HSI_VALUE is a constant defined in stm32f1xx.h file (default
 * value 8 MHz) but the real value may vary depending on the variations in
 * voltage and temperature.
 *
 *         (**) HSE_VALUE is a constant defined in stm32f1xx.h file (default
 * value 8 MHz or 25 MHz, depending on the product used), user has to ensure
 *              that HSE_VALUE is same as the real frequency of the crystal
 * used. Otherwise, this function may have wrong result.
 *
 *         - The result of this function could be not correct when using
 * fractional value for HSE crystal.
 * @param  None
 * @retval None
 */
void SystemCoreClockUpdate(void) {
  uint32_t tmp = 0U, pllmull = 0U, pllsource = 0U;

  /* Get SYSCLK source -------------------------------------------------------*/
  tmp = RCC->CFGR & RCC_CFGR_SWS;

  switch (tmp) {
    case 0x00U: /* HSI used as system clock */
      SystemCoreClock = HSI_VALUE;
      break;
    case 0x04U: /* HSE used as system clock */
      SystemCoreClock = HSE_VALUE;
      break;
    case 0x08U: /* PLL used as system clock */

      /* Get PLL clock source and multiplication factor ----------------------*/
      pllmull = RCC->CFGR & RCC_CFGR_PLLMULL;
      pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
      pllmull = (pllmull >> 18U) + 2U;

      if (pllsource == 0x00U) {
        /* HSI oscillator clock divided by 2 selected as PLL clock entry */
        SystemCoreClock = (HSI_VALUE >> 1U) * pllmull;
      } else {
        /* HSE selected as PLL clock entry */
        if ((RCC->CFGR & RCC_CFGR_PLLXTPRE) !=
            (uint32_t)RESET) { /* HSE oscillator clock divided by 2 */
          SystemCoreClock = (HSE_VALUE >> 1U) * pllmull;
        } else {
          SystemCoreClock = HSE_VALUE * pllmull;
        }
      }
      break;

    default:
      SystemCoreClock = HSI_VALUE;
      break;
  }
  /* Compute HCLK clock frequency ----------------*/
  /* Get HCLK prescaler */
  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4U)];
  /* HCLK clock frequency */
  SystemCoreClock >>= tmp;
}

/** System Clock Configuration
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  /**Initializes the CPU, AHB and APB busses clocks
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Initializes the CPU, AHB and APB busses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure the Systick interrupt time
   * 1 ms
   */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

  /**Configure the Systick
   */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/** Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
void GPIO_Init() {
  /* GPIO_InitTypeDef GPIO_InitStruct; */
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  UART1_Init();
}

/* USART1 init function */
void UART1_Init() {
  __HAL_RCC_USART1_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct;

  /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
  /* Peripheral interrupt init*/
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

  /* Initialize uart buffer */
  terminal_buffer_get_index = terminal_buffer_insert_index = 0;
}

/* TODO: Check for errors */
void UART1_SendStr(char *s) {
  HAL_UART_Transmit(&huart1, (uint8_t *)s, (uint16_t)strlen(s), 10);
  /* __HAL_UART_FLUSH_DRREGISTER(&huart1); */
}
/* TODO: Check for errors */
void UART1_SendStrN(char *s, uint16_t len) {
  HAL_UART_Transmit(&huart1, (uint8_t *)s, len, 10);
  /* __HAL_UART_FLUSH_DRREGISTER(&huart1); */
}
void UART1_SendByte(uint8_t c) {
  HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, 10);
  /* __HAL_UART_FLUSH_DRREGISTER(&huart1); */
}

void SysTick_Handler(void) {
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

void USART1_IRQHandler(void) {
  /* terminal_buffer[terminal_buffer_insert_index] = USART1->DR; */
  /* terminal_buffer_insert_index += 1; */
  /* terminal_buffer_insert_index &= (TIB_SIZE - 1); */
  HAL_UART_IRQHandler(&huart1);
}

/* This callback is called by the HAL_UART_IRQHandler when the given number of
 * bytes are received */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    terminal_buffer_insert_index += 1;
    terminal_buffer_insert_index &= (TIB_SIZE - 1);
    /* needed to fix locking huart1 while transmitting */
    __HAL_UNLOCK(&huart1);
    /* Receive one byte in interrupt mode */
    HAL_UART_Receive_IT(&huart1, &terminal_buffer[terminal_buffer_insert_index],
                        1);
  }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
/* void _Error_Handler(char *file, int line) { */
void _Error_Handler(char *file, int line) {
  (void)file;
  (void)line;
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state
   */
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

/**
 * @brief      function description
 *
 * @details    detailed description
 *
 * @param      param
 *
 * @return     return type
 */
Q_NORETURN assert_failed(char const *file, int line) { Q_onAssert(file, line); }

Q_NORETURN Q_onAssert(char const *const module, int_t const id) {
  (void)module;
  (void)id;
  while (true) {
  }
  /* TBD: damage control */
  NVIC_SystemReset(); /* reset the system */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF
 * FILE****/
