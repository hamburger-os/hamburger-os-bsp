/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-06     zm       the first version
 */
#include "board.h"

#ifdef BSP_USING_LTC4281I

#define DBG_TAG "ltc4281i"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define I2Cx_TIMEOUT_MAX          100
#define RESISTOR                  0.05

/*! @name Registers */

#define LTC4281_CONTROL_MSB_REG                     0x00
#define LTC4281_CONTROL_LSB_REG                     0x01
#define LTC4281_ALERT_MSB_REG                       0x02
#define LTC4281_ALERT_LSB_REG                       0x03
#define LTC4281_FAULT_LOG_REG                       0x04
#define LTC4281_ADC_ALERT_LOG_REG                   0x05
#define LTC4281_FET_BAD_FAULT_TIME_REG              0x06
#define LTC4281_GPIO_CONFIG_REG                     0x07
#define LTC4281_VSOURCE_ALARM_MIN_REG               0x08
#define LTC4281_VSOURCE_ALARM_MAX_REG               0x09
#define LTC4281_VGPIO_ALARM_MIN_REG                 0x0A
#define LTC4281_VGPIO_ALARM_MAX_REG                 0x0B
#define LTC4281_VSENSE_ALARM_MIN_REG                0x0C
#define LTC4281_VSENSE_ALARM_MAX_REG                0x0D
#define LTC4281_POWER_ALARM_MIN_REG                 0x0E
#define LTC4281_POWER_ALARM_MAX_REG                 0x0F
#define LTC4281_CLK_DEC_REG                         0x10
#define LTC4281_ILIM_ADJUST_REG                     0x11
#define LTC4281_METER_MSB5_REG                      0x12
#define LTC4281_METER_MSB4_REG                      0x13
#define LTC4281_METER_MSB3_REG                      0x14
#define LTC4281_METER_MSB2_REG                      0x15
#define LTC4281_METER_MSB1_REG                      0x16
#define LTC4281_METER_LSB_REG                       0x17
#define LTC4281_TICK_COUNTER_MSB3_REG               0x18
#define LTC4281_TICK_COUNTER_MSB2_REG               0x19
#define LTC4281_TICK_COUNTER_MSB1_REG               0x1A
#define LTC4281_TICK_COUNTER_LSB_REG                0x1B
#define LTC4281_ALERT_CONTROL_REG                   0x1C
#define LTC4281_ADC_CONTROL_REG                     0x1D
#define LTC4281_STATUS_MSB_REG                      0x1E
#define LTC4281_STATUS_LSB_REG                      0x1F
#define LTC4281_EE_CONTROL_MSB_REG                  0x20
#define LTC4281_EE_CONTROL_LSB_REG                  0x21
#define LTC4281_EE_ALERT_MSB_REG                    0x22
#define LTC4281_EE_ALERT_LSB_REG                    0x23
#define LTC4281_EE_FAULT_REG                        0x24
#define LTC4281_EE_ADC_ALERT_LOG_REG                0x25
#define LTC4281_EE_FET_BAD_FAULT_TIME_REG           0x26
#define LTC4281_EE_GPIO_CONFIG_REG                  0x27
#define LTC4281_EE_VSOURCE_ALARM_MIN_REG            0x28
#define LTC4281_EE_VSOURCE_ALARM_MAX_REG            0x29
#define LTC4281_EE_VGPIO_ALARM_MIN_REG              0x2A
#define LTC4281_EE_VGPIO_ALARM_MAX_REG              0x2B
#define LTC4281_EE_VSENSE_ALARM_MIN_REG             0x2C
#define LTC4281_EE_VSENSE_ALARM_MAX_REG             0x2D
#define LTC4281_EE_POWER_ALARM_MIN_REG              0x2E
#define LTC4281_EE_POWER_ALARM_MAX_REG              0x2F
#define LTC4281_EE_CLK_DEC_REG                      0x30
#define LTC4281_EE_ILIM_ADJUST_REG                  0x31
#define LTC4281_VGPIO_MSB_REG                       0x34
#define LTC4281_VGPIO_LSB_REG                       0x35
#define LTC4281_VGPIO_MIN_MSB_REG                   0x36
#define LTC4281_VGPIO_MIN_LSB_REG                   0x37
#define LTC4281_VGPIO_MAX_MSB_REG                   0x38
#define LTC4281_VGPIO_MAX_LSB_REG                   0x39
#define LTC4281_VSOURCE_MSB_REG                     0x3A
#define LTC4281_VSOURCE_LSB_REG                     0x3B
#define LTC4281_VSOURCE_MIN_MSB_REG                 0x3C
#define LTC4281_VSOURCE_MIN_LSB_REG                 0x3D
#define LTC4281_VSOURCE_MAX_MSB_REG                 0x3E
#define LTC4281_VSOURCE_MAX_LSB_REG                 0x3F
#define LTC4281_VSENSE_MSB_REG                      0x40
#define LTC4281_VSENSE_LSB_REG                      0x41
#define LTC4281_VSENSE_MIN_MSB_REG                  0x42
#define LTC4281_VSENSE_MIN_LSB_REG                  0x43
#define LTC4281_VSENSE_MAX_MSB_REG                  0x44
#define LTC4281_VSENSE_MAX_LSB_REG                  0x45
#define LTC4281_POWER_MSB_REG                       0x46
#define LTC4281_POWER_LSB_REG                       0x47
#define LTC4281_POWER_MIN_MSB_REG                   0x48
#define LTC4281_POWER_MIN_LSB_REG                   0x49
#define LTC4281_POWER_MAX_MSB_REG                   0x4A
#define LTC4281_POWER_MAX_LSB_REG                   0x4B

