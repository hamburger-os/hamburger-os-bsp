/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-27     zm       the first version
 */


#include "usb.h"

#define DBG_TAG "usbthread"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
/* #include <utime.h> */
#include <rtdef.h>
#include <ulog.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <type.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

#include <rtthread.h>
#include <rtdevice.h>

#include "utils.h"
#include "file_manager.h"
#include "led.h"

#define USB_COPY_FILE_BUFF_MAX_SIZE  (4096)

static S_LATEST_DIR_FILE_INFO latest_dir_info;

/*******************************************************
 *
 * @brief  将文件(from)拷贝到文件(to)中去
 *
 * @param  *from: 需要拷贝的文件名
 * @param  *to: 目的地址文件名
 * @retval 0:成功 负数:失败
 *
 *******************************************************/
static sint32_t copy_file(const char *from, const char *to)
{
    sint32_t from_fd, to_fd;
    ssize_t bytes_read, bytes_write;
    sint32_t i = 0;
    char buffer[USB_COPY_FILE_BUFF_MAX_SIZE];
    char *ptr;
    sint32_t ret = 0;

    /* 打开源文件 */
    from_fd = open(from, O_RDONLY);
    if (from_fd < 0)
    {
        LOG_E("can not open file %s", from);
        ret = -1;
        return ret;
    }

    /* 创建目的文件 */
    to_fd = open(to, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (to_fd < 0)
    {
        LOG_E("can not open file %s", to);
        ret = -2;
        return ret;
    }

    while ((bytes_read = read(from_fd, buffer, USB_COPY_FILE_BUFF_MAX_SIZE)) != 0)
    {
        if ((bytes_read < 0) && (errno != EINTR))
        {
            ret = -3;
            break; /* 一个致命的错误发生了 */
        }
        else if (bytes_read > 0)
        {
            ptr = buffer;
            while ((bytes_write = write(to_fd, ptr, bytes_read)) != 0)
            {
                if ((bytes_write == -1) && (errno != EINTR)) /* 一个致命错误发生了 */
                {
                    break;
                }
                else if (bytes_write == bytes_read) /* 写完了所有读的字节 */
                {
                    break;
                }
                else if (bytes_write > 0) /* 只写了一部分,继续写 */
                {
                    ptr += bytes_write;
                    bytes_read -= bytes_write;
                }
            }
            if (bytes_write == -1) /* 写的时候发生的致命错误 */
            {
                ret = -4;
                break;
            }
        }
        i++;
        if (i > 128)
        {
            fsync(to_fd);
            i = 0;
        }
    }

    if (ret == 0)
    {
        fsync(to_fd);
    }
    close(from_fd);
    close(to_fd);
    return ret;
}

/*******************************************************
 *
 * @brief  转储语音文件到U盘.如果mode为0,只是复制文件到U盘;mode为1,先复制文件到U盘,再备份文件.
 *
 * @param  *src: 语音文件在板子上的存储路径.
 * @param  *target: U盘中保存语音文件的路径.
 * @param  mode: 转储的方式; mode为0,只复制文件到U盘; mode为1,先复制文件到U盘,再备份文件.
 * @retval 0:成功 非0:失败
 *
 *******************************************************/
// todo, 增加文件列表大小限制.
static sint32_t store_file(const char *src, const char *target, sint32_t mode)
{
    char *name = NULL;
    char targetname[PATH_NAME_MAX_LEN]; /* 用于存放目标文件名 */
//    char bakname[PATH_NAME_MAX_LEN];    /* 用于存放备份文件名 */
    sint32_t error = 0, pathlen;
    file_info_t *p_file_list_head = NULL, *p = NULL;
    char full_path[PATH_NAME_MAX_LEN] = {0};

    /* 获取文件列表 */
    p_file_list_head = get_org_file_info(src);
    if (p_file_list_head != NULL)
    {
        /* 如果目录中有文件, 则按照文件序号, 先转储序号最小的文件 */
        p_file_list_head = sort_link(p_file_list_head, SORT_DOWN);
        show_link(p_file_list_head);
        p = p_file_list_head;
        while (p != NULL)
        {
            snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, p->record_name);

            LOG_I("copy '%s' to dir '%s'.\n", full_path, target);
            name = get_sigle_file_name(full_path);
            if (strstr(name, "DSW") == NULL) /* 没有找到DSW */
            {
                pathlen = snprintf(targetname,
                                   sizeof(targetname),
                                   "%s/%s.DSW",
                                   target, name);
                if (pathlen > sizeof(targetname))
                {
                    error = -1;
                    break; /* 缓冲区溢出, 字符串被截断了.*/
                }
            }
            else
            {
                pathlen = snprintf(targetname,
                                   sizeof(targetname),
                                   "%s/%s",
                                   target, name);
                if (pathlen > sizeof(targetname))
                {
                    error = -1;
                    break; /* 缓冲区溢出, 字符串被截断了.*/
                }
            }

            if (copy_file(full_path, targetname) < 0)
            {
                error = -1;
                LOG_E("copy file error ... \n");
                break;
            }
//            change_file_date(targetname);
            p = p->next;
        }
        /* 释放缓存空间 */
        free_link(p_file_list_head);
    }

    return error;
}

