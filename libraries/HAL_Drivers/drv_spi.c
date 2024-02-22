/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-5      SummerGift   first version
 * 2018-12-11     greedyhao    Porting for stm32f7xx
 * 2019-01-03     zylx         modify DMA initialization and spixfer function
 * 2020-01-15     whj4674672   Porting for stm32h7xx
 * 2020-06-18     thread-liu   Porting for stm32mp1xx
 * 2020-10-14     Dozingfiretruck   Porting for stm32wbxx
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"

#ifdef BSP_USING_SPI

#if defined(BSP_USING_SPI1) || defined(BSP_USING_SPI2) || defined(BSP_USING_SPI3) || defined(BSP_USING_SPI4) || defined(BSP_USING_SPI5) || defined(BSP_USING_SPI6)

#include "drv_spi.h"
#include "drv_config.h"
#include <string.h>
#include <math.h>

//#define DRV_DEBUG
#define LOG_TAG              "drv.spi"
#include <drv_log.h>

enum
{
#ifdef BSP_USING_SPI1
    SPI1_INDEX,
#endif
#ifdef BSP_USING_SPI2
    SPI2_INDEX,
#endif
#ifdef BSP_USING_SPI3
    SPI3_INDEX,
#endif
#ifdef BSP_USING_SPI4
    SPI4_INDEX,
#endif
#ifdef BSP_USING_SPI5
    SPI5_INDEX,
#endif
#ifdef BSP_USING_SPI6
    SPI6_INDEX,
#endif
};

static struct stm32_spi_config spi_config[] =
{
#ifdef BSP_USING_SPI1
    SPI1_BUS_CONFIG,
#endif

#ifdef BSP_USING_SPI2
    SPI2_BUS_CONFIG,
#endif

#ifdef BSP_USING_SPI3
    SPI3_BUS_CONFIG,
#endif

#ifdef BSP_USING_SPI4
    SPI4_BUS_CONFIG,
#endif

#ifdef BSP_USING_SPI5
    SPI5_BUS_CONFIG,
#endif

#ifdef BSP_USING_SPI6
    SPI6_BUS_CONFIG,
#endif
};

static struct stm32_spi spi_bus_obj[sizeof(spi_config) / sizeof(spi_config[0])] = {0};

static rt_err_t stm32_spi_init(struct stm32_spi *spi_drv, struct rt_spi_configuration *cfg)
{
    RT_ASSERT(spi_drv != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    SPI_HandleTypeDef *spi_handle = &spi_drv->handle;

    if (cfg->mode & RT_SPI_SLAVE)
    {
        spi_handle->Init.Mode = SPI_MODE_SLAVE;
    }
    else
    {
        spi_handle->Init.Mode = SPI_MODE_MASTER;
    }

    if (cfg->mode & RT_SPI_3WIRE)
    {
        spi_handle->Init.Direction = SPI_DIRECTION_1LINE;
    }
    else
    {
        spi_handle->Init.Direction = SPI_DIRECTION_2LINES;
    }

    if (cfg->data_width == 8)
    {
        spi_handle->Init.DataSize = SPI_DATASIZE_8BIT;
    }
    else if (cfg->data_width == 16)
    {
        spi_handle->Init.DataSize = SPI_DATASIZE_16BIT;
    }
    else
    {
        return -RT_EIO;
    }

    if (cfg->mode & RT_SPI_CPHA)
    {
        spi_handle->Init.CLKPhase = SPI_PHASE_2EDGE;
    }
    else
    {
        spi_handle->Init.CLKPhase = SPI_PHASE_1EDGE;
    }

    if (cfg->mode & RT_SPI_CPOL)
    {
        spi_handle->Init.CLKPolarity = SPI_POLARITY_HIGH;
    }
    else
    {
        spi_handle->Init.CLKPolarity = SPI_POLARITY_LOW;
    }

    spi_handle->Init.NSS = SPI_NSS_SOFT;

    uint32_t SPI_CLOCK = 0UL;
    /* Some series may only have APBPERIPH_BASE, but don't have HAL_RCC_GetPCLK2Freq */
#if defined(APBPERIPH_BASE)
    SPI_CLOCK = HAL_RCC_GetPCLK1Freq();
#elif defined(APB1PERIPH_BASE) || defined(APB2PERIPH_BASE)
    /* The SPI clock for H7 cannot be configured with a peripheral bus clock, so it needs to be written separately */
#if defined(SOC_SERIES_STM32H7)
    /* When the configuration is generated using CUBEMX, the configuration for the SPI clock is placed in the HAL_SPI_Init function.
    Therefore, it is necessary to initialize and configure the SPI clock to automatically configure the frequency division */
    HAL_SPI_Init(spi_handle);
    if (spi_handle->Instance == SPI1 || spi_handle->Instance == SPI2 || spi_handle->Instance == SPI3)
    {
    SPI_CLOCK = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI123);
    }
    else if (spi_handle->Instance == SPI4 || spi_handle->Instance == SPI5)
    {
        SPI_CLOCK = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI45);
    }
    else if (spi_handle->Instance == SPI6)
    {
        SPI_CLOCK = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI6);
    }
