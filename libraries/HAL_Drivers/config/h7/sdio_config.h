/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-13     BalanceTWK   first version
 */

#ifndef __SDIO_CONFIG_H__
#define __SDIO_CONFIG_H__

#include <rtthread.h>
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BSP_USING_SDIO
#define SDIO_BUS_CONFIG                                  \
    {                                                    \
        .Instance = SDMMC1,                              \
        .dma_rx.dma_rcc = SDIO_RX_DMA_RCC,               \
        .dma_rx.Instance = SDIO_RX_DMA_INSTANCE,         \
        .dma_rx.channel = SDIO_RX_DMA_CHANNEL,           \
        .dma_rx.dma_irq = SDIO_RX_DMA_IRQ,               \
        .dma_tx.dma_rcc = SDIO_TX_DMA_RCC,               \
        .dma_tx.Instance = SDIO_TX_DMA_INSTANCE,         \
        .dma_tx.channel = SDIO_TX_DMA_CHANNEL,           \
        .dma_tx.dma_irq = SDIO_TX_DMA_IRQ,               \
    }

#endif

#ifdef __cplusplus
}
#endif

#endif /*__SDIO_CONFIG_H__ */



