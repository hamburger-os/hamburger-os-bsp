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

#define DBG_TAG "mcp2517fd"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2
// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1
#define MAX_TXQUEUE_ATTEMPTS 50

#define SPI_DEFAULT_BUFFER_LENGTH (96)

#define MCP2517_CAN_RX_FLAG (0xa5)

typedef enum
{
#ifdef BSP_USE_MCP2517FD_CAN1
    MCP2517FD_CH_1,
#endif

#ifdef BSP_USE_MCP2517FD_CAN2
    MCP2517FD_CH_2,
#endif

#ifdef BSP_USE_MCP2517FD_CAN3
    MCP2517FD_CH_3,
#endif

#ifdef BSP_USE_MCP2517FD_CAN4
    MCP2517FD_CH_4,
#endif

    MCP2517FD_CH_ALL,
} E_MCP2517FD_CHANNEL;

typedef enum
{
    MCP2517FD_CAN_LIST_CLER_MODE_ONE = 0,
    MCP2517FD_CAN_LIST_CLER_MODE_ALL = 1,
} E_MCP2517FD_CAN_LIST_CLER_MODE;

typedef struct tagMcp2517CanBuf
{
    rt_list_t list; /* 链表 */
    uint16_t flag; /* 接收标志位 */
    struct rt_can_msg can_msg; /* 接收缓冲区 */
} MCP2517_CAN_BUF;

typedef struct
{
    MCP2517_CAN_BUF *rx_head;
    uint32_t rx_num;
} MCP2517_CAN_LIST;

typedef struct
{
    struct rt_device dev; /* 设备 */
    const char *dev_name; /* 设备名 */
    struct rt_spi_device *spi_dev; /* SPI总线设备句柄 */

    const char *spi_name; /* SPI总线名 */
    const char *spi_cs_pin_name; /* spi片选引脚名 */
    rt_uint8_t spi_cs_pin_index; /* spi片选引脚索引 */
    const char *spi_irq_pin_name; /* spi中断引脚名 */
    rt_uint8_t spi_irq_pin_index; /* spi中断引脚索引 */

    E_MCP2517FD_CHANNEL channel; /* can通道索引 */
    uint32_t baud;
    uint32_t mode;

    uint8_t spi_tx_buffer[SPI_DEFAULT_BUFFER_LENGTH + 5]; /* SPI Transmit buffer */
    uint8_t spi_rx_buffer[SPI_DEFAULT_BUFFER_LENGTH + 5]; /* SPI Receive buffer */

    MCP2517_CAN_LIST rx_list;
} MCP2517_Dev;

static MCP2517_Dev mcp2517_can_port[] = {

#ifdef BSP_USE_MCP2517FD_CAN1
    {
        .spi_name = BSP_MCP2517FD_CAN1_SPI_BUS,
        .dev_name = "mcp2517fd1",
        .spi_cs_pin_name = BSP_MCP2517FD_CAN1_CS_PIN,
        .spi_irq_pin_name = BSP_MCP2517FD_CAN1_INT_PIN,
        .channel = MCP2517FD_CH_1,
        .baud = CAN_500K_2M,
        .mode = CAN_NORMAL_MODE,
    },
#endif  /* BSP_USE_MCP2517FD_CAN1 */

#ifdef BSP_USE_MCP2517FD_CAN2
    {
        .spi_name = BSP_MCP2517FD_CAN2_SPI_BUS,
        .dev_name = "mcp2517fd2",
        .spi_cs_pin_name = BSP_MCP2517FD_CAN2_CS_PIN,
        .spi_irq_pin_name = BSP_MCP2517FD_CAN2_INT_PIN,
        .channel = MCP2517FD_CH_2,
        .baud = CAN_500K_2M,
        .mode = CAN_NORMAL_MODE,
    },
#endif  /* BSP_USE_MCP2517FD_CAN2 */

#ifdef BSP_USE_MCP2517FD_CAN3
    {
        .spi_name = BSP_MCP2517FD_CAN3_SPI_BUS,
        .dev_name = "mcp2517fd3",
        .spi_cs_pin_name = BSP_MCP2517FD_CAN3_CS_PIN,
        .spi_irq_pin_name = BSP_MCP2517FD_CAN3_INT_PIN,
        .channel = MCP2517FD_CH_3,
        .baud = CAN_500K_2M,
        .mode = CAN_NORMAL_MODE,
    },
#endif  /* BSP_USE_MCP2517FD_CAN3 */

#ifdef BSP_USE_MCP2517FD_CAN4
    {
        .spi_name = BSP_MCP2517FD_CAN4_SPI_BUS,
        .dev_name = "mcp2517fd4",
        .spi_cs_pin_name = BSP_MCP2517FD_CAN4_CS_PIN,
        .spi_irq_pin_name = BSP_MCP2517FD_CAN4_INT_PIN,
        .channel = MCP2517FD_CH_4,
        .baud = CAN_500K_2M,
        .mode = CAN_NORMAL_MODE,
    },
#endif  /* BSP_USE_MCP2517FD_CAN4 */
};

static rt_mq_t mcp2517fd_rx_event_mq = NULL;

//! Look-up table for CRC calculation
const uint16_t crc16_table[256] =
{
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

static rt_err_t mcp2517_can_list_init(MCP2517_CAN_LIST *can_list)
{
    if (NULL == can_list)
    {
        return -RT_ERROR;
    }

    can_list->rx_num = 0;

    /* 1.申请接收缓冲区空间 */
    can_list->rx_head = rt_malloc(sizeof(MCP2517_CAN_BUF));
    if (NULL == can_list->rx_head)
    {
        LOG_E("malloc size %d error", sizeof(MCP2517_CAN_BUF));
        return -RT_EEMPTY;
    }

    /* 2.接收缓冲区清零 */
    rt_memset((void *) can_list->rx_head, 0, sizeof(MCP2517_CAN_BUF));

    can_list->rx_head->flag = 0;
    /* 3.初始化链表 */
    rt_list_init(&can_list->rx_head->list);
    if (!rt_list_isempty(&can_list->rx_head->list))
    {
        LOG_E("rx_list init error");
        rt_free(can_list->rx_head);
        return -RT_EEMPTY;
    }

    return RT_EOK;
}

/*
 * @brief  清除CAN数据缓冲区，释放缓冲区.
 * @param  can_list 缓冲区指针
 * @param  mode 0：释放一包的缓冲区；1：释放全部缓冲区
 * @retval rt_err_t;
 */
static rt_err_t mcp2517_can_list_clear(MCP2517_CAN_LIST *can_list, E_MCP2517FD_CAN_LIST_CLER_MODE mode)
{
    MCP2517_CAN_BUF *can_buf = RT_NULL;
    rt_list_t *list_pos = NULL;
    rt_list_t *list_next = NULL;
    rt_base_t level;

    if (NULL == can_list)
    {
        return -RT_EEMPTY;
    }
    if (NULL == can_list->rx_head)
    {
        LOG_E("rx_head error");
        return -RT_EEMPTY;
    }

    if (0 == MCP2517FD_CAN_LIST_CLER_MODE_ONE)
    {
        level = rt_hw_interrupt_disable();
        rt_list_for_each_safe(list_pos, list_next, &can_list->rx_head->list)
        {
            can_buf = rt_list_entry(list_pos, struct tagMcp2517CanBuf, list);
            if (can_buf != RT_NULL)
            {
                if ((can_buf->flag & MCP2517_CAN_RX_FLAG) != 0U)
                {
                    rt_list_remove(list_pos);
                    rt_hw_interrupt_enable(level);
                    /* 释放接收接收缓冲区 */
                    rt_free(can_buf);
                    can_list->rx_num--;
                    return RT_EOK;
                }
            }
        }
        rt_hw_interrupt_enable(level);
    }
    else
    {
        level = rt_hw_interrupt_disable();
        rt_list_for_each_safe(list_pos, list_next, &can_list->rx_head->list)
        {
            can_buf = rt_list_entry(list_pos, struct tagMcp2517CanBuf, list);
            if (can_buf != RT_NULL)
            {
                if ((can_buf->flag & MCP2517_CAN_RX_FLAG) != 0U)
                {
                    rt_list_remove(list_pos);
                    rt_hw_interrupt_enable(level);
                    /* 释放接收接收缓冲区 */
                    rt_free(can_buf);
                    can_list->rx_num--;
                }
            }
        }
        rt_hw_interrupt_enable(level);
    }
    return RT_EOK;
}

static rt_err_t can_spi_drv_transfer_data(MCP2517_Dev *mcp2517_dev,
                                            const void *send_buf,
                                            void *recv_buf,
                                            rt_size_t length)
{
    rt_size_t result = 0;

    result = rt_spi_transfer(mcp2517_dev->spi_dev, send_buf, recv_buf, length);
    if (result != length)
    {
        LOG_HEX("send", 16, (uint8_t *)send_buf, length);
        LOG_HEX("recv", 16, (uint8_t *)recv_buf, length);
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}

//DRV_CANFDSPI_ReadByte
static rt_err_t can_spi_read_byte(MCP2517_Dev *mcp2517_dev, uint16_t address, uint8_t *rxd)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);
    mcp2517_dev->spi_tx_buffer[2] = 0xFF;

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 3);

    // Update data
    *rxd = mcp2517_dev->spi_rx_buffer[2];
    return ret;
}

