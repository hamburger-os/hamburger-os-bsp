/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-09-09     lvhan       the first version
 */
#include "board.h"
#include <drv_config.h>
#include <drv_dma.h>

#ifdef BSP_USING_EMMC
#include "fal_cfg.h"
#include "drv_fal.h"

#define DBG_TAG "drv.emmc"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* stm32 sdio dirver class */
struct stm32_emmc
{
    const char *name;
    MMC_TypeDef *Instance;
    IRQn_Type irq_type;

    MMC_HandleTypeDef hmmc;
    struct rt_mutex hmmc_mutex;
    struct rt_completion hmmc_completion;

    uint8_t aTxBuffer[MMC_BLOCKSIZE];
    uint8_t aRxBuffer[MMC_BLOCKSIZE];

#ifdef BSP_SDIO_RX_USING_DMA
    struct dma_config *dma_rx;
    DMA_HandleTypeDef hdma_sdio_rx;
#endif

#ifdef BSP_SDIO_TX_USING_DMA
    struct dma_config *dma_tx;
    DMA_HandleTypeDef hdma_sdio_tx;
#endif
};

#ifdef BSP_SDIO_RX_USING_DMA
static struct dma_config emmc_dma_rx =
{
    .Instance = SDIO_RX_DMA_INSTANCE,
    .channel = SDIO_RX_DMA_CHANNEL,
    .dma_rcc = SDIO_RX_DMA_RCC,
    .dma_irq = SDIO_RX_DMA_IRQ,
};
#endif

#ifdef BSP_SDIO_TX_USING_DMA
static struct dma_config emmc_dma_tx =
{
    .Instance = SDIO_TX_DMA_INSTANCE,
    .channel = SDIO_TX_DMA_CHANNEL,
    .dma_rcc = SDIO_TX_DMA_RCC,
    .dma_irq = SDIO_TX_DMA_IRQ,
};
#endif

static struct stm32_emmc emmc_obj =
{
    .name = "emmc",
#ifdef SOC_SERIES_STM32H7
    .Instance = SDMMC1,
    .irq_type = SDMMC1_IRQn,
#else
    .Instance = SDIO,
    .irq_type = SDIO_IRQn,
#endif

#ifdef BSP_SDIO_RX_USING_DMA
    .dma_rx = &emmc_dma_rx,
#endif

#ifdef BSP_SDIO_TX_USING_DMA
    .dma_tx = &emmc_dma_tx,
#endif
};

/**
  * @brief This function handles SDIO global interrupt.
  */
#ifdef SOC_SERIES_STM32H7
void SDMMC1_IRQHandler(void)
#else
void SDIO_IRQHandler(void)
#endif
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_MMC_IRQHandler(&emmc_obj.hmmc);
    /* leave interrupt */
    rt_interrupt_leave();
}

/**
  * @brief This function handles DMA2 stream3 global interrupt.
  */
#ifdef BSP_SDIO_RX_USING_DMA
void SDIO_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&emmc_obj.hdma_sdio_rx);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

void HAL_MMC_RxCpltCallback(MMC_HandleTypeDef *hmmc)
{
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_InvalidateDCache_by_Addr((uint32_t*)emmc_obj.aRxBuffer, MMC_BLOCKSIZE);
#endif
    rt_completion_done(&emmc_obj.hmmc_completion);
}

/**
  * @brief This function handles DMA2 stream6 global interrupt.
  */
#ifdef BSP_SDIO_TX_USING_DMA
void SDIO_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&emmc_obj.hdma_sdio_tx);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

void HAL_MMC_TxCpltCallback(MMC_HandleTypeDef *hmmc)
{
    rt_completion_done(&emmc_obj.hmmc_completion);
}

//void HAL_MMC_ErrorCallback(MMC_HandleTypeDef *hmmc)
//{
//    LOG_E("ErrorCallback 0x%x", HAL_MMC_GetError(hmmc));
//}
//void HAL_MMC_AbortCallback(MMC_HandleTypeDef *hmmc)
//{
//    LOG_E("AbortCallback 0x%x", HAL_MMC_GetState(hmmc));
//}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{
    /* Peripheral clock enable */
#ifdef SOC_SERIES_STM32H7
    __HAL_RCC_SDMMC1_CLK_ENABLE();
