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
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "type.h"
#include "usb.h"

/*******************************************************
 * 函数声明
 *******************************************************/

/*******************************************************
 *
 * @brief  将字符串的的小写转变为大写.
 *
 * @param  str: 需要转换大写的字符串
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
static int to_uppers(char *str)
{
    int count = 0;
    while (str[count] != '\0')
    {
        if (str[count] >= 'a' && str[count] <= 'z')
        {
            /* 小写字母的值比大写字母的值大32,所以转化为大写时,需要减去32. */
            str[count] -= 32;
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
static int __check_udisk_id(const char *udisk_id, const char *code_file)
{
    int i = 0;
    int fd, ret;
    char udisk_id_coded[USB_KEY_MAX_LEN];

    /* 判断是否有空指针 */
    if ((udisk_id == NULL) || (code_file == NULL))
    {
        return -1;
    }

    /* 打开文件 */
    fd = open(code_file, O_RDONLY);
    if (fd < 0)
    {
        return -1;
    }

    /* 读取U盘的ID */
    memset(udisk_id_coded, 0, sizeof(udisk_id_coded));
    ret = read(fd, udisk_id_coded, sizeof(udisk_id_coded));
    if (ret < 0)
    {
        return -1;
    }

    /* 关闭文件 */
    close(fd);

    /* 校验U盘ID */
    while (udisk_id[i] != '\0')
    {
        if (udisk_id[i] != (udisk_id_coded[i] ^ 0xAA))
        {
            return -1;
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
int check_udisk_id(char *id)
{
    int ret = 0;

    if (id == NULL)
        return -1;

    /* 将key中的小写字母全部转化为大写字母. */
    ret = to_uppers(id);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "to_uppers Err:%d\n", ret);
        return -1;
    }

    /* 经过和股份人员沟通, U盘鉴权很少用到, 暂时将此功能给关掉 */
    /* return __check_udisk_id(id, SW_KEY_FILE_PATH); */
    return 0;
}
