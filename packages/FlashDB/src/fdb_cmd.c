/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Shell commands.
 *
 * RT-Thread Finsh/MSH command for EasyFlash.
 */

#include <flashdb.h>
#include <rtthread.h>
#include <string.h>
#include <stdlib.h>

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) && defined(FDB_USING_KVDB)
#include <finsh.h>
#if defined(FDB_USING_KVDB)
static void kvdb_cmd_bench(fdb_kvdb_t db)
{
    off_t i;
    rt_tick_t start_tick, end_tick;
    char key[10] = { 0 }, value[10] = { 0 };
#define TEST_COUNT 48
    /* add KVs */
    start_tick = rt_tick_get();
    for (i = 0; i < TEST_COUNT; i++)
    {
        rt_snprintf(key, sizeof(key), "key:%d", i);
        rt_snprintf(value, sizeof(value), "value:%d", i);
        fdb_kv_set(db, key, value);
    }
    end_tick = rt_tick_get();
    rt_kprintf("%d KVs has been added  , total %d tick, avg %d tick/per\n", TEST_COUNT, end_tick - start_tick,
            (end_tick - start_tick) / TEST_COUNT);
    /* get KVs */
    start_tick = rt_tick_get();
    for (i = 0; i < TEST_COUNT; i++)
    {
        char *new_value;
        rt_snprintf(key, sizeof(key), "key:%d", i);
        rt_snprintf(value, sizeof(value), "value:%d", i);
        new_value = fdb_kv_get(db, key);
        if (strncmp(new_value, value, sizeof(value)) != 0)
        {
            rt_kprintf("ERROR: get KV failed, %s != %s\n", value, new_value);
        }
    }
    end_tick = rt_tick_get();
    rt_kprintf("%d KVs has been queried, total %d tick, avg %d tick/per\n", TEST_COUNT, end_tick - start_tick,
            (end_tick - start_tick) / TEST_COUNT);
    /* delete KVs */
    start_tick = rt_tick_get();
    for (i = 0; i < TEST_COUNT; i++)
    {
        rt_snprintf(key, sizeof(key), "key:%d", i);
        fdb_kv_del(db, key);
    }
    end_tick = rt_tick_get();
    rt_kprintf("%d KVs has been deleted, total %d tick, avg %d tick/per\n", TEST_COUNT, end_tick - start_tick,
            (end_tick - start_tick) / TEST_COUNT);
}

