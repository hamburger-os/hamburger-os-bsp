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

#define DBG_TAG "if_eth"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#include "if_gpio.h"

#define SW_ETH_RX_BUF_MAX_NUM (1500U)
#define SW_ETH_RX_MQ_MAX_NUM  (10U)
#define SW_ETH_LINK_LAYER_MAC_LENTH   (12U)

/* 单字节对齐 */
#pragma pack(1)

typedef struct
{
    uint16 len;
    uint8_t data_u8[SW_ETH_RX_BUF_MAX_NUM];
} S_SW_ETH_BUF;

#pragma pack()

typedef struct
{
    rt_uint16_t channel;
    rt_uint16_t len;
} S_ETH_LEN_INFO;

typedef struct
{
    rt_device_t dev;
    const char *name;
    const char *mq_name;
    S_ETH_LEN_INFO eth_info;
    S_SW_ETH_BUF rx_buf;
    S_SW_ETH_BUF tx_buf;
    rt_mq_t eth_rx_mq;
    rt_err_t (*rx_indicate)(rt_device_t dev, rt_size_t size);
} S_SWOS2_ETH;

typedef struct
{
    E_SLOT_ID id; /* 板子id */
    S_SWOS2_ETH *dev[E_ETH_CH_MAX];
    uint8_t ch_num;
    struct rt_mailbox mailbox;
    uint8_t (*p_src_mac)[6];
    uint8_t (*p_dst_mac)[6];
} S_SWOS2_ETH_DEV;

static char eth_mb_pool[1024];

static S_SWOS2_ETH_DEV swos2_eth_dev;

static S_SWOS2_ETH swos2_eth[] =
{
        {
                .name = "e0",
                .mq_name = "e0 mq",
        },
        {
                .name = "e1",
                .mq_name = "e1 mq",
        },
        {
                .name = "e",
                .mq_name = "e mq",
        },
};

/* 通信1底板I系mac配置 */
static uint8_t com_1_load_1_mac_addr[E_ETH_CH_MAX][6] =
{
    {0x4c, 0x53, 0x57, 0x00, 0x10, 0x30},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x10, 0x40},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x02, 0x20},  /* 原生网口   连接I系主控 */
};

/* 通信1底板I系目的地址mac配置 */
static uint8_t com_1_load_1_dst_mac_addr[E_ETH_CH_MAX][6] =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x02},  /* I系主控 */
};

/* 通信1子板I系mac配置 */
static uint8_t com_1_child_1_mac_addr[E_ETH_CH_MAX][6] =
{
    {0x4c, 0x53, 0x57, 0x00, 0x03, 0x11},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x21},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x21},  /* 原生网口   连接I系主控 */
};

/* 通信1子板I系目的地址mac配置 */
static uint8_t com_1_child_1_dst_mac_addr[E_ETH_CH_MAX][6] =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x02},  /* I系主控 */
};

/* 通信1底板II系mac配置 */
static uint8_t com_1_load_2_mac_addr[E_ETH_CH_MAX][6] =
{
    {0x4c, 0x53, 0x57, 0x00, 0x10, 0x31},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x10, 0x41},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x02, 0x21},  /* 原生网口   连接II系主控 */
};

/* 通信1底板II系目的地址mac配置 */
static uint8_t com_1_load_2_dst_mac_addr[E_ETH_CH_MAX][6] =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x0A},  /* II系主控 */
};

/* 通信1子板II系mac配置 */
static uint8_t com_1_child_2_mac_addr[E_ETH_CH_MAX][6] =
{
    {0x4c, 0x53, 0x57, 0x00, 0x03, 0x11},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x22},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x22},  /* 原生网口   连接II系主控 */
};

/* 通信1子板II系目的地址mac配置 */
static uint8_t com_1_child_2_dst_mac_addr[E_ETH_CH_MAX][6] =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x0A},  /* II系主控 */
};

/* 通信2底板I系mac配置 */
static uint8_t com_2_load_1_mac_addr[E_ETH_CH_MAX][6] =
{
    {0x4c, 0x53, 0x57, 0x00, 0x07, 0xA3},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x23},  /* 原生网口   I系主控 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 未使用 */
};

/* 通信2底板I系目的地址mac配置 */
static uint8_t com_2_load_1_dst_mac_addr[E_ETH_CH_MAX][6] =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x02},  /* I系主控 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 未使用 */
};