//DRV_CANFDSPI_WriteByte
static rt_err_t can_spi_write_byte(MCP2517_Dev *mcp2517_dev, uint16_t address, uint8_t txd)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_WRITE << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);
    mcp2517_dev->spi_tx_buffer[2] = txd;

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 3);
    return ret;
}

//DRV_CANFDSPI_ReadWord
static rt_err_t can_spi_read_word(MCP2517_Dev *mcp2517_dev, uint16_t address, uint32_t *rxd)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t i = 0;
    uint32_t x = 0;

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);
    mcp2517_dev->spi_tx_buffer[2] = 0xFF;
    mcp2517_dev->spi_tx_buffer[3] = 0xFF;
    mcp2517_dev->spi_tx_buffer[4] = 0xFF;
    mcp2517_dev->spi_tx_buffer[5] = 0xFF;

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 6);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Update data
    *rxd = 0;
    for (i = 2; i < 6; i++)
    {
        x = (uint32_t) mcp2517_dev->spi_rx_buffer[i];
        *rxd += x << ((i - 2) * 8);
    }

    return ret;
}

//DRV_CANFDSPI_WriteWord
static rt_err_t can_spi_write_word(MCP2517_Dev *mcp2517_dev, uint16_t address, uint32_t txd)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t i = 0;

    if (NULL == mcp2517_dev)
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

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 6);
    return ret;
}

//DRV_CANFDSPI_ReadHalfWord
static rt_err_t can_spi_read_half_word(MCP2517_Dev *mcp2517_dev, uint16_t address, uint16_t *rxd)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t i = 0;
    uint32_t x = 0;

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);
    mcp2517_dev->spi_tx_buffer[2] = 0xFF;
    mcp2517_dev->spi_tx_buffer[3] = 0xFF;

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 4);
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

//DRV_CANFDSPI_WriteHalfWord
static rt_err_t can_spi_write_half_word(MCP2517_Dev *mcp2517_dev, uint16_t address, uint16_t txd)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t i = 0;

    if (NULL == mcp2517_dev)
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

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 4);
    return ret;
}

//DRV_CANFDSPI_ReadByteArray
static rt_err_t can_spi_read_byte_array(MCP2517_Dev *mcp2517_dev, uint16_t address, uint8_t *rxd, uint16_t nBytes)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t i = 0;
    uint16_t transfer_size = nBytes + 2;

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) ((cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF));
    mcp2517_dev->spi_tx_buffer[1] = (uint8_t) (address & 0xFF);
    // Clear data
    for (i = 2; i < transfer_size; i++)
    {
        mcp2517_dev->spi_tx_buffer[i] = 0xFF;
    }

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, transfer_size);

    // Update data
    for (i = 0; i < nBytes; i++)
    {
        rxd[i] = mcp2517_dev->spi_rx_buffer[i + 2];
    }

    return ret;
}

//DRV_CANFDSPI_WriteByteArray
static rt_err_t can_spi_write_byte_array(MCP2517_Dev *mcp2517_dev, uint16_t address, uint8_t *txd, uint16_t nBytes)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t i = 0;
    uint16_t transfer_size = nBytes + 2;

    if (NULL == mcp2517_dev)
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

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, transfer_size);
    return ret;
}

//DRV_CANFDSPI_ReadWordArray
static rt_err_t can_spi_read_word_array(MCP2517_Dev *mcp2517_dev, uint16_t address, uint32_t *rxd, uint16_t nWords)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t i, j, n;
    REG_t w;
    uint16_t transfer_size = nWords * 4 + 2;

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Compose command
    mcp2517_dev->spi_tx_buffer[0] = (cINSTRUCTION_READ << 4) + ((address >> 8) & 0xF);
    mcp2517_dev->spi_tx_buffer[1] = address & 0xFF;

    // Clear data
    for (i = 2; i < transfer_size; i++)
    {
        mcp2517_dev->spi_tx_buffer[i] = 0xFF;
    }

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, transfer_size);
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

//DRV_CANFDSPI_WriteWordArray
static rt_err_t can_spi_write_word_array(MCP2517_Dev *mcp2517_dev, uint16_t address, uint32_t *txd, uint16_t nWords)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t i, j, n;
    REG_t w;
    uint16_t transfer_size = nWords * 4 + 2;

    if (NULL == mcp2517_dev)
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

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, transfer_size);
    return ret;
}

/*************************************************************************************************
 功能：将数据长度标识转换为实际字节数
 参数：dlc：数据长度标识
 返回值：dataBytesInObject can数据字节数
 **************************************************************************************************/
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

    if (NULL == mcp2517_dev || NULL == config)
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

    if (NULL == config)
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

