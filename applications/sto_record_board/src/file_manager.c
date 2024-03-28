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

#define DBG_TAG "FileManager"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/errno.h>

#include "utils.h"
#include "Record_FileCreate.h"

static S_CURRENT_FILE_INFO s_current_file_info;

/*******************************************************
 *
 * @brief  释放fram空间，删除fram路径下的临时文件
 *
 * @param  none
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
#if 0
sint32_t fm_free_fram_space(S_FILE_MANAGER *fm)
{
    sint32_t disk_free_space;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };
    struct stat stat_l;
    sint32_t ret = 0;

    /* 得到板载存储器的剩余空间大小 */
    disk_free_space = get_disk_free_space(RECORD_TEMP_FILE_PATH_NAME);

    LOG_I("free fram before: %d K", disk_free_space);

    if (disk_free_space >= (TMP_FILE_MAX_SIZE + FRAM_RESERVE_SIZE)) /* 单个文件大小加预留空间的大小 */
    {
        return 0;
    }

    if(0 == strncmp(fm->latest_tmp_file_info.file_name, RECORD_TEMP_FILE_1_NAME, strlen(fm->latest_tmp_file_info.file_name)))
    {
        rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_TEMP_FILE_PATH_NAME, RECORD_TEMP_FILE_2_NAME);
        strncpy(fm->latest_tmp_file_info.file_name, RECORD_TEMP_FILE_2_NAME, sizeof(fm->latest_tmp_file_info.file_name));
    }
    else
    {
        rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_TEMP_FILE_PATH_NAME, RECORD_TEMP_FILE_1_NAME);
        strncpy(fm->latest_tmp_file_info.file_name, RECORD_TEMP_FILE_1_NAME, sizeof(fm->latest_tmp_file_info.file_name));
    }

    if(stat(full_path, &stat_l) < 0)
    {
        /* full_path对应的文件不存在,则创建文件 */
        ret = create_file(full_path);
        if(ret < 0)
        {
            LOG_E("creat %s error", full_path);
            return ret;
        }
        ret = FMWriteLatestInfo(LATEST_TMP_NAME_FILE_PATH_NAME, LATEST_TEMP_FILE_NAME, (const void *) &fm->latest_tmp_file_info, sizeof(S_LATEST_TMP_FILE_INFO));
        if(ret < 0)
        {
            LOG_E("write latest error");
        }

        LOG_I("free fram 1 after: %d K", disk_free_space);
        return ret;
    }
    else
    {
        /* full_path对应的文件存在,则删除该文件，然后再创建该文件 */
        ret = unlink(full_path);
        if(ret < 0)
        {
            LOG_E("delete %s error", full_path);
            return ret;
        }

        ret = create_file(full_path);
        if(ret < 0)
        {
            LOG_E("creat error");
            return ret;
        }

        ret = FMWriteLatestInfo(LATEST_TMP_NAME_FILE_PATH_NAME, LATEST_TEMP_FILE_NAME, (const void *) &fm->latest_tmp_file_info, sizeof(S_LATEST_TMP_FILE_INFO));
        if(ret < 0)
        {
            LOG_E("write latest error");
            return ret;
        }
        /* 得到板载存储器的剩余空间大小 */
        disk_free_space = get_disk_free_space(RECORD_TEMP_FILE_PATH_NAME);

        LOG_I("free fram 2 after: %d K", disk_free_space);
        return ret;
    }
}

#else

