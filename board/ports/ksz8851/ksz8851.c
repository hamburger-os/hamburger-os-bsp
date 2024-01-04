/* include file */
#include "board.h"

#include <netif/ethernetif.h>

#include "ksz8851.h"

#define DBG_TAG "ks8851"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define U16_WRITE(__ADDRESS__, __DATA__)   do{                                                             \
                                               (*(__IO uint16_t *)((uint32_t)(__ADDRESS__)) = (__DATA__)); \
                                               __DSB();                                                    \
                                             } while(0)

static void sleepms(uint32_t utick)
{
    rt_thread_mdelay(utick);
}
/**
 * 读一个字节
 * @param addr: 读出地址指针
 */
inline static uint8_t readb(volatile uint8_t *addr)
{
    uint8_t ram;           //防止被优化
    ram = *addr;
    return ram;
}

/**
 * 读一个16位字
 * @param addr: 读出地址指针
 */
inline static uint16_t readw(volatile uint16_t *addr)
{
    uint16_t ram;           //防止被优化
    ram = *addr;
    return ram;
}

/**
 * 写一个16位字
 * @param v :写入值
 * @param addr: 写入地址指针
 */
inline static void writew(uint16_t data, volatile uint16_t *addr)
{
    U16_WRITE(addr, data);
}

/**
 * ks_rdreg8 - 读8位寄存器值
 * @ps_ks	  : 芯片信息指针
 * @offset: 寄存器地址
 *
 */
static uint8_t ks_rdreg8(struct rt_fmc_eth_port *ps_ks, uint16_t offset_u16)
{
    uint16_t data_u16;
    uint8_t shift_bit_u8;
    uint8_t shift_data_u8;

    shift_bit_u8 = offset_u16 & 0x03;
    shift_data_u8 = (offset_u16 & 1) << 3;
    ps_ks->cmd_reg_cache = (uint16_t)offset_u16 | (uint16_t)(BE0 << shift_bit_u8);
    writew(ps_ks->cmd_reg_cache, ps_ks->hw_addr_cmd);
    data_u16 = readw(ps_ks->hw_addr);
    return (uint8_t)(data_u16 >> shift_data_u8);
}

/**
 * ks_rdreg16 - 读16位寄存器值
 * @ps_ks	  : 芯片信息结构指针
 * @offset_u16: 寄存器地址
 *
 */
static uint16_t ks_rdreg16(struct rt_fmc_eth_port *ps_ks, uint16_t offset_u16)
{
    ps_ks->cmd_reg_cache = (uint16_t)offset_u16 | ((BE1 | BE0) << (offset_u16 & 0x02));
    writew(ps_ks->cmd_reg_cache, ps_ks->hw_addr_cmd);
    return readw(ps_ks->hw_addr);
}

/**
 * ks_wrreg8 - 写8位值到寄存器
 * @ps_ks: 芯片信息结构指针
 * @offset_u16: 寄存器地址
 * @value_u8: 待写入值
 *
 */
static void ks_wrreg8(struct rt_fmc_eth_port *ps_ks, uint16_t offset_u16, uint8_t value_u8)
{
    uint8_t shift_bit_u8;
    uint16_t value_write_u16;

    shift_bit_u8 = (offset_u16 & 0x03);
    value_write_u16 = (uint16_t)(value_u8 << ((offset_u16 & 1) << 3));
    ps_ks->cmd_reg_cache = (uint16_t)offset_u16 | (BE0 << shift_bit_u8);
    writew(ps_ks->cmd_reg_cache, ps_ks->hw_addr_cmd);
    writew(value_write_u16, ps_ks->hw_addr);
}

/**
 * ks_wrreg16 - 写16位值到寄存器
 * @ps_ks: 芯片信息结构指针
 * @offset_u16: 寄存器地址
 * @value_u16: 待写入的值
 *
 */

static void ks_wrreg16(struct rt_fmc_eth_port *ps_ks, uint16_t offset_u16, uint16_t value_u16)
{
    ps_ks->cmd_reg_cache = (uint16_t)offset_u16 | ((BE1 | BE0) << (offset_u16 & 0x02));
    writew(ps_ks->cmd_reg_cache, ps_ks->hw_addr_cmd);
    writew(value_u16, ps_ks->hw_addr);
}

/**
 * ks_inblk - 从QMU读一个数据块. 此函数只能在设置进入DMA操作状态后调用。
 * @ps_ks: 芯片信息结构指针
 * @wptr_u16: 存储读数据的缓冲区指针
 * @len_u32: 读的字节数
 *
 */
static void ks_inblk(struct rt_fmc_eth_port *ps_ks, uint16_t *p_wptr_u16, uint32_t len_u32)
{
    len_u32 >>= 1;
    while (len_u32--)
    {
        *p_wptr_u16++ = (uint16_t)readw(ps_ks->hw_addr);
    }
}

/**
 * ks_outblk - 写数据到QMU。此函数只能在设置进入DMA操作状态后调用。
 * @ps_ks: 芯片信息结构指针。
 * @p_wptr_u16: 待写数据缓冲区指针。
 * @len_u32: 待写数据字节数。
 *
 */
static void ks_outblk(struct rt_fmc_eth_port *ps_ks, uint16_t *p_wptr_u16, uint32_t len_u32)
{
    len_u32 >>= 1;
    while (len_u32--)
    {
        writew(*p_wptr_u16++, ps_ks->hw_addr);
    }
}

