/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-04     zm       the first version
 */

#include "board.h"

#ifdef BSP_USING_MCP2517FD

#include "drv_canfdspi_register.h"

#define DBG_TAG "mcp2517"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

typedef enum {
#ifdef BSP_USE_MCP2517FD_CAN1
    MCP2517_CH_1,
#endif

#ifdef BSP_USE_MCP2517FD_CAN2
    MCP2517_CH_2,
#endif

#ifdef BSP_USE_MCP2517FD_CAN3
    MCP2517_CH_3,
#endif

#ifdef BSP_USE_MCP2517FD_CAN4
    MCP2517_CH_4,
#endif

} MCP2517CanPort;

#define SPI_DEFAULT_BUFFER_LENGTH 96

typedef struct {
    struct rt_device dev;                 /* 设备 */
    struct rt_spi_device *spi_dev;        /* SPI总线设备句柄 */

    const char *spi_name;                 /* SPI总线名 */
    const char *spi_dev_name;             /* SPI总线上设备名 */
    const char *spi_cs_pin_name;          /* spi片选引脚名 */
    rt_uint8_t spi_cs_pin_index;          /* spi片选引脚索引 */
    const char *spi_irq_pin_name;         /* spi中断引脚名 */
    rt_uint8_t spi_irq_pin_index;         /* spi中断引脚索引 */

    MCP2517CanPort channel;               /* can通道索引 */

    uint8_t spi_tx_buffer[SPI_DEFAULT_BUFFER_LENGTH+5];  /* SPI Transmit buffer */
    uint8_t spi_rx_buffer[SPI_DEFAULT_BUFFER_LENGTH+5];  /* SPI Receive buffer */
} MCP2517_Dev;

static MCP2517_Dev mcp2517_can_port[] =
{
#ifdef BSP_USE_MCP2517FD_CAN1
    {
         .spi_name = BSP_MCP2517FD_CAN1_SPI_BUS,
         .spi_dev_name = BSP_MCP2517FD_CAN1_SPI_BUS_DEV,
         .spi_cs_pin_name = BSP_MCP2517FD_CAN1_CS_PIN,
         .spi_irq_pin_name = BSP_MCP2517FD_CAN1_INT_PIN,
         .channel = MCP2517_CH_1,
    },
#endif  /* BSP_USE_MCP2517FD_CAN1 */

#ifdef BSP_USE_MCP2517FD_CAN2
    {
         .spi_name = BSP_MCP2517FD_CAN2_SPI_BUS,
         .spi_dev_name = BSP_MCP2517FD_CAN2_SPI_BUS_DEV,
         .spi_cs_pin_name = BSP_MCP2517FD_CAN2_CS_PIN,
         .spi_irq_pin_name = BSP_MCP2517FD_CAN2_INT_PIN,
         .channel = MCP2517_CH_2,
    },
#endif  /* BSP_USE_MCP2517FD_CAN2 */

#ifdef BSP_USE_MCP2517FD_CAN3
    {
         .spi_name = BSP_MCP2517FD_CAN3_SPI_BUS,
         .spi_dev_name = BSP_MCP2517FD_CAN3_SPI_BUS_DEV,
         .spi_cs_pin_name = BSP_MCP2517FD_CAN3_CS_PIN,
         .spi_irq_pin_name = BSP_MCP2517FD_CAN3_INT_PIN,
         .channel = MCP2517_CH_3,
    },
#endif  /* BSP_USE_MCP2517FD_CAN3 */

#ifdef BSP_USE_MCP2517FD_CAN4
    {
         .spi_name = BSP_MCP2517FD_CAN4_SPI_BUS,
         .spi_dev_name = BSP_MCP2517FD_CAN4_SPI_BUS_DEV,
         .spi_cs_pin_name = BSP_MCP2517FD_CAN4_CS_PIN,
         .spi_irq_pin_name = BSP_MCP2517FD_CAN4_INT_PIN,
         .channel = MCP2517_CH_4,
    },
#endif  /* BSP_USE_MCP2517FD_CAN4 */
};

static rt_err_t can_spi_read_byte(MCP2517_Dev *mcp2517_dev, uint16_t address, uint8_t *rxd)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);
    mcp2517_dev->spi_tx_buffer[2] = 0;

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 3);

    // Update data
    *rxd = mcp2517_dev->spi_rx_buffer[2];

    return ret;
}

static rt_err_t can_spi_write_byte(MCP2517_Dev *mcp2517_dev, uint16_t address, uint8_t txd)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);
    mcp2517_dev->spi_tx_buffer[2] = txd;

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 3);
    return ret;
}

static rt_err_t can_spi_read_word(MCP2517_Dev *mcp2517_dev, uint16_t address, uint32_t *rxd)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t i = 0;
    uint32_t x = 0;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 6);
    if (ret != RT_EOK)
    {
      return ret;
    }

    // Update data
    *rxd = 0;
    for (i = 2; i < 6; i++)
    {
      x = (uint32_t)mcp2517_dev->spi_rx_buffer[i];
      *rxd += x << ((i - 2) * 8);
    }

    return ret;
}

