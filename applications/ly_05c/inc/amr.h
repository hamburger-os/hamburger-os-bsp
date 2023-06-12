/*******************************************************
 *
 * @FileName: log.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 主要定义了AMR音频格式文件的相关宏定义信息.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef AMR_H_
#define AMR_H_

/*******************************************************
 * 宏定义
 *******************************************************/

/* AMR文件头部 */
#define AMR_MAGIC_HEADER "#!AMR\n"

/* AMR一帧的最大数据长度 */
#define AMR_FRAME_MAX_LEN 32

/* AMR一帧的最大数据长度 */
#define AMR_FRAME_SRC_DATA_MAX_LEN 320

/* 采样通道数 */
#define CHANNEL_NUM 2
/* 采样频率 */
#define SMAPLE_RATE 8000
/* 采样样本数据大小 */
#define SMAPLE_BITS 16
/* 编码模式 */
extern short g_block_size[16];

#endif /* _AMR_H_*/
