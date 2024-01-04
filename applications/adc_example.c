/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-14     lvhan       the first version
 */
#include "board.h"

#define DBG_TAG "adc_exam"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void adc_test(int argc, char **argv)
{
    if (argc != 3)
    {
        rt_kprintf("Usage: adctest [dev name] [channel]\n");
        rt_kprintf("       example : adctest adc1 1\n");
        rt_kprintf("       example : adctest ltc186x 0\n");
    }
    else
    {
        char *dev_name = argv[1];
        uint8_t channel = strtoul(argv[2], NULL, 10);

        rt_adc_device_t adc_dev;
        rt_uint32_t value = 0;
        rt_uint32_t vol = 0;

        /* 查找设备 */
        adc_dev = (rt_adc_device_t)rt_device_find(dev_name);
        if (adc_dev == RT_NULL)
        {
            LOG_E("can't find %s device!", dev_name);
            return;
        }

        /* 使能设备 */
        rt_adc_enable(adc_dev, channel);

        /* 读取采样值 */
        value = rt_adc_read(adc_dev, channel);
        LOG_D("the value is :%u", value);

        /* 转换为对应电压值 */
        vol = rt_adc_voltage(adc_dev, channel);
        LOG_D("the voltage is :%u", vol);

        /* 关闭通道 */
        rt_adc_disable(adc_dev, channel);
    }
}
#ifdef RT_USING_FINSH
#include <finsh.h>
#ifdef RT_USING_FINSH
    MSH_CMD_EXPORT_ALIAS(adc_test, adctest, adc test);
#endif /* RT_USING_FINSH */
#endif /* RT_USING_FINSH */
