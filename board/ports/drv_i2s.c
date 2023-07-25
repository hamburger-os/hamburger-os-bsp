/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-01-17     lvhan       the first version
 */
#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>

#include "drv_i2s.h"
#include <drv_config.h>

#define DBG_TAG "drv.i2s"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef SOC_SERIES_STM32H7
#define TX_DMA_FIFO_SIZE (2048)
#define RX_DMA_FIFO_SIZE (2048)
#endif

struct stm32_i2s
{
    I2S_HandleTypeDef hi2s;
    DMA_HandleTypeDef hdma_rx;
    DMA_HandleTypeDef hdma_tx;

    void (*TxHalfCpltCallback)(void);
    void (*TxCpltCallback)(void);
    void (*RxHalfCpltCallback)(void);
    void (*RxCpltCallback)(void);
    void (*ErrorCallback)(void);

#ifdef SOC_SERIES_STM32H7
    uint8_t tx_fifo[TX_DMA_FIFO_SIZE];
    uint8_t rx_fifo[RX_DMA_FIFO_SIZE];
#endif
};
static struct stm32_i2s i2s_config;

/**
 * @brief This function handles I2S_DMA_RX global interrupt.
 */
#ifdef BSP_USING_I2S1
void I2S1_DMA_RX_IRQHandler(void)
#endif
#ifdef BSP_USING_I2S2
void I2S2_DMA_RX_IRQHandler(void)
#endif
#ifdef BSP_USING_I2S3
void I2S3_DMA_RX_IRQHandler(void)
#endif
{
    /* enter interrupt */
    rt_interrupt_enter();
    HAL_DMA_IRQHandler(&i2s_config.hdma_rx);
    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * @brief This function handles I2S_DMA_TX global interrupt.
 */
#ifdef BSP_USING_I2S1
void I2S1_DMA_TX_IRQHandler(void)
#endif
#ifdef BSP_USING_I2S2
void I2S2_DMA_TX_IRQHandler(void)
#endif
#ifdef BSP_USING_I2S3
void I2S3_DMA_TX_IRQHandler(void)
#endif
{
    /* enter interrupt */
    rt_interrupt_enter();
    HAL_DMA_IRQHandler(&i2s_config.hdma_tx);
    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * @brief This function handles SPI global interrupt.
 */
#ifdef BSP_USING_I2S1
void SPI1_IRQn_IRQHandler(void)
#endif
#ifdef BSP_USING_I2S2
void SPI2_IRQHandler(void)
#endif
#ifdef BSP_USING_I2S3
void SPI3_IRQHandler(void)
#endif
{
    /* enter interrupt */
    rt_interrupt_enter();
    HAL_I2S_IRQHandler(&i2s_config.hi2s);
    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * @brief  Tx Transfer Half completed callbacks
 * @param  hi2s pointer to a I2S_HandleTypeDef structure that contains
 *         the configuration information for I2S module
 * @retval None
 */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (i2s_config.TxHalfCpltCallback != NULL)
    {
        i2s_config.TxHalfCpltCallback();
    }
}

/**
 * @brief  Tx Transfer completed callbacks
 * @param  hi2s pointer to a I2S_HandleTypeDef structure that contains
 *         the configuration information for I2S module
 * @retval None
 */
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (i2s_config.TxCpltCallback != NULL)
    {
        i2s_config.TxCpltCallback();
    }
}

/**
  * @brief  Rx Transfer half completed callbacks
  * @param  hi2s pointer to a I2S_HandleTypeDef structure that contains
  *         the configuration information for I2S module
  * @retval None
  */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_InvalidateDCache_by_Addr((uint32_t*)i2s_config.rx_fifo, RX_DMA_FIFO_SIZE/2);
#endif
    if (i2s_config.RxHalfCpltCallback != NULL)
    {
        i2s_config.RxHalfCpltCallback();
    }
}

/**
 * @brief  Rx Transfer half completed callbacks
 * @param  hi2s pointer to a I2S_HandleTypeDef structure that contains
 *         the configuration information for I2S module
 * @retval None
 */
void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (i2s_config.RxHalfCpltCallback != NULL)
    {
        i2s_config.RxHalfCpltCallback();
    }
}

