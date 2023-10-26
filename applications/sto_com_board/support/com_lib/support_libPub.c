/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_libPub.c
 **@author: Created By Chengt
 **@date  : 2019.12.13
 **@brief : Implement the function interfaces of support_libPub
 ********************************************************************************************/

#include "support_libPub.h"

/*****************************************************************************************
 *        Local definitions
 *****************************************************************************************/

/*****************************************************************************************
 *        Local variables
 *****************************************************************************************/

/*****************************************************************************************
 *        Local functions
 *****************************************************************************************/
/********************************************************************
 功能：以指定字节内容，对内存区域进行填充操作
 参数：void *p_areaPara - 填充区域首地址
 uint8 u8_fillPara - 填充内容
 uint32 u32_lenPara - 填充长度
 返回：填充区域首地址
 *********************************************************************/
void *support_memset(void *p_areaPara, uint8 u8_fillPara, uint32 u32_lenPara)
{
    uint32 u32_fill = 0U;
    uint32 u32_count = 0U;
    uint8 * p_u8_ptr = NULL;
    uint32 u32_fillLen = u32_lenPara;

    p_u8_ptr = (uint8 *) p_areaPara;
    if (u32_fillLen > 4U) /* 长度大于uint32的情况 */
    {
        /* 构造4字节填充变量 */
        u32_fill = u8_fillPara;
        u32_fill |= u32_fill << 8;
        u32_fill |= u32_fill << 16;
        u32_fill |= u32_fill << 24;

        /* 开始区域不是4字节对齐的处理 */
        while (0 != (((uint32) p_u8_ptr) % 4))
        {
            *p_u8_ptr = u8_fillPara;
            p_u8_ptr++;
            u32_fillLen--;
        }

        /* 4字节填充 */
        u32_count = u32_fillLen / sizeof(uint32);
        while (u32_count > 0)
        {
            *((uint32 *) p_u8_ptr) = u32_fill;
            p_u8_ptr += 4;
            u32_fillLen -= 4;
            u32_count--;
        }
    }
    else
    { /* nothing */
    }

    /* 4字节以下的尾部处理 */
    while (u32_fillLen > 0)
    {
        *p_u8_ptr = u8_fillPara;
        p_u8_ptr++;
        u32_fillLen--;
    }

    return p_areaPara;
}

/********************************************************************
 功能：内存拷贝
 参数：void *p_destPara - 目的地址
 void *p_srcPara - 源地址
 uint32 u32_lenPara - 拷贝长度
 返回：目的地址
 *********************************************************************/
void *support_memcpy(void *p_destPara, const void *p_srcPara, uint32 u32_lenPara)
{
    uint8 *p_u8_dst = NULL;
    uint8 *p_u8_src = NULL;

    p_u8_dst = (uint8 *) p_destPara;
    p_u8_src = (uint8 *) p_srcPara;

    /* 复制操作 */
    while (u32_lenPara > 0U)
    {
        *(p_u8_dst++) = *(p_u8_src++);
        u32_lenPara--;
    }

    return p_destPara;
}

/********************************************************************
 功能：内存比较
 参数：void *p1Para - 内存1地址
 void *p2Para - 内存2地址
 uint32 u32_lenPara - 拷贝长度
 返回：内存1和内存2内容是否相同,TRUE:相同/FALSE:不相同。
 *********************************************************************/
BOOL support_memcmp(const void *p1Para, const void *p2Para, uint32 u32_lenPara)
{
    BOOL cmp_flg = TRUE;

    uint8 *p_u8_para1 = NULL;
    uint8 *p_u8_para2 = NULL;

    p_u8_para1 = (uint8 *) p1Para;
    p_u8_para2 = (uint8 *) p2Para;

    /* 复制操作 */
    while (u32_lenPara > 0U)
    {
        if (*(p_u8_para1++) != *(p_u8_para2++))
        {
            cmp_flg = FALSE;
            break;
        }
        u32_lenPara--;
    }

    return cmp_flg;
}

