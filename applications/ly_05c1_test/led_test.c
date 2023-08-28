
/*******************************************************
 *
 * @FileName: ly_05c.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: ly-05c产品的主程序实现.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>

/*******************************************************
 *
 * @brief  LY-05C录音板主线程函数
 *
 * @retval 0:成功 <0:失败
 *
 *******************************************************/

/*******************************************************
 *
 * @FileName: led.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: led模块测试程序
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdbool.h>
#include <pthread.h>
#include <delay.h>

/*******************************************************
 * 宏定义
 *******************************************************/

/* LED最大数量 */
#define LED_MAX_NUM 9

/* LED设备序号 */
#define LED_PIN_INDEX_1A 0
#define LED_PIN_INDEX_2A 1
#define LED_PIN_INDEX_3A 2
#define LED_PIN_INDEX_4A 3
#define LED_PIN_INDEX_5A 4
#define LED_PIN_INDEX_1B 5
#define LED_PIN_INDEX_3B 6
#define LED_PIN_INDEX_4B 7
#define LED_PIN_INDEX_5B 8
#define LED_MAX_NUM 9

/* LED设备名字 */
#define LED_PIN_NAME_1A BSP_GPIO_TABLE_SPI1_CS1
#define LED_PIN_NAME_2A BSP_GPIO_TABLE_SPI2_SCK
#define LED_PIN_NAME_3A BSP_GPIO_TABLE_SPI2_CS0
#define LED_PIN_NAME_4A BSP_GPIO_TABLE_SPI1_MISO
#define LED_PIN_NAME_5A BSP_GPIO_TABLE_SPI1_MOSI
#define LED_PIN_NAME_1B BSP_GPIO_TABLE_SPI2_MOSI
#define LED_PIN_NAME_3B BSP_GPIO_TABLE_SPI2_MISO
#define LED_PIN_NAME_4B BSP_GPIO_TABLE_SPI1_CS0
#define LED_PIN_NAME_5B BSP_GPIO_TABLE_SPI1_SCK

/* 电平状态 */
#define LED_ON ((bool)PIN_LOW)
#define LED_OFF ((bool)PIN_HIGH)

/*******************************************************
 * 数据结构
 *******************************************************/

/* LED描述数据结构 */
typedef struct
{
    char *ptr_name; /* 引脚名 */
    rt_base_t pin;  /* 引脚引脚编号 */
} led_desc_t;

static rt_thread_t led_test_thread = RT_NULL;

/* LED的结构体数组 */
static led_desc_t led_descs[LED_MAX_NUM] = {
    {
        .ptr_name = LED_PIN_NAME_1A,
    },
    {
        .ptr_name = LED_PIN_NAME_2A,
    },
    {
        .ptr_name = LED_PIN_NAME_3A,
    },
    {
        .ptr_name = LED_PIN_NAME_4A,
    },
    {
        .ptr_name = LED_PIN_NAME_5A,
    },
    {
        .ptr_name = LED_PIN_NAME_1B,
    },
    {
        .ptr_name = LED_PIN_NAME_3B,
    },
    {
        .ptr_name = LED_PIN_NAME_4B,
    },
    {
        .ptr_name = LED_PIN_NAME_5B,
    },
};
/*******************************************************
 *
 * @brief  LED初始化
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
static void led_init(void)
{
    size_t i = 0;
    /* 初始化LED状态 */
    for (i = 0; i < LED_MAX_NUM; i++)
    {
        led_descs[i].pin = rt_pin_get((const char *)led_descs[i].ptr_name);
        rt_pin_mode(led_descs[i].pin, (rt_base_t)PIN_MODE_OUTPUT);
    }
}
/*******************************************************
 *
 * @brief  LED轮流点亮
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
static void led_alter_on(void)
{
    size_t i = 0;
    for (i = 0; i < LED_MAX_NUM; i++)
    {
        if (i != 0)
            rt_pin_write(led_descs[i - 1].pin, LED_OFF);
        rt_pin_write(led_descs[i].pin, LED_ON);
        msleep(200);
    }
}
/*******************************************************
 *
 * @brief  LED全部灭掉
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
static void led_all_off(void)
{
    size_t i = 0;
    for (i = 0; i < LED_MAX_NUM; i++)
    {
        rt_pin_write(led_descs[i].pin, LED_OFF);
    }
}
/*******************************************************
 *
 * @brief  LED测试线程
 *
 * @param  args 参数
 * @retval none
 *
 *******************************************************/
static void *led_thread(const void *args)
{
    led_init();
    led_all_off(); /* 全灭 */
    msleep(200);
    led_alter_on(); /* 交替闪烁 */
    msleep(200);
    led_all_off(); /* 全灭 */
    led_test_thread = RT_NULL;
    return NULL;
}
/*******************************************************
 *
 * @brief  LED测试程序
 *
 * @param  args 参数
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
int ly_05c1_led_test(void)
{
    if (led_test_thread != RT_NULL)
    {
        rt_kprintf("led test thread already exists!\n");
        return -1;
    }
    /* 创建TAX线程 */
    led_test_thread =
        rt_thread_create("led_test",
                         led_thread,
                         NULL,
                         5 * 1024,
                         23,
                         10);
    if (led_test_thread != RT_NULL)
    {
        rt_thread_startup(led_test_thread);
    }
    return 0;
}

MSH_CMD_EXPORT_ALIAS(ly_05c1_led_test, test_led, ly - 05c led test.);
