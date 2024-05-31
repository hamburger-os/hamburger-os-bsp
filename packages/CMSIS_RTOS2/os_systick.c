/**************************************************************************//**
 * @file     os_systick.c
 * @brief    CMSIS OS Tick SysTick implementation
 * @version  V1.0.1
 * @date     29. November 2017
 ******************************************************************************/
/*
 * Copyright (c) 2017-2017 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <rtthread.h>
#include "os_tick.h"
#include "board.h"

#ifdef  SysTick

#ifndef SYSTICK_IRQ_PRIORITY
#define SYSTICK_IRQ_PRIORITY    0xFFU
#endif

static uint8_t PendST __attribute__((section(".bss.os")));

// Setup OS Tick.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak int32_t OS_Tick_Setup (uint32_t freq, IRQHandler_t handler) {
#else
RT_WEAK int32_t OS_Tick_Setup (uint32_t freq, IRQHandler_t handler) {
#endif
  uint32_t load;
  (void)handler;

  if (freq == 0U) {
    //lint -e{904} "Return statement before end of function"
    return (-1);
  }

  load = (SystemCoreClock / freq) - 1U;
  if (load > 0x00FFFFFFU) {
    //lint -e{904} "Return statement before end of function"
    return (-1);
  }

  NVIC_SetPriority(SysTick_IRQn, SYSTICK_IRQ_PRIORITY);

  SysTick->CTRL =  SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
  SysTick->LOAD =  load;
  SysTick->VAL  =  0U;

  PendST = 0U;

  return (0);
}

/// Enable OS Tick.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak void OS_Tick_Enable (void) {
#else
RT_WEAK void OS_Tick_Enable (void) {
#endif

  if (PendST != 0U) {
    PendST = 0U;
    SCB->ICSR = SCB_ICSR_PENDSTSET_Msk;
  }

  SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
}

/// Disable OS Tick.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak void OS_Tick_Disable (void) {
#else
RT_WEAK void OS_Tick_Disable (void) {
#endif

  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

  if ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0U) {
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
    PendST = 1U;
  }
}

// Acknowledge OS Tick IRQ.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak void OS_Tick_AcknowledgeIRQ (void) {
#else
RT_WEAK void OS_Tick_AcknowledgeIRQ (void) {
#endif
  (void)SysTick->CTRL;
}

// Get OS Tick IRQ number.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak int32_t  OS_Tick_GetIRQn (void) {
#else
RT_WEAK int32_t  OS_Tick_GetIRQn (void) {
#endif
  return ((int32_t)SysTick_IRQn);
}

// Get OS Tick clock.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak uint32_t OS_Tick_GetClock (void) {
#else
RT_WEAK uint32_t OS_Tick_GetClock (void) {
#endif
  return (SystemCoreClock);
}

// Get OS Tick interval.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak uint32_t OS_Tick_GetInterval (void) {
#else
RT_WEAK uint32_t OS_Tick_GetInterval (void) {
#endif
  return (SysTick->LOAD + 1U);
}

// Get OS Tick count value.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak uint32_t OS_Tick_GetCount (void) {
#else
RT_WEAK uint32_t OS_Tick_GetCount (void) {
#endif
  uint32_t load = SysTick->LOAD;
  return  (load - SysTick->VAL);
}

// Get OS Tick overflow status.
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
rt_weak uint32_t OS_Tick_GetOverflow (void) {
#else
RT_WEAK uint32_t OS_Tick_GetOverflow (void) {
#endif
  return ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) >> SCB_ICSR_PENDSTSET_Pos);
}

#endif  // SysTick
