
/*******************************************************
 *
 * @FileName: log.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 文件处理模块,相关接口的实现.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <stdio.h>
#include <pthread.h>

#include "log.h"
#include "file_manager.h"
#include "utils.h"

/*******************************************************
 * 全局变量
 *******************************************************/

/* 用于保护语音文件(VSW文件) */
pthread_mutex_t g_mutex_voice_file;

/* TAX通信数据 */
tax32_t g_tax32;
tax40_t g_tax40;

/* 当前录音文件信息 */
current_rec_file_info_t g_cur_rec_file_info;

/*******************************************************
 * 全局变量
 *******************************************************/

/* 校验语音头部信息 */
static sint32_t fm_check_voice_head(char *data);

/* 分析文件名为filename的语音文件,把相应信息放到g_cur_rec_file_info结构体中保存 */
static sint32_t fm_analyze_file(char *filename);

/* 判断当前目录下面有没有同名的文件, 如果有, 在文件后面加1 */
static sint32_t fm_check_dup_file_name(char *filename);

/*******************************************************
 * 函数实现
 *******************************************************/
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
    if (used_size == -1)
    {
        return 0; /* todo, 无法获取,则认为不满, 避免??? */
    }
    else
    {
    }
    log_print(LOG_INFO, "voice file size: %d kbytes.\n", used_size);
    if (used_size < YUYIN_STORAGE_MAX_SIZE)
    {
        return 0;
    }
    else
    {
    }
    return (sint32_t)-1;
}

/*******************************************************
 *
 * @brief  释放空间
 *          1.获取板载存储器的可用空间大小.
 *          2.如果剩余空间不够,先删除bak目录中的文件.
 *
 * @param  none
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
sint32_t free_space(void)
{
    sint32_t disk_free_space;
    file_info_t *p_file_list_head = NULL, *p = NULL;

    /* 得到板载存储器的剩余空间大小 */
    disk_free_space = get_disk_free_space(YUYIN_PATH_NAME);
    log_print(LOG_INFO, "free space: %d kbytes. \n", disk_free_space);
    if (check_disk_full(YUYIN_PATH_NAME) == 0)
    {
        return 0;
    }
    else
    {
    }
    /* 如果剩余空间不够,先删除bak目录中的文件 */
    /* 获取bak目录中的文件列表 */
    p_file_list_head = get_org_file_info(YUYIN_BAK_PATH_NAME);
    if (p_file_list_head != NULL)
    {
        /* 如果目录中有文件,则按照文件序号,先删除序号最小的文件 */
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序 */
        show_link(p_file_list_head);
        p = p_file_list_head;
        while (p != NULL)
        {
            unlink(p->filename);
            /* sync(); */
            log_print(LOG_INFO, "delete file '%s'. \n", p->filename);

            if (check_disk_full(YUYIN_PATH_NAME) == 0)
            {
                break;
            }
            else
            {
            }
            p = p->next;
        }
    }
    else
    {
    }
    log_print(LOG_INFO, "free space: %d K. \n", disk_free_space);
    if (p == NULL)
    {
        /* 把bak目录中的文件全部删除了,但还是不行,就需要再进一步删除/yysj/目录下面未转储的文件了. */
        free_link(p_file_list_head);
        p_file_list_head = get_org_file_info(YUYIN_PATH_NAME);
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序 */
        /* show_link(p_file_list_head); */
        p = p_file_list_head;
        while (p)
        {
            unlink(p->filename);
            /* sync(); */
            log_print(LOG_INFO, "del file: '%s'. \n", p->filename);
            if (check_disk_full(YUYIN_PATH_NAME) == 0)
            {
                break;
            }
            else
            {
            }
            p = p->next;
        }
    }
    else
    {
    }
    log_print(LOG_INFO, "free space: %d K\n", disk_free_space);
    free_link(p_file_list_head);
    return 0;
}

/*******************************************************
 *
 * @brief  从保存最新文件名的配置文件中,得到最新的语音文件名,放到Name中返回
 *
 * @param  *name: 保存最新文件名的配置文件.
 * @param  bak: 1:备份文件 0:非备份文件
 * @retval 正确得到最新文件名,返回0; 否则返回-1
 *
 *******************************************************/
