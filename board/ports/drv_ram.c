/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-02-07     lysw       the first version
 */

#include <stdlib.h>
#include <string.h>
#include "board.h"

#if defined(RT_USING_FAL)
#include "fal.h"
#include "fal_cfg.h"
#endif

#define DBG_TAG "drv.ram"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

struct memDef
{
    char *name;
    void *start;
    const int size;
#ifdef RT_USING_MEMHEAP_AS_HEAP
    struct rt_memheap heap;
#endif
    void *arg;
    int (*init)(void *);
};

#ifdef BSP_USING_SDRAM
extern int SDRAM_Init(void *);
#endif

#ifdef BSP_USING_FMCSRAM
static int MX_FMCSRAM_Init(void *);
#endif

//按照内存速度从慢到快进行排列
static uint32_t total = 0;
#ifdef RT_USING_MEMHEAP_AS_HEAP
static struct memDef memlist[] = {
#ifdef BSP_USING_SDRAM
    {
        .name = "sdram",
        .start = (void *)SDRAM_BANK_ADDR,
        .size = SDRAM_SIZE,
        .arg = NULL,
        .init = SDRAM_Init,
    },
#endif

#if defined(BSP_USING_FMCSRAM) && !defined(BSP_FMCSRAM_ENABLE_FS)
    {
        .name = "sram",
        .start = (void *)BSP_FMCSRAM_ADDR,
        .size = BSP_FMCSRAM_SIZE,
        .arg = (void *)BSP_FMCSRAM_NE,
        .init = MX_FMCSRAM_Init,
    },
#endif

#ifdef BSP_USING_AHBSRAM_D3
    {
        .name = "ahbsram3",
        .start = (void *)BSP_AHBSRAM_D3_ADDR,
        .size = BSP_AHBSRAM_D3_SIZE,
        .arg = NULL,
        .init = NULL,
    },
#endif

#ifdef BSP_USING_AHBSRAM_D2
    {
        .name = "ahbsram2",
        .start = (void *)BSP_AHBSRAM_D2_ADDR,
        .size = BSP_AHBSRAM_D2_SIZE,
        .arg = NULL,
        .init = NULL,
    },
#endif

//#ifdef BSP_USING_AXISRAM
//    {
//        .name = "axisram",
//        .start = (void *)BSP_AXISRAM_ADDR,
//        .size = BSP_AXISRAM_SIZE,
//        .arg = NULL,
//        .init = NULL,
//    },
//#endif

#ifdef BSP_USING_ITCMSRAM
    {
        .name = "itcmsram",
        .start = (void *)BSP_ITCMSRAM_ADDR,
        .size = BSP_ITCMSRAM_SIZE,
        .arg = NULL,
        .init = NULL,
    },
#endif

#ifdef BSP_USING_DTCMSRAM
    {
        .name = "dtcmsram",
        .start = (void *)BSP_DTCMSRAM_ADDR,
        .size = BSP_DTCMSRAM_SIZE,
        .arg = NULL,
        .init = NULL,
    },
#endif

#ifdef BSP_USING_ADDSRAM
    {
        .name = "addsram",
        .start = (void *)BSP_ADDSRAM_ADDR,
        .size = BSP_ADDSRAM_SIZE,
        .arg = NULL,
        .init = NULL,
    },
#endif

#ifdef BSP_USING_CCMSRAM
    {
        .name = "ccmsram",
        .start = (void *)BSP_CCMSRAM_ADDR,
        .size = BSP_CCMSRAM_SIZE,
        .arg = NULL,
        .init = NULL,
    },
#endif

    {
        .name = "ram",
        .start = (void *)HEAP_BEGIN,
        .size = STM32_SRAM_SIZE,
        .arg = NULL,
        .init = NULL,
    },
};
#endif

#ifdef BSP_USING_FMCSRAM
static int MX_FMCSRAM_Init(void *arg)
{
    uint32_t NSBank = (uint32_t)arg;

#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) || defined (STM32F417xx) || \
    defined (STM32F427xx) || defined (STM32F437xx) || defined (STM32F429xx) || defined (STM32F439xx) || \
    defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) || \
    defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) || defined (STM32F469xx) || \
    defined (STM32F479xx) || defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || \
    defined (STM32F412Zx) || defined (STM32F413xx) || defined (STM32F423xx)

    SRAM_HandleTypeDef hsram = {0};
    FMC_NORSRAM_TimingTypeDef Timing = {0};

    /** Perform the SRAM memory initialization sequence
    */
    hsram.Instance = FMC_NORSRAM_DEVICE;
    hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram.Init */
    hsram.Init.NSBank = NSBank;
#ifdef FSMC_Bank1
    hsram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = BSP_FMCSRAM_BUS_WIDTH;
    hsram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
    hsram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
    hsram.Init.PageSize = FSMC_PAGE_SIZE_NONE;
