/*******************************************************
 *
 * @FileName: data.h
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 数据模块的头文件.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef _DATA_H_
#define _DATA_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include <pthread.h>
#include <type.h>

/*******************************************************
 * 数据结构
 *******************************************************/
/*U盘状态 */
typedef enum
{
    USB_STATE_PLUG_IN,
    USB_STATE_UNPLUG
} E_USB_STATE;

/*转储状态 */
typedef enum
{
    DUMP_STATE_INIT,
    DUMP_STATE_DUMPING,
    DUMP_STATE_SUCCESS,
    DUMP_STATE_FAIL
} E_DUMP_STATE;

/*PCM设备状态 */
typedef enum
{
    PCM_STATE_IDLE,
    PCM_STATE_PLAYING,
    PCM_STATE_RECORDING,
    PCM_STATE_ERROR
} E_PCM_STATE;

/*TAX通信状态 */
typedef enum
{
    TAX_STATE_COMM_NORMAL,
    TAX_STATE_COMM_ERROR
} E_TAX_STATE;

/*程序更新状态 */
typedef enum
{
    UPDATE_STATE_INIT,
    UPDATE_STATE_UPDATING,
    UPDATE_STATE_FINISH
} E_UPDATE_STATE;

/*工作状态 */
typedef struct
{
    /*插上,拔下 */
    E_USB_STATE usb_state;
    /*初始化,正在转储全部文件,正在转储最新文件,转储成功,转储失败 */
    E_DUMP_STATE dump_state;
    /*初始化,正在PCM设备状态,正在播放最新,空闲 */
    E_PCM_STATE pcm_state;
    /*正常,故障 */
    E_TAX_STATE tax_comm_state;
    /*初始化,升级中,升级完成 */
    E_UPDATE_STATE update_state;
    /*保护锁 */
    pthread_mutex_t mutex;
} work_state_t;

/*******************************************************
 * 函数声明
 *******************************************************/

/*******************************************************
 *
 * @brief  设置USB状态
 *
 * @param  *usb_state: usb的状态
 * @retval none
 *
 *******************************************************/
void data_set_usb_state(E_USB_STATE usb_state);

/*******************************************************
 *
 * @brief  获取USB状态
 *
 * @retval E_USB_STATE usb的状态
 *
 *******************************************************/
E_USB_STATE data_get_usb_state(void);

/*******************************************************
 *
 * @brief  设置转储状态
 *
 * @param  *dump_state: 转储状态
 * @retval none
 *
 *******************************************************/
void data_set_dump_state(E_DUMP_STATE dump_state);

/*******************************************************
 *
 * @brief  获取转储状态
 *
 * @retval E_DUMP_STATE 转储状态
 *
 *******************************************************/
E_DUMP_STATE data_get_dump_state(void);

/*******************************************************
 *
 * @brief  设置播放状态
 *
 * @param  pcm_state: pcm设备状态
 * @retval none
 *
 *******************************************************/
void data_set_pcm_state(E_PCM_STATE pcm_state);

/*******************************************************
 *
 * @brief  获取播放状态
 *
 * @retval E_PCM_STATE 播放状态
 *
 *******************************************************/
E_PCM_STATE data_get_pcm_state(void);

/*******************************************************
 *
 * @brief  设置tax通信状态
 *
 * @param  tax_comm_state: tax通信状态
 * @retval none
 *
 *******************************************************/

void data_set_tax_comm_state(E_TAX_STATE tax_comm_state);

/*******************************************************
 *
 * @brief  获取TAX的通信状态
 *
 * @retval E_TAX_STATE tax的通信状态
 *
 *******************************************************/
E_TAX_STATE data_get_tax_comm_state(void);

/*******************************************************
 *
 * @brief  数据模块初始化
 *
 * @param  list: 提示音播放列表的数据指针
 * @retval int 0:成功 -1:失败
 *
 *******************************************************/
int data_init(void);

#endif
