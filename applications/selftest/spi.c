/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-12     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#include "drv_spi.h"
#include "drv_soft_spi.h"

#define DBG_TAG "spi "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

//通过读写底板上的spinor4的id来确认spi的完整性
void selftest_spi_test(SelftestlUserData *puserdata)
{
    struct rt_spi_device *spidev = NULL;

    char dev_name[RT_NAME_MAX];
    rt_uint8_t dev_num = 0;
    do
    {
        rt_snprintf(dev_name, RT_NAME_MAX, "%s%d", puserdata->spi_devname, dev_num++);
        if (dev_num == 255)
        {
            break;
        }
    } while (rt_device_find(dev_name));

//    rt_hw_soft_spi_device_attach(puserdata->spi_devname, dev_name, puserdata->spi_devname_cs);
    rt_hw_spi_device_attach(puserdata->spi_devname, dev_name, rt_pin_get(puserdata->spi_devname_cs));
    spidev = (struct rt_spi_device *)rt_device_find(dev_name);
    if (spidev == NULL)
    {
        LOG_E("device %s find error!", dev_name);
        return;
    }
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = BSP_AT45DB321E_SPI_SPEED;
    if (rt_spi_configure(spidev, &cfg) != RT_EOK)
    {
        LOG_E("device %s configure error!", dev_name);
        return;
    }

    rt_err_t ret = RT_EOK;

    uint8_t id[5] = {0};
    uint8_t cmd[] = {0x9F};
    ret = rt_spi_send_then_recv(spidev, cmd, sizeof(cmd), id, 5);
    if (ret != RT_EOK)
    {
        LOG_E("read id error %d!", ret);
        ret = -RT_EIO;
    }
    uint8_t check[] = {0x1f, 0x27, 0x01, 0x01, 0x00};
    if (rt_memcmp(id, check, 5) == 0)
    {
        LOG_D("spinor4     pass");
    }
    else
    {
        LOG_E("spinor4 error! 0x%02x %02x %02x %02x %02x.", id[0], id[1], id[2], id[3], id[4]);
    }

    return;
}