static rt_err_t can_spi_write_word(MCP2517_Dev *mcp2517_dev, uint16_t address, uint32_t txd)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t i = 0;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);

    // Split word into 4 bytes and add them to buffer
    for (i = 0; i < 4; i++)
    {
        mcp2517_dev->spi_tx_buffer[i + 2] = (uint8_t) ((txd >> (i * 8)) & 0xFF);
    }

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 6);

    return ret;
}

static rt_err_t can_spi_read_half_word(MCP2517_Dev *mcp2517_dev, uint16_t address, uint16_t *rxd)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t i = 0;
    uint32_t x = 0;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 4);
    if (ret != RT_EOK)
    {
      return ret;
    }

    // Update data
    *rxd = 0;
    for (i = 2; i < 4; i++)
    {
      x = (uint32_t) mcp2517_dev->spi_rx_buffer[i];
      *rxd += x << ((i - 2) * 8);
    }
    return ret;
}

static rt_err_t can_spi_write_half_word(MCP2517_Dev *mcp2517_dev, uint16_t address, uint16_t txd)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t i = 0;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);

    // Split word into 2 bytes and add them to buffer
    for (i = 0; i < 2; i++)
    {
        mcp2517_dev->spi_tx_buffer[i + 2] = (uint8_t) ((txd >> (i * 8)) & 0xFF);
    }

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 4);

    return ret;
}

static rt_err_t can_spi_read_byte_array(MCP2517_Dev *mcp2517_dev, uint16_t address, uint8_t *rxd, uint16_t nBytes)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t i = 0;
    uint16_t transfer_size = nBytes + 2;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);

    // Clear data
    for (i = 2; i < transfer_size; i++)
    {
        mcp2517_dev->spi_tx_buffer[i] = 0;
    }

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, transfer_size);

    // Update data
    for (i = 0; i < nBytes; i++)
    {
      rxd[i] = mcp2517_dev->spi_rx_buffer[i + 2];
    }

    return ret;
}

static rt_err_t can_spi_write_byte_array(MCP2517_Dev *mcp2517_dev, uint16_t address, uint8_t *txd, uint16_t nBytes)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t i = 0;
    uint16_t transfer_size = nBytes + 2;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);

    // Add data
    for (i = 2; i < transfer_size; i++)
    {
        mcp2517_dev->spi_tx_buffer[i] = txd[i - 2];
    }

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, transfer_size);

    return ret;
}

static rt_err_t can_spi_read_word_array(MCP2517_Dev *mcp2517_dev, uint16_t address, uint32_t *rxd, uint16_t nWords)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t i, j, n;
    REG_t w;
    uint16_t transfer_size = nWords * 4 + 2;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF);
    mcp2517_dev->spi_tx_buffer[1] = address & 0xFF;

    // Clear data
    for (i = 2; i < transfer_size; i++) {
        mcp2517_dev->spi_tx_buffer[i] = 0;
    }

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, transfer_size);
    if (ret != RT_EOK)
    {
      return ret;
    }

    // Convert Byte array to Word array
    n = 2;
    for (i = 0; i < nWords; i++)
    {
      w.word = 0;
      for (j = 0; j < 4; j++, n++)
      {
        w.byte[j] = mcp2517_dev->spi_rx_buffer[n];
      }
      rxd[i] = w.word;
    }

    return ret;
}

static rt_err_t can_spi_write_word_array(MCP2517_Dev *mcp2517_dev, uint16_t address, uint32_t *txd, uint16_t nWords)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t i, j, n;
    REG_t w;
    uint16_t transfer_size = nWords * 4 + 2;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF);
    mcp2517_dev->spi_tx_buffer[1] = address & 0xFF;

    // Convert ByteArray to word array
    n = 2;
    for (i = 0; i < nWords; i++)
    {
      w.word = txd[i];
      for (j = 0; j < 4; j++, n++)
      {
          mcp2517_dev->spi_tx_buffer[n] = w.byte[j];
      }
    }

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, transfer_size);
    return ret;
}

/*************************************************************************************************
功能：将数据长度标识转换为实际字节数
参数：dlc：数据长度标识
返回值：dataBytesInObject can数据字节数
**************************************************************************************************/
//DRV_CANFDSPI_DlcToDataBytes
static uint32_t can_spi_dlc_to_data_bytes(CAN_DLC dlc)
{
  uint32_t dataBytesInObject = 0;

  if (dlc < MCP_CAN_DLC_12)
  {
    dataBytesInObject = dlc;
  }
  else
  {
    switch (dlc)
    {
      case MCP_CAN_DLC_12:
        dataBytesInObject = 12;
        break;
      case MCP_CAN_DLC_16:
        dataBytesInObject = 16;
        break;
      case MCP_CAN_DLC_20:
        dataBytesInObject = 20;
        break;
      case MCP_CAN_DLC_24:
        dataBytesInObject = 24;
        break;
      case MCP_CAN_DLC_32:
        dataBytesInObject = 32;
        break;
      case MCP_CAN_DLC_48:
        dataBytesInObject = 48;
        break;
      case MCP_CAN_DLC_64:
        dataBytesInObject = 64;
        break;
      default:
        break;
    }
  }

  return dataBytesInObject;
}

