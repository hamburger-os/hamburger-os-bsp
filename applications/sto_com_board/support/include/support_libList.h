/********************************************************************
 文件名：support_list.h
 模块：支持层头文件，定义链表相关定义及接口
 *********************************************************************/
#ifndef SUPPORT_LIBLIST_H
#define SUPPORT_LIBLIST_H

#include "common.h"

#define MAX_LISTARRAY_NAMESIZE (24U)       /* 队列名称大小 */

typedef struct /* 队列数组控制 */
{
    uint16 u16_wIdx; /* 写数据下标 */
    uint16 u16_rIdx; /* 读数据下标 */
    uint16 u16_itemCount; /* 数组大小  */
    uint16 u16_itemMaxSize; /* 数据项最大size */
    char a_ch_name[MAX_LISTARRAY_NAMESIZE + 1U]; /* 名称 */
    uint8 *p_u8_data; /* 数据指针 */
} S_ArrayList;

/* 创建队列数组 */
extern S_ArrayList * support_arraylistCreate(const char *p_ch_namePara, uint16 u16_itemSizePara, uint16 u16_itemCountPara);
/* 队列数组中加入一条数据 */
extern BOOL support_arraylistAdd(S_ArrayList * p_arListPara, uint8 * p_u8_dataPara, uint16 u16_lenPara);
/* 从队列数组中读取数据 */
extern sint32 support_arraylistGet(S_ArrayList * p_arListPara, uint8 * p_u8_dataPara, uint16 *p_u16_dataLenPara,
        uint16 u16_bufLenPara);

#endif