/* 通信2子板I系mac配置 */
static uint8_t com_2_child_1_mac_addr[E_ETH_CH_MAX][6] =
{
    {0x4c, 0x53, 0x57, 0x00, 0x09, 0xD1},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x09, 0xC1},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x02, 0x30},  /* 原生网口   连接I系主控 */
};

/* 通信2子板I系目的地址mac配置 */
static uint8_t com_2_child_1_dst_mac_addr[E_ETH_CH_MAX][6] =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x02},  /* I系主控 */
};

/* 通信2底板II系mac配置 */
static uint8_t com_2_load_2_mac_addr[E_ETH_CH_MAX][6] =
{
    {0x4c, 0x53, 0x57, 0x00, 0x17, 0xA4},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x24},  /* 原生网口   连接II系主控 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 未使用 */
};

/* 通信2底板II系目的地址mac配置 */
static uint8_t com_2_load_2_dst_mac_addr[E_ETH_CH_MAX][6] =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x0A},  /* II系主控 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 未使用 */
};

/* 通信2子板II系mac配置 */
static uint8_t com_2_child_2_mac_addr[E_ETH_CH_MAX][6] =
{
    {0x4c, 0x53, 0x57, 0x00, 0x09, 0xD2},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x09, 0xC2},  /* 8851网口 外部 */
    {0x4c, 0x53, 0x57, 0x00, 0x02, 0x31},  /* 原生网口   连接II系主控 */
};

/* 通信2子板II系目的地址mac配置 */
static uint8_t com_2_child_2_dst_mac_addr[E_ETH_CH_MAX][6] =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  /* 连接外部网口 未设置 */
    {0x4c, 0x53, 0x57, 0x00, 0x01, 0x0A},  /* II系主控 */
};

static rt_err_t sw_eth1_rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_err_t ret = RT_EOK;
    S_ETH_LEN_INFO *info = &swos2_eth_dev.dev[E_ETH_CH_1]->eth_info;

    if(size != 0)
    {
        info->channel = E_ETH_CH_1;
        info->len = size;
        rt_mb_send(&swos2_eth_dev.mailbox, (rt_uint32_t)info);
    }
    return ret;
}

static rt_err_t sw_eth2_rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_err_t ret = RT_EOK;
    S_ETH_LEN_INFO *info = &swos2_eth_dev.dev[E_ETH_CH_2]->eth_info;

    if(size != 0)
    {
        info->channel = E_ETH_CH_2;
        info->len = size;
        rt_mb_send(&swos2_eth_dev.mailbox, (rt_uint32_t)info);
    }
    return ret;
}

static rt_err_t sw_eth3_rx_callback(rt_device_t dev, rt_size_t size)
{
    rt_err_t ret = RT_EOK;
    S_ETH_LEN_INFO *info = &swos2_eth_dev.dev[E_ETH_CH_3]->eth_info;

    if(size != 0)
    {
        info->channel = E_ETH_CH_3;
        info->len = size;
        rt_mb_send(&swos2_eth_dev.mailbox, (rt_uint32_t)info);
    }
    return ret;
}

static void sw_eth_set_rx_callback(S_SWOS2_ETH *eth, rt_err_t (*rx_ind)(rt_device_t dev, rt_size_t size))
{
    if(rx_ind != NULL && eth != NULL)
    {
        rt_device_set_rx_indicate(eth->dev, rx_ind);
    }
}

