/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-26     lvhan       the first version
 */
#include "board.h"

#define DBG_TAG "sc16is752"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "drv_spi.h"
#include "drv_soft_spi.h"

/* SC16IS752 dirver class */
struct SC16IS752Def
{
    const char *spi_bus;
    uint32_t spi_speed;
    const char *cs;
    const char *irq;
    const char *rst;

    struct rt_spi_device *spidev;
    rt_base_t cs_pin;
    rt_base_t irq_pin;
    rt_base_t rst_pin;

    const char *uart_name[2];
    struct rt_serial_device serial[2];
};

static struct SC16IS752Def sc16is752_config= {
    .spi_bus = SC16IS752_SPI_BUS,
    .spi_speed = SC16IS752_SPI_SPEED,
    .cs = SC16IS752_SPI_CS_PIN,
    .irq = SC16IS752_IRQ_PIN,
    .rst = SC16IS752_RST_PIN,

    .uart_name = {"scuart1", "scuart2"},
};

/* SC16IS7XX register definitions */
#define SC16IS7XX_RHR_REG       (0x00) /* RX FIFO */
#define SC16IS7XX_THR_REG       (0x00) /* TX FIFO */
#define SC16IS7XX_IER_REG       (0x01) /* Interrupt enable */
#define SC16IS7XX_IIR_REG       (0x02) /* Interrupt Identification */
#define SC16IS7XX_FCR_REG       (0x02) /* FIFO control */
#define SC16IS7XX_LCR_REG       (0x03) /* Line Control */
#define SC16IS7XX_MCR_REG       (0x04) /* Modem Control */
#define SC16IS7XX_LSR_REG       (0x05) /* Line Status */
#define SC16IS7XX_MSR_REG       (0x06) /* Modem Status */
#define SC16IS7XX_SPR_REG       (0x07) /* Scratch Pad */
#define SC16IS7XX_TXLVL_REG     (0x08) /* TX FIFO level */
#define SC16IS7XX_RXLVL_REG     (0x09) /* RX FIFO level */
#define SC16IS7XX_IODIR_REG     (0x0a) /* I/O Direction
                        * - only on 75x/76x
                        */
#define SC16IS7XX_IOSTATE_REG       (0x0b) /* I/O State
                        * - only on 75x/76x
                        */
#define SC16IS7XX_IOINTENA_REG      (0x0c) /* I/O Interrupt Enable
                        * - only on 75x/76x
                        */
#define SC16IS7XX_IOCONTROL_REG     (0x0e) /* I/O Control
                        * - only on 75x/76x
                        */
#define SC16IS7XX_EFCR_REG      (0x0f) /* Extra Features Control */

/* TCR/TLR Register set: Only if ((MCR[2] == 1) && (EFR[4] == 1)) */
#define SC16IS7XX_TCR_REG       (0x06) /* Transmit control */
#define SC16IS7XX_TLR_REG       (0x07) /* Trigger level */

/* Special Register set: Only if ((LCR[7] == 1) && (LCR != 0xBF)) */
#define SC16IS7XX_DLL_REG       (0x00) /* Divisor Latch Low */
#define SC16IS7XX_DLH_REG       (0x01) /* Divisor Latch High */

/* Enhanced Register set: Only if (LCR == 0xBF) */
#define SC16IS7XX_EFR_REG       (0x02) /* Enhanced Features */
#define SC16IS7XX_XON1_REG      (0x04) /* Xon1 word */
#define SC16IS7XX_XON2_REG      (0x05) /* Xon2 word */
#define SC16IS7XX_XOFF1_REG     (0x06) /* Xoff1 word */
#define SC16IS7XX_XOFF2_REG     (0x07) /* Xoff2 word */

/* IER register bits */
#define SC16IS7XX_IER_RDI_BIT       (1 << 0) /* Enable RX data interrupt */
#define SC16IS7XX_IER_THRI_BIT      (1 << 1) /* Enable TX holding register
                          * interrupt */
