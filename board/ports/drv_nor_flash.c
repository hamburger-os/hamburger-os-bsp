/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-06     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_NORFLASH

#define DBG_TAG "drv.nor"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#if defined(RT_USING_FAL)
#include "fal.h"

#define NORFLASH_MANUFACTURER_CODE          ((uint16_t)0x0089)
#define NORFLASH_DEVICE_CODE1               ((uint16_t)0x227E)
#define NORFLASH_DEVICE_CODE2               ((uint16_t)0x2228)
#define NORFLASH_DEVICE_CODE3               ((uint16_t)0x2201)

static int fal_nor_init(void);
static int fal_nor_read(long offset, rt_uint8_t *buf, size_t size);
static int fal_nor_write(long offset, const rt_uint8_t *buf, size_t size);
static int fal_nor_erase(long offset, size_t size);

/* ===================== Flash device Configuration ========================= */
const struct fal_flash_dev nor_flash =
{
    .name = NORFLASH_DEV_NAME,
    .addr = BSP_NORFLASH_ADDR,
    .len = NORFLASH_SIZE_GRANULARITY_TOTAL,
    .blk_size = NORFLASH_BLK_SIZE,
    .ops = {fal_nor_init, fal_nor_read, fal_nor_write, fal_nor_erase},
    .write_gran = 32,
};

static struct rt_mutex hnor_mutex = {0};
static NOR_HandleTypeDef hnor = {0};
/* FMC initialization function */
static void MX_FMC_Init(uint32_t NSBank)
{
    FMC_NORSRAM_TimingTypeDef Timing = {0};

#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) || defined (STM32F417xx) || \
    defined (STM32F427xx) || defined (STM32F437xx) || defined (STM32F429xx) || defined (STM32F439xx) || \
    defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) || \
    defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) || defined (STM32F469xx) || \
    defined (STM32F479xx) || defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || \
    defined (STM32F412Zx) || defined (STM32F413xx) || defined (STM32F423xx)

    /** Perform the NOR1 memory initialization sequence
    */
    hnor.Instance = FMC_NORSRAM_DEVICE;
    hnor.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hnor.Init */
    hnor.Init.NSBank = NSBank;
    hnor.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hnor.Init.MemoryType = FMC_MEMORY_TYPE_NOR;
    hnor.Init.MemoryDataWidth = BSP_NORFLASH_BUS_WIDTH;
    hnor.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hnor.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hnor.Init.WrapMode = FMC_WRAP_MODE_DISABLE;
    hnor.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hnor.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hnor.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hnor.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hnor.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hnor.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hnor.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hnor.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = BSP_NORFLASH_AddressSetupTime;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = BSP_NORFLASH_DataSetupTime;
    Timing.BusTurnAroundDuration = BSP_NORFLASH_BusTurnAroundDuration;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_NOR_Init(&hnor, &Timing, NULL) != HAL_OK)
    {
        Error_Handler( );
    }
#endif

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx)  || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx) || defined (STM32H747xx)  || defined (STM32H747xG) || defined (STM32H757xx)  || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx) || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ) || defined (STM32H725xx) || defined (STM32H723xx)

    /** Perform the NOR memory initialization sequence
    */
    hnor.Instance = FMC_NORSRAM_DEVICE;
    hnor.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hnor.Init */
    hnor.Init.NSBank = NSBank;
    hnor.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hnor.Init.MemoryType = FMC_MEMORY_TYPE_NOR;
    hnor.Init.MemoryDataWidth = BSP_NORFLASH_BUS_WIDTH;
    hnor.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hnor.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hnor.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hnor.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hnor.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hnor.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hnor.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_ENABLE;
    hnor.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hnor.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hnor.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
    hnor.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = BSP_NORFLASH_AddressSetupTime;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = BSP_NORFLASH_DataSetupTime;
    Timing.BusTurnAroundDuration = BSP_NORFLASH_BusTurnAroundDuration;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_NOR_Init(&hnor, &Timing, NULL) != HAL_OK)
    {
        Error_Handler( );
    }