/**
 * ks_disable_int - 禁止ksz8851中断。
 * @ps_ks : 芯片信息结构指针。
 */
static void ks_disable_int(struct rt_fmc_eth_port *ps_ks)
{
    ks_wrreg16(ps_ks, KSZ8851_IER, 0x0000);
} /* ks_disable_int */

/**
 * ks_enable_int - 使能ksz8851中断。
 * @ps_ks : 芯片信息结构指针。
 */
static void ks_enable_int(struct rt_fmc_eth_port *ps_ks)
{
    ks_wrreg16(ps_ks, KSZ8851_IER, ps_ks->rc_ier);
} /* ks_enable_int */

/**
 * ks_tx_fifo_space - 返回可用的硬件缓冲区大小。.
 * @ps_ks: 芯片信息结构指针。
 *
 */
static uint16_t ks_tx_fifo_space(struct rt_fmc_eth_port *ps_ks)
{
    return ks_rdreg16(ps_ks, KSZ8851_TXMIR) & TXMIR_SIZE_MASK;
}

/**
 * ks_save_cmd_reg - 保存命令寄存器值.
 * @ps_ks: 芯片信息结构指针。
 *
 */
static void ks_save_cmd_reg(struct rt_fmc_eth_port *ps_ks)
{
    /*ks8851 MLL has a bug to read back the command register.
     * So rely on software to save the content of command register.
     */
    ps_ks->cmd_reg_cache_int = ps_ks->cmd_reg_cache;
}

/**
 * ks_restore_cmd_reg - 恢复保存的命令寄存器值。
 * @ps_ks: 芯片信息结构指针。
 *
 */
static void ks_restore_cmd_reg(struct rt_fmc_eth_port *ps_ks)
{
    ps_ks->cmd_reg_cache = ps_ks->cmd_reg_cache_int;
    writew(ps_ks->cmd_reg_cache, ps_ks->hw_addr_cmd);
}

/**
 * ks_set_powermode - 设置电源模式
 * @ps_ks: 芯片信息结构指针
 * @pwrmode_u16: 写入KSZ8851_PMECR的模式值.
 *
 * Change the power mode of the chip.
 */
static void ks_set_powermode(struct rt_fmc_eth_port *ps_ks, uint16_t pwrmode_u16)
{
    uint16_t pmecr_u16;

    ks_rdreg16(ps_ks, KSZ8851_GRR);
    pmecr_u16 = ks_rdreg16(ps_ks, KSZ8851_PMECR);
    pmecr_u16 &= ~PMECR_PM_MASK;
    pmecr_u16 |= pwrmode_u16;

    ks_wrreg16(ps_ks, KSZ8851_PMECR, pmecr_u16);
}

/**
 * ks_read_config - 读芯片总线配置信息。
 * @ps_ks: 芯片信息结构指针。
 **/
static void ks_read_config(struct rt_fmc_eth_port *ps_ks)
{
    uint16_t reg_data_u16 = 0;

    /** 8位访问总应能工作。*/
    reg_data_u16 = ks_rdreg8(ps_ks, KSZ8851_CCR) & 0x00FF;
    reg_data_u16 |= ks_rdreg8(ps_ks, KSZ8851_CCR + 1) << 8;

    /** 地址/数据总线复用情况 */
    if ((reg_data_u16 & CCR_SHARED) == CCR_SHARED)
    {
        ps_ks->sharedbus_u16 = 1;
    }
    else
    {
        ps_ks->sharedbus_u16 = 0;
    }

    /** 根据总线宽度选取额外数据的字节数。   */
    if (reg_data_u16 & CCR_8BIT)
    {
        ps_ks->bus_width_u16 = ENUM_BUS_8BIT;
        ps_ks->extra_byte_u8 = 1;
    }
    else if (reg_data_u16 & CCR_16BIT)
    {
        ps_ks->bus_width_u16 = ENUM_BUS_16BIT;
        ps_ks->extra_byte_u8 = 2;
    }
    else
    {
        ps_ks->bus_width_u16 = ENUM_BUS_32BIT;
        ps_ks->extra_byte_u8 = 4;
    }
}

/**
 * ks_soft_reset - 软复位。
 * @ps_ks: 芯片信息结构指针。
 * @op_u16: 全局复位控制寄存器的位设置信息。
 *
 * 注意复位延时的选择。
 */
static void ks_soft_reset(struct rt_fmc_eth_port *ps_ks, uint16_t op_u16)
{
    /* 关中断 */
    ks_wrreg16(ps_ks, KSZ8851_IER, 0x0000);
    ks_wrreg16(ps_ks, KSZ8851_GRR, op_u16);
    /* 复位时间 */
    sleepms(10);
    ks_wrreg16(ps_ks, KSZ8851_GRR, 0);
    /* 清除复位延时 */
    sleepms(1);
}

/**
 * ks_enable_qmu - 使能QMU。
 * @ps_ks: 芯片信息结构指针。
 */