#else
    if ((rt_uint32_t)spi_drv->config->Instance >= APB2PERIPH_BASE)
    {
        SPI_CLOCK = HAL_RCC_GetPCLK2Freq();
    }
    else
    {
        SPI_CLOCK = HAL_RCC_GetPCLK1Freq();
    }
#endif /* SOC_SERIES_STM32H7) */
#endif /* APBPERIPH_BASE */

    if (cfg->max_hz >= SPI_CLOCK / 2)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    }
    else if (cfg->max_hz >= SPI_CLOCK / 4)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    }
    else if (cfg->max_hz >= SPI_CLOCK / 8)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    }
    else if (cfg->max_hz >= SPI_CLOCK / 16)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    }
    else if (cfg->max_hz >= SPI_CLOCK / 32)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    }
    else if (cfg->max_hz >= SPI_CLOCK / 64)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    }
    else if (cfg->max_hz >= SPI_CLOCK / 128)
    {
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    }
    else
    {
        /*  min prescaler 256 */
        spi_handle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    }

    LOG_D("sys freq: %d, pclk freq: %d, SPI limiting freq: %d, SPI usage freq: %d",
#if defined(SOC_SERIES_STM32MP1)
          HAL_RCC_GetSystemCoreClockFreq(),
#else
          HAL_RCC_GetSysClockFreq(),
#endif
          SPI_CLOCK,
          cfg->max_hz,
          SPI_CLOCK / (rt_size_t)pow(2,(spi_handle->Init.BaudRatePrescaler >> 28) + 1));

    if (cfg->mode & RT_SPI_MSB)
    {
        spi_handle->Init.FirstBit = SPI_FIRSTBIT_MSB;
    }
    else
    {
        spi_handle->Init.FirstBit = SPI_FIRSTBIT_LSB;
    }

    spi_handle->Init.TIMode = SPI_TIMODE_DISABLE;
    spi_handle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi_handle->State = HAL_SPI_STATE_RESET;
#if defined(SOC_SERIES_STM32L4) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32WB)
    spi_handle->Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
#elif defined(SOC_SERIES_STM32H7) || defined(SOC_SERIES_STM32MP1)
    spi_handle->Init.Mode                       = SPI_MODE_MASTER;
    spi_handle->Init.NSS                        = SPI_NSS_SOFT;
    spi_handle->Init.NSSPMode                   = SPI_NSS_PULSE_DISABLE;
    spi_handle->Init.NSSPolarity                = SPI_NSS_POLARITY_LOW;
    spi_handle->Init.CRCPolynomial              = 0x0;
    spi_handle->Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ONE_PATTERN;
    spi_handle->Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ONE_PATTERN;
    spi_handle->Init.MasterSSIdleness           = SPI_MASTER_SS_IDLENESS_00CYCLE;
    spi_handle->Init.MasterInterDataIdleness    = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    spi_handle->Init.MasterReceiverAutoSusp     = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    spi_handle->Init.MasterKeepIOState          = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    spi_handle->Init.IOSwap                     = SPI_IO_SWAP_DISABLE;
    spi_handle->Init.FifoThreshold              = SPI_FIFO_THRESHOLD_04DATA;
#endif

    if (HAL_SPI_Init(spi_handle) != HAL_OK)
    {
        return -RT_EIO;
    }

#if defined(SOC_SERIES_STM32L4) || defined(SOC_SERIES_STM32F0) \
        || defined(SOC_SERIES_STM32F7) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32WB)
    SET_BIT(spi_handle->Instance->CR2, SPI_RXFIFO_THRESHOLD_HF);
#endif

    /* DMA configuration */
    if (spi_drv->spi_dma_flag & SPI_USING_RX_DMA_FLAG)
    {
        HAL_DMA_Init(&spi_drv->dma.handle_rx);

        __HAL_LINKDMA(&spi_drv->handle, hdmarx, spi_drv->dma.handle_rx);

        /* NVIC configuration for DMA transfer complete interrupt */
        HAL_NVIC_SetPriority(spi_drv->config->dma_rx->dma_irq, 0, 0);
        HAL_NVIC_EnableIRQ(spi_drv->config->dma_rx->dma_irq);
    }

    if (spi_drv->spi_dma_flag & SPI_USING_TX_DMA_FLAG)
    {
        HAL_DMA_Init(&spi_drv->dma.handle_tx);

        __HAL_LINKDMA(&spi_drv->handle, hdmatx, spi_drv->dma.handle_tx);

        /* NVIC configuration for DMA transfer complete interrupt */
        HAL_NVIC_SetPriority(spi_drv->config->dma_tx->dma_irq, 1, 0);
        HAL_NVIC_EnableIRQ(spi_drv->config->dma_tx->dma_irq);
    }

    HAL_NVIC_SetPriority(spi_drv->config->irq_type, 2, 0);
    HAL_NVIC_EnableIRQ(spi_drv->config->irq_type);

    LOG_D("%s init done", spi_drv->config->bus_name);
    return RT_EOK;
}

