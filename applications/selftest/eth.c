/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-18     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#define DBG_TAG "eth "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

union LinkLayerPackDef
{
    //链路层包最短64字节
    uint8_t data[64];

    struct
    {
        uint8_t mac_des[6];
        uint8_t mac_src[6];
        uint8_t type[2];
        uint8_t data[46];
        uint8_t crc[4];
    }frame;
};

static char * error_log[] = {
    "ETH1      ----> ETH2     ",
    "ETH2      ----> ETH1     ",
};

void selftest_eth_test(SelftestlUserData *puserdata)
{
    union LinkLayerPackDef data_wr = {
        .frame.mac_des = {0xF8,0x09,0xA4,0x00,0x23,0x00},
        .frame.mac_src = {0xF8,0x09,0xA4,0x00,0x23,0x01},
        .frame.data = {1,2,3,4,5,6,7,8},
    };
    union LinkLayerPackDef data_rd = {0};
    uint16_t data_rd_len = 0;

    for (int i = 0; i<2; i++)
    {
        /* step1：查找设备 */
        puserdata->eth_dev[i][0] = rt_device_find(puserdata->eth_devname[i][0]);
        puserdata->eth_dev[i][1] = rt_device_find(puserdata->eth_devname[i][1]);

        /* step4：打开设备 */
        if(rt_device_open(puserdata->eth_dev[i][0], RT_DEVICE_FLAG_RDWR) != RT_EOK)
        {
            LOG_E("open eth '%s' failed", puserdata->eth_devname[i][0]);
        }
    }

    for (int i = 0; i<2; i++)
    {
        rt_device_read(puserdata->eth_dev[i][1], 0, &data_rd, sizeof(union LinkLayerPackDef));
        rt_device_read(puserdata->eth_dev[i][1], 0, &data_rd, sizeof(union LinkLayerPackDef));
        rt_device_write(puserdata->eth_dev[i][0], 0, &data_wr, sizeof(union LinkLayerPackDef));
        rt_thread_delay(100);

        rt_memset(&data_rd, 0, sizeof(union LinkLayerPackDef));
        data_rd_len = rt_device_read(puserdata->eth_dev[i][1], 0, &data_rd, sizeof(union LinkLayerPackDef));
        if (data_rd_len < 0)
        {
            LOG_E("%s read failed %d", error_log[i], data_rd_len);
        }
        if (rt_memcmp(&data_wr, &data_rd, sizeof(union LinkLayerPackDef)) == 0)
        {
            LOG_D("%s pass", error_log[i]);
        }
        else
        {
            LOG_E("%s error!", error_log[i]);
            LOG_HEX("wr", 16, (uint8_t *)&data_wr, sizeof(union LinkLayerPackDef));
            LOG_HEX("rd", 16, (uint8_t *)&data_rd, sizeof(union LinkLayerPackDef));
        }
    }

    for (int i = 0; i<2; i++)
    {
        rt_device_close(puserdata->eth_dev[i][0]);
    }
}