#endif
}

#if 0
/**
  * @brief  NOR BSP Wait for Ready/Busy signal.
  * @param  norHandle: Pointer to NOR handle
  * @param  Timeout: Timeout duration
  * @retval None
  */
/* NOR Ready/Busy signal GPIO definitions */
#define NOR_READY_BUSY_PIN    GPIO_PIN_6
#define NOR_READY_BUSY_GPIO   GPIOD
#define NOR_READY_STATE       GPIO_PIN_SET
#define NOR_BUSY_STATE        GPIO_PIN_RESET
void HAL_NOR_MspWait(NOR_HandleTypeDef *norHandle, uint32_t Timeout)
{
  uint32_t timeout = Timeout;

  /* Polling on Ready/Busy signal */
  while((HAL_GPIO_ReadPin(NOR_READY_BUSY_GPIO, NOR_READY_BUSY_PIN) != NOR_BUSY_STATE) && (timeout > 0))
  {
    timeout--;
  }

  timeout = Timeout;

  /* Polling on Ready/Busy signal */
  while((HAL_GPIO_ReadPin(NOR_READY_BUSY_GPIO, NOR_READY_BUSY_PIN) != NOR_READY_STATE) && (timeout > 0))
  {
    timeout--;
  }
}
#endif

static void norflash_hard_reset(void)
{
    rt_base_t pin = rt_pin_get(NORFLASH_RST);
    rt_pin_mode(pin, PIN_MODE_OUTPUT);
    rt_pin_write(pin, PIN_LOW);
    rt_thread_delay(10);
    rt_pin_write(pin, PIN_HIGH);
    rt_thread_delay(10);
}

static int fal_nor_init(void)
{
    norflash_hard_reset();
    MX_FMC_Init(BSP_NORFLASH_NE);

    /* 初始化互斥 */
    rt_mutex_init(&hnor_mutex, "nor", RT_IPC_FLAG_PRIO);

    NOR_IDTypeDef pNOR_ID = {0};
    if (HAL_NOR_Read_ID(&hnor, &pNOR_ID) != HAL_OK)
    {
        LOG_E("Read ID error!");
        return -RT_EIO;
    }
    else
    {
        if (pNOR_ID.Manufacturer_Code == NORFLASH_MANUFACTURER_CODE
                && pNOR_ID.Device_Code1 == NORFLASH_DEVICE_CODE1
                && pNOR_ID.Device_Code2 == NORFLASH_DEVICE_CODE2
                && pNOR_ID.Device_Code3 == NORFLASH_DEVICE_CODE3)
        {
            LOG_I("init succeed, Code : 0x%04x %04x %04x %04x len %d MB"
                    , pNOR_ID.Manufacturer_Code, pNOR_ID.Device_Code1, pNOR_ID.Device_Code2, pNOR_ID.Device_Code3, nor_flash.len/1024/1024);
        }
        else
        {
            LOG_W("init warning, Code : 0x%04x(%04x) %04x(%04x) %04x(%04x) %04x(%04x) len %d MB"
                    , pNOR_ID.Manufacturer_Code, NORFLASH_MANUFACTURER_CODE
                    , pNOR_ID.Device_Code1, NORFLASH_DEVICE_CODE1
                    , pNOR_ID.Device_Code2, NORFLASH_DEVICE_CODE2
                    , pNOR_ID.Device_Code3, NORFLASH_DEVICE_CODE3, nor_flash.len/1024/1024);
        }
    }

    return RT_EOK;
}