sint32_t fm_get_file_name(const char *name, sint32_t bak)
{
    FILE *fp = NULL;
    sint32_t len;
    char filename[PATH_NAME_MAX_LEN] = {0};
    char full_path[PATH_NAME_MAX_LEN] = {0};

    if (bak != 0)
    {
        sprintf(full_path, "%s/%s.bak", YUYIN_PATH_NAME, NEW_FILE_NAME_CONF);
    }
    else
    {
        sprintf(full_path, "%s/%s", YUYIN_PATH_NAME, NEW_FILE_NAME_CONF);
    }

    fp = fopen(full_path, "r");
    if (fp == NULL)
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    fseek(fp, 0, SEEK_SET);
    fgets(filename, 128, fp);
    fclose(fp);

    len = strlen(filename);
    if (filename[len - 1] == '\n')
    {
        filename[len - 1] = 0;
    }
    else
    {
    }

    strcpy(name, filename);
    return 0;
}

/*******************************************************
 *
 * @brief  校验文件头部信息
 *
 * @param  *data: 文件头部信息
 * @retval 0:校验成功 非0:校验失败
 *
 *******************************************************/
static sint32_t fm_check_file_head(char *data)
{
    file_head_t *file_head = (file_head_t *)data;
    return strcmp(
        (const char *)file_head->file_head_flag,
        (const char *)FILE_HEAD_FLAG);
}

/*******************************************************
 *
 * @brief  校验语音头部信息
 *
 * @param  *data: 文件头的数据
 * @retval 0:校验成功 非0:校验失败
 *
 *******************************************************/

static sint32_t fm_check_voice_head(char *data)
{
    voice_head_t *pvoice_head = (voice_head_t *)data;
    return strcmp(pvoice_head->voice_head_flag, VOICE_HEAD_FLAG);
}

/*******************************************************
 *
 * @brief  分析文件名为filename的语音文件,把相应信息放到CurrentRecFileInfo结构体中保存
 *
 * @param  *filename: 文件名filename
 * @retval 文件正常,返回0;出现错误返回-1.
 *
 *******************************************************/