#define ALIGN_SIZE(x)   (((x + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1)) + 32)
#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
static rt_ssize_t spixfer(struct rt_spi_device *device, struct rt_spi_message *message)
#else
static rt_uint32_t spixfer(struct rt_spi_device *device, struct rt_spi_message *message)
#endif
{
    #define DMA_TRANS_MIN_LEN  16 /* only buffer length >= DMA_TRANS_MIN_LEN will use DMA mode */

    HAL_StatusTypeDef state = HAL_OK;
    rt_size_t message_length, already_send_length;
    rt_uint16_t send_length;
    rt_uint8_t *recv_buf;
    const rt_uint8_t *send_buf;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    RT_ASSERT(message != RT_NULL);

    struct stm32_spi *spi_drv =  rt_container_of(device->bus, struct stm32_spi, spi_bus);
    SPI_HandleTypeDef *spi_handle = &spi_drv->handle;

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
    if (message->cs_take && !(device->config.mode & RT_SPI_NO_CS) && (device->cs_pin != PIN_NONE))
    {
        if (device->config.mode & RT_SPI_CS_HIGH)
            rt_pin_write(device->cs_pin, PIN_HIGH);
        else
            rt_pin_write(device->cs_pin, PIN_LOW);
    }
#else
    rt_base_t *cs = device->parent.user_data;
    if (message->cs_take && !(device->config.mode & RT_SPI_NO_CS))
    {
        if (device->config.mode & RT_SPI_CS_HIGH)
            rt_pin_write(*cs, PIN_HIGH);
        else
            rt_pin_write(*cs, PIN_LOW);
    }
#endif
    rt_hw_us_delay(4);//TODO：触摸屏驱动需要用这个延时？加上就好了

    LOG_D("%s transfer prepare and start", spi_drv->config->bus_name);
    LOG_D("%s sendbuf: %X, recvbuf: %X, length: %d",
          spi_drv->config->bus_name,
          (uint32_t)message->send_buf,
          (uint32_t)message->recv_buf, message->length);

    message_length = message->length;
    recv_buf = message->recv_buf;
    send_buf = message->send_buf;
    while (message_length)
    {
        /* the HAL library use uint16 to save the data length */
        if (message_length > 65535)
        {
            send_length = 65535;
            message_length = message_length - 65535;
        }
        else
        {
            send_length = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_send_length = message->length - send_length - message_length;
        /* avoid null pointer problems */
        if (message->send_buf)
        {
            send_buf = (rt_uint8_t *)message->send_buf + already_send_length;
        }
        if (message->recv_buf)
        {
            recv_buf = (rt_uint8_t *)message->recv_buf + already_send_length;
        }

        rt_uint32_t* dma_aligned_buffer = RT_NULL;
        rt_uint32_t* p_txrx_buffer = RT_NULL;

        if ((spi_drv->spi_dma_flag & SPI_USING_TX_DMA_FLAG) && (send_length >= DMA_TRANS_MIN_LEN))
        {
#if defined(SOC_SERIES_STM32H7) || defined(SOC_SERIES_STM32F7)
            if (RT_IS_ALIGN((rt_uint32_t)send_buf, 32)) /* aligned with 32 bytes? */
            {
                p_txrx_buffer = (rt_uint32_t *)send_buf; /* send_buf aligns with 32 bytes, no more operations */
            }
            else
            {
                /* send_buf doesn't align with 32 bytes, so creat a cache buffer with 32 bytes aligned */
                dma_aligned_buffer = (rt_uint32_t *)rt_malloc_align(send_length, 32);
                rt_memcpy(dma_aligned_buffer, send_buf, send_length);
                p_txrx_buffer = dma_aligned_buffer;
            }
#else
            if (RT_IS_ALIGN((rt_uint32_t)send_buf, 4)) /* aligned with 4 bytes? */
            {
                p_txrx_buffer = (rt_uint32_t *)send_buf; /* send_buf aligns with 4 bytes, no more operations */
            }
            else
            {
                /* send_buf doesn't align with 4 bytes, so creat a cache buffer with 4 bytes aligned */
                dma_aligned_buffer = (rt_uint32_t *)rt_malloc(send_length); /* aligned with RT_ALIGN_SIZE (8 bytes by default) */
                rt_memcpy(dma_aligned_buffer, send_buf, send_length);
                p_txrx_buffer = dma_aligned_buffer;
            }
#endif /* SOC_SERIES_STM32H7 || SOC_SERIES_STM32F7 */
        }

        /* start once data exchange in DMA mode */
        if (message->send_buf && message->recv_buf)
        {
            rt_memset((uint8_t *)recv_buf, 0xff, send_length);
            if ((spi_drv->spi_dma_flag & SPI_USING_TX_DMA_FLAG) && (spi_drv->spi_dma_flag & SPI_USING_RX_DMA_FLAG) && (send_length >= DMA_TRANS_MIN_LEN))
            {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
                SCB_CleanInvalidateDCache_by_Addr((uint32_t *)dma_aligned_buffer, ALIGN_SIZE(send_length));
#endif
                state = HAL_SPI_TransmitReceive_DMA(spi_handle, (uint8_t *)p_txrx_buffer, (uint8_t *)p_txrx_buffer, send_length);
            }
            else if ((spi_drv->spi_dma_flag & SPI_USING_TX_DMA_FLAG) && (send_length >= DMA_TRANS_MIN_LEN))
            {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
                SCB_CleanInvalidateDCache_by_Addr((uint32_t *)dma_aligned_buffer, ALIGN_SIZE(send_length));
#endif
                /* same as Tx ONLY. It will not receive SPI data any more. */
                state = HAL_SPI_Transmit_DMA(spi_handle, (uint8_t *)p_txrx_buffer, send_length);
            }
            else if ((spi_drv->spi_dma_flag & SPI_USING_RX_DMA_FLAG) && (send_length >= DMA_TRANS_MIN_LEN))
            {
                state = HAL_ERROR;
                LOG_E("It shoule be enabled both BSP_SPIx_TX_USING_DMA and BSP_SPIx_TX_USING_DMA flag, if wants to use SPI DMA Rx singly.");
                break;
            }
            else
            {
                state = HAL_SPI_TransmitReceive_IT(spi_handle, (uint8_t *)send_buf, (uint8_t *)recv_buf, send_length);
            }
        }
        else if (message->send_buf)
        {
            if ((spi_drv->spi_dma_flag & SPI_USING_TX_DMA_FLAG) && (send_length >= DMA_TRANS_MIN_LEN))
            {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
                SCB_CleanInvalidateDCache_by_Addr((uint32_t *)dma_aligned_buffer, ALIGN_SIZE(send_length));
#endif
                state = HAL_SPI_Transmit_DMA(spi_handle, (uint8_t *)p_txrx_buffer, send_length);
            }
            else
            {
                state = HAL_SPI_Transmit_IT(spi_handle, (uint8_t *)send_buf, send_length);
            }

            if (message->cs_release && (device->config.mode & RT_SPI_3WIRE))
            {
                /* release the CS by disable SPI when using 3 wires SPI */
                __HAL_SPI_DISABLE(spi_handle);
            }
        }
        else if(message->recv_buf)
        {
            rt_memset((uint8_t *)recv_buf, 0xff, send_length);
            if ((spi_drv->spi_dma_flag & SPI_USING_RX_DMA_FLAG) && (send_length >= DMA_TRANS_MIN_LEN))
            {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
                SCB_CleanInvalidateDCache_by_Addr((uint32_t *)dma_aligned_buffer, ALIGN_SIZE(send_length));
#endif
                state = HAL_SPI_Receive_DMA(spi_handle, (uint8_t *)p_txrx_buffer, send_length);
            }
            else
            {
                //h7 hal 的问题，rx api会报ovr错误
                state = HAL_SPI_TransmitReceive_IT(spi_handle, (uint8_t *)recv_buf, (uint8_t *)recv_buf, send_length);
            }
        }
        else
        {
            state = HAL_ERROR;
            LOG_E("message->send_buf and message->recv_buf are both NULL!");
        }

        if (state != HAL_OK)
        {
            LOG_E("SPI transfer error: %d 0x%p 0x%p 0x%p %d", state, send_buf, recv_buf, p_txrx_buffer, send_length);
            break;
        }
        else
        {
            LOG_D("%s transfer done", spi_drv->config->bus_name);
        }

        /* blocking the thread,and the other tasks can run */
        if (rt_completion_wait(&spi_drv->cpt, send_length * 4) != RT_EOK)
        {
            state = HAL_TIMEOUT;
            LOG_E("SPI transfer timeout: 0x%p 0x%p 0x%p %d", send_buf, recv_buf, p_txrx_buffer, send_length);
            break;
        }

        if(dma_aligned_buffer != RT_NULL) /* re-aligned, so need to copy the data to recv_buf */
        {
            if(recv_buf != RT_NULL)
            {
                rt_memcpy(recv_buf, p_txrx_buffer, send_length);
            }
#if defined(SOC_SERIES_STM32H7) || defined(SOC_SERIES_STM32F7)
            rt_free_align(dma_aligned_buffer);
#else
            rt_free(dma_aligned_buffer);
#endif /* SOC_SERIES_STM32H7 || SOC_SERIES_STM32F7 */
        }
    }

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
    if (message->cs_release && !(device->config.mode & RT_SPI_NO_CS) && (device->cs_pin != PIN_NONE))
    {
        if (device->config.mode & RT_SPI_CS_HIGH)
            rt_pin_write(device->cs_pin, PIN_LOW);
        else
            rt_pin_write(device->cs_pin, PIN_HIGH);
    }
#else
    if (message->cs_release && !(device->config.mode & RT_SPI_NO_CS))
    {
        if (device->config.mode & RT_SPI_CS_HIGH)
            rt_pin_write(*cs, PIN_LOW);
        else
            rt_pin_write(*cs, PIN_HIGH);
    }
#endif

    if(state != HAL_OK)
    {
        return -RT_ERROR;
    }
    return message->length;
}

static rt_err_t spi_configure(struct rt_spi_device *device,
                              struct rt_spi_configuration *configuration)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);

    struct stm32_spi *spi_drv =  rt_container_of(device->bus, struct stm32_spi, spi_bus);
    spi_drv->cfg = configuration;

    return stm32_spi_init(spi_drv, configuration);
}

