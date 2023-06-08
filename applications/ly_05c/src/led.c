/*******************************************************
 *
 * @FileName: led.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: led模块的实现
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include "led.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <type.h>
#include <stdbool.h>
#include <pthread.h>
#include <delay.h>

#include "config.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/*LED最大数量 */
#define LED_MAX_NUM 9

/*LED设备序号 */
#define LED_PIN_INDEX_1A 0
#define LED_PIN_INDEX_2A 1
#define LED_PIN_INDEX_3A 2
#define LED_PIN_INDEX_4A 3
#define LED_PIN_INDEX_5A 4
#define LED_PIN_INDEX_1B 5
#define LED_PIN_INDEX_3B 6
#define LED_PIN_INDEX_4B 7
#define LED_PIN_INDEX_5B 8

/*LED设备名字 */
#define LED_PIN_NAME_1A "PA.10"
#define LED_PIN_NAME_2A "PB.3"
#define LED_PIN_NAME_3A "PA.15"
#define LED_PIN_NAME_4A "PA.6"
#define LED_PIN_NAME_5A "PA.7"
#define LED_PIN_NAME_1B "PB.0"
#define LED_PIN_NAME_3B "PB.4"
#define LED_PIN_NAME_4B "PA.4"
#define LED_PIN_NAME_5B "PA.5"

/*电平状态 */
#define LED_ON ((bool)PIN_LOW)
#define LED_OFF ((bool)PIN_HIGH)

/*******************************************************
 * 数据结构
 *******************************************************/

/*LED状态 */
typedef enum
{
    LedPinStateOn = 0,          /*亮 */
    LedPinStateOff = 1,         /*灭 */
    LedPinStateBlink = 2,       /*闪烁 */
    LedPinStateAlterBlinkP = 3, /*正向交替闪烁 */
    LedPinStateAlterBlinkN = 4  /*负向交替闪烁 */
} E_LedPinState;

/*LED描述数据结构 */
typedef struct _LedDesc
{
    char *name;              /*引脚名 */
    rt_base_t pin;           /*引脚引脚编号 */
    E_LedPinState pin_state; /*引脚状态 */
    uint16_t period;         /*闪烁周期 */
    bool old_state;          /*老的引脚状态 */
} led_desc_t;

/*LED的结构体数组 */
static led_desc_t led_descs[LED_MAX_NUM] = {
    {.name = LED_PIN_NAME_1A},
    {.name = LED_PIN_NAME_2A},
    {.name = LED_PIN_NAME_3A},
    {.name = LED_PIN_NAME_4A},
    {.name = LED_PIN_NAME_5A},
    {.name = LED_PIN_NAME_1B},
    {.name = LED_PIN_NAME_3B},
    {.name = LED_PIN_NAME_4B},
    {.name = LED_PIN_NAME_5B},
};

/*******************************************************
 * 头文件
 *******************************************************/

/*保护锁, 用于保护 led_descs */
static pthread_mutex_t g_led_mutex;

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  设置一个引脚的状态
 *
 * @param  index: led序号
 * @param  led_pin_state: led引脚状态
 * @retval none
 *
 *******************************************************/

static int set_pin_state(uint16_t index, E_LedPinState led_pin_state)
{
    if (index >= LED_MAX_NUM)
        return -1;

    pthread_mutex_lock(&g_led_mutex);
    led_descs[index].pin_state = led_pin_state;
    pthread_mutex_unlock(&g_led_mutex);
    return 0;
}

/*******************************************************
 *
 * @brief  设置LED灯状态
 *
 * @param  led_state: led灯的状态
 * @retval none
 *
 *******************************************************/