static void ks_enable_qmu(struct rt_fmc_eth_port *ps_ks)
{
    uint16_t w_u16;

    w_u16 = ks_rdreg16(ps_ks, KSZ8851_TXCR);
    /** 使能QMU传输(TXCR). */
    ks_wrreg16(ps_ks, KSZ8851_TXCR, w_u16 | TXCR_TXE);

    /** 接收帧阈值和自动出队列使能    */
    w_u16 = ks_rdreg16(ps_ks, KSZ8851_RXQCR);
    ks_wrreg16(ps_ks, KSZ8851_RXQCR, w_u16 | RXQCR_RXFCTE);

    /** 使能QMU接收(RXCR1). */
    w_u16 = ks_rdreg16(ps_ks, KSZ8851_RXCR1);
    ks_wrreg16(ps_ks, KSZ8851_RXCR1, w_u16 | RXCR1_RXE);
    ps_ks->b_enabled = 1;
}

/**
 * ks_disable_qmu - 禁能QMU。
 * @ps_ks: 芯片信息结构指针。
 */
static void ks_disable_qmu(struct rt_fmc_eth_port *ps_ks)
{
    uint16_t w_u16;

    w_u16 = ks_rdreg16(ps_ks, KSZ8851_TXCR);

    /** 禁能QMU发送(TXCR). */
    w_u16 &= ~TXCR_TXE;
    ks_wrreg16(ps_ks, KSZ8851_TXCR, w_u16);

    /* 禁能QMU接收(RXCR1). */
    w_u16 = ks_rdreg16(ps_ks, KSZ8851_RXCR1);
    w_u16 &= ~RXCR1_RXE;
    ks_wrreg16(ps_ks, KSZ8851_RXCR1, w_u16);

    ps_ks->b_enabled = 0;
}

/*** ks_start_rx - 启动接收功能
 * @ps_ks		: 芯片信息结构指针。*/
static void ks_start_rx(struct rt_fmc_eth_port *ps_ks)
{
    uint16_t cntl_u16;
    /** 使能QMU接收(RXCR1). */
    cntl_u16 = ks_rdreg16(ps_ks, KSZ8851_RXCR1);
    cntl_u16 |= RXCR1_RXE;
    ks_wrreg16(ps_ks, KSZ8851_RXCR1, cntl_u16);
}

/*** ks_stop_rx - 停止接收数据包
 * @ps_ks		: 芯片信息结构指针。*/
static void ks_stop_rx(struct rt_fmc_eth_port *ps_ks)
{
    uint16_t cntl_u16;
    /** 停止QMU接收(RXCR1). */
    cntl_u16 = ks_rdreg16(ps_ks, KSZ8851_RXCR1);
    cntl_u16 &= ~RXCR1_RXE;
    ks_wrreg16(ps_ks, KSZ8851_RXCR1, cntl_u16);
}

/**
 * ks_read_qmu - 从QMU读一包数据.
 * @ps_ks: 芯片信息结构指针。
 * @p_buf_u16: 存储包数据的目的缓冲区指针。
 * @len_u32: 包长度
 * 读数据包的操作序列:
 *	1. 进入DMA方式
 *	2. 读前导数据
 *	3. 读包数据
 *	4. 退出DMA方式
 */
static void ks_read_qmu(struct rt_fmc_eth_port *ps_ks, uint16_t *p_buf_u16, uint32_t len_u32)
{
    uint32_t r_u32;
    uint32_t w_u32;

    r_u32 = ps_ks->extra_byte_u8 & 0x1;
    w_u32 = ps_ks->extra_byte_u8 - r_u32;
    /* 1. 设置DMA方式 */
    ks_wrreg16(ps_ks, KSZ8851_RXFDPR, RXFDPR_RXFPAI);
    ks_wrreg8(ps_ks, KSZ8851_RXQCR, (ps_ks->rc_rxqcr_u16 | RXQCR_SDA) & 0xff);

    /* 2. 读前导数据 */
    /**
     * read 4 + extra bytes and discard them.
     * extra bytes for dummy, 2 for status, 2 for len
     */
    if (r_u32 != 0)
    {
        readb(ps_ks->hw_addr);
    }
    ks_inblk(ps_ks, p_buf_u16, w_u32 + 2 + 2);
    /* 3. read pkt data */
    ks_inblk(ps_ks, p_buf_u16, (len_u32 + (4 - 1)) & (~(4 - 1)));
    /* 4. reset sudo DMA Mode */
    ks_wrreg8(ps_ks, KSZ8851_RXQCR, ps_ks->rc_rxqcr_u16);

    LOG_D("%s %d", __FUNCTION__, len_u32);
}

/**
 * ks_rcv - 从QMU接收多包数据.
 * @ps_ks: 芯片信息结构指针。
 * @netdev: The network device being opened.
 * Read all of header information before reading pkt content.
 * It is not allowed only port of pkts in QMU after issuing
 * interrupt ack.
 */
static uint8_t rx_buff[ETH_RXBUF_SIZE];
static struct pbuf *ks_rcv(struct rt_fmc_eth_port *ps_ks)
{
    struct pbuf *p = NULL;
    struct pbuf *q = NULL;
    uint16_t len = 0;

    FRAME_HEAD *ps_frame_hdr = ps_ks->frame_head;
    ps_ks->frm_cnt_u8 = ks_rdreg16(ps_ks, KSZ8851_RXFCTR) >> 8;
    if (ps_ks->frm_cnt_u8 > KSZ_MAX_RFRM_THD)
    {
        LOG_E("rx frame cnt error %d", ps_ks->frm_cnt_u8);
        ps_ks->frm_cnt_u8 = KSZ_MAX_RFRM_THD;
    }

