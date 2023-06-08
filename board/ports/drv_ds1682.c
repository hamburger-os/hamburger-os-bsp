/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-08-15     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_DS1682

#define DBG_TAG "ds1682"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define DS1682_RD           0x01
#define DS1682_WR           0x00

// ds1682 register address
#define REG_CFG             0x00
#define REG_ALARM_L         0x01
#define REG_ALARM_LM        0x02
#define REG_ALARM_HM        0x03
#define REG_ALARM_H         0x04
#define REG_ETC_L           0x05
#define REG_ETC_LM          0x06
#define REG_ETC_HM          0x07
#define REG_ETC_H           0x08
#define REG_EC_L            0x09
#define REG_EC_H            0x0A
#define REG_U_MEM_START     0x0B
#define REG_U_MEM_STO       0x14
#define REG_RESET_CMD       0x1D
#define REG_WT_DISABLE      0x1E
#define REG_WT_MEM_DISABLE  0x1F

struct __attribute__((packed)) DS1682DataDef
{
    rt_uint32_t times;
    rt_uint16_t count;
};

static struct rt_i2c_bus_device *i2c_bus = NULL; /* I2C总线设备句柄 */

static rt_err_t ds1682_write_byte(rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2] = {0};
    buf[0] = reg; // cmd
    buf[1] = data;

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(i2c_bus, DS1682_I2C_ADD, 0, buf, 2) != 2)
    {
        LOG_E("i2c_master_send write error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t ds1682_read_byte(rt_uint8_t reg, rt_uint8_t *data)
{
    rt_uint8_t buf[1];
    buf[0] = reg; // cmd

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(i2c_bus, DS1682_I2C_ADD, 0, buf, 1) != 1)
    {
        LOG_E("i2c_master_send write addr error!");
        return -RT_ERROR;
    }

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_recv(i2c_bus, DS1682_I2C_ADD, 0, buf, 1) == 1)
    {
        *data = buf[0];
    }
    else
    {
        LOG_E("i2c_master_recv read error!");
        return -RT_ERROR;
    }

    return RT_EOK;
}

/* RT-Thread Device Driver Interface */
static rt_err_t rt_ds1682_init(rt_device_t dev)
{
    rt_uint8_t data = 0;

    /* 查找I2C总线设备，获取I2C总线设备句柄 */
    i2c_bus = rt_i2c_bus_device_find(DS1682_I2C_DEV);
    if (i2c_bus == NULL)
    {
        LOG_E("i2c bus error!");
        return -RT_EIO;
    }

    // memory read write test
    ds1682_read_byte(REG_U_MEM_START, &data);
    if (data != 0xAA) // first power on
    {
        ds1682_write_byte(REG_U_MEM_START, 0xAA);
        ds1682_read_byte(REG_U_MEM_START, &data);
        if (data != 0xAA)
        {
            LOG_E("init error 0x%x != 0x%x!", data, 0xAA);
            return -RT_ERROR;
        }
    }
    LOG_I("init succeed!");
    return RT_EOK;
}

static rt_size_t rt_ds1682_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    if (size < sizeof(struct DS1682DataDef))
    {
        LOG_E("size < %d", sizeof(struct DS1682DataDef));
    }
    else
    {
        //读取ds1682上电运行时间,单位S
        rt_uint32_t times = 0;
        rt_uint8_t time_node[4] = {0};
        ds1682_read_byte(REG_ETC_L, &time_node[0]);
        ds1682_read_byte(REG_ETC_LM, &time_node[1]);
        ds1682_read_byte(REG_ETC_HM, &time_node[2]);
        ds1682_read_byte(REG_ETC_H, &time_node[3]);
        times = (rt_uint32_t)((time_node[0]) + (time_node[1] << 8) + (time_node[2] << 16) + (time_node[3] << 24));
        times = times >> 2;

        //读取ds1682上电次数
        rt_uint16_t count = 0;
        rt_uint8_t count_node[2] = {0};
        ds1682_read_byte(REG_EC_L, &count_node[0]);
        ds1682_read_byte(REG_EC_H, &count_node[1]);
        count = (rt_uint16_t)((count_node[0]) + (count_node[1] << 8));

        struct DS1682DataDef *data = buffer;
        data->times = times;
        data->count = count;

        LOG_D("time : %d s, count : %d", data->times, data->count);
    }

    return 0;
}

static int rt_hw_ds1682_init(void)
{
    //注册设备
    rt_device_t ds1682_device = rt_malloc(sizeof(struct rt_device));
    if (ds1682_device)
    {
        ds1682_device->type = RT_Device_Class_Char;
        ds1682_device->init = rt_ds1682_init;
        ds1682_device->open = NULL;
        ds1682_device->close = NULL;
        ds1682_device->read = rt_ds1682_read;
        ds1682_device->write = NULL;
        ds1682_device->control = NULL;
        if (rt_device_register(ds1682_device, "ds1682", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE) == RT_EOK)
        {
            LOG_I("device created successfully!");
        }
        else
        {
            LOG_E("device created failed!");
            return -RT_ERROR;
        }
    }
    else
    {
        LOG_E("no memory for create device");
        return -RT_ERROR;
    }

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_ds1682_init);

#endif
