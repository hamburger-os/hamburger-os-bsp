/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-5      balanceTWK   first version
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtthread.h>
#include <rtdevice.h>

#ifdef SOC_SERIES_STM32F1
#include <stm32f1xx.h>
#endif

#ifdef SOC_SERIES_STM32F4
#include <stm32f4xx.h>
#endif

#ifdef SOC_SERIES_STM32H7
#include <stm32h7xx.h>
#endif

#include "drv_common.h"
#include "drv_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_VERSION "v1.2.0.2_20240124"

#define STM32_FLASH_START_ADRESS        ((uint32_t)BSP_FLASH_START_ADDR)
#ifdef FLASH_USING_BLK256
#define STM32_FLASH_SIZE                (3072 * 1024)
#else
#define STM32_FLASH_SIZE                (BSP_FLASH_SIZE)
#endif
#define STM32_FLASH_END_ADDRESS         ((uint32_t)(STM32_FLASH_START_ADRESS + STM32_FLASH_SIZE))

#define STM32_SRAM_SIZE                 (BSP_SRAM_SIZE)
#define STM32_SRAM_END                  (BSP_SRAM_START_ADDR + STM32_SRAM_SIZE)

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#else
extern int __bss_end;
#define HEAP_BEGIN      (&__bss_end)
#endif

#define HEAP_END        STM32_SRAM_END

#ifdef BSP_ENABLE_MPU
#ifndef MemManage_Handler
#define MemManage_Handler() _MemManage_Handler(__FILE__, __LINE__)
#endif

int rt_hw_mpu_init(void);
#endif

#define __SWP16(A)  ((( (uint16_t)(A) & 0xff00) >> 8) | \
                    (( (uint16_t)(A) & 0x00ff) << 8))

#define __SWP24(A)  ((( (uint32_t)(A) & 0x00ff0000) >> 16)   | \
                    (( (uint32_t)(A) & 0x000000ff) << 16))

#define __SWP32(A)  ((( (uint32_t)(A) & 0xff000000) >> 24) | \
                    (( (uint32_t)(A) & 0x00ff0000) >> 8)   | \
                    (( (uint32_t)(A) & 0x0000ff00) << 8)   | \
                    (( (uint32_t)(A) & 0x000000ff) << 24))

uint32_t memory_info_size(void);

//rt_kprintf("%d: %s %s %d\n", rt_tick_get(), __FILE__, __FUNCTION__, __LINE__);
//LOG_E("%s %s %d", __FILE__, __FUNCTION__, __LINE__);

#ifdef __cplusplus
}
#endif

#endif