    /* read all header information */
    for (uint16_t i = 0; i < ps_ks->frm_cnt_u8; i++)
    {
        /* Checking Received packet status */
        ps_frame_hdr->sts_u16 = ks_rdreg16(ps_ks, KSZ8851_RXFHSR);
        /* Get packet len from hardware */
        ps_frame_hdr->len_u16 = ks_rdreg16(ps_ks, KSZ8851_RXFHBCR);
        if ((ps_frame_hdr->sts_u16 & RXFSHR_RXFV) &&
            (ps_frame_hdr->len_u16 < ETH_RXBUF_SIZE) && (ps_frame_hdr->len_u16 >= 4))
        {
            len += ps_frame_hdr->len_u16;
        }
        ps_frame_hdr++;
    }

    /* Obtain the size of the packet and put it into the "len" variable. */
    if (len > 0)
    {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p == NULL)
        {
            LOG_E("rx pbuf_alloc error.");
        }
    }

    if (p != NULL)
    {
        q = p;
        uint32_t payloadoffset = 0;
        uint32_t payloadlen = 0;
        uint32_t framelength = 0;
        ps_frame_hdr = ps_ks->frame_head;

        while (ps_ks->frm_cnt_u8--)
        {
            if ((ps_frame_hdr->sts_u16 & RXFSHR_RXFV) &&
                (ps_frame_hdr->len_u16 < ETH_RXBUF_SIZE) && (ps_frame_hdr->len_u16 >= 4))
            {
                payloadlen = q->len;
                if (payloadoffset + ps_frame_hdr->len_u16 <= payloadlen)
                {
                    /* read data block including CRC 4 bytes */
                    ks_read_qmu(ps_ks, (uint16_t *)((uint8_t *)q->payload + payloadoffset), ps_frame_hdr->len_u16);
                    payloadoffset += ps_frame_hdr->len_u16;
                    framelength += ps_frame_hdr->len_u16;
                }
                else
                {
                    /* read data block including CRC 4 bytes */
                    uint32_t payloadmore = payloadoffset + ps_frame_hdr->len_u16 - payloadlen;
                    ks_read_qmu(ps_ks, (uint16_t *)rx_buff, ps_frame_hdr->len_u16);
                    SMEMCPY((uint16_t *)((uint8_t *)q->payload + payloadoffset), rx_buff, ps_frame_hdr->len_u16 - payloadmore);
                    framelength += ps_frame_hdr->len_u16 - payloadmore;
                    q = q->next;
                    SMEMCPY((uint16_t *)((uint8_t *)q->payload), rx_buff + ps_frame_hdr->len_u16 - payloadmore, payloadmore);
                    framelength += payloadmore;
                    payloadoffset = payloadmore;
                }
                if (q == NULL)
                {
                    LOG_E("pbuf next is NULL %d", ps_ks->frm_cnt_u8);
                }
            }
            else
            {
                ks_wrreg16(ps_ks, KSZ8851_RXQCR, (ps_ks->rc_rxqcr_u16 | RXQCR_RRXEF));
                LOG_E("rx invalid data %d %d", ps_ks->frm_cnt_u8, ps_frame_hdr->len_u16);
            }

            ps_frame_hdr++;
        }

        if (p->tot_len != framelength)
        {
            LOG_E("rx frame length :%d %d %d", p->tot_len, len, framelength);
        }
    }

    return p;
}

/**
 * ks_update_link_status - 更新链接状态.
 * @ps_ks: 芯片信息结构指针。
 *
 */
static void ks_update_link_status(struct rt_fmc_eth_port *ps_ks)
{
    /* check the status of the link */
    rt_bool_t link_up_status = 0;

    if (ks_rdreg16(ps_ks, KSZ8851_P1SR) & P1SR_LINK_GOOD)
    {
        link_up_status = 1;
        LOG_I("device %s link up", ps_ks->dev_name);
    }
    else
    {
        link_up_status = 0;
        LOG_W("device %s link down", ps_ks->dev_name);
    }
    /* send link up. */
    eth_device_linkchange(&ps_ks->parent, link_up_status);
}

/**
 * ks_clean_rx_queue - 清除QME接收缓冲区.
 * @ps_ks: 芯片信息结构指针。
 *
 */
#define KSZ8851_WAIT_STOP (100)
static void ks_clean_rx_queue(struct rt_fmc_eth_port *ps_ks)
{
    uint16_t int_status_u16;
    uint16_t reg_cnt_u16;
    int i;

    ks_stop_rx(ps_ks);
    for (i = 0; i < KSZ8851_WAIT_STOP; i++)
    {
        int_status_u16 = ks_rdreg16(ps_ks, KSZ8851_ISR);
        if (int_status_u16 & IRQ_RXPSI)
            break;
    }

    reg_cnt_u16 = ks_rdreg16(ps_ks, KSZ8851_RXCR1);
    ks_wrreg16(ps_ks, KSZ8851_RXCR1, reg_cnt_u16 | RXCR1_FRXQ);
    ks_wrreg16(ps_ks, KSZ8851_RXCR1, reg_cnt_u16);

    ks_wrreg16(ps_ks, KSZ8851_ISR, IRQ_RXI);
    if (ps_ks->b_enabled)
    {
        ks_start_rx(ps_ks);
    }
    ks_wrreg16(ps_ks, KSZ8851_ISR, IRQ_RXPSI);
}