static const struct rt_spi_ops stm_spi_ops =
{
    .configure = spi_configure,
    .xfer = spixfer,
};

static int rt_hw_spi_bus_init(void)
{
    rt_err_t result;

    for (rt_size_t i = 0; i < sizeof(spi_config) / sizeof(spi_config[0]); i++)
    {
        spi_bus_obj[i].config = &spi_config[i];
        spi_bus_obj[i].spi_bus.parent.user_data = &spi_config[i];
        spi_bus_obj[i].handle.Instance = spi_config[i].Instance;

        if (spi_bus_obj[i].spi_dma_flag & SPI_USING_RX_DMA_FLAG)
        {
            /* Configure the DMA handler for Transmission process */
            spi_bus_obj[i].dma.handle_rx.Instance = spi_config[i].dma_rx->Instance;
#if defined(SOC_SERIES_STM32F2) || defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7)
            spi_bus_obj[i].dma.handle_rx.Init.Channel = spi_config[i].dma_rx->channel;
#elif defined(SOC_SERIES_STM32L4) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32MP1) || defined(SOC_SERIES_STM32WB) || defined(SOC_SERIES_STM32H7)
            spi_bus_obj[i].dma.handle_rx.Init.Request = spi_config[i].dma_rx->request;
#endif
#ifndef SOC_SERIES_STM32U5
            spi_bus_obj[i].dma.handle_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
            spi_bus_obj[i].dma.handle_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
            spi_bus_obj[i].dma.handle_rx.Init.MemInc              = DMA_MINC_ENABLE;
            spi_bus_obj[i].dma.handle_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            spi_bus_obj[i].dma.handle_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
            spi_bus_obj[i].dma.handle_rx.Init.Mode                = DMA_NORMAL;
            spi_bus_obj[i].dma.handle_rx.Init.Priority            = DMA_PRIORITY_HIGH;
#endif
#if defined(SOC_SERIES_STM32F2) || defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7) || defined(SOC_SERIES_STM32MP1) || defined(SOC_SERIES_STM32H7)
            spi_bus_obj[i].dma.handle_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
            spi_bus_obj[i].dma.handle_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
            spi_bus_obj[i].dma.handle_rx.Init.MemBurst            = DMA_MBURST_INC4;
            spi_bus_obj[i].dma.handle_rx.Init.PeriphBurst         = DMA_PBURST_INC4;
#endif

            {
                rt_uint32_t tmpreg = 0x00U;
#if defined(SOC_SERIES_STM32F1) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32F0)
                /* enable DMA clock && Delay after an RCC peripheral clock enabling*/
                SET_BIT(RCC->AHBENR, spi_config[i].dma_rx->dma_rcc);
                tmpreg = READ_BIT(RCC->AHBENR, spi_config[i].dma_rx->dma_rcc);
#elif defined(SOC_SERIES_STM32F2) || defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7) || defined(SOC_SERIES_STM32L4) || defined(SOC_SERIES_STM32WB) || defined(SOC_SERIES_STM32H7)
                SET_BIT(RCC->AHB1ENR, spi_config[i].dma_rx->dma_rcc);
                /* Delay after an RCC peripheral clock enabling */
                tmpreg = READ_BIT(RCC->AHB1ENR, spi_config[i].dma_rx->dma_rcc);
#elif defined(SOC_SERIES_STM32MP1)
                __HAL_RCC_DMAMUX_CLK_ENABLE();
                SET_BIT(RCC->MP_AHB2ENSETR, spi_config[i].dma_rx->dma_rcc);
                tmpreg = READ_BIT(RCC->MP_AHB2ENSETR, spi_config[i].dma_rx->dma_rcc);
#endif
                UNUSED(tmpreg); /* To avoid compiler warnings */
            }
        }

        if (spi_bus_obj[i].spi_dma_flag & SPI_USING_TX_DMA_FLAG)
        {
            /* Configure the DMA handler for Transmission process */
            spi_bus_obj[i].dma.handle_tx.Instance = spi_config[i].dma_tx->Instance;
#if defined(SOC_SERIES_STM32F2) || defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7)
            spi_bus_obj[i].dma.handle_tx.Init.Channel = spi_config[i].dma_tx->channel;
#elif defined(SOC_SERIES_STM32L4) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32MP1) || defined(SOC_SERIES_STM32WB) || defined(SOC_SERIES_STM32H7)
            spi_bus_obj[i].dma.handle_tx.Init.Request = spi_config[i].dma_tx->request;
#endif
#ifndef SOC_SERIES_STM32U5
            spi_bus_obj[i].dma.handle_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
            spi_bus_obj[i].dma.handle_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
            spi_bus_obj[i].dma.handle_tx.Init.MemInc              = DMA_MINC_ENABLE;
            spi_bus_obj[i].dma.handle_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            spi_bus_obj[i].dma.handle_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
            spi_bus_obj[i].dma.handle_tx.Init.Mode                = DMA_NORMAL;
            spi_bus_obj[i].dma.handle_tx.Init.Priority            = DMA_PRIORITY_LOW;
#endif
#if defined(SOC_SERIES_STM32F2) || defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7) || defined(SOC_SERIES_STM32MP1) || defined(SOC_SERIES_STM32H7)
            spi_bus_obj[i].dma.handle_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
            spi_bus_obj[i].dma.handle_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
            spi_bus_obj[i].dma.handle_tx.Init.MemBurst            = DMA_MBURST_INC4;
            spi_bus_obj[i].dma.handle_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
#endif

            {
                rt_uint32_t tmpreg = 0x00U;
#if defined(SOC_SERIES_STM32F1) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32F0)
                /* enable DMA clock && Delay after an RCC peripheral clock enabling*/
                SET_BIT(RCC->AHBENR, spi_config[i].dma_tx->dma_rcc);
                tmpreg = READ_BIT(RCC->AHBENR, spi_config[i].dma_tx->dma_rcc);
#elif defined(SOC_SERIES_STM32F2) || defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7) || defined(SOC_SERIES_STM32L4) || defined(SOC_SERIES_STM32WB) || defined(SOC_SERIES_STM32H7)
                SET_BIT(RCC->AHB1ENR, spi_config[i].dma_tx->dma_rcc);
                /* Delay after an RCC peripheral clock enabling */
                tmpreg = READ_BIT(RCC->AHB1ENR, spi_config[i].dma_tx->dma_rcc);
#elif defined(SOC_SERIES_STM32MP1)
                __HAL_RCC_DMAMUX_CLK_ENABLE();
                SET_BIT(RCC->MP_AHB2ENSETR, spi_config[i].dma_tx->dma_rcc);
                tmpreg = READ_BIT(RCC->MP_AHB2ENSETR, spi_config[i].dma_tx->dma_rcc);
#endif
                UNUSED(tmpreg); /* To avoid compiler warnings */
            }
        }

        /* initialize completion object */
        rt_completion_init(&spi_bus_obj[i].cpt);

        result = rt_spi_bus_register(&spi_bus_obj[i].spi_bus, spi_config[i].bus_name, &stm_spi_ops);
        RT_ASSERT(result == RT_EOK);

        LOG_D("%s bus init done", spi_config[i].bus_name);
    }

    return result;
}

