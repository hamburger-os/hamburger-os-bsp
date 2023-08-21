/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-18     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_KSZ8851
#include <drv_config.h>
#include "drv_fmc_eth.h"
#include <netif/ethernetif.h>
#include "ksz8851.h"

#include <string.h>

#define DBG_TAG "drv.eth"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

enum
{
#ifdef BSP_USE_ETH1
    ETH1_INDEX,
#endif

#ifdef BSP_USE_ETH2
    ETH2_INDEX,
#endif

#ifdef BSP_USE_ETH3
    ETH3_INDEX,
#endif
};

static struct rt_fmc_eth_port fmc_eth_port[] = {
#ifdef BSP_USE_ETH1
    {
        .dev_name = "e0",
        .hw_addr_cmd = (volatile void *)ETH1_CMD,
        .hw_addr = (volatile void *)(ETH1_CMD - 2),
        .NE = ETH1_NE,
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
#ifdef BSP_USE_ETH1_LINK_LAYER
        .link_layer_enable = 1,
#else
        .link_layer_enable = 0,
#endif
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
    },
#endif
#ifdef BSP_USE_ETH2
    {
        .dev_name = "e1",
        .hw_addr_cmd = (volatile void *)ETH2_CMD,
        .hw_addr = (volatile void *)(ETH2_CMD - 2),
        .NE = ETH2_NE,
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
#ifdef BSP_USE_ETH2_LINK_LAYER
        .link_layer_enable = 1,
#else
        .link_layer_enable = 0,
#endif
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
    },
#endif
#ifdef BSP_USE_ETH3
    {
        .dev_name = "e2",
        .hw_addr_cmd = (volatile void *)ETH3_CMD,
        .hw_addr = (volatile void *)(ETH3_CMD - 2),
        .NE = ETH3_NE,
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
#ifdef BSP_USE_ETH3_LINK_LAYER
        .link_layer_enable = 1,
#else
        .link_layer_enable = 0,
#endif
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
    },
#endif
};

static struct rt_fmc_eth fmc_eth_device = {
    .port = fmc_eth_port,
};

/* FMC initialization function */
static void MX_FMC_Init(uint32_t NSBank)
{
    SRAM_HandleTypeDef hsram = {0};
    FMC_NORSRAM_TimingTypeDef Timing = {0};

#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) || defined (STM32F417xx) || \
    defined (STM32F427xx) || defined (STM32F437xx) || defined (STM32F429xx) || defined (STM32F439xx) || \
    defined (STM32F401xC) || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) || \
    defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) || defined (STM32F469xx) || \
    defined (STM32F479xx) || defined (STM32F412Cx) || defined (STM32F412Rx) || defined (STM32F412Vx) || \
    defined (STM32F412Zx) || defined (STM32F413xx) || defined (STM32F423xx)

    /** Perform the SRAM memory initialization sequence
     */
    hsram.Instance = FMC_NORSRAM_DEVICE;
    hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram.Init */
    hsram.Init.NSBank = NSBank;
    hsram.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WrapMode = FMC_WRAP_MODE_DISABLE;
    hsram.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = BSP_USING_KSZ8851_ADDRESSSETUPTIME;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = BSP_USING_KSZ8851_DATASETUPTIME;
    Timing.BusTurnAroundDuration = BSP_USING_KSZ8851_BUSTURNAROUNDDURATION;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram, &Timing, NULL) != HAL_OK)
    {
        Error_Handler();
    }
#endif

#if defined (STM32H743xx) || defined (STM32H753xx)  || defined (STM32H750xx) || defined (STM32H742xx) || \
    defined (STM32H745xx) || defined (STM32H745xG)  || defined (STM32H755xx)  || defined (STM32H747xx) || defined (STM32H747xG) || defined (STM32H757xx) || \
    defined (STM32H7A3xx) || defined (STM32H7A3xxQ) || defined (STM32H7B3xx) || defined (STM32H7B3xxQ) || defined (STM32H7B0xx)  || defined (STM32H7B0xxQ) || \
    defined (STM32H735xx) || defined (STM32H733xx)  || defined (STM32H730xx) || defined (STM32H730xxQ)  || defined (STM32H725xx) || defined (STM32H723xx)

    /** Perform the SRAM memory initialization sequence
    */
    hsram.Instance = FMC_NORSRAM_DEVICE;
    hsram.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram.Init */
    hsram.Init.NSBank = NSBank;
    hsram.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
    hsram.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
    hsram.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
    hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
    hsram.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
    hsram.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
    hsram.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
    hsram.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
    hsram.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
    hsram.Init.PageSize = FMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = BSP_USING_KSZ8851_ADDRESSSETUPTIME;
    Timing.AddressHoldTime = 15;
    Timing.DataSetupTime = BSP_USING_KSZ8851_DATASETUPTIME;
    Timing.BusTurnAroundDuration = BSP_USING_KSZ8851_BUSTURNAROUNDDURATION;
    Timing.CLKDivision = 16;
    Timing.DataLatency = 17;
    Timing.AccessMode = FMC_ACCESS_MODE_A;
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram, &Timing, NULL) != HAL_OK)
    {
        Error_Handler();
    }