#define LTC4281_EE_SPARE_MSB3_REG                   0x4C
#define LTC4281_EE_SPARE_MSB2_REG                   0x4D
#define LTC4281_EE_SPARE_MSB1_REG                   0x4E
#define LTC4281_EE_SPARE_LSB_REG                    0x4F

/*! @name Command Codes */

#define LTC4281_VIN_MODE_3_V_3                  0x00
#define LTC4281_VIN_MODE_5_V                    0x01
#define LTC4281_VIN_MODE_12_V                   0x02
#define LTC4281_VIN_MODE_24_V                   0x03
#define LTC4281_ILIM_ADJUST_12_V_5              0x00
#define LTC4281_ILIM_ADJUST_15_V_6              0x20
#define LTC4281_ILIM_ADJUST_18_V_7              0x40
#define LTC4281_ILIM_ADJUST_21_V_8              0x60
#define LTC4281_ILIM_ADJUST_25_V_0              0x80
#define LTC4281_ILIM_ADJUST_28_V_1              0xA0
#define LTC4281_ILIM_ADJUST_31_V_2              0xC0
#define LTC4281_ILIM_ADJUST_34_V_3              0xE0
#define LTC4281_FOLDBACK_MODE_3_V_3             0x00
#define LTC4281_FOLDBACK_MODE_5_V_0             0x08
#define LTC4281_FOLDBACK_MODE_12_V_0            0x10
#define LTC4281_FOLDBACK_MODE_24_V_0            0x18
#define LTC4281_ADC_VSOURCE                     0x04
#define LTC4281_ADC_GPIO2_MODE                  0x02
#define LTC4281_ADC_16_BIT                      0x01

#define LTC4281_BAD_FAULT_TIME_3MS              0x03

typedef struct {
  struct rt_adc_device    adcdev;   /* 设备 */
  struct rt_i2c_bus_device *i2c_bus;    /* I2C总线设备句柄 */
  uint8_t                 resolution;
  uint16_t                vref_mv;

} S_LTC4281I_DRIVER;

static S_LTC4281I_DRIVER ltc4281i_dev = {
    .resolution = 1,
    .vref_mv = 1,
};

