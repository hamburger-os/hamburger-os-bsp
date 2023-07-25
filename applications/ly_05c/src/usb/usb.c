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
/* #include <utime.h> */
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
#include "log.h"
#include "voice.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* 转储最新,全部语音文件按钮 */
#define NEW_ALL_PIN "PF.11"
/* size大,转储速度快 */
#define BUFFER_SIZE 4096

/*******************************************************
 * 数据结构
 *******************************************************/

/* 转储模式 */
typedef enum
{
    COPYMODE_NEW = 0, /* 转储最新文件模式 */
    COPYMODE_ALL = 1  /* 转储全部文件模式 */
} E_CopyMode;

/* 备份模式 */
typedef enum
{
    BACKUP_NO = 0, /* 只是复制文件到U盘 */
    BACKUP = 1     /* 先复制文件到U盘,再备份文件 */
} E_Backup;

/*******************************************************
 * 全局变量
 *******************************************************/

/* 最新语音文件 */
static char latest_filename[USB_KEY_MAX_LEN];
/* 机车型号 */
static uint16_t g_locomotive_type = 0;
/* 机车号 */
static uint16_t g_locomotive_id = 0;

/*******************************************************
 * 函数声明
 *******************************************************/
/* 获取年份 */
static sint32_t get_year(const uint8_t *timedata);
/* 获取月份 */
static sint32_t get_month(const uint8_t *timedata);
/* 获取日期 */
static sint32_t get_day(const uint8_t *timedata);
/* 获取时 */
static sint32_t get_hour(const uint8_t *timedata);
/* 获取分 */
static sint32_t get_minute(const uint8_t *timedata);
/* 改变日期的时间 */
static sint32_t change_file_date(const char *filename);
/* 将文件(from)拷贝到文件(to)中去 */
static sint32_t copy_file(const char *from, const char *to);
/* 转储语音文件到U盘. 如果mode为0, 只是复制文件到U盘;mode为1, 先复制文件到U盘,再备份文件 */
static sint32_t store_file(const char *src, const char *target, sint32_t mode);
/* 将语音文件自动转储到U盘中 */
static sint32_t usb_auto_copy(E_CopyMode mode);
/* USB转储线程入口函数 */
static void usb_thread(void *args);

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  获取机车型号
 *
 * @retval uint16_t 机车型号
 *
 *******************************************************/
uint16_t get_locomotive_type(void)
{
    return g_locomotive_type;
}
/*******************************************************
 *
 * @brief  机车号
 *
 * @retval uint16_t 机车号
 *
 *******************************************************/
uint16_t get_locomotive_id(void)
{
    return g_locomotive_id;
}

/*******************************************************
 *
 * @brief  获取年份
 *
 * @param  *timedata: 时间数据
 * @retval sint32_t 年份
 *
 *******************************************************/

static sint32_t get_year(const uint8_t *timedata)
{
    return (sint32_t)(((timedata[3]) >> 2));
}

/*******************************************************
 *
 * @brief  获取月份
 *
 * @param  *timedata: 时间数据
 * @retval sint32_t 月份
 *
 *******************************************************/

static sint32_t get_month(const uint8_t *timedata)
{
    return (sint32_t)(((timedata[2] & 0xC0) >> 6) | ((timedata[3] & 0x03) << 2));
}

/*******************************************************
 *
 * @brief  获取日期
 *
 * @param  *timedata: 时间数据
 * @retval sint32_t 日期
 *
 *******************************************************/

static sint32_t get_day(const uint8_t *timedata)
{
    return (sint32_t)((timedata[2] & 0x3E) >> 1);
}

/*******************************************************
 *
 * @brief  获取时
 *
 * @param  *timedata: 时间数据
 * @retval sint32_t 时
 *
 *******************************************************/

static sint32_t get_hour(const uint8_t *timedata)
{
    return (sint32_t)(((timedata[1] & 0xF0) >> 4) | ((timedata[2] & 0x01) << 4));
}

/*******************************************************
 *
 * @brief  获取分
 *
 * @param  *timedata: 时间数据
 * @retval sint32_t 分
 *
 *******************************************************/

static sint32_t get_minute(const uint8_t *timedata)
{
    return (sint32_t)(((timedata[0] & 0xC0) >> 6) | ((timedata[1] & 0x0F) << 2));
}

/*******************************************************
 *
 * @brief  改变日期的时间
 *
 * @param  *timedata: 时间数据
 * @retval sint32_t 分
 *
 *******************************************************/

