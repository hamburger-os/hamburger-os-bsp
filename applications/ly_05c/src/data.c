/*******************************************************
 *
 * @FileName: data.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 数据管理模块.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include "data.h"

/*******************************************************
 * 全局变量
 *******************************************************/

/* 工作状态数据 */
static work_state_t g_work_state;

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  设置USB状态
 *
 * @param  *usb_state: usb的状态
 * @retval none
 *
 *******************************************************/
void data_set_usb_state(E_USB_STATE state)
{
    pthread_mutex_lock(&g_work_state.mutex);
    g_work_state.usb_state = state;
    pthread_mutex_unlock(&g_work_state.mutex);
}

/*******************************************************
 *
 * @brief  获取USB状态
 *
 * @retval E_USB_STATE usb的状态
 *
 *******************************************************/
E_USB_STATE data_get_usb_state(void)
{
    E_USB_STATE ret;

    pthread_mutex_lock(&g_work_state.mutex);
    ret = g_work_state.usb_state;
    pthread_mutex_unlock(&g_work_state.mutex);

    return ret;
}
/*******************************************************
 *
 * @brief  设置转储状态
 *
 * @param  *dump_state: 转储状态
 * @retval none
 *
 *******************************************************/

void data_set_dump_state(E_DUMP_STATE state)
{
    pthread_mutex_lock(&g_work_state.mutex);
    g_work_state.dump_state = state;
    pthread_mutex_unlock(&g_work_state.mutex);
}

/*******************************************************
 *
 * @brief  获取转储状态
 *
 * @retval E_DUMP_STATE 转储状态
 *
 *******************************************************/
E_DUMP_STATE data_get_dump_state(void)
{
    E_DUMP_STATE ret;

    pthread_mutex_lock(&g_work_state.mutex);
    ret = g_work_state.dump_state;
    pthread_mutex_unlock(&g_work_state.mutex);
    return ret;
}

/*******************************************************
 *
 * @brief  设置播放状态
 *
 * @param  pcm_state: pcm设备状态
 * @retval none
 *
 *******************************************************/

void data_set_pcm_state(E_PCM_STATE state)
{
    pthread_mutex_lock(&g_work_state.mutex);
    g_work_state.pcm_state = state;
    pthread_mutex_unlock(&g_work_state.mutex);
}

/*******************************************************
 *
 * @brief  获取播放状态
 *
 * @retval E_PCM_STATE 播放状态
 *
 *******************************************************/

E_PCM_STATE data_get_pcm_state(void)
{
    E_PCM_STATE ret;

    pthread_mutex_lock(&g_work_state.mutex);
    ret = g_work_state.pcm_state;
    pthread_mutex_unlock(&g_work_state.mutex);

    return ret;
}

/*******************************************************
 *
 * @brief  设置tax通信状态
 *
 * @param  state: tax通信状态
 * @retval none
 *
 *******************************************************/

void data_set_tax_comm_state(E_TAX_STATE state)
{
    pthread_mutex_lock(&g_work_state.mutex);
    g_work_state.tax_comm_state = state;
    pthread_mutex_unlock(&g_work_state.mutex);
}

/*******************************************************
 *
 * @brief  获取TAX的通信状态
 *
 * @retval E_TAX_STATE tax的通信状态
 *
 *******************************************************/

E_TAX_STATE data_get_tax_comm_state(void)
{
    E_TAX_STATE ret;

    pthread_mutex_lock(&g_work_state.mutex);
    ret = g_work_state.tax_comm_state;
    pthread_mutex_unlock(&g_work_state.mutex);

    return ret;
}

/*******************************************************
 *
 * @brief  数据模块初始化
 *
 * @param  list: 提示音播放列表的数据指针
 * @retval sint32_t 0:成功 -1:失败
 *
 *******************************************************/
sint32_t data_init(void)
{
    g_work_state.pcm_state = PCM_STATE_IDLE;
    pthread_mutex_init(&g_work_state.mutex, NULL);
    return 0;
}
