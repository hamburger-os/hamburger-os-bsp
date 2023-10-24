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
#define DATA_NUM    8

//通过读写底板上的eeprom来确认i2c的完整性
void selftest_i2c_test(SelftestUserData *puserdata)
{
    uint8_t data_wr[DATA_LEN] = {1, 2, 3, 4};
    uint8_t data_rd[DATA_LEN] = {0};

    puserdata->i2c_dev = (struct fal_partition *)fal_partition_find(puserdata->i2c_devname);
    if (puserdata->i2c_dev != NULL)
    {
        uint8_t i = 0;
        for (i = 0; i < DATA_NUM; i++)
        {
            fal_partition_write(puserdata->i2c_dev, DATA_ADDR, data_wr, DATA_LEN);
            fal_partition_read(puserdata->i2c_dev, DATA_ADDR, data_rd, DATA_LEN);

            if (rt_memcmp(data_wr, data_rd, DATA_LEN) == 0)
            {
                i = 0;
                LOG_D("eeprom      pass");
                puserdata->result[RESULT_EEPROM].result = 0;
                break;
            }
        }
        if (i > 0)
        {
            LOG_E("eeprom      error!");
            puserdata->result[RESULT_EEPROM].result = 1;
        }
    }
    else
    {
        LOG_E("Device %s NOT found. Probe failed.", puserdata->i2c_devname);
    }
}