/**
 * ks_irq - 中断处理函数
 * @ps_ks: 芯片信息结构指针。
 *
 */
struct pbuf *ks_irq(struct rt_fmc_eth_port *ps_ks)
{
    struct pbuf *p = NULL;
    uint16_t status_u16 = 0;

    /** 首先保存当前操作的寄存器地址 */
    ks_save_cmd_reg(ps_ks);

    status_u16 = ks_rdreg16(ps_ks, KSZ8851_ISR);
    if (status_u16 == 0)
    {
        /* this should be the last in IRQ handler*/
        ks_restore_cmd_reg(ps_ks);
        return NULL;
    }

    ks_wrreg16(ps_ks, KSZ8851_ISR, status_u16); /** 清中断 */

    if (status_u16 & IRQ_RXI)
    {
        p = ks_rcv(ps_ks);
    }

    if (status_u16 & IRQ_LCI)
    {
        ks_update_link_status(ps_ks);
    }

    if (status_u16 & IRQ_LDI)
    {
        uint16_t pmecr_u16 = ks_rdreg16(ps_ks, KSZ8851_PMECR);
        pmecr_u16 &= ~PMECR_WKEVT_MASK;
        ks_wrreg16(ps_ks, KSZ8851_PMECR, pmecr_u16 | PMECR_WKEVT_LINK);
        LOG_D("build link irq");
    }

    if (status_u16 & (IRQ_RXOI | IRQ_RXPSI))
    {
        ks_clean_rx_queue(ps_ks);
        LOG_D("rx overload or stop irq");
    }

    /* this should be the last in IRQ handler*/
    ks_restore_cmd_reg(ps_ks);
    return p;
}

/**
 * ks_net_open - 打开网络设备
 * @ps_ks: 芯片信息结构指针。
 *
 */
static int32_t ks_net_open(struct rt_fmc_eth_port *ps_ks)
{
    /* wake up powermode to normal mode */
    ks_set_powermode(ps_ks, PMECR_PM_NORMAL);
    /* wait for normal mode to take effect */
    sleepms(1);
    /* 初始化数据缓冲区 */
    ks_wrreg16(ps_ks, KSZ8851_ISR, 0xffff); /** 清除全部中断 */
    ks_enable_int(ps_ks);
    ks_enable_qmu(ps_ks);
    return 0;
}

/**
 * ks_net_stop - 关闭网络设备。
 * @ps_ks: 芯片信息结构指针。
 *
 */
static int32_t ks_net_stop(struct rt_fmc_eth_port *ps_ks)
{
    /* 禁止全部中断并清除中断标志位 */
    ks_wrreg16(ps_ks, KSZ8851_IER, 0x0000);
    ks_wrreg16(ps_ks, KSZ8851_ISR, 0xffff);
    /* 关闭QMU接收和发送 */
    ks_disable_qmu(ps_ks);
    ks_disable_int(ps_ks);
    /* set powermode to soft power down to save power */
    ks_set_powermode(ps_ks, PMECR_PM_SOFTDOWN);
    return 0;
}

/**
 * ks_write_qmu - 发送一包数据到QMU.
 * @ps_ks: 芯片信息结构指针。
 * @pdata: 待发送包数据缓冲区指针
 * @len_u16: 包长度
 * Here is the sequence to write 1 pkt:
 *	1. 设置进入DMA模式
 *	2. 写包的状态和长度。
 *	3. 写包数据内容
 *	4. reset sudo DMA Mode
 *	5. reset sudo DMA mode
 *	6. 等待包发送完成。
 */
static void ks_write_qmu(struct rt_fmc_eth_port *ps_ks, uint8_t *pdata, uint16_t len_u16)
{
    /* start header at txb[0] to align txw entries */
    ps_ks->txw[0] = 0;
    ps_ks->txw[1] = len_u16;

    /* 1. set sudo-DMA mode */
    ks_wrreg8(ps_ks, KSZ8851_RXQCR, (ps_ks->rc_rxqcr_u16 | RXQCR_SDA) & 0xff);
    /* 2. write status/lenth info */
    ks_outblk(ps_ks, ps_ks->txw, 4);
    /* 3. write pkt data */
    ks_outblk(ps_ks, (uint16_t *)pdata, (len_u16 + (4 - 1)) & (~(4 - 1)));
    /* 4. reset sudo-DMA mode */
    ks_wrreg8(ps_ks, KSZ8851_RXQCR, ps_ks->rc_rxqcr_u16);
    /* 5. Enqueue Tx(move the pkt from TX buffer into TXQ) */
    ks_wrreg16(ps_ks, KSZ8851_TXQCR, TXQCR_METFE);
    /* 6. wait until TXQCR_METFE is auto-cleared */
    while (ks_rdreg16(ps_ks, KSZ8851_TXQCR) & TXQCR_METFE)
        ;

    LOG_D("%s %d", __FUNCTION__, len_u16);
}

/**
 * ks_start_xmit - transmit packet
 * @skb     : The buffer to transmit
 * @netdev  : The device used to transmit the packet.
 *
 * Called by the network layer to transmit the @skb.
 * spin_lock_irqsave is required because tx and rx should be mutual exclusive.
 * So while tx is in-progress, prevent IRQ interrupt from happenning.
 */