static sint32_t fm_analyze_file(char *filename)
{
    struct stat stat_l;
    char full_path[PATH_NAME_MAX_LEN] = {0};
    sint32_t fd;
    char data[PAGE_SIZE] = {0};
    off_t offset;
    sint32_t ret;
    sint32_t file_len;
    sint32_t voices_num;
    sint32_t num;
    sint32_t last_voice_head_offset;

    /* 获取录音文件的文件信息. */
    sprintf(full_path, "%s/%s", YUYIN_PATH_NAME, filename);
    ret = stat(full_path, &stat_l);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "can not stat file %s. error code: 0x%08x. \n", ret, full_path);
        return (sint32_t)-1;
    }
    else
    {
    }
    /* 如果录音文件的大小应大于(PAGE_SIZE * 2). */
    if (stat_l.st_size < (PAGE_SIZE * 2))
    {
        log_print(LOG_ERROR, "file '%s' size :%ld. \n", full_path, stat_l.st_size);
        return (sint32_t)-1;
    }
    else
    {
    }

    /* 开始分析录音文件 */
    fd = open(full_path, O_RDWR);
    if (fd > 0)
    {
        /* 正常打开文件了 */
        if (read(fd, (void *)data, (size_t)PAGE_SIZE) == PAGE_SIZE)
        {
            /* 正确读到了512字节,文件头 */
            if (fm_check_file_head(data) == 0)
            {
                /* 如果是文件头,读取文件头中的信息到 g_cur_rec_file_info 中 */
                g_cur_rec_file_info.file_index = ((file_head_t *)data)->file_index;
                strcpy(g_cur_rec_file_info.filename, filename);
                file_len = ((file_head_t *)data)->file_len * PAGE_SIZE;
                voices_num = ((file_head_t *)data)->total_voice_number[0] + ((file_head_t *)data)->total_voice_number[1] * 0x100;
                num = 0;

                while ((ret = read(fd, data, PAGE_SIZE)) > 0)
                {
                    last_voice_head_offset = lseek(fd, 0, SEEK_CUR) - PAGE_SIZE;
                    if (fm_check_voice_head(data) == 0)
                    {
                        /* 读到了语音头 */
                        offset = lseek(fd, (((voice_head_t *)data)->voice_length - 1) * PAGE_SIZE, SEEK_CUR);
                        num++;
                        if (offset == stat_l.st_size)
                        {
                            /* 可能正常的语言文件,整个语言链是正常的,还需要再进一步判断文件头是否正常 */
                            if (file_len == stat_l.st_size)
                            {
                                /* 文件头也正常,可以确定是正常的文件 */
                                g_cur_rec_file_info.voices_num = voices_num;
                                g_cur_rec_file_info.voice_index = num;
                                g_cur_rec_file_info.new_voice_head_offset = last_voice_head_offset;
                                log_print(LOG_INFO, "文件头正常, 整个语音链也正常, 是正常的文件. \n");
                            }
                            else
                            {
                                /* 文件头不正常,需要修正文件头. */
                                g_cur_rec_file_info.voices_num = voices_num + 1;
                                g_cur_rec_file_info.voice_index = num;
                                g_cur_rec_file_info.new_voice_head_offset = last_voice_head_offset;
                                fm_modify_file_head(fd);
                                log_print(LOG_WARNING, "整个语音链正常, 但是文件头不正常, 需要修正文件头. \n");
                            }
                            close(fd);
                            return 0;
                        }
                        else if (offset > stat_l.st_size)
                        {
                            /* 此种情况不会出现,记录日志中. */
                            log_print(LOG_WARNING, "warning, 偏移大于实际大小. 偏移%ld, 实际大小%ld.\n", offset, stat_l.st_size);
                        }
                        else /* offset < stat_l.st_size */
                        {
                        }
                    }
                    else
                    {
                        /* 应该是语音头的位置,但是读出来的内容不是语音头, 应该是语音数据 */
                        sint32_t fill_len;
                        char fill_buf[PAGE_SIZE];

                        log_print(LOG_INFO, "上次录音时掉电, 偏移%d, 实际大小%ld. \n", last_voice_head_offset, stat_l.st_size);
                        last_voice_head_offset = lseek(fd, 0, SEEK_CUR) - ret - PAGE_SIZE;

                        g_cur_rec_file_info.voices_num = voices_num + 1;
                        g_cur_rec_file_info.record_datalen = stat_l.st_size - last_voice_head_offset - PAGE_SIZE;
                        g_cur_rec_file_info.new_voice_head_offset = last_voice_head_offset;
                        g_cur_rec_file_info.voice_index = num;

                        /* 按PageSize字节对其填充文件 */
                        fill_len = g_cur_rec_file_info.record_datalen % PAGE_SIZE;
                        if (fill_len != 0)
                        {
                            /* 需要填充FF */
                            memset(fill_buf, 0xFF, sizeof(fill_buf));
                            fill_len = PAGE_SIZE - fill_len;
                            lseek(fd, last_voice_head_offset + g_cur_rec_file_info.record_datalen + PAGE_SIZE, SEEK_SET);
                            write(fd, fill_buf, fill_len);
                        }
                        fm_modify_voice_head(fd);
                        fm_modify_file_head(fd);
                        fsync(fd);
                        close(fd);
                        return 0;
                    }
                }
            }
            else
            {
                /* 如果不是文件头,则为错误状态. */
                log_print(LOG_ERROR, "error, 不是文件头.\n");
            }
        }
        else
        {
            /*
             * 文件大小小于512,应作废.
             * 在实际中,这种情况不会出现,因为只有文件头和语音头都写入Flash后,才会更新latest_filename.conf文件.
             */
            log_print(LOG_ERROR, "error, 文件%s的大小小于512.\n", full_path);
        }
    }
    else
    {
        /* 打开文件失败,需要特殊处理 */
        log_print(LOG_ERROR, "error, can not open file %s. \n", full_path);
    }
    return (sint32_t)-1;
}
/*******************************************************
 *
 * @brief   初始化最新的文件信息,把最新文件的信息放到全局变量g_cur_rec_file_info中.
 *          如果文件是在上次录音时候掉电,还要把文件的信息补全.
 *
 * @retval 0:成功
 *         -1:文件解析失败
 *
 *******************************************************/
sint32_t fm_init_latest_file(void)
{
    char filename[PATH_NAME_MAX_LEN] = {0};

    if (fm_get_file_name(filename, 0) == 0)
    {
        log_print(LOG_INFO, "上一个文件名是: %s. \n", filename);
        if (fm_analyze_file(filename) == 0)
        {
            g_cur_rec_file_info.not_exsit = 0;
            return 0;
        }
        else
        {
        }
    }
    else
    {
        log_print(LOG_ERROR, "error, 得不到最新的语音文件名. \n");
    }
    strcpy(g_cur_rec_file_info.filename, "0-0.0000");
    g_cur_rec_file_info.fd = -1;
    g_cur_rec_file_info.voices_num = 0;             /* 文件中的语音条数 */
    g_cur_rec_file_info.new_voice_head_offset = -1; /* 最新语音的偏移量 */
    g_cur_rec_file_info.voice_index = 0;
    g_cur_rec_file_info.file_index = 0; /* 文件的序号 */
    g_cur_rec_file_info.not_exsit = 1;

    return (sint32_t)-1;
}
/*******************************************************
 *
 * @brief 判断当前目录下面有没有同名的文件, 如果有, 在文件后面加1
 *
 * @param  *filename: 文件名
 * @retval  0:成功 1:失败
 *
 *******************************************************/