sint32_t fm_free_fram_space(S_FILE_MANAGER *fm)
{
    sint32_t disk_free_space;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };
    struct stat stat_l;
    sint32_t ret = 0;

    /* 得到板载存储器的剩余空间大小 */
    disk_free_space = get_disk_free_space(RECORD_TEMP_FILE_PATH_NAME);

    if (disk_free_space >= (TMP_FILE_MAX_SIZE + FRAM_RESERVE_SIZE)) /* 单个文件大小加预留空间的大小 */
    {
        return 0;
    }

    LOG_D("free fram before: %d K", disk_free_space);

    if(0 == strncmp(fm->latest_tmp_file_info.file_name, RECORD_TEMP_FILE_1_NAME, strlen(fm->latest_tmp_file_info.file_name)))
    {
        rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_TEMP_FILE_PATH_NAME, RECORD_TEMP_FILE_2_NAME);
        strncpy(fm->latest_tmp_file_info.file_name, RECORD_TEMP_FILE_2_NAME, sizeof(fm->latest_tmp_file_info.file_name));
    }
    else
    {
        rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_TEMP_FILE_PATH_NAME, RECORD_TEMP_FILE_1_NAME);
        strncpy(fm->latest_tmp_file_info.file_name, RECORD_TEMP_FILE_1_NAME, sizeof(fm->latest_tmp_file_info.file_name));
    }

    if(stat(full_path, &stat_l) < 0)
    {
        /* full_path对应的文件不存在,则创建文件 */
        ret = create_file(full_path);
        if(ret < 0)
        {
            LOG_E("creat %s error", full_path);
            return ret;
        }
        ret = FMWriteLatestInfo(LATEST_TMP_NAME_FILE_PATH_NAME, LATEST_TEMP_FILE_NAME, (const void *) &fm->latest_tmp_file_info, sizeof(S_LATEST_TMP_FILE_INFO));
        if(ret < 0)
        {
            LOG_E("write latest error");
        }

        LOG_D("free fram 1 after: %d K", disk_free_space);
        return ret;
    }
    else   /* 文件存在 */
    {
        /* full_path对应的文件存在,则删除该文件，然后再创建该文件 */
        ret = unlink(full_path);
        if(ret < 0)
        {
            LOG_E("delete %s error", full_path);
            return ret;
        }

        ret = create_file(full_path);
        if(ret < 0)
        {
            LOG_E("creat error");
            return ret;
        }

        ret = FMWriteLatestInfo(LATEST_TMP_NAME_FILE_PATH_NAME, LATEST_TEMP_FILE_NAME, (const void *) &fm->latest_tmp_file_info, sizeof(S_LATEST_TMP_FILE_INFO));
        if(ret < 0)
        {
            LOG_E("write latest error");
            return ret;
        }
        /* 得到板载存储器的剩余空间大小 */
        disk_free_space = get_disk_free_space(RECORD_TEMP_FILE_PATH_NAME);

        LOG_D("free fram 2 after: %d K", disk_free_space);
        return ret;
    }
}

#endif

/*
 * 文件追加内容
 * */
sint32_t FMWriteTmpFile(S_FILE_MANAGER *fm, const char * file_path, const void *data, size_t count)
{
    sint32_t fd = 0, ret = 0;
    fd = open(file_path, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        LOG_E("write open %s error", file_path);
        return (sint32_t)-1;
    }

    /* 写之前判断下fram剩余空间，若空间不够则创建新文件，把内容成功写入新文件后，把上次的文件删除 */
    ret = write(fd, (const void *)data, count);
    fsync(fd);
    close(fd);
    if (ret < 0)
    {
        LOG_E("write %s error", file_path);
        return (sint32_t)-1;
    }

    return 0;
}

/*
 * 从指定位置读取文件
 * */
static sint32_t FMReadTmpFile(S_FILE_MANAGER *fm, const char * file_path, void *data, size_t count)
{
    sint32_t bytes_read;
    sint32_t fd = 0;
    sint32_t lseek_count = 0;

    fd = open(file_path, O_RDONLY);
    if (fd > 0)
    {
        lseek_count = ChangeValuePositiveAndNegative(count);
//        LOG_I("lseek count %d", lseek_count);
        lseek(fd, lseek_count, SEEK_END);
        bytes_read = read(fd, data, count);
        close(fd);
        if (bytes_read == count)
        {
            return 0;
        }
        else
        {
            LOG_E("read %s size = %d", file_path, bytes_read);
            return -1;
        }
    }
    else
    {
        LOG_E("read open %s error", file_path);
        return -1;
    }

    return 0;
}