static void sw_ethrx_thread_entry(void *param)
{
    S_ETH_LEN_INFO *info = RT_NULL;
    rt_uint32_t mbval;
    rt_err_t ret;

    static rt_uint32_t nocnt = 0U;
    while (1)
    {
        ret = rt_mb_recv(&swos2_eth_dev.mailbox, &mbval, RT_WAITING_FOREVER);
        if (RT_EOK == ret)
        {
            info = (S_ETH_LEN_INFO *) mbval;
            RT_ASSERT(info != RT_NULL);

            if(rt_device_read(swos2_eth_dev.dev[info->channel]->dev, 0, (void *)swos2_eth_dev.dev[info->channel]->rx_buf.data_u8, info->len) == info->len)
            {
                //MY_Printf("rt_device_read len:%d !!!\r\n", info->len);
                if( 800 < info->len)
                {
                    //MY_Printf("rt_device_read len:%d !!!\r\n", info->len);

#if 0

                MY_Printf("ETHdata_no :%d\r\n",nocnt);
                nocnt++;
                MY_Printf("rt_device_read len:%d !!!\r\n", info->len);
                MY_Printf("ETHT data:\r\n");
                for(uint16 i=0; i<info->len; i++)
                {
                    if( ( i > 27U ) && ( i <= 31U ) )
                    {
                        MY_Printf("%d ",swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[i]);
                    }
                    else
                    {
                        MY_Printf("%.2x ",swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[i]);
                    }

                    if ((13U == i) || (27U == i) || (31U == i )||(39U == i )||(46U == i ))
                    {
                        MY_Printf("\r\n");
                    }
                    else if( i > 46U )
                    {
                        if( 0U == (i+1+46-2) % 13 )
                            MY_Printf("\r\n");
                    }
                }
                MY_Printf("\r\n");

#endif
                }
                /* 目的地址与广播地址判断 */
                if((swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[0] == swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[6] && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[1] == swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[7] && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[2] == swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[8] && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[3] == swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[9] && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[4] == swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[10] && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[5] == swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[11]) || \
                        (swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[0] == 0xff && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[1] == 0xff && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[2] == 0xff && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[3] == 0xff && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[4] == 0xff && \
                        swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[5] == 0xff))
                {

                    swos2_eth_dev.dev[info->channel]->rx_buf.len = info->len;
                    ret = rt_mq_send(swos2_eth_dev.dev[info->channel]->eth_rx_mq,
                                    (const void *)&swos2_eth_dev.dev[info->channel]->rx_buf, sizeof(S_SW_ETH_BUF));
                    if (ret != RT_EOK)
                    {
                        LOG_E("eth %d mq send error", info->channel);
                    }


                }
//                else
//                {
//                    LOG_E("rx %x %x %x %x %x %x",
//                            swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[0],
//                            swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[1],
//                            swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[2],
//                            swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[3],
//                            swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[4],
//                            swos2_eth_dev.dev[info->channel]->rx_buf.data_u8[5]);
//                    LOG_E("tx %x %x %x %x %x %x",
//                            swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[6],
//                            swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[7],
//                            swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[8],
//                            swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[9],
//                            swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[10],
//                            swos2_eth_dev.dev[info->channel]->tx_buf.data_u8[11]);
//                }
            }

            info = RT_NULL;
        }
        else
        {
            LOG_E("mb rcv error %d", ret);
        }

        rt_thread_mdelay(5);
    }
}

static BOOL swos2_eth_dev_init(S_SWOS2_ETH_DEV *dev)
{
    if (NULL == dev)
    {
        return FALSE;
    }

    switch (dev->id)
    {
    case E_SLOT_ID_1:
    case E_SLOT_ID_5:

        if(E_SLOT_ID_1 == dev->id)
        {
            dev->p_dst_mac = com_1_load_1_dst_mac_addr;
            dev->p_src_mac = com_1_load_1_mac_addr;
        }
        else
        {
            dev->p_dst_mac = com_1_load_2_dst_mac_addr;
            dev->p_src_mac = com_1_load_2_mac_addr;
        }

        swos2_eth[E_ETH_CH_1].rx_indicate = sw_eth1_rx_callback;
        swos2_eth[E_ETH_CH_2].rx_indicate = sw_eth2_rx_callback;
        swos2_eth[E_ETH_CH_3].rx_indicate = sw_eth3_rx_callback;

        dev->dev[E_ETH_CH_1] = &swos2_eth[E_ETH_CH_1];
        dev->dev[E_ETH_CH_2] = &swos2_eth[E_ETH_CH_2];
        dev->dev[E_ETH_CH_3] = &swos2_eth[E_ETH_CH_3];
        dev->ch_num = 3;
        break;
    case E_SLOT_ID_3:
    case E_SLOT_ID_7:

        if(E_SLOT_ID_3 == dev->id)
        {
            dev->p_dst_mac = com_2_load_1_dst_mac_addr;
            dev->p_src_mac = com_2_load_1_mac_addr;
        }
        else
        {
            dev->p_dst_mac = com_2_load_2_dst_mac_addr;
            dev->p_src_mac = com_2_load_2_mac_addr;
        }

        swos2_eth[E_ETH_CH_1].rx_indicate = sw_eth1_rx_callback;
        swos2_eth[E_ETH_CH_3].rx_indicate = sw_eth2_rx_callback;

        dev->dev[E_ETH_CH_1] = &swos2_eth[E_ETH_CH_1];
        dev->dev[E_ETH_CH_2] = &swos2_eth[E_ETH_CH_3];
        dev->ch_num = 2;
        break;
    case E_SLOT_ID_2:
    case E_SLOT_ID_4:
    case E_SLOT_ID_6:
    case E_SLOT_ID_8:

        if(E_SLOT_ID_2 == dev->id)
        {
            dev->p_dst_mac = com_1_child_1_dst_mac_addr;
            dev->p_src_mac = com_1_child_1_mac_addr;
        }
        else if(E_SLOT_ID_4 == dev->id)
        {
            dev->p_dst_mac = com_2_child_1_dst_mac_addr;
            dev->p_src_mac = com_2_child_1_mac_addr;
        }
        else if(E_SLOT_ID_6 == dev->id)
        {
            dev->p_dst_mac = com_1_child_2_dst_mac_addr;
            dev->p_src_mac = com_1_child_2_mac_addr;
        }
        else
        {
            dev->p_dst_mac = com_2_child_2_dst_mac_addr;
            dev->p_src_mac = com_2_child_2_mac_addr;
        }


        swos2_eth[E_ETH_CH_1].rx_indicate = sw_eth1_rx_callback;
        swos2_eth[E_ETH_CH_2].rx_indicate = sw_eth2_rx_callback;
        swos2_eth[E_ETH_CH_3].rx_indicate = sw_eth3_rx_callback;

        dev->dev[E_ETH_CH_1] = &swos2_eth[E_ETH_CH_1];
        dev->dev[E_ETH_CH_2] = &swos2_eth[E_ETH_CH_2];
        dev->dev[E_ETH_CH_3] = &swos2_eth[E_ETH_CH_3];
        dev->ch_num = 3;
        break;
    default:
        LOG_E("eth cfg slot id error");
        return FALSE;
    }
    return TRUE;
}