#define SC16IS7XX_IER_RLSI_BIT      (1 << 2) /* Enable RX line status
                          * interrupt */
#define SC16IS7XX_IER_MSI_BIT       (1 << 3) /* Enable Modem status
                          * interrupt */

/* IER register bits - write only if (EFR[4] == 1) */
#define SC16IS7XX_IER_SLEEP_BIT     (1 << 4) /* Enable Sleep mode */
#define SC16IS7XX_IER_XOFFI_BIT     (1 << 5) /* Enable Xoff interrupt */
#define SC16IS7XX_IER_RTSI_BIT      (1 << 6) /* Enable nRTS interrupt */
#define SC16IS7XX_IER_CTSI_BIT      (1 << 7) /* Enable nCTS interrupt */

/* FCR register bits */
#define SC16IS7XX_FCR_FIFO_BIT      (1 << 0) /* Enable FIFO */
#define SC16IS7XX_FCR_RXRESET_BIT   (1 << 1) /* Reset RX FIFO */
#define SC16IS7XX_FCR_TXRESET_BIT   (1 << 2) /* Reset TX FIFO */
#define SC16IS7XX_FCR_RXLVLL_BIT    (1 << 6) /* RX Trigger level LSB */
#define SC16IS7XX_FCR_RXLVLH_BIT    (1 << 7) /* RX Trigger level MSB */

/* FCR register bits - write only if (EFR[4] == 1) */
#define SC16IS7XX_FCR_TXLVLL_BIT    (1 << 4) /* TX Trigger level LSB */
#define SC16IS7XX_FCR_TXLVLH_BIT    (1 << 5) /* TX Trigger level MSB */

/* IIR register bits */
#define SC16IS7XX_IIR_NO_INT_BIT    (1 << 0) /* No interrupts pending */
#define SC16IS7XX_IIR_ID_MASK       0x3e     /* Mask for the interrupt ID */
#define SC16IS7XX_IIR_THRI_SRC      0x02     /* TX holding register empty */
#define SC16IS7XX_IIR_RDI_SRC       0x04     /* RX data interrupt */
#define SC16IS7XX_IIR_RLSE_SRC      0x06     /* RX line status error */
#define SC16IS7XX_IIR_RTOI_SRC      0x0c     /* RX time-out interrupt */
#define SC16IS7XX_IIR_MSI_SRC       0x00     /* Modem status interrupt
                          * - only on 75x/76x
                          */
#define SC16IS7XX_IIR_INPIN_SRC     0x30     /* Input pin change of state
                          * - only on 75x/76x
                          */
#define SC16IS7XX_IIR_XOFFI_SRC     0x10     /* Received Xoff */
#define SC16IS7XX_IIR_CTSRTS_SRC    0x20     /* nCTS,nRTS change of state
                          * from active (LOW)
                          * to inactive (HIGH)
                          */
/* LCR register bits */
#define SC16IS7XX_LCR_LENGTH0_BIT   (1 << 0) /* Word length bit 0 */
#define SC16IS7XX_LCR_LENGTH1_BIT   (1 << 1) /* Word length bit 1
                          *
                          * Word length bits table:
                          * 00 -> 5 bit words
                          * 01 -> 6 bit words
                          * 10 -> 7 bit words
                          * 11 -> 8 bit words
                          */
#define SC16IS7XX_LCR_STOPLEN_BIT   (1 << 2) /* STOP length bit
                          *
                          * STOP length bit table:
                          * 0 -> 1 stop bit
                          * 1 -> 1-1.5 stop bits if
                          *      word length is 5,
                          *      2 stop bits otherwise
                          */
