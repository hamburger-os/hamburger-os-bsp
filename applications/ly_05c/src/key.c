
/*******************************************************
 *
 * @FileName: key.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 按键模块的实现.
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

#include "event.h"
#include "voice.h"
#include "log.h"
#include "key.h"

/*******************************************************
 * 宏定义
 *******************************************************/

#define CREC0_KEY "PC.4" /* 声控+电控, 低电平有效 */
#define CREC1_KEY "PA.9" /* 电控信号,低电平有效 */
#define PLAY_KEY "PH.7"  /* 播放控制引脚,低电平有效 */

/* 按键队列消息 */
#define KEY_CREC0_MSG 0 /* 声控+电控按键消息 */
#define KEY_CREC1_MSG 1 /* 电控按键消息 */
#define KEY_PLAY_MSG 2  /* 播放按键消息 */

/*******************************************************
 * 全局变量
 *******************************************************/

/* 按键引脚句柄 */
static sint32_t g_key_crec0_pin = 0;
static sint32_t g_key_crec1_pin = 0;
static sint32_t g_key_play_pin = 0;

/* 按键引脚的当前状态 */
static sint32_t g_key_crec0_state = 0;
static sint32_t g_key_crec1_state = 0;
static sint32_t g_key_play_state = 0;

/* 按键延迟处理线程 */
static rt_mq_t key_mq = NULL;

/*******************************************************
 * 函数实现
 *******************************************************/

/* KEY延时处理 */
static void *key_thread(void *args);
/* CREC0引脚电平变化处理函数 */
static void key_crec0_irq(const void *args);
/* CREC1引脚电平变化处理函数 */
static void key_crec1_irq(const void *args);
/* play引脚电平变化处理函数 */
static void key_play_irq(const void *args);

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  KEY延时处理
 *
 * @param  args: 线程输入参数
 * @retval void * 线程结束数据
 *
 *******************************************************/

static void *key_thread(void *args)
{
    rt_err_t ret = 0;
    sint32_t key_val = 0;
    uint8_t msg = 0;

    /* 创建消息队列 */
    key_mq = rt_mq_create("key_mq", sizeof(uint8_t), (rt_size_t)1, (rt_uint8_t)RT_IPC_FLAG_FIFO);
    if (key_mq == NULL)
    {
        return NULL;
    }
    else
    {
    }

    while (true)
    {
        /* 从消息队列中接收消息, 阻塞方式 */
        ret = rt_mq_recv(key_mq, (void *)&msg, sizeof(uint8_t), (rt_int32_t)RT_WAITING_FOREVER);
        if (ret < 0)
        {
            continue;
        }
        else
        {
            switch ((sint32_t)msg)
            {
            case KEY_CREC0_MSG:
                /* 读取key的值 */
                key_val = rt_pin_read((rt_base_t)g_key_crec0_pin);
                msleep((uint32_t)10);
                if ((key_val == rt_pin_read((rt_base_t)g_key_crec0_pin)) && (g_key_crec0_state != key_val))
                {
                    /* rt_kprintf("Key_CREC0 value: %d\n", key_val); */
                    /* 按键电平确实发生变化 */
                    if (key_val == 0) /* 开始录音 */
                    {
                        record_start_record();
                    }
                    else /* 停止录音 */
                    {
                        record_stop_record();
                    }
                    /* 更新按键状态 */
                    g_key_crec0_state = key_val;
                }
                else
                {
                    /* 抖动,不做任何处理 */
                }

                break;
            case KEY_CREC1_MSG:
                /* 读取key的值 */
                key_val = rt_pin_read((rt_base_t)g_key_crec1_pin);
                msleep((uint32_t)10);
                if ((key_val == rt_pin_read((rt_base_t)g_key_crec1_pin)) && (g_key_crec1_state != key_val))
                {
                    /* rt_kprintf("Key_CREC1 value: %d\n", key_val); */
                    /* 按键电平确实发生变化 */
                    if (key_val == 0) /* 开始录音 */
                    {
                        record_start_record();
                    }
                    else /* 停止录音 */
                    {
                        record_stop_record();
                    }
                    /* 更新按键状态 */
                    g_key_crec1_state = key_val;
                }
                else
                {
                    /* 抖动,不做任何处理 */
                }
                break;
            case KEY_PLAY_MSG:
                /* 读取key的值 */
                key_val = rt_pin_read((rt_base_t)g_key_play_pin);
                msleep((uint32_t)10);
                if ((key_val == rt_pin_read((rt_base_t)g_key_play_pin)) && (g_key_play_state != key_val))
                {
                    /* rt_kprintf("KeyPlayPin value: %d\n", key_val); */
                    /* 按键电平确实发生变化 */
                    if (key_val == 0) /* 播放按钮被按下 */
                    {
                        /* 回放最新一条语音 */
                        event_push_queue(EVENT_PLAY_LAST);
                    }
                    else
                    {
                        /* 不做任何处理 */
                    }
                    /* 更新按键状态 */
                    g_key_play_state = key_val;
                }
                else
                {
                    /* 抖动,不做任何处理 */
                }
                break;
            default: /* 缺省 */
                break;
            }
        }
    }
    return NULL;
}
/*******************************************************
 *
 * @brief  CREC0引脚电平变化处理函数
 *
 * @param  args: 引脚CREC0的中断处理函数
 * @retval none
 *
 *******************************************************/

