/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-22     lvhan       the first version
 */
#include "board.h"

#define DBG_TAG "drv.fmc"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

struct HWFMCDef
{
    SRAM_HandleTypeDef hsram;
    FMC_NORSRAM_TimingTypeDef Timing;
};

static struct HWFMCDef fmc_list[] = {
#ifdef BSP_USING_FMC1
    {
        .hsram.Init.NSBank = FMC_NORSRAM_BANK1,
#if FMC1_BUS_WIDTH == 8
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8,
#endif
#if FMC1_BUS_WIDTH == 16
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16,
#endif
#if FMC1_BUS_WIDTH == 32
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_32,
#endif
        .Timing.AddressSetupTime = FMC1_AddressSetupTime,
        .Timing.DataSetupTime = FMC1_DataSetupTime,
        .Timing.BusTurnAroundDuration = FMC1_BusTurnAroundDuration,
    },
#endif

#ifdef BSP_USING_FMC2
    {
        .hsram.Init.NSBank = FMC_NORSRAM_BANK2,
#if FMC2_BUS_WIDTH == 8
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8,
#endif
#if FMC2_BUS_WIDTH == 16
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16,
#endif
#if FMC2_BUS_WIDTH == 32
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_32,
#endif
        .Timing.AddressSetupTime = FMC2_AddressSetupTime,
        .Timing.DataSetupTime = FMC2_DataSetupTime,
        .Timing.BusTurnAroundDuration = FMC2_BusTurnAroundDuration,
    },
#endif

#ifdef BSP_USING_FMC3
    {
        .hsram.Init.NSBank = FMC_NORSRAM_BANK3,
#if FMC3_BUS_WIDTH == 8
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8,
#endif
#if FMC3_BUS_WIDTH == 16
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16,
#endif
#if FMC3_BUS_WIDTH == 32
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_32,
#endif
        .Timing.AddressSetupTime = FMC3_AddressSetupTime,
        .Timing.DataSetupTime = FMC3_DataSetupTime,
        .Timing.BusTurnAroundDuration = FMC3_BusTurnAroundDuration,
    },
#endif

#ifdef BSP_USING_FMC4
    {
        .hsram.Init.NSBank = FMC_NORSRAM_BANK4,
#if FMC4_BUS_WIDTH == 8
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8,
#endif
#if FMC4_BUS_WIDTH == 16
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16,
#endif
#if FMC4_BUS_WIDTH == 32
        .hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_32,
#endif
        .Timing.AddressSetupTime = FMC4_AddressSetupTime,
        .Timing.DataSetupTime = FMC4_DataSetupTime,
        .Timing.BusTurnAroundDuration = FMC4_BusTurnAroundDuration,
    },
#endif
};

static int fmc_hw_init(void)
{
    SRAM_HandleTypeDef hsram;
    FMC_NORSRAM_TimingTypeDef Timing = {0};

    for (int i = 0; i < sizeof(fmc_list) / sizeof(struct HWFMCDef); i++)
    {
        /** Perform the SRAM memory initialization sequence
        */
        hsram.Instance = FMC_NORSRAM_DEVICE;
        hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
        /* hsram.Init */
        hsram.Init.NSBank = fmc_list[i].hsram.Init.NSBank;
        hsram.Init.MemoryDataWidth = fmc_list[i].hsram.Init.MemoryDataWidth;
#ifdef FSMC_Bank1
        hsram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
        hsram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;

        hsram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
        hsram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
#ifdef SOC_SERIES_STM32F4
        hsram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
#endif
        hsram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
        hsram.Init.WriteOperation = FSMC_WRITE_OPERATION_DISABLE;
        hsram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
        hsram.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
        hsram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_ENABLE;
        hsram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
        hsram.Init.ContinuousClock = FSMC_CONTINUOUS_CLOCK_SYNC_ONLY;
#ifdef SOC_SERIES_STM32H7
        hsram.Init.WriteFifo = FSMC_WRITE_FIFO_ENABLE;
#endif
        hsram.Init.PageSize = FSMC_PAGE_SIZE_NONE;
#else
        hsram.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
        hsram.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;

        hsram.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
        hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
#ifdef SOC_SERIES_STM32F4
        hsram.Init.WrapMode = FMC_WRAP_MODE_DISABLE;
#endif
        hsram.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
        hsram.Init.WriteOperation = FMC_WRITE_OPERATION_DISABLE;
        hsram.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
        hsram.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
        hsram.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_ENABLE;
        hsram.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
        hsram.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
#ifdef SOC_SERIES_STM32H7
        hsram.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
#endif
        hsram.Init.PageSize = FMC_PAGE_SIZE_NONE;
#endif
        /* Timing */
        Timing.AddressSetupTime = fmc_list[i].Timing.AddressSetupTime;
        Timing.AddressHoldTime = 15;
        Timing.DataSetupTime = fmc_list[i].Timing.DataSetupTime;
        Timing.BusTurnAroundDuration = fmc_list[i].Timing.BusTurnAroundDuration;
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
    }

    return RT_EOK;
}
INIT_DEVICE_EXPORT(fmc_hw_init);