//DRV_CANFDSPI_OperationModeSelect
static rt_err_t can_spi_operation_mode_select(MCP2517_Dev *mcp2517_dev, CAN_OPERATION_MODE opMode)
{
    rt_err_t ret = -RT_ERROR;
    uint8_t d = 0;

    if (NULL == mcp2517_dev)
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
static rt_err_t can_spi_transmit_channel_configure(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel,
        CAN_TX_FIFO_CONFIG* config)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t a = 0;

    if (NULL == mcp2517_dev || NULL == config)
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

    if (NULL == config)
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

    if (NULL == mcp2517_dev || NULL == config)
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

    if (NULL == config)
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

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Set UINC
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET) + 1; // Byte that contains FRESET
    ciFifoCon.word = 0;
    ciFifoCon.txBF.UINC = 1;

    // Set TXREQ
    if (flush)
    {
        ciFifoCon.txBF.TxRequest = 1;
    }

    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[1]);
    if (ret != RT_EOK)
    {
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

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Get FIFO registers
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_word_array(mcp2517_dev, a, fifoReg, 3);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Check that it is a transmit buffer
    ciFifoCon.word = fifoReg[0];
    if (!ciFifoCon.txBF.TxEnable)
    {
        return -RT_EIO;
    }

    // Check that DLC is big enough for data
    dataBytesInObject = can_spi_dlc_to_data_bytes((CAN_DLC) txObj->bF.ctrl.DLC);
    if (dataBytesInObject < txdNumBytes)
    {
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
    for (i = 0; i < txdNumBytes; i++)
    {
        txBuffer[i + 8] = txd[i];
    }

    // Make sure we write a multiple of 4 bytes to RAM
    uint16_t n = 0;
    uint8_t j = 0;

    if (txdNumBytes % 4)
    {
        // Need to add bytes
        n = 4 - (txdNumBytes % 4);
        i = txdNumBytes + 8;

        for (j = 0; j < n; j++)
        {
            txBuffer[i + 8 + j] = 0;
        }
    }

    ret = can_spi_write_byte_array(mcp2517_dev, a, txBuffer, txdNumBytes + 8 + n);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Set UINC and TXREQ
    ret = can_spi_transmit_channel_update(mcp2517_dev, channel, flush);
    if (ret != RT_EOK)
    {
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

    if (NULL == mcp2517_dev)
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

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Get FIFO registers
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_word_array(mcp2517_dev, a, fifoReg, 2);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Update data
    ciFifoCon.word = fifoReg[0];
    ciFifoSta.word = fifoReg[1];

    // Update status
    sta = ciFifoSta.byte[0];

    if (ciFifoCon.txBF.TxRequest)
    {
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

    if (NULL == mcp2517_dev)
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
    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    return can_spi_receive_channel_reset(mcp2517_dev, channel);
}

//DRV_CANFDSPI_TransmitRequestSet
static rt_err_t can_spi_transmit_request_set(MCP2517_Dev *mcp2517_dev, CAN_TXREQ_CHANNEL txreq)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
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

    if (NULL == mcp2517_dev)
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

    if (NULL == mcp2517_dev || NULL == id)
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

    if (NULL == mcp2517_dev || NULL == mask)
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

    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    // Enable
    if (enable)
    {
        fCtrl.bF.Enable = 1;
    }
    else
    {
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
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev || NULL == config)
    {
        return -RT_ERROR;
    }

    if (channel == CAN_TXQUEUE_CH0)
    {
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

    if ( NULL == config)
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

    if (NULL == mcp2517_dev)
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

    if (NULL == mcp2517_dev || NULL == rxd || NULL == rxObj)
    {
        return -RT_ERROR;
    }

    // Get FIFO registers
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_word_array(mcp2517_dev, a, fifoReg, 3);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Check that it is a receive buffer
    ciFifoCon.word = fifoReg[0];
    if (ciFifoCon.txBF.TxEnable)
    {
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

    if (ciFifoCon.rxBF.RxTimeStampEnable)
    {
        n += 4; // Add 4 time stamp bytes
    }

    // Make sure we read a multiple of 4 bytes from RAM
    if (n % 4)
    {
        n = n + 4 - (n % 4);
    }

    // Read rxObj using one access
    uint8_t ba[MAX_MSG_SIZE];

    if (n > MAX_MSG_SIZE)
    {
        n = MAX_MSG_SIZE;
    }

    ret = can_spi_read_byte_array(mcp2517_dev, a, ba, n);
    if (ret != RT_EOK)
    {
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

    if (ciFifoCon.rxBF.RxTimeStampEnable)
    {
        myReg.byte[0] = ba[8];
        myReg.byte[1] = ba[9];
        myReg.byte[2] = ba[10];
        myReg.byte[3] = ba[11];
        rxObj->word[2] = myReg.word;

        // Assign message data
        for (i = 0; i < nBytes; i++)
        {
            rxd[i] = ba[i + 12];
        }
    }
    else
    {
        rxObj->word[2] = 0;

        // Assign message data
        for (i = 0; i < nBytes; i++)
        {
            rxd[i] = ba[i + 8];
        }
    }

    // UINC channel
    ret = can_spi_receive_channel_update(mcp2517_dev, channel);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_TefUpdate
static rt_err_t can_spi_tef_update(MCP2517_Dev *mcp2517_dev)
{
    rt_err_t ret = -RT_ERROR;
    uint16_t a = 0;

    if (NULL == mcp2517_dev)
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

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt Enables
    a = cREGADDR_CiINTENABLE;
    REG_CiINTENABLE intEnables;
    intEnables.word = 0;

    ret = can_spi_read_half_word(mcp2517_dev, a, &intEnables.word);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Modify
    intEnables.word |= (flags & CAN_ALL_EVENTS);

    // Write
    ret = can_spi_write_half_word(mcp2517_dev, a, intEnables.word);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_TransmitChannelEventGet
static rt_err_t can_spi_transmit_channel_event_get(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_EVENT* flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev || NULL == flags)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt flags
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoSta.byte[0]);
    if (ret != RT_EOK)
    {
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

    if (NULL == mcp2517_dev || NULL == idx)
    {
        return -RT_EINVAL;
    }

    // Read index
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_word(mcp2517_dev, a, &ciFifoSta.word);
    if (ret != RT_EOK)
    {
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

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoCon.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Modify
    ciFifoCon.byte[0] |= (flags & CAN_TX_FIFO_ALL_EVENTS);

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_TransmitChannelEventDisable
static rt_err_t can_spi_transmit_channel_event_disable(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel, CAN_TX_FIFO_EVENT flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoCon.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Modify
    ciFifoCon.byte[0] &= ~(flags & CAN_TX_FIFO_ALL_EVENTS);

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_TransmitChannelEventAttemptClear
static rt_err_t can_spi_transmit_channel_event_attempt_clear(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoSta.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Modify
    ciFifoSta.byte[0] &= ~CAN_TX_FIFO_ATTEMPTS_EXHAUSTED_EVENT;

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoSta.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_ReceiveChannelEventGet
static rt_err_t can_spi_receive_channel_event_get(MCP2517_Dev *mcp2517_dev, CAN_FIFO_CHANNEL channel,
        CAN_RX_FIFO_EVENT* flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev || NULL == flags)
    {
        return -RT_EINVAL;
    }

    if (channel == CAN_TXQUEUE_CH0)
        return -100;

    // Read Interrupt flags
    REG_CiFIFOSTA ciFifoSta;
    ciFifoSta.word = 0;
    a = cREGADDR_CiFIFOSTA + (channel * CiFIFO_OFFSET);

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoSta.byte[0]);
    if (ret != RT_EOK)
    {
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

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    if (channel == CAN_TXQUEUE_CH0)
        return -100;

    // Read Interrupt Enables
    a = cREGADDR_CiFIFOCON + (channel * CiFIFO_OFFSET);
    REG_CiFIFOCON ciFifoCon;
    ciFifoCon.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &ciFifoCon.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Modify
    ciFifoCon.byte[0] |= (flags & CAN_RX_FIFO_ALL_EVENTS);

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, ciFifoCon.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_ErrorCountStateGet
static rt_err_t can_spi_error_count_state_get(MCP2517_Dev *mcp2517_dev, uint8_t* tec, uint8_t* rec, CAN_ERROR_STATE* flags)
{
    uint16_t a = 0;
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev || NULL == tec || NULL == rec || NULL == flags)
    {
        return -RT_EINVAL;
    }

    // Read Error
    a = cREGADDR_CiTREC;
    REG_CiTREC ciTrec;
    ciTrec.word = 0;

    ret = can_spi_read_word(mcp2517_dev, a, &ciTrec.word);
    if (ret != RT_EOK)
    {
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

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Read
    ret = can_spi_read_byte(mcp2517_dev, cREGADDR_ECCCON, &d);
    if (ret != RT_EOK)
    {
        LOG_E("can spi ecc enable read error");
        return ret;
    }

    // Modify
    d |= 0x01;

    // Write
    ret = can_spi_write_byte(mcp2517_dev, cREGADDR_ECCCON, d);
    if (ret != RT_EOK)
    {
        LOG_E("can spi ecc enable write error");
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_RamInit
static rt_err_t can_spi_ram_init(MCP2517_Dev *mcp2517_dev, uint8_t d)
{
    uint8_t txd[SPI_DEFAULT_BUFFER_LENGTH];
    uint32_t k;
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Prepare data
    for (k = 0; k < SPI_DEFAULT_BUFFER_LENGTH; k++)
    {
        txd[k] = d;
    }

    uint16_t a = cRAMADDR_START;

    for (k = 0; k < (cRAM_SIZE / SPI_DEFAULT_BUFFER_LENGTH); k++)
    {
        ret = can_spi_write_byte_array(mcp2517_dev, a, txd, SPI_DEFAULT_BUFFER_LENGTH);
        if (ret != RT_EOK)
        {
            return ret;
        }
        a += SPI_DEFAULT_BUFFER_LENGTH;
    }

    return ret;
}

//DRV_CANFDSPI_BitTimeConfigureNominal40MHz
static rt_err_t can_spi_bit_time_configure_nominal_40mhz(MCP2517_Dev *mcp2517_dev, CAN_BITTIME_SETUP bitTime)
{
    rt_err_t ret = -RT_ERROR;
    REG_CiNBTCFG ciNbtcfg;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    ciNbtcfg.word = canControlResetValues[cREGADDR_CiNBTCFG / 4];

    // Arbitration Bit rate
    switch (bitTime)
    {
    // All 500K
    case CAN_500K_1M:
    case CAN_500K_2M:
    case CAN_500K_3M:
    case CAN_500K_4M:
    case CAN_500K_5M:
    case CAN_500K_6M7:
    case CAN_500K_8M:
    case CAN_500K_10M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 62;
        ciNbtcfg.bF.TSEG2 = 15;
        ciNbtcfg.bF.SJW = 15;
        break;

        // All 250K
    case CAN_250K_500K:
    case CAN_250K_833K:
    case CAN_250K_1M:
    case CAN_250K_1M5:
    case CAN_250K_2M:
    case CAN_250K_3M:
    case CAN_250K_4M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 126;
        ciNbtcfg.bF.TSEG2 = 31;
        ciNbtcfg.bF.SJW = 31;
        break;

    case CAN_1000K_4M:
    case CAN_1000K_8M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 30;
        ciNbtcfg.bF.TSEG2 = 7;
        ciNbtcfg.bF.SJW = 7;
        break;

    case CAN_125K_500K:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 254;
        ciNbtcfg.bF.TSEG2 = 63;
        ciNbtcfg.bF.SJW = 63;
        break;

    default:
        return -RT_ERROR;
    }

    // Write Bit time registers
    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiNBTCFG, ciNbtcfg.word);

    return ret;
}

//DRV_CANFDSPI_BitTimeConfigureData40MHz
static rt_err_t can_spi_bit_time_configure_data_40mhz(MCP2517_Dev *mcp2517_dev, CAN_BITTIME_SETUP bitTime, CAN_SSP_MODE sspMode)
{
    rt_err_t ret = -RT_ERROR;
    REG_CiDBTCFG ciDbtcfg;
    REG_CiTDC ciTdc;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    //    sspMode;

    ciDbtcfg.word = canControlResetValues[cREGADDR_CiDBTCFG / 4];
    ciTdc.word = 0;

    // Configure Bit time and sample point
    ciTdc.bF.TDCMode = CAN_SSP_MODE_AUTO;
    uint32_t tdcValue = 0;

    // Data Bit rate and SSP
    switch (bitTime)
    {
    case CAN_500K_1M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 30;
        ciDbtcfg.bF.TSEG2 = 7;
        ciDbtcfg.bF.SJW = 7;
        // SSP
        ciTdc.bF.TDCOffset = 31;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_2M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 14;
        ciDbtcfg.bF.TSEG2 = 3;
        ciDbtcfg.bF.SJW = 3;
        // SSP
        ciTdc.bF.TDCOffset = 15;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_3M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 8;
        ciDbtcfg.bF.TSEG2 = 2;
        ciDbtcfg.bF.SJW = 2;
        // SSP
        ciTdc.bF.TDCOffset = 9;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_4M:
    case CAN_1000K_4M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 6;
        ciDbtcfg.bF.TSEG2 = 1;
        ciDbtcfg.bF.SJW = 1;
        // SSP
        ciTdc.bF.TDCOffset = 7;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_5M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 4;
        ciDbtcfg.bF.TSEG2 = 1;
        ciDbtcfg.bF.SJW = 1;
        // SSP
        ciTdc.bF.TDCOffset = 5;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_6M7:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 3;
        ciDbtcfg.bF.TSEG2 = 0;
        ciDbtcfg.bF.SJW = 0;
        // SSP
        ciTdc.bF.TDCOffset = 4;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_8M:
    case CAN_1000K_8M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 2;
        ciDbtcfg.bF.TSEG2 = 0;
        ciDbtcfg.bF.SJW = 0;
        // SSP
        ciTdc.bF.TDCOffset = 3;
        ciTdc.bF.TDCValue = 1;
        break;
    case CAN_500K_10M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 1;
        ciDbtcfg.bF.TSEG2 = 0;
        ciDbtcfg.bF.SJW = 0;
        // SSP
        ciTdc.bF.TDCOffset = 2;
        ciTdc.bF.TDCValue = 0;
        break;

    case CAN_250K_500K:
    case CAN_125K_500K:
        ciDbtcfg.bF.BRP = 1;
        ciDbtcfg.bF.TSEG1 = 30;
        ciDbtcfg.bF.TSEG2 = 7;
        ciDbtcfg.bF.SJW = 7;
        // SSP
        ciTdc.bF.TDCOffset = 31;
        ciTdc.bF.TDCValue = tdcValue;
        ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
        break;
    case CAN_250K_833K:
        ciDbtcfg.bF.BRP = 1;
        ciDbtcfg.bF.TSEG1 = 17;
        ciDbtcfg.bF.TSEG2 = 4;
        ciDbtcfg.bF.SJW = 4;
        // SSP
        ciTdc.bF.TDCOffset = 18;
        ciTdc.bF.TDCValue = tdcValue;
        ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
        break;
    case CAN_250K_1M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 30;
        ciDbtcfg.bF.TSEG2 = 7;
        ciDbtcfg.bF.SJW = 7;
        // SSP
        ciTdc.bF.TDCOffset = 31;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_1M5:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 18;
        ciDbtcfg.bF.TSEG2 = 5;
        ciDbtcfg.bF.SJW = 5;
        // SSP
        ciTdc.bF.TDCOffset = 19;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_2M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 14;
        ciDbtcfg.bF.TSEG2 = 3;
        ciDbtcfg.bF.SJW = 3;
        // SSP
        ciTdc.bF.TDCOffset = 15;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_3M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 8;
        ciDbtcfg.bF.TSEG2 = 2;
        ciDbtcfg.bF.SJW = 2;
        // SSP
        ciTdc.bF.TDCOffset = 9;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_4M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 6;
        ciDbtcfg.bF.TSEG2 = 1;
        ciDbtcfg.bF.SJW = 1;
        // SSP
        ciTdc.bF.TDCOffset = 7;
        ciTdc.bF.TDCValue = tdcValue;
        break;

    default:
        return -RT_ERROR;
    }

    // Write Bit time registers
    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiDBTCFG, ciDbtcfg.word);
    if (ret != RT_EOK)
    {
        return ret;
    }

// Write Transmitter Delay Compensation
#ifdef REV_A
    ciTdc.bF.TDCOffset = 0;
    ciTdc.bF.TDCValue = 0;
#endif

    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiTDC, ciTdc.word);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_BitTimeConfigureNominal20MHz
static rt_err_t can_spi_bit_time_configure_nominal_20mhz(MCP2517_Dev *mcp2517_dev, CAN_BITTIME_SETUP bitTime)
{
    REG_CiNBTCFG ciNbtcfg;
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    ciNbtcfg.word = canControlResetValues[cREGADDR_CiNBTCFG / 4];

    // Arbitration Bit rate
    switch (bitTime)
    {
    // All 500K
    case CAN_500K_1M:
    case CAN_500K_2M:
    case CAN_500K_4M:
    case CAN_500K_5M:
    case CAN_500K_6M7:
    case CAN_500K_8M:
    case CAN_500K_10M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 30;
        ciNbtcfg.bF.TSEG2 = 7;
        ciNbtcfg.bF.SJW = 7;
        break;

        // All 250K
    case CAN_250K_500K:
    case CAN_250K_833K:
    case CAN_250K_1M:
    case CAN_250K_1M5:
    case CAN_250K_2M:
    case CAN_250K_3M:
    case CAN_250K_4M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 62;
        ciNbtcfg.bF.TSEG2 = 15;
        ciNbtcfg.bF.SJW = 15;
        break;

    case CAN_1000K_4M:
    case CAN_1000K_8M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 14;
        ciNbtcfg.bF.TSEG2 = 3;
        ciNbtcfg.bF.SJW = 3;
        break;

    case CAN_125K_500K:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 126;
        ciNbtcfg.bF.TSEG2 = 31;
        ciNbtcfg.bF.SJW = 31;
        break;

    default:
        return -RT_ERROR;
    }

    // Write Bit time registers
    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiNBTCFG, ciNbtcfg.word);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_BitTimeConfigureData20MHz
static rt_err_t can_spi_bit_time_configure_data_20mhz(MCP2517_Dev *mcp2517_dev, CAN_BITTIME_SETUP bitTime,
        CAN_SSP_MODE sspMode)
{
    REG_CiDBTCFG ciDbtcfg;
    REG_CiTDC ciTdc;

    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    //    sspMode;
    ciDbtcfg.word = canControlResetValues[cREGADDR_CiDBTCFG / 4];
    ciTdc.word = 0;

    // Configure Bit time and sample point
    ciTdc.bF.TDCMode = CAN_SSP_MODE_AUTO;
    uint32_t tdcValue = 0;

    // Data Bit rate and SSP
    switch (bitTime)
    {
    case CAN_500K_1M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 14;
        ciDbtcfg.bF.TSEG2 = 3;
        ciDbtcfg.bF.SJW = 3;
        // SSP
        ciTdc.bF.TDCOffset = 15;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_2M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 6;
        ciDbtcfg.bF.TSEG2 = 1;
        ciDbtcfg.bF.SJW = 1;
        // SSP
        ciTdc.bF.TDCOffset = 7;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_4M:
    case CAN_1000K_4M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 2;
        ciDbtcfg.bF.TSEG2 = 0;
        ciDbtcfg.bF.SJW = 0;
        // SSP
        ciTdc.bF.TDCOffset = 3;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_5M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 1;
        ciDbtcfg.bF.TSEG2 = 0;
        ciDbtcfg.bF.SJW = 0;
        // SSP
        ciTdc.bF.TDCOffset = 2;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_6M7:
    case CAN_500K_8M:
    case CAN_500K_10M:
    case CAN_1000K_8M:
        //qDebug("Data Bitrate not feasible with this clock!");
        return -RT_ERROR;

    case CAN_250K_500K:
    case CAN_125K_500K:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 30;
        ciDbtcfg.bF.TSEG2 = 7;
        ciDbtcfg.bF.SJW = 7;
        // SSP
        ciTdc.bF.TDCOffset = 31;
        ciTdc.bF.TDCValue = tdcValue;
        ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
        break;
    case CAN_250K_833K:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 17;
        ciDbtcfg.bF.TSEG2 = 4;
        ciDbtcfg.bF.SJW = 4;
        // SSP
        ciTdc.bF.TDCOffset = 18;
        ciTdc.bF.TDCValue = tdcValue;
        ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
        break;
    case CAN_250K_1M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 14;
        ciDbtcfg.bF.TSEG2 = 3;
        ciDbtcfg.bF.SJW = 3;
        // SSP
        ciTdc.bF.TDCOffset = 15;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_1M5:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 8;
        ciDbtcfg.bF.TSEG2 = 2;
        ciDbtcfg.bF.SJW = 2;
        // SSP
        ciTdc.bF.TDCOffset = 9;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_2M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 6;
        ciDbtcfg.bF.TSEG2 = 1;
        ciDbtcfg.bF.SJW = 1;
        // SSP
        ciTdc.bF.TDCOffset = 7;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_3M:
        //qDebug("Data Bitrate not feasible with this clock!");
        return -RT_ERROR;
    case CAN_250K_4M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 2;
        ciDbtcfg.bF.TSEG2 = 0;
        ciDbtcfg.bF.SJW = 0;
        // SSP
        ciTdc.bF.TDCOffset = 3;
        ciTdc.bF.TDCValue = tdcValue;
        break;

    default:
        return -RT_ERROR;
    }

    // Write Bit time registers
    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiDBTCFG, ciDbtcfg.word);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Write Transmitter Delay Compensation
#ifdef REV_A
    ciTdc.bF.TDCOffset = 0;
    ciTdc.bF.TDCValue = 0;
#endif

    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiTDC, ciTdc.word);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_BitTimeConfigureNominal10MHz
static rt_err_t can_spi_bit_time_configure_nominal_10mhz(MCP2517_Dev *mcp2517_dev, CAN_BITTIME_SETUP bitTime)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    REG_CiNBTCFG ciNbtcfg;

    ciNbtcfg.word = canControlResetValues[cREGADDR_CiNBTCFG / 4];

    // Arbitration Bit rate
    switch (bitTime)
    {
    // All 500K
    case CAN_500K_1M:
    case CAN_500K_2M:
    case CAN_500K_4M:
    case CAN_500K_5M:
    case CAN_500K_6M7:
    case CAN_500K_8M:
    case CAN_500K_10M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 14;
        ciNbtcfg.bF.TSEG2 = 3;
        ciNbtcfg.bF.SJW = 3;
        break;

        // All 250K
    case CAN_250K_500K:
    case CAN_250K_833K:
    case CAN_250K_1M:
    case CAN_250K_1M5:
    case CAN_250K_2M:
    case CAN_250K_3M:
    case CAN_250K_4M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 30;
        ciNbtcfg.bF.TSEG2 = 7;
        ciNbtcfg.bF.SJW = 7;
        break;

    case CAN_1000K_4M:
    case CAN_1000K_8M:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 7;
        ciNbtcfg.bF.TSEG2 = 2;
        ciNbtcfg.bF.SJW = 2;
        break;

    case CAN_125K_500K:
        ciNbtcfg.bF.BRP = 0;
        ciNbtcfg.bF.TSEG1 = 62;
        ciNbtcfg.bF.TSEG2 = 15;
        ciNbtcfg.bF.SJW = 15;
        break;

    default:
        return -RT_ERROR;
    }

    // Write Bit time registers
    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiNBTCFG, ciNbtcfg.word);
    if (ret != RT_EOK)
    {
        return ret;
    }
    return ret;
}

//DRV_CANFDSPI_BitTimeConfigureData10MHz
static rt_err_t can_spi_bit_time_configure_data_10mhz(MCP2517_Dev *mcp2517_dev, CAN_BITTIME_SETUP bitTime, CAN_SSP_MODE sspMode)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    REG_CiDBTCFG ciDbtcfg;
    REG_CiTDC ciTdc;

    ciDbtcfg.word = canControlResetValues[cREGADDR_CiDBTCFG / 4];
    ciTdc.word = 0;

    // Configure Bit time and sample point
    ciTdc.bF.TDCMode = CAN_SSP_MODE_AUTO;
    uint32_t tdcValue = 0;

    // Data Bit rate and SSP
    switch (bitTime)
    {
    case CAN_500K_1M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 6;
        ciDbtcfg.bF.TSEG2 = 1;
        ciDbtcfg.bF.SJW = 1;
        // SSP
        ciTdc.bF.TDCOffset = 7;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_2M:
        // Data BR
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 2;
        ciDbtcfg.bF.TSEG2 = 0;
        ciDbtcfg.bF.SJW = 0;
        // SSP
        ciTdc.bF.TDCOffset = 3;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_500K_4M:
    case CAN_500K_5M:
    case CAN_500K_6M7:
    case CAN_500K_8M:
    case CAN_500K_10M:
    case CAN_1000K_4M:
    case CAN_1000K_8M:
        //qDebug("Data Bitrate not feasible with this clock!");
        return -RT_ERROR;

    case CAN_250K_500K:
    case CAN_125K_500K:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 14;
        ciDbtcfg.bF.TSEG2 = 3;
        ciDbtcfg.bF.SJW = 3;
        // SSP
        ciTdc.bF.TDCOffset = 15;
        ciTdc.bF.TDCValue = tdcValue;
        ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
        break;
    case CAN_250K_833K:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 7;
        ciDbtcfg.bF.TSEG2 = 2;
        ciDbtcfg.bF.SJW = 2;
        // SSP
        ciTdc.bF.TDCOffset = 8;
        ciTdc.bF.TDCValue = tdcValue;
        ciTdc.bF.TDCMode = CAN_SSP_MODE_OFF;
        break;
    case CAN_250K_1M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 6;
        ciDbtcfg.bF.TSEG2 = 1;
        ciDbtcfg.bF.SJW = 1;
        // SSP
        ciTdc.bF.TDCOffset = 7;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_1M5:
        //qDebug("Data Bitrate not feasible with this clock!");
        return -RT_ERROR;
    case CAN_250K_2M:
        ciDbtcfg.bF.BRP = 0;
        ciDbtcfg.bF.TSEG1 = 2;
        ciDbtcfg.bF.TSEG2 = 0;
        ciDbtcfg.bF.SJW = 0;
        // SSP
        ciTdc.bF.TDCOffset = 3;
        ciTdc.bF.TDCValue = tdcValue;
        break;
    case CAN_250K_3M:
    case CAN_250K_4M:
        //qDebug("Data Bitrate not feasible with this clock!");
        return -RT_ERROR;
    default:
        return -RT_ERROR;
    }

    // Write Bit time registers
    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiDBTCFG, ciDbtcfg.word);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Write Transmitter Delay Compensation
#ifdef REV_A
    ciTdc.bF.TDCOffset = 0;
    ciTdc.bF.TDCValue = 0;
#endif

    ret = can_spi_write_word(mcp2517_dev, cREGADDR_CiTDC, ciTdc.word);
    if (ret != RT_EOK)
    {
        return ret;
    }
    return ret;
}

//DRV_CANFDSPI_BitTimeConfigure
static rt_err_t can_spi_bit_time_configure(MCP2517_Dev *mcp2517_dev, CAN_BITTIME_SETUP bitTime, CAN_SSP_MODE sspMode,
        CAN_SYSCLK_SPEED clk)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    // Decode clk
    switch (clk)
    {
    case CAN_SYSCLK_40M:
        ret = can_spi_bit_time_configure_nominal_40mhz(mcp2517_dev, bitTime);
        if (ret != RT_EOK)
        {
            return ret;
        }

        ret = can_spi_bit_time_configure_data_40mhz(mcp2517_dev, bitTime, sspMode);
        break;
    case CAN_SYSCLK_20M:
        ret = can_spi_bit_time_configure_nominal_20mhz(mcp2517_dev, bitTime);
        if (ret != RT_EOK)
        {
            return ret;
        }

        ret = can_spi_bit_time_configure_data_20mhz(mcp2517_dev, bitTime, sspMode);
        break;
    case CAN_SYSCLK_10M:
        ret = can_spi_bit_time_configure_nominal_10mhz(mcp2517_dev, bitTime);
        if (ret != RT_EOK)
        {
            return ret;
        }

        ret = can_spi_bit_time_configure_data_10mhz(mcp2517_dev, bitTime, sspMode);
        break;
    default:
        ret = -RT_ERROR;
        break;
    }
    return ret;
}

//DRV_CANFDSPI_GpioModeConfigure
static rt_err_t can_spi_gpio_mode_configure(MCP2517_Dev *mcp2517_dev, GPIO_PIN_MODE gpio0, GPIO_PIN_MODE gpio1)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    uint16_t a = 0;

    // Read
    a = cREGADDR_IOCON + 3;
    REG_IOCON iocon;
    iocon.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &iocon.byte[3]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Modify
    iocon.bF.PinMode0 = gpio0;
    iocon.bF.PinMode1 = gpio1;

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, iocon.byte[3]);
    if (ret != RT_EOK)
    {
        return ret;
    }
    return ret;
}

//DRV_CANFDSPI_GpioDirectionConfigure
static rt_err_t can_spi_gpio_direction_configure(MCP2517_Dev *mcp2517_dev, GPIO_PIN_DIRECTION gpio0, GPIO_PIN_DIRECTION gpio1)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev)
    {
        return -RT_EINVAL;
    }

    uint16_t a = 0;

    // Read
    a = cREGADDR_IOCON;
    REG_IOCON iocon;
    iocon.word = 0;

    ret = can_spi_read_byte(mcp2517_dev, a, &iocon.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    // Modify
    iocon.bF.TRIS0 = gpio0;
    iocon.bF.TRIS1 = gpio1;

    // Write
    ret = can_spi_write_byte(mcp2517_dev, a, iocon.byte[0]);
    if (ret != RT_EOK)
    {
        return ret;
    }

    return ret;
}

//DRV_CANFDSPI_CalculateCRC16
#define CRCBASE    0xFFFF
#define CRCUPPER   1
static uint16_t can_spi_calculate_crc16(uint8_t* data, uint16_t size)
{
    uint16_t init = CRCBASE;
    uint8_t index;

    while (size-- != 0)
    {
        index = ((uint8_t*) &init)[CRCUPPER] ^ *data++;
        init = (init << 8) ^ crc16_table[index];
    }

    return init;
}

//DRV_CANFDSPI_DataBytesToDlc
/*************************************************************************************************
 功能：将CAN数据字节数转换为数据长度标识
 参数：n can数据字节数
 返回：dlc 数据长度标识
 **************************************************************************************************/
static CAN_DLC can_spi_data_bytes_to_dlc(uint8_t n)
{
    CAN_DLC dlc = MCP_CAN_DLC_0;

    if (n <= 4)
    {
        dlc = MCP_CAN_DLC_4;
    }
    else if (n <= 8)
    {
        dlc = MCP_CAN_DLC_8;
    }
    else if (n <= 12)
    {
        dlc = MCP_CAN_DLC_12;
    }
    else if (n <= 16)
    {
        dlc = MCP_CAN_DLC_16;
    }
    else if (n <= 20)
    {
        dlc = MCP_CAN_DLC_20;
    }
    else if (n <= 24)
    {
        dlc = MCP_CAN_DLC_24;
    }
    else if (n <= 32)
    {
        dlc = MCP_CAN_DLC_32;
    }
    else if (n <= 48)
    {
        dlc = MCP_CAN_DLC_48;
    }
    else if (n <= 64)
    {
        dlc = MCP_CAN_DLC_64;
    }

    return dlc;
}

//MCP2517_CAN_GetFrame
/*************************************************************************************************
 功能：从MCP2517读取接收到的数据
 参数：mcp2517_dev 设备对象
 返回：无
 **************************************************************************************************/
static rt_err_t mcp2517_can_get_frame(MCP2517_Dev *mcp2517_dev)
{
    if (NULL == mcp2517_dev)
    {
        return -RT_ERROR;
    }

    rt_err_t ret = -RT_ERROR;
    CAN_RX_FIFO_EVENT rxFlags;
    CAN_RX_MSGOBJ rxObj;
    struct rt_can_msg *pmsg = NULL;
    MCP2517_CAN_BUF *mcp2517_can_buf = NULL;
    rt_base_t level;

    ret = can_spi_receive_channel_event_get(mcp2517_dev, APP_RX_FIFO, &rxFlags);
    if (ret != RT_EOK)
    {
        return ret;
    }

    /* MCP2517内有数据就会全部读出来 */
    while (rxFlags & CAN_RX_FIFO_NOT_EMPTY_EVENT)
    {
        if (mcp2517_dev->rx_list.rx_num < BSP_MCP2517FD_RX_BUF_NUM)
        {
            mcp2517_can_buf = rt_malloc(sizeof(MCP2517_CAN_BUF));
            if (mcp2517_can_buf != NULL)
            {
                mcp2517_dev->rx_list.rx_num++;
                pmsg = &mcp2517_can_buf->can_msg;
            }
            else
            {
                LOG_E("mcp2517_can_buf malloc null");
                return -RT_EEMPTY;
            }
        }
        else
        {
            //释放一个buf
            if (RT_EOK == mcp2517_can_list_clear(&mcp2517_dev->rx_list, MCP2517FD_CAN_LIST_CLER_MODE_ONE))
            {
                //再申请一个Buf
                mcp2517_can_buf = rt_malloc(sizeof(MCP2517_CAN_BUF));
                if (mcp2517_can_buf != NULL)
                {
                    mcp2517_dev->rx_list.rx_num++;
                    pmsg = &mcp2517_can_buf->can_msg;
                }
                else
                {
                    LOG_E("mcp2517_can_buf clear and malloc null");
                    return -RT_EEMPTY;
                }
            }
            else
            {
                LOG_E("mcp2517_can_list_clear one error");
                return -RT_EEMPTY;
            }
        }

        can_spi_receive_message_get(mcp2517_dev, APP_RX_FIFO, &rxObj, pmsg->data, MAX_DATA_BYTES);
        pmsg->len = can_spi_dlc_to_data_bytes(rxObj.bF.ctrl.DLC);
        if (rxObj.bF.ctrl.IDE == 1)
        {
            pmsg->id = (rxObj.bF.id.SID << 18) | rxObj.bF.id.EID;
        }
        else
        {
            pmsg->id = rxObj.bF.id.SID;
        }

        mcp2517_can_buf->flag = MCP2517_CAN_RX_FLAG;
        level = rt_hw_interrupt_disable();
        rt_list_insert_before(&mcp2517_dev->rx_list.rx_head->list, &mcp2517_can_buf->list);
        rt_hw_interrupt_enable(level);

        rt_size_t rx_length;
        if (mcp2517_dev->dev.rx_indicate != NULL)
        {
            level = rt_hw_interrupt_disable();
            rx_length = rt_list_len(&mcp2517_dev->rx_list.rx_head->list) * sizeof(struct rt_can_msg);
            rt_hw_interrupt_enable(level);
            mcp2517_dev->dev.rx_indicate(&mcp2517_dev->dev, rx_length);
        }

        ret = can_spi_receive_channel_event_get(mcp2517_dev, APP_RX_FIFO, &rxFlags);
        if (ret != RT_EOK)
        {
            /* 查询失败就退出本次接收 */
            return ret;
        }
    }
    return RT_EOK;
}

//DRV_CANFDSPI_Reset
static rt_err_t mcp2517_can_spi_reset(MCP2517_Dev *mcp2517_dev)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == mcp2517_dev || NULL == mcp2517_dev->spi_dev)
    {
        return -RT_ERROR;
    }

    mcp2517_dev->spi_tx_buffer[0] = (uint8_t) (cINSTRUCTION_RESET << 4);
    mcp2517_dev->spi_tx_buffer[1] = 0;

    ret = can_spi_drv_transfer_data(mcp2517_dev, mcp2517_dev->spi_tx_buffer, mcp2517_dev->spi_rx_buffer, 2);
    return ret;
}

