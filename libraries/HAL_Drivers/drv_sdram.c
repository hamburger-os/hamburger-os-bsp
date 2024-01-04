/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-04     zylx         first version
 */

#include <board.h>

#ifdef BSP_USING_SDRAM

#if defined(RT_USING_FAL)
#include "fal.h"
#include "fal_cfg.h"
#endif

#define DRV_DEBUG
#define LOG_TAG             "drv.sdram"
#include <drv_log.h>

static SDRAM_HandleTypeDef hsdram;
static FMC_SDRAM_CommandTypeDef command;

/**
  * @brief
  * @param  hsdram: SDRAM handle
  * @param  Command: Pointer to SDRAM command structure
  * @retval None
  */
static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
    __IO uint32_t tmpmrd = 0;
    uint32_t target_bank = 0;

#if SDRAM_TARGET_BANK == 1
    target_bank = FMC_SDRAM_CMD_TARGET_BANK1;
#else
    target_bank = FMC_SDRAM_CMD_TARGET_BANK2;
#endif

    /* Configure a clock configuration enable command */
    Command->CommandMode           = FMC_SDRAM_CMD_CLK_ENABLE;
    Command->CommandTarget         = target_bank;
    Command->AutoRefreshNumber     = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

    /* Insert 100 ms delay */
    /* interrupt is not enable, just to delay some time. */
    for (tmpmrd = 0; tmpmrd < 0xffff; tmpmrd ++)
        ;

    /* Configure a PALL (precharge all) command */
    Command->CommandMode            = FMC_SDRAM_CMD_PALL;
    Command->CommandTarget          = target_bank;
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

    /* Configure a Auto-Refresh command */
    Command->CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command->CommandTarget          = target_bank;
    Command->AutoRefreshNumber      = 8;
    Command->ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

    /* Program the external memory mode register */
#if SDRAM_DATA_WIDTH == 8
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1     |
#elif SDRAM_DATA_WIDTH == 16
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2     |
#else
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_4     |
#endif
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL        |
#if SDRAM_CAS_LATENCY == 3
             SDRAM_MODEREG_CAS_LATENCY_3                |
#else
             SDRAM_MODEREG_CAS_LATENCY_2                |
#endif
             SDRAM_MODEREG_OPERATING_MODE_STANDARD      |
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    Command->CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command->CommandTarget          = target_bank;
    Command->AutoRefreshNumber      = 1;
    Command->ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

    /* Set the device refresh counter */
    HAL_SDRAM_ProgramRefreshRate(hsdram, SDRAM_REFRESH_COUNT);
}

int SDRAM_Init(void *arg)
{
    int result = RT_EOK;
    FMC_SDRAM_TimingTypeDef SDRAM_Timing;

    /* SDRAM device configuration */
    hsdram.Instance = FMC_SDRAM_DEVICE;
    SDRAM_Timing.LoadToActiveDelay    = LOADTOACTIVEDELAY;
    SDRAM_Timing.ExitSelfRefreshDelay = EXITSELFREFRESHDELAY;
    SDRAM_Timing.SelfRefreshTime      = SELFREFRESHTIME;
    SDRAM_Timing.RowCycleDelay        = ROWCYCLEDELAY;
    SDRAM_Timing.WriteRecoveryTime    = WRITERECOVERYTIME;
    SDRAM_Timing.RPDelay              = RPDELAY;
    SDRAM_Timing.RCDDelay             = RCDDELAY;

#if SDRAM_TARGET_BANK == 1
    hsdram.Init.SDBank             = FMC_SDRAM_BANK1;
#else
    hsdram.Init.SDBank             = FMC_SDRAM_BANK2;
#endif
#if SDRAM_COLUMN_BITS == 8
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
#elif SDRAM_COLUMN_BITS == 9
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
#elif SDRAM_COLUMN_BITS == 10
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_10;
#else
    hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_11;
#endif
#if SDRAM_ROW_BITS == 11
    hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_11;
#elif SDRAM_ROW_BITS == 12
    hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
#else
    hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;
#endif

#if SDRAM_DATA_WIDTH == 8
    hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_8;
#elif SDRAM_DATA_WIDTH == 16
    hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_16;
#else
    hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
#endif
    hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
#if SDRAM_CAS_LATENCY == 1
    hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_1;
#elif SDRAM_CAS_LATENCY == 2
    hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_2;
#else
    hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
#endif
    hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
#if SDCLOCK_PERIOD == 2
    hsdram.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
#else
    hsdram.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_3;
#endif
    hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
#if SDRAM_RPIPE_DELAY == 0
    hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;
#elif SDRAM_RPIPE_DELAY == 1
    hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;
#else
    hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_2;
#endif

    /* Initialize the SDRAM controller */
    if (HAL_SDRAM_Init(&hsdram, &SDRAM_Timing) != HAL_OK)
    {
        LOG_E("SDRAM init failed!");
        result = -RT_ERROR;
    }
    else
    {
        /* Program the SDRAM external device */
        SDRAM_Initialization_Sequence(&hsdram, &command);
    }

    return result;
}

#ifdef BSP_SDRAM_ENABLE_BLK
static int fal_sdram_init(void);
static int fal_sdram_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_sdram_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_sdram_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
const struct fal_flash_dev sdram_flash =
{
    .name = SDRAM_DEV_NAME,
    .addr = SDRAM_BANK_ADDR + SDRAM_SIZE - BSP_SDRAM_BLK_SIZE,
    .len = BSP_SDRAM_BLK_SIZE,
    .blk_size = SDRAM_BLK_SIZE,
    .ops = {fal_sdram_init, fal_sdram_read, fal_sdram_write, fal_sdram_erase},
    .write_gran = 0,
};

static int fal_sdram_init(void)
{
    rt_err_t ret = RT_EOK;

    LOG_I("init succeed %d MB.", sdram_flash.len/1024/1024);
    return ret;
}
static int fal_sdram_read(long offset, rt_uint8_t *buf, size_t size)
{
    uint32_t addr = sdram_flash.addr + offset;
    if (addr + size > sdram_flash.addr + sdram_flash.len)
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
static int fal_sdram_write(long offset, const rt_uint8_t *buf, size_t size)
{
    uint32_t addr = sdram_flash.addr + offset;
    if (addr + size > sdram_flash.addr + sdram_flash.len)
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
static int fal_sdram_erase(long offset, size_t size)
{
    rt_uint32_t addr = sdram_flash.addr + offset;
    if ((addr + size) > sdram_flash.addr + sdram_flash.len)
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

#endif /* BSP_USING_SDRAM */