#else
    __HAL_RCC_SDIO_CLK_ENABLE();
#endif

    /* SDIO interrupt Init */
    HAL_NVIC_SetPriority(emmc_obj.irq_type, 0, 0);
    HAL_NVIC_EnableIRQ(emmc_obj.irq_type);

#ifdef BSP_SDIO_RX_USING_DMA
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();
    /* SDIO_RX_DMA_IRQ interrupt configuration */
    HAL_NVIC_SetPriority(emmc_obj.dma_rx->dma_irq, 0, 0);
    HAL_NVIC_EnableIRQ(emmc_obj.dma_rx->dma_irq);

    /* SDIO DMA Init */
    /* SDIO_RX Init */
    emmc_obj.hdma_sdio_rx.Instance = emmc_obj.dma_rx->Instance;
    emmc_obj.hdma_sdio_rx.Init.Channel = emmc_obj.dma_rx->channel;
    emmc_obj.hdma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    emmc_obj.hdma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    emmc_obj.hdma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
    emmc_obj.hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    emmc_obj.hdma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    emmc_obj.hdma_sdio_rx.Init.Mode = DMA_PFCTRL;
    emmc_obj.hdma_sdio_rx.Init.Priority = DMA_PRIORITY_LOW;
    emmc_obj.hdma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    emmc_obj.hdma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    emmc_obj.hdma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
    emmc_obj.hdma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&emmc_obj.hdma_sdio_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&emmc_obj.hmmc, hdmarx, emmc_obj.hdma_sdio_rx);
#endif

#ifdef BSP_SDIO_TX_USING_DMA
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();
    /* SDIO_TX_DMA_IRQ interrupt configuration */
    HAL_NVIC_SetPriority(emmc_obj.dma_tx->dma_irq, 0, 0);
    HAL_NVIC_EnableIRQ(emmc_obj.dma_tx->dma_irq);

    /* SDIO_TX Init */
    emmc_obj.hdma_sdio_tx.Instance = emmc_obj.dma_tx->Instance;
    emmc_obj.hdma_sdio_tx.Init.Channel = emmc_obj.dma_tx->channel;
    emmc_obj.hdma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    emmc_obj.hdma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    emmc_obj.hdma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
    emmc_obj.hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    emmc_obj.hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    emmc_obj.hdma_sdio_tx.Init.Mode = DMA_PFCTRL;
    emmc_obj.hdma_sdio_tx.Init.Priority = DMA_PRIORITY_LOW;
    emmc_obj.hdma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    emmc_obj.hdma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    emmc_obj.hdma_sdio_tx.Init.MemBurst = DMA_MBURST_INC4;
    emmc_obj.hdma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&emmc_obj.hdma_sdio_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&emmc_obj.hmmc, hdmatx, emmc_obj.hdma_sdio_tx);
#endif
}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_MMC_Init(void)
{
#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) || defined (STM32F417xx) || \
    defined (STM32F427xx) || defined (STM32F437xx) || defined (STM32F429xx) || defined (STM32F439xx) || \
    defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) || \
    defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) || defined (STM32F469xx) || \
    defined (STM32F479xx) || defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || \
    defined (STM32F412Zx) || defined (STM32F413xx) || defined (STM32F423xx)

    emmc_obj.hmmc.Instance = emmc_obj.Instance;
    emmc_obj.hmmc.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    emmc_obj.hmmc.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    emmc_obj.hmmc.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    emmc_obj.hmmc.Init.BusWide = SDIO_BUS_WIDE_1B;
    emmc_obj.hmmc.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    emmc_obj.hmmc.Init.ClockDiv = 0;
    if (HAL_MMC_Init(&emmc_obj.hmmc) != HAL_OK)
    {
        Error_Handler();
    }
#ifdef BSP_SDIO_BUS_WIDE_4B
    if (HAL_MMC_ConfigWideBusOperation(&emmc_obj.hmmc, SDIO_BUS_WIDE_4B) != HAL_OK)
#endif
#ifdef BSP_SDIO_BUS_WIDE_8B
    if (HAL_MMC_ConfigWideBusOperation(&emmc_obj.hmmc, SDIO_BUS_WIDE_8B) != HAL_OK)
#endif
    {
        Error_Handler();
    }