static rt_err_t can_spi_configure(MCP2517_Dev *mcp2517_dev, CAN_CONFIG *config)
{
    rt_err_t ret = -RT_ERROR;
    REG_CiCON ciCon;

    if(NULL == mcp2517_dev || NULL == config)
    {
        return -RT_ERROR;
    }

    ciCon.word = canControlResetValues[cREGADDR_CiCON / 4];

    ciCon.bF.DNetFilterCount = config->DNetFilterCount;
    ciCon.bF.IsoCrcEnable = config->IsoCrcEnable;
    ciCon.bF.ProtocolExceptionEventDisable = config->ProtocolExpectionEventDisable;
    ciCon.bF.WakeUpFilterEnable = config->WakeUpFilterEnable;
    ciCon.bF.WakeUpFilterTime = config->WakeUpFilterTime;
    ciCon.bF.BitRateSwitchDisable = config->BitRateSwitchDisable;
    ciCon.bF.RestrictReTxAttempts = config->RestrictReTxAttempts;
    ciCon.bF.EsiInGatewayMode = config->EsiInGatewayMode;
    ciCon.bF.SystemErrorToListenOnly = config->SystemErrorToListenOnly;
    ciCon.bF.StoreInTEF = config->StoreInTEF;
    ciCon.bF.TXQEnable = config->TXQEnable;
    ciCon.bF.TxBandWidthSharing = config->TxBandWidthSharing;

    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiCON, ciCon.word);
    return ret;
}

static rt_err_t can_spi_configure_object_reset(CAN_CONFIG *config)
{
    REG_CiCON ciCon;

    if(NULL == config)
    {
        return -RT_EINVAL;
    }

    ciCon.word = canControlResetValues[cREGADDR_CiCON / 4];

    config->DNetFilterCount = ciCon.bF.DNetFilterCount;
    config->IsoCrcEnable = ciCon.bF.IsoCrcEnable;
    config->ProtocolExpectionEventDisable = ciCon.bF.ProtocolExceptionEventDisable;
    config->WakeUpFilterEnable = ciCon.bF.WakeUpFilterEnable;
    config->WakeUpFilterTime = ciCon.bF.WakeUpFilterTime;
    config->BitRateSwitchDisable = ciCon.bF.BitRateSwitchDisable;
    config->RestrictReTxAttempts = ciCon.bF.RestrictReTxAttempts;
    config->EsiInGatewayMode = ciCon.bF.EsiInGatewayMode;
    config->SystemErrorToListenOnly = ciCon.bF.SystemErrorToListenOnly;
    config->StoreInTEF = ciCon.bF.StoreInTEF;
    config->TXQEnable = ciCon.bF.TXQEnable;
    config->TxBandWidthSharing = ciCon.bF.TxBandWidthSharing;
    return RT_EOK;
}

static rt_err_t can_spi_operation_mode_select(MCP2517_Dev *mcp2517_dev, CAN_OPERATION_MODE opMode)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t d = 0;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Read
    ret = can_spi_read_byte(mcp2517_dev, cREGADDR_CiCON + 3, &d);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Modify
    d &= ~0x07;
    d |= opMode;

    // Write
    ret = can_spi_write_byte(mcp2517_dev, cREGADDR_CiCON + 3, d);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

// Section: CAN Transmit
static rt_err_t can_spi_transmit_channel_configure(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_CONFIG* config)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t a = 0;

    if(NULL == mcp2517_dev || NULL == config)
    {
        return -RT_ERROR;
    }

    // Setup FIFO
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = canFifoResetValues[0];
    ciFifoCon.txBF.TxEnable = 1;
    ciFifoCon.txBF.FifoSize = config->FifoSize;
    ciFifoCon.txBF.PayLoadSize = config->PayLoadSize;
    ciFifoCon.txBF.TxAttempts = config->TxAttempts;
    ciFifoCon.txBF.TxPriority = config->TxPriority;
    ciFifoCon.txBF.RTREnable = config->RTREnable;

    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

    ret = can_spi_write_word(mcp2517_dev, a, ciFifoCon.word);

    return ret;
}

static rt_err_t can_spi_transmit_channel_configure_object_reset(CAN_TX_FIFO_CONFIG* config)
{
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = canFifoResetValues[0];

    if(NULL == config)
    {
        return -RT_EINVAL;
    }

    config->RTREnable = ciFifoCon.txBF.RTREnable;
    config->TxPriority = ciFifoCon.txBF.TxPriority;
    config->TxAttempts = ciFifoCon.txBF.TxAttempts;
    config->FifoSize = ciFifoCon.txBF.FifoSize;
    config->PayLoadSize = ciFifoCon.txBF.PayLoadSize;

    return RT_EOK;
}

