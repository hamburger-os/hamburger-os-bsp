/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : support_libList.c
**@author: Created By Chengt
**@date  : 2022.04.07
**@brief : Implement the function interfaces of support_libList
********************************************************************************************/
#include <stdio.h>
#include <string.h>

#include "support_libList.h"

/*******************************************************************************************
 *        Local definitions
 *******************************************************************************************/
#define LIST_BUF_TOTALLENGTH   ( 100*1024 )      /* 缓冲区总size */
#define MAX_LISTARRY_COUNT     ( 2048U )         /* 最大队列计数 */


/* 列表管理控制 */
typedef struct
{
  uint8 a_u8_buf[LIST_BUF_TOTALLENGTH];          /* 缓冲Buffer */
  uint32 u32_bufferUsedCount;                    /* 缓冲使用计数 */

  S_ArrayList arrayList[MAX_LISTARRY_COUNT];     /* 队列控制 */
  uint8 u8_arrListUsedCount;                     /* 队列使用计数 */
} S_ListMgr;


/*******************************************************************************************
 *        Local variables
 *******************************************************************************************/
/* 全局列表管理变量 */
static S_ListMgr g_listMgr =
{
  {0, },                                         /* 缓冲Buffer */
  0,                                             /* 缓冲使用计数 */
  {{0, 0, 0, 0, {0, }, NULL}, },                 /* 队列控制 */
  0                                              /* 队列使用计数 */
};

/*******************************************************************************************
 *        Local functions
 *******************************************************************************************/
/********************************************************************
功能：创建队列数组
参数：const char *p_ch_namePara - 队列名称
      uint16 u16_itemSizePara - 队列条目大小
      uint16 u16_itemCountPara - 队列数组大小
返回：创建成功，返回指向队列管理结构的指针；失败，返回NULL。
*********************************************************************/
extern S_ArrayList * support_arraylistCreate(const char *p_ch_namePara, uint16 u16_itemSizePara, uint16 u16_itemCountPara)
{
  uint32 u32_listBufferSize = 0U;
  S_ArrayList *p_arList = NULL;

  /* 1.输入参数检查,资源检查 */
  /* 1.1.检查名称 */
  if ((NULL == p_ch_namePara) || (strlen(p_ch_namePara) > MAX_LISTARRAY_NAMESIZE))
  {
    return NULL;
  }
  else { /* nothing */ }

  /* 1.2.检查条目大小及数量输入参数 */
  if ((0U == u16_itemSizePara) || (0U == u16_itemCountPara))
  {
    return NULL;
  }
  else { /* nothing */ }

  /* 1.3.检查队列计数 */
  if (g_listMgr.u8_arrListUsedCount >= MAX_LISTARRY_COUNT)
  {
    return NULL;
  }
  else { /* nothing */ }

  /* 1.4.检查缓冲buffer资源 */
  u32_listBufferSize = (u16_itemSizePara + 2U) * u16_itemCountPara;
  if (u32_listBufferSize > (LIST_BUF_TOTALLENGTH - g_listMgr.u32_bufferUsedCount))
  {
    return NULL;
  }
  else { /* nothing */ }

  /* 2.创建队列，初始化参数 */
  p_arList = &g_listMgr.arrayList[g_listMgr.u8_arrListUsedCount];
  p_arList->p_u8_data = &g_listMgr.a_u8_buf[g_listMgr.u32_bufferUsedCount];

  g_listMgr.u8_arrListUsedCount++;
  g_listMgr.u32_bufferUsedCount += u32_listBufferSize;

  /* 3. 初始化队列参数 */
  p_arList->u16_wIdx = 0U;
  p_arList->u16_rIdx = 0U;
  p_arList->u16_itemCount = u16_itemCountPara;
  p_arList->u16_itemMaxSize = u16_itemSizePara;

  /* 4.拷贝数据 */
  memset(p_arList->a_ch_name, 0, sizeof(MAX_LISTARRAY_NAMESIZE + 1U));
  memcpy(p_arList->a_ch_name, p_ch_namePara, strlen(p_ch_namePara));

  return p_arList;
}

