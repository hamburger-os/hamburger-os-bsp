/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-29     zm       the first version
 */
#include "linklib/inc/swos2_cfg_init.h"

#define DBG_TAG "swos2 cfg"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

#include <flashdb.h>

#include <string.h>
#include <stdlib.h>

#include "linklib/inc/if_gpio.h"

#if defined(FDB_USING_KVDB)

#define SWOS2_ETH_INFO_NUM (4U)

#define KVDB_DEV_NAME  "kvdb"
#define KVDB_PART_NAME "kvdb"

typedef struct {
    char *field_name;    /* 字段名 */
    char *field_info;    /* 字段信息 */
} SWOS2_KVDB_INFO;       /* 数据库信息 */

typedef struct {
    SWOS2_KVDB_INFO info[SWOS2_ETH_INFO_NUM];
} SWOS2_ETH_INFO;       /* 网络通道信息 */

typedef struct {
    struct fdb_kvdb kvdb;      /* 数据库句柄 */

    uint8_t cfg_eth_num;       /* 需要配置的网络通道个数 */
    SWOS2_ETH_INFO *eth_info;  /* 网络通道信息 */
    uint8_t is_change;         /* 1：修改网络配置 0：未修改网络配置 */
} SWOS2_ETH_CFG;

/* I系通信1子板 */
static SWOS2_ETH_INFO com1_child_I_eth_info[2] = {
    {
        .info[0].field_name = "e_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e_ip",
        .info[2].field_info = "192.168.1.20",

        .info[3].field_name = "e_mac",
        .info[3].field_info = "4C-53-57-00-03-21",
    },
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.21",

        .info[3].field_name = "e0_mac",
        .info[3].field_info = "4C-53-57-00-03-11",
    },
};
/* II系通信1子板 */
static SWOS2_ETH_INFO com1_child_II_eth_info[2] = {
    {
        .info[0].field_name = "e_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e_ip",
        .info[2].field_info = "192.168.1.22",

        .info[3].field_name = "e_mac",
        .info[3].field_info = "4C-53-57-00-03-22",
    },
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.23",

        .info[3].field_name = "e0_mac",
        .info[3].field_info = "4C-53-57-00-03-11",
    },
};

/* I系通信1底板 */
static SWOS2_ETH_INFO com1_load_I_eth_info[2] = {
    {
        .info[0].field_name = "e_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e_ip",
        .info[2].field_info = "192.168.1.24",

        .info[3].field_name = "e_mac",
        .info[3].field_info = "4C-53-57-00-10-30",
    },
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.25",

        .info[3].field_name = "e0_mac",
        .info[3].field_info = "4C-53-57-00-10-40",
    },
};

/* II系通信1底板 */
static SWOS2_ETH_INFO com1_load_II_eth_info[2] = {
    {
        .info[0].field_name = "e_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e_ip",
        .info[2].field_info = "192.168.1.26",

        .info[3].field_name = "e_mac",
        .info[3].field_info = "4C-53-57-00-10-31",
    },
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.27",

        .info[3].field_name = "e0_mac",
        .info[3].field_info = "4C-53-57-00-10-41",
    },
};

/* I系通信2子板 */
static SWOS2_ETH_INFO com2_child_I_eth_info[3] = {
    {
        .info[0].field_name = "e_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e_ip",
        .info[2].field_info = "192.168.1.28",

        .info[3].field_name = "e_mac",
        .info[3].field_info = "4C-53-57-00-02-30",
    },
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.29",

        .info[3].field_name = "e0_mac",
        .info[3].field_info = "4C-53-57-00-09-D1",
    },
    {
        .info[0].field_name = "e1_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e1_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e1_ip",
        .info[2].field_info = "192.168.1.30",

        .info[3].field_name = "e1_mac",
        .info[3].field_info = "4C-53-57-00-09-C1",
    },
};

/* II系通信2子板 */
static SWOS2_ETH_INFO com2_child_II_eth_info[3] = {
    {
        .info[0].field_name = "e_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e_ip",
        .info[2].field_info = "192.168.1.31",

        .info[3].field_name = "e_mac",
        .info[3].field_info = "4C-53-57-00-02-31",
    },
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.32",

        .info[3].field_name = "e0_mac",
        .info[3].field_info = "4C-53-57-00-09-D2",
    },
    {
        .info[0].field_name = "e1_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e1_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e1_ip",
        .info[2].field_info = "192.168.1.33",

        .info[3].field_name = "e1_mac",
        .info[3].field_info = "4C-53-57-00-09-C2",
    },
};

/* I系通信2底板 */
static SWOS2_ETH_INFO com2_load_I_eth_info[2] = {
    {
        .info[0].field_name = "e_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e_ip",
        .info[2].field_info = "192.168.1.34",

        .info[3].field_name = "e_mac",
        .info[3].field_info = "4C-53-57-00-01-23",
    },
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.35",

        .info[3].field_name = "e0_mac",
        .info[3].field_info = "4C-53-57-00-07-A3",
    },
};

