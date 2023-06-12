/*******************************************************
 *
 * @FileName: udisk_id.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: u盘鉴权相关功能.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "type.h"
#include "usb.h"
#include "log.h"

/*******************************************************
 * 函数声明
 *******************************************************/
/* 将字符串的的小写转变为大写 */
static sint32_t to_uppers(char *str);
/* 模块内部函数, 校验U盘的ID. */
static sint32_t check_udisk(const char *udisk_id, const char *code_file);

/*******************************************************
 * 函数实现
 *******************************************************/

/*******************************************************
 *
 * @brief  将字符串的的小写转变为大写.
 *
 * @param  str: 需要转换大写的字符串
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t to_uppers(char *str)
{
    sint32_t count = 0;
    while (str[count] != (char)'\0')
    {
        if (((char)'a' <= str[count]) && ((char)'z' >= str[count]))
        {
            /* 小写字母的值比大写字母的值大32,所以转化为大写时,需要减去32. */
            str[count] = str[count] - (char)32;
        }
        else
        {
        }
        count++;
    }
    return 0;
}
/*******************************************************
 *
 * @brief  模块内部函数, 校验U盘的ID.
 *
 * @param  *udisk_id: U盘的ID
 * @param  *code_file: 校验文件
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t check_udisk(const char *udisk_id, const char *code_file)
{
    sint32_t i = 0;
    sint32_t fd, ret;
    char udisk_id_coded[USB_KEY_MAX_LEN];

    /* 判断是否有空指针 */
    if ((udisk_id == NULL) || (code_file == NULL))
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    /* 打开文件, 此处并没有在不同的线程中使用, 故不存在共享资源竞争  */
    fd = open((const char *)code_file, O_RDONLY);
    if (fd < 0)
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    /* 读取U盘的ID */
    memset(udisk_id_coded, 0, sizeof(udisk_id_coded));
    ret = read(fd, udisk_id_coded, sizeof(udisk_id_coded));
    if (ret < 0)
    {

        /* 关闭文件 */
        close(fd);
        return (sint32_t)-1;
    }
    else
    {
    }

    /* 关闭文件 */
    close(fd);

    /* 校验U盘ID */
    while (udisk_id[i] != (char)'\0')
    {
        if ((uint8_t)udisk_id[i] != (uint8_t)((uint8_t)udisk_id_coded[i] ^ (uint8_t)0xAA))
        {
            return (sint32_t)-1;
        }
        else
        {
        }
        i++;
    }
    return 0;
}

/*******************************************************
 *
 * @brief 校验U盘的ID是否正确
 *
 * @param  *id: U盘的ID
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t check_udisk_id(char *id)
{
    sint32_t ret = 0;

    if (id == NULL)
    {

        return (sint32_t)-1;
    }
    else
    {
    }

    /* 将key中的小写字母全部转化为大写字母. */
    ret = to_uppers(id);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "to_uppers Err:%d\n", ret);
        return (sint32_t)-1;
    }
    else
    {
    }

    /* 经过和股份人员沟通, U盘鉴权很少用到, 暂时将此功能给关掉 */
    return check_udisk(id, SW_KEY_FILE_PATH);
}
