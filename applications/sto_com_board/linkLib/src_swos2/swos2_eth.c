/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_eth.h"

#include <rtthread.h>

#if RT_VER_NUM >= 0x40100
#include <unistd.h>
#else
#include <dfs_posix.h>
#endif /* RT_VER_NUM >= 0x40100 */
#include "netif/ethernetif.h"
#include "netif/ethernet.h"

#define DBG_TAG "if_eth"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#define ETH_RXMB_MAX_MSG             (10)

#define STO_TX_BOARD_ETH_PACK_PRINT 1
struct eth_dump_buf
{
    rt_uint16_t channel;
    rt_uint16_t tot_len;
    rt_tick_t tick;
    void *buf;
};

static struct netif *st_p_netif_ch1 = RT_NULL, *st_p_netif_ch2 = RT_NULL, *st_p_netif_ch3 = RT_NULL;

static netif_input_fn st_fn_ch1_ethif_input, st_fn_ch2_ethif_input, st_fn_ch3_ethif_input;

static netif_output_fn st_fn_ch1_ethif_output, st_fn_ch2_ethif_output, st_fn_ch3_ethif_output;

static struct rt_mailbox *st_ethrx_mb = RT_NULL, *st_ethtx_mb = RT_NULL;

static uint8_t st_a_u8_ethrxbuf[E_ETH_CH_MAX][1600];
static uint8_t st_a_u8_ethtxbuf[E_ETH_CH_MAX][1600];

static uint8_t sw_eth_mac[3][6] = { 0 };

#ifdef  STO_TX_BOARD_ETH_PACK_PRINT
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void hex_dump(const rt_uint8_t *ptr, rt_size_t buflen)
{
    unsigned char *buf = (unsigned char *) ptr;
    int i, j;

    RT_ASSERT(ptr != RT_NULL);

    for (i = 0; i < buflen; i += 16)
    {
        rt_kprintf("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%02X ", buf[i + j]);
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        rt_kprintf("\n");
    }
}
#endif

static void sw_ethrx_thread_entry(void *param)
{
    // struct pbuf *pbuf = RT_NULL;
    struct eth_dump_buf *tbuf = RT_NULL;
    rt_uint32_t mbval;

    while (1)
    {
        if (rt_mb_recv(st_ethrx_mb, (rt_ubase_t *) &mbval, RT_WAITING_FOREVER) == RT_EOK)
        {
            tbuf = (struct eth_dump_buf *) mbval;
            RT_ASSERT(tbuf != RT_NULL);
            rt_kprintf("ETH:%d,tbuf->tot_len = %d\n", tbuf->channel, tbuf->tot_len);
#ifdef STO_TX_BOARD_ETH_PACK_PRINT
            rt_kprintf("tbuf->tot_len = %d\n", tbuf->tot_len);
            hex_dump(tbuf->buf, tbuf->tot_len);
#endif
            //rt_free(tbuf);
            tbuf = RT_NULL;
        }
        else
        {
            rt_kprintf("sw_ethrx_thread exit!\n");
            return;
        }
    }
}