static rt_err_t mcp2517_spi_rx(MCP2517_Dev *mcp2517_dev)
{
    rt_err_t ret;

    ret = mcp2517_can_get_frame(mcp2517_dev);
    if (ret != RT_EOK)
    {
        return ret;
    }
    return ret;
}

/* mcp2517fd rx thread */
static void mcp2517_spi_thread_entry(void *arg)
{
    rt_err_t ret = 0;
    E_MCP2517FD_CHANNEL event_channel = 0;
    while (1)
    {
        /* 从消息队列中接收消息, 阻塞方式 */
        ret = rt_mq_recv(mcp2517fd_rx_event_mq, (void *)&event_channel, sizeof(E_MCP2517FD_CHANNEL), (rt_int32_t)RT_WAITING_FOREVER);
        if (RT_EOK == ret)
        {
            if (event_channel >= MCP2517FD_CH_1 && event_channel < MCP2517FD_CH_ALL)
            {
                if (mcp2517_spi_rx(&mcp2517_can_port[event_channel]) != RT_EOK)
                {
                    LOG_E("rx ch %d error", event_channel);
                }
            }
            else
            {
                LOG_E("rx event ch %d error", event_channel);
            }

        }

        rt_thread_mdelay(10);
    }
}

static rt_err_t mcp2517_spi_thread_init(void)
{
    static rt_thread_t mcp2517_thread = RT_NULL;

    /* 创建消息队列 */
    mcp2517fd_rx_event_mq = rt_mq_create("mcp2517fd_event_mq", sizeof(E_MCP2517FD_CHANNEL),
            (rt_size_t) BSP_MCP2517FD_RX_EVENT_MSG_QUEUE_MAX_NUM, (rt_uint8_t) RT_IPC_FLAG_FIFO);
    if (NULL == mcp2517fd_rx_event_mq)
    {
        return -RT_ERROR;
    }

    mcp2517_thread = rt_thread_create("mcp2517fd rx", mcp2517_spi_thread_entry, RT_NULL,
    BSP_MCP2517FD_RX_THREAD_STACK_SIZE, BSP_MCP2517FD_RX_THREAD_PRIORITY, BSP_MCP2517FD_RX_THREAD_TIMESLICE);
    if (mcp2517_thread != NULL)
    {
        rt_thread_startup(mcp2517_thread);
        return RT_EOK;
    }
    else
    {
        LOG_E("thread create error!");
        return -RT_ERROR;
    }
}

