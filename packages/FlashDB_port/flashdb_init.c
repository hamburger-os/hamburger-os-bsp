/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-01     lvhan       the first version
 *
 */
#include "board.h"

#include <time.h>
#include <unistd.h>

#include "flashdb_port.h"

#define DBG_TAG "fdb"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#ifdef FLASHDB_PORT_USING_KVDB
static struct fdb_kvdb kvdb;

static int kvdb_init(void)
{
    fdb_err_t ret = FDB_NO_ERR;

    char mac[4][18];
    for (int i = 0; i < 4; i++)
    {
        rt_snprintf(mac[i], 18, "F8-09-A4-%02X-%02X-%02X"
            , *(uint8_t *)(UID_BASE + 2 + i)
            , *(uint8_t *)(UID_BASE + 1 + i)
            , *(uint8_t *)(UID_BASE + 0 + i));
    }

    struct fdb_default_kv_node default_kv_table[] =
    {
        {"e_ip"                         , "192.168.1.29"            },
        {"e_gw"                         , "192.168.1.1"             },
        {"e_mask"                       , "255.255.255.0"           },
        {"e_mac"                        , mac[0]                    },
        {"e0_ip"                        , "192.168.1.30"            },
        {"e0_gw"                        , "192.168.1.1"             },
        {"e0_mask"                      , "255.255.255.0"           },
        {"e0_mac"                       , mac[1]                    },
        {"e1_ip"                        , "192.168.1.31"            },
        {"e1_gw"                        , "192.168.1.1"             },
        {"e1_mask"                      , "255.255.255.0"           },
        {"e1_mac"                       , mac[2]                    },
        {"e2_ip"                        , "192.168.1.32"            },
        {"e2_gw"                        , "192.168.1.1"             },
        {"e2_mask"                      , "255.255.255.0"           },
        {"e2_mac"                       , mac[3]                    },
    };

    struct fdb_default_kv default_kv = {0};
    default_kv.kvs = default_kv_table;
    default_kv.num = sizeof(default_kv_table) / sizeof(default_kv_table[0]);

    ret = fdb_kvdb_init(&kvdb, "kvdb", FLASHDB_PORT_KVDB_PARTNAME, &default_kv, NULL);
    if (ret != FDB_NO_ERR)
    {
        LOG_E("kvdb init failed!");
        return -RT_ERROR;
    }

    struct fdb_blob blob = {0};
    uint32_t boot_count = 0;

    /* GET the KV value */
    /* get the "boot_count" KV value */
    fdb_kv_get_blob(&kvdb, "boot_count", fdb_blob_make(&blob, &boot_count, sizeof(boot_count)));
    /* the blob.saved.len is more than 0 when get the value successful */
    if (blob.saved.len > 0) {
        LOG_D("get 'boot_count' value is %d", boot_count);
    } else {
        LOG_E("get 'boot_count' failed");
    }

    /* CHANGE the KV value */
    /* increase the boot count */
    boot_count ++;
    /* change the "boot_count" KV's value */
    fdb_kv_set_blob(&kvdb, "boot_count", fdb_blob_make(&blob, &boot_count, sizeof(boot_count)));
    LOG_I("Welcome to the system for the %u time", boot_count);

    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_COMPONENT_EXPORT(kvdb_init);
#endif

size_t kvdb_get_blob(const char *key, fdb_blob_t blob)
{
    /* get the KV value */
    return fdb_kv_get_blob(&kvdb, key, blob);
}
RTM_EXPORT(kvdb_get_blob);

size_t kvdb_get(const char *key, char *value)
{
    /* get the KV value */
    char *pvalue = fdb_kv_get(&kvdb, key);
    if (pvalue != NULL)
    {
        rt_strcpy(value, pvalue);
        return rt_strlen(value);
    }

    LOG_E("kvdb get '%s' error!", key);
    return 0;
}
RTM_EXPORT(kvdb_get);

fdb_err_t kvdb_set_blob(const char *key, fdb_blob_t blob)
{
    /* change the KV's value */
    return fdb_kv_set_blob(&kvdb, key, blob);
}
RTM_EXPORT(kvdb_set_blob);

fdb_err_t kvdb_set(const char *key, char *value)
{
    /* change the KV's value */
    return fdb_kv_set(&kvdb, key, value);
}
RTM_EXPORT(kvdb_set);

fdb_err_t kvdb_del(const char *key)
{
    /* del the KV */
    return fdb_kv_del(&kvdb, key);
}
RTM_EXPORT(kvdb_del);


#ifdef FLASHDB_PORT_USING_TSDB
static struct fdb_tsdb tsdb;

//自纪元 Epoch（1970-01-01 00:00:00 UTC）起经过的时间，以秒为单位
static fdb_time_t tsdb_get_time(void)
{
    return time(NULL);
}

static int tsdb_init(void)
{
    fdb_err_t ret = FDB_NO_ERR;

    ret = fdb_tsdb_init(&tsdb, "tsdb", FLASHDB_PORT_TSDB_PARTNAME, tsdb_get_time, ULOG_LINE_BUF_SIZE, NULL);
    if (ret != FDB_NO_ERR)
    {
        LOG_E("tsdb init failed!");
        return -RT_ERROR;
    }

    LOG_D("tsdb init succeed.");
    return RT_EOK;
}
/* 导出到自动初始化 */
INIT_ENV_EXPORT(tsdb_init);
#endif