/*******************************************************
 *
 * @brief  将语音文件自动转储到U盘中
 *
 * @param  mode: 拷贝模式, 拷贝全部文件或者拷贝最新文件.
 * @retval sint32_t 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t usb_auto_copy(E_CopyMode mode)
{
    sint32_t udisk_free_space;
    sint32_t voice_size;
    struct stat stat_l;
//    char logname[PATH_NAME_MAX_LEN];

    /* 获取最新目录文件名.*/
    if (FMGetLatestFileInfo(NEW_DIR_FILE_NAME_CONF, &latest_dir_info) == 0)
    {
        /* 正确读到了文件内容 */
        if (LATEST_DIR_FILE_HEAD_FLAG != latest_dir_info.head_flag)
        {
            /* 如果不是文件头,则为错误状态. */
            LOG_E("file head flag error");
            return -1;
        }
        else
        {

        }
    }
    else
    {
        LOG_E("get latest dir error");
        return -1;
    }
    /* 建立U盘目录 */
    create_dir(TARGET_DIR_NAME);     /* 在U盘目录中建立语音文件目录 */

    /* step1: 查看所有文件的大小 */
    /* U盘的剩余空间大小 */
    udisk_free_space = get_disk_free_space(LATEST_DIR_NAME_FILE_PATH_NAME);
    /* 所有文件的大小 */
#if 0
    if (mode == COPYMODE_ALL)
    {
        voice_size = dir_size(RECORD_FILE_PATH_NAME);
    }
    else
    {
//        voice_size = dir_size(YUYIN_PATH_NAME) - dir_size(YUYIN_BAK_PATH_NAME);  //这里计算把转存过的大小减去
    }
#else
    voice_size = dir_size(RECORD_FILE_PATH_NAME);  //这里实际应该计算未转存过的文件大小
#endif
    /* U盘没有剩余空间 */
    if (udisk_free_space < 0)
    {
        /* U盘转储失败 */
        LOG_E("U盘转储失败");
        return (sint32_t)-1;
    }
    if (voice_size / 1024 > udisk_free_space)
    {
        LOG_E("U盘空间不够! U盘剩余空间:%dK, 文件大小:%dKB",
                  udisk_free_space, voice_size / 1024);
        /* U盘已满 */
        return (sint32_t)-1;
    }

    /* step2: 转储语音文件 */
#if 1
    LOG_I("转储记录文件到U盘 ...");
    /* 开始转储全部文件 */
    /* 从"DIR_FILE_PATH_NAME"目录下面读出所有存储文件的文件名，然后把所有存储文件复制到U盘 */
    if (store_file(DIR_FILE_PATH_NAME, TARGET_DIR_NAME, BACKUP_NO) != 0)
    {
        /* 转储失败 */
        LOG_E("转储失败");
        return (sint32_t)-1;
    }
#else
    if (mode == COPYMODE_ALL) /* 开始转储全部文件 */
    {
        LOG_I("转储全部记录文件到U盘 ...");
        /* 开始转储全部文件 */
        /* 把"yysj/bak"目录下面的所有文件复制到U盘 */
        if (store_file(RECORD_FILE_PATH_NAME, TARGET_DIR_NAME, BACKUP_NO) != 0)
        {
            /* 转储失败 */
            LOG_E("转储失败");
            return (sint32_t)-1;
        }
    }
    else /* 开始转储最新文件 */
    {
        LOG_I("转储最新文件到U盘...");
        /* 开始转储最新文件. */
    }
#endif

#if 0
    /* step3:转储日志文件 */
    /* 搜索"yysj/目录"下面的所有文件, 把语音文件分别复制到U盘, 并把文件移动到bak子目录中 */
    log_print(LOG_INFO, "开始转储、移动yysj/目录下的文件. \n");
    if (store_file(YUYIN_PATH_NAME, TARGET_DIR_NAME, BACKUP))
    {
        log_print(LOG_ERROR, "转储失败.\n");
        event_push_queue(EVENT_DUMP_FAIL);
        return (sint32_t)-1;
    }

    /* step4:转储日志文件 */
    /* 增加转储日志文件的功能, 如果发现有日志文件, 则复制到U盘 */
    snprintf(logname, sizeof(logname),
             "%s/LY05C_%d-%d.log",
             LOG_FILE_PATH,
             g_locomotive_type,
             g_locomotive_id);
    if (stat(logname, &stat_l) == 0)
    {
        log_print(LOG_ERROR, "转储日志.\n");
        copy_file(logname, logname);
    }
#endif

    LOG_I("转储完成");

    return 0;
}

/*******************************************************
 *
 * @brief  USB转储线程入口函数
 *
 * @param  *args: 提示音播放列表的数据指针
 * @retval none
 *
 *******************************************************/