/********************************************************************
功能：队列数组中加入一条数据
参数：char *p_arListPara - 队列管理
      uint8 * p_u8_dataPara - 数据
      uint16 u16_lenPara - 数据长度
返回：TRUE - 添加成功；FALSE - 操作失败。
*********************************************************************/
extern BOOL support_arraylistAdd(S_ArrayList * p_arListPara, uint8 * p_u8_dataPara, uint16 u16_lenPara)
{
  uint32 u32_addr = 0U;
  uint16 u16_nextWrIdx = 0U;

  /* 1.输入参数检查 */
  /* 1.1.检查队列管理指针, 数据指针*/
  if ((NULL == p_arListPara) || (NULL == p_u8_dataPara))
  {
    return FALSE;
  }
  else { /* nothing */ }

  /* 1.2.检查数据长度 */
  if (u16_lenPara > p_arListPara->u16_itemMaxSize)
  {
    return FALSE;
  }
  else { /* nothing */ }

  /* 2. 数据加入操作  */
  /* 2.1.检查队列数组空间 */
  u16_nextWrIdx = (p_arListPara->u16_wIdx + 1U) %  p_arListPara->u16_itemCount;
  if (u16_nextWrIdx == p_arListPara->u16_rIdx)
  {
    return FALSE;  /* 队列满 */
  }
  else { /* nothing */ }

  /* 2.2.数据复制, 最前面2字节为长度  */
  u32_addr = p_arListPara->u16_wIdx * (p_arListPara->u16_itemMaxSize + 2U);
  memset(p_arListPara->p_u8_data + u32_addr + 2U, 0, p_arListPara->u16_itemMaxSize);
  memcpy(p_arListPara->p_u8_data + u32_addr + 2U, p_u8_dataPara, u16_lenPara);

  /* 2.3.记录长度 */
  *(p_arListPara->p_u8_data + u32_addr) = u16_lenPara & 0xFFU;
  *(p_arListPara->p_u8_data + u32_addr + 1U) = (uint8)(u16_lenPara >> 8U);

  /* 2.4.数组下标调整 */
  p_arListPara->u16_wIdx = u16_nextWrIdx;

  return TRUE;
}

/********************************************************************
功能：从队列数组中读取数据
参数：char *p_arListPara - 队列管理
      uint8 * p_u8_dataPara - 数据缓冲区指针
      uint16 *p_u16_dataLenPara - 数据长度变量指针
      uint16 u16_bufLenPara -- 缓冲区长度
返回：0 - 队列中没有数据；>0 - 读取数据的长度；<0 操作失败。
*********************************************************************/
extern sint32 support_arraylistGet(S_ArrayList * p_arListPara, uint8 * p_u8_dataPara, uint16 *p_u16_dataLenPara, uint16 u16_bufLenPara)
{
  uint32 u32_addr = 0U;
  uint16 u16_len = 0U;

  /* 1.输入参数检查 */
  /* 1.1.检查队列管理指针, 数据缓冲区指针*/
  if ((NULL == p_arListPara) || (NULL == p_u8_dataPara))
  {
    return -1;
  }
  else { /* nothing */ }

  /* 1.2.检查数据缓冲区指针，长度变量指针*/
  if ((NULL == p_u8_dataPara) || (NULL == p_u16_dataLenPara))
  {
      return -2;
  }
  else { /* nothing */ }

  /* 2.计算地址 */
  u32_addr = p_arListPara->u16_rIdx * (p_arListPara->u16_itemMaxSize + 2U);

  /* 3.获取长度 */
  u16_len = *((uint8 *)(p_arListPara->p_u8_data + u32_addr + 1U));
  u16_len <<= 8U;
  u16_len |= *((uint8 *)(p_arListPara->p_u8_data + u32_addr));

  /* 4.检查缓冲区长度 */
  if (u16_bufLenPara < u16_len)
  {
    return -3;
  }
  else { /* nothing */ }

  /* 5.数据输出,判断数组是否为空 */
  if (p_arListPara->u16_wIdx == p_arListPara->u16_rIdx)
  {
    return 0;
  }
  else { /* nothing */ }

  /* 6.数据复制 */
  memcpy(p_u8_dataPara, p_arListPara->p_u8_data + u32_addr + 2U, p_arListPara->u16_itemMaxSize);

  /* 7.返回数据长度 */
  *p_u16_dataLenPara = u16_len;

  /* 7.数组下标调整 */
  p_arListPara->u16_rIdx = ( p_arListPara->u16_rIdx + 1U ) % p_arListPara->u16_itemCount;

  return (sint32)u16_len;
}
/**************************************end file*********************************************/