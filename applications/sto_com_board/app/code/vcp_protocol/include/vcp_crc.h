/* ***************************************************
 * 文件名： crc.h
 * 模  块： CRC校验模块
 *
 *
 ****************************************************/

#ifndef VCP_CRC_H
#define VCP_CRC_H

#include "common.h"

uint8 crc8_create(const uint8 * p_dat_u8, uint16 len_u16, uint8 crc_init_val_u8);
//static uint16 crc16_create(const uint8 * p_dat_u8, uint16 len, uint16 crc);
uint16 Common_CRC16(uint8* pData, uint32 nLength);
uint16  crc16l(uint8 *ptr,uint8 len);
uint32 crc32_create(const uint8 * p_dat_u8, uint16 len, uint32 crc);
uint32 generate_CRC32(const uint8 * p_dat_u8, uint32 len, uint32 oldcrc32);

uint32 crc32(const uint8 *p_dat_u8, uint32 len);

#define LMP_CRC32_INIT_VAL        (0xFFFFFFFF)             /* CRC32计算初始值 */

/**
 * 功能：计算SCTP标准的CRC32校验码，多项式0x1EDC6F41
 * 参数：*pbuffer_u8-待计算数据缓冲区指针
 * 参数：length_u32 -待计算数据字节数
 * 参数：initval_u32-初始值
 * 返回：
 */
uint32 crc32c(uint8 *pbuffer_u8, uint32 length_u32, uint32 initval_u32);

/*****************************************************
 * 功能：生成计算表
 * 参数：poly_u32    - 计算多项式
 * 返回：无
 ******************************************************/
void init_config_crc32(uint32 poly_u32);

/*********************************************************
 * 功能：计算根据配置的多项式生成计算表的CRC32校验码
 * 参数：*pbuffer_u8-待计算数据缓冲区指针
 * 参数：length_u32 -待计算数据字节数
 * 参数：initval_u32-初始值
 * 返回：
 **********************************************************/
uint32 crc32_cfged(uint8 *pbuffer_u8, uint32 length_u32, uint32 initval_u32);

#endif /** CRC_H */