static uint8_t tx_buff[ETH_TXBUF_SIZE];
int ks_start_xmit(struct rt_fmc_eth_port *ps_ks, struct pbuf *p)
{
    struct pbuf *q = NULL;
    int payloadoffset = 0;
    int framelength = 0;

    if (p != NULL)
    {
        /* Extra space are required:
         *  4 byte for alignment, 4 for status/length, 4 for CRC
         */
        while(ks_tx_fifo_space(ps_ks) < p->tot_len + 12)
        {
            rt_thread_delay(1);
        }
        /* copy frame from pbufs to driver buffers */
        for (q = p; q != NULL; q = q->next)
        {
            if (payloadoffset + q->len < ETH_TXBUF_SIZE)
            {
                SMEMCPY(tx_buff + payloadoffset, q->payload, q->len);
                framelength += q->len;
                payloadoffset += q->len;

            }
            else
            {
                SMEMCPY(tx_buff + payloadoffset, q->payload, ETH_TXBUF_SIZE - payloadoffset);
                framelength += ETH_TXBUF_SIZE - payloadoffset;
                payloadoffset = 0;
                /* Copy data to Tx buffer*/
                ks_write_qmu(ps_ks, (uint8_t *)((uint8_t *)tx_buff), ETH_TXBUF_SIZE);
                SMEMCPY(tx_buff + payloadoffset, (uint8_t *)q->payload + ETH_TXBUF_SIZE - payloadoffset, q->len - ETH_TXBUF_SIZE + payloadoffset);
                framelength += q->len - ETH_TXBUF_SIZE + payloadoffset;
                payloadoffset += q->len - ETH_TXBUF_SIZE + payloadoffset;
            }
        }
        if (payloadoffset > 0)
        {
            /* Copy data to Tx buffer*/
            ks_write_qmu(ps_ks, (uint8_t *)((uint8_t *)tx_buff), payloadoffset);
        }
        if (p->tot_len != framelength)
        {
            LOG_E("tx frame length :%d %d", p->tot_len, framelength);
        }
    }
    else
    {
        LOG_E("struct pbuf p == NULL");
    }
    return framelength;
}

#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
/**
 * ks_start_xmit_link_layer - 链路层发送数据包
 * @skb     : The buffer to transmit
 * @netdev  : The device used to transmit the packet.
 *
 * Called by the network layer to transmit the @skb.
 * spin_lock_irqsave is required because tx and rx should be mutual exclusive.
 * So while tx is in-progress, prevent IRQ interrupt from happenning.
 */
int32_t ks_start_xmit_link_layer(struct rt_fmc_eth_port *ps_ks, S_LEP_BUF *ps_lep_buf)
{
    int32_t retv_i32 = 0;

    while(ks_tx_fifo_space(ps_ks) < ps_lep_buf->len + 12)
    {
        rt_thread_delay(1);
    }

    /* Extra space are required:
     *  4 byte for alignment, 4 for status/length, 4 for CRC
     */
    ks_write_qmu(ps_ks, ps_lep_buf->buf, ps_lep_buf->len);

    return retv_i32;
}
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */

static unsigned long const ethernet_polynomial = 0x04c11db7U;
static unsigned long ether_gen_crc(int length, uint8_t *data)
{
    long crc = -1;
    while (--length >= 0)
    {
        uint8_t current_octet_u8 = *data++;
        int bit;

        for (bit = 0; bit < 8; bit++, current_octet_u8 >>= 1)
        {
            crc = (crc << 1) ^
                  ((crc < 0) ^ (current_octet_u8 & 1) ? ethernet_polynomial : 0);
        }
    }
    return (unsigned long)crc;
}

/** ks_set_grpaddr - 设置多播地址
 * @ps_ks : 芯片信息结构指针。*/
static void ks_set_grpaddr(struct rt_fmc_eth_port *ps_ks)
{
    uint8_t i;
    uint32_t index_u32, position_u32, value_u32;

    rt_memset(ps_ks->mcast_bits, 0, sizeof(uint8_t) * HW_MCAST_SIZE);

    for (i = 0; i < ps_ks->mcast_lst_size_u16; i++)
    {
        position_u32 = (ether_gen_crc(6, ps_ks->mcast_lst[i]) >> 26) & 0x3f;
        index_u32 = position_u32 >> 3;
        value_u32 = 1 << (position_u32 & 7);
        ps_ks->mcast_bits[index_u32] |= (uint8_t)value_u32;
    }

    for (i = 0; i < HW_MCAST_SIZE; i++)
    {
        if (i & 1)
        {
            ks_wrreg16(ps_ks, (uint16_t)((KSZ8851_MAHTR0 + i) & ~1),
                       (ps_ks->mcast_bits[i] << 8) |
                           ps_ks->mcast_bits[i - 1]);
        }
    }
}

/**ks_clear_mcast - 清除组播信息
 * @ps_ks : 芯片信息结构指针。
 * 清除芯片组播寄存器设置值*/