static int fal_nor_read(long offset, rt_uint8_t *buf, size_t size)
{
    rt_mutex_take(&hnor_mutex, RT_WAITING_FOREVER);
    uint32_t addr = nor_flash.addr + offset;
    if (addr + size > nor_flash.addr + nor_flash.len)
    {
        LOG_E("read outrange flash size! addr is (0x%p)", (void*)(addr + size));
        rt_mutex_release(&hnor_mutex);
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("read size %d! addr is (0x%p)", size, (void*)(addr + size));
        rt_mutex_release(&hnor_mutex);
        return 0;
    }
    LOG_D("read : 0x%p %d", addr, size);

    /* Return to read mode */
    if(HAL_NOR_ReturnToReadMode(&hnor) != HAL_OK)
    {
        LOG_E("ReturnToReadMode Error!");
        rt_mutex_release(&hnor_mutex);
        return 0;
    }

    /* Read back data from the NOR memory */
    if(HAL_NOR_ReadBuffer(&hnor, addr, (rt_uint16_t*)buf, (size % 2 == 0)?(size/2):(size/2 + 1)) != HAL_OK)
    {
        LOG_E("ReadBuffer Error!");
        rt_mutex_release(&hnor_mutex);
        return 0;
    }

    rt_mutex_release(&hnor_mutex);
    return size;
}

static int nor_program(uint32_t addr, uint16_t *buf, size_t size)
{
    uint16_t *pdata = NULL;
    int32_t index  = 0;
    uint32_t startaddress = 0;

    pdata = buf;
    index = size;
    startaddress = addr;

    while(index > 0)
    {
        /* Write data to NOR */
        if (HAL_NOR_Program(&hnor, (uint32_t *)startaddress, pdata) != HAL_OK)
        {
            LOG_E("Program Error!");
            return 0;
        }

        if (startaddress % 4096 == 0)
        {
            rt_thread_delay(1);
        }

        //TODO: 需要优化为调度器等待
        /* Read NOR device status */
        HAL_NOR_StatusTypeDef status = HAL_NOR_GetStatus(&hnor, nor_flash.addr, NOR_TMEOUT, 0);
        if(status != HAL_NOR_STATUS_SUCCESS)
        {
            LOG_E("Program GetStatus Error %08p %d/%d %d!", startaddress, size - index, size, status);
            rt_thread_delay(1);
            /* Read back data from the NOR memory */
            if(HAL_NOR_Read(&hnor, (uint32_t *)startaddress, pdata) != HAL_OK)
            {
                LOG_E("Program Read Error!");
            }
        }

        /* Update the counters */
        index -= 2;
        startaddress += 2;
        pdata++;
    }

    return size;
}
static int fal_nor_write(long offset, const rt_uint8_t *buf, size_t size)
{
    rt_mutex_take(&hnor_mutex, RT_WAITING_FOREVER);
    uint16_t *pdata = NULL;
    int32_t index  = 0;
    uint32_t startaddress = 0;
    uint32_t addr = nor_flash.addr + offset;
    if (addr + size > nor_flash.addr + nor_flash.len)
    {
        LOG_E("write outrange flash size! addr is (0x%p)", (void*)(addr + size));
        rt_mutex_release(&hnor_mutex);
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("write size %d! addr is (0x%p)", size, (void*)(addr + size));
        rt_mutex_release(&hnor_mutex);
        return 0;
    }
    LOG_D("write : 0x%p %d", addr, size);

    /* Write data to the NOR memory */
    pdata = (uint16_t *)buf;
    index = size;
    startaddress = addr;
#ifdef  NORFLASH_ENABLE_BUFFER_PROGRAM
    uint32_t nbr = (size % NORFLASH_BUFFER_PROGRAM_MAX == 0)?(size/NORFLASH_BUFFER_PROGRAM_MAX):(size/NORFLASH_BUFFER_PROGRAM_MAX + 1);
    uint32_t sizeWr  = 0;
    for (uint32_t i = 0; i < nbr; i++)
    {
        if (index >= NORFLASH_BUFFER_PROGRAM_MAX)
        {
            sizeWr = NORFLASH_BUFFER_PROGRAM_MAX;

            /* Write data to the NOR memory */
            if (HAL_NOR_ProgramBuffer(&hnor, startaddress, pdata, sizeWr/2) != HAL_OK)
            {
                LOG_E("ProgramBuffer Error!");
                rt_mutex_release(&hnor_mutex);
                return 0;
            }

            if (startaddress % 4096 == 0)
            {
                rt_thread_delay(1);
            }

            //TODO: 需要优化为调度器等待
            /* Read NOR device status */
            HAL_NOR_StatusTypeDef status = HAL_NOR_GetStatus(&hnor, nor_flash.addr, NOR_TMEOUT, 0);
            if(status != HAL_NOR_STATUS_SUCCESS)
            {
                LOG_E("ProgramBuffer GetStatus Error 0x%08p %d/%d %d!", startaddress, size - index, size, status);
                rt_thread_delay(sizeWr/16);
                /* Read back data from the NOR memory */
                if(HAL_NOR_ReadBuffer(&hnor, startaddress, pdata, sizeWr/2) != HAL_OK)
                {
                    LOG_E("ProgramBuffer ReadBuffer Error!");
                }
            }
        }
        else
        {
            sizeWr = index;

            size = nor_program(startaddress, pdata, index);
        }

        index -= sizeWr;
        startaddress += sizeWr;
        pdata += (sizeWr / 2);
    }
#else
    size = nor_program(startaddress, pdata, index);
#endif

    rt_mutex_release(&hnor_mutex);
    return size;
}

