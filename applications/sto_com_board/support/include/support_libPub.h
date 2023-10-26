/********************************************************************
 文件名：supprot_libPub.h
 模块：支持层通用库头文件，常用公用函数接口定义
 *********************************************************************/
#ifndef SUPPORT_LIBPUB_H
#define SUPPORT_LIBPUB_H

#include "common.h"

/* 以指定字节内容，对内存区域进行填充操作 */
void *support_memset(void *p_areaPara, uint8 u8_fillPara, uint32 u32_lenPara);

/* 内存拷贝 */
void *support_memcpy(void *p_destPara, const void *p_srcPara, uint32 u32_lenPara);

/* 内存比较 */
BOOL support_memcmp(const void *p1Para, const void *p2Para, uint32 u32_lenPara);

/* 计算字符串长度 */
uint32 support_strlen(const char *p_ch_strPara);

/* 字符串拷贝 */
char *support_strcpy(char *p_ch_dest, const char *p_ch_src);

/* 字条串中字符查找 */
char *support_strchr(const char *p_ch_srcPara, char chPara);

/* 内存区域中字节查找 */
void *support_memchr(const void *p_srcPara, uint8 u8_bytePara, uint32 u32_lenPara);

/* 字符串比较 */
sint32 support_strcmp(const char *p_ch_p1Para, const char *p_ch_p2Para);
#endif