/**
  * @brief  Rx Transfer completed callbacks
  * @param  hi2s pointer to a I2S_HandleTypeDef structure that contains
  *         the configuration information for I2S module
  * @retval None
  */
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (i2s_config.RxCpltCallback != NULL)
    {
        i2s_config.RxCpltCallback();
    }
}

/**
 * @brief  Rx Transfer completed callbacks
 * @param  hi2s pointer to a I2S_HandleTypeDef structure that contains
 *         the configuration information for I2S module
 * @retval None
 */
void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s)
{
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_InvalidateDCache_by_Addr((uint32_t*)i2s_config.rx_fifo, RX_DMA_FIFO_SIZE);
#endif
    if (i2s_config.RxCpltCallback != NULL)
    {
        i2s_config.RxCpltCallback();
    }
}

/**
 * @brief  I2S error callbacks
 * @param  hi2s pointer to a I2S_HandleTypeDef structure that contains
 *         the configuration information for I2S module
 * @retval None
 */
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    if (i2s_config.ErrorCallback != NULL)
    {
        i2s_config.ErrorCallback();
    }
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(I2S_HandleTypeDef* hi2s)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();
//    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA interrupt init */
#ifdef BSP_USING_I2S1
    /* I2S_RX_DMA_IRQ interrupt configuration */
    HAL_NVIC_SetPriority(I2S1_RX_DMA_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(I2S1_RX_DMA_IRQ);
    /* I2S_TX_DMA_IRQ interrupt configuration */
    HAL_NVIC_SetPriority(I2S1_TX_DMA_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(I2S1_TX_DMA_IRQ);
#endif
#ifdef BSP_USING_I2S2
    /* I2S_RX_DMA_IRQ interrupt configuration */
    HAL_NVIC_SetPriority(I2S2_RX_DMA_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(I2S2_RX_DMA_IRQ);
    /* I2S_TX_DMA_IRQ interrupt configuration */
    HAL_NVIC_SetPriority(I2S2_TX_DMA_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(I2S2_TX_DMA_IRQ);
#endif
#ifdef BSP_USING_I2S3
    /* I2S_RX_DMA_IRQ interrupt configuration */
    HAL_NVIC_SetPriority(I2S3_RX_DMA_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(I2S3_RX_DMA_IRQ);
    /* I2S_TX_DMA_IRQ interrupt configuration */
    HAL_NVIC_SetPriority(I2S3_TX_DMA_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(I2S3_TX_DMA_IRQ);
#endif

    /* I2S DMA Init */
    /* I2S_RX_DMA Init */
#ifdef BSP_USING_I2S1
    i2s_config.hdma_rx.Instance = I2S1_RX_DMA_INSTANCE;
#ifdef SOC_SERIES_STM32H7
    i2s_config.hdma_rx.Init.Request = I2S1_RX_DMA_REQUEST;
#else
    i2s_config.hdma_rx.Init.Channel = I2S1_RX_DMA_CHANNEL;
#endif
#endif
#ifdef BSP_USING_I2S2
    i2s_config.hdma_rx.Instance = I2S2_RX_DMA_INSTANCE;
#ifdef SOC_SERIES_STM32H7
    i2s_config.hdma_rx.Init.Request = I2S2_RX_DMA_REQUEST;
#else
    i2s_config.hdma_rx.Init.Channel = I2S2_RX_DMA_CHANNEL;
#endif
#endif
#ifdef BSP_USING_I2S3
    i2s_config.hdma_rx.Instance = I2S3_RX_DMA_INSTANCE;
#ifdef SOC_SERIES_STM32H7
    i2s_config.hdma_rx.Init.Request = I2S3_RX_DMA_REQUEST;
#else
    i2s_config.hdma_rx.Init.Channel = I2S3_RX_DMA_CHANNEL;
#endif
#endif
    i2s_config.hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    i2s_config.hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    i2s_config.hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
    i2s_config.hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    i2s_config.hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    i2s_config.hdma_rx.Init.Mode = DMA_CIRCULAR;
    i2s_config.hdma_rx.Init.Priority = DMA_PRIORITY_LOW;
    i2s_config.hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&i2s_config.hdma_rx) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(hi2s, hdmarx, i2s_config.hdma_rx);

    /* I2S_TX_DMA Init */
#ifdef BSP_USING_I2S1
    i2s_config.hdma_tx.Instance = I2S1_TX_DMA_INSTANCE;
#ifdef SOC_SERIES_STM32H7
    i2s_config.hdma_tx.Init.Request = I2S1_TX_DMA_REQUEST;
#else
    i2s_config.hdma_tx.Init.Channel = I2S1_TX_DMA_CHANNEL;
#endif
#endif
#ifdef BSP_USING_I2S2
    i2s_config.hdma_tx.Instance = I2S2_TX_DMA_INSTANCE;
#ifdef SOC_SERIES_STM32H7
    i2s_config.hdma_tx.Init.Request = I2S2_TX_DMA_REQUEST;
#else
    i2s_config.hdma_tx.Init.Channel = I2S2_TX_DMA_CHANNEL;
#endif
#endif
#ifdef BSP_USING_I2S3
    i2s_config.hdma_tx.Instance = I2S3_TX_DMA_INSTANCE;
#ifdef SOC_SERIES_STM32H7
    i2s_config.hdma_tx.Init.Request = I2S3_TX_DMA_REQUEST;
#else
    i2s_config.hdma_tx.Init.Channel = I2S3_TX_DMA_CHANNEL;
#endif
#endif
    i2s_config.hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    i2s_config.hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    i2s_config.hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
    i2s_config.hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    i2s_config.hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    i2s_config.hdma_tx.Init.Mode = DMA_CIRCULAR;
    i2s_config.hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
    i2s_config.hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&i2s_config.hdma_tx) != HAL_OK)
    {
        Error_Handler();
    }

    __HAL_LINKDMA(hi2s, hdmatx, i2s_config.hdma_tx);
}

/**
 * @brief I2S Initialization Function
 * @param None
 * @retval None
 */
void MX_I2S_Init(void)
{
#ifdef BSP_USING_I2S1
    i2s_config.hi2s.Instance = SPI1;
#endif
#ifdef BSP_USING_I2S2
    i2s_config.hi2s.Instance = SPI2;
#endif
#ifdef BSP_USING_I2S3
    i2s_config.hi2s.Instance = SPI3;
#endif

#ifdef SOC_SERIES_STM32H7
    i2s_config.hi2s.Init.Mode = I2S_MODE_MASTER_FULLDUPLEX;
    i2s_config.hi2s.Init.Standard = I2S_STANDARD_PHILIPS;
    i2s_config.hi2s.Init.DataFormat = I2S_DATAFORMAT_16B_EXTENDED;
    i2s_config.hi2s.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    i2s_config.hi2s.Init.AudioFreq = I2S_AUDIOFREQ_8K;
    i2s_config.hi2s.Init.CPOL = I2S_CPOL_LOW;
    i2s_config.hi2s.Init.FirstBit = I2S_FIRSTBIT_MSB;
    i2s_config.hi2s.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
    i2s_config.hi2s.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
    i2s_config.hi2s.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_DISABLE;
#else
    i2s_config.hi2s.Init.Mode = I2S_MODE_MASTER_TX;
    i2s_config.hi2s.Init.Standard = I2S_STANDARD_PHILIPS;
    i2s_config.hi2s.Init.DataFormat = I2S_DATAFORMAT_16B_EXTENDED;
    i2s_config.hi2s.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    i2s_config.hi2s.Init.AudioFreq = I2S_AUDIOFREQ_8K;
    i2s_config.hi2s.Init.CPOL = I2S_CPOL_LOW;
    i2s_config.hi2s.Init.ClockSource = I2S_CLOCK_PLL;
    i2s_config.hi2s.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_ENABLE;
#endif
    if (HAL_I2S_Init(&i2s_config.hi2s) != HAL_OK)
    {
        Error_Handler();
    }

#ifdef SOC_SERIES_STM32H7
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3|RCC_PERIPHCLK_SPI2
                              |RCC_PERIPHCLK_SPI1|RCC_PERIPHCLK_CKPER;
    PeriphClkInitStruct.PLL2.PLL2M = 25;
    PeriphClkInitStruct.PLL2.PLL2N = 192;
    PeriphClkInitStruct.PLL2.PLL2P = 3;
    PeriphClkInitStruct.PLL2.PLL2Q = 1;
    PeriphClkInitStruct.PLL2.PLL2R = 1;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSE;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
#else
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
#endif

    /* I2S interrupt Init */
#ifdef BSP_USING_I2S1
    HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
#endif
#ifdef BSP_USING_I2S2
    HAL_NVIC_SetPriority(SPI2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
#endif
#ifdef BSP_USING_I2S3
    HAL_NVIC_SetPriority(SPI3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI3_IRQn);
#endif

    MX_DMA_Init(&i2s_config.hi2s);
    HAL_I2S_DMAStop(&i2s_config.hi2s);
}

//采样率计算公式:Fs=I2SxCLK/[256*(2*I2SDIV+ODD)]
//I2SxCLK=(HSE/pllm)*PLLI2SN/PLLI2SR
//一般HSE=8Mhz
//pllm:在Sys_Clock_Set设置的时候确定,一般是8
//PLLI2SN:一般是192~432
//PLLI2SR:2~7
//I2SDIV:2~255
//ODD:0/1
//I2S分频系数表@pllm=8,HSE=8Mhz,即vco输入频率为1Mhz
//表格式:采样率/10,PLLI2SN,PLLI2SR,I2SDIV,ODD
static const uint32_t I2S_PSC_TBL[][3] =
{
#ifdef BSP_I2S_USING_MCLK
    {  I2S_AUDIOFREQ_8K, 256, 5 },      //8Khz采样率
    { I2S_AUDIOFREQ_11K, 429, 4 },      //11.025Khz采样率
    { I2S_AUDIOFREQ_16K, 213, 2 },      //16Khz采样率
    { I2S_AUDIOFREQ_22K, 429, 4 },      //22.05Khz采样率
    { I2S_AUDIOFREQ_32K, 213, 2 },      //32Khz采样率
    { I2S_AUDIOFREQ_44K, 271, 2 },      //44.1Khz采样率
    { I2S_AUDIOFREQ_48K, 258, 3 },      //48Khz采样率3
    {             88200, 316, 2 },      //88.2Khz采样率
    { I2S_AUDIOFREQ_96K, 344, 2 },      //96Khz采样率
    {            176400, 361, 2 },      //176.4Khz采样率
    {I2S_AUDIOFREQ_192K, 393, 2 },      //192Khz采样率
#else
    {  I2S_AUDIOFREQ_8K, 192, 3 },      //8Khz采样率
    { I2S_AUDIOFREQ_11K, 429, 4 },      //11.025Khz采样率
    { I2S_AUDIOFREQ_16K, 256, 2 },      //16Khz采样率
    { I2S_AUDIOFREQ_22K, 302, 2 },      //22.05Khz采样率
    { I2S_AUDIOFREQ_32K, 256, 5 },      //32Khz采样率
    { I2S_AUDIOFREQ_44K, 429, 4 },      //44.1Khz采样率
    { I2S_AUDIOFREQ_48K, 384, 5 },      //48Khz采样率3
    {             88200, 316, 2 },      //88.2Khz采样率
    { I2S_AUDIOFREQ_96K, 424, 3 },      //96Khz采样率
    {            176400, 361, 2 },      //176.4Khz采样率
    {I2S_AUDIOFREQ_192K, 258, 3 },      //192Khz采样率
#endif
};
void I2S_Samplerate_Set(uint32_t freq)
{
    int i;

    /* check frequence */
    for (i = 0; i < (sizeof(I2S_PSC_TBL) / sizeof(I2S_PSC_TBL[0])); i++)
    {
        if ((freq) == I2S_PSC_TBL[i][0])
            break;
    }
    if (i == (sizeof(I2S_PSC_TBL) / sizeof(I2S_PSC_TBL[0])))
    {
        LOG_E("Can not support this frequence: %d.", freq);
        return;
    }

#ifdef SOC_SERIES_STM32H7
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3|RCC_PERIPHCLK_SPI2
                              |RCC_PERIPHCLK_SPI1|RCC_PERIPHCLK_CKPER;
    PeriphClkInitStruct.PLL2.PLL2M = 25;
    PeriphClkInitStruct.PLL2.PLL2N = I2S_PSC_TBL[i][1];
    PeriphClkInitStruct.PLL2.PLL2P = I2S_PSC_TBL[i][2];
    PeriphClkInitStruct.PLL2.PLL2Q = 1;
    PeriphClkInitStruct.PLL2.PLL2R = 1;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSE;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
#else
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = I2S_PSC_TBL[i][1];
    PeriphClkInitStruct.PLLI2S.PLLI2SR = I2S_PSC_TBL[i][2];

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
#endif

    i2s_config.hi2s.Init.AudioFreq = freq;
    if (HAL_I2S_Init(&i2s_config.hi2s) != HAL_OK)
    {
        Error_Handler();
    }
}

void I2S_Channels_Set(uint16_t channels)
{
    if (channels == 1)
    {
        I2S_Samplerate_Set(i2s_config.hi2s.Init.AudioFreq/2);
    }
    else
    {
        I2S_Samplerate_Set(i2s_config.hi2s.Init.AudioFreq);
    }
}

void I2S_Samplebits_Set(uint16_t samplebits)
{
    switch (samplebits)
    {
    case 16:
        i2s_config.hi2s.Init.DataFormat = I2S_DATAFORMAT_16B_EXTENDED;
        break;
    case 24:
        i2s_config.hi2s.Init.DataFormat = I2S_DATAFORMAT_24B;
        break;
    case 32:
        i2s_config.hi2s.Init.DataFormat = I2S_DATAFORMAT_32B;
        break;
    default:
        i2s_config.hi2s.Init.DataFormat = I2S_DATAFORMAT_16B_EXTENDED;
        break;
    }
    if (HAL_I2S_Init(&i2s_config.hi2s) != HAL_OK)
    {
        Error_Handler();
    }
}

void I2S_TxHalfCpltCallback_Set(void (*callback)(void))
{
    i2s_config.TxHalfCpltCallback = callback;
}
void I2S_TxCpltCallback_Set(void (*callback)(void))
{
    i2s_config.TxCpltCallback = callback;
}
void I2S_RxHalfCpltCallback_Set(void (*callback)(void))
{
    i2s_config.RxHalfCpltCallback = callback;
}
void I2S_RxCpltCallback_Set(void (*callback)(void))
{
    i2s_config.RxCpltCallback = callback;
}
void I2S_ErrorCallback_Set(void (*callback)(void))
{
    i2s_config.ErrorCallback = callback;
}

//I2S开始播放
void I2S_Play_Start(const uint8_t *pData, uint16_t Size)
{
    HAL_I2S_DMAStop(&i2s_config.hi2s);
    if (Size > 0 && pData != NULL)
    {
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanDCache_by_Addr((uint32_t*)i2s_config.tx_fifo, TX_DMA_FIFO_SIZE);
#endif
        HAL_I2S_Transmit_DMA(&i2s_config.hi2s, (uint16_t *) pData, Size);
    }
}

//关闭I2S播放
void I2S_Play_Stop(void)
{
    HAL_I2S_DMAStop(&i2s_config.hi2s);
}

//I2S开始录音
void I2S_Rec_Start(uint8_t *pData, uint16_t Size)
{
    HAL_I2S_DMAStop(&i2s_config.hi2s);
    if (Size > 0 && pData != NULL)
    {
#ifdef SOC_SERIES_STM32H7
#if defined (__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
        SCB_CleanDCache_by_Addr((uint32_t*)i2s_config.rx_fifo, RX_DMA_FIFO_SIZE);
#endif
//        HAL_I2S_Receive_DMA(&i2s_config.hi2s, (uint16_t *) pData, Size);
        HAL_I2SEx_TransmitReceive_DMA(&i2s_config.hi2s, (uint16_t *) pData, (uint16_t *) pData, Size);
#else
        HAL_I2SEx_TransmitReceive_DMA(&i2s_config.hi2s, (uint16_t *) pData, (uint16_t *) pData, Size);
#endif
    }
}

//关闭I2S录音
void I2S_Rec_Stop(void)
{
    HAL_I2S_DMAStop(&i2s_config.hi2s);
}