#else
    hsram.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = BSP_FMCSRAM_BUS_WIDTH;
    hsram.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WrapMode = FMC_WRAP_MODE_DISABLE;
    hsram.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram.Init.PageSize = FMC_PAGE_SIZE_NONE;
#endif
    /* Timing */
    Timing.AddressSetupTime = BSP_FMCSRAM_AddressSetupTime;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = BSP_FMCSRAM_DataSetupTime;
    Timing.BusTurnAroundDuration = BSP_FMCSRAM_BusTurnAroundDuration;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
#ifdef FSMC_Bank1
    Timing.AccessMode = FSMC_ACCESS_MODE_A;
#else
    Timing.AccessMode = FMC_ACCESS_MODE_A;
#endif
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram, &Timing, NULL) != HAL_OK)
    {
        Error_Handler( );
    }

#endif

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx)  || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx) || defined (STM32H747xx)  || defined (STM32H747xG) || defined (STM32H757xx)  || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx) || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ) || defined (STM32H725xx) || defined (STM32H723xx)

    SRAM_HandleTypeDef hsram = {0};
    FMC_NORSRAM_TimingTypeDef Timing = {0};

    /** Perform the SRAM memory initialization sequence
      */
    hsram.Instance = FMC_NORSRAM_DEVICE;
    hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram.Init */
    hsram.Init.NSBank = NSBank;
    hsram.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = BSP_FMCSRAM_BUS_WIDTH;
    hsram.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
    hsram.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = BSP_FMCSRAM_AddressSetupTime;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = BSP_FMCSRAM_DataSetupTime;
    Timing.BusTurnAroundDuration = BSP_FMCSRAM_BusTurnAroundDuration;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram, &Timing, NULL) != HAL_OK)
    {
        Error_Handler( );
    }
#endif
    return RT_EOK;
}
#endif

#define TEST_MAX 16
static int rt_mem_init(void)
{
    rt_err_t ret = RT_EOK;

    total = HEAP_END - (uint32_t)HEAP_BEGIN;
#ifdef RT_USING_MEMHEAP_AS_HEAP
    for (int i = 0; i < (sizeof(memlist) / sizeof(struct memDef)) - 1; i++)
    {
        if (memlist[i].init != NULL)
        {
            memlist[i].init(memlist[i].arg);
        }
        /* 一个简单的方式验证sram是否正常启动 */
        uint8_t *sram_start = ((uint8_t *)memlist[i].start);
        uint8_t *sram_end = ((uint8_t *)memlist[i].start + memlist[i].size - memlist[i].size/TEST_MAX);

        for (int j=0; j < memlist[i].size/TEST_MAX; j++)
        {
            sram_start[j] = j;
            sram_end[j] = j;
        }
        if (rt_memcmp(sram_start, sram_end, memlist[i].size/TEST_MAX) == 0)
        {
            ret = rt_memheap_init(&memlist[i].heap, memlist[i].name, memlist[i].start, memlist[i].size);
            if (ret != RT_EOK)
            {
                LOG_E("memheap init %s error %d.", memlist[i].name, ret);
            }
            total += memlist[i].size;
        }
        else
        {
            LOG_E("init %s failed 0x%p %d", memlist[i].name, memlist[i].start, memlist[i].size);
        }
    }
#endif

#ifdef RT_USING_MEMHEAP_AS_HEAP
    extern void list_memheap(void);
    list_memheap();
#else /* RT_USING_MEMHEAP_AS_HEAP */
    rt_size_t total = 0, used = 0, max_used = 0;

    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("total    : %d\n", total);
    rt_kprintf("used     : %d\n", used);
    rt_kprintf("maximum  : %d\n", max_used);
    rt_kprintf("available: %d\n", total - used);
#endif
    LOG_I("init succeed %d KB(%d).", total / 1024, total);
    return ret;
}
/* 导出到自动初始化 */
INIT_PREV_EXPORT(rt_mem_init);

uint32_t memory_info_size(void)
{
    return total;
}

#ifdef BSP_FMCSRAM_ENABLE_BLK
static int fal_sram_init(void);
static int fal_sram_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_sram_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_sram_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
const struct fal_flash_dev sram_flash =
{
    .name = FMCSRAM_DEV_NAME,
    .addr = BSP_FMCSRAM_ADDR,
    .len = BSP_FMCSRAM_SIZE,
    .blk_size = FMCSRAM_BLK_SIZE,
    .ops = {fal_sram_init, fal_sram_read, fal_sram_write, fal_sram_erase},
    .write_gran = 0,
};

static int fal_sram_init(void)
{
    rt_err_t ret = RT_EOK;

    MX_FMCSRAM_Init((void *)BSP_FMCSRAM_NE);

    LOG_I("init succeed %d MB.", sram_flash.len/1024/1024);
    return ret;
}
static int fal_sram_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr = sram_flash.addr + offset;
    if (addr + size > sram_flash.addr + sram_flash.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }

    rt_memcpy(buf, (const void *)addr, size);
