/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-26     lvhan       the first version
 */

#include "board.h"
#include <ipc/ringbuffer.h>

#define DBG_TAG "lora"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* lora dirver class */
struct lora_driver
{
    rt_device_t     dev;

    char *          uart_name;
    rt_device_t     uart_dev;
    rt_thread_t     uart_thread;
    rt_mq_t         uart_mq;
    uint8_t         buffer[BSP_LORA_BUFFER_SIZE];
    struct rt_ringbuffer * rb;
#ifdef BSP_LORA_USING_POWER_PIN
    rt_base_t       power_pin;
#endif

    rt_mutex_t mutex;

    int isThreadRun;
};
static struct lora_driver lora_dev = {
    .uart_name = BSP_LORA_DEVNAME,
};

static int rt_lora_init(void)
{
    lora_dev.power_pin = rt_pin_get(BSP_LORA_POWER_PIN);

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_DEVICE_EXPORT(rt_lora_init);