#endif

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx)  || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx) || defined (STM32H747xx)  || defined (STM32H747xG) || defined (STM32H757xx)  || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx) || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ) || defined (STM32H725xx) || defined (STM32H723xx)

    emmc_obj.hmmc.Instance = emmc_obj.Instance;
    emmc_obj.hmmc.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    emmc_obj.hmmc.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    emmc_obj.hmmc.Init.BusWide = SDMMC_BUS_WIDE_1B;
    emmc_obj.hmmc.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    emmc_obj.hmmc.Init.ClockDiv = 1;
    if (HAL_MMC_Init(&emmc_obj.hmmc) != HAL_OK)
    {
        Error_Handler();
    }
#ifdef BSP_SDIO_BUS_WIDE_4B
    if (HAL_MMC_ConfigWideBusOperation(&emmc_obj.hmmc, SDMMC_BUS_WIDE_4B) != HAL_OK)
#endif
#ifdef BSP_SDIO_BUS_WIDE_8B
    if (HAL_MMC_ConfigWideBusOperation(&emmc_obj.hmmc, SDMMC_BUS_WIDE_8B) != HAL_OK)
#endif
    {
        Error_Handler();
    }
#endif
}

/**
  * @brief  Wait MMC Card ready status
  * @param  None
  * @retval None
  */
static int Wait_MMCCARD_Ready(void)
{
    rt_tick_t tick = rt_tick_get();
    int loop = 1024;

    /* Wait for the Erasing process is completed */
    /* Verify that MMC card is ready to use after the Erase */
    while(loop > 0)
    {
        if(HAL_MMC_GetCardState(&emmc_obj.hmmc) == HAL_MMC_CARD_TRANSFER)
        {
            if (loop < 512)
            {
                LOG_W("Wait MMCCARD Ready used %d ms", rt_tick_get() - tick);
            }
            return HAL_OK;
        }
        rt_thread_delay(1);
        loop--;
    }
    LOG_E("Wait MMCCARD Ready timeout %d ms", rt_tick_get() - tick);
    return -HAL_ERROR;
}

static int fal_emmc_init(void);

#if EMMC_OFFSET_FS > 0
static int fal_emmc_32read(long offset, uint8_t *buf, size_t size);
static int fal_emmc_32write(long offset, const uint8_t *buf, size_t size);
static int fal_emmc_32erase(long offset, size_t size);
#endif

static uint32_t fal_emmc_read(uint64_t offset, uint8_t *buf, uint32_t size);
static uint32_t fal_emmc_write(uint64_t offset, const uint8_t *buf, uint32_t size);
static uint32_t fal_emmc_erase(uint64_t offset, uint32_t size);

/* ===================== Flash device Configuration ========================= */
#if EMMC_OFFSET_FS > 0
const struct fal_flash_dev emmc_fal_flash =
{
    .name = EMMC_DEV_NAME,
    .addr = EMMC_START_ADRESS,
    .len = EMMC_OFFSET_FS,
    .blk_size = EMMC_BLK_SIZE,
    .ops = {fal_emmc_init, fal_emmc_32read, fal_emmc_32write, fal_emmc_32erase},
    .write_gran = 32,
};
#endif

struct fal_flash64_dev emmc_flash[] =
{
    {//总块设备
        .name = EMMC_DEV_NAME,
        .addr = EMMC_START_ADRESS,
        .len = 4 * 1024 * 1024,
        .blk_size = EMMC_BLK_SIZE,
        .ops = {fal_emmc_init, fal_emmc_read, fal_emmc_write, fal_emmc_erase},
        .write_gran = 32,
    },
    {//剩余挂载文件系统分区
        .name = BLK_EMMC,
        .addr = EMMC_START_ADRESS + EMMC_OFFSET_FS,
        .len = 4 * 1024 * 1024,
        .blk_size = EMMC_BLK_SIZE,
        .ops = {fal_emmc_init, fal_emmc_read, fal_emmc_write, fal_emmc_erase},
        .write_gran = 32,
    },
    {//结尾
        .name = "",
        .addr = 0,
        .len = 0,
        .blk_size = 0,
        .ops = {fal_emmc_init, fal_emmc_read, fal_emmc_write, fal_emmc_erase},
        .write_gran = 32,
    },
};

