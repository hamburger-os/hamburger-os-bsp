/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-29     zm       the first version
 */
#include "swos2_cfg_init.h"

#define DBG_TAG "swos2 cfg"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <flashdb.h>

#include <string.h>
#include <stdlib.h>

#if defined(FDB_USING_KVDB)

#define KVDB_DEV_NAME  "kvdb"
#define KVDB_PART_NAME "kvdb"

#define SWOS2_ETH_CH_NUM  (3U)

typedef struct {
    char *field_name;    /* 字段名 */
    char *field_info;    /* 字段信息 */
} SWOS2_KVDB_INFO;       /* 数据库信息 */

typedef struct {
    SWOS2_KVDB_INFO info[3];
} SWOS2_ETH_INFO;       /* 网络通道信息 */

typedef struct {
    struct fdb_kvdb kvdb;      /* 数据库句柄 */

    uint8_t cfg_eth_num;       /* 需要配置的网络通道个数 */
    SWOS2_ETH_INFO *eth_info;  /* 网络通道信息 */
    uint8_t is_change;         /* 1：修改网络配置 0：未修改网络配置 */
} SWOS2_ETH_CFG;

static SWOS2_ETH_INFO swos2_eth_info[SWOS2_ETH_CH_NUM] = {
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.7",
    },
    {
        .info[0].field_name = "e1_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e1_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e1_ip",
        .info[2].field_info = "192.168.1.8",
    },
    {
        .info[0].field_name = "e2_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e2_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e2_ip",
        .info[2].field_info = "192.168.1.9",
    },
};

static SWOS2_ETH_CFG swos2_eth_cfg;

rt_err_t SWOS2CfgETHIP(void)
{
    fdb_err_t err = FDB_NO_ERR;
    bool not_formatable = RT_TRUE;
    char *return_value, temp_data[SWOS2_CFG_INIT_INFO_SIZE+1] = { 0 };
    uint8_t i = 0, j = 0;

    if (swos2_eth_cfg.kvdb.parent.init_ok)
    {
        LOG_I("fdb_kvdb_deinit");
        /* Close the last open KVDB */
        fdb_kvdb_deinit(&swos2_eth_cfg.kvdb);
    }

    /* 初始化数据 */
    rt_memset(&swos2_eth_cfg.kvdb, 0, sizeof(swos2_eth_cfg.kvdb));
    swos2_eth_cfg.cfg_eth_num = SWOS2_ETH_CH_NUM;
    swos2_eth_cfg.eth_info = swos2_eth_info;

#ifdef FDB_USING_FAL_MODE

    /* fdb partition */
    if (fal_partition_find(KVDB_PART_NAME) == NULL)
    {
        LOG_E("The '%s' partition not found!", KVDB_PART_NAME);
        return -RT_ERROR;
    }
#else
    LOG_E("fdb not using fal mode");
    return -RT_EIO;
#endif

    fdb_kvdb_control(&swos2_eth_cfg.kvdb, FDB_KVDB_CTRL_SET_NOT_FORMAT, &not_formatable);
    err = fdb_kvdb_init(&swos2_eth_cfg.kvdb, KVDB_DEV_NAME, KVDB_PART_NAME, NULL, NULL);

    if (err != FDB_NO_ERR)
    {
        LOG_E("KVDB '%s' not found. Probe failed!", KVDB_DEV_NAME);
        return -RT_ERROR;
    }
    else
    {
        /* success */
        if (!swos2_eth_cfg.kvdb.parent.init_ok)
        {
            LOG_E("kvdb init error. dev %s part %s", KVDB_DEV_NAME, KVDB_PART_NAME);
            return -RT_ERROR;
        }

        for(i = 0; i < swos2_eth_cfg.cfg_eth_num; i++)
        {
            for(j = 0; j < 3; j++)
            {
                /* 获取字段对应的值 */
                return_value = fdb_kv_get(&swos2_eth_cfg.kvdb, swos2_eth_cfg.eth_info[i].info[j].field_name);
                if (return_value != NULL)
                {
                    strncpy(temp_data, return_value, (sizeof(temp_data) - 1));
                    /* 数据库种存储的信息与预期信息不一致 */
                    if(strncmp(swos2_eth_cfg.eth_info[i].info[j].field_info, temp_data, sizeof(temp_data)) != 0)
                    {
                        LOG_E("%s target value %s current value: %s", swos2_eth_cfg.eth_info[i].info[j].field_name, swos2_eth_cfg.eth_info[i].info[j].field_info, temp_data);
                        /* 修改数据库中的信息 */
                        err = fdb_kv_set(&swos2_eth_cfg.kvdb, swos2_eth_cfg.eth_info[i].info[j].field_name, swos2_eth_cfg.eth_info[i].info[j].field_info);
                        if(err != FDB_NO_ERR)
                        {
                            LOG_E("set %s error", swos2_eth_cfg.eth_info[i].info[j].field_name);
                            return -RT_ERROR;
                        }
                        else
                        {
                            /* 修改成功后读取信息，打印 */
                            return_value = fdb_kv_get(&swos2_eth_cfg.kvdb, swos2_eth_cfg.eth_info[i].info[j].field_name);
                            if (return_value != NULL) {
                                swos2_eth_cfg.is_change = 1;
                                strncpy(temp_data, return_value, (sizeof(temp_data) - 1));
                                LOG_I("change %s value: %s", temp_data);
                            }
                            else
                            {
                                LOG_E("change after get %s error", swos2_eth_cfg.eth_info[i].info[j].field_name);
                                return -RT_EEMPTY;
                            }

                        }
                    }
                }
                else
                {
                    LOG_E("get %s error", swos2_eth_cfg.eth_info[i].info[j].field_name);
                    return -RT_EEMPTY;
                }
            }
        }
    }

    if(1 == swos2_eth_cfg.is_change)
    {
        rt_thread_mdelay(500);
        rt_hw_cpu_reset();
        rt_thread_mdelay(500);
    }
    return RT_EOK;
}

#endif /* defined(FDB_USING_KVDB) */