static rt_err_t can_spi_transmit_queue_configure(MCP2517_Dev *mcp2517_dev, CAN_TX_QUEUE_CONFIG* config)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t a = 0;

    if(NULL == mcp2517_dev || NULL == config)
    {
        return -RT_ERROR;
    }

    // Setup FIFO
    REG_CiTXQCON ciFifoCon;
    ciFifoCon.word = canFifoResetValues[0];

    ciFifoCon.txBF.TxEnable = 1;
    ciFifoCon.txBF.FifoSize = config->FifoSize;
    ciFifoCon.txBF.PayLoadSize = config->PayLoadSize;
    ciFifoCon.txBF.TxAttempts = config->TxAttempts;
    ciFifoCon.txBF.TxPriority = config->TxPriority;

    a = cREGADDR_CiTXQCON;
    ret = can_spi_write_word(mcp2517_dev, a, ciFifoCon.word);

    return ret;
}

static rt_err_t can_spi_transmit_queue_configure_object_reset(CAN_TX_QUEUE_CONFIG* config)
{
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = canFifoResetValues[0];

    if(NULL == config)
    {
        return -RT_EINVAL;
    }

    config->TxPriority = ciFifoCon.txBF.TxPriority;
    config->TxAttempts = ciFifoCon.txBF.TxAttempts;
    config->FifoSize = ciFifoCon.txBF.FifoSize;
    config->PayLoadSize = ciFifoCon.txBF.PayLoadSize;
    return RT_EOK;
}

//DRV_CANFDSPI_TransmitChannelUpdate
int8_t can_spi_transmit_channel_update(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, bool flush)
{
    uint16_t a;
    REG_CiFIFOCON ciFifoCon;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Set UINC
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET) + 1; // Byte that contains FRESET
    ciFifoCon.word = 0;
    ciFifoCon.txBF.UINC = 1;

    // Set TXREQ
    if (flush) {
        ciFifoCon.txBF.TxRequest = 1;
    }

    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[1]);
    if (ret != RT_EOK) {
        return ret;
    }

    return ret;
}


//DRV_CANFDSPI_TransmitChannelLoad
static rt_err_t can_spi_transmit_channel_load(MCP2517_Dev *mcp2517_dev,
                                        CAN_FIFO_CHANNEL channel, CAN_TX_MSGOBJ* txObj,
                                        uint8_t *txd, uint32_t txdNumBytes, bool flush)
{
    uint16_t a;
    uint32_t fifoReg[3];
    uint32_t dataBytesInObject;
    REG_CiFIFOCON ciFifoCon;
    REG_CiFIFOSTA ciFifoSta;
    REG_CiFIFOUA ciFifoUa;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Get FIFO registers
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_word_array(mcp2517_can_port, a, fifoReg, 3);
    if (ret != RT_EOK) {
        return ret;
    }

    // Check that it is a transmit buffer
    ciFifoCon.word = fifoReg[0];
    if (!ciFifoCon.txBF.TxEnable) {
        return -RT_EIO;
    }

    // Check that DLC is big enough for data
    dataBytesInObject = can_spi_dlc_to_data_bytes((CAN_DLC) txObj->bF.ctrl.DLC);
    if (dataBytesInObject < txdNumBytes) {
        return -RT_EINVAL;
    }

    // Get status
    ciFifoSta.word = fifoReg[1];

    // Get address
    ciFifoUa.word = fifoReg[2];
#ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciFifoUa.bF.UserAddress;
#else
    a = ciFifoUa.bF.UserAddress;
#endif
    a += cRAMADDR_START;

    uint8_t txBuffer[MAX_MSG_SIZE];

    txBuffer[0] = txObj->byte[0]; //not using 'for' to reduce no of instructions
    txBuffer[1] = txObj->byte[1];
    txBuffer[2] = txObj->byte[2];
    txBuffer[3] = txObj->byte[3];

    txBuffer[4] = txObj->byte[4];
    txBuffer[5] = txObj->byte[5];
    txBuffer[6] = txObj->byte[6];
    txBuffer[7] = txObj->byte[7];

    uint8_t i;
    for (i = 0; i < txdNumBytes; i++) {
        txBuffer[i + 8] = txd[i];
    }

    // Make sure we write a multiple of 4 bytes to RAM
    uint16_t n = 0;
    uint8_t j = 0;

    if (txdNumBytes % 4) {
        // Need to add bytes
        n = 4 - (txdNumBytes % 4);
        i = txdNumBytes + 8;

        for (j = 0; j < n; j++) {
            txBuffer[i + 8 + j] = 0;
        }
    }

    ret = can_spi_write_byte_array(mcp2517_dev, a, txBuffer, txdNumBytes + 8 + n);
    if (ret != RT_EOK) {
        return ret;
    }

    // Set UINC and TXREQ
    ret = can_spi_transmit_channel_update(mcp2517_dev, channel, flush);
    if (ret != RT_EOK) {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_TransmitChannelFlush
static rt_err_t can_spi_transmit_channel_flush(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel)
{
    uint8_t d = 0;
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Address of TXREQ
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
    a += 1;

    // Set TXREQ
    d = 0x02;

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, d);

    return ret;
}

//DRV_CANFDSPI_TransmitChannelStatusGet
static rt_err_t can_spi_transmit_channel_status_get(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_STATUS* status)
{
    uint16_t a = 0;
    uint32_t sta = 0;
    uint32_t fifoReg[2];
    REG_CiFIFOSTA ciFifoSta;
    REG_CiFIFOCON ciFifoCon;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Get FIFO registers
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_word_array(mcp2517_dev, a, fifoReg, 2);
    if (ret != RT_EOK) {
        return ret;
    }

    // Update data
    ciFifoCon.word = fifoReg[0];
    ciFifoSta.word = fifoReg[1];

    // Update status
    sta = ciFifoSta.byte[0];

    if (ciFifoCon.txBF.TxRequest) {
        sta |= CAN_TX_FIFO_TRANSMITTING;
    }

    *status = (CAN_TX_FIFO_STATUS) (sta & CAN_TX_FIFO_STATUS_MASK);

    return ret;
}

//DRV_CANFDSPI_ReceiveChannelReset
static rt_err_t can_spi_receive_channel_reset(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel)
{
    uint16_t a = 0;
    REG_CiFIFOCON ciFifoCon;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Address and data
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET) + 1; // Byte that contains FRESET
    ciFifoCon.word = 0;
    ciFifoCon.rxBF.FRESET = 1;

    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[1]);
    return ret;
}