static void ks_clear_mcast(struct rt_fmc_eth_port *ps_ks)
{
    uint16_t i, mcast_size_u16;

    for (i = 0; i < HW_MCAST_SIZE; i++)
    {
        ps_ks->mcast_bits[i] = 0;
    }

    mcast_size_u16 = HW_MCAST_SIZE >> 2;
    for (i = 0; i < mcast_size_u16; i++)
    {
        ks_wrreg16(ps_ks, KSZ8851_MAHTR0 + (2 * i), 0);
    }
}

/**
 * ks_set_promis - 设置接收模式－进入或退出混杂模式。
 * @ps_ks : 芯片信息结构指针。
 * @promiscuous_mode_u16 : 混杂模式
 */
static void ks_set_promis(struct rt_fmc_eth_port *ps_ks, uint16_t promiscuous_mode_u16)
{
    uint16_t cntl_u16;

    ps_ks->promiscuous_u16 = promiscuous_mode_u16;
    ks_stop_rx(ps_ks); /** 配置前先停止接收 */
    cntl_u16 = ks_rdreg16(ps_ks, KSZ8851_RXCR1);

    cntl_u16 &= ~RXCR1_FILTER_MASK;
    if (promiscuous_mode_u16)
    {
        /** 使能混杂模式 */
        cntl_u16 |= RXCR1_RXAE | RXCR1_RXINVF;
    }
    else
    {
        /** 禁止混杂模式 */
        cntl_u16 |= RXCR1_RXPAFMA;
    }

    ks_wrreg16(ps_ks, KSZ8851_RXCR1, cntl_u16);

    if (ps_ks->b_enabled)
    {
        ks_start_rx(ps_ks);
    }
}

/**
 * ks_set_mcast- 油墨组播。
 * @ps_ks : 芯片信息结构指针。
 * @mcast_u16 : 组播使能标志
 */
static void ks_set_mcast(struct rt_fmc_eth_port *ps_ks, uint16_t mcast_u16)
{
    uint16_t cntl_u16;

    ps_ks->all_mcast_u16 = mcast_u16;
    ks_stop_rx(ps_ks); /** 配置前先停止接收 */
    cntl_u16 = ks_rdreg16(ps_ks, KSZ8851_RXCR1);
    cntl_u16 &= ~RXCR1_FILTER_MASK;
    if (mcast_u16)
    {
        /* Enable "Perfect with Multicast address passed mode" */
        /* 使能完美组播地址过滤模式 */
        cntl_u16 |= (RXCR1_RXAE | RXCR1_RXMAFMA | RXCR1_RXPAFMA);
    }
    else
    {
        /**
         * Disable "Perfect with Multicast address passed
         * mode" (normal mode).
         * 哈希表组合MAC地址过滤模式。
         */
        cntl_u16 |= RXCR1_RXPAFMA;
    }

    ks_wrreg16(ps_ks, KSZ8851_RXCR1, cntl_u16);

    if (ps_ks->b_enabled)
    {
        ks_start_rx(ps_ks);
    }
}

/**
 * ks_set_mac - 设置芯片MAC地址
 * @ps_ks : 芯片信息结构指针。
 * @p_data : MAC地址数据缓冲区指针。
 */
void ks_set_mac(struct rt_fmc_eth_port *ps_ks, uint8_t *p_data)
{
    uint16_t *pw_u16;
    uint16_t w_u16, u_u16;
    uint8_t i;

    pw_u16 = (uint16_t *)p_data;

    ks_stop_rx(ps_ks); /* Stop receiving for reconfiguration */

    u_u16 = *pw_u16++;
    w_u16 = ((u_u16 & 0xFF) << 8) | ((u_u16 >> 8) & 0xFF);
    ks_wrreg16(ps_ks, KSZ8851_MARH, w_u16);

    u_u16 = *pw_u16++;
    w_u16 = ((u_u16 & 0xFF) << 8) | ((u_u16 >> 8) & 0xFF);
    ks_wrreg16(ps_ks, KSZ8851_MARM, w_u16);

    u_u16 = *pw_u16;
    w_u16 = ((u_u16 & 0xFF) << 8) | ((u_u16 >> 8) & 0xFF);
    ks_wrreg16(ps_ks, KSZ8851_MARL, w_u16);

    for (i = 0; i < 6; i++)
    {
        ps_ks->mac[i] = p_data[i];
    }

    if (ps_ks->b_enabled)
    {
        ks_start_rx(ps_ks);
    }
}

/**
 * ks_read_selftest - 读自测试内存信息.
 * @ps_ks: 芯片信息结构指针。
 *
 */
static int32_t ks_read_selftest(struct rt_fmc_eth_port *ps_ks)
{
    uint32_t both_done_u32 = MBIR_TXMBF | MBIR_RXMBF;
    int32_t ret_i32 = 0;
    uint32_t rd_u32;

    rd_u32 = ks_rdreg16(ps_ks, KSZ8851_MBIR);

    if ((rd_u32 & both_done_u32) == both_done_u32)
    {

        if (rd_u32 & MBIR_TXMBFA)
        {
            ret_i32 |= 1;
        }

        if (rd_u32 & MBIR_RXMBFA)
        {
            ret_i32 |= 2;
        }
    }
    return ret_i32;
}

/**
 * ks_setup - 配置芯片.
 * @ps_ks: 芯片信息结构指针。
 *
 */