static rt_err_t mcp2517_open(rt_device_t dev, rt_uint16_t oflag)
{
    rt_err_t ret = -RT_ERROR;

    if (NULL == dev)
    {
        return ret;
    }

    MCP2517_Dev *mcp2517_dev = NULL;

    mcp2517_dev = (MCP2517_Dev *) dev;
    if (mcp2517_dev == NULL)
    {
        return -RT_ERROR;
    }

    CAN_CONFIG config;
    REG_CiFLTOBJ fObj;
    REG_CiMASK mObj;
    CAN_TX_FIFO_CONFIG txConfig;
    CAN_RX_FIFO_CONFIG rxConfig;

    /* 芯片内部复位 */
    ret = mcp2517_can_spi_reset(mcp2517_dev);
    if (ret != RT_EOK)
    {
        LOG_E("can spi reset error!");
        return ret;
    }

    /* enable ECC and initialize RAM */
    ret = can_spi_ecc_enable(mcp2517_dev);
    if(ret != RT_EOK)
    {
        LOG_E("can_spi_ecc_enable error!");
        return ret;
    }

    ret = can_spi_ram_init(mcp2517_dev, 0xff);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_ram_init error!");
        return ret;
    }

    /* Configure device */
    can_spi_configure_object_reset(&config);
    config.IsoCrcEnable = 1;
    config.StoreInTEF = 0;
    config.BitRateSwitchDisable = 1;
    ret = can_spi_configure(mcp2517_dev, &config);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_configure error!");
        return ret;
    }

    /* Setup TX FIFO */
    can_spi_transmit_channel_configure_object_reset(&txConfig);
    txConfig.FifoSize = 7;
    txConfig.PayLoadSize = CAN_PLSIZE_64;
    txConfig.TxPriority = 1;
    ret = can_spi_transmit_channel_configure(mcp2517_dev, APP_TX_FIFO, &txConfig);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_transmit_channel_configure error!");
        return ret;
    }

    /* Setup RX FIFO */
    can_spi_receive_channel_configure_object_reset(&rxConfig);
    rxConfig.FifoSize = 7;
    rxConfig.PayLoadSize = CAN_PLSIZE_64;
    ret = can_spi_receive_channel_configure(mcp2517_dev, APP_RX_FIFO, &rxConfig);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_receive_channel_configure error!");
        return ret;
    }

    /* Setup RX Filter */
    fObj.word = 0;
    fObj.bF.SID = 0x000;
    fObj.bF.EXIDE = 0;
    fObj.bF.EID = 0x00;
    ret = can_spi_filter_object_configure(mcp2517_dev, CAN_FILTER0, &fObj.bF);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_filter_object_configure error!");
        return ret;
    }

    /* Setup RX Mask */
    mObj.word = 0;
    mObj.bF.MSID = 0x000;
    mObj.bF.MIDE = 0; /* 支持扩展帧 */
    mObj.bF.MEID = 0x0;
    ret = can_spi_filter_mask_configure(mcp2517_dev, CAN_FILTER0, &mObj.bF);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_filter_mask_configure error!");
        return ret;
    }

    /* Link FIFO and Filter */
    ret = can_spi_filter_to_fifo_link(mcp2517_dev, CAN_FILTER0, APP_RX_FIFO, true);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_filter_to_fifo_link error!");
        return ret;
    }

    /* Setup Bit Time 设置波特率 */
    ret = can_spi_bit_time_configure(mcp2517_dev, mcp2517_dev->baud, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_bit_time_configure error!");
        return ret;
    }

    /* Setup Transmit and Receive Interrupts */
    ret = can_spi_gpio_mode_configure(mcp2517_dev, GPIO_MODE_INT, GPIO_MODE_INT);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_gpio_mode_configure error!");
        return ret;
    }
    ret = can_spi_receive_channel_event_enable(mcp2517_dev, APP_RX_FIFO, CAN_RX_FIFO_ALL_EVENTS);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_gpio_mode_configure error!");
        return ret;
    }
    //  DRV_CANFDSPI_ModuleEventEnable(cps, CAN_TX_EVENT | CAN_RX_EVENT);
    ret = can_spi_module_event_enable(mcp2517_dev, CAN_RX_EVENT);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_module_event_enable error!");
        return ret;
    }

    // Select Normal Mode
    /* CAN  CANFD 都支持 */
    ret = can_spi_operation_mode_select(mcp2517_dev, mcp2517_dev->mode);
    if (ret != RT_EOK)
    {
        LOG_E("can_spi_operation_mode_select error!");
        return ret;
    }

    ret = mcp2517_can_list_init(&mcp2517_dev->rx_list);
    if (ret != RT_EOK)
    {
        LOG_E("can %d list init error", mcp2517_dev->channel);
        return -RT_ERROR;
    }

    rt_pin_irq_enable(mcp2517_dev->spi_irq_pin_index, PIN_IRQ_ENABLE);

    return ret;
}