/* II系通信2底板 */
static SWOS2_ETH_INFO com2_load_II_eth_info[2] = {
    {
        .info[0].field_name = "e_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e_ip",
        .info[2].field_info = "192.168.1.36",

        .info[3].field_name = "e_mac",
        .info[3].field_info = "4C-53-57-00-01-24",
    },
    {
        .info[0].field_name = "e0_gw",
        .info[0].field_info = "192.168.1.1",

        .info[1].field_name = "e0_mask",
        .info[1].field_info = "255.255.255.0",

        .info[2].field_name = "e0_ip",
        .info[2].field_info = "192.168.1.37",

        .info[3].field_name = "e0_mac",
        .info[3].field_info = "4C-53-57-00-17-A4",
    },
};

static SWOS2_ETH_CFG swos2_eth_cfg;

E_SWOS2_CFG_RET SWOS2CfgETHIP(int slot_id)
{
    fdb_err_t err = FDB_NO_ERR;
    bool not_formatable = RT_TRUE;
    char *return_value, temp_data[SWOS2_CFG_INIT_INFO_SIZE + 1] = { 0 };
    uint8_t i = 0, j = 0;

    if (swos2_eth_cfg.kvdb.parent.init_ok)
    {
        LOG_I("fdb_kvdb_deinit");
        /* Close the last open KVDB */
        fdb_kvdb_deinit(&swos2_eth_cfg.kvdb);
    }

    /* 初始化数据 */
    rt_memset(&swos2_eth_cfg.kvdb, 0, sizeof(swos2_eth_cfg.kvdb));

    switch(slot_id)
    {
        case E_SLOT_ID_1:
            swos2_eth_cfg.eth_info = com1_load_I_eth_info;
            swos2_eth_cfg.cfg_eth_num = sizeof(com1_load_I_eth_info) / sizeof(SWOS2_ETH_INFO);
        break;
        case E_SLOT_ID_2:
            swos2_eth_cfg.eth_info = com1_child_I_eth_info;
            swos2_eth_cfg.cfg_eth_num = sizeof(com1_child_I_eth_info) / sizeof(SWOS2_ETH_INFO);
        break;
        case E_SLOT_ID_3:
            swos2_eth_cfg.eth_info = com2_load_I_eth_info;
            swos2_eth_cfg.cfg_eth_num = sizeof(com2_load_I_eth_info) / sizeof(SWOS2_ETH_INFO);
        break;
        case E_SLOT_ID_4:
            swos2_eth_cfg.eth_info = com2_child_I_eth_info;
            swos2_eth_cfg.cfg_eth_num = sizeof(com2_load_I_eth_info) / sizeof(SWOS2_ETH_INFO);
        break;
        case E_SLOT_ID_5:
            swos2_eth_cfg.eth_info = com1_load_II_eth_info;
            swos2_eth_cfg.cfg_eth_num = sizeof(com1_load_II_eth_info) / sizeof(SWOS2_ETH_INFO);
        break;
        case E_SLOT_ID_6:
            swos2_eth_cfg.eth_info = com1_child_II_eth_info;
            swos2_eth_cfg.cfg_eth_num = sizeof(com1_child_II_eth_info) / sizeof(SWOS2_ETH_INFO);
        break;
        case E_SLOT_ID_7:
            swos2_eth_cfg.eth_info = com2_load_II_eth_info;
            swos2_eth_cfg.cfg_eth_num = sizeof(com2_load_II_eth_info) / sizeof(SWOS2_ETH_INFO);
        break;
        case E_SLOT_ID_8:
            swos2_eth_cfg.eth_info = com2_child_II_eth_info;
            swos2_eth_cfg.cfg_eth_num = sizeof(com2_child_II_eth_info) / sizeof(SWOS2_ETH_INFO);
        break;
        default:
            LOG_E("slot_id %d error", slot_id);
            return SWOS2_CFG_ERR;

    }

#ifdef FDB_USING_FAL_MODE

    /* fdb partition */
    if (fal_partition_find(KVDB_PART_NAME) == NULL)
    {
        LOG_E("The '%s' partition not found!", KVDB_PART_NAME);
        return SWOS2_CFG_ERR;
    }
#else
    LOG_E("fdb not using fal mode");
    return SWOS2_CFG_ERR;
#endif

    fdb_kvdb_control(&swos2_eth_cfg.kvdb, FDB_KVDB_CTRL_SET_NOT_FORMAT, &not_formatable);
    err = fdb_kvdb_init(&swos2_eth_cfg.kvdb, KVDB_DEV_NAME, KVDB_PART_NAME, NULL, NULL);

    if (err != FDB_NO_ERR)
    {
        LOG_E("KVDB '%s' not found. Probe failed!", KVDB_DEV_NAME);
        return SWOS2_CFG_ERR;
    }
    else
    {
        /* success */
        if (!swos2_eth_cfg.kvdb.parent.init_ok)
        {
            LOG_E("kvdb init error. dev %s part %s", KVDB_DEV_NAME, KVDB_PART_NAME);
            return SWOS2_CFG_ERR;
        }

        for(i = 0; i < swos2_eth_cfg.cfg_eth_num; i++)
        {
            for(j = 0; j < SWOS2_ETH_INFO_NUM; j++)
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
                            return SWOS2_CFG_ERR;
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
                                return SWOS2_CFG_ERR;
                            }

                        }
                    }
                }
                else
                {
                    LOG_E("get %s error", swos2_eth_cfg.eth_info[i].info[j].field_name);
                    return SWOS2_CFG_ERR;
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
    return SWOS2_CFG_OK;
}

#endif /* defined(FDB_USING_KVDB) */

