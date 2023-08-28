
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

#include <stdio.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdbool.h>
#include <pthread.h>
#include <delay.h>

#define CREC0_KEY BSP_GPIO_TABLE_SPI2_CS1 /* 声控+电控, 低电平有效 */
#define CREC1_KEY BSP_GPIO_TABLE_GPIO8    /* 电控信号,低电平有效 */
#define PLAY_KEY BSP_GPIO_TABLE_GPIO5     /* 播放控制引脚,低电平有效 */
#define NEW_ALL_PIN BSP_GPIO_TABLE_GPIO3/* 转储最新,全部语音文件按钮 */

static bool udisk_test_runinig_state = false;

/*******************************************************
 *
 * @brief  输出用法
 *
 * @param  file: 文件名
 * @retval none
 *
 *******************************************************/

static void print_usage(void)
{
    rt_kprintf("Usage: \n");
    rt_kprintf("    test_gpio -i <gpio_num>\n");
}
/*******************************************************
 *
 * @brief  读取按键值
 *
 * @param  pin_name: 按键名称
 * @retval none
 *
 *******************************************************/
static void read_key(const char *pin_name)
{
    int pin = 0;
    int pin_state = 0;

    pin = rt_pin_get(pin_name);
    rt_pin_mode((rt_base_t)pin, (rt_base_t)PIN_MODE_INPUT);
    pin_state = rt_pin_read((rt_base_t)pin);

    rt_kprintf("IO_STATE:%d \n", pin_state);
}
/*******************************************************
 *
 * @brief  USB接口测试程序
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
int ly_05c1_key_test(int argc, char *argv[])
{
    if ((argc != 3) || (strcmp(argv[1], "i") != 0))
    {
        print_usage();
        return -1;
    }
    else
    {
        if (strcmp(argv[2], "10") == 0)
        {
            read_key(CREC0_KEY);
        }
        else if (strcmp(argv[2], "11") == 0)
        {
            read_key(CREC1_KEY);
        }
        else if (strcmp(argv[2], "12") == 0)
        {
            read_key(PLAY_KEY);
        }
        else if (strcmp(argv[2], "13") == 0)
        {
            read_key(NEW_ALL_PIN); 
        }
    }

    return RT_EOK;
}

MSH_CMD_EXPORT_ALIAS(ly_05c1_key_test, test_gpio, ly - 05c gpio test.);