/**
  * Attach the spi device to SPI bus, this function must be used after initialization.
  */
rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cs_pin)
{
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);

    rt_err_t result;
    struct rt_spi_device *spi_device;

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
#else
    /* initialize the cs pin && select the slave*/
    rt_pin_mode(cs_pin,PIN_MODE_OUTPUT);
    rt_pin_write(cs_pin,PIN_HIGH);

    rt_base_t *pcs_pin;
    pcs_pin = (rt_base_t *)rt_malloc(sizeof(rt_base_t));
    RT_ASSERT(pcs_pin != RT_NULL);
    *pcs_pin = cs_pin;
#endif

    /* attach the device to spi bus*/
    spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);

#if RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2)
    result = rt_spi_bus_attach_device_cspin(spi_device, device_name, bus_name, cs_pin, RT_NULL);
#else
    result = rt_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)pcs_pin);
#endif
    if (result != RT_EOK)
    {
        LOG_E("%s attach to %s faild, %d", device_name, bus_name, result);
    }

    RT_ASSERT(result == RT_EOK);

    LOG_D("%s attach to %s done", device_name, bus_name);

    return result;
}
RTM_EXPORT(rt_hw_spi_device_attach);

#if defined(BSP_USING_SPI1)
void SPI1_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_SPI_IRQHandler(&spi_bus_obj[SPI1_INDEX].handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI1) && defined(BSP_SPI1_RX_USING_DMA)
/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
void SPI1_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI1_INDEX].dma.handle_rx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI1) && defined(BSP_SPI1_TX_USING_DMA)
/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void SPI1_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI1_INDEX].dma.handle_tx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(BSP_USING_SPI1) && defined(BSP_SPI_USING_DMA) */

