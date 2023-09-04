/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-06     lvhan       the first version
 */
#ifndef PACKAGES_SYSINFO_H_
#define PACKAGES_SYSINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

struct __attribute__ ((packed)) SysInfoDef
{
    uint8_t     version[22];    //系统版本号
    uint32_t    type;           //设备类型
    uint8_t     SN[16];         //设备编号,16个字符的条形码
    int32_t     cpu_temp;       //内部温度计 * 100
    uint8_t     chip_id[8];     //max31826序列号
    int32_t     chip_temp;      //max31826温度 * 100
    uint32_t    times;          //ds1682上电历时
    uint16_t    count;          //ds1682复位次数
};

//V0版本系统固化信息
struct __attribute__ ((packed)) SysInfoFixV0Def
{
    uint16_t        version;        //参数版本号:0
    uint32_t        type;           //设备类型
    uint8_t         SN[16];         //设备编号,16个字符的条形码
    uint8_t         mac[3][6];      //mac地址
    uint8_t         reserve[84];    //保留位，需保证数据结构整体为128字节
    uint32_t        crc32;          //校验值
};

void sysinfo_get(struct SysInfoDef *info);
void sysinfo_show(void);

//int8_t sysinfofix_set(void *data);//设置api不提供给应用直接调用
int8_t sysinfofix_get(void *data);
void sysinfofix_show(void);

#ifdef __cplusplus
}
#endif

#endif /* PACKAGES_SYSINFO_H_ */