static rt_err_t mcp2517_close(rt_device_t dev)
{
    if (NULL == dev)
    {
        return -RT_ERROR;
    }

    MCP2517_Dev *mcp2517_dev = NULL;

    mcp2517_dev = (MCP2517_Dev *) dev;
    if (mcp2517_dev == NULL)
    {
        return -RT_ERROR;
    }

    if (mcp2517_can_list_clear(&mcp2517_dev->rx_list, MCP2517FD_CAN_LIST_CLER_MODE_ALL) != RT_EOK)
    {
        LOG_E("mcp2517_can_list_clear all error");
        return -RT_ERROR;
    }

    rt_free(mcp2517_dev->rx_list.rx_head);
    return RT_EOK;
}

static rt_size_t mcp2517_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    if (NULL == dev)
    {
        return 0;
    }

    MCP2517_Dev *mcp2517_dev = NULL;

    mcp2517_dev = (MCP2517_Dev *) dev;
    if (mcp2517_dev == NULL)
    {
        return 0;
    }

    rt_size_t msg_size = size;
    MCP2517_CAN_BUF *can_buf = RT_NULL;
    struct rt_can_msg *data = (struct rt_can_msg *) buffer;
    rt_base_t level;

    while (size)
    {
        level = rt_hw_interrupt_disable();
        /* step1：遍历链表 */
        if (!rt_list_isempty(&mcp2517_dev->rx_list.rx_head->list))
        {
            can_buf = rt_list_entry(mcp2517_dev->rx_list.rx_head->list.next, struct tagMcp2517CanBuf, list);
            if (can_buf != RT_NULL)
            {
                if ((can_buf->flag & MCP2517_CAN_RX_FLAG) != 0U)
                {
                    /* step2：提取包数据 */
                    rt_memcpy(data, &can_buf->can_msg, sizeof(struct rt_can_msg));
                    rt_list_remove(&can_buf->list);
                    /* step3：释放接收接收缓冲区 */
                    rt_free(can_buf);
                    mcp2517_dev->rx_list.rx_num--;
                }
            }
            rt_hw_interrupt_enable(level);
        }
        else
        {
            rt_hw_interrupt_enable(level);
            break;
        }
        data++;
        size -= sizeof(struct rt_can_msg);
    }
    return (msg_size - size);
}

