/*******************************************************
 *
 * @FileName: log.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 日志模块的实现
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <utime.h>
#include <rtthread.h>
#include <rtdef.h>
#include <ulog.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <type.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

#include "utils.h"
#include "usb.h"
#include "file_manager.h"
#include "event.h"
#include "delay.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* 转储最新,全部语音文件按钮 */
#define NEW_ALL_PIN "PF.11"
/* Size大,转储速度快 */
#define BUFFER_SIZE 4096

/*******************************************************
 * 数据结构
 *******************************************************/

/* 转储模式 */
typedef enum _CopyMode
{
    CopyMode_New = 0, /*转储最新文件模式 */
    CopyMode_All = 1  /*转储全部文件模式 */
} E_CopyMode;

/*******************************************************
 * 全局变量
 *******************************************************/

/* 最新语音文件 */
static char latest_filename[USB_KEY_MAX_LEN];
/* 机车型号 */
static unsigned short g_locomotive_type = 0;
/* 机车号 */
static unsigned short g_locomotive_id = 0;

/*******************************************************
 * 函数声明
 *******************************************************/

/*******************************************************
 *
 * @brief  获取年份
 *
 * @param  *time: 时间数据
 * @retval int 年份
 *
 *******************************************************/

int get_year(unsigned char *time)
{
    return (((time[3]) >> 2));
}

/*******************************************************
 *
 * @brief  获取月份
 *
 * @param  *time: 时间数据
 * @retval int 月份
 *
 *******************************************************/

int get_month(unsigned char *time)
{
    return (((time[2] & 0xC0) >> 6) | ((time[3] & 0x03) << 2));
}

/*******************************************************
 *
 * @brief  获取日期
 *
 * @param  *time: 时间数据
 * @retval int 日期
 *
 *******************************************************/

int get_day(unsigned char *time)
{
    return ((time[2] & 0x3E) >> 1);
}

/*******************************************************
 *
 * @brief  获取时
 *
 * @param  *time: 时间数据
 * @retval int 时
 *
 *******************************************************/

int get_hour(unsigned char *time)
{
    return (((time[1] & 0xF0) >> 4) | ((time[2] & 0x01) << 4));
}

/*******************************************************
 *
 * @brief  获取分
 *
 * @param  *time: 时间数据
 * @retval int 分
 *
 *******************************************************/

int get_minute(unsigned char *time)
{
    return (((time[0] & 0xC0) >> 6) | ((time[1] & 0x0F) << 2));
}

/*******************************************************
 *
 * @brief  改变日期的时间
 *
 * @param  *time: 时间数据
 * @retval int 分
 *
 *******************************************************/

int change_file_date(const char *filename)
{
    file_head_t file_head;
    int fd;
    int year, month, day, hour, minute;
    struct tm tm_v;
    time_t time_v;
    struct utimbuf utimebuf;

    if ((fd = open(filename, O_RDONLY)) == -1)
    {

        log_print(LOG_ERROR, "can not open file %s, error code: %s\n",
                  filename, strerror(errno));
        return -1;
    }
    if (read(fd, (char *)&file_head, sizeof(file_head)) == sizeof(file_head))
    {
        if (strcmp((char *)file_head.file_head_flag, FILE_HEAD_FLAG) == 0)
        {
            close(fd);
            /**
             * int tm_sec 代表目前秒数,正常范围为0-59,但允许至61秒
             * int tm_sec 代表目前秒数,正常范围为0-59,但允许至61秒
             * int tm_min 代表目前分数,范围0-59
             * int tm_hour 从午夜算起的时数,范围为0-23
             * int tm_mday 目前月份的日数,范围01-31
             * int tm_mon 代表目前月份,从一月算起,范围从0-11
             * int tm_year 从1900 年算起至今的年数
             */
            year = get_year(file_head.date_time) + 2000 - 1900;
            month = get_month(file_head.date_time) - 1;
            day = get_day(file_head.date_time);
            hour = get_hour(file_head.date_time);
            minute = get_minute(file_head.date_time);

            if (month > 11 || month < 0)
                month = 0;
            if (day > 31 || day < 1)
                day = 1;
            if (hour > 23 || hour < 0)
                hour = 0;
            if (minute > 59 || minute < 0)
                minute = 0;

            tm_v.tm_year = year;
            tm_v.tm_mon = month;
            tm_v.tm_mday = day;
            tm_v.tm_hour = hour;
            tm_v.tm_min = minute;
            tm_v.tm_sec = 00;
            time_v = mktime(&tm_v); /*更改文件的修改日期 */

            utimebuf.actime = utimebuf.modtime = time_v;
            /*utime(filename, &utimebuf); */

            g_locomotive_id = file_head.locomotive_num[0] + file_head.locomotive_num[1] * 0x100;
            g_locomotive_type = file_head.locomotive_type[0] + file_head.locomotive_type[1] * 0x100;
            return 0;
        }
    }
    close(fd);
    return -1;
}

