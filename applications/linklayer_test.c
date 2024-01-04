/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-19     zm       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>

#define DBG_TAG "linklayer"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>


#define LINK_LAYER_THREAD_STACK_SIZE       2048
#define LINK_LAYER_THREAD_PRIORITY         19
#define LINK_LAYER_THREAD_TIMESLICE        5

#ifdef BSP_USING_LINK_LAYER_CHANNEL_1
static uint8_t link_layer1_Txbuf[1500];
static uint8_t link_layer1_Rxbuf[1500];

const uint8_t linklayer_mac1[6]= {0xfc,0x3f,0xab,0x23,0x00,0x39};
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_2
static uint8_t link_layer2_Txbuf[1500];
static uint8_t link_layer2_Rxbuf[1500];

const uint8_t linklayer_mac2[6]= {0xfc,0x3f,0xab,0x00,0x23,0x00};
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_3
static uint8_t link_layer3_Txbuf[1500];
static uint8_t link_layer3_Rxbuf[1500];

const uint8_t linklayer_mac3[6]= {0xfc,0x3f,0xab,0x0f,0x00,0x23};
#endif

typedef enum {
#ifdef BSP_USING_LINK_LAYER_CHANNEL_1
    LinkLayerChannel1 = 0x00U,
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_2
    LinkLayerChannel2 = 0x01U,
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_3
    LinkLayerChannel3 = 0x02U,
#endif

    LinkLayerChannelALL
} LinkLayerChannel;

static rt_device_t eth_dev[LinkLayerChannelALL];

static void LinkLayerTx(rt_uint16_t channel, const void *buffer, rt_uint16_t size)
{
    rt_uint16_t r_size = 0;

    r_size = rt_device_write(eth_dev[channel], 0, (void *)buffer, size);
    if(r_size != size)
    {
        LOG_D("eth %d tx error", channel);
    }
}

static void LinkLayerSetRXCallback(rt_uint16_t ch, rt_err_t (*rx_ind)(rt_device_t dev,rt_size_t size))
{
    if(rx_ind != NULL)
    {
        rt_device_set_rx_indicate(eth_dev[ch], rx_ind);
    }
}