static rt_size_t mcp2517_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    if (NULL == dev)
    {
        return 0;
    }

    MCP2517_Dev *mcp2517_dev = NULL;

    mcp2517_dev = (MCP2517_Dev *) dev;
    if (mcp2517_dev == NULL)
    {
        return 0;
    }

    if (NULL == buffer || 0 == size)
    {
        return 0;
    }

    CAN_TX_MSGOBJ txObj_tmp = { 0U };
    rt_can_msg_t can_msg = (rt_can_msg_t) buffer;
    static uint8_t txd_tmp[MAX_DATA_BYTES];
    uint8_t attempts = MAX_TXQUEUE_ATTEMPTS;
    CAN_TX_FIFO_EVENT txFlags_tmp = { 0U };
    CAN_ERROR_STATE errorFlags_tmp;
    uint8_t tec_tmp, rec_tmp;

    txObj_tmp.word[0] = 0;
    txObj_tmp.word[1] = 1;
    if (can_msg->id <= 0x7FFU)
    {
        /* 标准帧 */
        txObj_tmp.bF.id.SID = can_msg->id;
        txObj_tmp.bF.ctrl.IDE = 0;
    }
    else
    {
        /* 扩展帧 */
        txObj_tmp.bF.id.SID = (can_msg->id >> 18) & 0X7FFU;
        txObj_tmp.bF.id.EID = can_msg->id & 0X3FFFFU;
        txObj_tmp.bF.ctrl.IDE = 1;
    }

    txObj_tmp.bF.ctrl.BRS = 1; /* 数据区加速 */
    txObj_tmp.bF.ctrl.DLC = can_spi_data_bytes_to_dlc(can_msg->len);
    txObj_tmp.bF.ctrl.FDF = can_msg->fd_frame;  /* 0: CAN2.0模式发送 / 1: FDCAN模式发送 */
    txObj_tmp.bF.ctrl.ESI = 0;
    txObj_tmp.bF.ctrl.SEQ = 2;

    // Configure message data
    for (uint8_t i = 0; i < can_msg->len; i++)
    {
        txd_tmp[i] = can_msg->data[i];
    }

    // Check if FIFO is not full
    do
    {
        can_spi_transmit_channel_event_get(mcp2517_dev, APP_TX_FIFO, &txFlags_tmp);
        if (attempts == 0)
        {
            rt_thread_mdelay(1);
            can_spi_error_count_state_get(mcp2517_dev, &tec_tmp, &rec_tmp, &errorFlags_tmp);
            return 0;
        }
        attempts--;
    } while (!(txFlags_tmp & CAN_TX_FIFO_NOT_FULL_EVENT));

    // Load message and transmit
    uint8_t n = can_spi_dlc_to_data_bytes(txObj_tmp.bF.ctrl.DLC);
    if (can_spi_transmit_channel_load(mcp2517_dev, APP_TX_FIFO, &txObj_tmp, txd_tmp, n, true) == RT_EOK)
    {
        return sizeof(struct rt_can_msg);
    }
    else
    {
        return 0;
    }
}