static void emmc_thread_entry(void* parameter)
{
    uint8_t data[EMMC_BLK_SIZE];
    while(1)
    {
        rt_thread_delay(1000*60);
        if (fal_emmc_read(0, data, EMMC_BLK_SIZE) != EMMC_BLK_SIZE)
        {
            LOG_E("heartbeat read error!");
        }
    }
}

#ifdef BSP_SDIO_USING_CTRL
static void emmc_ctrl(uint8_t onoff)
{
    rt_base_t pin = rt_pin_get(BSP_SDIO_CTRL_PIN);
    rt_pin_mode(pin, PIN_MODE_OUTPUT);

    rt_pin_write(pin, onoff);
    rt_thread_delay(100);
}
#endif

static uint8_t fal_emmc_isinit = 0;
static int fal_emmc_init(void)
{
    if (fal_emmc_isinit == 1)
        return RT_EOK;
    fal_emmc_isinit = 1;
#ifdef BSP_SDIO_USING_CTRL
    emmc_ctrl(1);
#endif
    MX_DMA_Init();
    MX_SDIO_MMC_Init();

    /* 初始化互斥 */
    rt_mutex_init(&emmc_obj.hmmc_mutex, "emmc", RT_IPC_FLAG_PRIO);
    /* 初始化完成量 */
    rt_completion_init(&emmc_obj.hmmc_completion);

    HAL_MMC_CardInfoTypeDef CardInfo;
    if (HAL_MMC_GetCardInfo(&emmc_obj.hmmc, &CardInfo) != HAL_OK)
    {
        Error_Handler();
    }
    emmc_flash[0].blk_size = CardInfo.LogBlockSize;
    emmc_flash[0].len = (uint64_t)CardInfo.LogBlockNbr * CardInfo.LogBlockSize;
    emmc_flash[1].blk_size = emmc_flash[0].blk_size;
    emmc_flash[1].len = emmc_flash[0].len - EMMC_OFFSET_FS;

    //触发心跳读取(当数据引脚有上拉时，不再需要心跳读取即可保持总线正确)
    rt_thread_t emmc_thread = rt_thread_create( "emmc",
                                     emmc_thread_entry,
                                     NULL,
                                     2048,
                                     RT_THREAD_PRIORITY_MAX-3,
                                     10);
    if ( emmc_thread != RT_NULL)
    {
        rt_thread_startup(emmc_thread);
    }

    LOG_I("init succeed %d MB [ %d block ]"
        , (uint32_t)(emmc_flash[0].len / 1024 / 1024)
        , emmc_flash[0].blk_size);

    return RT_EOK;
}

