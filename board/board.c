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

#define DBG_TAG "board"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static int rt_hw_board_information(void)
{
    rt_kprintf("|---------------------------------------------------------|\n");
    rt_kprintf("| *              System Clock information               * |\n");
    rt_kprintf("|         name         |\t frequency                |\n");
    rt_kprintf("|---------------------------------------------------------|\n");
    rt_kprintf("|         SYSCLK       |\t%10d                |\n", HAL_RCC_GetSysClockFreq());
    rt_kprintf("|         HCLK         |\t%10d                |\n", HAL_RCC_GetHCLKFreq());
    rt_kprintf("|         PCLK1        |\t%10d                |\n", HAL_RCC_GetPCLK1Freq());
    rt_kprintf("|         PCLK2        |\t%10d                |\n", HAL_RCC_GetPCLK2Freq());
    rt_kprintf("|---------------------------------------------------------|\n");
    rt_kprintf("| *              System memory information              * |\n");
#ifdef RT_USING_MEMHEAP_AS_HEAP
    extern void list_memheap(void);
    list_memheap();
#else /* RT_USING_MEMHEAP_AS_HEAP */
    rt_size_t total = 0, used = 0, max_used = 0;

    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("|         total        |\t%10d                \n", total);
    rt_kprintf("|         used         |\t%10d                \n", used);
    rt_kprintf("|         maximum      |\t%10d                \n", max_used);
    rt_kprintf("|         available    |\t%10d                \n", total - used);
#endif
    rt_kprintf("|---------------------------------------------------------|\n");

    return RT_EOK;
}
INIT_PREV_EXPORT(rt_hw_board_information);

/*
 ******************************************************************************************************
 * 函 数 名: JumpToBootloader
 * 功能说明: 跳转到系统 BootLoader
 * 形 参: 无
 * 返 回 值: 无
 ******************************************************************************************************
 */
static void JumpToBootloader(void)
{
    uint32_t i=0;
    void (*SysMemBootJump)(void); /* 声明一个函数指针 */

#ifdef SOC_SERIES_STM32F1
    __IO uint32_t BootAddr = 0x1FFF0000; /* STM32F1 的系统 BootLoader 地址 */
#endif
#ifdef SOC_SERIES_STM32F4
    __IO uint32_t BootAddr = 0x1FFF0000; /* STM32F4 的系统 BootLoader 地址 */
#endif
#ifdef SOC_SERIES_STM32H7
    __IO uint32_t BootAddr = 0x1FF09800; /* STM32H7 的系统 BootLoader 地址 */
#endif

    LOG_D("Jump to bootloader 0x%p running ...", BootAddr);

    /* 关闭全局中断 */
    __disable_irq();
    HAL_DeInit();

    /* 关闭所有中断，清除所有中断挂起标志 */
    for (i = 0; i < 8; i++)
    {
        NVIC->ICER[i]=0xFFFFFFFF;
        NVIC->ICPR[i]=0xFFFFFFFF;
    }

    /* 关闭滴答定时器，复位到默认值 */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /* 设置所有时钟到默认状态， 使用 HSI 时钟 */
    HAL_RCC_DeInit();

    /* 跳转到系统 BootLoader，首地址是 MSP，地址+4 是复位中断服务程序地址 */
    SysMemBootJump = (void (*)(void)) (*((uint32_t *) (BootAddr + 4)));

    /* 在 RTOS 工程，这条语句很重要，设置为特权级模式，使用 MSP 指针 */
    __set_CONTROL(0);

    /* 设置主堆栈指针 */
    __set_MSP(*(uint32_t *)BootAddr);

    /* 跳转到系统 BootLoader */
    SysMemBootJump();

    /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
    LOG_E("Jump to system bootloader fail.");
}

static void jump_test(int argc, char *argv[])
{
    JumpToBootloader();
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(jump_test, jump, jump to system bootloader.);
#endif