/*******************************************************
 *
 * @brief  将文件(From)拷贝到文件(to)中去
 *
 * @param  *From: 需要拷贝的文件名
 * @param  *to: 目的地址文件名
 * @retval 0:成功 负数:失败
 *
 *******************************************************/
int copy_file(char *from, char *to)
{
    int from_fd, to_fd;
    ssize_t bytes_read, bytes_write;
    int i = 0;
    char buffer[BUFFER_SIZE];
    char *ptr;
    int ret = 0;

    /* 打开源文件 */
    if ((from_fd = open(from, O_RDONLY)) == -1)
    {
        log_print(LOG_ERROR, "can not open file %s, error code: %s\n",
                  from, strerror(errno));
        ret = -1;
        return ret;
    }

    /* 创建目的文件 */
    if ((to_fd = open(to, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR)) == -1)
    {
        log_print(LOG_ERROR, "can not open file %s, error code: %s\n",
                  to, strerror(errno));
        ret = -2;
        return ret;
    }

    while ((bytes_read = read(from_fd, buffer, BUFFER_SIZE)) != 0)
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
        if (++i > 128)
        {
            fsync(to_fd);
            i = 0;
        }
    }

    close(from_fd);
    if (ret == 0)
    {
        fsync(to_fd);
    }
    close(to_fd);
    return ret;
}
/*******************************************************
 *
 * @brief  转储语音文件到U盘.如果mode为0,只是复制文件到U盘;mode为1,先复制文件到U盘,再备份文件
 *
 * @param  *Source: 语音文件在板子上的存储路径
 * @param  *target: U盘中保存语音文件的路径
 * @param  mode: 转储的方式;mode为0,只复制文件到U盘;mode为1,先复制文件到U盘,再备份文件
 * @retval 0:成功 非0:失败
 *
 *******************************************************/

int store_file(char *Source, char *target, int mode)
{
    char *name;
    char targetname[PATH_NAME_MAX_LEN]; /* 用于存放目标文件名 */
    char bakname[PATH_NAME_MAX_LEN];    /* 用于存放备份文件名 */
    int error = 0;
    file_info_t *file_list_head = NULL, *p = NULL;

    file_list_head = get_org_file_info(Source);

    if (file_list_head != NULL)
    {
        /* 如果目录中有文件,则按照文件序号,先转储序号最小的文件 */
        file_list_head = sort_link(file_list_head, SORT_DOWN);
        show_link(file_list_head);
        p = file_list_head;
        while (p)
        {
            log_print(LOG_INFO, "复制文件%32s到目录%s中.\n", p->filename, target);
            name = get_sigle_file_name(p->filename);
            if (strstr(name, "VSW") == NULL) /* 没有找到VSW */
            {
                sprintf(targetname, "%s/%s.VSW", target, name);
            }
            else
            {
                sprintf(targetname, "%s/%s", target, name);
            }

            if (copy_file(p->filename, targetname) < 0)
            {
                error = -1;
                log_print(LOG_ERROR, "copy file error ... \n");
                break;
            }

            log_print(LOG_INFO, "targetname:%s\n", targetname);
            change_file_date(targetname);

            if (mode == COPY_MODE_COPY_BAK) /* mode为1,先复制文件到U盘,再备份文件 */
            {
                if (strcmp(name, latest_filename) != 0)
                {
                    sprintf(bakname, "%s/%s", YUYIN_BAK_PATH_NAME, name);

                    log_print(LOG_INFO, "备份文件%32s到%s\n", p->filename, bakname);
                    if (rename(p->filename, bakname))
                    {
                        perror("rename");
                        error = -2;
                        break;
                    }
                }
            }
            p = p->next;
        }
        /*释放缓存空间 */
        free_link(file_list_head);
    }
    return error;
}

