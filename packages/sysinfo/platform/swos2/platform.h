/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-01     lvhan       the first version
 */
#ifndef PACKAGES_SYSINFO_INCLUDE_SWOS2_PLATFORM_H_
#define PACKAGES_SYSINFO_INCLUDE_SWOS2_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "board.h"
#include <fal.h>

#define DBG_TAG "sysinfo"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define __PLATFORM_SWOS2__
#define SYS_VERSION "v1.1.0.0_20230831"

#define pPrintf     rt_kprintf
#define pLOG_HEX    LOG_HEX
#define pLOG_D      LOG_D
#define pLOG_I      LOG_I
#define pLOG_W      LOG_W
#define pLOG_E      LOG_E

#ifdef __cplusplus
}
#endif

#endif /* PACKAGES_SYSINFO_INCLUDE_SWOS2_PLATFORM_H_ */