static void key_crec0_irq(const void *args)
{
    uint8_t msg = KEY_CREC0_MSG;
    if (key_mq == NULL)
    {
        return;
    }
    else
    {
        /* 表明有电平变化发生 */
        rt_mq_send(key_mq, (void *)&msg, sizeof(msg));
    }
}
/*******************************************************
 *
 * @brief  CREC1引脚电平变化处理函数
 *
 * @param  args: 引脚CREC1的中断处理函数
 * @retval none
 *
 *******************************************************/

static void key_crec1_irq(const void *args)
{
    uint8_t msg = KEY_CREC1_MSG;
    if (key_mq == NULL)
    {
        return;
    }
    else
    {

        /* 表明有电平变化发生 */
        rt_mq_send(key_mq, (void *)&msg, sizeof(msg));
    }
}

/*******************************************************
 *
 * @brief  play引脚电平变化处理函数
 *
 * @param  args: 引脚play的中断处理函数
 * @retval none
 *
 *******************************************************/

static void key_play_irq(const void *args)
{
    uint8_t msg = KEY_PLAY_MSG;
    if (key_mq == NULL)
    {
        return;
    }
    else
    {

        /* 表明有电平变化发生 */
        rt_mq_send(key_mq, (void *)&msg, sizeof(msg));
    }
}

/*******************************************************
 *
 * @brief  初始化key模块
 *
 * @param  none
 * @retval sint32_t 0:成功 -1:失败
 *
 *******************************************************/
sint32_t key_init(void)
{
    sint32_t ret = 0;
    pthread_t key_tid;
    pthread_attr_t pthread_attr_data;

    /* 初始化按键 */
    g_key_crec0_pin = (sint32_t)rt_pin_get(CREC0_KEY);
    g_key_crec1_pin = (sint32_t)rt_pin_get(CREC1_KEY);
    g_key_play_pin = (sint32_t)rt_pin_get(PLAY_KEY);

    /* 设置为输入模式 */
    rt_pin_mode((rt_base_t)g_key_crec0_pin, (rt_base_t)PIN_MODE_INPUT);
    rt_pin_mode((rt_base_t)g_key_crec1_pin, (rt_base_t)PIN_MODE_INPUT);
    rt_pin_mode((rt_base_t)g_key_play_pin, (rt_base_t)PIN_MODE_INPUT);

    /* 绑定中断,边沿模式 */
    rt_pin_attach_irq(g_key_crec0_pin, (rt_uint32_t)PIN_IRQ_MODE_RISING_FALLING, key_crec0_irq, NULL);
    rt_pin_attach_irq(g_key_crec1_pin, (rt_uint32_t)PIN_IRQ_MODE_RISING_FALLING, key_crec1_irq, NULL);
    rt_pin_attach_irq(g_key_play_pin, (rt_uint32_t)PIN_IRQ_MODE_RISING_FALLING, key_play_irq, NULL);

    /* 使能中断 */
    rt_pin_irq_enable((rt_base_t)g_key_crec0_pin, (rt_uint32_t)PIN_IRQ_ENABLE);
    rt_pin_irq_enable((rt_base_t)g_key_crec1_pin, (rt_uint32_t)PIN_IRQ_ENABLE);
    rt_pin_irq_enable((rt_base_t)g_key_play_pin, (rt_uint32_t)PIN_IRQ_ENABLE);

    /* 更新引脚的默认状态 */
    g_key_crec0_state = rt_pin_read((rt_base_t)g_key_crec0_pin);
    g_key_crec1_state = rt_pin_read((rt_base_t)g_key_crec1_pin);
    g_key_play_state = rt_pin_read((rt_base_t)g_key_play_pin);

    /* 创建key抖动处理线程 */
    pthread_attr_init(&pthread_attr_data);
    pthread_attr_data.stacksize = 1024 * 1;
    pthread_attr_data.schedparam.sched_priority = 19;
    ret = pthread_create(&key_tid, &pthread_attr_data, key_thread, NULL);
    pthread_attr_destroy(&pthread_attr_data);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "create key thread error, ret=%d.\n", ret);
        return (sint32_t)-1;
    }
    else
    {
        return (sint32_t)0;
    }
}