uint8_t is_ip_package(uint8_t *p_buf, uint16_t len)
{
    if (len < 14)
    {
        return 0;
    }
    if ((p_buf[12] * 0x100 + p_buf[13]) >= 1500)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* 截取输入IP协议栈函数 */
static err_t _dup_netifch1_input(struct pbuf *p, struct netif *inp)
{
    // pbuf->payload + sizeof(struct tcpdump_buf)
    //struct eth_dump_buf *tbuf = (struct eth_dump_buf *)rt_malloc(p->tot_len+sizeof(struct eth_dump_buf));
    struct eth_dump_buf *tbuf = (struct eth_dump_buf *) st_a_u8_ethrxbuf[0];

    //RT_ASSERT(tbuf != RT_NULL);
    //RT_ASSERT(netif != RT_NULL);
    if (is_ip_package(p->payload, p->tot_len) == 0)
    {
        tbuf->channel = 1;
        tbuf->tick = rt_tick_get();
        tbuf->buf = ((rt_uint8_t *) tbuf) + sizeof(struct eth_dump_buf);
        tbuf->tot_len = p->tot_len;
        pbuf_copy_partial(p, tbuf->buf, p->tot_len, 0);

        LOG_I("1 recv");
        if (p != RT_NULL)
        {
            if (rt_mb_send(st_ethrx_mb, (rt_uint32_t) tbuf) != RT_EOK)
            {
                //rt_free(tbuf);
            }
        }
    }
    else
    {
        return st_fn_ch1_ethif_input(p, inp);
    }
}
static err_t _dup_netifch2_input(struct pbuf *p, struct netif *inp)
{
    // pbuf->payload + sizeof(struct tcpdump_buf)
    //struct eth_dump_buf *tbuf = (struct eth_dump_buf *)rt_malloc(p->tot_len+sizeof(struct eth_dump_buf));
    struct eth_dump_buf *tbuf = (struct eth_dump_buf *) st_a_u8_ethrxbuf[1];
    //RT_ASSERT(tbuf != RT_NULL);
    //RT_ASSERT(netif != RT_NULL);
    if (is_ip_package(p->payload, p->tot_len) == 0)
    {
        tbuf->channel = 2;
        tbuf->tick = rt_tick_get();
        tbuf->buf = ((rt_uint8_t *) tbuf) + sizeof(struct eth_dump_buf);
        tbuf->tot_len = p->tot_len;
        pbuf_copy_partial(p, tbuf->buf, p->tot_len, 0);

        LOG_I("2 recv");
        if (p != RT_NULL)
        {
            if (rt_mb_send(st_ethrx_mb, (rt_uint32_t) tbuf) != RT_EOK)
            {
                //rt_free(tbuf);
            }
        }
    }
    else
    {
        return st_fn_ch2_ethif_input(p, inp);
    }
}
static err_t _dup_netifch3_input(struct pbuf *p, struct netif *inp)
{
    // pbuf->payload + sizeof(struct tcpdump_buf)
    //struct eth_dump_buf *tbuf = (struct eth_dump_buf *)rt_malloc(p->tot_len+sizeof(struct eth_dump_buf));
    struct eth_dump_buf *tbuf = (struct eth_dump_buf *) st_a_u8_ethrxbuf[2];

    //RT_ASSERT(tbuf != RT_NULL);
    //RT_ASSERT(netif != RT_NULL);
    if (is_ip_package(p->payload, p->tot_len) == 0)
    {
        tbuf->channel = 3;
        tbuf->tick = rt_tick_get();
        tbuf->buf = ((rt_uint8_t *) tbuf) + sizeof(struct eth_dump_buf);
        tbuf->tot_len = p->tot_len;
        pbuf_copy_partial(p, tbuf->buf, p->tot_len, 0);

        if (p != RT_NULL)
        {
            if (rt_mb_send(st_ethrx_mb, (rt_uint32_t) tbuf) != RT_EOK)
            {
                //rt_free(tbuf);
            }
        }
    }
    else
    {
        return st_fn_ch3_ethif_input(p, inp);
    }
}

BOOL if_eth_init(void)
{
    struct eth_device *device;
    rt_base_t level;
    rt_thread_t tid;

    st_ethrx_mb = rt_mb_create("eth_rx_t_mb", ETH_RXMB_MAX_MSG, RT_IPC_FLAG_FIFO);
    if (st_ethrx_mb == RT_NULL)
    {
        //rt_kprintf("eth_rx_t_mb create fail!\n");
        LOG_E("eth_rx_t_mb create fail!");
        return FALSE;
    }

    tid = rt_thread_create("sw_eth_rx", sw_ethrx_thread_entry, RT_NULL, 2048, 12, 10);
    if (tid == RT_NULL)
    {
        rt_mb_delete(st_ethrx_mb);
        st_ethrx_mb = RT_NULL;
        //rt_kprintf("sw_ethrx_thread create fail!\n");
        LOG_E("sw_eth_rx thread create fail!");
        return FALSE;
    }

    device = (struct eth_device *) rt_device_find("e");
    if (RT_NULL != device)
    {
        level = rt_hw_interrupt_disable();
        st_p_netif_ch1 = device->netif;
        st_fn_ch1_ethif_input = st_p_netif_ch1->input;
        st_fn_ch1_ethif_output = st_p_netif_ch1->output;
        st_p_netif_ch1->input = _dup_netifch1_input;
        rt_hw_interrupt_enable(level);
    }
    else
    {
        //rt_kprintf("can not find i0 if!\n");
        LOG_E("can not find e if!");
        return FALSE;
    }

    device = (struct eth_device *) rt_device_find("e0");
    if (RT_NULL != device)
    {
        level = rt_hw_interrupt_disable();
        st_p_netif_ch2 = device->netif;
        st_fn_ch2_ethif_input = st_p_netif_ch2->input;
        st_fn_ch2_ethif_output = st_p_netif_ch2->output;
        st_p_netif_ch2->input = _dup_netifch2_input;
        rt_hw_interrupt_enable(level);
    }
    else
    {
        //rt_kprintf("can not find e0 if!\n");
        LOG_E("can not find e0 if!");
        return FALSE;
    }

//    device = (struct eth_device *) rt_device_find("e1");
//    if (RT_NULL != device)
//    {
//        level = rt_hw_interrupt_disable();
//        st_p_netif_ch3 = device->netif;
//        st_fn_ch3_ethif_input = st_p_netif_ch3->input;
//        st_fn_ch3_ethif_output = st_p_netif_ch3->output;
//        st_p_netif_ch3->input = _dup_netifch3_input;
//        rt_hw_interrupt_enable(level);
//    }
//    else
//    {
//        //rt_kprintf("can not find e1 if!\n");
//        LOG_E("can not find e1 if!");
//        return FALSE;
//    }
    sw_eth_mac[0][0] = 0x4C;
    sw_eth_mac[0][1] = 0x53;
    sw_eth_mac[0][2] = 0x57;
    sw_eth_mac[0][3] = 0x00;
    sw_eth_mac[0][4] = 0x90;
    sw_eth_mac[0][5] = 0x20;

    sw_eth_mac[1][0] = 0x4C;
    sw_eth_mac[1][1] = 0x53;
    sw_eth_mac[1][2] = 0x57;
    sw_eth_mac[1][3] = 0x00;
    sw_eth_mac[1][4] = 0x02;
    sw_eth_mac[1][5] = 0x20;

    sw_eth_mac[2][0] = 0x4C;
    sw_eth_mac[2][1] = 0x53;
    sw_eth_mac[2][2] = 0x57;
    sw_eth_mac[2][3] = 0x00;
    sw_eth_mac[2][4] = 0x80;
    sw_eth_mac[2][5] = 0x20;
    rt_thread_startup(tid);
    //rt_kprintf("if_eth_init start!\n");
    LOG_I("if_eth_init start success!");

    return TRUE;
}

//static int rt_sw_eth_if_init(void)
//{
//    if (if_eth_init() == TRUE)
//    {
//        return 0;
//    }
//    else
//    {
//        return 1;
//    }
//}

//INIT_APP_EXPORT(rt_sw_eth_if_init);

BOOL if_eth_send(E_ETH_CH ch, uint8 *pdata, uint16 len)
{
    err_t err_ret;
    struct pbuf* p = pbuf_alloc(PBUF_LINK, len, PBUF_RAM);
    ;
    uint8_t dst_mac[6];
    switch (ch)
    {
    case E_ETH_CH_1:
        if (st_p_netif_ch1 != RT_NULL)
        {
//            dst_mac[0] = 0x4C;
//            dst_mac[1] = 0x53;
//            dst_mac[2] = 0x57;
//            dst_mac[3] = 0x00;
//            dst_mac[4] = 0x02;
//            dst_mac[5] = 0x0A;


            sw_eth_mac[0][0] = 0xF8;
            sw_eth_mac[0][1] = 0x09;
            sw_eth_mac[0][2] = 0xA4;
            sw_eth_mac[0][3] = 0x51;
            sw_eth_mac[0][4] = 0x0e;
            sw_eth_mac[0][5] = 0x00;

            dst_mac[0] = 0xF8;
            dst_mac[1] = 0x09;
            dst_mac[2] = 0xA4;
            dst_mac[3] = 0x27;
            dst_mac[4] = 0x00;
            dst_mac[5] = 0x44;

            pbuf_take(p, pdata, len);
            err_ret = ethernet_output(st_p_netif_ch1, p, (struct eth_addr *) sw_eth_mac[0],
                                                         (struct eth_addr *) dst_mac, len);
        }
        break;
    case E_ETH_CH_2:
        if (st_p_netif_ch2 != RT_NULL)
        {
//            dst_mac[0] = 0x4C;
//            dst_mac[1] = 0x53;
//            dst_mac[2] = 0x57;
//            dst_mac[3] = 0x00;
//            dst_mac[4] = 0x02;
//            dst_mac[5] = 0x0A;

            dst_mac[0] = 0xF8;
            dst_mac[1] = 0x09;
            dst_mac[2] = 0xA4;
            dst_mac[3] = 0x51;
            dst_mac[4] = 0x0e;
            dst_mac[5] = 0x00;


            sw_eth_mac[1][0] = 0xF8;
            sw_eth_mac[1][1] = 0x09;
            sw_eth_mac[1][2] = 0xA4;
            sw_eth_mac[1][3] = 0x27;
            sw_eth_mac[1][4] = 0x00;
            sw_eth_mac[1][5] = 0x44;

            pbuf_take(p, pdata, len);
            err_ret = ethernet_output(st_p_netif_ch2, p, (struct eth_addr *) sw_eth_mac[1],
                                                         (struct eth_addr *) dst_mac, len);
        }
        break;
    case E_ETH_CH_3:
        if (st_p_netif_ch3 != RT_NULL)
        {
            dst_mac[0] = 0x4C;
            dst_mac[1] = 0x53;
            dst_mac[2] = 0x57;
            dst_mac[3] = 0x00;
            dst_mac[4] = 0x02;
            dst_mac[5] = 0x0A;
            pbuf_take(p, pdata, len);
            err_ret = ethernet_output(st_p_netif_ch3, p, (struct eth_addr *) sw_eth_mac[2],
                                                         (struct eth_addr *) dst_mac, len);
        }
        break;
    default:
        break;
    }
    if (p)
    {
        pbuf_free(p);
    }
    return TRUE;
}

uint16 if_eth_get(E_ETH_CH ch, uint8 *pdata, uint16 len)
{
    return TRUE;
}
