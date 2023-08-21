/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-31     lvhan       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>

#include "selftest.h"
#include "sysinfo.h"

#define DBG_TAG "selftest"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

SelftestlUserData selftest_userdata = {
    .gpio_devname = {
        {BSP_GPIO_TABLE_PWM1        , BSP_GPIO_TABLE_GPIO1      },
        {BSP_GPIO_TABLE_GPIO5       , BSP_GPIO_TABLE_GPIO6      },
        {BSP_GPIO_TABLE_GPIO8       , BSP_GPIO_TABLE_SPI2_CS1   },
        {BSP_GPIO_TABLE_GPIO4       , BSP_GPIO_TABLE_GPIO7      },
        {BSP_GPIO_TABLE_SPI1_CS1    , BSP_GPIO_TABLE_SPI1_CS2   },
        {BSP_GPIO_TABLE_PWM2        , BSP_GPIO_TABLE_GPIO2      }},
    .key_devname = BSP_GPIO_TABLE_PWM3,
    .fs_path = {
        "/mnt/fram",
        "/mnt/spiflash",
        "/mnt/at45db321e",
        "/mnt/nor",
        "/mnt/emmc",
        "/mnt/udisk/ud0p0"},
    .i2c_devname = "eeprom",
    .wav_path = "/usr/5s_8000_2ch.wav",
    .uart_devname = {
        {BSP_DEV_TABLE_UART2        , BSP_DEV_TABLE_UART2       },
        {BSP_DEV_TABLE_UART3        , BSP_DEV_TABLE_UART4       },
        {BSP_DEV_TABLE_UART4        , BSP_DEV_TABLE_UART3       }},
    .can_devname = {
        {BSP_DEV_TABLE_CAN1         , BSP_DEV_TABLE_CAN2        },
        {BSP_DEV_TABLE_CAN2         , BSP_DEV_TABLE_CAN1        }},
    .eth_devname = {
        {"e0"                       , "e1"                      },
        {"e1"                       , "e0"                      }},
};

#if 0 //运行Gui-Guider创建的app

#include <lvgl.h>
#include "gui_guider.h"
#include "custom.h"
#include "widgets_init.h"

lv_ui guider_ui;

void lv_user_gui_init(void)
{
    /*Create a GUI-Guider app */
    setup_ui(&guider_ui);
    custom_init(&guider_ui);
}
#endif

static void selftest_thread_entry(void* parameter)
{
    SelftestlUserData *puserdata = (SelftestlUserData *)parameter;
    rt_thread_delay(4000);

    LOG_I("startup...");

    //系统信息
    sysinfo_show();
    //gpio
    selftest_gpio_test(puserdata);
    //filesysterm
    selftest_fs_test(puserdata);
    //i2c
    selftest_i2c_test(puserdata);
    //i2s
    selftest_i2s_test(puserdata);
    //uart
    selftest_uart_test(puserdata);
    //can
    selftest_can_test(puserdata);
    //eth
    selftest_eth_test(puserdata);

    LOG_I("end.");
    while(1)
    {
        rt_thread_delay(1000);
    }
}

static int selftest_init(void)
{
    //启动自测任务
    rt_thread_t thread = rt_thread_create( "selftest",
                                            selftest_thread_entry,
                                            &selftest_userdata,
                                            4096,
                                            RT_THREAD_PRIORITY_MAX-2,
                                            10);
    if ( thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("creat thread error!");

        return -RT_ERROR;
    }
    return RT_EOK;
}

#if 0 //如果使用dl动态模块方式
int main(int argc, char *argv[])
{
    selftest_init();

    while(1)
    {
        rt_thread_delay(1000);
    }
    return 0;
}
#else
INIT_APP_EXPORT(selftest_init);
#endif