//    LOG_D("read (0x%p) %d", (void*)(addr), size);

    return size;
}
static int fal_sram_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr = sram_flash.addr + offset;
    if (addr + size > sram_flash.addr + sram_flash.len)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }

    rt_memcpy((void*)addr, buf, size);
//    LOG_D("write (0x%p) %d", (void*)(addr), size);

    return size;
}
static int fal_sram_erase(long offset, size_t size)
{
    rt_uint32_t addr = sram_flash.addr + offset;
    if ((addr + size) > sram_flash.addr + sram_flash.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        return -RT_EINVAL;
    }
    if (size < 1)
    {
        LOG_W("erase size %d! addr is (0x%p)", size, (void*)(addr + size));
        return -RT_EINVAL;
    }

//    LOG_D("erase (0x%p) %d", (void*)(addr), size);

    return size;
}
#endif

#define MEMTEST_WIDTH (1024)
int memory_test(void)
{
    rt_err_t ret = RT_EOK;
    uint32_t maxcount = total / MEMTEST_WIDTH;
    uint32_t maxsize = maxcount * sizeof(uint32_t *);
    uint32_t **addr = rt_malloc(maxsize);
    uint8_t *test_data = rt_malloc(MEMTEST_WIDTH);
    rt_tick_t tick = rt_tick_get_millisecond();

    if (addr == NULL || test_data == NULL)
    {
        LOG_E("insufficient memory size to support memory test case.");
        ret = -RT_ERROR;
    }
    else
    {
        for (uint16_t i = 0; i < MEMTEST_WIDTH; i++)
        {
            test_data[i]  = i;
        }
        LOG_D("memory test starts :");
        LOG_D("test 0x%p (%d) width %d.", addr, maxsize, MEMTEST_WIDTH);

        // malloc
        uint32_t mi = 0;
        tick = rt_tick_get_millisecond();
        for (mi = 0; mi < maxcount; mi++)
        {
            addr[mi] = malloc(MEMTEST_WIDTH);
            if (addr[mi] != NULL)
            {
                rt_memcpy(addr[mi], test_data, MEMTEST_WIDTH);
                if (rt_memcmp(addr[mi], test_data, MEMTEST_WIDTH) != 0)
                {
                    LOG_E("rt_malloc 0x%p data error", addr[mi]);
                    LOG_HEX("malloc", 16, (uint8_t *)addr[mi], 64);
                    ret = -RT_ERROR;
                }
                if (addr[mi] - addr[(mi > 0)?(mi - 1):(mi)] != (MEMTEST_WIDTH + 24)/4)
                {
                    LOG_D("addr 0x%p malloc %d/%d in %d ms", addr[mi], mi, maxcount, rt_tick_get_millisecond() - tick);
                }
            }
            else
            {
                if (mi > 0)
                {
                    LOG_D("addr 0x%p malloc %d/%d in %d ms", addr[mi - 1], (mi - 1), maxcount, rt_tick_get_millisecond() - tick);
                }
                LOG_D("rt_malloc %d KB, used %d ms.", mi * MEMTEST_WIDTH / 1024, rt_tick_get_millisecond() - tick);
                break;
            }
        }
        // free
        uint32_t fi = 0;
        tick = rt_tick_get_millisecond();
        for (fi = 0; fi < mi; fi++)
        {
            if (rt_memcmp(addr[fi], test_data, MEMTEST_WIDTH) != 0)
            {
                LOG_E("rt_free 0x%p data error", addr[fi]);
                LOG_HEX("free", 16, (uint8_t *)addr[fi], 64);
                ret = -RT_ERROR;
            }
            if (addr[fi] - addr[(fi > 0)?(fi - 1):(fi)] != (MEMTEST_WIDTH + 24)/4)
            {
                LOG_D("addr:0x%p free %d/%d in %d ms", addr[fi], fi, mi, rt_tick_get_millisecond() - tick);
            }
            free(addr[fi]);
        }
        if (fi > 0)
        {
            LOG_D("addr 0x%p free %d/%d in %d ms", addr[fi - 1], (fi - 1), mi, rt_tick_get_millisecond() - tick);
        }
        LOG_D("rt_free %d KB, used %d ms.", fi * MEMTEST_WIDTH / 1024, rt_tick_get_millisecond() - tick);

        rt_free(addr);
        rt_free(test_data);
    }

#ifdef RT_USING_MEMHEAP_AS_HEAP
    extern void list_memheap(void);
    list_memheap();
#else /* RT_USING_MEMHEAP_AS_HEAP */
    rt_size_t total = 0, used = 0, max_used = 0;

    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("total    : %d\n", total);
    rt_kprintf("used     : %d\n", used);
    rt_kprintf("maximum  : %d\n", max_used);
    rt_kprintf("available: %d\n", total - used);
#endif

    if (ret == RT_EOK)
    {
        LOG_I("memory test succeed.");
    }
    else
    {
        LOG_E("memory test failed!");
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(memory_test, memory test);