#if defined(BSP_USING_SPI2)
void SPI2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_SPI_IRQHandler(&spi_bus_obj[SPI2_INDEX].handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI2) && defined(BSP_SPI2_RX_USING_DMA)
/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
void SPI2_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI2_INDEX].dma.handle_rx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI2) && defined(BSP_SPI2_TX_USING_DMA)
/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void SPI2_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI2_INDEX].dma.handle_tx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(BSP_USING_SPI2) && defined(BSP_SPI_USING_DMA) */

#if defined(BSP_USING_SPI3)
void SPI3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_SPI_IRQHandler(&spi_bus_obj[SPI3_INDEX].handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI3) && defined(BSP_SPI3_RX_USING_DMA)
/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
void SPI3_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI3_INDEX].dma.handle_rx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI3) && defined(BSP_SPI3_TX_USING_DMA)
/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void SPI3_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI3_INDEX].dma.handle_tx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(BSP_USING_SPI3) && defined(BSP_SPI_USING_DMA) */

#if defined(BSP_USING_SPI4)
void SPI4_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_SPI_IRQHandler(&spi_bus_obj[SPI4_INDEX].handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI4) && defined(BSP_SPI4_RX_USING_DMA)
/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
void SPI4_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI4_INDEX].dma.handle_rx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI4) && defined(BSP_SPI4_TX_USING_DMA)
/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void SPI4_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI4_INDEX].dma.handle_tx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(BSP_USING_SPI4) && defined(BSP_SPI_USING_DMA) */