static void kvdb(uint8_t argc, char **argv)
{
#define KVDB_CMD_PROBE_INDEX0           0
#define KVDB_CMD_PROBE_INDEX1           1
#define KVDB_CMD_SHOW_INDEX             2
#define KVDB_CMD_SET_INDEX              3
#define KVDB_CMD_DEL_INDEX              4
#define KVDB_CMD_RESET_INDEX            5
#define KVDB_CMD_CLOSE_INDEX            6
#define KVDB_CMD_BENCH_INDEX            7

    size_t i = 0;
    fdb_err_t err = FDB_NO_ERR;
    static struct fdb_kvdb static_kvdb = { 0 };
    static char dev_name[FDB_KV_NAME_MAX] = { 0 };
    static char part_name[FDB_KV_NAME_MAX] = { 0 };

    const char* help_info[] =
    {
        [KVDB_CMD_PROBE_INDEX0]     = "kvdb probe [partition]               - probe KVDB by given KVDB partition",
        [KVDB_CMD_PROBE_INDEX1]     = "kvdb probe [kvdb_name] [file_path] [sec_size] [db_size] - probe KVDB by given KVDB name and file path",
        [KVDB_CMD_SHOW_INDEX]       = "kvdb show                            - show all KV data.",
        [KVDB_CMD_SET_INDEX]        = "kvdb set [key] [value]               - set value of a key.",
        [KVDB_CMD_DEL_INDEX]        = "kvdb del [key]                       - delete an KV.",
        [KVDB_CMD_RESET_INDEX]      = "kvdb reset                           - recovery all KV to default.",
        [KVDB_CMD_CLOSE_INDEX]      = "kvdb deinit                          - deinit the KVDB",
        [KVDB_CMD_BENCH_INDEX]      = "kvdb bench                           - benchmark test",
    };

    if (argc < 2)
    {
        rt_kprintf("Usage:\n");
        for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
        {
            rt_kprintf("%s\n", help_info[i]);
        }
        rt_kprintf("\n");
    }
    else
    {
        const char *operator = argv[1];

        if (!strcmp(operator, "probe"))
        {
            if (static_kvdb.parent.init_ok)
            {
                /* Close the last open KVDB */
                fdb_kvdb_deinit(&static_kvdb);
            }

            if (argc >= 3)
            {
                bool not_formatable = RT_TRUE;
                bool file_mode = RT_TRUE;

                rt_memset(&static_kvdb, 0, sizeof(static_kvdb));

                if (argc >= 6)
                {
                    /* file path */
                    uint32_t sec_size = strtol(argv[4], NULL, 0);
                    uint32_t db_size = strtol(argv[5], NULL, 0);

                    rt_strncpy(dev_name, argv[2], FDB_KV_NAME_MAX);
                    dev_name[FDB_KV_NAME_MAX - 1] = '\0';
                    rt_strncpy(part_name, argv[3], FDB_KV_NAME_MAX);
                    part_name[FDB_KV_NAME_MAX - 1] = '\0';

                    if ((sec_size == 0) || (db_size == 0) || (sec_size * 2 > db_size))
                    {
                        rt_kprintf("The following conditions must be met:\n");
                        rt_kprintf("(sec_size != 0) and (db_size != 0) and (db_size/sec_size > 2)\n");
                        rt_kprintf("Usage: %s.\n", help_info[KVDB_CMD_PROBE_INDEX0]);
                        rt_kprintf("       %s.\n", help_info[KVDB_CMD_PROBE_INDEX1]);
                        return;
                    }
                    fdb_kvdb_control(&static_kvdb, FDB_KVDB_CTRL_SET_SEC_SIZE, &sec_size);
                    fdb_kvdb_control(&static_kvdb, FDB_KVDB_CTRL_SET_FILE_MODE, &file_mode);
                    fdb_kvdb_control(&static_kvdb, FDB_KVDB_CTRL_SET_MAX_SIZE, &db_size);
                }
                else
                {
#ifdef FDB_USING_FAL_MODE
                    /* partition */
                    rt_strncpy(dev_name, argv[2], FDB_KV_NAME_MAX);
                    dev_name[FDB_KV_NAME_MAX - 1] = '\0';
                    rt_strncpy(part_name, argv[2], FDB_KV_NAME_MAX);
                    part_name[FDB_KV_NAME_MAX - 1] = '\0';

                    /* fdb partition */
                    if (fal_partition_find(part_name) == NULL)
                    {
                        rt_kprintf("The '%s' partition not found!\n", part_name);
                        rt_kprintf("Usage: %s.\n", help_info[KVDB_CMD_PROBE_INDEX0]);
                        rt_kprintf("       %s.\n", help_info[KVDB_CMD_PROBE_INDEX1]);
                        return;
                    }
#else
                    rt_kprintf("Usage: %s.\n", help_info[KVDB_CMD_PROBE_INDEX1]);
                    return;
#endif
                }
                fdb_kvdb_control(&static_kvdb, FDB_KVDB_CTRL_SET_NOT_FORMAT, &not_formatable);
                err = fdb_kvdb_init(&static_kvdb, dev_name, part_name, NULL, NULL);
                if (err != FDB_NO_ERR)
                {
                    rt_kprintf("KVDB '%s' not found. Probe failed!\n", dev_name);
                    rt_kprintf("Usage: %s.\n", help_info[KVDB_CMD_PROBE_INDEX0]);
                    rt_kprintf("       %s.\n", help_info[KVDB_CMD_PROBE_INDEX1]);
                }
                else
                {
                    rt_kprintf("Probed a KVDB | %s | part_name: %s | sec_size: %d | max_size: %d |.\n",
                            static_kvdb.parent.name, part_name, static_kvdb.parent.sec_size,
                            static_kvdb.parent.max_size);
                }
            }
            else
            {
                rt_kprintf("No KVDB was probed!\n");
                rt_kprintf("Usage: %s.\n", help_info[KVDB_CMD_PROBE_INDEX0]);
                rt_kprintf("       %s.\n", help_info[KVDB_CMD_PROBE_INDEX1]);
            }
        }
        else
        {
            if (!static_kvdb.parent.init_ok)
            {
                rt_kprintf("No KVDB was probed! Please run 'kvdb probe [kvdb_name] [part_name]'.\n");
                return;
            }

            if (!rt_strcmp(operator, "show"))
            {
                rt_kprintf("Read data success. The KV data is:\n");
                rt_kprintf("----------------------------------\n");
                fdb_kv_print(&static_kvdb);
            }
            else if (!rt_strcmp(operator, "set"))
            {
                if (argc > 3)
                {
                    err = fdb_kv_set(&static_kvdb, argv[2], argv[3]);
                    if (err != FDB_NO_ERR)
                    {
                        rt_kprintf("set the KV '%s' failed!\n", argv[2]);
                    }
                    else
                    {
                        rt_kprintf("set the KV '%s', value is: %s\n", argv[2], argv[3]);
                    }
                }
                else
                {
                    rt_kprintf("Usage: %s.\n", help_info[KVDB_CMD_SET_INDEX]);
                }
            }
            else if (!rt_strcmp(operator, "del"))
            {
                if (argc >= 3)
                {
                    err = fdb_kv_del(&static_kvdb, argv[2]);
                    if (err == FDB_KV_NAME_ERR)
                    {
                        rt_kprintf("Not found '%s' in KV!\n", argv[2]);
                    }
                    else if (err == FDB_NO_ERR)
                    {
                        rt_kprintf("delete the KV '%s' success.\n", argv[2]);
                    }
                    else
                    {
                        rt_kprintf("delete the KV '%s' failed!\n", argv[2]);
                    }
                }
                else
                {
                    rt_kprintf("Usage: %s.\n", help_info[KVDB_CMD_DEL_INDEX]);
                }
            }
            else if (!rt_strcmp(operator, "reset"))
            {
                fdb_kv_set_default(&static_kvdb);
                rt_kprintf("KVDB '%s' reset success.\n", static_kvdb.parent.name);
            }
            else if (!rt_strcmp(operator, "deinit"))
            {
                if (static_kvdb.parent.init_ok)
                {
                    /* deinit the KVDB */
                    fdb_kvdb_deinit(&static_kvdb);
                    rt_kprintf("KVDB '%s' deinit success.\n", static_kvdb.parent.name);
                }
            }
            else if (!rt_strcmp(operator, "bench"))
            {
                kvdb_cmd_bench(&static_kvdb);
            }
            else
            {
                rt_kprintf("Usage:\n");
                for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
                {
                    rt_kprintf("%s\n", help_info[i]);
                }
                rt_kprintf("\n");
                return;
            }
        }
    }
}
MSH_CMD_EXPORT(kvdb, FlashDB KVDB command.);

