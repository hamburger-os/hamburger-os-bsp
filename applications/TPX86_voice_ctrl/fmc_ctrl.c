/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-01-05     lvhan       the first version
 */

#include "board.h"

#include "tpx86_voice_ctrl.h"

#define DBG_TAG "tpx86-fmc"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static void fmc_hw_init(uint32_t NSBank)
{
    SRAM_HandleTypeDef hsram;
    FMC_NORSRAM_TimingTypeDef Timing = {0};

    /** Perform the SRAM4 memory initialization sequence
    */
    hsram.Instance = FMC_NORSRAM_DEVICE;
    hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram.Init */
    hsram.Init.NSBank = NSBank;
    hsram.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WrapMode = FMC_WRAP_MODE_DISABLE;
    hsram.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FMC_WRITE_OPERATION_DISABLE;
    hsram.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_ENABLE;
    hsram.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = 15;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = 255;
    Timing.BusTurnAroundDuration = 15;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram, &Timing, NULL) != HAL_OK)
    {
        Error_Handler( );
    }
}

static void fmc_thread_entry(void *parameter)
{
    VoiceCtrlUserData *puserdata = (VoiceCtrlUserData *)parameter;

    for (int i = 0; i < sizeof(puserdata->NSBank)/sizeof(puserdata->NSBank[0]); i++)
    {
        fmc_hw_init(puserdata->NSBank[i]);
    }

    LOG_I("fmc thread startup...");
    while(puserdata->isThreadRun)
    {
        rt_thread_delay(1000);

        puserdata->NSBankValue[0] = *(FMC_ValueDef *)puserdata->NSBankAddress[0];
        puserdata->NSBankValue[1] = *(FMC_ValueDef *)puserdata->NSBankAddress[1];

        LOG_D("value  : 0x%x 0x%x", puserdata->NSBankValue[0].value, puserdata->NSBankValue[1].value);
    }
}

int tpx86_voice_ctrl_fmc_init(void)
{
    /* 创建 fmc 线程 */
    rt_thread_t thread = rt_thread_create("fmc", fmc_thread_entry, &voice_ctrl_userdata, 2048, 26, 10);
    /* 创建成功则启动线程 */
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("thread startup error!");
    }

    return RT_EOK;
}