static void usb_thread(void *args)
{
    sint32_t ret = 0;
    E_DUMP_STATE state = DUMP_STATE_INIT;
    char udisk_id[USB_KEY_MAX_LEN];
    struct stat stat_l;
    sint32_t fd;
    LOG_I("usb thread start");
    while (1)
    {
        switch (state)
        {
        case DUMP_STATE_INIT: /* 初始化状态 */

            /* 一直等待直至U盘插入 */
            ret = stat(USB_UD0P0_PATH, &stat_l);
            LOG_I("wait usb");
            while (ret < 0)
            {
                /* 没有插入U盘, 休眠500ms. */
                rt_thread_mdelay((uint32_t)500);
                ret = stat(USB_UD0P0_PATH, &stat_l);
            }

            /* 等待系统识别U盘内部的数据.*/
            rt_thread_mdelay((uint32_t)500);

            /* 表明插上U盘 */
            LOG_I("have usb");
            /* 如果存在升级文件, 则不进行转储. */
            ret = stat(UPGRADE_FILE_NAME, &stat_l);
            if (ret == 0)
            {
                rt_thread_mdelay((uint32_t)500);
                break;
            }
#if 1

            state = DUMP_STATE_DUMPING;
#else
            /* 如果存在格式化文件, 则不进行转储. */
            ret = stat(FORMAT_DIR_NAME, &stat_l);
            if (ret == 0)
            {
                /* 格式化U盘.*/
                format_board_emmc();
                /* 重新初始化文件管理模块.*/
                fm_init();
                /* 进入离开模式,等待拔出U盘*/
                state = DUMP_STATE_EXIT;
                break;
            }
            /* 打开文件 */
            fd = open(UDISK_ID_PATH, O_RDONLY);
            if (fd < 0)
            {
                LOG_E("can not open udisk id file %s. \n", UDISK_ID_PATH);
                break;
            }

            /* 读取U盘ID */
            memset(udisk_id, 0, sizeof(udisk_id));
            ret = read(fd, udisk_id, USB_KEY_MAX_LEN);
            if (ret < 0)
            {
                /* 没有读取到U盘的ID,则可以直接转储,不需要鉴权 */
                state = DUMP_STATE_DUMPING;
            }
            else
            {
                /* 已经读取到了U盘的ID, 进行鉴权认证 */
                ret = check_udisk_id(udisk_id);
                if (ret < 0)
                {
                    state = DUMP_STATE_FAIL;
                }
                else
                {
                    state = DUMP_STATE_DUMPING;
                }
            }

            /* 关闭文件 */
            ret = close(fd);
            if (ret < 0)
            {
                LOG_E("close error");
            }
#endif
            break;
        case DUMP_STATE_DUMPING: /* 转储文件中 */
            LOG_I("save file");
            LedCtrlON(USB_LED);
            /* 转储最新文件 */
            ret = usb_auto_copy(COPYMODE_NEW);
            if (ret < 0)
            {
                state = DUMP_STATE_FAIL;
                LOG_E("save error");
            }
            else
            {
                state = DUMP_STATE_SUCCESS;
                LOG_I("save ok");
            }
            break;
        case DUMP_STATE_SUCCESS: /* 转储成功 */
            state = DUMP_STATE_EXIT;
            LedCtrlON(USB_LED);
            break;
        case DUMP_STATE_FAIL: /* 转储失败 */
            LedCtrlON(USB_LED);
            state = DUMP_STATE_EXIT;
            break;
        case DUMP_STATE_EXIT: /* USB操作已完成, 离开中 */
#if 1
            ret = stat(USB_UD0P0_PATH, &stat_l);
            LOG_I("wait usb levae");
            LedCtrlOFF(USB_LED);
            while (ret == 0)
            {
                rt_thread_sleep(1);
                ret = stat(USB_UD0P0_PATH, &stat_l);
            }
            LOG_I("usb leave");
#else
            ret = stat(UDISK_ID_PATH, &stat_l);
            /* 等待U盘拔出 */
            while (ret == 0)
            {
                rt_thread_sleep(1);
                ret = stat(UDISK_ID_PATH, &stat_l);
            }
            /* 表明拔下U盘 */
#endif
            state = DUMP_STATE_INIT;
            break;
        default:
            break;
        }
    }
}

/*******************************************************
 *
 * @brief  U盘转储操作
 *
 * @retval sint32_t 0:成功 -1:失败
 *
 *******************************************************/
sint32_t usb_init(void)
{
    rt_thread_t usb_tid;

    /* 创建USB线程 */
    usb_tid = rt_thread_create("record_usb",
                               usb_thread,
                               RT_NULL,
                               1024 * 10,
                               25,
                               10);
    if (usb_tid != NULL)
    {
        rt_thread_startup(usb_tid);
        return (sint32_t)0;
    }
    else
    {
        return (sint32_t)-1;
    }
}

