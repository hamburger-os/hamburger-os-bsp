/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-22     zm       the first version
 */

#ifndef CRC_H
#define CRC_H

#include <rtthread.h>


uint8_t crc8_create(const uint8_t * p_dat_u8, uint16_t len_u16, uint8_t crc_init_val_u8);

uint16_t Common_CRC16(uint8_t* pData, uint32_t nLength);
uint16_t  crc16l(uint8_t *ptr,uint8_t len);
uint16_t Crc16TabCCITT(unsigned char* data, unsigned int length);
uint32_t crc32_create(const uint8_t * p_dat_u8, uint16_t len, uint32_t crc);
uint32_t generate_CRC32(const uint8_t * p_dat_u8, uint32_t len, uint32_t oldcrc32);

uint32_t crc32(const uint8_t *p_dat_u8, uint32_t len);

#define LMP_CRC32_INIT_VAL        (0xFFFFFFFF)             /* CRC32计算初始值 */

/**
 * 功能：计算SCTP标准的CRC32校验码，多项式0x1EDC6F41
 * 参数：*pbuffer_u8-待计算数据缓冲区指针
 * 参数：length_u32 -待计算数据字节数
 * 参数：initval_u32-初始值
 * 返回：
 */
uint32_t crc32c(uint8_t *pbuffer_u8, uint32_t length_u32, uint32_t initval_u32);

/*****************************************************
 * 功能：生成计算表
 * 参数：poly_u32    - 计算多项式
 * 返回：无
 ******************************************************/
void init_config_crc32(uint32_t poly_u32);

/*********************************************************
 * 功能：计算根据配置的多项式生成计算表的CRC32校验码
 * 参数：*pbuffer_u8-待计算数据缓冲区指针
 * 参数：length_u32 -待计算数据字节数
 * 参数：initval_u32-初始值
 * 返回：
 **********************************************************/
uint32_t crc32_cfged(uint8_t *pbuffer_u8, uint32_t length_u32, uint32_t initval_u32);

#endif /** CRC_H */