#endif
}

static void fmc_eth_hard_reset(void)
{
    for (int i = 0; i < sizeof(fmc_eth_port) / sizeof(struct rt_fmc_eth_port); i++)
    {
        rt_pin_mode(fmc_eth_device.port[i].rst_pin, PIN_MODE_OUTPUT);
        rt_pin_write(fmc_eth_device.port[i].rst_pin, PIN_LOW);
    }
    rt_thread_mdelay(100);
    for (int i = 0; i < sizeof(fmc_eth_port) / sizeof(struct rt_fmc_eth_port); i++)
    {
        rt_pin_write(fmc_eth_device.port[i].rst_pin, PIN_HIGH);
    }
    rt_thread_mdelay(100);
}

static void fmc_eth_irq_callback(void *args)
{
    struct rt_fmc_eth_port *fmc_eth = args;
    eth_device_ready(&(fmc_eth->parent));
}

/* EMAC initialization function */
static rt_err_t fmc_eth_init(rt_device_t dev)
{
    rt_err_t res = RT_EOK;
    struct rt_fmc_eth_port *fmc_eth = dev->user_data;

    /* 初始化 eth 互斥 */
    res = rt_mutex_init(&fmc_eth->eth_mux, fmc_eth->dev_name, RT_IPC_FLAG_PRIO);
    if (res != RT_EOK)
    {
        LOG_E("mutex init error %d!", res);
        return -RT_ERROR;
    }

    MX_FMC_Init(fmc_eth->NE);

    res = ks_init(fmc_eth);
    if (res != RT_EOK)
    {
        LOG_E("ks init error %d!", res);
        return -RT_ERROR;
    }

    rt_pin_mode(fmc_eth->isr_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(fmc_eth->isr_pin, PIN_IRQ_MODE_FALLING, fmc_eth_irq_callback, fmc_eth);
    rt_pin_irq_enable(fmc_eth->isr_pin, PIN_IRQ_ENABLE);

    return RT_EOK;
}

static rt_err_t fmc_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    struct rt_fmc_eth_port *fmc_eth = dev->user_data;

    if(NULL == fmc_eth)
    {
        return -RT_EEMPTY;
    }

    if(fmc_eth->link_layer_enable)
    {
        return lep_eth_if_clear(&fmc_eth->link_layer_buf);
    }
    else
    {
        return RT_EOK;
    }
#else
    return RT_EOK;
#endif
}

static rt_err_t fmc_eth_close(rt_device_t dev)
{
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    struct rt_fmc_eth_port *fmc_eth = dev->user_data;

    if(NULL == fmc_eth)
    {
        return -RT_EEMPTY;
    }

    if(fmc_eth->link_layer_enable)
    {
        return lep_eth_if_clear(&fmc_eth->link_layer_buf);
    }
    else
    {
        return RT_EOK;
    }
#else
    return RT_EOK;
#endif
}

#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
static rt_size_t fmc_eth_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct rt_fmc_eth_port *fmc_eth = dev->user_data;
    rt_uint16_t read_size = 0;
    S_LEP_BUF *p_s_LepBuf = RT_NULL;
    rt_list_t *list_pos = NULL;
    rt_list_t *list_next = NULL;

    /* step1：遍历链表 */
    rt_list_for_each_safe(list_pos, list_next, &fmc_eth->link_layer_buf.rx_head->list)
    {
        p_s_LepBuf = rt_list_entry(list_pos, struct tagLEP_BUF, list);
        if (p_s_LepBuf != RT_NULL)
        {
            if ((p_s_LepBuf->flag & LEP_RBF_RV) != 0U)
            {
                /* step2：获取接收包数据长度 */
                if(p_s_LepBuf->len > LEP_MAC_PKT_MAX_LEN)
                {
                    read_size = LEP_MAC_PKT_MAX_LEN;
                }
                else
                {
                    read_size = p_s_LepBuf->len;
                    if (read_size > size)
                    {
                        read_size = size;
                    }
                }

                /* step3：提取包数据 */
                rt_memcpy(buffer, p_s_LepBuf->buf, read_size);
                rt_list_remove(list_pos);
                /* step4：释放接收接收缓冲区 */
                rt_free(p_s_LepBuf);
                return read_size;
            }
        }
    }
    return read_size;
}