#if defined(BSP_USING_SPI5)
void SPI5_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_SPI_IRQHandler(&spi_bus_obj[SPI5_INDEX].handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI5) && defined(BSP_SPI5_RX_USING_DMA)
/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
void SPI5_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI5_INDEX].dma.handle_rx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI5) && defined(BSP_SPI5_TX_USING_DMA)
/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void SPI5_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI5_INDEX].dma.handle_tx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(BSP_USING_SPI5) && defined(BSP_SPI_USING_DMA) */

#if defined(BSP_USING_SPI6)
void SPI6_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_SPI_IRQHandler(&spi_bus_obj[SPI6_INDEX].handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI6) && defined(BSP_SPI6_RX_USING_DMA)
/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
void SPI6_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI6_INDEX].dma.handle_rx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_SPI6) && defined(BSP_SPI6_TX_USING_DMA)
/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void SPI6_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&spi_bus_obj[SPI6_INDEX].dma.handle_tx);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(BSP_USING_SPI6) && defined(BSP_SPI_USING_DMA) */

static void stm32_get_dma_info(void)
{
#ifdef BSP_SPI1_RX_USING_DMA
    spi_bus_obj[SPI1_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static struct dma_config spi1_dma_rx = SPI1_RX_DMA_CONFIG;
    spi_config[SPI1_INDEX].dma_rx = &spi1_dma_rx;
#endif
#ifdef BSP_SPI1_TX_USING_DMA
    spi_bus_obj[SPI1_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static struct dma_config spi1_dma_tx = SPI1_TX_DMA_CONFIG;
    spi_config[SPI1_INDEX].dma_tx = &spi1_dma_tx;
#endif

#ifdef BSP_SPI2_RX_USING_DMA
    spi_bus_obj[SPI2_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static struct dma_config spi2_dma_rx = SPI2_RX_DMA_CONFIG;
    spi_config[SPI2_INDEX].dma_rx = &spi2_dma_rx;
#endif
#ifdef BSP_SPI2_TX_USING_DMA
    spi_bus_obj[SPI2_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static struct dma_config spi2_dma_tx = SPI2_TX_DMA_CONFIG;
    spi_config[SPI2_INDEX].dma_tx = &spi2_dma_tx;
#endif

#ifdef BSP_SPI3_RX_USING_DMA
    spi_bus_obj[SPI3_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static struct dma_config spi3_dma_rx = SPI3_RX_DMA_CONFIG;
    spi_config[SPI3_INDEX].dma_rx = &spi3_dma_rx;
#endif
#ifdef BSP_SPI3_TX_USING_DMA
    spi_bus_obj[SPI3_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static struct dma_config spi3_dma_tx = SPI3_TX_DMA_CONFIG;
    spi_config[SPI3_INDEX].dma_tx = &spi3_dma_tx;
#endif

#ifdef BSP_SPI4_RX_USING_DMA
    spi_bus_obj[SPI4_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static struct dma_config spi4_dma_rx = SPI4_RX_DMA_CONFIG;
    spi_config[SPI4_INDEX].dma_rx = &spi4_dma_rx;
#endif
#ifdef BSP_SPI4_TX_USING_DMA
    spi_bus_obj[SPI4_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static struct dma_config spi4_dma_tx = SPI4_TX_DMA_CONFIG;
    spi_config[SPI4_INDEX].dma_tx = &spi4_dma_tx;
#endif

#ifdef BSP_SPI5_RX_USING_DMA
    spi_bus_obj[SPI5_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static struct dma_config spi5_dma_rx = SPI5_RX_DMA_CONFIG;
    spi_config[SPI5_INDEX].dma_rx = &spi5_dma_rx;
#endif
#ifdef BSP_SPI5_TX_USING_DMA
    spi_bus_obj[SPI5_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static struct dma_config spi5_dma_tx = SPI5_TX_DMA_CONFIG;
    spi_config[SPI5_INDEX].dma_tx = &spi5_dma_tx;
#endif

#ifdef BSP_SPI6_RX_USING_DMA
    spi_bus_obj[SPI6_INDEX].spi_dma_flag |= SPI_USING_RX_DMA_FLAG;
    static struct dma_config spi6_dma_rx = SPI6_RX_DMA_CONFIG;
    spi_config[SPI6_INDEX].dma_rx = &spi6_dma_rx;
#endif
#ifdef BSP_SPI6_TX_USING_DMA
    spi_bus_obj[SPI6_INDEX].spi_dma_flag |= SPI_USING_TX_DMA_FLAG;
    static struct dma_config spi6_dma_tx = SPI6_TX_DMA_CONFIG;
    spi_config[SPI6_INDEX].dma_tx = &spi6_dma_tx;
#endif
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    struct stm32_spi *spi_drv =  rt_container_of(hspi, struct stm32_spi, handle);
    rt_completion_done(&spi_drv->cpt);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    struct stm32_spi *spi_drv =  rt_container_of(hspi, struct stm32_spi, handle);
    rt_completion_done(&spi_drv->cpt);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    struct stm32_spi *spi_drv =  rt_container_of(hspi, struct stm32_spi, handle);
    rt_completion_done(&spi_drv->cpt);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    struct stm32_spi *spi_drv =  rt_container_of(hspi, struct stm32_spi, handle);
    uint32_t error = HAL_SPI_GetError(hspi);

    switch(error)
    {
    case HAL_SPI_ERROR_OVR:
        __HAL_SPI_CLEAR_OVRFLAG(hspi);
        LOG_E("ErrorCallback 0x%x HAL_SPI_ERROR_OVR", error);
        break;
    case HAL_SPI_ERROR_DMA:
        LOG_E("ErrorCallback 0x%x HAL_SPI_ERROR_DMA", error);
        break;
    case HAL_SPI_ERROR_TIMEOUT:
        LOG_E("ErrorCallback 0x%x HAL_SPI_ERROR_TIMEOUT", error);
        break;
    default:
        LOG_E("ErrorCallback 0x%x", error);
        break;
    }
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi)
{
    LOG_E("AbortCpltCallback 0x%x", HAL_SPI_GetState(hspi));
}

#if defined(SOC_SERIES_STM32F0)
void SPI1_DMA_RX_TX_IRQHandler(void)
{
#if defined(BSP_USING_SPI1) && defined(BSP_SPI1_TX_USING_DMA)
    SPI1_DMA_TX_IRQHandler();
#endif

#if defined(BSP_USING_SPI1) && defined(BSP_SPI1_RX_USING_DMA)
    SPI1_DMA_RX_IRQHandler();
#endif
}

void SPI2_DMA_RX_TX_IRQHandler(void)
{
#if defined(BSP_USING_SPI2) && defined(BSP_SPI2_TX_USING_DMA)
    SPI2_DMA_TX_IRQHandler();
#endif

#if defined(BSP_USING_SPI2) && defined(BSP_SPI2_RX_USING_DMA)
    SPI2_DMA_RX_IRQHandler();
#endif
}
#endif  /* SOC_SERIES_STM32F0 */

int rt_hw_spi_init(void)
{
    stm32_get_dma_info();
    return rt_hw_spi_bus_init();
}
INIT_BOARD_EXPORT(rt_hw_spi_init);

#endif /* BSP_USING_SPI1 || BSP_USING_SPI2 || BSP_USING_SPI3 || BSP_USING_SPI4 || BSP_USING_SPI5 */
#endif /* BSP_USING_SPI */
