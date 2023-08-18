
/*******************************************************
 *
 * @FileName: usb.h
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: USB转储模块头文件.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef USB_H_
#define USB_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include "type.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* 存储U盘ID的文件 */
#define UDISK_ID_PATH "/proc/udisk/SerialNumber"
#define USB_UD0P0_PATH "/mnt/udisk/ud0p0"

/* 思维验证key的文件 */
//#define SW_KEY_FILE_PATH "/mnt/udisk/ud0p0/sw.lib"
/* U盘ID的长度,单位:byte */
#define USB_KEY_MAX_LEN 64

/* 只复制文件到U盘 */
#define COPY_MODE_COPY_OLAY 0
/* 先复制文件到U盘,再备份文件 */
#define COPY_MODE_COPY_BAK 1


/* 转储模式 */
typedef enum
{
    COPYMODE_NEW = 0, /* 转储最新文件模式  未转存过的文件 */
    COPYMODE_ALL = 1  /* 转储全部文件模式 */
} E_CopyMode;

/* 转储状态 */
typedef enum
{
    DUMP_STATE_INIT,    /* 初始化状态 */
    DUMP_STATE_DUMPING, /* 转储中 */
    DUMP_STATE_SUCCESS, /* 转储成功 */
    DUMP_STATE_FAIL,    /* 转储失败 */
    DUMP_STATE_EXIT     /* USB操作已完成,离开中 */
} E_DUMP_STATE;

/*******************************************************
 * 函数声明
 *******************************************************/

uint16_t get_locomotive_type(void);
uint16_t get_locomotive_id(void);

/*******************************************************
 *
 * @brief 校验U盘的ID是否正确
 *
 * @param  *id: U盘的ID
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t check_udisk_id(char *id);

/*******************************************************
 *
 * @brief  U盘转储操作
 *
 * @retval sint32_t 0:成功 -1:失败
 *
 *******************************************************/
sint32_t usb_init(void);

#endif