//DRV_CANFDSPI_TransmitChannelReset
static rt_err_t can_spi_transmit_channel_reset(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel)
{
    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    return DRV_CANFDSPI_ReceiveChannelReset(mcp2517_dev, channel);
}

//DRV_CANFDSPI_TransmitRequestSet
static can_spi_transmit_request_set(MCP2517_Dev *mcp2517_dev, CAN_TXREQ_CHANNEL txreq)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Write TXREQ register
    uint32_t w = txreq;

    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiTXREQ, w);

    return ret;
}

//DRV_CANFDSPI_TransmitChannelAbort
static rt_err_t can_spi_transmit_channel_abort(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel)
{
  uint16_t a;
  uint8_t d;
  rt_err_t ret = -RT_ERROR;

  if(NULL == mcp2517_dev)
  {
      return -RT_ERROR;
  }

  // Address
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
  a += 1; // byte address of TXREQ

  // Clear TXREQ
  d = 0x00;

  // Write
  ret = can_spi_write_byte(mcp2517_dev, a, d);
  return ret;
}

//DRV_CANFDSPI_FilterObjectConfigure
static rt_err_t can_spi_filter_object_configure(MCP2517_Dev *mcp2517_dev, CAN_FILTER filter, CAN_FILTEROBJ_ID* id)
{
    uint16_t a;
    REG_CiFLTOBJ fObj;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev || NULL == id)
    {
        return -RT_ERROR;
    }

    // Setup
    fObj.word = 0;
    fObj.bF = *id;
    a = cREGADDR_CiFLTOBJ + (filter * CiFILTER_OFFSET);

    ret = can_spi_write_word(mcp2517_dev, a, fObj.word);
    return ret;
}

//DRV_CANFDSPI_FilterMaskConfigure
static rt_err_t can_spi_filter_mask_configure(MCP2517_Dev *mcp2517_dev, CAN_FILTER filter, CAN_MASKOBJ_ID* mask)
{
    uint16_t a;
    REG_CiMASK mObj;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev || NULL == mask)
    {
        return -RT_ERROR;
    }

    // Setup
    mObj.word = 0;
    mObj.bF = *mask;
    a = cREGADDR_CiMASK + (filter * CiFILTER_OFFSET);

    ret = can_spi_write_word(mcp2517_dev, a, mObj.word);

    return ret;
}

//DRV_CANFDSPI_FilterToFifoLink
static rt_err_t can_spi_filter_to_fifo_link(MCP2517_Dev *mcp2517_dev, CAN_FILTER filter, CAN_FIFO_CHANNEL channel, bool enable)
{
    uint16_t a;
    REG_CiFLTCON_BYTE fCtrl;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Enable
    if (enable) {
        fCtrl.bF.Enable = 1;
    } else {
        fCtrl.bF.Enable = 0;
    }

    // Link
    fCtrl.bF.BufferPointer = channel;
    a = cREGADDR_CiFLTCON + filter;

    ret = can_spi_write_byte(mcp2517_dev, a, fCtrl.byte);

    return ret;
}

//DRV_CANFDSPI_ReceiveChannelConfigure
static rt_err_t can_spi_receive_channel_configure(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_RX_FIFO_CONFIG* config)
{
    int8_t spiTransferError = 0;
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev || NULL == config)
    {
        return -RT_ERROR;
    }

    if (channel == CAN_TXQUEUE_CH0) {
        return -100;
    }

    // Setup FIFO
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = canFifoResetValues[0];

    ciFifoCon.rxBF.TxEnable = 0;
    ciFifoCon.rxBF.FifoSize = config->FifoSize;
    ciFifoCon.rxBF.PayLoadSize = config->PayLoadSize;
    ciFifoCon.rxBF.RxTimeStampEnable = config->RxTimeStampEnable;

    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

    ret = can_spi_write_word(mcp2517_dev, a, ciFifoCon.word);

    return ret;
}