static rt_size_t fmc_eth_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct rt_fmc_eth_port *fmc_eth = dev->user_data;
    rt_uint16_t tx_size = 0;

    memset(&fmc_eth->link_layer_buf.tx_buf, 0, sizeof(S_LEP_BUF));
    if(size > LEP_MAC_PKT_MAX_LEN)
    {
        tx_size = LEP_MAC_PKT_MAX_LEN;
    }
    else
    {
        tx_size = size;
    }
    rt_memcpy(fmc_eth->link_layer_buf.tx_buf.buf, buffer, tx_size);
    fmc_eth->link_layer_buf.tx_buf.len = tx_size;

    if (ks_start_xmit_link_layer(fmc_eth, &fmc_eth->link_layer_buf.tx_buf) != 0)
    {
        LOG_D("link layer write error");
        return 0;
    }

    return tx_size;
}

#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */

static rt_err_t fmc_eth_control(rt_device_t dev, int cmd, void *args)
{
    struct rt_fmc_eth_port *fmc_eth = dev->user_data;
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args)
        {
            SMEMCPY(args, fmc_eth->dev_addr, 6);
        }
        else
        {
            return -RT_ERROR;
        }
        break;

    default:
        break;
    }

    return RT_EOK;
}

/* ethernet device interface */
/* transmit data*/
rt_err_t fmc_eth_tx(rt_device_t dev, struct pbuf *p)
{
    rt_err_t ret = RT_EOK;
    struct rt_fmc_eth_port *fmc_eth = dev->user_data;

    rt_mutex_take(&fmc_eth->eth_mux, RT_WAITING_FOREVER);

    if (ks_start_xmit(fmc_eth, p) == 0)
    {
        ret = -RT_ERROR;
    }

#ifdef BSP_USING_KSZ8851_TX_DUMP
    if (p != NULL)
    {
        LOG_D("%s tx start >>>>>>>>>>>>>>>>", fmc_eth->dev_name);
        struct pbuf *q = NULL;
        for (q = p; q != NULL; q = q->next)
        {
            LOG_HEX("tx", 16, (uint8_t *)((uint8_t *)q->payload), q->len);
            LOG_D("%s tx pbuf_len %d >>>>>>>>>>>>>>>>", fmc_eth->dev_name, q->len);
        }
        LOG_D("%s tx tot_len %d >>>>>>>>>>>>>>>>", fmc_eth->dev_name, p->tot_len);
    }
#endif
    rt_mutex_release(&fmc_eth->eth_mux);

    return ret;
}

/* receive data*/
struct pbuf *fmc_eth_rx(rt_device_t dev)
{
    struct pbuf *p = NULL;
    struct rt_fmc_eth_port *fmc_eth = dev->user_data;

    rt_mutex_take(&fmc_eth->eth_mux, RT_WAITING_FOREVER);

    p = ks_irq(fmc_eth);
#ifdef BSP_USE_LINK_LAYER_COMMUNICATION
    if(p != NULL)
    {
        S_LEP_BUF *ps_lep_buf = RT_NULL;

        ps_lep_buf = rt_malloc(sizeof(S_LEP_BUF));
        if(RT_NULL != ps_lep_buf)
        {
            ps_lep_buf->flag = 0;
            ps_lep_buf->flag |= LEP_RBF_RV;
            ps_lep_buf->len = p->len;
            rt_memcpy(ps_lep_buf->buf, p->payload, p->len);
            rt_list_insert_before(&fmc_eth->link_layer_buf.rx_head->list, &ps_lep_buf->list);
            if(dev->rx_indicate != NULL)
            {
                dev->rx_indicate(dev, p->len);
            }
        }
        else
        {
            LOG_E("ps_lep_buf rx null");
        }
    }
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */

#ifdef BSP_USING_KSZ8851_RX_DUMP
    if (p != NULL)
    {
        LOG_D("%s rx start >>>>>>>>>>>>>>>>", fmc_eth->dev_name);
        struct pbuf *q = NULL;
        for (q = p; q != NULL; q = q->next)
        {
            LOG_HEX("rx", 16, (uint8_t *)((uint8_t *)q->payload), q->len);
            LOG_D("%s rx pbuf_len %d >>>>>>>>>>>>>>>>", fmc_eth->dev_name, q->len);
        }
        LOG_D("%s rx tot_len %d >>>>>>>>>>>>>>>>", fmc_eth->dev_name, p->tot_len);
    }
#endif
    rt_mutex_release(&fmc_eth->eth_mux);

    return p;
}