#define SC16IS7XX_LCR_PARITY_BIT    (1 << 3) /* Parity bit enable */
#define SC16IS7XX_LCR_EVENPARITY_BIT    (1 << 4) /* Even parity bit enable */
#define SC16IS7XX_LCR_FORCEPARITY_BIT   (1 << 5) /* 9-bit multidrop parity */
#define SC16IS7XX_LCR_TXBREAK_BIT   (1 << 6) /* TX break enable */
#define SC16IS7XX_LCR_DLAB_BIT      (1 << 7) /* Divisor Latch enable */
#define SC16IS7XX_LCR_WORD_LEN_5    (0x00)
#define SC16IS7XX_LCR_WORD_LEN_6    (0x01)
#define SC16IS7XX_LCR_WORD_LEN_7    (0x02)
#define SC16IS7XX_LCR_WORD_LEN_8    (0x03)
#define SC16IS7XX_LCR_CONF_MODE_A   SC16IS7XX_LCR_DLAB_BIT /* Special
                                * reg set */
#define SC16IS7XX_LCR_CONF_MODE_B   0xBF                   /* Enhanced
                                * reg set */

/* MCR register bits */
#define SC16IS7XX_MCR_DTR_BIT       (1 << 0) /* DTR complement
                          * - only on 75x/76x
                          */
#define SC16IS7XX_MCR_RTS_BIT       (1 << 1) /* RTS complement */
#define SC16IS7XX_MCR_TCRTLR_BIT    (1 << 2) /* TCR/TLR register enable */
#define SC16IS7XX_MCR_LOOP_BIT      (1 << 4) /* Enable loopback test mode */
#define SC16IS7XX_MCR_XONANY_BIT    (1 << 5) /* Enable Xon Any
                          * - write enabled
                          * if (EFR[4] == 1)
                          */
#define SC16IS7XX_MCR_IRDA_BIT      (1 << 6) /* Enable IrDA mode
                          * - write enabled
                          * if (EFR[4] == 1)
                          */
#define SC16IS7XX_MCR_CLKSEL_BIT    (1 << 7) /* Divide clock by 4
                          * - write enabled
                          * if (EFR[4] == 1)
                          */

/* LSR register bits */
#define SC16IS7XX_LSR_DR_BIT        (1 << 0) /* Receiver data ready */
#define SC16IS7XX_LSR_OE_BIT        (1 << 1) /* Overrun Error */
#define SC16IS7XX_LSR_PE_BIT        (1 << 2) /* Parity Error */
#define SC16IS7XX_LSR_FE_BIT        (1 << 3) /* Frame Error */
#define SC16IS7XX_LSR_BI_BIT        (1 << 4) /* Break Interrupt */
#define SC16IS7XX_LSR_BRK_ERROR_MASK    0x1E     /* BI, FE, PE, OE bits */
#define SC16IS7XX_LSR_THRE_BIT      (1 << 5) /* TX holding register empty */
#define SC16IS7XX_LSR_TEMT_BIT      (1 << 6) /* Transmitter empty */
#define SC16IS7XX_LSR_FIFOE_BIT     (1 << 7) /* Fifo Error */

/* MSR register bits */
#define SC16IS7XX_MSR_DCTS_BIT      (1 << 0) /* Delta CTS Clear To Send */
#define SC16IS7XX_MSR_DDSR_BIT      (1 << 1) /* Delta DSR Data Set Ready
                          * or (IO4)
                          * - only on 75x/76x
                          */
#define SC16IS7XX_MSR_DRI_BIT       (1 << 2) /* Delta RI Ring Indicator
                          * or (IO7)
                          * - only on 75x/76x
                          */
#define SC16IS7XX_MSR_DCD_BIT       (1 << 3) /* Delta CD Carrier Detect
                          * or (IO6)
                          * - only on 75x/76x
                          */
#define SC16IS7XX_MSR_CTS_BIT       (1 << 4) /* CTS */
#define SC16IS7XX_MSR_DSR_BIT       (1 << 5) /* DSR (IO4)
                          * - only on 75x/76x
                          */
#define SC16IS7XX_MSR_RI_BIT        (1 << 6) /* RI (IO7)
                          * - only on 75x/76x
                          */