static sint32_t fm_check_dup_file_name(char *filename)
{
    char full_path[PATH_NAME_MAX_LEN] = {0};
    char bak_full_path[PATH_NAME_MAX_LEN] = {0};
    struct stat stat_l;
    sint32_t i;

    sprintf(full_path, "%s/%s", YUYIN_PATH_NAME, filename);
    sprintf(bak_full_path, "%s/%s", YUYIN_BAK_PATH_NAME, filename);
    if (stat(full_path, &stat_l) == -1)
    {
        /* 如果文件不存在重名,则返回 */
        if (stat(bak_full_path, &stat_l) == -1)
        {
            return 0;
        }
        else
        {
            log_print(LOG_ERROR, "发现重名文件:%s\n", bak_full_path);
        }
    }
    else
    {
        log_print(LOG_ERROR, "发现重名文件:%s\n", full_path);
    }

    i = 1;
    while (i < 100)
    {
        sprintf(full_path, "%s/%s%d", YUYIN_PATH_NAME, filename, i);
        sprintf(bak_full_path, "%s/%s%d", YUYIN_BAK_PATH_NAME, filename, i);
        if (stat(full_path, &stat_l) == -1)
        {
            /* 如果文件不存在重名, 则返回 */
            if (stat(bak_full_path, &stat_l) == -1)
            {
                sprintf(filename, "%s%d", filename, i);
                return 0;
            }
            else
            {
                log_print(LOG_ERROR, "发现重名文件:%s\n", bak_full_path);
            }
        }
        else
        {
            log_print(LOG_ERROR, "发现重名文件:%s\n", full_path);
        }
        i++;
    }
    return (sint32_t)-1;
}
/*******************************************************
 *
 * @brief  监控信息, 判断是否需要生成新的文件,把相应信息放到CurrentRecFileInfo结构体中保存
 *
 * @param  *args: 传输参数
 * @retval  0:产生新文件 1:追加文件
 *
 *******************************************************/
sint32_t fm_is_new(void)
{
    sint32_t train_id, driver_id, day, Month, i, j, locomotive_num;
    char filename[PATH_NAME_MAX_LEN] = {0};
    char CheCiInString[PATH_NAME_MAX_LEN] = {0};
    char tmp1[PATH_NAME_MAX_LEN] = {0}, tmp2[PATH_NAME_MAX_LEN] = {0};
    char *ptr = NULL;

    /* 先根据TAX2的信息生成文件名 */
    train_id = g_tax32.train_id[0] + g_tax32.train_id[1] * 0x100 + g_tax32.train_id[2] * 0x10000;
    for (i = 0; i < 4; i++)
    {
        if (g_tax32.train_num_type[i] != 0x20)
        {
            break;
        }

        else
        {
        }
    }

    memcpy(CheCiInString, &(g_tax32.train_num_type[i]), 4 - i);
    for (j = 0; j < 4 - i; j++)
    {
        if (!((CheCiInString[j] >= 'A' && CheCiInString[j] <= 'Z') ||
              (CheCiInString[j] >= 'a' && CheCiInString[j] <= 'z') ||
              (CheCiInString[j] >= '0' && CheCiInString[j] <= '9')))
        {
            CheCiInString[j] = '_';
        }
        else
        {
        }
    }
    sprintf(CheCiInString + (4 - i), "%d", train_id);

    locomotive_num = g_tax40.locomotive_num[0] + g_tax40.locomotive_num[1] * 0x100;
    driver_id = g_tax40.driver_id[0] + g_tax40.driver_id[1] * 0x100 + g_tax32.driver_id * 0x10000;
    day = (g_tax40.date[2] & 0x3E) >> 1;
    Month = ((g_tax40.date[2] & 0xC0) >> 6) | ((g_tax40.date[3] & 0x03) << 2);

    if (train_id == 0 || driver_id == 0 || Month == 0 || day == 0 || locomotive_num == 0 || strstr(CheCiInString, "_")) /* 2010-5-24 10:15:41*/
    {
        log_print(LOG_INFO, "无监控信息,车次:%s,司机号:%d,机车号:%d,月:%d,日:%d\n", CheCiInString, driver_id, locomotive_num, Month, day);
        /* 有2种情况:1.原来板子上没有语音文件;2.板子上有语音文件(即最新文件正常) */
        if (g_cur_rec_file_info.not_exsit)
        {
            /* 新生成文件来处理 */
            strcpy(g_cur_rec_file_info.filename, "0-0-0-0000.VSW");
            g_cur_rec_file_info.not_exsit = 0;
            log_print(LOG_INFO, "第一次生成文件!\n");
            return 0;
        }
        else
        {
            /* 如果板子上有文件,则追加处理 */
            return 1;
        }
    }
    else
    {
    }
    log_print(LOG_INFO, "车次:%s,司机号:%d,机车号:%d,月:%d,日:%d\n", CheCiInString, driver_id, locomotive_num, Month, day);

    /* 根据实时信息,产生文件名(车次-司机号.月日) */
    sprintf(filename, "%s-%d-%d-%02d%02d.VSW", CheCiInString, driver_id, locomotive_num, Month, day);

    strcpy(tmp1, filename);
    strcpy(tmp2, g_cur_rec_file_info.filename);
    ptr = strstr(tmp1, ".");
    *(ptr - 5) = 0;
    ptr = strstr(tmp2, ".");
    *(ptr - 5) = 0;

    if (strcmp(tmp1, tmp2) == 0)
    {
        /* 车次和司机号相同,追加处理 */
        return 1;
    }
    else
    {
        /* 车次和司机号不相同, 新生成文件来处理 */
        /* 判断当前目录下面有没有同名的文件, 如果有, 在文件后面加1 */
        fm_check_dup_file_name(filename);
        strcpy(g_cur_rec_file_info.filename, filename);
        return 0;
    }
}
/*******************************************************
 *
 * @brief  构造文件头.
 *
 * @retval 无
 *
 *******************************************************/
