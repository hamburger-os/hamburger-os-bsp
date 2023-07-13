/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-11     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_LED

#define DBG_TAG "drv.led"
#define DBG_LVL DBG_LOG
#include <ulog.h>

struct LEDConfig
{
    rt_base_t pin;
    rt_base_t level;
    rt_uint8_t count;
    rt_uint8_t is_error;
};
static struct LEDConfig leds_config[BSP_LED_MAX] = {0};

/* 创建一个led设备 */
rt_base_t led_creat(rt_base_t pin, rt_base_t level)
{
    uint8_t i;
    for (i = 0; i<BSP_LED_MAX; i++)
    {
        if (leds_config[i].pin == 0)
        {
            leds_config[i].pin = pin;
            leds_config[i].level = level;
            rt_pin_mode(pin, PIN_MODE_OUTPUT);
            rt_pin_write(leds_config[i].pin, !leds_config[i].level);
            return i;
        }
    }
    LOG_E("creation failed, exceeding the supported quantity.");
    return -1;
}

/* 在收发数据或者idle时调用此函数即会产生对应的led运行状态 */
void led_execution_phase(rt_base_t index)
{
    if (index < BSP_LED_MAX && BSP_LED_MAX >= 0)
    {
        leds_config[index].is_error = 0;
        leds_config[index].count = 1000/BSP_LED_CYCLE;
        if (leds_config[index].count == 0)
            leds_config[index].count = 1;
    }
}

/* 在发生错误时调用此函数 */
void led_error_handler(rt_base_t index)
{
    if (index < BSP_LED_MAX && BSP_LED_MAX >= 0)
    {
        leds_config[index].is_error = 1;
    }
}

/* 线程的入口函数 */
static void led_thread_entry(void *parameter)
{
    uint8_t i = 0;
    rt_tick_t tick = rt_tick_get();

    LOG_I("init succeed.");
    while (1)
    {
        rt_thread_delay_until(&tick, BSP_LED_CYCLE);
        tick = rt_tick_get();

        for (i = 0; i<BSP_LED_MAX; i++)
        {
            if (leds_config[i].pin != 0)
            {
                if (leds_config[i].is_error == 1)
                {
                    rt_pin_write(leds_config[i].pin, leds_config[i].level);
                }
                else if (leds_config[i].count > 0)
                {
                    rt_pin_write(leds_config[i].pin, !rt_pin_read(leds_config[i].pin));
                    leds_config[i].count --;
                }
                else
                {
                    rt_pin_write(leds_config[i].pin, !leds_config[i].level);
                }
            }
        }
    }
}

#ifdef BSP_USING_LED_RUN
static rt_base_t len_run_index = 0;

static void led_run_hook(void)
{
    led_execution_phase(len_run_index);
}
#endif

static int rt_led_init()
{
#ifdef BSP_USING_LED_RUN
    len_run_index = led_creat(rt_pin_get(LED_RUN_GPIO), LED_RUN_LEVEL);
    rt_thread_idle_sethook(led_run_hook);
#endif
    /* 创建线程，名称是 thread，入口是 thread_entry */
    rt_thread_t tid = rt_thread_create("led",
                            led_thread_entry, RT_NULL,
                            1024,
                            RT_THREAD_PRIORITY_MAX - 3, 20);

    /* 如果获得线程控制块，启动这个线程 */
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("init failed.");
    }
    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_led_init);

#endif