static int fal_nor_erase(long offset, size_t size)
{
    rt_mutex_take(&hnor_mutex, RT_WAITING_FOREVER);
    uint32_t addr = nor_flash.addr + offset;
    if (addr + size > nor_flash.addr + nor_flash.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%p)", (void*)(addr + size));
        rt_mutex_release(&hnor_mutex);
        return -RT_EINVAL;
    }
    if (size < 1)
    {
//        LOG_W("erase size %d! addr is (0x%p)", size, (void*)(addr + size));
        rt_mutex_release(&hnor_mutex);
        return 0;
    }
    LOG_D("erase : 0x%p %d", addr, size);

    size_t countmax = (size%nor_flash.blk_size == 0)?(size/nor_flash.blk_size):(size/nor_flash.blk_size + 1);
    for (size_t count = 0; count < countmax; count++)
    {
        rt_tick_t tick = rt_tick_get();
        /* Erase the NOR memory block to write on */
        if (HAL_NOR_Erase_Block(&hnor, addr - nor_flash.addr + count*nor_flash.blk_size, nor_flash.addr) != HAL_OK)
        {
            LOG_E("Erase Block Error!");
            rt_mutex_release(&hnor_mutex);
            return 0;
        }

        //TODO: 需要优化为调度器等待
        /* Return the NOR memory status */
        HAL_NOR_StatusTypeDef status = HAL_NOR_GetStatus(&hnor, nor_flash.addr, NOR_TMEOUT, 1);
        if(status != HAL_NOR_STATUS_SUCCESS)
        {
            LOG_E("Erase Block GetStatus Error %d!", status);
            rt_thread_delay(500);
        }

        LOG_D("erase blk : 0x%p %d/%d used %d ms", addr + count*nor_flash.blk_size, count, countmax, rt_tick_get() - tick);
    }

    rt_mutex_release(&hnor_mutex);
    return size;
}

#ifdef NORFLASH_ENABLE_TEST

static int is_all_equal(char arr[], int n, char value)
{
    for (int i = 0; i < n; i++)
    {
        if (arr[i] != value)
        {
            return 0;
        }
    }

    return 1;
}