void fm_build_file_head(file_head_t *ps_FileHead)
{
    if (ps_FileHead == NULL)
    {
        return;
    }
    else
    {
    }

    memset((char *)ps_FileHead, 0xFF, sizeof(file_head_t));
    memcpy(ps_FileHead->file_head_flag, FILE_HEAD_FLAG, sizeof(FILE_HEAD_FLAG));

    ps_FileHead->file_len = 0x01;                               /* 文件长度(512页为单位),暂时写入1 */
    ps_FileHead->file_index = ++g_cur_rec_file_info.file_index; /* 文件序号 */

    ps_FileHead->trainid_type[0] = g_tax32.train_num_type[0]; /* 车次种类 */
    ps_FileHead->trainid_type[1] = g_tax32.train_num_type[1]; /* 车次种类 */
    ps_FileHead->trainid_type[2] = g_tax32.train_num_type[2]; /* 车次种类 */
    ps_FileHead->trainid_type[3] = g_tax32.train_num_type[3]; /* 车次种类 */

    ps_FileHead->train_id[0] = g_tax32.train_id[0]; /* 车次 */
    ps_FileHead->train_id[1] = g_tax32.train_id[1]; /* 车次 */
    ps_FileHead->train_id[2] = g_tax32.train_id[2];

    ps_FileHead->benbu_kehuo = g_tax40.benbu_kehuo; /* 本/补、客/货 */

    ps_FileHead->driver_id[0] = g_tax40.driver_id[0]; /* 司机号 */
    ps_FileHead->driver_id[1] = g_tax40.driver_id[1]; /* 司机号 */
    ps_FileHead->driver_id[2] = g_tax32.driver_id;    /* 司机号 */

    ps_FileHead->version = PRG_VERISON; /* 程序版本 */

    ps_FileHead->date_time[0] = g_tax40.date[0]; /* 年、月、日、时、分、秒 */
    ps_FileHead->date_time[1] = g_tax40.date[1]; /* 年、月、日、时、分、秒 */
    ps_FileHead->date_time[2] = g_tax40.date[2]; /* 年、月、日、时、分、秒 */
    ps_FileHead->date_time[3] = g_tax40.date[3]; /* 年、月、日、时、分、秒 */

    ps_FileHead->kilometer_post[0] = g_tax40.kilometer_post[0];        /* 公里标 */
    ps_FileHead->kilometer_post[1] = g_tax40.kilometer_post[1];        /* 公里标 */
    ps_FileHead->kilometer_post[2] = g_tax40.kilometer_post[2] & 0xBF; /* 公里标 */

    ps_FileHead->speed[0] = g_tax40.speed[0]; /* 实速 */
    ps_FileHead->speed[1] = g_tax40.speed[1]; /* 实速 */
    ps_FileHead->speed[2] = g_tax40.speed[2]; /* 实速 */

    ps_FileHead->real_road = g_tax32.real_road; /* 实际交路号 */

    ps_FileHead->input_road = g_tax40.section_id; /* 区段号(输入交路号) */

    ps_FileHead->station[0] = g_tax40.station_ext;        /* 车站号 */
    ps_FileHead->station[1] = g_tax32.station_ext & 0x7F; /* 车站号 */

    ps_FileHead->total_weight[0] = g_tax40.total_weight[0]; /* 总重 */
    ps_FileHead->total_weight[1] = g_tax40.total_weight[1]; /* 总重 */

    ps_FileHead->assistant_driver[0] = g_tax40.assistant_driver[0]; /* 副司机号 */
    ps_FileHead->assistant_driver[1] = g_tax40.assistant_driver[1]; /* 副司机号 */
    ps_FileHead->assistant_driver[2] = g_tax32.assistant_driver;    /* 副司机号 */

    ps_FileHead->locomotive_num[0] = g_tax40.locomotive_num[0]; /* 机车号 */
    ps_FileHead->locomotive_num[1] = g_tax40.locomotive_num[1]; /* 机车号 */

    ps_FileHead->locomotive_type[0] = g_tax40.locomotive_type;        /* 机车型号 */
    ps_FileHead->locomotive_type[1] = g_tax32.locomotive_type & 0x01; /* 机车型号 */

    ps_FileHead->total_voice_number[0] = 0; /* 总的语音条数 */
    ps_FileHead->total_voice_number[1] = 0; /* 总的语音条数 */

    /* 下面是测试代码 */
    memcpy(ps_FileHead->reserve3 + 11, (char *)(&g_tax32), sizeof(g_tax32));
    memcpy(ps_FileHead->reserve3 + 11 + sizeof(g_tax32), (char *)(&g_tax40), sizeof(g_tax40));
    memcpy(ps_FileHead->reserve3 + 11 + 72 + 24, VERISON, strlen(VERISON));
}