/*******************************************************
 *
 * @brief   初始化最新的临时文件信息,把最新临时文件的信息放到latest_info中.
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t FMInitLatestTmpFile(S_FILE_MANAGER *fm)
{
    S_LATEST_TMP_FILE_INFO *latest_info = &fm->latest_tmp_file_info;
    sint32_t ret = -1;
    sint32_t fd = 0;
    char full_path[PATH_NAME_MAX_LEN] = {0};

    LOG_I("latest file name: %s.", LATEST_TEMP_FILE_NAME);
    if (FMGetLatestFileInfo(fm, LATEST_TMP_NAME_FILE_PATH_NAME, LATEST_TEMP_FILE_NAME, latest_info, sizeof(S_LATEST_TMP_FILE_INFO)) == 0)
    {
        /* 正确读到了文件内容 */
        if(LATEST_DIR_FILE_HEAD_FLAG != latest_info->head_flag)
        {
            /* 如果不是文件头,则为错误状态. */
            LOG_E("file head error");
            return -1;
        }
        else
        {
            rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_TEMP_FILE_PATH_NAME, fm->latest_tmp_file_info.file_name);
            ret = FMReadTmpFile(fm, full_path, s_current_file_info.write_buf, sizeof(WRITE_BUF));
            if(ret < 0)
            {
                LOG_E("FMReadTmpFile error");
                return (sint32_t)-1;
            }
            else
            {
                LOG_D("write pos:%d", fm->current_info->write_buf->pos);
            }
        }
        latest_info->not_exsit = 0;

        return ret;
    }

    strncpy(latest_info->file_name, RECORD_TEMP_FILE_1_NAME, sizeof(latest_info->file_name));
    latest_info->head_flag = LATEST_DIR_FILE_HEAD_FLAG;
    latest_info->not_exsit = 1;

    rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_TEMP_FILE_PATH_NAME, fm->latest_tmp_file_info.file_name);
    fd = open(full_path, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        LOG_E("creat %s error", full_path);
        return (sint32_t)-1;
    }
    close(fd);

    return FMWriteLatestInfo(LATEST_TMP_NAME_FILE_PATH_NAME, LATEST_TEMP_FILE_NAME, (const void *) latest_info, sizeof(S_LATEST_TMP_FILE_INFO));
}

sint32_t FMWriteDirFile(S_FILE_MANAGER *fm, const char * dirname, const void *dir_file, size_t count)
{
    sint32_t fd = 0;
    sint32_t ret = -1;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };

    rt_mutex_take(fm->file_mutex, RT_WAITING_FOREVER);
    rt_snprintf(full_path, sizeof(full_path), "%s/%s", DIR_FILE_PATH_NAME, dirname);
    fd = open(full_path, O_RDWR);
    if (fd > 0)
    {
        lseek(fd, 0, SEEK_SET);
        ret = write(fd, dir_file, count);
        fsync(fd);
        close(fd);
        if (ret < 0)
        {
            LOG_E("%s write error", full_path);
            rt_mutex_release(fm->file_mutex);
            return (sint32_t)-1;
        }
        rt_mutex_release(fm->file_mutex);
        return 0;
    }
    else
    {
        LOG_E("open %s error", full_path);
        rt_mutex_release(fm->file_mutex);
        return -1;
    }
}

