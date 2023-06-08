/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-12-07     lvhan       the first version
 */
#include "board.h"

#ifdef BSP_USING_USBH_STM32
#include "usb_host.h"

#define DBG_TAG "drv.usbh"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static int rt_usbh_stm32_init(void)
{
    MX_USB_HOST_Init();

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_ENV_EXPORT(rt_usbh_stm32_init);

#endif