#define SC16IS7XX_MSR_CD_BIT        (1 << 7) /* CD (IO6)
                          * - only on 75x/76x
                          */
#define SC16IS7XX_MSR_DELTA_MASK    0x0F     /* Any of the delta bits! */

/*
 * TCR register bits
 * TCR trigger levels are available from 0 to 60 characters with a granularity
 * of four.
 * The programmer must program the TCR such that TCR[3:0] > TCR[7:4]. There is
 * no built-in hardware check to make sure this condition is met. Also, the TCR
 * must be programmed with this condition before auto RTS or software flow
 * control is enabled to avoid spurious operation of the device.
 */
#define SC16IS7XX_TCR_RX_HALT(words)    ((((words) / 4) & 0x0f) << 0)
#define SC16IS7XX_TCR_RX_RESUME(words)  ((((words) / 4) & 0x0f) << 4)

/*
 * TLR register bits
 * If TLR[3:0] or TLR[7:4] are logical 0, the selectable trigger levels via the
 * FIFO Control Register (FCR) are used for the transmit and receive FIFO
 * trigger levels. Trigger levels from 4 characters to 60 characters are
 * available with a granularity of four.
 *
 * When the trigger level setting in TLR is zero, the SC16IS740/750/760 uses the
 * trigger level setting defined in FCR. If TLR has non-zero trigger level value
 * the trigger level defined in FCR is discarded. This applies to both transmit
 * FIFO and receive FIFO trigger level setting.
 *
 * When TLR is used for RX trigger level control, FCR[7:6] should be left at the
 * default state, that is, '00'.
 */
#define SC16IS7XX_TLR_TX_TRIGGER(words) ((((words) / 4) & 0x0f) << 0)
#define SC16IS7XX_TLR_RX_TRIGGER(words) ((((words) / 4) & 0x0f) << 4)

/* IOControl register bits (Only 750/760) */
#define SC16IS7XX_IOCONTROL_LATCH_BIT   (1 << 0) /* Enable input latching */
#define SC16IS7XX_IOCONTROL_MODEM_BIT   (1 << 1) /* Enable GPIO[7:4] as modem pins */
#define SC16IS7XX_IOCONTROL_SRESET_BIT  (1 << 3) /* Software Reset */

/* EFCR register bits */
#define SC16IS7XX_EFCR_9BIT_MODE_BIT    (1 << 0) /* Enable 9-bit or Multidrop
                          * mode (RS485) */
#define SC16IS7XX_EFCR_RXDISABLE_BIT    (1 << 1) /* Disable receiver */
#define SC16IS7XX_EFCR_TXDISABLE_BIT    (1 << 2) /* Disable transmitter */
#define SC16IS7XX_EFCR_AUTO_RS485_BIT   (1 << 4) /* Auto RS485 RTS direction */
#define SC16IS7XX_EFCR_RTS_INVERT_BIT   (1 << 5) /* RTS output inversion */
#define SC16IS7XX_EFCR_IRDA_MODE_BIT    (1 << 7) /* IrDA mode
                          * 0 = rate upto 115.2 kbit/s
                          *   - Only 750/760
                          * 1 = rate upto 1.152 Mbit/s
                          *   - Only 760
                          */

/* EFR register bits */
#define SC16IS7XX_EFR_AUTORTS_BIT   (1 << 6) /* Auto RTS flow ctrl enable */
#define SC16IS7XX_EFR_AUTOCTS_BIT   (1 << 7) /* Auto CTS flow ctrl enable */
#define SC16IS7XX_EFR_XOFF2_DETECT_BIT  (1 << 5) /* Enable Xoff2 detection */
#define SC16IS7XX_EFR_ENABLE_BIT    (1 << 4) /* Enable enhanced functions
                          * and writing to IER[7:4],
                          * FCR[5:4], MCR[7:5]
                          */
