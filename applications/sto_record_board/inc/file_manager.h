/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-24     zm       the first version
 */
#ifndef APPLICATIONS_STO_RECORD_BOARD_INC_FILE_MANAGER_H_
#define APPLICATIONS_STO_RECORD_BOARD_INC_FILE_MANAGER_H_

#include <rtthread.h>
#include "type.h"

#define FILE_NAME_MAX_NUM   (40)//(24U)
#define LATEST_DIR_FILE_HEAD_FLAG (0xa5a5a5a5)

#define RECORD_BOARD_EMMC_MAX_SIZE (3600 * 1024 * 1024) //板子EMMC最大空间 单位KB  实际大小为3656M 预留了56M
#define RECORD_FILE_MAN_SIZE (20 * 1024 * 1024)         //单个记录文件大小 单位 KB  20M
#define RESERVE_SIZE         (1024 * 1024)

#define FIRST_LATEST_DIR_NAME_NULL "NULL"

#define SORT_UP 0x01   /* 升序 */
#define SORT_DOWN 0x10 /* 降序 */

#define F_MODE (S_IRUSR | S_IWUSR)                   /* 目录 */

#define RECORD_FILE_PATH_NAME "/mnt/emmc/record"
#define DIR_FILE_PATH_NAME    "/mnt/emmc/dir"
#define LATEST_DIR_NAME_FILE_PATH_NAME    "/mnt/emmc"

#define RECORD_TEMP_FILE_PATH_NAME "/mnt/fram/record" //记录临时内容的文件路径  只记录几包很少的数据
#define RECORD_TEMP_FILE_NAME      "temp.dat"

#define NEW_DIR_FILE_NAME_CONF "latest_dirname.conf"         /* 存放最新文件名的配置文件名称 */

/* 最新目录文件信息 */
typedef struct __attribute__((packed)) {
    uint32_t head_flag;
    char file_name[FILE_NAME_MAX_NUM];  /* 最新的目录文件名 */
    uint8_t dir_num;/* 目录个数 */
    uint8_t not_exsit;               /* 为1表示录音板中还没有生成文件 */
} S_LATEST_DIR_FILE_INFO;

typedef struct __attribute__((packed)) {
    S_LATEST_DIR_FILE_INFO latest_dir_file_info;
    rt_mutex_t file_mutex;
} S_FILE_MANAGER;


sint32_t FMReadDirFile(const char * dirname, void *dir_file);
sint32_t FMAppendWrite(const char *filename, const void *buffer, size_t count);
sint32_t FMInit(S_FILE_MANAGER *fm);

#endif /* APPLICATIONS_STO_RECORD_BOARD_INC_FILE_MANAGER_H_ */