//DRV_CANFDSPI_ReceiveChannelConfigureObjectReset
static rt_err_t can_spi_receive_channel_configure_object_reset(CAN_RX_FIFO_CONFIG* config)
{
    REG_CiFIFOCON ciFifoCon;

    if( NULL == config)
    {
        return -RT_ERROR;
    }

    ciFifoCon.word = canFifoResetValues[0];

    config->FifoSize = ciFifoCon.rxBF.FifoSize;
    config->PayLoadSize = ciFifoCon.rxBF.PayLoadSize;
    config->RxTimeStampEnable = ciFifoCon.rxBF.RxTimeStampEnable;

    return RT_EOK;
}

//DRV_CANFDSPI_ReceiveChannelUpdate
static rt_err_t can_spi_receive_channel_update(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel)
{
  uint16_t a = 0;
  REG_CiFIFOCON ciFifoCon;
  rt_err_t ret = -RT_ERROR;

  if(NULL == mcp2517_dev)
  {
      return -RT_ERROR;
  }

  ciFifoCon.word = 0;

  // Set UINC
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET) + 1; // Byte that contains FRESET
  ciFifoCon.rxBF.UINC = 1;

  // Write byte
  ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[1]);

  return ret;
}

//DRV_CANFDSPI_ReceiveMessageGet
static rt_err_t can_spi_receive_message_get(MCP2517_Dev *mcp2517_dev,
                                            CAN_FIFO_CHANNEL channel, CAN_RX_MSGOBJ* rxObj,
                                            uint8_t *rxd, uint8_t nBytes)
{
  uint8_t n = 0;
  uint8_t i = 0;
  uint16_t a;
  uint32_t fifoReg[3];
  REG_CiFIFOCON ciFifoCon;
  REG_CiFIFOSTA ciFifoSta;
  REG_CiFIFOUA ciFifoUa;
  rt_err_t ret = -RT_ERROR;

  if(NULL == mcp2517_dev || NULL == rxd || NULL == rxObj)
  {
      return -RT_ERROR;
  }

  // Get FIFO registers
  a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

  ret = can_spi_read_word_array(mcp2517_dev, a, fifoReg, 3);
  if (ret != RT_EOK) {
    return ret;
  }

  // Check that it is a receive buffer
  ciFifoCon.word = fifoReg[0];
  if (ciFifoCon.txBF.TxEnable) {
    return -RT_EINVAL;
  }

  // Get Status
  ciFifoSta.word = fifoReg[1];

  // Get address
  ciFifoUa.word = fifoReg[2];
#ifdef USERADDRESS_TIMES_FOUR
  a = 4 * ciFifoUa.bF.UserAddress;
#else
  a = ciFifoUa.bF.UserAddress;
#endif
  a += cRAMADDR_START;

  // Number of bytes to read
  n = nBytes + 8; // Add 8 header bytes

  if (ciFifoCon.rxBF.RxTimeStampEnable) {
    n += 4; // Add 4 time stamp bytes
  }

  // Make sure we read a multiple of 4 bytes from RAM
  if (n % 4) {
    n = n + 4 - (n % 4);
  }

  // Read rxObj using one access
  uint8_t ba[MAX_MSG_SIZE];

  if (n > MAX_MSG_SIZE) {
    n = MAX_MSG_SIZE;
  }

  ret = can_spi_read_byte_array(mcp2517_dev, a, ba, n);
  if (ret != RT_EOK) {
    return ret;
  }

  // Assign message header
  REG_t myReg;

  myReg.byte[0] = ba[0];
  myReg.byte[1] = ba[1];
  myReg.byte[2] = ba[2];
  myReg.byte[3] = ba[3];
  rxObj->word[0] = myReg.word;

  myReg.byte[0] = ba[4];
  myReg.byte[1] = ba[5];
  myReg.byte[2] = ba[6];
  myReg.byte[3] = ba[7];
  rxObj->word[1] = myReg.word;

  if (ciFifoCon.rxBF.RxTimeStampEnable) {
    myReg.byte[0] = ba[8];
    myReg.byte[1] = ba[9];
    myReg.byte[2] = ba[10];
    myReg.byte[3] = ba[11];
    rxObj->word[2] = myReg.word;

    // Assign message data
    for (i = 0; i < nBytes; i++) {
      rxd[i] = ba[i + 12];
    }
  } else {
    rxObj->word[2] = 0;

    // Assign message data
    for (i = 0; i < nBytes; i++) {
      rxd[i] = ba[i + 8];
    }
  }

  // UINC channel
  ret = can_spi_receive_channel_update(mcp2517_dev, channel);
  if (ret != RT_EOK) {
    return ret;
  }

  return ret;
}

//DRV_CANFDSPI_TefUpdate
static rt_err_t can_spi_tef_update(MCP2517_Dev *mcp2517_dev)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t a = 0;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Set UINC
    a = cREGADDR_CiTEFCON + 1;
    REG_CiTEFCON ciTefCon;
    ciTefCon.word = 0;
    ciTefCon.bF.UINC = 1;

    // Write byte
    ret = can_spi_write_byte(mcp2517_dev, a, ciTefCon.byte[1]);

    return ret;
}