static void LinkLayerChannelInit(char *device_name, rt_uint16_t ch)
{
    eth_dev[ch] = rt_device_find(device_name);
    if (eth_dev[ch] == RT_NULL)
    {
        LOG_E("%s find NULL.", device_name);
    }

    if(rt_device_open(eth_dev[ch], RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("%s open fail.", device_name);
    }

    LOG_I("%s open successful", device_name);
}


static rt_err_t LinkLayerRXCallbackTest(rt_device_t dev, rt_size_t size)
{
    if(size != 0)
    {
        if(dev == eth_dev[LinkLayerChannel1])
        {
            LOG_I("e0:");
            LOG_I("rx size %d", size);
            rt_device_read(dev, 0, (void *)link_layer1_Rxbuf, size);
            LOG_I("des mac %x %x %x %x %x %x ",
                    link_layer1_Rxbuf[0], link_layer1_Rxbuf[1], link_layer1_Rxbuf[2], link_layer1_Rxbuf[3], link_layer1_Rxbuf[4], link_layer1_Rxbuf[5]);
            LOG_I("src mac %x %x %x %x %x %x ",
                    link_layer1_Rxbuf[6], link_layer1_Rxbuf[7], link_layer1_Rxbuf[8], link_layer1_Rxbuf[9], link_layer1_Rxbuf[10], link_layer1_Rxbuf[11]);
            LOG_I("recv data %x %x %x %x %x %x ",
                    link_layer1_Rxbuf[1018], link_layer1_Rxbuf[1019], link_layer1_Rxbuf[1020], link_layer1_Rxbuf[1021], link_layer1_Rxbuf[1022], link_layer1_Rxbuf[1023]);
#if 0
            if(strncmp(link_layer1_Rxbuf, link_layer2_Txbuf, 1024) == 0)
            {
                LOG_I("e1 ---> e0 test ok");
            }
            else
            {
                LOG_E("e1 ---> e0 test error");
            }
#endif
        }
#ifdef BSP_USING_LINK_LAYER_CHANNEL_2
        else if(dev == eth_dev[LinkLayerChannel2])
        {
            LOG_I("e1:");
            rt_device_read(dev, 0, (void *)link_layer2_Rxbuf, size);
            LOG_I("des mac %x %x %x %x %x %x ",
                    link_layer2_Rxbuf[0], link_layer2_Rxbuf[1], link_layer2_Rxbuf[2], link_layer2_Rxbuf[3], link_layer2_Rxbuf[4], link_layer2_Rxbuf[5]);
            LOG_I("src mac %x %x %x %x %x %x ",
                    link_layer2_Rxbuf[6], link_layer2_Rxbuf[7], link_layer2_Rxbuf[8], link_layer2_Rxbuf[9], link_layer2_Rxbuf[10], link_layer2_Rxbuf[11]);
            LOG_I("recv data %x %x %x %x %x %x ",
                    link_layer2_Rxbuf[1018], link_layer2_Rxbuf[1019], link_layer2_Rxbuf[1020], link_layer2_Rxbuf[1021], link_layer2_Rxbuf[1022], link_layer2_Rxbuf[1023]);
        }
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_3
        else if(dev == eth_dev[LinkLayerChannel3])
        {
            LOG_I("e2:");
            rt_device_read(dev, 0, (void *)link_layer3_Rxbuf, size);
            LOG_I("des mac %x %x %x %x %x %x ",
                    link_layer3_Rxbuf[0], link_layer3_Rxbuf[1], link_layer3_Rxbuf[2], link_layer3_Rxbuf[3], link_layer3_Rxbuf[4], link_layer3_Rxbuf[5]);
            LOG_I("src mac %x %x %x %x %x %x ",
                    link_layer3_Rxbuf[6], link_layer3_Rxbuf[7], link_layer3_Rxbuf[8], link_layer3_Rxbuf[9], link_layer3_Rxbuf[10], link_layer3_Rxbuf[11]);
            LOG_I("recv data %x %x %x %x %x %x ",
                    link_layer3_Rxbuf[1018], link_layer3_Rxbuf[1019], link_layer3_Rxbuf[1020], link_layer3_Rxbuf[1021], link_layer3_Rxbuf[1022], link_layer3_Rxbuf[1023]);
        }
#endif
    }
    return RT_EOK;
}

static void LinkLayerTestThreadEntry(void *arg)
{
#ifdef BSP_USING_LINK_LAYER_CHANNEL_1
    LinkLayerChannelInit("e0", LinkLayerChannel1);
    LinkLayerSetRXCallback(LinkLayerChannel1, LinkLayerRXCallbackTest);

    memcpy(link_layer1_Txbuf,linklayer_mac2,sizeof(linklayer_mac2));          //目的地址
    memcpy(&link_layer1_Txbuf[6],linklayer_mac1,sizeof(linklayer_mac1));      //源地址
    memset(&link_layer1_Txbuf[12],0xcc,1024);
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_2
    LinkLayerChannelInit("e1", LinkLayerChannel2);
    LinkLayerSetRXCallback(LinkLayerChannel2, LinkLayerRXCallbackTest);

    memcpy(link_layer2_Txbuf,linklayer_mac1,sizeof(linklayer_mac1));
    memcpy(&link_layer2_Txbuf[6],linklayer_mac2,sizeof(linklayer_mac2));
    memset(&link_layer2_Txbuf[12],0xaa,1024);
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_3
    LinkLayerChannelInit("e2", LinkLayerChannel3);
    LinkLayerSetRXCallback(LinkLayerChannel3, LinkLayerRXCallbackTest);

    memcpy(link_layer3_Txbuf,linklayer_mac1,sizeof(linklayer_mac1));
    memcpy(&link_layer3_Txbuf[6],linklayer_mac3,sizeof(linklayer_mac3));
    memset(&link_layer3_Txbuf[12],0xbb,1024);
#endif

    while(1)
    {
#ifdef BSP_USING_LINK_LAYER_CHANNEL_1
        LinkLayerTx(LinkLayerChannel1, (const void *)link_layer1_Txbuf, 1024);
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_2
        LinkLayerTx(LinkLayerChannel2, (const void *)link_layer2_Txbuf, 1024);
#endif

#ifdef BSP_USING_LINK_LAYER_CHANNEL_3
        LinkLayerTx(LinkLayerChannel3, (const void *)link_layer3_Txbuf, 1024);
#endif
        rt_thread_mdelay(100);
    }
}

static rt_thread_t linklayer_thread = RT_NULL;

static void linklayer_test(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: linklayertest [cmd]\n");
        rt_kprintf("       linklayertest --start\n");
        rt_kprintf("       linklayertest --stop\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--start") == 0)
        {
            if (linklayer_thread == RT_NULL)
            {
                linklayer_thread = rt_thread_create("link layer test", LinkLayerTestThreadEntry, RT_NULL,
                                       LINK_LAYER_THREAD_STACK_SIZE, LINK_LAYER_THREAD_PRIORITY, LINK_LAYER_THREAD_TIMESLICE);
                if(linklayer_thread != NULL)
                {
                    rt_thread_startup(linklayer_thread);
                }
                else
                {
                    LOG_E("thread create error!");
                }
            }
            else
            {
                LOG_W("thread already exists!");
            }
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            if(linklayer_thread != RT_NULL)
            {
                if(rt_thread_delete(linklayer_thread) == RT_EOK)
                {
                    LOG_I("thread delete ok!");
                }
                else
                {
                    LOG_E("thread delete error!");
                }
            }
            else
            {
                LOG_W("thread already delete!");
            }
            linklayer_thread = RT_NULL;
        }
        else
        {
            rt_kprintf("Usage: linklayertest [cmd]\n");
            rt_kprintf("       linklayertest --start\n");
            rt_kprintf("       linklayertest --stop\n");
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(linklayer_test, linklayertest, link layer test);
#endif /* RT_USING_FINSH */