BOOL if_eth_init(void)
{
    rt_thread_t tid;
    uint8_t i;

    memset(&swos2_eth_dev, 0, sizeof(S_SWOS2_ETH_DEV));

    /* 1.识别板子类型 */
    swos2_eth_dev.id = if_gpio_getSlotId();

    /* 2.配置板子网络通道信息 */
    if(swos2_eth_dev_init(&swos2_eth_dev) != TRUE)
    {
        LOG_E("eth slot id error");
        return FALSE;
    }

    /* 3.创建邮箱 */
    if (rt_mb_init(&swos2_eth_dev.mailbox, "eth_rx_t_mb", &eth_mb_pool[0], sizeof(eth_mb_pool) / 4, RT_IPC_FLAG_PRIO) != RT_EOK)
    {
        LOG_E("eth mb init error");
        return FALSE;
    }

    /* 4.创建，启动接收线程 */
    tid = rt_thread_create("if eth rx", sw_ethrx_thread_entry, RT_NULL, 2048, 20, 5);
    if (tid == RT_NULL)
    {
        LOG_E("sw_eth_rx thread create fail!");
        return FALSE;
    }
    else
    {
        rt_thread_startup(tid);
    }

    for(i = 0; i < swos2_eth_dev.ch_num; i++)
    {
        /* 5.查找并打开网口 */
        swos2_eth_dev.dev[i]->dev = rt_device_find(swos2_eth_dev.dev[i]->name);
        if (RT_NULL != swos2_eth_dev.dev[i]->dev)
        {
            if(rt_device_open(swos2_eth_dev.dev[i]->dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
            {
                LOG_E("%s open error", swos2_eth_dev.dev[i]->name);
                return FALSE;
            }
            else
            {
                if(swos2_eth_dev.dev[i]->rx_indicate != NULL)
                {
                    sw_eth_set_rx_callback(swos2_eth_dev.dev[i], swos2_eth_dev.dev[i]->rx_indicate);
                }
                else
                {
                    LOG_E("%s set rx callbac error", swos2_eth_dev.dev[i]->name);
                }
            }
        }
        else
        {
            LOG_E("eth not find %s if!", swos2_eth_dev.dev[i]->name);
            return FALSE;
        }

        /* 6.创建队列 */
        swos2_eth_dev.dev[i]->eth_rx_mq = rt_mq_create(swos2_eth_dev.dev[i]->mq_name,
                                                    sizeof(S_SW_ETH_BUF), SW_ETH_RX_MQ_MAX_NUM, RT_IPC_FLAG_FIFO);
        if (RT_NULL == swos2_eth_dev.dev[i]->eth_rx_mq)
        {
            LOG_E("%s null", swos2_eth_dev.dev[i]->mq_name);
            return FALSE;
        }

        /* 7.设置MAC地址 */
        //目的地址
        swos2_eth_dev.dev[i]->tx_buf.data_u8[0] = *(*(swos2_eth_dev.p_dst_mac + i) + 0);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[1] = *(*(swos2_eth_dev.p_dst_mac + i) + 1);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[2] = *(*(swos2_eth_dev.p_dst_mac + i) + 2);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[3] = *(*(swos2_eth_dev.p_dst_mac + i) + 3);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[4] = *(*(swos2_eth_dev.p_dst_mac + i) + 4);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[5] = *(*(swos2_eth_dev.p_dst_mac + i) + 5);

        //源地址
        swos2_eth_dev.dev[i]->tx_buf.data_u8[6] = *(*(swos2_eth_dev.p_src_mac + i) + 0);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[7] = *(*(swos2_eth_dev.p_src_mac + i) + 1);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[8] = *(*(swos2_eth_dev.p_src_mac + i) + 2);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[9] = *(*(swos2_eth_dev.p_src_mac + i) + 3);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[10] = *(*(swos2_eth_dev.p_src_mac + i) + 4);
        swos2_eth_dev.dev[i]->tx_buf.data_u8[11] = *(*(swos2_eth_dev.p_src_mac + i) + 5);
    }

    return TRUE;
}

BOOL if_eth_send(E_ETH_CH ch, uint8 *pdata, uint16 len)
{
    if(ch >= E_ETH_CH_MAX)
    {
        return FALSE;
    }

    if(len >= (SW_ETH_RX_BUF_MAX_NUM - SW_ETH_LINK_LAYER_MAC_LENTH - 2))
    {
        return FALSE;
    }

    swos2_eth_dev.dev[ch]->tx_buf.len = len + SW_ETH_LINK_LAYER_MAC_LENTH + 2;

    //长度
    swos2_eth_dev.dev[ch]->tx_buf.data_u8[SW_ETH_LINK_LAYER_MAC_LENTH + 0] = swos2_eth_dev.dev[ch]->tx_buf.len >> 8;
    swos2_eth_dev.dev[ch]->tx_buf.data_u8[SW_ETH_LINK_LAYER_MAC_LENTH + 1] = (uint8_t)(swos2_eth_dev.dev[ch]->tx_buf.len & 0xFF);

    rt_memcpy(&swos2_eth_dev.dev[ch]->tx_buf.data_u8[SW_ETH_LINK_LAYER_MAC_LENTH + 2], (const void *)pdata, len);
    
    if(rt_device_write(swos2_eth_dev.dev[ch]->dev, 0, (const void *)swos2_eth_dev.dev[ch]->tx_buf.data_u8, swos2_eth_dev.dev[ch]->tx_buf.len)
            != swos2_eth_dev.dev[ch]->tx_buf.len)
    {
        return FALSE;
    }
    //MY_Printf("swos2_eth_dev eth data len is legal, len:%d !!!\r\n",swos2_eth_dev.dev[ch]->tx_buf.len);
#if 0
    if( 0U != swos2_eth_dev.dev[ch]->tx_buf.len )
    {

        MY_Printf("swos2_eth_dev eth data len is legal, len:%d !!!\r\n",swos2_eth_dev.dev[ch]->tx_buf.len);
        MY_Printf("data:");
        for(uint16 i=0;i<swos2_eth_dev.dev[ch]->tx_buf.len;i++)
        {
            MY_Printf("%.2x ",swos2_eth_dev.dev[ch]->tx_buf.data_u8[i]);
        }
        MY_Printf("\r\n");
    }
#endif

    return TRUE;
}

uint16 if_eth_get(E_ETH_CH ch, uint8 *pdata, uint16 len)
{
    rt_err_t ret;
    S_SW_ETH_BUF rx_buf;
    uint16 read_len = 0;

    ret = rt_mq_recv(swos2_eth_dev.dev[ch]->eth_rx_mq, (void *) &rx_buf, sizeof(S_SW_ETH_BUF), RT_WAITING_NO);
    if (RT_EOK == ret)
    {
        // read_len = rx_buf.len - SW_ETH_LINK_LAYER_MAC_LENTH - 2;
        // rt_memcpy(pdata, &rx_buf.data_u8[SW_ETH_LINK_LAYER_MAC_LENTH + 2], read_len);

        read_len = rx_buf.len - SW_ETH_LINK_LAYER_MAC_LENTH - 2;
        if(read_len <= len)
        {
            rt_memcpy(pdata, &rx_buf.data_u8[SW_ETH_LINK_LAYER_MAC_LENTH + 2], read_len);
        }
        else
        {
            read_len = 0;
        }

        return read_len;
    }
    else
    {
        return 0;
    }
    return 0;
}
