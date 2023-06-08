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
#

/*******************************************************
 *  宏定义
 *******************************************************/

#define DBG_TAG "ly-05c"
#define DBG_LVL DBG_LOG

/*日志最大缓存, 单位:kbyte.   */
#define LOG_BUFFER_LEN (256)
/*单个日志文件最大大小.单位: kbyte.  */
#define LOG_FILE_MAX_SIZE (512)

/*******************************************************
 * 全局变量
 *******************************************************/

static FILE *fp = NULL;
/*日志锁,用于保护fp */
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
void log_print(int level, char *format, ...)
{
    va_list args;
    int size = 0, i = 0;
    char name[PATH_NAME_MAX_LEN];
    char log_buffer[LOG_BUFFER_LEN]; /*为保证线程安全, 采用局部变量. */

    /*通过调试等级,判断是否记录信息. */
    if (level < LOG_LEVEL)
        return;

    /*格式化文件信息 */
    va_start(args, format);
    vsprintf(log_buffer, format, args);
    va_end(args);

    /*在ulog中记录, LOG_E为线程安全函数. */
    /*LOG_D("%s", log_buffer); */
    rt_kprintf("%s", log_buffer);

    /*在日志文件中记录,防止日志信息丢失,每次log都及时写入文件. */
    pthread_mutex_lock(&log_mutex);
    /*如果日志文件超过512 kbyte, 则分割文件. */
    size = file_size(LOG_FILE_NAME);
    if (size > LOG_FILE_MAX_SIZE * 1024)
    {
        i = 0;
        while (access(name, 0) == 0)
        {
            sprintf(name, "%s-%d", LOG_FILE_NAME, ++i);
        }
        rename(LOG_FILE_NAME, name);
    }
    /*记录日志 */
    fp = fopen(LOG_FILE_NAME, "a+");
    if (fp == NULL)
    {
        pthread_mutex_unlock(&log_mutex);
        return;
    }
    fprintf(fp, "%s", log_buffer);
    fclose(fp);
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
int log_init(void)
{
    int ret;

    /*初始化互斥锁 */
    pthread_mutex_init(&log_mutex, NULL);

    /*递归的创建目录和文件 */
    ret = create_file(LOG_FILE_NAME);
    if (ret < 0)
    {
        return ret;
    }

    return 0;
}
