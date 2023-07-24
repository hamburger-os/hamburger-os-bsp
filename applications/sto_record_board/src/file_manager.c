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
#include <pthread.h>

#define DBG_TAG "FileManager"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_mutex_t file_mutex = RT_NULL;

#if 0
/*******************************************************
 *
 * @brief  文件管理模块初始化
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t FileManagerInit(void)
{
    sint32_t ret = 0;

    /* 创建一个动态互斥量 */
    file_mutex = rt_mutex_create("filemutex", RT_IPC_FLAG_PRIO);
    if (RT_NULL == file_mutex)
    {
        LOG_E("create file mutex failed.\n");
        return -1;
    }

    /* 创建目录 */
    ret = create_dir(RECORD_FILE_PATH_NAME);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "create_dir %s. \n", RECORD_FILE_PATH_NAME);
    }
    ret = create_dir(DIR_FILE_PATH_NAME);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "create_dir %s. \n", DIR_FILE_PATH_NAME);
    }

    /* 初始化最新的文件, 把最新文件的信息放到全局变量 g_cur_rec_file_info 中.  */
    ret = fm_init_latest_file();     //更换为 Init_FlashState
    if (ret < 0)
    {
        // log_print(LOG_ERROR, "fm_init_latest_file error. \n");
        return (sint32_t)-1;
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

    return (sint32_t)0;
}
#endif
void FM25V05_Manage_WriteEnable( void )
{

}

/*******************************************************************************************
 ** @brief: FM25V05_Manage_WriteData
 ** @param: add   dat  len
 *******************************************************************************************/
void FM25V05_Manage_WriteData( uint16_t add, uint8_t *dat, uint32_t len )
{

}

/*******************************************************************************************
 ** @brief: FM25V05_Manage_ReadData
 ** @param: add   dat  len
 *******************************************************************************************/
void FM25V05_Manage_ReadData( uint16_t add, uint8_t *dat, uint32_t len )
{

}

/**
 * \brief Writes data at the specified address on the serial firmware dataflash.
 * The page(s) to program must have been erased prior to writing. This function
 * handles page boundary crossing automatically.
 *
 * \param pS25fl1  Pointer to an S25FL1 driver instance.
 * \param pData  Data buffer.
 * \param size  Number of bytes in buffer.
 * \param address  Write address.
 *
 * \return 0 if successful; otherwise, returns ERROR_PROGRAM is there has
 * been an error during the data programming.
 */
unsigned char S25FL256S_Write( uint32_t *pData,\
                                   uint32_t size,\
                                   uint32_t address,\
                                   uint8_t  Secure )
{

}

