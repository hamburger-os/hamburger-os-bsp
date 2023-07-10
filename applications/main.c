/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#ifdef BSP_USING_SYS_LED
/* defined the LED pin */
#define LED_PIN    rt_pin_get(BSP_SYS_LED_GPIO)
#endif

int main(void)
{
    /* Lower thread priority */
    uint8_t parity = RT_THREAD_PRIORITY_MAX - 2;
    rt_thread_control(rt_thread_self(), RT_THREAD_CTRL_CHANGE_PRIORITY, &parity);

#ifdef BSP_USING_SYS_LED
    /* set LED pin mode to output */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
#endif

#ifndef PKG_USING_QBOOT
    LOG_D("system self-test completed and started.");
#endif
    while (1)
    {
#ifdef BSP_USING_SYS_LED
        rt_pin_write(LED_PIN, PIN_HIGH);
#endif
        rt_thread_mdelay(BSP_SYS_LED_DELAY);
#ifdef BSP_USING_SYS_LED
        rt_pin_write(LED_PIN, PIN_LOW);
#endif
        rt_thread_mdelay(BSP_SYS_LED_DELAY);
    }

    return RT_EOK;
}