/*******************************************************
 *
 * @brief  构造语音头.
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
void fm_build_voice_head(voice_head_t *pvoice_head)
{
    if (pvoice_head == NULL)
    {

        return;
    }
    else
    {
    }

    memset((char *)pvoice_head, 0xFF, sizeof(voice_head_t));
    memcpy(pvoice_head->voice_head_flag, VOICE_HEAD_FLAG, sizeof(VOICE_HEAD_FLAG));

    pvoice_head->voice_length = 0x01; /* 语音长度(单位页page),暂时写入1 */

    pvoice_head->trainid_type[0] = g_tax32.train_num_type[0]; /* 车次种类 */
    pvoice_head->trainid_type[1] = g_tax32.train_num_type[1]; /* 车次种类 */
    pvoice_head->trainid_type[2] = g_tax32.train_num_type[2]; /* 车次种类 */
    pvoice_head->trainid_type[3] = g_tax32.train_num_type[3]; /* 车次种类 */

    pvoice_head->train_id[0] = g_tax32.train_id[0]; /* 车次 */
    pvoice_head->train_id[1] = g_tax32.train_id[1]; /* 车次 */
    pvoice_head->train_id[2] = g_tax32.train_id[2];

    pvoice_head->benbu_kehuo = g_tax40.benbu_kehuo; /* 本/补、客/货 */

    pvoice_head->driver_id[0] = g_tax40.driver_id[0]; /* 司机号 */
    pvoice_head->driver_id[1] = g_tax40.driver_id[1]; /* 司机号 */
    pvoice_head->driver_id[2] = g_tax32.driver_id;    /* 司机号 */

    pvoice_head->version = PRG_VERISON;

    pvoice_head->date_time[0] = g_tax40.date[0]; /* 年、月、日、时、分、秒 */
    pvoice_head->date_time[1] = g_tax40.date[1]; /* 年、月、日、时、分、秒 */
    pvoice_head->date_time[2] = g_tax40.date[2]; /* 年、月、日、时、分、秒 */
    pvoice_head->date_time[3] = g_tax40.date[3]; /* 年、月、日、时、分、秒 */

    pvoice_head->kilometer_post[0] = g_tax40.kilometer_post[0]; /* 公里标 */
    pvoice_head->kilometer_post[1] = g_tax40.kilometer_post[1]; /* 公里标 */
    pvoice_head->kilometer_post[2] = g_tax40.kilometer_post[2]; /* 公里标 */

    pvoice_head->speed[0] = g_tax40.speed[0]; /* 实速 */
    pvoice_head->speed[1] = g_tax40.speed[1]; /* 实速 */
    pvoice_head->speed[2] = g_tax40.speed[2]; /* 实速 */

    pvoice_head->real_road = g_tax32.real_road; /* 实际交路号 */

    pvoice_head->input_road = g_tax40.section_id; /* (输入交路号)区段号 */

    pvoice_head->station[0] = g_tax40.station_ext; /* 车站号 */
    pvoice_head->station[1] = g_tax32.station_ext; /* 车站号 */

    pvoice_head->total_weight[0] = g_tax40.total_weight[0]; /* 总重 */
    pvoice_head->total_weight[1] = g_tax40.total_weight[1]; /* 总重 */

    pvoice_head->assistant_driver[0] = g_tax40.assistant_driver[0]; /* 副司机号 */
    pvoice_head->assistant_driver[1] = g_tax40.assistant_driver[1]; /* 副司机号 */
    pvoice_head->assistant_driver[2] = g_tax32.assistant_driver;    /* 副司机号 */

    pvoice_head->locomotive_num[0] = g_tax40.locomotive_num[0]; /* 机车号 */
    pvoice_head->locomotive_num[1] = g_tax40.locomotive_num[1]; /* 机车号 */

    pvoice_head->locomotive_type[0] = g_tax40.locomotive_type;        /* 机车型号 */
    pvoice_head->locomotive_type[1] = g_tax32.locomotive_type & 0x01; /* 机车型号 */

    pvoice_head->channel = g_cur_rec_file_info.channel;           /* 通道号 */
    pvoice_head->voice_index = ++g_cur_rec_file_info.voice_index; /* 语音序号 */
    pvoice_head->valid_data_length = 0;                           /* 语音编码数据的有效长度(在形成语音头时,不填) */

    pvoice_head->locomotive_signal_type = g_tax40.locomotive_signal_type; /* 机车信号类型 */
    pvoice_head->signal_machine_id[0] = g_tax40.signal_machine_id[0];     /* 信号机编号 */
    pvoice_head->signal_machine_id[1] = g_tax40.signal_machine_id[1];
    pvoice_head->signal_machine_type = g_tax40.signal_machine_type; /* 信号机种类 */
    pvoice_head->monitor_state = g_tax40.device_state;              /* 监控状态 */

    /* 下面是测试代码 */
    memcpy(pvoice_head->reserve3 + 12, (char *)(&g_tax32), sizeof(g_tax32));
    memcpy(pvoice_head->reserve3 + 12 + sizeof(g_tax32), (char *)(&g_tax40), sizeof(g_tax40));
    memcpy(pvoice_head->reserve3 + 12 + 72 + 24, VERISON, strlen(VERISON));
}