//DRV_CANFDSPI_ModuleEventEnable
static rt_err_t can_spi_module_event_enable(MCP2517_Dev *mcp2517_dev, CAN_MODULE_EVENT flags)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t a = 0;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt Enables
    a = cREGADDR_CiINTENABLE;
    REG_CiINTENABLE intEnables;
    intEnables.word = 0;

    ret = can_spi_write_read_word(mcp2517_dev, a, &intEnables.word);
    if (ret != RT_EOK) {
        return ret;
    }

    // Modify
    intEnables.word |= (flags & CAN_ALL_EVENTS);

    // Write
    ret = can_spi_write_half_word(mcp2517_dev, a, intEnables.word);
    if (ret != RT_EOK) {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_TransmitChannelEventGet
static rt_err_t can_spi_transmit_channel_event_get(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_EVENT* flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev || NULL == flags)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt flags
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoSta.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    // Update data
    *flags = (CAN_TX_FIFO_EVENT) (ciFifoSta.byte[0] & CAN_TX_FIFO_ALL_EVENTS);

    return ret;
}

//DRV_CANFDSPI_TransmitChannelIndexGet
static rt_err_t can_spi_transmit_channel_index_get(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, uint8_t* idx)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev || NULL == idx)
    {
        return -RT_EINVAL;
    }

    // Read index
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_word(mcp2517_dev, a, &ciFifoSta.word);
    if (ret != RT_EOK) {
        return ret;
    }

    // Update data
    *idx = ciFifoSta.txBF.FifoIndex;

    return ret;
}

//DRV_CANFDSPI_TransmitChannelEventEnable
static rt_err_t can_spi_transmit_channel_event_enable(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_EVENT flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoCon.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    // Modify
    ciFifoCon.byte[0] |= (flags & CAN_TX_FIFO_ALL_EVENTS);

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_TransmitChannelEventDisable
static rt_err_t can_spi_transmit_channel_event_disable(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_EVENT flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoCon.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    // Modify
    ciFifoCon.byte[0] &= ~(flags & CAN_TX_FIFO_ALL_EVENTS);

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_TransmitChannelEventAttemptClear
static rt_err_t can_spi_transmit_channel_event_attempt_clear(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoSta.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    // Modify
    ciFifoSta.byte[0] &= ~CAN_TX_FIFO_ATTEMPTS_EXHAUSTED_EVENT;

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoSta.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_ReceiveChannelEventGet
static rt_err_t can_spi_receive_channel_event_get(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_RX_FIFO_EVENT* flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev || NULL == flags)
    {
        return -RT_EINVAL;
    }

    if (channel == CAN_TXQUEUE_CH0) return -100;

    // Read Interrupt flags
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoSta.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    // Update data
    *flags = (CAN_RX_FIFO_EVENT) (ciFifoSta.byte[0] & CAN_RX_FIFO_ALL_EVENTS);

    return ret;
}

//DRV_CANFDSPI_ReceiveChannelEventEnable
static rt_err_t can_spi_receive_channel_event_enable(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_RX_FIFO_EVENT flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    if (channel == CAN_TXQUEUE_CH0) return -100;

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoCon.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    // Modify
    ciFifoCon.byte[0] |= (flags & CAN_RX_FIFO_ALL_EVENTS);

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[0]);
    if (ret != RT_EOK) {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_ErrorCountStateGet
static rt_err_t can_spi_error_count_state_get(MCP2517_Dev *mcp2517_dev, uint8_t* tec, uint8_t* rec, CAN_ERROR_STATE* flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev || NULL == tec || NULL == rec || NULL == flags)
    {
        return -RT_EINVAL;
    }

    // Read Error
    a = cREGADDR_CiTREC;
    REG_CiTREC ciTrec;
    ciTrec.word = 0;

    ret = can_spi_read_word(mcp2517_dev, a, &ciTrec.word);
    if (ret != RT_EOK) {
        return ret;
    }

    // Update data
    *tec = ciTrec.byte[1];
    *rec = ciTrec.byte[0];
    *flags = (CAN_ERROR_STATE) (ciTrec.byte[2] & CAN_ERROR_ALL);

    return ret;
}

// Section: ECC
//DRV_CANFDSPI_EccEnable
static rt_err_t can_spi_ecc_enable(MCP2517_Dev *mcp2517_dev)
{
    uint8_t d = 0;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read
    ret = can_spi_read_byte(mcp2517_dev, cREGADDR_ECCCON, &d);
    if (ret != RT_EOK) {
        return ret;
    }

    // Modify
    d |= 0x01;

    // Write
    ret = can_spi_write_byte(mcp2517_dev, cREGADDR_ECCCON, d);
    if (ret != RT_EOK) {
        return ret;
    }

    return 0;
}

//DRV_CANFDSPI_RamInit
static rt_err_t can_spi_ram_init(MCP2517_Dev *mcp2517_dev, uint8_t d)
{
    uint8_t txd[SPI_DEFAULT_BUFFER_LENGTH];
    uint32_t k;
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Prepare data
    for (k = 0; k < SPI_DEFAULT_BUFFER_LENGTH; k++) {
        txd[k] = d;
    }

    uint16_t a = cRAMADDR_START;

    for (k = 0; k < (cRAM_SIZE / SPI_DEFAULT_BUFFER_LENGTH); k++) {
        ret = can_spi_write_byte_array(mcp2517_dev, a, txd, SPI_DEFAULT_BUFFER_LENGTH);
        if (ret != RT_EOK) {
            return ret;
        }
        a += SPI_DEFAULT_BUFFER_LENGTH;
    }

    return ret;
}

//DRV_CANFDSPI_BitTimeConfigure
static rt_err_t can_spi_bit_time_configure(MCP2517_Dev *mcp2517_dev,
                                            CAN_BITTIME_SETUP bitTime, CAN_SSP_MODE sspMode,
                                            CAN_SYSCLK_SPEED clk)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Decode clk
    switch (clk)
    {
        case CAN_SYSCLK_40M:
            ret = DRV_CANFDSPI_BitTimeConfigureNominal40MHz(index, bitTime);
            if (ret != RT_EOK) {
                return ret;
            }

            ret = DRV_CANFDSPI_BitTimeConfigureData40MHz(index, bitTime, sspMode);
            break;
        case CAN_SYSCLK_20M:
            ret = DRV_CANFDSPI_BitTimeConfigureNominal20MHz(index, bitTime);
            if (ret != RT_EOK) {
                return ret;
            }

            ret = DRV_CANFDSPI_BitTimeConfigureData20MHz(index, bitTime, sspMode);
            break;
        case CAN_SYSCLK_10M:
            ret = DRV_CANFDSPI_BitTimeConfigureNominal10MHz(index, bitTime);
            if (ret != RT_EOK) {
                return ret;
            }

            ret = DRV_CANFDSPI_BitTimeConfigureData10MHz(index, bitTime, sspMode);
            break;
        default:
            ret = -RT_ERROR;
            break;
    }

    return ret;
}

static rt_err_t mcp2517_can_spi_reset(MCP2517_Dev *mcp2517_dev)
{
    rt_err_t ret = -RT_ERROR;

    if(NULL == mcp2517_dev || NULL == mcp2517_dev->spi_dev)
    {
        return -RT_ERROR;
    }

    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) (cINSTRUCTION_RESET << 4);
    mcp2517_dev->spi_tx_buffer[1] = 0;

    ret = rt_spi_transfer(mcp2517_dev->spi_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 2);
    return ret;
}

