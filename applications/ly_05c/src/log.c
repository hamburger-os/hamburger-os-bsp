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
#include "file_manager.h"
#include "log.h"
#include "utils.h"
#include "file_manager.h"
#include "usb.h"

/*******************************************************
 *  宏定义
 *******************************************************/

#define DBG_TAG "ly-05c"
#define DBG_LVL DBG_LOG

/* 日志最大缓存, 单位:kbyte. */
#define LOG_BUFFER_LEN (256)
/* 单个日志文件最大大小.单位: kbyte.  */
#define LOG_FILE_MAX_SIZE (512)

/*******************************************************
 * 全局变量
 *******************************************************/
/* 日志文件句柄 */
static int fd = -1;
/* 日志锁,用于保护fp */
static pthread_mutex_t log_mutex;

/* 当前日志文件的文件名 */
char cur_log_name[PATH_NAME_MAX_LEN];

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
    char log_buffer[LOG_BUFFER_LEN]; /* 为保证线程安全, 采用局部变量. */

    /* 通过调试等级,判断是否记录信息. */
    if (level < LOG_LEVEL)
    {
        return; /* 低于日志记录等级, 不记录.*/
    }
    else
    {
    }

    /* 格式化文件信息 */
    va_start(args, format);
    vsprintf(log_buffer, format, args);
    va_end(args);

    /* 在ulog中记录, LOG_E为线程安全函数. */
    /* LOG_D("%s", log_buffer); */
    rt_kprintf("%s", log_buffer);

    /* 在日志文件中记录,防止日志信息丢失,每次log都及时写入文件. */
    sprintf(name, "%s/LY05C_%d-%d.log",
            LOG_FILE_PATH,
            get_locomotive_type(),
            get_locomotive_id());

    pthread_mutex_lock(&log_mutex);

    /* 日志文件名发生变化时 */
    if (strcmp(name, cur_log_name) != 0)
    {
        if (fd > 0)
        {
            close(fd);
            fd = -1;
        }
    }
    strcpy(cur_log_name, name);
    
    /* 记录日志 */
    if (fd < 0)
    {
        fd = open(cur_log_name, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    }
    if (fd > 0)
    {
        write(fd, log_buffer, strlen(log_buffer));
        fsync(fd); /* 使用fflush不行 */
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