static void ks_setup(struct rt_fmc_eth_port *ps_ks)
{
    uint16_t w_u16;

    /**
     * Configure QMU Transmit
     */
    /* Setup Transmit Frame Data Pointer Auto-Increment (TXFDPR) */
    ks_wrreg16(ps_ks, KSZ8851_TXFDPR, TXFDPR_TXFPAI);

    /* Setup Receive Frame Data Pointer Auto-Increment */
    ks_wrreg16(ps_ks, KSZ8851_RXFDPR, RXFDPR_RXFPAI);

    /* Setup Receive Frame Threshold - 1 frame (RXFCTFC) */
    ks_wrreg16(ps_ks, KSZ8851_RXFCTR, 1 & RXFCTR_THRESHOLD_MASK);

    /* Setup RxQ Command Control (RXQCR) */
    ps_ks->rc_rxqcr_u16 = RXQCR_CMD_CNTL;
    ks_wrreg16(ps_ks, KSZ8851_RXQCR, ps_ks->rc_rxqcr_u16);

    /**
     * set the force mode to half duplex, default is full duplex
     *  because if the auto-negotiation fails, most switch uses
     *  half-duplex.
     */

    w_u16 = ks_rdreg16(ps_ks, KSZ8851_P1MBCR);
    w_u16 &= ~P1MBCR_FORCE_FDX;
    ks_wrreg16(ps_ks, KSZ8851_P1MBCR, w_u16);

    w_u16 = TXCR_TXFCE | TXCR_TXPE | TXCR_TXCRC | TXCR_TCGIP;
    ks_wrreg16(ps_ks, KSZ8851_TXCR, w_u16);

    w_u16 = RXCR1_RXFCE | RXCR1_RXBE | RXCR1_RXUE | RXCR1_RXME | RXCR1_RXIPFCC;

    if (ps_ks->promiscuous_u16)         /* bPromiscuous 混杂模式，就是全部接收模式 */
        w_u16 |= (RXCR1_RXAE | RXCR1_RXINVF);
    else if (ps_ks->all_mcast_u16)      /* Multicast address passed mode */
        w_u16 |= (RXCR1_RXAE | RXCR1_RXMAFMA | RXCR1_RXPAFMA);
    else                                /* Normal mode MAC地址过滤接收模式*/
        w_u16 |= RXCR1_RXPAFMA;

    ks_wrreg16(ps_ks, KSZ8851_RXCR1, w_u16);
} /*ks_setup */

/**
 * ks_setup_int - 配置中断.
 * @ps_ks: 芯片信息结构指针。
 *
 */
static void ks_setup_int(struct rt_fmc_eth_port *ps_ks)
{
    ps_ks->rc_ier = 0x00;
    /* Clear the interrupts status of the hardware. */
    ks_wrreg16(ps_ks, KSZ8851_ISR, 0xffff);

    /* Enables the interrupts of the hardware. */
    ps_ks->rc_ier = (IRQ_LCI | IRQ_RXI | IRQ_RXOI | IRQ_RXPSI);
}

/**
 * ks_hw_init - 硬件初始化.
 * @ps_ks: 芯片信息结构指针。
 *
 */
static int32_t ks_hw_init(struct rt_fmc_eth_port *ps_ks)
{
    ps_ks->promiscuous_u16 = 0;
    ps_ks->all_mcast_u16 = 0;
    ps_ks->mcast_lst_size_u16 = 0;

    ks_set_mac(ps_ks, ps_ks->mac);
    return 0;
}

static int ks_readid(struct rt_fmc_eth_port *ps_ks)
{
    /* simple check for a valid chip being connected to the bus */
    uint32_t CID = ks_rdreg16(ps_ks, KSZ8851_CIDER);
    CID = CID & ~CIDER_REV_MASK;
    if (CID != CIDER_ID)
    {
        LOG_E("CID: failed %X != %X", CID, CIDER_ID);
        return -1;
    }
    else
    {
        LOG_D("CID: %04X succeed.", CID);
    }

    return 0;
}

static int32_t ks_probe(struct rt_fmc_eth_port *ps_ks)
{
    int32_t err_i32 = 0;
    uint16_t data_u16;
    ks_read_config(ps_ks);

    if (ks_readid(ps_ks) < 0)
    {
        err_i32 = -1;
    }
    else if (ks_read_selftest(ps_ks))
    {
        err_i32 = -2;
        LOG_E("probe selftest error");
    }
    else
    {
        ks_soft_reset(ps_ks, GRR_GSR);
        ks_hw_init(ps_ks);
        ks_disable_qmu(ps_ks);
        ks_setup(ps_ks);     /* ksz8851网络通信功能配置 */
        ks_setup_int(ps_ks); /* ksz8851中断配置 */

        data_u16 = ks_rdreg16(ps_ks, KSZ8851_OBCR);
        ks_wrreg16(ps_ks, KSZ8851_OBCR, data_u16 | OBCR_ODS_16MA);
#if 1
        ks_set_promis(ps_ks, 1);
#else
        ks_set_mcast(ps_ks, 0);
#endif
    }

    return err_i32;
}

int ks_init(struct rt_fmc_eth_port *ps_ks)
{
    /* 网口芯片初始化 0(成功)、-1、-2 */
    if (ks_probe(ps_ks) == 0)
    {
        ks_net_open(ps_ks); // 打开网络设备
        LOG_D("probe succeed.");
    }
    else
    {
        LOG_E("probe failed!");
        return -RT_ERROR;
    }
    return RT_EOK;
}