#endif /* defined(FDB_USING_KVDB) */

#if defined(FDB_USING_TSDB)

static struct fdb_tsdb static_tsdb = { 0 };
static char dev_name[FDB_KV_NAME_MAX] = { 0 };
static char part_name[FDB_KV_NAME_MAX] = { 0 };

static bool tsl_cb(fdb_tsl_t tsl, void *arg)
{
    struct fdb_blob blob;

    size_t read_len = 0;
    size_t logstr_len = 0;

    char *log = rt_malloc(tsl->log_len);
    if (log != NULL)
    {
        fdb_blob_make(&blob, log, tsl->log_len);
        read_len = fdb_blob_read((fdb_db_t) &static_tsdb, fdb_tsl_to_blob(tsl, &blob));

        char *logstr = rt_malloc(24 + 8 + read_len + 1 + 1);
        if (logstr != NULL)
        {
            rt_snprintf(logstr + logstr_len, 24 + 1, "%s", ctime((const time_t *) (&tsl->time)));
            logstr_len += 24;
            rt_snprintf(logstr + logstr_len, 8 + 1, " : (%3d)", read_len);
            logstr_len += 8;
            rt_snprintf(logstr + logstr_len, read_len + 1, "%s", blob.buf);
            logstr_len += read_len;
            rt_snprintf(logstr + logstr_len, 1 + 1, "\n");
            logstr_len += 1;

            rt_kprintf("%s", logstr);

            rt_free(logstr);
        }

        rt_free(log);
    }

    return false;
}

static bool tsl_bench_cb(fdb_tsl_t tsl, void *arg)
{
    rt_tick_t *end_tick = arg;

    *end_tick = rt_tick_get();

    return false;
}

//自纪元 Epoch（1970-01-01 00:00:00 UTC）起经过的时间，以秒为单位
static fdb_time_t tsl_get_time(void)
{
    return time(NULL);
}

