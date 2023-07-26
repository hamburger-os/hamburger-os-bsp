/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-24     zm       the first version
 */
#include "file_manager.h"
#include <rtthread.h>

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "utils.h"
#include "Record_FileCreate.h"

#define DBG_TAG "FileManager"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

sint32_t FMReadDirFile(const char * dirname, void *dir_file)
{
    sint32_t fd = 0;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };
    sint32_t bytes_read;

    snprintf(full_path, sizeof(full_path), "%s/%s", DIR_FILE_PATH_NAME, dirname);
    fd = open(full_path, O_RDONLY);
    if (fd > 0)
    {
        bytes_read = read(fd, (void *) dir_file, (size_t) sizeof(SFile_Directory));
        close(fd);
        if (bytes_read == sizeof(SFile_Directory))
        {
            return 0;
        }
        else
        {
            LOG_E("read dir %s size =  %%", full_path, bytes_read);
            return -1;
        }
    }
    else
    {
        LOG_E("read dir %s error", full_path);
        return -1;
    }
}

sint32_t FMAppendWrite(const char *filename, const void *buffer, size_t count)
{
    sint32_t fd = 0;
    sint32_t ret = -1;

    fd = open(filename, O_RDWR);
    if(fd < 0)
    {
        LOG_E("open %s error", filename);
        return -1;
    }

    /* 定位文件指针到文件末尾 */
    lseek(fd, 0, SEEK_END);
    /* 追加内容 */
    ret = write(fd, buffer, count);
    fsync(fd);
    close(fd);
    if (ret < 0)
    {
        LOG_E("%s write error", filename);
        return (sint32_t)-1;
    }
    return 0;
}

/*******************************************************
 *
 * @brief  判断板载存储器存储器是满
 *
 * @param  *args: 参数描述
 * @retval sint32_t 0:不满 -1:已满
 *
 *******************************************************/
sint32_t check_disk_full(const char *name)
{
    sint32_t used_size;

    used_size = dir_size(name) / 1000;
    if (used_size < RECORD_BOARD_EMMC_MAX_SIZE)
    {
        return 0;
    }
    LOG_I("disk is full, total size is %d KB.\n", used_size);
    return (sint32_t)-1;
}

/*******************************************************
 *
 * @brief  删除目录与记录文件，释放空间
 *
 * @param  none
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
sint32_t fm_free_space(void)
{
    sint32_t disk_free_space;
    file_info_t *p_file_list_head = NULL, *p = NULL;

    /* 得到板载存储器的剩余空间大小 */
    disk_free_space = get_disk_free_space(RECORD_FILE_PATH_NAME);

    LOG_I("free space: %d K. \n", disk_free_space);
    if (disk_free_space >= RECORD_FILE_MAN_SIZE + RESERVE_SIZE) /* 单个记录文件大小加预留空间的大小 */
    {
        return 0;
    }

    /* 1.先删除/record/目录下未转存过的文件 */
    p_file_list_head = get_org_file_info(RECORD_FILE_PATH_NAME);
    if(p_file_list_head != NULL)
    {
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序,先删除序号最小的文件 */
//        show_link(p_file_list_head);
        p = p_file_list_head;
        while(p != NULL)
        {
            if(p->is_save)
            {
                unlink(p->dir_name);   //删除目录文件
                unlink(p->record_name);   //删除记录文件
                /* sync(); */
                LOG_I("delete dir: '%s' record: %s", p->dir_name, p->record_name);
                if (get_disk_free_space(RECORD_FILE_PATH_NAME) >= RECORD_FILE_MAN_SIZE + RESERVE_SIZE) //可以不加这个判断，在这里一次把转存过的删除
                {
                    LOG_I("free space: %d K\n", disk_free_space);
                    free_link(p_file_list_head);
                    return 0;
                }
            }
            p = p->next;
        }
    }
    free_link(p_file_list_head);

    /* 2.如果还不够，则需要删除/record/目录下面未转储的文件了. */
    p_file_list_head = get_org_file_info(RECORD_FILE_PATH_NAME);
    if(p_file_list_head != NULL)
    {
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序 */
        /* show_link(p_file_list_head); */
        p = p_file_list_head;
        while (p)
        {
            unlink(p->dir_name);   //删除目录文件
            unlink(p->record_name);   //删除记录文件
            /* sync(); */
            LOG_I("delete not save dir: '%s' record: %s", p->dir_name, p->record_name);
            if (get_disk_free_space(RECORD_FILE_PATH_NAME) >= RECORD_FILE_MAN_SIZE + RESERVE_SIZE)
            {
                break;
            }
            p = p->next;
        }
    }
    LOG_I("free space: %d K\n", disk_free_space);
    free_link(p_file_list_head);
    return 0;
}

