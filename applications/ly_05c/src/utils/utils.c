/*******************************************************
 *
 * @FileName: utils.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 工具模块
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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>
#include <utime.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/vfs.h>

#include "utils.h"
#include "file_manager.h"

/*******************************************************
 * 宏定义
 *******************************************************/

#define F_OK 0
#define R_OK 4
#define W_OK 2
#define X_OK 1

/*******************************************************
 * 函数声明
*******************************************************/

/*******************************************************
 *
 * @brief  获取U盘剩余空间,单位:KB
 *
 * @param  *path: U盘的文件目录
 * @retval int 剩余空间大小
 *
 *******************************************************/
int get_disk_free_space(const char *path)
{
    ssize_t ret = -1;
    struct statfs disk_info;
    unsigned long long total_blocks;
    unsigned long long total_size;
    ssize_t kb_total_size;
    unsigned long long free_disk;
    ssize_t kb_free_disk;

    ret = statfs(path, &disk_info);
    if (ret != 0)
    {
        log_print(LOG_ERROR, "can not get mount dir %s\n", path);
        return ret;
    }
    total_blocks = disk_info.f_bsize;
    total_size = total_blocks * disk_info.f_blocks;
    kb_total_size = total_size >> 10;
    free_disk = disk_info.f_bfree * total_blocks;
    kb_free_disk = free_disk >> 10;

    log_print(LOG_ERROR, "%s total=%dKB, free=%dKB\n", path, kb_total_size, kb_free_disk);
    return kb_free_disk;
}
/*******************************************************
 *
 * @brief  获取文件的大小,单位:byte.
 *
 * @param  *name: 文件的绝对路径
 * @retval 正数:文件的大小 负数:失败
 *
 *******************************************************/
long file_size(char *name)
{
    int ret = 0;
    struct stat stat_buf;

    ret = stat(name, &stat_buf);
    if (ret < 0)
    {
        return -1;
    }
    return stat_buf.st_size;
}

/*******************************************************
 *
 * @brief  获取指定目录已使用的空间, 单位:字节
 *
 * @param  *dir_path: 文件夹的绝对路径
 * @retval 正数:文件的大小 负数:失败
 *
 *******************************************************/
int dir_size(char *dir_path)
{
    char path[PATH_NAME_MAX_LEN] = {0};
    long total_size = 0;
    int ret = 0;

    /* 把目录路径存放在字符数组内 */
    strcpy(path, dir_path);

    /* 打开目录 */
    DIR *dir = opendir(path);
    if (NULL == dir)
    {
        log_print(LOG_ERROR, "can open dir %s \n", path);
        return -1;
    }

    /* 如果打开成功,则循环读取目录 */
    struct dirent *ent = readdir(dir);
    while (NULL != ent)
    {
        memset(path, 0, sizeof(path));
        strcpy(path, dir_path);
        strcat(path, "/");
        strcat(path, ent->d_name);

        if (DT_DIR == ent->d_type) /*目录文件 */
        {
            ret = dir_size(path);
        }
        else /*非目录文件 */
        {
            ret = file_size(path);
        }
        if (ret < 0)
            return -1;
        total_size += ret;
        ent = readdir(dir);
    }

    ret = closedir(dir);
    if (ret < 0)
        return -1;
    return total_size;
}

/*******************************************************
 *
 * @brief  排序文件列表
 *
 * @param  *h: 需要排序的文件列表
 * @param  flag: 0x01:从小到大(最新) 0x10:从大(最新)到小
 * @retval 排序好的文件列表
 *
 *******************************************************/

file_info_t *sort_link(file_info_t *h, int flag)
{
    file_info_t *endpt, *u, *v, *p;
    u = (file_info_t *)malloc(sizeof(file_info_t));
    u->next = h;
    h = u;
    for (endpt = NULL; endpt != h; endpt = p)
    {
        for (p = u = h; u->next->next != endpt; u = u->next)
        {
            /* 原来是>,修改为<,原来是按照序号由小到大排列,现在为由大到小排列,即先转储需要最大(最新)的文件. */
            /* 当flag==0x01时,按照由小到大排列;当flag == 0x10时,按照由大到小排列 */
            if (flag == 0x01)
            {
                if (u->next->file_id > u->next->next->file_id)
                {
                    v = u->next->next;
                    u->next->next = v->next;
                    v->next = u->next;
                    u->next = v;
                    p = u->next->next;
                }
            }
            else if (flag == 0x10)
            {
                if (u->next->file_id < u->next->next->file_id)
                {
                    v = u->next->next;
                    u->next->next = v->next;
                    v->next = u->next;
                    u->next = v;
                    p = u->next->next;
                }
            }
        }
    }
    u = h;
    h = h->next;
    free(u);

    return h;
}
/*******************************************************
 *
 * @brief  释放申请的缓存空间
 *
 * @param  *list_head: 文件列表数据指针
 * @retval none
 *
 *******************************************************/

void free_link(file_info_t * list_head)
{
    file_info_t *bakp;
    while (list_head)
    {
        bakp = list_head;
        list_head = list_head->next;
        free(bakp);
    }
}