/*******************************************************
 *
 * @brief  将语音文件自动转储到U盘中
 *
 * @param  mode: 拷贝模式, 拷贝全部文件或者拷贝最新文件.
 * @retval int 0:成功 -1:失败
 *
 *******************************************************/
int usb_auto_copy(E_CopyMode mode)
{
    int udisk_free_space;
    int voice_size;
    struct stat stat_l;
    char logname[PATH_NAME_MAX_LEN];

    fm_get_file_name(latest_filename, 0);
    log_print(LOG_INFO, "最新的语音文件名是:%s\n", latest_filename);

    create_dir(TARGET_DIR_NAME);     /* 在U盘目录中建立语音文件目录 */
    create_dir(YUYIN_BAK_PATH_NAME); /* 建立语音文件的备份的目录 */

    /* U盘的剩余空间大小 */
    udisk_free_space = get_disk_free_space(YUYIN_PATH_NAME);
    log_print(LOG_INFO, "U盘的剩余空间大小:%dKB\n", udisk_free_space);

    /* 所有文件的大小 */
    if (mode == CopyMode_All)
    {
        voice_size = dir_size(YUYIN_PATH_NAME);
    }
    else
    {
        voice_size = dir_size(YUYIN_PATH_NAME) - dir_size(YUYIN_BAK_PATH_NAME);
    }
    log_print(LOG_INFO, "所有文件的大小:%dKB %dKB %dKB\n",
              voice_size,
              dir_size(YUYIN_PATH_NAME),
              dir_size(YUYIN_BAK_PATH_NAME));
    voice_size = voice_size / 1024;
    if (udisk_free_space == -1)
    {
        /* 播放提示音, U盘转储失败 */
        event_push_queue(EVENT_DUMP_USB_FULL);
        log_print(LOG_ERROR, "U盘转储失败. \n");
        return -1;
    }

    if (voice_size > udisk_free_space)
    {
        log_print(LOG_ERROR, "U盘空间不够! U盘剩余空间:%dK, 语音文件大小:%d kbytes.\n", udisk_free_space, voice_size);
        /* 播放语音提示, U盘已满 */
        event_push_queue(EVENT_DUMP_USB_FULL);
        return -1;
    }

    if (mode == CopyMode_All) /* 开始转储全部文件 */
    {
        log_print(LOG_INFO, "转储全部语音文件到U盘 ...\n");
        /* 播放提示音, 开始转储全部文件 */
        event_push_queue(EVENT_DUMP_START_ALL);
        /* 把"yysj/bak"目录下面的所有文件复制到U盘 */
        if (store_file(YUYIN_BAK_PATH_NAME, TARGET_DIR_NAME, 0))
        {
            /* 播放提示音,转储失败 */
            log_print(LOG_ERROR, "转储失败\n");
            event_push_queue(EVENT_DUMP_FAIL);
            return -1;
        }
    }
    else /* 开始转储全部文件 */
    {
        log_print(LOG_INFO, "转储最新语音文件到U盘......\n");
        /* 播放语音提示,开始转储最新文件. */
        event_push_queue(EVENT_DUMP_START_LAST);
    }

    /* 搜索"yysj/目录"下面的所有文件,把语音文件分别复制到U盘,并把文件移动到bak子目录中 */
    log_print(LOG_INFO, "\n开始转储、移动yysj/目录下的文件. \n");
    if (store_file(YUYIN_PATH_NAME, TARGET_DIR_NAME, 1))
    {
        log_print(LOG_ERROR, "转储失败.\n");
        event_push_queue(EVENT_DUMP_FAIL);
        return -1;
    }
    /* 增加转储日志文件的功能,如果发现有日志文件,则复制到U盘 */
    if (stat(LOG_FILE_NAME, &stat_l) == 0)
    {
        log_print(LOG_ERROR, "转储日志\n");
        sprintf(logname, "/mnt/usb/LY05C_%d-%d.log", g_locomotive_type, g_locomotive_id);
        copy_file(LOG_FILE_NAME, logname);
    }

    if (mode == CopyMode_All)
    {
        /* 播放语音提示, 转储全部文件完成. */
        event_push_queue(EVENT_DUMP_END_ALL);
    }
    else
    {
        /* 播放语音提示,转储最新文件完成 */
        event_push_queue(EVENT_DUMP_END_LAST);
    }
    log_print(LOG_INFO, "转储完成. \n");
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
    int ret = 0;
    E_DUMP_STATE state = DUMP_STATE_INIT;
    rt_base_t new_all_pin;
    char udisk_id[USB_KEY_MAX_LEN] = {0};
    struct stat stat_l;
    int fd;
    E_CopyMode mode;

    /*获取引脚 */
    new_all_pin = rt_pin_get(NEW_ALL_PIN);
    /*设置为输入模式 */
    rt_pin_mode(new_all_pin, PIN_MODE_INPUT);
    while (1)
    {
        switch (state)
        {
        case DUMP_STATE_INIT: /*初始化状态 */
            /*打开存储U盘ID的文件 */
            ret = stat(UDISK_ID_PATH, &stat_l);
            while (ret < 0)
            {
                /*没有插入U盘,休眠500ms. */
                msleep(500);
                ret = stat(UDISK_ID_PATH, &stat_l);
                event_push_queue(EVENT_UNPLUG_USB);
            }

            /*表明插上U盘 */
            event_push_queue(EVENT_PLUG_IN_USB);

            /*如果存在升级文件, 则不进行转储. */
            ret = stat(UPGRADE_FILE_NAME, &stat_l);
            if (ret == 0)
            {
                msleep(500);
                break;
            }

            /*打开文件 */
            fd = open(UDISK_ID_PATH, O_RDONLY);
            if (fd < 0)
            {
                log_print(LOG_ERROR, "can not open file %s. \n", UDISK_ID_PATH);
                break;
            }

            /*读取U盘ID */
            ret = read(fd, udisk_id, USB_KEY_MAX_LEN);
            if (ret < 0)
            {
                /*没有读取到U盘的ID,则可以直接转储,不需要鉴权 */
                state = DUMP_STATE_DUMPING;
            }
            else
            {
                /*已经读取到了U盘的ID,进行鉴权认证 */
                ret = check_udisk_id(udisk_id);
                if (ret < 0)
                    state = DUMP_STATE_FAIL;
                else
                    state = DUMP_STATE_DUMPING;
            }

            /*关闭文件 */
            ret = close(fd);
            if (ret < 0)
                log_print(LOG_ERROR, "close error.\n");

            break;
        case DUMP_STATE_DUMPING: /*转储文件中 */
            mode = (E_CopyMode)rt_pin_read(new_all_pin);
            if (mode == CopyMode_All) /*转储全部文件 */
            {
                ret = usb_auto_copy(CopyMode_All);
            }
            else /*转储最新文件 */
            {
                ret = usb_auto_copy(CopyMode_New);
            }
            if (ret < 0)
                state = DUMP_STATE_FAIL;
            else
                state = DUMP_STATE_SUCCESS;
            break;
        case DUMP_STATE_SUCCESS: /*转储成功 */
        case DUMP_STATE_FAIL:    /*转储失败 */
            ret = stat(UDISK_ID_PATH, &stat_l);
            while (ret == 0)
            {
                sleep(1);
                ret = stat(UDISK_ID_PATH, &stat_l);
            }
            /*表明拔下U盘 */
            event_push_queue(EVENT_UNPLUG_USB);
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
 * @retval int 0:成功 -1:失败
 *
 *******************************************************/
int usb_init(void)
{
    rt_thread_t usb_tid;

    /* 创建USB线程 */
    usb_tid = rt_thread_create("ly_05c_usb",
                               usb_thread,
                               RT_NULL,
                               1024 * 10,
                               25,
                               10);
    if (usb_tid)
        rt_thread_startup(usb_tid);

    return 0;
}
