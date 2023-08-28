/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-04-25     lvhan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <flashdb.h>
#include "flashdb_port.h"
#include "coupler_controller.h"

#define DBG_TAG "dbhelp"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define DB_ADDR_KEY     "addr"

typedef struct
{
    uint8_t addr;               //与站防通信的485总线设备地址
} CouplerCtrlDB;

static CouplerCtrlDB coupler_controller_db = { 0 };

//一个值为32位整数的键值对数据表
struct DbhelpTable
{
    char *key;
    uint32_t *value;
    uint8_t size;
    uint32_t defaultv;
};
static struct DbhelpTable dbhelp_table[] =
{
    { DB_ADDR_KEY, (uint32_t *) &coupler_controller_db.addr, sizeof(coupler_controller_db.addr), 0x02 },
};

void coupler_controller_dbinit(void)
{
    struct fdb_blob blob = { 0 };

    LOG_I("========================= dbhelp table =========================");
    LOG_I("|      key\t|    value\t|     size\t|  default\t|");
    LOG_I("----------------------------------------------------------------");
    for (uint32_t i = 0; i < sizeof(dbhelp_table) / sizeof(struct DbhelpTable); i++)
    {
        struct DbhelpTable *node = &dbhelp_table[i];
        /* get the KV value */
        kvdb_get_blob(node->key, fdb_blob_make(&blob, node->value, node->size));
        /* the blob.saved.len is more than 0 when get the value successful */
        if (blob.saved.len > 0)
        {
            LOG_I("| %8s\t| %8d\t| %8d\t| %8d\t|", node->key, *node->value & (0xffffffff >> (32 - node->size * 8)),
                    node->size, node->defaultv);
        }
        else
        {
            //数据库没有值，则创建一个默认值
            /* change the KV's value */
            *node->value = node->defaultv;
            kvdb_set_blob(node->key, fdb_blob_make(&blob, node->value, node->size));
            kvdb_get_blob(node->key, fdb_blob_make(&blob, node->value, node->size));
            LOG_W("| %8s\t| %8d\t| %8d\t| %8d\t|", node->key, *node->value & (0xffffffff >> (32 - node->size * 8)),
                    node->size, node->defaultv);
        }
    }
    LOG_I("================================================================");
}

void set_device_addr(uint8_t addr)
{
    struct fdb_blob blob = { 0 };

    coupler_controller_db.addr = addr;
    kvdb_set_blob(DB_ADDR_KEY, fdb_blob_make(&blob, &coupler_controller_db.addr, sizeof(coupler_controller_db.addr)));
    kvdb_get_blob(DB_ADDR_KEY, fdb_blob_make(&blob, &coupler_controller_db.addr, sizeof(coupler_controller_db.addr)));
}

uint8_t get_device_addr(void)
{
    return coupler_controller_db.addr;
}

/** \brief change the dbhelp table
 * \return void
 *
 */
static void dbhelp_change(int argc, char *argv[])
{
    if (argc != 4)
    {
        rt_kprintf("Usage: dbhelp set [key] [value]\n");
    }
    else
    {
        uint32_t i = 0;
        uint32_t table_max = sizeof(dbhelp_table) / sizeof(struct DbhelpTable);

        struct fdb_blob blob = { 0 };
        char *key = argv[2];
        uint32_t value = strtoul(argv[3], NULL, 10);
        for (i = 0; i < table_max; i++)
        {
            struct DbhelpTable *node = &dbhelp_table[i];
            if (rt_strcmp(node->key, key) == 0)
            {
                *node->value = value;
                kvdb_set_blob(node->key, fdb_blob_make(&blob, node->value, node->size));
                kvdb_get_blob(node->key, fdb_blob_make(&blob, node->value, node->size));
                rt_kprintf("set '%s' key to %d\n", node->key, *node->value & (0xffffffff >> (32 - node->size * 8)));

                break;
            }
        }
        if (i >= table_max)
        {
            rt_kprintf("can't find '%s' key!\n", key);
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(dbhelp_change, dbhelp, change the dbhelp table.);
#endif
