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

#define DBG_TAG "test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static SelftestUserData selftest_userdata = {
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
        "/mnt/spinor64",
        "/mnt/nor",//TODO:并口数据回环
        "/mnt/emmc",
        "/mnt/udisk/ud0p0"},
    .spi_devname = BSP_DEV_TABLE_SPI2,
    .spi_devname_cs = BSP_GPIO_TABLE_SPI2_CS0,
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
    .result = {
        {"MAX31826", 1},{"DS1682", 1},
        {"GPIO_LOW", 1},{"GPIO_HIGH", 1},
        {"FRAM", 1},{"SPINOR64", 1},{"NOR", 1},{"EMMC", 1},{"UDISK", 1},
        {"SPINOR4", 1},{"EEPROM", 1},
        {"UART2_UART2", 1},{"UART3_UART4", 1},{"UART4_UART3", 1},
        {"CAN1_CAN2", 1},{"CAN2_CAN1", 1},
        {"ETH1_ETH2", 1},{"ETH2_ETH1", 1},
    },
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
    while (rt_tick_get() < 20000)
    {
        rt_thread_delay(10);
    }

    SelftestUserData *puserdata = (SelftestUserData *)parameter;

    while(1)
    {
        LOG_I("startup...");

        //系统信息
        struct SysInfoDef info = {0};
        sysinfo_get(&info);
        if (info.chip_id[0] != 0 && info.chip_id[0] != 0xff)
        {
            LOG_D("max31826    pass");
            puserdata->result[RESULT_MAX31826].result = 0;
        }
        else
        {
            LOG_E("max31826    error!");
            puserdata->result[RESULT_MAX31826].result = 1;
        }
        if (info.times != 0)
        {
            LOG_D("ds1682      pass");
            puserdata->result[RESULT_DS1682].result = 0;
        }
        else
        {
            LOG_E("ds1682      error!");
            puserdata->result[RESULT_DS1682].result = 1;
        }
        //gpio
        selftest_gpio_test(puserdata);
        //filesysterm
        selftest_fs_test(puserdata);
        //key
        selftest_key_test(puserdata);
        //i2s
        rt_tick_t tick = rt_tick_get();
        selftest_i2s_test(puserdata);
        //spi
        selftest_spi_test(puserdata);
        //i2c
        selftest_i2c_test(puserdata);
        //uart
        selftest_uart_test(puserdata);
        //can
        selftest_can_test(puserdata);
        //eth
        selftest_eth_test(puserdata);
        //tcpip
        selftest_tcpip_test(puserdata);

        LOG_I("end.");

        rt_kprintf("\n");
        char SN_str[sizeof(info.SN) + 1] = {0};
        rt_memcpy(SN_str, info.SN, sizeof(info.SN));
        rt_kprintf("--Ans TestResult T:M4H7 SN:%s R=", SN_str);

        for (int i = 0; i < sizeof(puserdata->result)/sizeof(puserdata->result[0]); i++)
        {
            rt_kprintf("%s:%02d", puserdata->result[i].name, puserdata->result[i].result);
            if (i == sizeof(puserdata->result)/sizeof(puserdata->result[0]) - 1)
            {
                rt_kprintf("\n");
            }
            else
            {
                rt_kprintf(";");
            }
        }

        rt_thread_delay_until(&tick, 6000);
    }
}

static int selftest_init(void)
{
    //启动自测任务
    rt_thread_t thread = rt_thread_create( "selftest",
                                            selftest_thread_entry,
                                            &selftest_userdata,
                                            4096,
                                            24,
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
