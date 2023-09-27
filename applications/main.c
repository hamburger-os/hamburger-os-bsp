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
static uint16_t sys_led_delay = BSP_SYS_LED_DELAY;
#endif

int main(void)
{
#ifdef BSP_USING_SYS_LED
    /* set LED pin mode to output */
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);
#endif

#ifndef PKG_USING_QBOOT
    rt_kprintf("Type 'help' to get the list of commands.\n");
    rt_kprintf("Use UP/DOWN arrows to navigate through command history.\n");
    rt_kprintf("Press TAB when typing command name to auto-complete.\n");
#endif

#ifdef BSP_USING_SYS_LED
    /* low thread priority */
    uint8_t parity = RT_THREAD_PRIORITY_MAX - 2;
    rt_thread_control(rt_thread_self(), RT_THREAD_CTRL_CHANGE_PRIORITY, &parity);

    while (1)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);
        rt_thread_mdelay(sys_led_delay);
        rt_pin_write(LED_PIN, PIN_LOW);
        rt_thread_mdelay(sys_led_delay);
    }
#endif

    return RT_EOK;
}

/** \brief change sys led delay
 * \return void
 *
 */
#ifdef BSP_USING_SYS_LED
static void change_sys_led(int argc, char *argv[])
{
    if (argc != 2)
    {
        rt_kprintf("Usage: change_sys_led [ms]\n");
    }
    else
    {
        sys_led_delay = strtoul(argv[1], NULL, 10);
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(change_sys_led, change_sys_led, change sys led delay.);
#endif
#endif