sint32_t FMReadDirFile(S_FILE_MANAGER *fm, const char * dirname, void *dir_file, size_t len)
{
    sint32_t fd = 0;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };
    sint32_t bytes_read;

    rt_mutex_take(fm->file_mutex, RT_WAITING_FOREVER);
    rt_snprintf(full_path, sizeof(full_path), "%s/%s", DIR_FILE_PATH_NAME, dirname);
    fd = open(full_path, O_RDONLY);
    if (fd > 0)
    {
        bytes_read = read(fd, (void *) dir_file, len);
        close(fd);
        if (bytes_read == len)
        {
            rt_mutex_release(fm->file_mutex);
            return 0;
        }
        else
        {
            LOG_E("read %s size = %d", full_path, bytes_read);
            rt_mutex_release(fm->file_mutex);
            return -1;
        }
    }
    else
    {
        LOG_E("read %s error", full_path);
        rt_mutex_release(fm->file_mutex);
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
    LOG_I("disk is full, total size is %d KB", used_size);
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
sint32_t fm_free_emmc_space(void)
{
    sint32_t disk_free_space;
    file_info_t *p_file_list_head = NULL, *p = NULL;
    char full_path[PATH_NAME_MAX_LEN] = {0};

    /* 得到板载存储器的剩余空间大小 */
    disk_free_space = get_disk_free_space(DIR_FILE_PATH_NAME);

    LOG_I("free emmc before: %d K", disk_free_space);
#if !FILE_MANAGER_TEST  //TODO(mingzhao)   调试删除文件时，将此段代码屏蔽
    if (disk_free_space >= RECORD_FILE_MAX_SIZE + RESERVE_SIZE) /* 单个记录文件大小加预留空间的大小 */
    {
        return 0;
    }
#endif

    /* 1.先删除/record/目录下未转存过的文件 */
    p_file_list_head = get_org_file_info(DIR_FILE_PATH_NAME);
    if(p_file_list_head != NULL)
    {
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序,先删除序号最小的文件 */
//        show_link(p_file_list_head);
        p = p_file_list_head;
        while(p != NULL)
        {
            if(p->is_save)
            {
                rt_snprintf(full_path, sizeof(full_path), "%s/%s", DIR_FILE_PATH_NAME, p->dir_name);
                unlink(full_path);   //删除目录文件

                rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, p->record_name);
                unlink(full_path);   //删除记录文件
                /* sync(); */
                LOG_I("delete save dir: %s", p->dir_name);
                LOG_I("delete save record: %s", p->record_name);
                if (get_disk_free_space(DIR_FILE_PATH_NAME) >= RECORD_FILE_MAX_SIZE + RESERVE_SIZE) //可以不加这个判断，在这里一次把转存过的删除
                {
                    LOG_I("1.free space after: %d K", disk_free_space);
                    free_link(p_file_list_head);
                    return 0;
                }
            }
            p = p->next;
        }
    }
    free_link(p_file_list_head);

    /* 2.如果还不够，则需要删除/record/目录下面未转储的文件了. */
    p_file_list_head = get_org_file_info(DIR_FILE_PATH_NAME);
    if(p_file_list_head != NULL)
    {
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序 */
        /* show_link(p_file_list_head); */
        p = p_file_list_head;
        while (p)
        {
            rt_snprintf(full_path, sizeof(full_path), "%s/%s", DIR_FILE_PATH_NAME, p->dir_name);
            unlink(full_path);   //删除目录文件

            rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, p->record_name);
            unlink(full_path);   //删除记录文件
            /* sync(); */
            LOG_I("delete not save dir: %s", p->dir_name);
            LOG_I("delete not save record: %s", p->record_name);
            if (get_disk_free_space(DIR_FILE_PATH_NAME) >= RECORD_FILE_MAX_SIZE + RESERVE_SIZE)
            {
                break;
            }
            p = p->next;
        }
    }
    else
    {
        LOG_E("fm_free_emmc null");
    }
    LOG_I("2.free space after: %d K", disk_free_space);
    free_link(p_file_list_head);
    return 0;
}

