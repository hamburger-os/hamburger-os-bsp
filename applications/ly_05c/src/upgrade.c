/*******************************************************
 *
 * @FileName: upgrade.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 升级模块
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <ota_from_file.h>
#include <stdio.h>
#include <event.h>

/*******************************************************
 * 宏定义
 *******************************************************/

/* 升级状态标志文件 */
#define UPGRADE_STATA_FILE "/mnt/emmc/yysj/upgrade_state"
/* 升级状态标志文件读写最大缓冲区大小 */
#define UPGRADE_STATA_MAX_LEN 16

/*******************************************************
 * 数据结构
 *******************************************************/

/* 升级状态标志 */
typedef enum
{
    UPGRADE_STATE_NOT = 0,       /* 非升级状态 */
    UPGRADE_STATE_UPGRADING = 1, /* 升级中 */
    UPGRADE_STATE_INVALID = 2,   /* 非有效标志 */
} E_UPGRADE_STATE;

/*******************************************************
 * 函数声明
 *******************************************************/
/* 设置升级标志 */
static sint32_t upgrade_set_flag(E_UPGRADE_STATE state);
/* 获取升级状态标志 */
static E_UPGRADE_STATE upgrade_get_flag(void);

/*******************************************************
 * 函数实现
 *******************************************************/
/*******************************************************
 *
 * @brief  设置升级标志
 *
 * @param  sint32_t update: 0:非升级状态 1:升级状态
 * @retval sint32_t 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t upgrade_set_flag(E_UPGRADE_STATE state)
{

    sint32_t fd;
    sint32_t ret = 0;
    char buf[UPGRADE_STATA_MAX_LEN];

    /* 打开文件 */
    fd = open(UPGRADE_STATA_FILE, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    /* 写入标志 */
    snprintf(buf, sizeof(buf), "%d", state);

    ret = write(fd, buf, strlen(buf) + 1);
    if (ret < 0)
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    /* 关闭文件 */
    ret = close(fd);
    if (ret < 0)
    {

        return (sint32_t)-1;
    }
    else
    {
        return 0;
    }
}
/*******************************************************
 *
 * @brief  获取升级状态标志
 *
 * @retval E_UPGRADE_STATE 升级状态标志
 *
 *******************************************************/
static E_UPGRADE_STATE upgrade_get_flag(void)
{
    sint32_t fd;
    sint32_t ret = 0;
    char buff[UPGRADE_STATA_MAX_LEN] = {0};

    /* 打开文件 */
    fd = open(UPGRADE_STATA_FILE, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        return UPGRADE_STATE_INVALID;
    }
    else
    {
    }
    ret = read(fd, buff, 1);
    if (ret < 0)
    {
        return UPGRADE_STATE_INVALID;
    }
    else
    {
    }

    /* 关闭文件 */
    ret = close(fd);
    if (ret < 0)
    {

        return UPGRADE_STATE_INVALID;
    }
    else
    {
    }

    /* 返回值 */
    if (buff[0] == '1')
    {
        return UPGRADE_STATE_UPGRADING;
    }
    else
    {
        return UPGRADE_STATE_NOT;
    }
}
/*******************************************************
 *
 * @brief  重定义升级函数
 *
 * @param  OtaHandleTypeDef type: 升级过程中,发生的消息类型.
 * @retval void 无
 *
 *******************************************************/
void ota_from_file_handle(OtaHandleTypeDef type)
{
    E_UPGRADE_STATE state = UPGRADE_STATE_NOT;

    switch (type)
    {
    case OTA_HANDLE_START: /* 开始更新 */
        upgrade_set_flag(UPGRADE_STATE_UPGRADING);
        event_push_queue(EVENT_UPDATE_BEGIN);
        sleep(2); /* 此处延迟, 主要为播放提示音留出时间. */
        break;
    case OTA_HANDLE_FINISH: /* 完成更新 */
        break;
    case OTA_HANDLE_LOADED: /* download 分区内的镜像文件已经被装载.  */
        state = upgrade_get_flag();
        if (state == UPGRADE_STATE_UPGRADING)
        {
            upgrade_set_flag(UPGRADE_STATE_NOT);
            event_push_queue(EVENT_UPDATE_SUCCESS);
        }
        else
        {
        }
        break;
    case OTA_HANDLE_FAILED: /* download 分区内的镜像文件没有被装载. */
        state = upgrade_get_flag();
        if (state == UPGRADE_STATE_UPGRADING)
        {
            upgrade_set_flag(UPGRADE_STATE_NOT);
            event_push_queue(EVENT_UPDATE_FAIL);
        }
        else
        {
        }
        break;
    default: /*缺省*/
        break;
    }
}