/*******************************************************
 *
 * @brief  把当前录音文件名写入latest_dir.conf文件中
 *
 * @param  *name: 当前录音文件名
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t FMWriteLatestInfo(const S_LATEST_DIR_FILE_INFO *info)
{
    char full_path[PATH_NAME_MAX_LEN] = {0};
    sint32_t fd = 0, ret = 0;

    snprintf(full_path, sizeof(full_path), "%s/%s", LATEST_DIR_NAME_FILE_PATH_NAME, NEW_DIR_FILE_NAME_CONF);
    fd = open(full_path, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        LOG_E("不能修改文件:%s", full_path);
        return (sint32_t)-1;
    }
    ret = write(fd, (const void *)info, sizeof(S_LATEST_DIR_FILE_INFO));
    fsync(fd);
    close(fd);
    if (ret < 0)
    {
        LOG_E("%s write error", full_path);
        return (sint32_t)-1;
    }

    return 0;
}

static sint32_t FMGetLatestFileInfo(const char *filename, void *data)
{
    struct stat stat_l;
    char full_path[PATH_NAME_MAX_LEN] = {0};
    sint32_t fd;
    sint32_t ret;

    /* 获取目录文件的文件信息. */
    snprintf(full_path,
             sizeof(full_path),
             "%s/%s",
             LATEST_DIR_NAME_FILE_PATH_NAME,
             filename);
    ret = stat(full_path, &stat_l);
    if (ret < 0)
    {
        LOG_W("can not stat file '%s'. error code: 0x%08x. \n",
                  full_path, ret);
        return ret;
    }

    /* 目录文件的大小应大于S_LATEST_DIR_FILE_INFO. */
    if (stat_l.st_size < sizeof(S_LATEST_DIR_FILE_INFO))
    {
        LOG_E("file '%s' size :%ld. \n",
                  full_path, stat_l.st_size);
        return (sint32_t)-1;
    }

    /* 开始分析目录文件 */
    fd = open(full_path, O_RDWR);
    if (fd > 0)
    {
        /* 正常打开文件了 */
        if (read(fd, (void *)data, (size_t)sizeof(S_LATEST_DIR_FILE_INFO)) != sizeof(S_LATEST_DIR_FILE_INFO))
        {
            LOG_E("error, 文件%s的大小小于%d.\n", full_path, sizeof(S_LATEST_DIR_FILE_INFO));
            close(fd);
            return (sint32_t)-1;
        }
        else
        {
            close(fd);
            return 0;
        }
    }
    else
    {
        /* 打开文件失败,需要特殊处理 */
        LOG_E("error, can not open file %s. \n", full_path);
    }
    return (sint32_t)-1;
}

/*******************************************************
 *
 * @brief   初始化最新的文件信息,把最新文件的信息放到latest_info中.
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t FMInitLatestFile(S_LATEST_DIR_FILE_INFO *latest_info)
{
    LOG_I("latest file name: %s.", NEW_DIR_FILE_NAME_CONF);
    if (FMGetLatestFileInfo(NEW_DIR_FILE_NAME_CONF, latest_info) == 0)
    {
        /* 正确读到了文件内容 */
        if(LATEST_DIR_FILE_HEAD_FLAG != latest_info->head_flag)
        {
            /* 如果不是文件头,则为错误状态. */
            LOG_E("error, 不是文件头.\n");
        }
        latest_info->not_exsit = 0;

        return 0;
    }

    /*
         * 目录文件名
         * 车次-车次扩充-司机号-日期-时间-序号  可以通过Get_FileName去计算目录文件铭
     * */
    strcpy(latest_info->file_name, "xxxx-xxxx-xxxx-xxxx-000");
    latest_info->head_flag = LATEST_DIR_FILE_HEAD_FLAG;
    latest_info->dir_num = 0;
    latest_info->not_exsit = 1;
    return FMWriteLatestInfo(latest_info);
}

/*******************************************************
 *
 * @brief  文件管理模块初始化
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t FMInit(S_FILE_MANAGER *fm)
{
    if(RT_NULL == fm)
    {
        return -1;
    }
    sint32_t ret = 0;

    memset(fm, 0, sizeof(S_FILE_MANAGER));

    /* 创建一个动态互斥量 */
    fm->file_mutex = rt_mutex_create("filemutex", RT_IPC_FLAG_PRIO);
    if (RT_NULL == fm->file_mutex)
    {
        LOG_E("create file mutex failed");
        return -1;
    }

    /* 创建目录 */
    ret = create_dir(RECORD_FILE_PATH_NAME);
    if (ret < 0)
    {
        LOG_E("create_dir %s", RECORD_FILE_PATH_NAME);
    }
    ret = create_dir(DIR_FILE_PATH_NAME);
    if (ret < 0)
    {
        LOG_E("create_dir %s", DIR_FILE_PATH_NAME);
    }

    ret = create_dir(RECORD_TEMP_FILE_PATH_NAME);
    if (ret < 0)
    {
        LOG_E("create_dir %s", RECORD_TEMP_FILE_PATH_NAME);
    }

    /* 初始化最新的文件, 读出最新的目录文件信息  */
    ret = FMInitLatestFile(&fm->latest_dir_file_info);
    if (ret < 0)
    {
        LOG_E("fm_init_latest_file error. \n");
        return (sint32_t)-1;
    }
    /* 打印相关信息 */
    LOG_I("current rec file info:");
    LOG_I("-----------------------------------------------");
    LOG_I("| filename:                 %s", fm->latest_dir_file_info.file_name);
    LOG_I("| head_flag:                %x", fm->latest_dir_file_info.head_flag);
    LOG_I("| dir_num:                  %d", fm->latest_dir_file_info.dir_num);
    LOG_I("| not_exsit:                %d", fm->latest_dir_file_info.not_exsit);
    LOG_I("-----------------------------------------------");
    return (sint32_t)0;
}