/*******************************************************
 *
 * @brief  写入文件头.
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t fm_write_file_head(sint32_t fd)
{
    sint32_t ret;
    file_head_t file_head;

    fm_build_file_head(&file_head); /* 构造文件头 */
    ret = write(fd, (char *)&file_head, sizeof(file_head));
    fsync(fd);

    return ret;
}
/*******************************************************
 *
 * @brief  写入语音头.
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t fm_write_voice_head(sint32_t fd)
{
    sint32_t ret;
    voice_head_t voice_head;

    fm_build_voice_head(&voice_head); /* 构造语音头 */
    ret = write(fd, (char *)&voice_head, sizeof(voice_head));
    fsync(fd);

    return ret;
}
/*******************************************************
 *
 * @brief  修正语音头.其中要更改:
 *              1.语音长度 ( voice_length, 单位页page );
 *              2.语音的有效长度( valid_data_length );
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t fm_modify_voice_head(sint32_t fd)
{
    voice_head_t voice_head;

    lseek(fd, g_cur_rec_file_info.new_voice_head_offset, SEEK_SET);
    read(fd, (char *)&voice_head, sizeof(voice_head));

    voice_head.valid_data_length = g_cur_rec_file_info.record_datalen;
    voice_head.voice_length = g_cur_rec_file_info.record_datalen / PAGE_SIZE;
    voice_head.voice_length++;
    if (g_cur_rec_file_info.record_datalen % PAGE_SIZE != 0)
    {
        voice_head.voice_length++;
    }
    else
    {
    }

    lseek(fd, g_cur_rec_file_info.new_voice_head_offset, SEEK_SET);
    write(fd, (char *)&voice_head, sizeof(voice_head));
    return 0;
}
/*******************************************************
 *
 * @brief  修正语音头,增加语音已放音标识和时间.
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/

sint32_t fm_modify_play_flag(sint32_t fd)
{
    voice_head_t voice_head;
    sint32_t ret = 0;

    lseek(fd, g_cur_rec_file_info.new_voice_head_offset, SEEK_SET);
    ret = read(fd, (char *)&voice_head, sizeof(voice_head));
    if (ret < 0)
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    if (voice_head.voice_flag & 0x01)
    {
        log_print(LOG_INFO, "增加放音标识. \n");
        voice_head.voice_flag &= ~(0x01);
        voice_head.voice_play_time[0] = g_tax40.date[0];
        voice_head.voice_play_time[1] = g_tax40.date[1];
        voice_head.voice_play_time[2] = g_tax40.date[2];
        voice_head.voice_play_time[3] = g_tax40.date[3];

        lseek(fd, g_cur_rec_file_info.new_voice_head_offset, SEEK_SET);
        ret = write(fd, (char *)&voice_head, sizeof(voice_head));
        if (ret < 0)
        {
            return (sint32_t)-1;
        }
        else
        {
        }
    }
    return 0;
}
/*******************************************************
 *
 * @brief  修正文件头. 其中要更改:
 *         1.文件长度 (file_len, 单位页page)
 *         2.语音总条数 (total_voice_number, 单位页page)
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/

sint32_t fm_modify_file_head(sint32_t fd)
{
    struct stat stat_l;
    file_head_t file_head;

    lseek(fd, 0, SEEK_SET);
    read(fd, (char *)&file_head, sizeof(file_head));

    fstat(fd, &stat_l);
    file_head.file_len = stat_l.st_size / PAGE_SIZE;

    file_head.total_voice_number[0] = g_cur_rec_file_info.voices_num & 0xff;        /* 语音总条数 */
    file_head.total_voice_number[1] = (g_cur_rec_file_info.voices_num >> 8) & 0xff; /* 语音总条数 */

    lseek(fd, 0, SEEK_SET);
    write(fd, (char *)&file_head, sizeof(file_head));

    return 0;
}
/*******************************************************
 *
 * @brief  把当前录音文件名写入latest_filename.conf文件中
 *
 * @param  *name: 当前录音文件名
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t fm_write_name(char *name)
{
    char full_path[PATH_NAME_MAX_LEN] = {0};
    sint32_t fd = 0, ret = 0;

    sprintf(full_path, "%s/%s", YUYIN_PATH_NAME, NEW_FILE_NAME_CONF);
    fd = open(full_path, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        log_print(LOG_ERROR, "不能修改文件:%s\n", full_path);
        return (sint32_t)-1;
    }
    else
    {
    }
    ret = write(fd, name, strlen(name));
    if (ret < 0)
    {

        return (sint32_t)-1;
    }
    else
    {
    }

    fsync(fd);
    close(fd);

    return 0;
}

/*******************************************************
 *
 * @brief  文件管理模块初始化
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t fm_init(void)
{
    sint32_t ret = 0;

    /* 初始化锁 */
    pthread_mutex_init(&g_mutex_voice_file, NULL);

    /* 创建目录 */
    ret = create_dir(YUYIN_PATH_NAME);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "create_dir %s. \n", YUYIN_PATH_NAME);
    }
    else
    {
    }
    ret = create_dir(YUYIN_BAK_PATH_NAME);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "create_dir %s. \n", YUYIN_BAK_PATH_NAME);
    }
    else
    {
    }
    ret = create_dir(LOG_FILE_PATH);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "create_dir %s. \n", LOG_FILE_PATH);
    }
    else
    {
    }

    /* 初始化最新的文件, 把最新文件的信息放到全局变量 g_cur_rec_file_info 中 */
    ret = fm_init_latest_file();
    if (ret < 0)
    {
        log_print(LOG_ERROR, "fm_init_latest_file error. \n");
        return (sint32_t)-1;
    }
    else
    {
    }
    /* 打印相关信息 */
    log_print(LOG_INFO, "current rec file info: \n");
    log_print(LOG_INFO, "-----------------------------------------------\n");
    log_print(LOG_INFO, "| filename:                 %s\n", g_cur_rec_file_info.filename);
    log_print(LOG_INFO, "| fd:                       %d\n", g_cur_rec_file_info.fd);
    log_print(LOG_INFO, "| voices_num:               %d\n", g_cur_rec_file_info.voices_num);
    log_print(LOG_INFO, "| new_voice_head_offset:    0x%x\n", g_cur_rec_file_info.new_voice_head_offset);
    log_print(LOG_INFO, "| voice_index:              %d\n", g_cur_rec_file_info.voice_index);
    log_print(LOG_INFO, "| file_index:               %d\n", g_cur_rec_file_info.file_index);
    log_print(LOG_INFO, "| not_exsit:                %d\n", g_cur_rec_file_info.not_exsit);
    log_print(LOG_INFO, "-----------------------------------------------\n");

    return 0;
}