static rt_err_t ltc4281i_write_byte(S_LTC4281I_DRIVER *dev, rt_uint8_t reg, rt_uint8_t data)
{
    rt_uint8_t buf[2] = {0};
    buf[0] = reg; // cmd
    buf[1] = data;

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(dev->i2c_bus, LTC4281I_I2C_ADD, 0, buf, 2) != 2)
    {
        LOG_E("i2c_master_send write error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t ltc4281i_read_word(S_LTC4281I_DRIVER *dev, rt_uint8_t reg, rt_uint16_t *data)
{
    rt_uint8_t buf[2] = {0};
    buf[0] = reg; // cmd

    if(NULL == dev)
    {
        return -RT_EEMPTY;
    }

    if(NULL == dev->i2c_bus)
    {
        return -RT_EEMPTY;
    }

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_master_send(dev->i2c_bus, LTC4281I_I2C_ADD, reg, buf, 1) != 1)
    {
        LOG_E("i2c_master_send write addr error!");
        return -RT_ERROR;
    }

    /* 调用I2C设备接口传输数*/
    if (rt_i2c_master_recv(dev->i2c_bus, LTC4281I_I2C_ADD, reg, buf, 2) == 2)
    {
        *data = (buf[0] << 8) + buf[1];
        LOG_I("rt_i2c_master_recv %d", *data);
    }
    else
    {
        LOG_E("i2c_master_recv read error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t ltc4281i_init(S_LTC4281I_DRIVER *dev)
{
    /* 寄存器配置 */
    uint8_t ILIM_ADJUST = 0;
    uint16_t pbuf;

    ltc4281i_write_byte(dev, LTC4281_CONTROL_LSB_REG, LTC4281_VIN_MODE_24_V);
    rt_thread_mdelay(50);
    ltc4281i_write_byte(dev, LTC4281_EE_CONTROL_LSB_REG, LTC4281_VIN_MODE_24_V);
    rt_thread_mdelay(50);
    ltc4281i_write_byte(dev, LTC4281_FET_BAD_FAULT_TIME_REG, LTC4281_BAD_FAULT_TIME_3MS);
    rt_thread_mdelay(50);

    /* ILIM_ADJUST_REG配置(Ilim = 25mV/50mO = 0.5A) */
    ILIM_ADJUST = (LTC4281_ILIM_ADJUST_25_V_0|LTC4281_FOLDBACK_MODE_24_V_0|LTC4281_ADC_VSOURCE|LTC4281_ADC_GPIO2_MODE)& ~LTC4281_ADC_16_BIT;
    ltc4281i_write_byte(dev, LTC4281_ILIM_ADJUST_REG, ILIM_ADJUST);
    rt_thread_mdelay(50);

    /* test */
    if(ltc4281i_read_word(dev, LTC4281_CONTROL_MSB_REG, &pbuf) != RT_EOK)
    {
        LOG_E("test read word MSB_REG error");
        return -RT_ERROR;
    }
    return RT_EOK;
}


static rt_err_t ltc4281i_adc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{
    RT_ASSERT(device != RT_NULL);
    LOG_I("ltc4281i_adc_enabled");
    return RT_EOK;
}

static rt_err_t tc4281i_adc_get_value(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
    RT_ASSERT(device != RT_NULL);
    rt_err_t ret = RT_EOK;

    LOG_I("tc4281i_adc_get_value");
    uint16_t data = 0xffff;
    S_LTC4281I_DRIVER *driver = device->parent.user_data;

    RT_ASSERT(driver != RT_NULL);

    switch(channel)
    {
    case 0:  /* 读电压 */
        ret = ltc4281i_read_word(driver, LTC4281_VSOURCE_MSB_REG, &data);
//        voltage = (data * 33.28) / ((65536) - 1) * 100;
        data = (data * 3328) / ((65536) - 1);
        break;
    case 1:  /* 读电流 */
        ret = ltc4281i_read_word(driver, LTC4281_VSENSE_MSB_REG, &data);
//        data = (data * 0.100) / (((65536) - 1) * RESISTOR);
        data = (data * 0.100) / (((65536) - 1) * 5);
        break;
    default:
        break;
    }

    LOG_I("tc4281i_adc_get_value data %d", data);
    *value = data;

    return ret;
}

static rt_uint8_t ltc4281i_adc_get_resolution(struct rt_adc_device *device)
{
    RT_ASSERT(device != RT_NULL);

    S_LTC4281I_DRIVER *driver = device->parent.user_data;

    LOG_I("ltc4281i_adc_get_resolution");
    return driver->resolution;
}

static rt_uint32_t ltc4281i_adc_get_vref (struct rt_adc_device *device)
{
    RT_ASSERT(device != RT_NULL);

    S_LTC4281I_DRIVER *driver = device->parent.user_data;

    LOG_I("ltc4281i_adc_get_vref");
    return driver->vref_mv;
}

static const struct rt_adc_ops ltc4281i_adc_ops =
{
    .enabled = ltc4281i_adc_enabled,
    .convert = tc4281i_adc_get_value,
    .get_resolution = ltc4281i_adc_get_resolution,
    .get_vref = ltc4281i_adc_get_vref,
};

static int rt_hw_ltc4281i_init(void)
{
    /* 查找I2C总线设备，获取I2C总线设备句柄 */
    ltc4281i_dev.i2c_bus = rt_i2c_bus_device_find(LTC4281I_I2C_DEV);
    if (ltc4281i_dev.i2c_bus == NULL)
    {
      LOG_E("i2c bus is not find!");
      return -RT_EIO;
    }

    if(ltc4281i_init(&ltc4281i_dev) != RT_EOK)
    {
        LOG_E("ltc4281i_init error");
        return -RT_ERROR;
    }

    /* register ADC device */
    if (rt_hw_adc_register(&ltc4281i_dev.adcdev, "ltc4281i", &ltc4281i_adc_ops, &ltc4281i_dev) == RT_EOK)
    {
        LOG_I("init success");
    }
    else
    {
        LOG_E("register failed");
        return -RT_ERROR;
    }

    return RT_EOK;
}

/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_ltc4281i_init);
#endif /* BSP_USING_LTC4281I */