static void mcp2517_spi_irq_callback(void *param)
{
    //HAL_SPICANIRQ_Callback
}

static rt_err_t mcp2517_init(MCP2517_Dev *mcp2517_dev)
{
    rt_err_t ret = -RT_ERROR;

    if(mcp2517_dev == NULL)
    {
        return -RT_ERROR;
    }

    ret = mcp2517_can_spi_reset(mcp2517_dev);
    if(ret != RT_EOK)
    {
        LOG_D("can spi reset error!");
        return ret;
    }

    for(int i = 0; i < 10000; i++) {};





}


static rt_err_t rt_mcp2517_init(void)
{
    rt_err_t ret = -RT_ERROR;

    for(rt_uint8_t i = 0; i < sizeof(mcp2517_can_port) / sizeof(MCP2517_Dev); i++)
    {
        /* get pin index */
        mcp2517_can_port[i].spi_cs_pin_index = rt_pin_get(mcp2517_can_port[i].spi_cs_pin_name);
        mcp2517_can_port[i].spi_irq_pin_index = rt_pin_get(mcp2517_can_port[i].spi_irq_pin_name);

        /* attach cs pin */
        if(rt_hw_spi_device_attach(mcp2517_can_port[i].spi_name, mcp2517_can_port[i].spi_dev_name, mcp2517_can_port[i].spi_cs_pin_index) != RT_EOK)
        {
            LOG_D("can port %d %s dev %d attach cs %d pin error", i, mcp2517_can_port[i].spi_name, mcp2517_can_port[i].spi_dev_name, mcp2517_can_port[i].spi_irq_pin_index);
            return -RT_ERROR;
        }

        /* find device */
        mcp2517_can_port[i].spi_dev = (struct rt_spi_device *)rt_device_find(mcp2517_can_port[i].spi_dev_name);
        if(RT_NULL == mcp2517_can_port[i].spi_dev)
        {
            LOG_E("mcp2517 can %d not find %s", i, mcp2517_can_port[i].spi_name);
            return -RT_ERROR;
        }

        mcp2517_can_port[i].dev.user_data = &mcp2517_can_port[i];

        /* set irq pin */
        rt_pin_mode(mcp2517_can_port[i].spi_irq_pin_index, PIN_MODE_INPUT_PULLUP);
        rt_pin_attach_irq(mcp2517_can_port[i].spi_irq_pin_index, PIN_IRQ_MODE_FALLING, mcp2517_spi_irq_callback, (void *)&mcp2517_can_port[i]);
    }
}

#endif /* BSP_USING_MCP2517FD */