/*******************************************************
 *
 * @brief  删除目录与记录文件，释放空间
 * 
 * @param  none
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
sint32_t fm_free_record_file(S_FILE_MANAGER *fm)
{
    sint32_t disk_free_space;
    file_info_t *p_file_list_head = NULL, *p = NULL;
    char full_path[PATH_NAME_MAX_LEN] = {0};

    if(RT_NULL == fm)
    {
        return -1;
    }

    /* 得到板载存储器的剩余空间大小 */
    disk_free_space = get_disk_free_space(DIR_FILE_PATH_NAME);

    LOG_I("free emmc before: %d K", disk_free_space);

    p_file_list_head = get_org_file_info(DIR_FILE_PATH_NAME);
    if(p_file_list_head != NULL)
    {
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序 */
        p = p_file_list_head;
        while(p != NULL)
        {
            if(p->file_id == fm->current_info->file_dir->file_id)
            {
                rt_snprintf(full_path, sizeof(full_path), "%s/%s", DIR_FILE_PATH_NAME, p->dir_name);
                unlink(full_path);   //删除目录文件

                rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, p->record_name);
                unlink(full_path);   //删除记录文件
                /* sync(); */
                LOG_I("delete save dir: %s", p->dir_name);
                LOG_I("delete save record: %s", p->record_name);
                disk_free_space = get_disk_free_space(DIR_FILE_PATH_NAME);
                LOG_I("free space after: %d K", disk_free_space);
                free_link(p_file_list_head);
                return 0;
            }
            p = p->next;
        }
        free_link(p_file_list_head);
        return -2;
    }
    return -3;
}

/*******************************************************
 *
 * @brief  把当前目录文件名写入latest_dir.conf文件中
 *
 * @param  *info: 最新的目录信息
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t FMWriteLatestInfo(const char *pathname, const char *filename, const void *data, size_t size)
{
    char full_path[PATH_NAME_MAX_LEN] = {0};
    sint32_t fd = 0, ret = 0;

    rt_snprintf(full_path, sizeof(full_path), "%s/%s", pathname, filename);
    fd = open(full_path, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        LOG_E("can not modify file:%s", full_path);
        return (sint32_t)-1;
    }
    ret = write(fd, data, sizeof(S_LATEST_DIR_FILE_INFO));
    fsync(fd);
    close(fd);
    if (ret < 0)
    {
        LOG_E("%s write error", full_path);
        return (sint32_t)-1;
    }

    return 0;
}

sint32_t FMGetLatestFileInfo(S_FILE_MANAGER *fm, const char *pathname, const char *filename, void *data, size_t size)
{
    struct stat stat_l;
    char full_path[PATH_NAME_MAX_LEN] = {0};
    sint32_t fd;
    sint32_t ret;

    rt_mutex_take(fm->file_mutex, RT_WAITING_FOREVER);

    /* 获取目录文件的文件信息. */
    rt_snprintf(full_path,
             sizeof(full_path),
             "%s/%s",
             pathname,
             filename);
    ret = stat(full_path, &stat_l);
    if (ret < 0)
    {
        LOG_W("can not stat file '%s'. error code: 0x%08x",
                  full_path, ret);
        rt_mutex_release(fm->file_mutex);
        return ret;
    }

    /* 目录文件的大小应大于S_LATEST_DIR_FILE_INFO. */
    if (stat_l.st_size < size)
    {
        LOG_E("file '%s' size :%ld",
                  full_path, stat_l.st_size);
        rt_mutex_release(fm->file_mutex);
        return (sint32_t)-1;
    }

    /* 开始分析目录文件 */
    fd = open(full_path, O_RDONLY);
    if (fd > 0)
    {
        /* 正常打开文件了 */
        if (read(fd, (void *)data, size) != size)
        {
            LOG_E("error file %s size < %d", full_path, size);
            close(fd);
            rt_mutex_release(fm->file_mutex);
            return (sint32_t)-1;
        }
        else
        {
            /* 将文件关闭 */
            close(fd);
            rt_mutex_release(fm->file_mutex);
            return 0;
        }
    }
    else
    {
        /* 打开文件失败,需要特殊处理 */
        LOG_E("error, can not open file %s", full_path);
    }
    rt_mutex_release(fm->file_mutex);
    return (sint32_t)-1;
}