void led_set(E_LED_State led_state)
{
    switch (led_state)
    {
    case LED_STATE_RS485_NORMAL: /* 485通信正常 */
        set_pin_state(LED_PIN_INDEX_1A, LedPinStateOff);
        set_pin_state(LED_PIN_INDEX_1B, LedPinStateBlink);
        set_pin_state(LED_PIN_INDEX_2A, LedPinStateOff);
        break;
    case LED_STATE_RS485_ERROR: /* 未收到正确的485数据 */
        set_pin_state(LED_PIN_INDEX_1A, LedPinStateBlink);
        set_pin_state(LED_PIN_INDEX_1B, LedPinStateBlink);
        set_pin_state(LED_PIN_INDEX_2A, LedPinStateBlink);
        break;
    case LED_STATE_RECORDING: /* 正在录音 */
        set_pin_state(LED_PIN_INDEX_1A, LedPinStateOn);
        set_pin_state(LED_PIN_INDEX_1B, LedPinStateBlink);
        set_pin_state(LED_PIN_INDEX_2A, LedPinStateOff);
        break;
    case LED_STATE_PLAYING: /* 正在放音 */
        set_pin_state(LED_PIN_INDEX_1A, LedPinStateAlterBlinkP);
        set_pin_state(LED_PIN_INDEX_2A, LedPinStateAlterBlinkN);
        set_pin_state(LED_PIN_INDEX_1B, LedPinStateOff);
        break;
    case LED_STATE_DUMPING: /* 正在转储 */
        set_pin_state(LED_PIN_INDEX_3A, LedPinStateBlink);
        break;
    case LED_STATE_DUMP_SUCCESS: /* 转储成功 */
        set_pin_state(LED_PIN_INDEX_3A, LedPinStateOn);
        break;
    case LED_STATE_DUMP_FAIL: /* 转储失败 */
        set_pin_state(LED_PIN_INDEX_3A, LedPinStateOff);
        break;
    case LED_STATE_VOICE_DATA_RECORDING: /* 正在记录语音数据 */
        set_pin_state(LED_PIN_INDEX_3B, LedPinStateBlink);
        break;
    case LED_STATE_VOICE_DATA_NO: /* 没有记录语音数据 */
        set_pin_state(LED_PIN_INDEX_3B, LedPinStateOff);
        break;
    case LED_STATE_WORK_NORMAL: /* 工作正常 */
        set_pin_state(LED_PIN_INDEX_4A, LedPinStateBlink);
        break;
    case LED_STATE_WORK_ERROR: /* 故障指示 */
        set_pin_state(LED_PIN_INDEX_4A, LedPinStateOn);
        break;
    case LED_STATE_USB_PLUGED_IN: /* 插上U盘 */
        set_pin_state(LED_PIN_INDEX_4B, LedPinStateOn);
        break;
    case LED_STATE_USB_NO: /* 拔下U盘 */
        set_pin_state(LED_PIN_INDEX_4B, LedPinStateOff);
        break;
    default:
        break;
    }
}
/*******************************************************
 *
 * @brief  LED显示线程
 *
 * @param  args: led显示线程参数
 * @retval void * 返回数据
 *
 *******************************************************/
static void *led_thread(void *args)
{
    int i = 0;
    static bool led_pin_state_alter_blink_p = false; /*正向闪烁状态 */
    E_LedPinState pin_state;

    while (true)
    {
        for (i = 0; i < LED_MAX_NUM; i++)
        {
            pthread_mutex_lock(&g_led_mutex);
            pin_state = led_descs[i].pin_state;
            pthread_mutex_unlock(&g_led_mutex);

            switch (pin_state)
            {
            case LedPinStateOn:
                if (led_descs[i].old_state != LED_ON)
                {
                    rt_pin_write(led_descs[i].pin, LED_ON);
                    led_descs[i].old_state = LED_ON;
                }
                break;
            case LedPinStateOff:
                if (led_descs[i].old_state != LED_OFF)
                {
                    rt_pin_write(led_descs[i].pin, LED_OFF);
                    led_descs[i].old_state = LED_OFF;
                }
                break;
            case LedPinStateBlink:
                if (led_descs[i].old_state == LED_ON)
                {
                    rt_pin_write(led_descs[i].pin, LED_OFF);
                    led_descs[i].old_state = LED_OFF;
                }
                else
                {
                    rt_pin_write(led_descs[i].pin, LED_ON);
                    led_descs[i].old_state = LED_ON;
                }
                break;
            case LedPinStateAlterBlinkP:
                rt_pin_write(led_descs[i].pin, led_pin_state_alter_blink_p);
                led_descs[i].old_state = led_pin_state_alter_blink_p;
                break;
            case LedPinStateAlterBlinkN:
                rt_pin_write(led_descs[i].pin, !led_pin_state_alter_blink_p);
                led_descs[i].old_state = !led_pin_state_alter_blink_p;

                break;
            default:
                break;
            }
        }
        msleep(200);
        led_pin_state_alter_blink_p = !led_pin_state_alter_blink_p;
    }
    return NULL;
}

/*******************************************************
 *
 * @brief  初始化LED模块
 *
 * @param  args: led显示线程参数
 * @retval int 0:成功 <0:失败
 *
 *******************************************************/

int led_init(void)
{
    int i = 0;
    int ret = 0;
    pthread_t s_LedTid;
    pthread_attr_t pthread_attr_t;

    /*初始化锁 */
    pthread_mutex_init(&g_led_mutex, NULL);
    /*初始化LED状态 */
    for (i = 0; i < LED_MAX_NUM; i++)
    {
        led_descs[i].pin = rt_pin_get(led_descs[i].name);
        led_descs[i].pin_state = LedPinStateOff;
        led_descs[i].period = 500;      /*500ms */
        led_descs[i].old_state = false; /*老的引脚状态 */

        rt_pin_mode(led_descs[i].pin, PIN_MODE_OUTPUT);
    }

    /* 创建LED显示线程, pth02 */
    pthread_attr_init(&pthread_attr_t);
    pthread_attr_t.stacksize = 1024 * 1; /* 0x1400, 5k */
    pthread_attr_t.schedparam.sched_priority = 18;
    ret = pthread_create(&s_LedTid, &pthread_attr_t, (void *)led_thread, NULL);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "pthread_create error.\n ");
        return -1;
    }

    led_set(LED_STATE_RS485_NORMAL);

    return 0;
}
