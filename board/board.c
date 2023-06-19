/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-7      SummerGift   first version
 */

#include "board.h"

static int rt_hw_clock_information(void)
{
    rt_kprintf("|------------------------------\n");
    rt_kprintf("| * System Clock information * \n");
    rt_kprintf("|    name    |\tfrequency\n");
    rt_kprintf("|------------------------------\n");
    rt_kprintf("|    SYSCLK  |\t%d\n", HAL_RCC_GetSysClockFreq());
    rt_kprintf("|    HCLK    |\t%d\n", HAL_RCC_GetHCLKFreq());
    rt_kprintf("|    PCLK1   |\t%d\n", HAL_RCC_GetPCLK1Freq());
    rt_kprintf("|    PCLK2   |\t%d\n", HAL_RCC_GetPCLK2Freq());
    rt_kprintf("|------------------------------\n");

    return RT_EOK;
}
INIT_PREV_EXPORT(rt_hw_clock_information);