#if EMMC_OFFSET_FS > 0
static int fal_emmc_32read(long offset, uint8_t *buffer, size_t size)
{
    rt_mutex_take(&emmc_obj.hmmc_mutex, RT_WAITING_FOREVER);
    HAL_StatusTypeDef ret = HAL_OK;
    uint32_t addr = emmc_fal_flash.addr + offset;
    if (addr + size > emmc_fal_flash.addr + emmc_fal_flash.len)
    {
        LOG_E("read outrange flash size! addr is (0x%x)", (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EINVAL;
    }
    if (size < emmc_fal_flash.blk_size)
    {
        LOG_W("read size %d! addr is (0x%x)", size, (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return 0;
    }
    LOG_D("read (0x%x) %d", (uint32_t)(addr), size);

    uint32_t blk_addr = addr/emmc_fal_flash.blk_size;
    uint32_t blk_size = size/emmc_fal_flash.blk_size;

#if defined(BSP_SDIO_RX_USING_DMA) || defined(SOC_SERIES_STM32H7)
    rt_size_t nblk = blk_size;
    while (nblk > 0)
    {
        ret = Wait_MMCCARD_Ready();
        if(ret != HAL_OK)
        {
            LOG_E("ReadBlocks Wait Error %d!", ret);
            rt_mutex_release(&emmc_obj.hmmc_mutex);
            return -RT_EIO;
        }

        rt_memset(emmc_obj.aRxBuffer, 0, MMC_BLOCKSIZE);
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanDCache_by_Addr((uint32_t*)emmc_obj.aRxBuffer, MMC_BLOCKSIZE);
#endif
        ret = HAL_MMC_ReadBlocks_DMA(&emmc_obj.hmmc, emmc_obj.aRxBuffer, blk_addr, 1);
        if(ret != HAL_OK)
        {
            LOG_E("ReadBlocks Error (0x%p) %d %d", blk_addr, 1, ret);
            rt_mutex_release(&emmc_obj.hmmc_mutex);
            return -RT_EIO;
        }

        if (rt_completion_wait(&emmc_obj.hmmc_completion, RT_WAITING_FOREVER) != HAL_OK)
        {
            LOG_E("ReadBlocks comple Error (0x%p) %d %d", blk_addr, 1, ret);
        }
        rt_memcpy(buffer, emmc_obj.aRxBuffer, MMC_BLOCKSIZE);
        LOG_HEX("rd", 16, (uint8_t *)buffer, MMC_BLOCKSIZE);

        nblk--;
        blk_addr++;
        buffer += MMC_BLOCKSIZE;
    }
#else
    ret = Wait_MMCCARD_Ready();
    if(ret != HAL_OK)
    {
        LOG_E("ReadBlocks Wait Error %d!", ret);
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EIO;
    }

    ret = HAL_MMC_ReadBlocks(&emmc_obj.hmmc, buffer, blk_addr, blk_size, 0xffff);
    if(ret != HAL_OK)
    {
        LOG_E("ReadBlocks Error (0x%p) %d %d", blk_addr, blk_size, ret);
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EIO;
    }
#endif

    rt_mutex_release(&emmc_obj.hmmc_mutex);
    return size;
}
#endif

static uint32_t fal_emmc_read(uint64_t offset, uint8_t *buffer, uint32_t size)
{
    rt_mutex_take(&emmc_obj.hmmc_mutex, RT_WAITING_FOREVER);
    HAL_StatusTypeDef ret = HAL_OK;
    uint64_t addr = emmc_flash[0].addr + offset;
    if (addr + size > emmc_flash[0].addr + emmc_flash[0].len)
    {
        LOG_E("read outrange flash size! addr is (0x%x %x)", (uint32_t)((addr + size) >> 32), (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EINVAL;
    }
    if (size < emmc_flash[0].blk_size)
    {
        LOG_W("read size %d! addr is (0x%x %x)", size, (uint32_t)((addr + size) >> 32), (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return 0;
    }
    LOG_D("read (0x%x %x) %d", (uint32_t)(addr) >> 32, (uint32_t)(addr), size);

    uint64_t blk_addr = addr/emmc_flash[0].blk_size;
    uint32_t blk_size = size/emmc_flash[0].blk_size;

#if defined(BSP_SDIO_RX_USING_DMA) || defined(SOC_SERIES_STM32H7)
    rt_size_t nblk = blk_size;
    while (nblk > 0)
    {
        ret = Wait_MMCCARD_Ready();
        if(ret != HAL_OK)
        {
            LOG_E("ReadBlocks Wait Error %d!", ret);
            rt_mutex_release(&emmc_obj.hmmc_mutex);
            return -RT_EIO;
        }

        rt_memset(emmc_obj.aRxBuffer, 0, MMC_BLOCKSIZE);
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanDCache_by_Addr((uint32_t*)emmc_obj.aRxBuffer, MMC_BLOCKSIZE);
#endif
        ret = HAL_MMC_ReadBlocks_DMA(&emmc_obj.hmmc, emmc_obj.aRxBuffer, blk_addr, 1);
        if(ret != HAL_OK)
        {
            LOG_E("ReadBlocks Error (0x%p) %d %d", blk_addr, 1, ret);
            rt_mutex_release(&emmc_obj.hmmc_mutex);
            return -RT_EIO;
        }

        if (rt_completion_wait(&emmc_obj.hmmc_completion, RT_WAITING_FOREVER) != HAL_OK)
        {
            LOG_E("ReadBlocks comple Error (0x%p) %d %d", blk_addr, 1, ret);
        }
        rt_memcpy(buffer, emmc_obj.aRxBuffer, MMC_BLOCKSIZE);
        LOG_HEX("rd", 16, (uint8_t *)buffer, MMC_BLOCKSIZE);

        nblk--;
        blk_addr++;
        buffer += MMC_BLOCKSIZE;
    }
#else
    ret = Wait_MMCCARD_Ready();
    if(ret != HAL_OK)
    {
        LOG_E("ReadBlocks Wait Error %d!", ret);
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EIO;
    }

    ret = HAL_MMC_ReadBlocks(&emmc_obj.hmmc, buffer, blk_addr, blk_size, 0xffff);
    if(ret != HAL_OK)
    {
        LOG_E("ReadBlocks Error (0x%p) %d %d", blk_addr, blk_size, ret);
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EIO;
    }
#endif

    rt_mutex_release(&emmc_obj.hmmc_mutex);
    return size;
}

#if EMMC_OFFSET_FS > 0
static int fal_emmc_32write(long offset, const uint8_t *buffer, size_t size)
{
    rt_mutex_take(&emmc_obj.hmmc_mutex, RT_WAITING_FOREVER);
    HAL_StatusTypeDef ret = HAL_OK;
    uint32_t addr = emmc_fal_flash.addr + offset;
    if (addr + size > emmc_fal_flash.addr + emmc_fal_flash.len)
    {
        LOG_E("write outrange flash size! addr is (0x%x)", (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EINVAL;
    }
    if (size < emmc_fal_flash.blk_size)
    {
        LOG_W("write size %d! addr is (0x%x)", size, (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return 0;
    }
    LOG_D("write (0x%x) %d", (uint32_t)(addr), size);

    uint32_t blk_addr = addr/emmc_fal_flash.blk_size;
    uint32_t blk_size = size/emmc_fal_flash.blk_size;

#if defined(BSP_SDIO_TX_USING_DMA) || defined(SOC_SERIES_STM32H7)
    rt_size_t nblk = blk_size;
    while (nblk > 0)
    {
        ret = Wait_MMCCARD_Ready();
        if(ret != HAL_OK)
        {
            LOG_E("WriteBlocks Wait Error %d!", ret);
            rt_mutex_release(&emmc_obj.hmmc_mutex);
            return -RT_EIO;
        }

        rt_memcpy(emmc_obj.aTxBuffer, buffer, MMC_BLOCKSIZE);
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanDCache_by_Addr((uint32_t*)emmc_obj.aTxBuffer, MMC_BLOCKSIZE);
#endif
        ret = HAL_MMC_WriteBlocks_DMA(&emmc_obj.hmmc, (uint8_t *)emmc_obj.aTxBuffer, blk_addr, 1);
        if(ret != HAL_OK)
        {
            LOG_E("WriteBlocks Error (0x%p) %d %d", blk_addr, 1, ret);
            rt_mutex_release(&emmc_obj.hmmc_mutex);
            return -RT_EIO;
        }

        if (rt_completion_wait(&emmc_obj.hmmc_completion, RT_WAITING_FOREVER) != HAL_OK)
        {
            LOG_E("WriteBlocks comple Error (0x%p) %d %d", blk_addr, 1, ret);
        }
        LOG_HEX("wr", 16, (uint8_t *)buffer, MMC_BLOCKSIZE);

        nblk--;
        blk_addr++;
        buffer += MMC_BLOCKSIZE;
    }
#else
    ret = Wait_MMCCARD_Ready();
    if(ret != HAL_OK)
    {
        LOG_E("WriteBlocks Wait Error %d!", ret);
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EIO;
    }

    ret = HAL_MMC_WriteBlocks(&emmc_obj.hmmc, (uint8_t *)buffer, blk_addr, blk_size, 0xffff);
    if(ret != HAL_OK)
    {
        LOG_E("WriteBlocks Error (0x%p) %d %d", blk_addr, blk_size, ret);
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EIO;
    }
#endif

    rt_mutex_release(&emmc_obj.hmmc_mutex);
    return size;
}
#endif

static uint32_t fal_emmc_write(uint64_t offset, const rt_uint8_t *buffer, uint32_t size)
{
    rt_mutex_take(&emmc_obj.hmmc_mutex, RT_WAITING_FOREVER);
    HAL_StatusTypeDef ret = HAL_OK;
    uint64_t addr = emmc_flash[0].addr + offset;
    if (addr + size > emmc_flash[0].addr + emmc_flash[0].len)
    {
        LOG_E("write outrange flash size! addr is (0x%x %x)", (uint32_t)((addr + size) >> 32), (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EINVAL;
    }
    if (size < emmc_flash[0].blk_size)
    {
        LOG_W("write size %d! addr is (0x%x %x)", size, (uint32_t)((addr + size) >> 32), (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return 0;
    }
    LOG_D("write (0x%x %x) %d", (uint32_t)(addr) >> 32, (uint32_t)(addr), size);

    uint64_t blk_addr = addr/emmc_flash[0].blk_size;
    uint32_t blk_size = size/emmc_flash[0].blk_size;

#if defined(BSP_SDIO_TX_USING_DMA) || defined(SOC_SERIES_STM32H7)
    rt_size_t nblk = blk_size;
    while (nblk > 0)
    {
        ret = Wait_MMCCARD_Ready();
        if(ret != HAL_OK)
        {
            LOG_E("WriteBlocks Wait Error %d!", ret);
            rt_mutex_release(&emmc_obj.hmmc_mutex);
            return -RT_EIO;
        }

        rt_memcpy(emmc_obj.aTxBuffer, buffer, MMC_BLOCKSIZE);
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanDCache_by_Addr((uint32_t*)emmc_obj.aTxBuffer, MMC_BLOCKSIZE);
#endif
        ret = HAL_MMC_WriteBlocks_DMA(&emmc_obj.hmmc, (uint8_t *)emmc_obj.aTxBuffer, blk_addr, 1);
        if(ret != HAL_OK)
        {
            LOG_E("WriteBlocks Error (0x%p) %d %d", blk_addr, 1, ret);
            rt_mutex_release(&emmc_obj.hmmc_mutex);
            return -RT_EIO;
        }

        if (rt_completion_wait(&emmc_obj.hmmc_completion, RT_WAITING_FOREVER) != HAL_OK)
        {
            LOG_E("WriteBlocks comple Error (0x%p) %d %d", blk_addr, 1, ret);
        }
        LOG_HEX("wr", 16, (uint8_t *)buffer, MMC_BLOCKSIZE);

        nblk--;
        blk_addr++;
        buffer += MMC_BLOCKSIZE;
    }
#else
    ret = Wait_MMCCARD_Ready();
    if(ret != HAL_OK)
    {
        LOG_E("WriteBlocks Wait Error %d!", ret);
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EIO;
    }

    ret = HAL_MMC_WriteBlocks(&emmc_obj.hmmc, (uint8_t *)buffer, blk_addr, blk_size, 0xffff);
    if(ret != HAL_OK)
    {
        LOG_E("WriteBlocks Error (0x%p) %d %d", blk_addr, blk_size, ret);
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EIO;
    }
#endif

    rt_mutex_release(&emmc_obj.hmmc_mutex);
    return size;
}

#if EMMC_OFFSET_FS > 0
static int fal_emmc_32erase(long offset, size_t size)
{
    rt_mutex_take(&emmc_obj.hmmc_mutex, RT_WAITING_FOREVER);
    uint32_t addr = emmc_fal_flash.addr + offset;
    if ((addr + size) > emmc_fal_flash.addr + emmc_fal_flash.len)
    {
        LOG_E("erase outrange flash size! addr is (0x%x)", (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EINVAL;
    }
    if (size < emmc_fal_flash.blk_size)
    {
        LOG_W("erase size %d! addr is (0x%x)", size, (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return 0;
    }
    LOG_D("erase (0x%x) %d", (uint32_t)(addr), size);

    rt_mutex_release(&emmc_obj.hmmc_mutex);
    return size;
}
#endif

static uint32_t fal_emmc_erase(uint64_t offset, uint32_t size)
{
    rt_mutex_take(&emmc_obj.hmmc_mutex, RT_WAITING_FOREVER);
    uint64_t addr = emmc_flash[0].addr + offset;
    if ((addr + size) > emmc_flash[0].addr + emmc_flash[0].len)
    {
        LOG_E("erase outrange flash size! addr is (0x%x %x)", (uint32_t)((addr + size) >> 32), (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return -RT_EINVAL;
    }
    if (size < emmc_flash[0].blk_size)
    {
        LOG_W("erase size %d! addr is (0x%x %x)", size, (uint32_t)((addr + size) >> 32), (uint32_t)(addr + size));
        rt_mutex_release(&emmc_obj.hmmc_mutex);
        return 0;
    }
    LOG_D("erase (0x%x %x) %d", (uint32_t)(addr) >> 32, (uint32_t)(addr), size);

    rt_mutex_release(&emmc_obj.hmmc_mutex);
    return size;
}

#endif