#define SC16IS7XX_EFR_SWFLOW3_BIT   (1 << 3) /* SWFLOW bit 3 */
#define SC16IS7XX_EFR_SWFLOW2_BIT   (1 << 2) /* SWFLOW bit 2
                          *
                          * SWFLOW bits 3 & 2 table:
                          * 00 -> no transmitter flow
                          *       control
                          * 01 -> transmitter generates
                          *       XON2 and XOFF2
                          * 10 -> transmitter generates
                          *       XON1 and XOFF1
                          * 11 -> transmitter generates
                          *       XON1, XON2, XOFF1 and
                          *       XOFF2
                          */
#define SC16IS7XX_EFR_SWFLOW1_BIT   (1 << 1) /* SWFLOW bit 2 */
#define SC16IS7XX_EFR_SWFLOW0_BIT   (1 << 0) /* SWFLOW bit 3
                          *
                          * SWFLOW bits 3 & 2 table:
                          * 00 -> no received flow
                          *       control
                          * 01 -> receiver compares
                          *       XON2 and XOFF2
                          * 10 -> receiver compares
                          *       XON1 and XOFF1
                          * 11 -> receiver compares
                          *       XON1, XON2, XOFF1 and
                          *       XOFF2
                          */

static int sc16is752_spi_device_init(void)
{
    rt_err_t ret = RT_EOK;

    rt_uint8_t dev_num = 0;
    char dev_name[RT_NAME_MAX];
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", sc16is752_config.spi_bus, dev_num++);
        if (dev_num == 255)
        {
            return -RT_EIO;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(sc16is752_config.spi_bus, dev_name, sc16is752_config.cs));
    rt_hw_spi_device_attach(sc16is752_config.spi_bus, dev_name, rt_pin_get(sc16is752_config.cs));
    sc16is752_config.spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (sc16is752_config.spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return -RT_EIO;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_LTC186X_SPI_SPEED;
    ret = rt_spi_configure(sc16is752_config.spidev, &cfg);
    if (ret != RT_EOK)
    {
        LOG_E("device %s configure error %d!", dev_name, ret);
        return -RT_EIO;
    }

    return RT_EOK;
}
INIT_PREV_EXPORT(sc16is752_spi_device_init);

static rt_err_t sc16is752_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    return RT_EOK;
}

static rt_err_t sc16is752_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    return RT_EOK;
}

static int sc16is752_putc(struct rt_serial_device *serial, char c)
{
    return 1;
}

static int sc16is752_getc(struct rt_serial_device *serial)
{
    int ch = 0;
    return ch;
}

static rt_size_t sc16is752_transmit(struct rt_serial_device     *serial,
                                       rt_uint8_t           *buf,
                                       rt_size_t             size,
                                       rt_uint32_t           tx_flag)
{
    return size;
}

static const struct rt_uart_ops sc16is752_uart_ops =
{
    .configure = sc16is752_configure,
    .control = sc16is752_control,
    .putc = sc16is752_putc,
    .getc = sc16is752_getc,
    .transmit = sc16is752_transmit
};

static int sc16is752_init(void)
{
    rt_err_t result = RT_EOK;

    sc16is752_config.irq_pin = rt_pin_get(sc16is752_config.irq);
    sc16is752_config.rst_pin = rt_pin_get(sc16is752_config.rst);

    for (uint8_t i = 0; i < sizeof(sc16is752_config.serial) / sizeof(sc16is752_config.serial[0]); i++)
    {
        /* init UART object */
        sc16is752_config.serial[i].ops = &sc16is752_uart_ops;
        /* register UART device */
        result = rt_hw_serial_register(
                &sc16is752_config.serial[i],
                sc16is752_config.uart_name[i],
                RT_DEVICE_FLAG_RDWR,
                NULL);
    }
    if (result == RT_EOK)
    {
        LOG_I("register success");
    }
    else
    {
        LOG_E("register failed");
        return -RT_ERROR;
    }

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(sc16is752_init);