static int norflash_test(void)
{
    rt_err_t ret = RT_EOK;
    uint32_t testsize = nor_flash.blk_size;
    LOG_I("norflash read write test start...");

    char *tbuf = rt_malloc(testsize);
    char *rbuf = rt_malloc(testsize);
    if (tbuf == NULL || rbuf == NULL)
    {
        rt_free(tbuf);
        rt_free(rbuf);
        LOG_W("Not enough memory to request a block.");

        testsize = 1024;
        tbuf = rt_malloc(testsize);
        rbuf = rt_malloc(testsize);
        if (tbuf == NULL || rbuf == NULL)
        {
            rt_free(tbuf);
            rt_free(rbuf);
            LOG_E("Not enough memory to complete the test.");
            return -RT_ERROR;
        }
    }

    for(uint32_t i = 0; i<testsize; i+=4)
    {
        uint32_t *data32 = (uint32_t *)&tbuf[i];
        *data32 = i;
    }

    rt_tick_t tick1, tick2, tick3, tick4;
    for (uint32_t offset = 0; offset <= nor_flash.len; offset += nor_flash.blk_size * (nor_flash.len/nor_flash.blk_size/8))
    {
        if (offset == nor_flash.len)
        {
            offset -= nor_flash.blk_size * 2;
        }

        tick1 = rt_tick_get_millisecond();
        fal_nor_erase(offset, testsize * 2);
        tick2 = rt_tick_get_millisecond();
        LOG_I("erase %08p %d use %u ms", offset, testsize, tick2 - tick1);

        fal_nor_write(offset, (uint8_t *)tbuf, testsize);
        fal_nor_write(offset + nor_flash.blk_size, (uint8_t *)tbuf, testsize);
        tick3 = rt_tick_get_millisecond();
        LOG_I("write %08p %d use %u ms: %08x %08x", offset, testsize, tick3 - tick2, *(uint32_t *)&tbuf[0], *(uint32_t *)&tbuf[testsize-4]);

        fal_nor_read (offset, (uint8_t *)rbuf, testsize);
        tick4 = rt_tick_get_millisecond();
        LOG_I("read  %08p %d use %u ms: %08x %08x", offset, testsize, tick4 - tick3, *(uint32_t *)&rbuf[0], *(uint32_t *)&rbuf[testsize-4]);

        if (rt_memcmp(tbuf, rbuf, testsize) != 0)
        {
            LOG_E("check %08p %d failed!", offset, testsize);
            ret = -RT_ERROR;
        }
        rt_memset(rbuf, 0, testsize);
    }

    for (uint32_t offset = 0; offset <= nor_flash.len; offset += nor_flash.blk_size * (nor_flash.len/nor_flash.blk_size/8))
    {
        if (offset == nor_flash.len)
        {
            offset -= nor_flash.blk_size * 2;
        }

        tick1 = rt_tick_get_millisecond();
        fal_nor_erase(offset, testsize);
        tick2 = rt_tick_get_millisecond();
        LOG_I("erase %08p %d use %u ms", offset, testsize, tick2 - tick1);

        fal_nor_read (offset, (uint8_t *)rbuf, testsize);
        if (is_all_equal(rbuf, testsize, 0xff) == 0)
        {
            LOG_E("block %08p %d failed!", offset, testsize);
            ret = -RT_ERROR;
        }
        tick3 = rt_tick_get_millisecond();
        LOG_I("block %08p %d use %u ms: %08x %08x", offset, testsize, tick3 - tick2, *(uint32_t *)&rbuf[0], *(uint32_t *)&rbuf[testsize-4]);

        fal_nor_read (offset + nor_flash.blk_size, (uint8_t *)rbuf, testsize);
        if (rt_memcmp(tbuf, rbuf, testsize) != 0)
        {
            LOG_E("check %08p %d failed!", offset + nor_flash.blk_size, testsize);
            ret = -RT_ERROR;
        }
        tick4 = rt_tick_get_millisecond();
        LOG_I("read  %08p %d use %u ms: %08x %08x", offset, testsize, tick4 - tick3, *(uint32_t *)&rbuf[0], *(uint32_t *)&rbuf[testsize-4]);

        rt_memset(rbuf, 0, testsize);
    }

    if (ret == RT_EOK)
    {
        LOG_I("norflash read write test succsess.");
    }
    else
    {
        LOG_E("norflash read write test failed!");
    }

    rt_free(tbuf);
    rt_free(rbuf);

    return ret;
}
MSH_CMD_EXPORT(norflash_test, nor flash test);
#endif

#endif
#endif