/*******************************************************
 *
 * @brief   初始化最新的文件信息,把最新文件的信息放到latest_info中.
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t FMInitLatestFile(S_FILE_MANAGER *fm)
{
    S_LATEST_DIR_FILE_INFO *latest_info = &fm->latest_dir_file_info;
    sint32_t fd = 0;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };
    sint32_t ret = -1;

    LOG_I("latest file name: %s.", NEW_DIR_FILE_NAME_CONF);
    if (FMGetLatestFileInfo(fm, LATEST_DIR_NAME_FILE_PATH_NAME, NEW_DIR_FILE_NAME_CONF, latest_info, sizeof(S_LATEST_DIR_FILE_INFO)) == 0)
    {
        /* 正确读到了文件内容 */
        if(LATEST_DIR_FILE_HEAD_FLAG != latest_info->head_flag)
        {
            /* 如果不是文件头,则为错误状态. */
            LOG_E("file head error");
            return -1;
        }
        else
        {
          /* 正常读到最新目录文件信息 */
          /* 读取最新的目录文件 */
          rt_snprintf(full_path, sizeof(full_path), "%s/%s", DIR_FILE_PATH_NAME, latest_info->file_name);
          fd = open(full_path, O_RDONLY);
          if(fd < 0)
          {
            LOG_E("error, can not open file %s", full_path);
            return (sint32_t)-1;
          }

          /* 正常打开文件了 */
          if (read(fd, (void *)fm->current_info->file_dir, (size_t)sizeof(SFile_Directory)) != sizeof(SFile_Directory))
          {
            LOG_E("error, %s size < %d.", full_path, sizeof(SFile_Directory));
            close(fd);
            ret = -1;
          }
          else
          {
            /* 正确读出目录文件信息 */
            close(fd);
            ret = 0;
          }
        }
        latest_info->not_exsit = 0;

        return ret;
    }

    /*
         * 目录文件名
         * 车次-车次扩充-司机号-日期-时间-序号  可以通过Get_FileName去计算目录文件铭
     * */
    strncpy(latest_info->file_name, FIRST_LATEST_DIR_NAME_NULL, sizeof(latest_info->file_name));
    latest_info->head_flag = LATEST_DIR_FILE_HEAD_FLAG;
    latest_info->dir_num = 0;
    latest_info->not_exsit = 1;
    return FMWriteLatestInfo(LATEST_DIR_NAME_FILE_PATH_NAME, NEW_DIR_FILE_NAME_CONF, (const void *) latest_info, sizeof(S_LATEST_DIR_FILE_INFO));
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

    s_current_file_info.file_dir = &s_File_Directory;
    s_current_file_info.file_head = &s_file_head;
    s_current_file_info.write_buf = &write_buf;

    fm->current_info = &s_current_file_info;

    /* 初始化最新的文件, 读出最新的目录文件信息  */
    ret = FMInitLatestFile(fm);
    if (ret < 0)
    {
        LOG_E("FMInitLatestFile error");
//        return (sint32_t)-1;
    }

    /* 初始化最新的临时文件, 读出最新的临时文件信息  */
    ret = FMInitLatestTmpFile(fm);
    if (ret < 0)
    {
        LOG_E("FMInitLatestTmpFile error");
//        return (sint32_t)-1;
    }

    /* 打印相关信息 */
    rt_kprintf("---------------- latest file ------------------\n");
    rt_kprintf("| filename:        %s\n", fm->latest_dir_file_info.file_name);
    rt_kprintf("| head_flag:       %x\n", fm->latest_dir_file_info.head_flag);
    rt_kprintf("| dir_num:         %d\n", fm->latest_dir_file_info.dir_num);
    rt_kprintf("| not_exsit:       %d\n", fm->latest_dir_file_info.not_exsit);
    rt_kprintf("| tmp:             %s\n", fm->latest_tmp_file_info.file_name);
    rt_kprintf("| not_exsit:       %d\n", fm->latest_tmp_file_info.not_exsit);
    rt_kprintf("------------------- dir file ------------------\n");
    rt_kprintf("| filename:        %s\n", fm->current_info->file_dir->ch_file_name);
    rt_kprintf("| filesize:        %d\n", fm->current_info->file_dir->u32_file_size);
    rt_kprintf("| file id:         %d\n", fm->current_info->file_dir->file_id);
    rt_kprintf("| save:            %d\n", fm->current_info->file_dir->is_save);
    rt_kprintf("| CheCi:           %d.%d.%d.%d\n",
                                    fm->current_info->file_dir->ch_checi[0], fm->current_info->file_dir->ch_checi[1],
                                    fm->current_info->file_dir->ch_checi[2], fm->current_info->file_dir->ch_checi[3]);
    rt_kprintf("| KuoChong:        %d.%d.%d.%d\n",
                                    fm->current_info->file_dir->ch_checikuochong[0], fm->current_info->file_dir->ch_checikuochong[1],
                                    fm->current_info->file_dir->ch_checikuochong[2], fm->current_info->file_dir->ch_checikuochong[3]);
    rt_kprintf("| SiJi:            %d.%d.%d.%d\n",
                                    fm->current_info->file_dir->ch_siji[0], fm->current_info->file_dir->ch_siji[1],
                                    fm->current_info->file_dir->ch_siji[2], fm->current_info->file_dir->ch_siji[3]);
    rt_kprintf("| Date:            %d.%d.%d.%d\n",
                                    fm->current_info->file_dir->ch_date[0], fm->current_info->file_dir->ch_date[1],
                                    fm->current_info->file_dir->ch_date[2], fm->current_info->file_dir->ch_date[3]);
    rt_kprintf("| Time:            %d.%d.%d.%d\n",
                                    fm->current_info->file_dir->ch_time[0], fm->current_info->file_dir->ch_time[1],
                                    fm->current_info->file_dir->ch_time[2], fm->current_info->file_dir->ch_time[3]);
    rt_kprintf("-----------------------------------------------\n");


    return (sint32_t)0;
}

