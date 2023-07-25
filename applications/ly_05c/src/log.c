/********************************************************************
 * @FileName: log.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 日志模块的实现
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *******************************************************/

#include <stdarg.h>
#include <rtdbg.h>
#include <pthread.h>
#include "log.h"
#include "utils.h"
#include "file_manager.h"
#include "usb.h"
#include "rtc.h"

/*******************************************************
 *  宏定义
 *******************************************************/

#define DBG_TAG "ly-05c"
#define DBG_LVL DBG_LOG

/* 日志最大缓存, 单位:byte. */
#define LOG_BUFFER_LEN (512)
/* 单个日志文件最大大小.单位: KB.  */
#define LOG_FILE_MAX_SIZE (1024)

/*******************************************************
 * 全局变量
 *******************************************************/

/* 日志锁,用于保护fp */
static pthread_mutex_t log_mutex;

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  记录日志函数
 *
 * @param  level: 日志记录等级
 * @param  format: 记录格式
 * @retval none
 *
 *******************************************************/
void log_print(sint32_t level, const char *format, ...)
{
    va_list args;
    char name[PATH_NAME_MAX_LEN];
    char log_buffer[LOG_BUFFER_LEN];                /* 为保证线程安全, 采用局部变量. */
    static sint32_t fd = -1;                        /* 日志文件句柄 */
    static char cur_log_name[PATH_NAME_MAX_LEN];    /* 当前日志文件的文件名 */
    static char cur_logbak_name[PATH_NAME_MAX_LEN]; /* 当前日志文件的文件名 */
    time_t t;
    struct tm *timeinfo = NULL;

    /* 通过调试等级,判断是否记录信息. */
    if (level < LOG_LEVEL)
    {
        return; /* 低于日志记录等级, 不记录.*/
    }
    /* 获取时间*/
    t = time(NULL);
    timeinfo = localtime(&t);
    strftime(log_buffer, sizeof(log_buffer), "[%Y-%m-%d %H:%M:%S] ", timeinfo);

    /* 格式化文件信息 */
    va_start(args, format);
    vsprintf(&log_buffer[strlen(log_buffer)], format, args);
    va_end(args);

    /* 在ulog中记录, LOG_E为线程安全函数. */
    /* LOG_D("%s", log_buffer); */
    rt_kprintf("%s", log_buffer);

    /* 在日志文件中记录,防止日志信息丢失,每次log都及时写入文件. */
    snprintf(name, sizeof(name), "%s/LY05C_%d-%d.log",
             LOG_FILE_PATH,
             get_locomotive_type(),
             get_locomotive_id());

    pthread_mutex_lock(&log_mutex);
    /* 日志文件名发生变化时 */
    if (strcmp(name, cur_log_name) != 0)
    {
        if (fd >= 0)
        {
            close(fd);
            fd = -1;
        }
    }
    snprintf(cur_log_name, sizeof(cur_log_name), "%s", name);

    /* 记录日志 */
    if (fd < 0)
    {
        fd = open(cur_log_name, O_CREAT | O_WRONLY | O_APPEND);
    }

    if (fd >= 0)
    {
        write(fd, log_buffer, strlen(log_buffer));
        fsync(fd); /* 使用fflush不行*/
        /**
         文件系统特性, 文件处于打开状态, 则不能删除这个文件;
         所以写入一次日志信息, 需要关闭一次文件, 防止干扰其
         他线程操作这个文件, 比较耗时.
        */
        if (close(fd) >= 0)
        {
            fd = -1;
            // 如果文件大于512KB, 则备份文件
            if (file_size(cur_log_name) > LOG_FILE_MAX_SIZE * 1024)
            {
                snprintf(cur_logbak_name,
                         sizeof(cur_logbak_name),
                         "%s.bak",
                         cur_log_name);

                // 检查文件是否存在, 如果存在, 则删除.
                if (access(cur_logbak_name, F_OK) == 0)
                {
                    delete_file(cur_logbak_name);
                }
                // 重新命名文件
                if (rename(cur_log_name, cur_logbak_name) < 0)
                {
                    rt_kprintf("bakup file error. \n");
                }
            }
        }
    }
    pthread_mutex_unlock(&log_mutex);
}
/*******************************************************
 *
 * @brief  日志模块初始化
 *
 * @param  none
 * @retval none
 *
 *******************************************************/
sint32_t log_init(void)
{
    sint32_t ret;

    /* 初始化互斥锁 */
    pthread_mutex_init(&log_mutex, NULL);

    /* 递归的创建目录和文件 */
    ret = create_dir(LOG_FILE_PATH);
    if (ret < 0)
    {
        return ret;
    }
    return 0;
}
