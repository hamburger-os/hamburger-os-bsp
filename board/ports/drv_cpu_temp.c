/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-06-16     lvhan       the first version
 */
#include "board.h"

#include "sensor.h"

#define DBG_TAG "cputemp"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static rt_adc_device_t      adc_dev;

/* RT-Thread Device Driver Interface */
static rt_size_t cpu_temp_polling_get_data(struct rt_sensor_device *sensor, struct rt_sensor_data *data)
{
    float temperature = 0;
    int32_t temperature_x100 = 0;
    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        /* 使能设备 */
        rt_adc_enable(adc_dev, CPUTEMP_ADC_DEV_CHANNEL);
        /* 读取采样值 */
        temperature_x100 = rt_adc_read(adc_dev, CPUTEMP_ADC_DEV_CHANNEL);
        /* 关闭通道 */
        rt_adc_disable(adc_dev, CPUTEMP_ADC_DEV_CHANNEL);
        LOG_D("value  : %d", temperature_x100);
#ifdef SOC_SERIES_STM32F4
        temperature = temperature_x100 * 3.3f / 4096; //电压值
#endif
#ifdef SOC_SERIES_STM32H7
        temperature = temperature_x100 * 3.3f / 65536; //电压值
#endif
        LOG_D("volt   : %d.%02d v", (int32_t)temperature, abs((int32_t)((temperature - (int32_t)temperature) * 100)));
#ifdef SOC_SERIES_STM32F4
        temperature = (temperature - 0.76f) / 0.0025f + 25; //转换为温度值
#endif
#ifdef SOC_SERIES_STM32H7
        uint16_t TS_CAL1 = *(__IO uint16_t *)(0x1FF1E820);
        uint16_t TS_CAL2 = *(__IO uint16_t *)(0x1FF1E840);
        temperature = (110.0f - 30.0f) * (temperature_x100 - TS_CAL1)/ (TS_CAL2 - TS_CAL1) + 30;//转换为温度值
#endif
        LOG_D("temp   : %d.%02d ℃", (int32_t)temperature, abs((int32_t)((temperature - (int32_t)temperature) * 100)));
        temperature_x100 = temperature * 100;

        LOG_D("temp10 : %d", temperature_x100);
        data->data.temp = temperature_x100;
        data->timestamp = rt_tick_get_millisecond();
    }
    return 1;
}

static rt_size_t cpu_temp_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return cpu_temp_polling_get_data(sensor, buf);
    }
    else
        return 0;
}

static struct rt_sensor_ops sensor_ops =
{
    cpu_temp_fetch_data,
    NULL,
};

static int rt_hw_cpu_temp_init()
{
    rt_int8_t result;
    rt_sensor_t sensor_temp = RT_NULL;

    /* temperature sensor register */
    sensor_temp = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_temp == RT_NULL)
        return -RT_ERROR;

    sensor_temp->info.type       = RT_SENSOR_CLASS_TEMP;
    sensor_temp->info.vendor     = RT_SENSOR_VENDOR_MAXIM;
    sensor_temp->info.model      = "cpu";
    sensor_temp->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
    sensor_temp->info.intf_type  = RT_SENSOR_INTF_ONEWIRE;
    sensor_temp->info.range_max  = 125;
    sensor_temp->info.range_min  = -40;
    sensor_temp->info.period_min = 150;

    sensor_temp->ops = &sensor_ops;

    adc_dev = (rt_adc_device_t)rt_device_find(CPUTEMP_ADC_DEV);
    if (adc_dev == RT_NULL)
    {
        LOG_E("can't find %s device!", CPUTEMP_ADC_DEV);
        goto __exit;
    }

    result = rt_hw_sensor_register(sensor_temp, "cpu", RT_DEVICE_FLAG_RDONLY, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    /* 使能设备 */
    rt_adc_enable(adc_dev, CPUTEMP_ADC_DEV_CHANNEL);
    /* 读取采样值 */
    rt_adc_read(adc_dev, CPUTEMP_ADC_DEV_CHANNEL);
    /* 关闭通道 */
    rt_adc_disable(adc_dev, CPUTEMP_ADC_DEV_CHANNEL);

    return RT_EOK;

__exit:
    rt_free(sensor_temp);
    return -RT_ERROR;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_hw_cpu_temp_init);