static void tsl(uint8_t argc, char **argv)
{
    fdb_err_t err = FDB_NO_ERR;

    const char* help_info[] =
    {
        "tsdb probe [partition]               - probe TSDB by given TSDB partition",
        "tsdb probe [tsdb_name] [file_path] [sec_size] [db_size] - probe TSDB by given TSDB name and file path",
        "tsdb show                            - show all TS log data.",
        "tsdb query [status 0~5]              - query all TS.",
        "tsdb add [log_content]               - add a log content.",
        "tsdb clean                           - clean all TS.",
        "tsdb deinit                          - deinit the TSDB",
        "tsdb bench                           - benchmark test",
    };

    if (argc < 2)
    {
        rt_kprintf("Usage:\n");
        for (int i = 0; i < sizeof(help_info) / sizeof(char*); i++)
        {
            rt_kprintf("%s\n", help_info[i]);
        }
        rt_kprintf("\n");
    }
    else if (!strcmp(argv[1], "probe"))
    {
        if (static_tsdb.parent.init_ok)
        {
            /* Close the last open TSDB */
            fdb_tsdb_deinit(&static_tsdb);
        }

        if (argc >= 3)
        {
            bool not_formatable = RT_TRUE;
            bool file_mode = RT_TRUE;

            rt_memset(&static_tsdb, 0, sizeof(static_tsdb));

            if (argc >= 6)
            {
                /* file path */
                uint32_t sec_size = strtol(argv[4], NULL, 0);
                uint32_t db_size = strtol(argv[5], NULL, 0);

                rt_strncpy(dev_name, argv[2], FDB_KV_NAME_MAX);
                dev_name[FDB_KV_NAME_MAX - 1] = '\0';
                rt_strncpy(part_name, argv[3], FDB_KV_NAME_MAX);
                part_name[FDB_KV_NAME_MAX - 1] = '\0';

                if ((sec_size == 0) || (db_size == 0) || (sec_size * 2 > db_size))
                {
                    rt_kprintf("The following conditions must be met:\n");
                    rt_kprintf("(sec_size != 0) and (db_size != 0) and (db_size/sec_size > 2)\n");
                    return;
                }
                fdb_tsdb_control(&static_tsdb, FDB_TSDB_CTRL_SET_SEC_SIZE, &sec_size);
                fdb_tsdb_control(&static_tsdb, FDB_TSDB_CTRL_SET_FILE_MODE, &file_mode);
                fdb_tsdb_control(&static_tsdb, FDB_TSDB_CTRL_SET_MAX_SIZE, &db_size);
            }
            else
            {
#ifdef FDB_USING_FAL_MODE
                /* partition */
                rt_strncpy(dev_name, argv[2], FDB_KV_NAME_MAX);
                dev_name[FDB_KV_NAME_MAX - 1] = '\0';
                rt_strncpy(part_name, argv[2], FDB_KV_NAME_MAX);
                part_name[FDB_KV_NAME_MAX - 1] = '\0';

                /* fdb partition */
                if (fal_partition_find(part_name) == NULL)
                {
                    rt_kprintf("The '%s' partition not found!\n", part_name);
                    return;
                }
#endif
            }
            fdb_tsdb_control(&static_tsdb, FDB_TSDB_CTRL_SET_NOT_FORMAT, &not_formatable);
            err = fdb_tsdb_init(&static_tsdb, dev_name, part_name, tsl_get_time, ULOG_LINE_BUF_SIZE, NULL);
            if (err != FDB_NO_ERR)
            {
                rt_kprintf("TSDB '%s' not found. Probe failed!\n", dev_name);
            }
            else
            {
                rt_kprintf("Probed a TSDB | %s | part_name: %s | sec_size: %d | max_size: %d |.\n",
                        static_tsdb.parent.name, part_name, static_tsdb.parent.sec_size,
                        static_tsdb.parent.max_size);
            }
        }
        else
        {
            rt_kprintf("No TSDB was probed!\n");
        }
    }
    else
    {
        if (!static_tsdb.parent.init_ok)
        {
            rt_kprintf("No TSDB was probed!\n");
            return;
        }

        if ((argc > 2) && !strcmp(argv[1], "add"))
        {
            struct fdb_blob blob;
            fdb_tsl_append(&static_tsdb, fdb_blob_make(&blob, argv[2], strlen(argv[2])));
        }
        else if (!strcmp(argv[1], "show"))
        {
            struct tm tm_from = { .tm_year = 1970 - 1900, .tm_mon = 0, .tm_mday = 1, .tm_hour = 0, .tm_min = 0, .tm_sec = 0 };
            struct tm tm_to = { .tm_year = 2030 - 1900, .tm_mon = 0, .tm_mday = 1, .tm_hour = 0, .tm_min = 0, .tm_sec = 0 };
            time_t from_time = mktime(&tm_from), to_time = mktime(&tm_to);

            fdb_tsl_iter_by_time(&static_tsdb, from_time, to_time, tsl_cb, NULL);
        }
        else if (!strcmp(argv[1], "clean"))
        {
            fdb_tsl_clean(&static_tsdb);
        }
        else if (!strcmp(argv[1], "deinit"))
        {
            if (static_tsdb.parent.init_ok)
            {
                /* Close the last open TSDB */
                fdb_tsdb_deinit(&static_tsdb);
            }
        }
        else if ((argc > 2) && !strcmp(argv[1], "query"))
        {
            struct tm tm_from = { .tm_year = 1970 - 1900, .tm_mon = 0, .tm_mday = 1, .tm_hour = 0, .tm_min = 0, .tm_sec = 0 };
            struct tm tm_to = { .tm_year = 2030 - 1900, .tm_mon = 0, .tm_mday = 1, .tm_hour = 0, .tm_min = 0, .tm_sec = 0 };
            time_t from_time = mktime(&tm_from), to_time = mktime(&tm_to);

            int status = atoi(argv[2]);
            size_t count;
            count = fdb_tsl_query_count(&static_tsdb, from_time, to_time, status);
            rt_kprintf("query count: %d\n", count);
        }
        else if (!strcmp(argv[1], "bench"))
        {
#define BENCH_TIMEOUT        (5*1000)
            struct fdb_blob blob;
            static char data[11], log[128];
            size_t append_num = 0;
            fdb_time_t start, end, cur;
            rt_tick_t end_tick;
            rt_tick_t bench_start_tick, spent_tick, min_tick = 9999, max_tick = 0, total_tick = 0;
            float temp;

            fdb_tsl_clean(&static_tsdb);
            bench_start_tick = rt_tick_get();
            start = static_tsdb.get_time();
            while (rt_tick_get() - bench_start_tick <= (rt_tick_t) rt_tick_from_millisecond(BENCH_TIMEOUT))
            {
                rt_snprintf(data, sizeof(data), "%d", append_num++);
                fdb_tsl_append(&static_tsdb, fdb_blob_make(&blob, data, rt_strnlen(data, sizeof(data))));
            }
            end = static_tsdb.get_time();
            temp = (float) append_num / (float) (BENCH_TIMEOUT / 1000);
            snprintf(log, sizeof(log), "Append %d TSL in %d seconds, average: %d.%02d tsl/S\n", append_num,
                    BENCH_TIMEOUT / 1000, (int) temp, ((int) temp) % 100);
            rt_kprintf("%s", log);
            cur = start;
            while (cur < end)
            {
                end_tick = bench_start_tick = rt_tick_get();
                fdb_tsl_iter_by_time(&static_tsdb, cur, cur, tsl_bench_cb, &end_tick);
                spent_tick = rt_tick_get() - bench_start_tick;
                if (spent_tick < min_tick)
                {
                    min_tick = spent_tick;
                }
                if (spent_tick > max_tick)
                {
                    max_tick = spent_tick;
                }
                total_tick += spent_tick;
                cur++;
            }
            snprintf(log, sizeof(log), "Query %lu (tick) for %ld TSL, min %lu, max %lu, average: %d.%02d tick/per\n",
                    total_tick, end - start, min_tick, max_tick, (int) ((float) total_tick / (float) (end - start)),
                    ((int) ((float) total_tick / (float) (end - start))) % 100);
            rt_kprintf("%s", log);
            fdb_tsl_clean(&static_tsdb);
        }
        else
        {
            rt_kprintf("Usage:\n");
            for (int i = 0; i < sizeof(help_info) / sizeof(char*); i++)
            {
                rt_kprintf("%s\n", help_info[i]);
            }
            rt_kprintf("\n");
            return;
        }
    }
}
MSH_CMD_EXPORT_ALIAS(tsl, tsdb, FlashDB TSDB command.);
#endif /* defined(FDB_USING_TSDB) */

#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