/********************************************************************
 功能：计算字符串长度
 参数：const char *p_ch_str - 字符串首地址
 返回：字符串长度
 *********************************************************************/
uint32 support_strlen(const char *p_ch_strPara)
{
    const char *p_ch_str = p_ch_strPara;

    /* 查找字符串末尾 */
    while (0 != *p_ch_str)
    {
        p_ch_str++;
    }

    /* 返回长度 */
    return p_ch_str - p_ch_strPara;
}

/********************************************************************
 功能：字符串比较
 参数：const char *p_ch_p1Para - 字符串1
 const char *p_ch_p2Para - 字符串2
 返回：0 - 字符串相等
 非0 - 不等
 *********************************************************************/
sint32 support_strcmp(const char *p_ch_p1Para, const char *p_ch_p2Para)
{
    const uint8 *p_u8_s1 = (const uint8*) p_ch_p1Para;
    const uint8 *p_u8_s2 = (const uint8*) p_ch_p2Para;
    uint8 u8_c1 = 0U, u8_c2 = 0U;

    /* 循环检查 */
    do
    {
        u8_c1 = *p_u8_s1++; /* 记录字符内容 */
        u8_c2 = *p_u8_s2++;

        if (0U == u8_c1) /* 到字符结束了 */
        {
            return u8_c1 - u8_c2; /* 返回差值 */
        }
        else
        { /* nothing */
        }
    } while (p_u8_s1 == p_u8_s2);

    return u8_c1 - u8_c2; /* 返回差值 */
}

/********************************************************************
 功能：字符串拷贝
 char *p_ch_destPara - 目的地址
 const char *p_ch_srcPara - 源地址
 返回：目的地址
 *********************************************************************/
char *support_strcpy(char *p_ch_destPara, const char *p_ch_srcPara)
{
    return support_memcpy(p_ch_destPara, p_ch_srcPara, support_strlen(p_ch_srcPara) + 1U);
}

/********************************************************************
 功能：字符串中字符查找
 char *p_ch_dest - 目的地址
 char chPara - 待查找字符
 返回：查找字符在字符串中第一次出现的地址或NULL
 *********************************************************************/
char *support_strchr(const char *p_ch_srcPara, char chPara)
{
    char *p_ch_str = (char *) p_ch_srcPara;

    /* 源字符串指针检查 */
    if (NULL == p_ch_srcPara)
    {
        return NULL;
    }
    else
    { /* nothing */
    }

    while (*p_ch_str != chPara) /* 比较 */
    {
        if (0 == *p_ch_str) /* 判断字符串结束 */
        {
            p_ch_str = NULL;
            break;
        }
        else
        {
            p_ch_str++; /* 指针移动 */
        }
    }

    return p_ch_str;
}

/********************************************************************
 功能：内存指定区域中字节查找
 const void *p_srcPara - 起始地址
 uint8 u8_bytePara - 待查找字节
 uint32 u32_lenPara - 查找区域长度
 返回：查找字节在内存指定区域中第一次出现的地址或NULL
 *********************************************************************/
void *support_memchr(const void *p_srcPara, uint8 u8_bytePara, uint32 u32_lenPara)
{
    uint8 *p_u8_byte = (uint8 *) p_srcPara;
    void *p_ret = NULL;

    /* 区域指针检查, 查找长度检查 */
    if ((NULL == p_u8_byte) || (0U == u32_lenPara))
    {
        return NULL;
    }
    else
    { /* nothing */
    }

    while (u32_lenPara > 0) /* 循环查找 */
    {
        if (*p_u8_byte == u8_bytePara) /* 字节比较 */
        {
            p_ret = (void *) p_u8_byte;
            break;
        }
        else
        {
            u32_lenPara--;
            p_u8_byte++; /* 移动指针 */
        }
    }

    return p_ret;
}
/**************************************end file*********************************************/
