/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-08-17     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"

#include <fal.h>

#define DBG_TAG "i2c "
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define DATA_ADDR   2
#define DATA_LEN    4

//通过读写底板上的eeprom来确认i2c的完整性
void selftest_i2c_test(SelftestlUserData *puserdata)
{
    int result = 0;
    uint8_t data_wr1[DATA_LEN] = {1, 2, 3, 4};
    uint8_t data_wr2[DATA_LEN] = {11, 12, 13, 14};
    uint8_t data_rd1[DATA_LEN] = {0};
    uint8_t data_rd2[DATA_LEN] = {0};

    puserdata->i2c_dev = (struct fal_partition *)fal_partition_find(puserdata->i2c_devname);
    if (puserdata->i2c_dev != NULL)
    {
        result = fal_partition_write(puserdata->i2c_dev, DATA_ADDR, data_wr1, DATA_LEN);
        if (result < 0)
        {
            LOG_E("eeprom write1 error!");
        }
        result = fal_partition_read(puserdata->i2c_dev, DATA_ADDR, data_rd1, DATA_LEN);
        if (result < 0)
        {
            LOG_E("eeprom read1 error!");
        }
        result = fal_partition_write(puserdata->i2c_dev, DATA_ADDR, data_wr2, DATA_LEN);
        if (result < 0)
        {
            LOG_E("eeprom write2 error!");
        }
        result = fal_partition_read(puserdata->i2c_dev, DATA_ADDR, data_rd2, DATA_LEN);
        if (result < 0)
        {
            LOG_E("eeprom read2 error!");
        }

        if (rt_memcmp(data_wr1, data_rd1, DATA_LEN) == 0 && rt_memcmp(data_wr2, data_rd2, DATA_LEN) == 0)
        {
            LOG_D("eeprom read write pass");
        }
        else
        {
            LOG_E("eeprom read write error!");
            LOG_HEX("data1", 16, data_rd1, DATA_LEN);
            LOG_HEX("data2", 16, data_rd2, DATA_LEN);
        }
    }
    else
    {
        LOG_E("Device %s NOT found. Probe failed.", puserdata->i2c_devname);
    }
}
