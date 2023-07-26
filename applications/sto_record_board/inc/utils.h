/*******************************************************
 *
 * @FileName: utils.h
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 工具模块都文件.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef UTILS_H_
#define UTILS_H_

#include <rtthread.h>
#include "type.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* 路径的最大长度 */
#define PATH_NAME_MAX_LEN 256

/* 文件操作的定义 */
#ifndef SEEK_SET
#define SEEK_SET 0 /* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1 /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define SEEK_END 2 /* set file offset to EOF plus offset */
#endif


/*******************************************************
 * 宏定义
 *******************************************************/

/* VSW文件的信息,单个条目大小  字节 */
typedef struct file_info_t
{
    uint32_t file_id;                 /* 文件序号 */
    sint32_t dir_file_size;               /* 目录文件大小 */
    sint32_t record_file_size;               /* 记录文件大小 */
    char dir_name[PATH_NAME_MAX_LEN]; /* 目录文件名 */
    char record_name[PATH_NAME_MAX_LEN]; /* 记录文件名 */
    uint8_t is_save;/* 是否转存过  1：转存 0：未转存 */
    struct file_info_t *next;         /* 下一文件 */
} file_info_t;
#if 0
/*******************************************************
 * 函数声明
 *******************************************************/

/*******************************************************
 *
 * @brief  从绝对路径中获取单独文件的名字
 *
 * @param  *full_path: 绝对路径
 * @retval char * 文件名
 *
 *******************************************************/
char *get_sigle_file_name(char *full_path);

/*******************************************************
 *
 * @brief  判断EMMC存储器是满
 *
 * @param  *args: 参数描述
 * @retval sint32_t 0:不满 -1:已满
 *
 *******************************************************/
sint32_t check_disk_full(const char *name);

/*******************************************************
 *
 * @brief  获取U盘剩余空间,单位:KB
 *
 * @param  *path: U盘的文件目录
 * @retval sint32_t 剩余空间大小
 *
 *******************************************************/
sint32_t get_disk_free_space(const char *path);
#endif
/*******************************************************
 *
 * @brief  获取指定目录已使用的空间, 单位:字节
 *
 * @param  *dir_path: 文件夹的绝对路径
 * @retval 正数:文件的大小 负数:失败
 *
 *******************************************************/
sint32_t dir_size(const char *name);
#if 0
/*******************************************************
 *
 * @brief  查询指定目录下的VSW文件,如果存在, 则将文件信息
 *         加入到需要拷贝的文件列表中去.
 *
 * @param  *path: 目录路径
 * @retval file_info_t * 文件列表
 *
 *******************************************************/
file_info_t *get_org_file_info(const char *path);

/*******************************************************
 *
 * @brief  排序文件列表
 *
 * @param  *h: 需要排序的文件列表
 * @param  flag: 0x01:从小到大(最新) 0x10:从大(最新)到小
 * @retval 排序好的文件列表
 *
 *******************************************************/
file_info_t *sort_link(file_info_t *h, sint32_t flag);

/*******************************************************
 *
 * @brief  显示所有的文件列表
 *
 * @param  *list_head: 文件列表
 * @retval none
 *
 *******************************************************/
void show_link(file_info_t *list_head);

/*******************************************************
 *
 * @brief  释放申请的缓存空间
 *
 * @param  *list_head: 文件列表数据指针
 * @retval none
 *
 *******************************************************/
void free_link(file_info_t *list_head);

#endif
/*******************************************************
 *
 * @brief  递归创建目录
 *
 * @param  *path: 被创建的目录路径
 * @retval sint32_t 0:成功 负数:失败
 *
 *******************************************************/
sint32_t create_dir(const char *path);

/*******************************************************
 *
 * @brief  递归创建制定目录下的文件
 *
 * @param  *path: 被创建的文件路径
 * @retval 0:成功 负数:失败
 *
 *******************************************************/
sint32_t create_file(const char *path);

#if 0
/*******************************************************
 *
 * @brief  获取文件的大小,单位:byte.
 *
 * @param  *name: 文件的绝对路径
 * @retval 正数:文件的大小 负数:失败
 *
 *******************************************************/
uint32_t file_size(char *name);

/*******************************************************
 *
 * @brief  递归删除文件夹或者文件
 *
 * @param  *path: 文件夹或者文件路径
 * @retval 0:成功 负数:失败
 *
 *******************************************************/

int delete_file(const char *path);

/*******************************************************
 *
 * @brief  递归拷贝文件夹
 *
 * @param src 源目录
 * @param dest 目标目录
 * @retval none
 *
 *******************************************************/
void copy_files(const char *src, const char *dest);

#endif

#endif