static sint32_t change_file_date(const char *filename)
{
    file_head_t file_head;
    sint32_t fd;
    sint32_t year, month, day, hour, minute;
    struct tm tm_v;
    time_t time_v;
    /* struct utimbuf utimebuf;*/
    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        log_print(LOG_ERROR, "can not open file %s.\n",
                  filename);
        return (sint32_t)-1;
    }
    if (read(fd, (void *)&file_head, sizeof(file_head)) == sizeof(file_head))
    {
        if (strcmp((char *)file_head.file_head_flag, FILE_HEAD_FLAG) == 0)
        {
            close(fd);
            /**
             * tm_sec 代表目前秒数,正常范围为0-59,但允许至61秒
             * tm_sec 代表目前秒数,正常范围为0-59,但允许至61秒
             * tm_min 代表目前分数,范围0-59
             * tm_hour 从午夜算起的时数,范围为0-23
             * tm_mday 目前月份的日数,范围01-31
             * tm_mon 代表目前月份,从一月算起,范围从0-11
             * tm_year 从1900 年算起至今的年数
             */
            year = get_year(file_head.date_time) + 2000 - 1900;
            month = get_month(file_head.date_time) - 1;
            day = get_day(file_head.date_time);
            hour = get_hour(file_head.date_time);
            minute = get_minute(file_head.date_time);

            if ((month > 11) || (month < 0))
            {
                month = 0;
            }
            if ((day > 31) || (day < 1))
            {
                day = 1;
            }
            if ((hour > 23) || (hour < 0))
            {
                hour = 0;
            }
            if ((minute > 59) || (minute < 0))
            {
                minute = 0;
            }

            tm_v.tm_year = year;
            tm_v.tm_mon = month;
            tm_v.tm_mday = day;
            tm_v.tm_hour = hour;
            tm_v.tm_min = minute;
            tm_v.tm_sec = 0;
            time_v = mktime(&tm_v);

            /* 更改文件的修改日期 */
            /* utimebuf.actime = utimebuf.modtime = time_v;*/
            /* utime(filename, &utimebuf);*/

            g_locomotive_id = file_head.locomotive_num[0] + file_head.locomotive_num[1] * 0x100;
            g_locomotive_type = file_head.locomotive_type[0] + file_head.locomotive_type[1] * 0x100;

            return 0;
        }
    }
    close(fd);
    return (sint32_t)-1;
}

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
    char buffer[BUFFER_SIZE];
    char *ptr;
    sint32_t ret = 0;

    /* 打开源文件 */
    from_fd = open(from, O_RDONLY);
    if (from_fd < 0)
    {
        log_print(LOG_ERROR, "can not open file %s. \n", from);
        ret = -1;
        return ret;
    }

    /* 创建目的文件 */
    to_fd = open(to, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (to_fd < 0)
    {
        log_print(LOG_ERROR, "can not open file %s. \n",
                  to);
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
static sint32_t store_file(const char *src, const char *target, sint32_t mode)
{
    char *name = NULL;
    char targetname[PATH_NAME_MAX_LEN]; /* 用于存放目标文件名 */
    char bakname[PATH_NAME_MAX_LEN];    /* 用于存放备份文件名 */
    sint32_t error = 0, pathlen;
    file_info_t *p_file_list_head = NULL, *p = NULL;

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
            log_print(LOG_INFO,
                      "copy '%s' to dir '%s'.\n",
                      p->filename, target);
            name = get_sigle_file_name(p->filename);
            if (strstr(name, "VSW") == NULL) /* 没有找到VSW */
            {
                pathlen = snprintf(targetname,
                                   sizeof(targetname),
                                   "%s/%s.VSW",
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

            if (copy_file(p->filename, targetname) < 0)
            {
                error = -1;
                log_print(LOG_ERROR, "copy file error ... \n");
                break;
            }
            change_file_date(targetname);
            if (mode == COPY_MODE_COPY_BAK) /* mode为1,先复制文件到U盘,再备份文件 */
            {
                if (strcmp(name, latest_filename) != 0)
                {
                    pathlen = snprintf(bakname,
                                       sizeof(bakname),
                                       "%s/%s",
                                       YUYIN_BAK_PATH_NAME, name);
                    if (pathlen > sizeof(bakname))
                    {
                        error = -2;
                        break; /* 缓冲区溢出, 字符串被截断了.*/
                    }

                    log_print(LOG_INFO, "备份文件 '%32s' 到 '%s'. \n", p->filename, bakname);
                    /* 如果备份文件存在, 那么 */
                    if (access(bakname, F_OK) == 0)
                    {
                        delete_file(bakname);
                    }
                    if (rename(p->filename, bakname))
                    {
                        error = -2;
                        break;
                    }
                }
            }
            p = p->next;
        }
        /* 释放缓存空间 */
        free_link(p_file_list_head);
    }

    return error;
}
/*******************************************************
 *
 * @brief  将日志文件(包含日志备份文件)自动转储到U盘中.
 *
 * @param  void 无
 * @retval none 无
 *
 *******************************************************/
void usb_copy_log(void)
{
    char log_src[PATH_NAME_MAX_LEN] = {0};
    char log_dest[PATH_NAME_MAX_LEN] = {0};

    /* 拷贝日志文件 */
    snprintf(log_src, sizeof(log_src),
             "%s/LY05C_%d-%d.log",
             LOG_FILE_PATH,
             g_locomotive_type,
             g_locomotive_id);
    snprintf(log_dest, sizeof(log_dest),
             "%s/LY05C_%d-%d.log",
             UDISK_LOG_DIR_NAME,
             g_locomotive_type,
             g_locomotive_id);
    if (access(log_src, F_OK) == 0)
    {
        log_print(LOG_INFO, "copy log file '%s' to udisk.\n", log_src);
        copy_file(log_src, log_dest);
    }

    /* 拷贝日志备份文件 */
    snprintf(log_src, sizeof(log_src),
             "%s/LY05C_%d-%d.log.bak",
             LOG_FILE_PATH,
             g_locomotive_type,
             g_locomotive_id);
    snprintf(log_dest, sizeof(log_dest),
             "%s/LY05C_%d-%d.log.bak",
             UDISK_LOG_DIR_NAME,
             g_locomotive_type,
             g_locomotive_id);
    if (access(log_src, F_OK) == 0)
    {
        log_print(LOG_INFO, "copy log bak file '%s' to udisk.\n", log_src);
        copy_file(log_src, log_dest);
    }
    log_print(LOG_INFO, "copy log file successfully. \n");

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

    /* 获取最新语音文件名.*/
    fm_get_file_name(latest_filename, SRC_FILE);
    /* 建立U盘目录 */
    create_dir(TARGET_DIR_NAME);     /* 在U盘目录中建立语音文件目录 */
    create_dir(YUYIN_BAK_PATH_NAME); /* 建立语音文件的备份的目录 */

    /* 查看所有文件的大小 */
    /* U盘的剩余空间大小 */
    udisk_free_space = get_disk_free_space(YUYIN_PATH_NAME);
    /* 所有文件的大小 */
    if (mode == COPYMODE_ALL)
    {
        voice_size = dir_size(YUYIN_PATH_NAME);
    }
    else
    {
        voice_size = dir_size(YUYIN_PATH_NAME) - dir_size(YUYIN_BAK_PATH_NAME);
    }
    /* U盘没有剩余空间, 则播放提示音 */
    if (udisk_free_space < 0)
    {
        /* 播放提示音, U盘转储失败 */
        event_push_queue(EVENT_DUMP_USB_FULL);
        log_print(LOG_ERROR, "U盘转储失败. \n");
        return (sint32_t)-1;
    }
    if (voice_size / 1024 > udisk_free_space)
    {
        log_print(LOG_ERROR, "U盘空间不够! U盘剩余空间:%dK, 语音文件大小:%dKB.\n",
                  udisk_free_space, voice_size / 1024);
        /* 播放语音提示, U盘已满 */
        event_push_queue(EVENT_DUMP_USB_FULL);
        return (sint32_t)-1;
    }

    /* 转储语音文件 */
    if (mode == COPYMODE_ALL) /* 开始转储全部文件 */
    {
        log_print(LOG_INFO, "转储全部语音文件到U盘 ...\n");
        /* 播放提示音, 开始转储全部文件 */
        event_push_queue(EVENT_DUMP_START_ALL);
        /* 把"yysj/bak"目录下面的所有文件复制到U盘 */
        if (store_file(YUYIN_BAK_PATH_NAME, TARGET_DIR_NAME, BACKUP_NO) != 0)
        {
            /* 播放提示音,转储失败 */
            log_print(LOG_ERROR, "转储失败.\n");
            event_push_queue(EVENT_DUMP_FAIL);
            return (sint32_t)-1;
        }
    }
    else /* 开始转储最新文件 */
    {
        log_print(LOG_INFO, "转储最新语音文件到U盘...\n");
        /* 播放语音提示,开始转储最新文件. */
        event_push_queue(EVENT_DUMP_START_LAST);
    }

    /* 转储语音文件 */
    /* 搜索"yysj/目录"下面的所有文件, 把语音文件分别复制到U盘, 并把文件移动到bak子目录中 */
    log_print(LOG_INFO, "开始转储、移动yysj/目录下的文件. \n");
    if (store_file(YUYIN_PATH_NAME, TARGET_DIR_NAME, BACKUP))
    {
        log_print(LOG_ERROR, "转储失败.\n");
        event_push_queue(EVENT_DUMP_FAIL);
        return (sint32_t)-1;
    }

    /* 转储日志文件 */
    usb_copy_log();

    /* 上报事件 */
    if (mode == COPYMODE_ALL)
    {
        /* 播放语音提示, 转储全部文件完成. */
        event_push_queue(EVENT_DUMP_END_ALL);
    }
    else
    {
        /* 播放语音提示,转储最新文件完成 */
        event_push_queue(EVENT_DUMP_END_LAST);
    }

    return 0;
}

/*******************************************************
 *
 * @brief   格式化板载存储器emmc.具体步骤如下:
 *          1.删除板载存储器的yysj文件夹.
 *          2.创建新的目录:yysj, yysj/voice,并拷贝提示音文件.
 *          3.退出.
 *
 * @param  无
 * @retval none
 *
 *******************************************************/
static void format_board_emmc(void)
{
    /* 提示开始擦除U盘.*/
    event_push_queue(EVENT_BEGIN_FORMAT_STORAGE);
    log_print(LOG_INFO, "begin format emmc...\n");
    /* 删除板载存储器的yysj文件夹.*/
    delete_file(YUYIN_PATH_NAME);
    /* 创建新的目录:yysj, yysj/voice,并拷贝提示音文件.*/
    create_dir(EVENT_VOICE_DIR);
    /* 并拷贝提示音文件.*/
    copy_files(FORMAT_DIR_NAME, EVENT_VOICE_DIR);
    msleep((uint32_t)500);
    /* 提示开始删除U盘.*/
    event_push_queue(EVENT_FINISH_FORMAT_STORAGE);
    log_print(LOG_INFO, "format emmc finished. \n");
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
    rt_base_t new_all_pin;
    char udisk_id[USB_KEY_MAX_LEN];
    struct stat stat_l;
    sint32_t fd;
    E_CopyMode mode;

    /* 获取引脚句柄(全部或者最新转储模式选择引脚) */
    new_all_pin = rt_pin_get(NEW_ALL_PIN);
    /* 设置为输入模式 */
    rt_pin_mode(new_all_pin, PIN_MODE_INPUT);
    while (1)
    {
        switch (state)
        {
        case DUMP_STATE_INIT: /* 初始化状态 */

            /* 一直等待直至U盘插入 */
            ret = stat(UDISK_ID_PATH, &stat_l);
            while (ret < 0)
            {
                /* 没有插入U盘, 休眠500ms. */
                msleep((uint32_t)500);
                ret = stat(UDISK_ID_PATH, &stat_l);
                event_push_queue(EVENT_UNPLUG_USB);
            }

            /* 等待系统识别U盘内部的数据.*/
            msleep((uint32_t)500);

            /* 表明插上U盘 */
            event_push_queue(EVENT_PLUG_IN_USB);

            /* 如果存在升级文件, 则不进行转储. */
            ret = stat(UPGRADE_FILE_NAME, &stat_l);
            if (ret == 0)
            {
                msleep((uint32_t)500);
                break;
            }

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
                log_print(LOG_ERROR, "can not open udisk id file %s. \n", UDISK_ID_PATH);
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
                log_print(LOG_ERROR, "close error.\n");
            }

            break;
        case DUMP_STATE_DUMPING: /* 转储文件中 */
            mode = (E_CopyMode)rt_pin_read(new_all_pin);
            if (mode == COPYMODE_ALL) /* 转储全部文件 */
            {
                ret = usb_auto_copy(COPYMODE_ALL);
            }
            else /* 转储最新文件 */
            {
                ret = usb_auto_copy(COPYMODE_NEW);
            }
            if (ret < 0)
            {
                state = DUMP_STATE_FAIL;
            }
            else
            {
                state = DUMP_STATE_SUCCESS;
            }
            break;
        case DUMP_STATE_SUCCESS: /* 转储成功 */
            state = DUMP_STATE_EXIT;
            break;
        case DUMP_STATE_FAIL: /* 转储失败 */
            state = DUMP_STATE_EXIT;
            break;
        case DUMP_STATE_EXIT: /* USB操作已完成, 离开中 */
            ret = stat(UDISK_ID_PATH, &stat_l);
            /* 等待U盘拔出 */
            while (ret == 0)
            {
                sleep(1);
                ret = stat(UDISK_ID_PATH, &stat_l);
            }
            /* 表明拔下U盘 */
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
 * @retval sint32_t 0:成功 -1:失败
 *
 *******************************************************/
sint32_t usb_init(void)
{
    rt_thread_t usb_tid;

    /* 创建USB线程 */
    usb_tid = rt_thread_create("ly_05c_usb",
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
