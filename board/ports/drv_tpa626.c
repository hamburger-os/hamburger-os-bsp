/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-25     Administrator       the first version
 */

#include "board.h"
#include <rtdbg.h>
#include <drv_tpa626.h>

static struct rt_mutex tpa626_mux;

typedef struct {
    struct rt_device dev;                 /* 设备 */
    const char *device_name;
    struct rt_i2c_bus_device *i2c_bus;    /* I2C总线设备句柄 */
    const char *i2c_bus_name;
    uint8_t i2c_addr;
} TPA626_Dev;


static TPA626_Dev tpa626_dev =
{
    .device_name = "tpa626",
    .i2c_bus_name = TPA626_I2C_DEV,
    .i2c_addr = TPA626_I2C_ADD,
};


static rt_err_t tpa626_write_reg(rt_uint8_t regaddr, uint16_t data)
{
    rt_uint8_t buf[3] = {0};

    rt_mutex_take(&tpa626_mux, RT_WAITING_FOREVER);
    buf[0] = regaddr;
    buf[1] = (data>>8) & 0xFF;
    buf[2] = data & 0xFF;

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(tpa626_dev.i2c_bus, tpa626_dev.i2c_addr, 0, buf, 3) != 3)
    {
        LOG_E("i2c_master_send write error!");
        rt_mutex_release(&tpa626_mux);
        return -RT_ERROR;
    }
    rt_mutex_release(&tpa626_mux);
    return RT_EOK;
}

static rt_err_t tpa626_read_reg(rt_uint8_t regaddr, uint16_t *pdata)
{

    rt_uint8_t buf[2] = {0};
    buf[0] = regaddr; // cmd

    if(NULL == pdata)
    {
        return -RT_EEMPTY;
    }

    rt_mutex_take(&tpa626_mux, RT_WAITING_FOREVER);

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(tpa626_dev.i2c_bus, tpa626_dev.i2c_addr, 0, buf, 1) != 1)
    {
        LOG_E("i2c_master_send write addr error!");
        rt_mutex_release(&tpa626_mux);
        return -RT_ERROR;
    }

    /* 调用I2C设备接口传输数*/
    if (rt_i2c_master_recv(tpa626_dev.i2c_bus, tpa626_dev.i2c_addr, 0, buf, 2) == 2)
    {
        *pdata = (buf[0] << 8) + buf[1];
    }
    else
    {
        LOG_E("i2c_master_recv read error!");
        rt_mutex_release(&tpa626_mux);
        return -RT_ERROR;
    }
    rt_mutex_release(&tpa626_mux);
    return RT_EOK;
}


static rt_err_t tpa626_init(rt_device_t dev)
{
    if(RT_EOK != tpa626_write_reg(CFG_REG_ADDR, CFG_REG_DFLT_VAL))
    {
        LOG_E("tpa626 init err");
        return -RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}


static rt_size_t tpa626_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_uint16_t read_val;
    rt_uint32_t cur_vol_temp;

    if(dev == NULL)
    {
        return -RT_ERROR;
    }

    switch(pos)
    {
        case TPA626_CURRENT_CHANL:
            tpa626_read_reg(SHUNT_REG_ADDR, &read_val);
            cur_vol_temp = 5*(rt_uint32_t)read_val/2/SAMP_RES_VAL;
            rt_memcpy(buffer, &cur_vol_temp, size);
            break;

        case TPA626_BUS_VOL_CHANL:
            tpa626_read_reg(BUS_VOLT_REG_ADDR, &read_val);
            cur_vol_temp = 5*(rt_uint32_t)read_val/4;
            rt_memcpy(buffer, &cur_vol_temp, size);
            break;
        default:
            break;
    }

}


static int rt_hw_tpa626_init(void)
{
    rt_err_t result;

    result = rt_mutex_init(&tpa626_mux, "tpa626", RT_IPC_FLAG_PRIO);
    if (result != RT_EOK)
    {
        LOG_E("mutex init error %d!", result);
        return -RT_EIO;
    }

    /* 查找I2C总线设备，获取I2C总线设备句柄 */
    tpa626_dev.i2c_bus = rt_i2c_bus_device_find(tpa626_dev.i2c_bus_name);
    if (tpa626_dev.i2c_bus == NULL)
    {
      LOG_E("i2c bus is not find!");
      return -RT_EIO;
    }

//    if(tpa626_init() != RT_EOK)
//    {
//        LOG_E("tpa626 init error");
//        return -RT_ERROR;
//    }
    tpa626_dev.dev.type = RT_Device_Class_I2CBUS;
    tpa626_dev.dev.read = tpa626_read;
    tpa626_dev.dev.init = tpa626_init;

    /* register ADC device */
    if (rt_device_register(&tpa626_dev.dev, tpa626_dev.device_name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE) == RT_EOK)
    {
        LOG_I("tpa626 init success");
    }
    else
    {
        LOG_E("tpa626 register failed");
        return -RT_ERROR;
    }

    return RT_EOK;
}

/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_tpa626_init);