static rt_err_t mcp2517_control(rt_device_t dev, int cmd, void *args)
{
    rt_uint32_t argval;
    rt_err_t ret;

    if (NULL == dev)
    {
        return -RT_ERROR;
    }

    MCP2517_Dev *mcp2517_dev = NULL;

    mcp2517_dev = (MCP2517_Dev *) dev;
    if (mcp2517_dev == NULL)
    {
        return -RT_ERROR;
    }

    switch (cmd)
    {
    case RT_CAN_CMD_SET_BAUD:
        argval = (rt_uint32_t) args;

        switch (argval)
        {
        case CAN500kBaud:
            if (CAN_500K_2M != mcp2517_dev->baud)
            {
                ret = can_spi_bit_time_configure(mcp2517_dev, CAN_500K_2M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);
                if (ret != RT_EOK)
                {
                    LOG_E("control set 500K 2M error!");
                    return ret;
                }
                mcp2517_dev->baud = CAN_500K_2M;
            }
            break;
        case CAN250kBaud:
            if (CAN_250K_2M != mcp2517_dev->baud)
            {
                ret = can_spi_bit_time_configure(mcp2517_dev, CAN_250K_2M, CAN_SSP_MODE_AUTO, CAN_SYSCLK_40M);
                if (ret != RT_EOK)
                {
                    LOG_E("control set 250K 2M error!");
                    return ret;
                }
                mcp2517_dev->baud = CAN_250K_2M;
            }
            break;
        default:
            return -RT_ERROR;
        }
        break;
    case RT_CAN_CMD_SET_MODE:
        argval = (rt_uint32_t) args;

        switch (argval)
        {
        case RT_CAN_MODE_NORMAL:
            if (mcp2517_dev->mode != CAN_NORMAL_MODE)
            {
                ret = can_spi_operation_mode_select(mcp2517_dev, CAN_NORMAL_MODE);
                if (ret != RT_EOK)
                {
                    LOG_E("control set normal error!");
                    return ret;
                }
                mcp2517_dev->mode = CAN_NORMAL_MODE;
            }
            break;
        case RT_CAN_MODE_LISTEN:
            if (mcp2517_dev->mode != CAN_LISTEN_ONLY_MODE)
            {
                ret = can_spi_operation_mode_select(mcp2517_dev, CAN_LISTEN_ONLY_MODE);
                if (ret != RT_EOK)
                {
                    LOG_E("control set listen error!");
                    return ret;
                }
                mcp2517_dev->mode = CAN_LISTEN_ONLY_MODE;
            }
            break;
        case RT_CAN_MODE_LOOPBACK:
            if (mcp2517_dev->mode != CAN_INTERNAL_LOOPBACK_MODE)
            {
                ret = can_spi_operation_mode_select(mcp2517_dev, CAN_INTERNAL_LOOPBACK_MODE);
                if (ret != RT_EOK)
                {
                    LOG_E("control set internal loopback error!");
                    return ret;
                }
                mcp2517_dev->mode = CAN_INTERNAL_LOOPBACK_MODE;
            }
            break;
        default:
            return -RT_ERROR;

        }
        break;
    default:
        return -RT_ERROR;
    }
    return RT_EOK;
}

static void mcp2517_spi_irq_callback(void *param)
{
    if (NULL == param)
    {
        return;
    }

    MCP2517_Dev *mcp2517_dev = NULL;

    mcp2517_dev = (MCP2517_Dev *) param;
    if (mcp2517_dev == NULL)
    {
        return;
    }

    rt_mq_send(mcp2517fd_rx_event_mq, &mcp2517_dev->channel, sizeof(E_MCP2517FD_CHANNEL));
}

static int rt_hw_mcp2517_init(void)
{
    int ret = -RT_ERROR;
    char dev_name[RT_NAME_MAX];

    for (rt_uint8_t i = 0; i < sizeof(mcp2517_can_port) / sizeof(MCP2517_Dev); i++)
    {
        /* get pin index */
        mcp2517_can_port[i].spi_cs_pin_index = rt_pin_get(mcp2517_can_port[i].spi_cs_pin_name);
        mcp2517_can_port[i].spi_irq_pin_index = rt_pin_get(mcp2517_can_port[i].spi_irq_pin_name);

        rt_memset(dev_name, 0, sizeof(dev_name));
        uint8_t dev_num = 0;
        do
        {
            rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", mcp2517_can_port[i].spi_name, dev_num++);
            if (dev_num == 255)
            {
                return -RT_EIO;
            }
        } while (rt_device_find(dev_name));


        /* attach cs pin */
        if (rt_hw_spi_device_attach(mcp2517_can_port[i].spi_name, dev_name, mcp2517_can_port[i].spi_cs_pin_index) != RT_EOK)
        {
            LOG_D("can port %d %s dev %d attach cs %d pin error", i, mcp2517_can_port[i].spi_name, dev_name,
                    mcp2517_can_port[i].spi_irq_pin_index);
            return -RT_ERROR;
        }

        /* find device */
        mcp2517_can_port[i].spi_dev = (struct rt_spi_device *) rt_device_find(dev_name);
        if (RT_NULL == mcp2517_can_port[i].spi_dev)
        {
            LOG_E("mcp2517 can %d not find %s", i, dev_name);
            return -RT_ERROR;
        }

        /* cfg spi */
        struct rt_spi_configuration cfg = { 0 };
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_3 | RT_SPI_MSB;
        cfg.max_hz = BSP_MCP2517FD_SPI_SPEED;
        ret = rt_spi_configure(mcp2517_can_port[i].spi_dev, &cfg);
        if (ret != RT_EOK)
        {
            LOG_E("device %s configure error %d!", dev_name, ret);
            return -RT_EIO;
        }

        /* set irq pin */
        rt_pin_mode(mcp2517_can_port[i].spi_irq_pin_index, PIN_MODE_INPUT_PULLUP);
        rt_pin_attach_irq(mcp2517_can_port[i].spi_irq_pin_index, PIN_IRQ_MODE_FALLING,
                            mcp2517_spi_irq_callback, (void *) &mcp2517_can_port[i]);

        /* set user data */
        mcp2517_can_port[i].dev.user_data = &mcp2517_can_port[i];

        mcp2517_can_port[i].dev.type = RT_Device_Class_CAN;
        /* set device ops */
        mcp2517_can_port[i].dev.init = NULL; //mcp2517_init;
        mcp2517_can_port[i].dev.open = mcp2517_open;
        mcp2517_can_port[i].dev.close = mcp2517_close;
        mcp2517_can_port[i].dev.read = mcp2517_read;
        mcp2517_can_port[i].dev.write = mcp2517_write;
        mcp2517_can_port[i].dev.control = mcp2517_control;

        /* register mcp2517fd device */
        if (rt_device_register(&mcp2517_can_port[i].dev, mcp2517_can_port[i].dev_name, RT_DEVICE_FLAG_RDWR) == RT_EOK)
        {
            LOG_I("register '%s' success", mcp2517_can_port[i].dev_name);
        }
        else
        {
            LOG_E("register '%s' failed", mcp2517_can_port[i].dev_name);
            return -RT_ERROR;
        }
    }
    ret = mcp2517_spi_thread_init();
    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_mcp2517_init);

#endif /* BSP_USING_MCP2517FD */
