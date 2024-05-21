/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-29     zm       the first version
 */
#ifndef _SWOS2_CFG_H
#define _SWOS2_CFG_H

#define SWOS2_CFG_INIT_INFO_SIZE (18)

/* E_SWOS2_CFG_RET */
typedef enum
{
    SWOS2_CFG_OK = 0U,
    SWOS2_CFG_ERR
} E_SWOS2_CFG_RET;

E_SWOS2_CFG_RET SWOS2CfgETHIP(int slot_id);

#endif /* _SWOS2_CFG_H */