static void fmc_eth_get_config(void)
{
#ifdef BSP_USE_ETH1
    fmc_eth_port[ETH1_INDEX].rst_pin = rt_pin_get(ETH1_RST);
    fmc_eth_port[ETH1_INDEX].isr_pin = rt_pin_get(ETH1_ISR);
#endif

#ifdef BSP_USE_ETH2
    fmc_eth_port[ETH2_INDEX].rst_pin = rt_pin_get(ETH2_RST);
    fmc_eth_port[ETH2_INDEX].isr_pin = rt_pin_get(ETH2_ISR);
#endif

#ifdef BSP_USE_ETH3
    fmc_eth_port[ETH3_INDEX].rst_pin = rt_pin_get(ETH3_RST);
    fmc_eth_port[ETH3_INDEX].isr_pin = rt_pin_get(ETH3_ISR);
#endif
}

/* Register the EMAC device */
static int rt_fmc_eth_init(void)
{
    rt_err_t state = RT_EOK;
    fmc_eth_get_config();
    fmc_eth_hard_reset();

    for (int i = 0; i < sizeof(fmc_eth_port) / sizeof(struct rt_fmc_eth_port); i++)
    {
        /* OUI 00-80-E1 STMICROELECTRONICS.前三个字节为厂商ID */
        fmc_eth_device.port[i].dev_addr[0] = 0xfc;
        fmc_eth_device.port[i].dev_addr[1] = 0x3f;
        fmc_eth_device.port[i].dev_addr[2] = 0xab;
        /* generate MAC addr from 96bit unique ID (only for test). */
        fmc_eth_device.port[i].dev_addr[3] = *(uint8_t *)(UID_BASE + 2 + i);
        fmc_eth_device.port[i].dev_addr[4] = *(uint8_t *)(UID_BASE + 1 + i);
        fmc_eth_device.port[i].dev_addr[5] = *(uint8_t *)(UID_BASE + 0 + i);

        fmc_eth_device.port[i].parent.parent.init = fmc_eth_init;
        fmc_eth_device.port[i].parent.parent.open = fmc_eth_open;
        fmc_eth_device.port[i].parent.parent.close = fmc_eth_close;
        fmc_eth_device.port[i].parent.parent.control = fmc_eth_control;
        fmc_eth_device.port[i].parent.parent.user_data = &fmc_eth_device.port[i];

        fmc_eth_device.port[i].parent.eth_rx = fmc_eth_rx;
        fmc_eth_device.port[i].parent.eth_tx = fmc_eth_tx;

#ifdef BSP_USE_LINK_LAYER_COMMUNICATION

        fmc_eth_device.port[i].parent.parent.read = fmc_eth_read;
        fmc_eth_device.port[i].parent.parent.write = fmc_eth_write;

        if(fmc_eth_device.port[i].link_layer_enable)
        {
            state = lep_eth_if_init(&fmc_eth_device.port[i].link_layer_buf);
            if(state != RT_EOK)
            {
                LOG_E("device %s init linklayer faild: %d", fmc_eth_device.port[i].dev_name, state);
                return state;
            }
        }
#endif /* BSP_USE_LINK_LAYER_COMMUNICATION */
        /* register eth device */
        state = eth_device_init(&(fmc_eth_device.port[i].parent), fmc_eth_device.port[i].dev_name);
        if (RT_EOK == state)
        {
            LOG_I("device %s init success", fmc_eth_device.port[i].dev_name);
        }
        else
        {
            LOG_E("device %s init faild: %d", fmc_eth_device.port[i].dev_name, state);
            state = -RT_ERROR;
        }
    }

    return state;
}
INIT_DEVICE_EXPORT(rt_fmc_eth_init);

#include <netdev.h>       /* 当需要网卡操作是，需要包含这两个头文件 */

static int netdev_set_default_test(int argc, char **argv)
{
    struct netdev *netdev = RT_NULL;

    if (argc != 2)
    {
        LOG_E("netdev_set_default [netdev_name]   --set default network interface device.");
        return -1;
    }

    /* 通过网卡名称获取网卡对象，名称可以通过 ifconfig 命令查看 */
    netdev = netdev_get_by_name(argv[1]);
    if (netdev == RT_NULL)
    {
        LOG_E("not find network interface device name(%s).", argv[1]);
        return -1;
    }

    /* 设置默认网卡对象 */
    netdev_set_default(netdev);

    LOG_I("set default network interface device(%s) success.", argv[1]);
    return 0;
}
#ifdef FINSH_USING_MSH
#include <finsh.h>
/* 导出命令到 FinSH 控制台 */
MSH_CMD_EXPORT_ALIAS(netdev_set_default_test, netdev_set_default, set default network interface device);
#endif /* FINSH_USING_MSH */

#endif