/*******************************************************
 *
 * @brief  显示所有的文件列表
 *
 * @param  *list_head: 文件列表
 * @retval none
 *
 *******************************************************/

void show_link(file_info_t *list_head)
{
    while (list_head != NULL)
    { 
        /* d_type＝8表示正常的文件 */
        log_print(LOG_ERROR, "NUM:%d name:%s size:%d \n",
               list_head->file_id,
               list_head->filename,
               list_head->file_size);
        list_head = list_head->next;
    }
}

/*******************************************************
 *
 * @brief  从绝对路径中获取单独文件的名字
 *
 * @param  *full_path: 绝对路径
 * @retval char * 文件名
 *
 *******************************************************/

char *get_sigle_file_name(char *full_path)
{
    char *p;
    p = full_path + strlen(full_path);
    p--;
    while (p >= full_path)
    {
        if (*p == '/')
        {
            break;
        }
        p--;
    }
    return (p + 1);
}
/*******************************************************
 *
 * @brief  查询指定目录下的VSW文件,如果存在, 则将文件信息加入到需要拷贝的文件列表中去.
 *
 * @param  *path: 目录路径
 * @retval file_info_t * 文件列表
 * 
 *******************************************************/

file_info_t *get_org_file_info(char *path)
{
    DIR *dir;
    struct dirent *next;
    char full_path[PATH_NAME_MAX_LEN] = {0};
    char buffer[PAGE_SIZE];
    int bytes_read;
    struct stat stat_l;
    int fd;
    file_info_t *list_head = NULL;
    file_info_t *cur_file_list = NULL;
    file_info_t *tmp_file_list = NULL;

    list_head = NULL;
    dir = opendir(path);
    if (!dir)
    {
        fprintf(stderr, "get_org_file_info error,can not open %s.\n", path);
        return NULL;
    }

    /* 读取目录内文件名 */
    while ((next = readdir(dir)) != NULL)
    {
        sprintf(full_path, "%s/%s", path, next->d_name);
        if (stat(full_path, &stat_l) == 0)
        {
            /* 成功所取文件的信息 */
            if (S_ISREG(stat_l.st_mode))
            {
                /* 如果是普通文件 */
                fd = open(full_path, O_RDONLY);
                if (fd > 0)
                {
                    bytes_read = read(fd, buffer, PAGE_SIZE);
                    close(fd);
                    if (bytes_read == PAGE_SIZE)
                    {
                        if ((strcmp((const char *)(((file_head_t *)buffer)->file_head_flag), (const char *)FILE_HEAD_FLAG)) == 0)
                        {
                            tmp_file_list = malloc(sizeof(file_info_t));
                            if (tmp_file_list)
                            {
                                if (list_head == NULL)
                                {
                                    list_head = tmp_file_list;
                                }
                                else
                                {
                                    cur_file_list->next = tmp_file_list;
                                }
                                cur_file_list = tmp_file_list;
                                cur_file_list->next = NULL;
                                strcpy(cur_file_list->filename, full_path);
                                cur_file_list->file_id = ((file_head_t *)buffer)->file_index;
                                cur_file_list->file_size = stat_l.st_size;
                            }
                        }
                    }
                    /* close(fd); */
                }
            }
        }
    }

    closedir(dir);
    return list_head;
}

/*******************************************************
 *
 * @brief  递归创建目录
 *
 * @param  *path: 被创建的目录路径
 * @retval int 0:成功 负数:失败
 *
 *******************************************************/

int create_dir(const char *path)
{
    char dir_name[PATH_NAME_MAX_LEN] = {0};
    int i, len;

    strcpy(dir_name, path);
    len = strlen(dir_name);

    for (i = 1; i <= len; i++)
    {

        if ((dir_name[i] == '/') || (dir_name[i] == '\0'))
        {
            dir_name[i] = 0;
            if (access(dir_name, 0) != 0) /* 如果文件不存在 */
            {
                if (mkdir(dir_name, 0777) < 0)
                {
                    log_print(LOG_ERROR, "can not create dir %s. \n", dir_name);
                    return -1;
                }
            }
            if (i != len)
            {
                dir_name[i] = '/';
            }
        }
    }
    return 0;
}

/*******************************************************
 *
 * @brief  递归创建制定目录下的文件
 *
 * @param  *path: 被创建的文件路径
 * @retval 0:成功 负数:失败
 *
 *******************************************************/
int create_file(const char *path)
{
    char file_path[PATH_NAME_MAX_LEN] = {0};
    int i, len;

    strcpy(file_path, path);
    len = strlen(file_path);

    for (i = 1; i <= len; i++)
    {
        if (file_path[i] == '/')
        {
            file_path[i] = 0;
            if (access(file_path, 0) != 0) /* 如果文件不存在 */
            {
                /* 创建目录 */
                if (mkdir(file_path, 0766) < -1)
                {
                    log_print(LOG_ERROR, "can not create dir %s. \n",file_path);
                    return -1;
                }
            }
            file_path[i] = '/';
        }

        if (file_path[i] == 0)
        {
            /* 创建文件 */
            if (creat(file_path, 0766) < 0)
            {
                log_print(LOG_ERROR, "can not create file %s. \n", file_path);
                return -1;
            }
        }
    }

    return 0;
}