sint32_t FMReadFile(S_FILE_MANAGER *fm,const char *file_path, uint32_t offset, void *data, size_t size)
{

    sint32_t bytes_read;
    sint32_t fd = 0;

    rt_mutex_take(fm->file_mutex, RT_WAITING_FOREVER);
    fd = open(file_path, O_RDONLY);
    if (fd > 0)
    {
        lseek(fd, offset, SEEK_SET);
        bytes_read = read(fd, data, size);
        close(fd);
        if (bytes_read == size)
        {
            rt_mutex_release(fm->file_mutex);
            return 0;
        }
        else
        {
            rt_mutex_release(fm->file_mutex);
            LOG_E("read %s size = %d", file_path, bytes_read);
            return -1;
        }
    }
    else
    {
        rt_mutex_release(fm->file_mutex);
        LOG_E("read open %s error", file_path);
        return -1;
    }
    rt_mutex_release(fm->file_mutex);
    return (sint32_t)0;
}

int32_t FMGetIndexFile(S_FILE_MANAGER *fm, const char *path, uint8_t file_id, file_info_t *file_info)
{
    file_info_t *p_file_list_head = NULL, *p_file = NULL;

    p_file_list_head = get_org_file_info(DIR_FILE_PATH_NAME);
    if(p_file_list_head != NULL)
    {
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序 */
        p_file = p_file_list_head;
    }
    else
    {
        LOG_W("p_file_list_head = NULL line %d", __LINE__);
        return -1;
    }

    /* 无效ID */
    if ( file_id > fm->latest_dir_file_info.dir_num )
    {
        LOG_E("id > dir_num line %d", __LINE__);
        return -1;
    } /* end if */

    /* 查找对应的目录 */
    while (p_file)
    {
        if(p_file->file_id == file_id)
        {
            break;
        }
        p_file = p_file->next;
    }

    if(p_file != NULL)
    {
        memcpy(file_info, p_file, sizeof(file_info_t));
        free_link(p_file_list_head);
        return 0;
    }
    else
    {
        LOG_E("can not find id %d dir file %d", file_id, __LINE__);
        free_link(p_file_list_head);
        return -1;
    }
    return 0;
}

