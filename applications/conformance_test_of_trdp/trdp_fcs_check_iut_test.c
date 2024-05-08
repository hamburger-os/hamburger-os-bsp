/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-12     zm       the first version
 */

#include "trdp_test_public.h"
#include "trdp_if_light.h"
#include "vos_thread.h"
#include "vos_utils.h"

#define DBG_TAG "trdp_pd_fcs_check_iut_test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/select.h>

#define TRDP_PUSH_FCS_CHECK_IUT_TEST_PORT_NUM (6)

#define TRDP_PUSH_FCS_CHECK_IUT_TEST_PUB_MCAST ("239.255.3.3")
#define TRDP_PUSH_FCS_CHECK_IUT_TEST_SUB_MCAST ("239.255.3.2")

#define TRDP_PUSH_FCS_CHECK_IUT_TEST_PUB_COM_ID (303)
#define TRDP_PUSH_FCS_CHECK_IUT_TEST_SUB_COM_ID (302)

/* --- globals ---------------------------------------------------------------*/
typedef enum
{
    PORT_PUSH,                      /* outgoing port ('Pd'/push)   (TSN suppport)*/
    PORT_PULL,                      /* outgoing port ('Pp'/pull)   */
    PORT_REQUEST,                   /* outgoing port ('Pr'/request)*/
    PORT_SINK,                      /* incoming port               */
    PORT_SINK_PUSH,                 /* incoming port for pushed messages (TSN suppport)*/
} Type;

static const char * types[] =
    { "Pd ->", "Pp ->", "Pr ->", "   <-", "   <-" };

typedef struct
{
    Type type;                      /* port type */
    TRDP_ERR_T err;                 /* put/get status */
    TRDP_PUB_T ph;                  /* publish handle */
    TRDP_SUB_T sh;                  /* subscribe handle */
    UINT32 comid;                   /* comid            */
    UINT32 repid;                   /* reply comid (for PULL requests) */
    UINT32 size;                    /* size                            */
    TRDP_IP_ADDR_T src;             /* source ip address               */
    TRDP_IP_ADDR_T dst;             /* destination ip address          */
    TRDP_IP_ADDR_T rep;             /* reply ip address (for PULL requests) */
    UINT32 cycle;                   /* cycle                                */
    UINT32 timeout;                 /* timeout (for SINK ports)             */
    unsigned char data[TRDP_MAX_PD_DATA_SIZE];       /* data buffer                          */
//    UINT32 data[TRDP_MAX_PD_DATA_SIZE];       /* data buffer                          */
    uint32_t sign;
    int link;                       /* index of linked port (echo or subscribe) */
} Port;

typedef struct {
    /* default addresses - overriden from command line */
    TRDP_IP_ADDR_T srcip;
    TRDP_IP_ADDR_T dstip;
    TRDP_IP_ADDR_T pub_mcast;
    TRDP_IP_ADDR_T sub_mcast;
} TRDP_PD_TEST_IP_CFG;

typedef struct {
    TRDP_PD_TEST_IP_CFG ip_cfg;
    uint32_t pub_com_id;
    uint32_t sub_com_id;

    Port ports[TRDP_PUSH_FCS_CHECK_IUT_TEST_PORT_NUM];   /* array of ports          */
    uint32_t nports;                    /* number of ports         */
    TRDP_PD_INFO_T pdi;

    TRDP_MEM_CONFIG_T memcfg;
    TRDP_APP_SESSION_T apph;
    TRDP_PD_CONFIG_T pdcfg;
    TRDP_PROCESS_CONFIG_T proccfg;
} TRDP_PD_TEST;

static TRDP_PD_TEST trdp_pd_test_dev;

#define PORT_FLAGS TRDP_FLAGS_NONE

/***********************************************************************************************************************
 * PROTOTYPES
 */

/* --- generate PUSH ports ---------------------------------------------------*/

/*
 * 设置所有发布者
 * */
static void set_gen_pub_config(TRDP_PD_TEST *dev)
{
    Port src;

    if(dev == RT_NULL)
    {
        LOG_E("set_gen_pub_config dev is null");
    }

    LOG_I("set_gen_pub_config");

    memset(&src, 0, sizeof(src));

    src.type = PORT_PUSH;
    src.comid = dev->pub_com_id;
    /* dataset size */
    src.size = 1024;
    /* period [usec] */
    src.cycle = (UINT32) 1000u * (UINT32)1000; /* 1000ms */

#if 0
    /* unicast address 单点 */
    src.dst = dev->ip_cfg.dstip;
    src.src = dev->ip_cfg.srcip;
#else
    /* unicast address 组播 */
    src.dst = dev->ip_cfg.pub_mcast;
    src.src = dev->ip_cfg.srcip;
#endif

    src.link = -1;

    /* add ports to config */
    dev->ports[dev->nports++] = src;
}

/*
 * 设置所有订阅者
 * */
static void set_gen_sub_config(TRDP_PD_TEST *dev)
{
    Port snk;

    if(dev == RT_NULL)
    {
        LOG_E("set_gen_sub_config dev is null");
    }

    LOG_I("set_gen_sub_config");
    memset(&snk, 0, sizeof(snk));

    snk.type = PORT_SINK_PUSH;
    snk.timeout = 5000000;         /* 5 secs timeout*/
    snk.comid = dev->sub_com_id;
    /* dataset size */
    snk.size = 1024;

    snk.src = dev->ip_cfg.dstip;
    snk.dst = dev->ip_cfg.sub_mcast;

    /* add ports to config */
    dev->ports[dev->nports++] = snk;
}

/* --- setup ports -----------------------------------------------------------*/

static void setup_ports()
{
    int i;

    LOG_I("setup_ports:");
    /* setup ports one-by-one */
    for (i = 0; i < trdp_pd_test_dev.nports; ++i)
    {
        Port * p = &trdp_pd_test_dev.ports[i];
        TRDP_COM_PARAM_T comPrams = TRDP_PD_DEFAULT_SEND_PARAM;

        LOG_I("%3d: commid <%d> / %s / %4d / %3d ... ",
            i, p->comid, types[p->type], p->size, p->cycle / 1000);

        /* depending on port type */
        switch (p->type)
        {
        case PORT_PUSH:
            p->err = tlp_publish(
                trdp_pd_test_dev.apph,               /* session handle */
                &p->ph,             /* publish handle */
                NULL, NULL,
                0u,                 /* serviceId        */
                p->comid,           /* comid            */
                0,                  /* topo counter     */
                0,
                p->src,             /* source address   */
                p->dst,             /* destination address */
                p->cycle,           /* cycle period   */
                0,                  /* redundancy     */
                PORT_FLAGS,         /* flags          */
                &comPrams,          /* default send parameters */
                p->data,            /* data           */
                p->size);           /* data size      */

            if (p->err != TRDP_NO_ERR)
                LOG_E("tlp_publish() failed, err: %d", p->err);
            else
                LOG_I("PORT_PUSH ok");
            break;
        case PORT_PULL:
            p->err = tlp_publish(
                trdp_pd_test_dev.apph,               /* session handle */
                &p->ph,             /* publish handle */
                NULL, NULL,
                0u,                 /* serviceId        */
                p->comid,           /* comid            */
                0,                  /* topo counter     */
                0,
                p->src,             /* source address   */
                p->dst,             /* destination address */
                p->cycle,           /* cycle period   */
                0,                  /* redundancy     */
                TRDP_FLAGS_NONE,    /* flags          */
                NULL,               /* default send parameters */
                p->data,            /* data           */
                p->size);           /* data size      */

            if (p->err != TRDP_NO_ERR)
                LOG_E("tlp_publish() failed, err: %d", p->err);
            else
                LOG_I("PORT_PULL ok");
            break;

        case PORT_REQUEST:
            p->err = tlp_request(
                trdp_pd_test_dev.apph,               /* session handle */
                trdp_pd_test_dev.ports[p->link].sh,  /* related subscribe handle */
                0u,                 /* serviceId        */
                p->comid,           /* comid          */
                0,                  /* topo counter   */
                0,
                p->src,             /* source address */
                p->dst,             /* destination address */
                0,                  /* redundancy     */
                TRDP_FLAGS_NONE,    /* flags          */
                NULL,               /* default send parameters */
                p->data,            /* data           */
                p->size,            /* data size      */
                p->repid,           /* reply comid    */
                p->rep);            /* reply ip address  */

            if (p->err != TRDP_NO_ERR)
                LOG_E("tlp_request() failed, err: %d", p->err);
            else
                LOG_I("PORT_REQUEST ok");
            break;

        case PORT_SINK:
            p->err = tlp_subscribe(
                trdp_pd_test_dev.apph,               /* session handle   */
                &p->sh,             /* subscribe handle */
                NULL,               /* user ref         */
                NULL,               /* callback funktion */
                0u,                 /* serviceId        */
                p->comid,           /* comid            */
                0,                  /* topo counter     */
                0,
                p->src,             /* source address   */
                VOS_INADDR_ANY,
                p->dst,             /* destination address    */
                TRDP_FLAGS_NONE,    /* No flags set     */
                NULL,               /*    Receive params */
                p->timeout,             /* timeout [usec]   */
                TRDP_TO_SET_TO_ZERO);   /* timeout behavior */

            if (p->err != TRDP_NO_ERR)
                LOG_E("tlp_subscribe() failed, err: %d", p->err);
            else
                LOG_I("PORT_SINK ok");
            break;
        case PORT_SINK_PUSH:
            p->err = tlp_subscribe(
                trdp_pd_test_dev.apph,               /* session handle   */
                &p->sh,             /* subscribe handle */
                NULL,               /* user ref         */
                NULL,               /* callback funktion */
                0u,                 /* serviceId        */
                p->comid,           /* comid            */
                0,                  /* topo counter     */
                0,
                p->src,             /* source address   */
                VOS_INADDR_ANY,
                p->dst,             /* destination address    */
                PORT_FLAGS,         /* No flags set     */
                &comPrams,              /*    Receive params */
                p->timeout,             /* timeout [usec]   */
                TRDP_TO_SET_TO_ZERO);   /* timeout behavior */

            if (p->err != TRDP_NO_ERR)
                LOG_E("tlp_subscribe() failed, err: %d", p->err);
            else
                LOG_I("PORT_SINK_PUSH ok");
            break;
        }
    }
}

/* --- test data processing --------------------------------------------------*/

static void process_data(void)
{
    int i = 0;
    static uint32_t last_life_signt = 0;

    /* go through ports one-by-one */
    for (i = 0; i < trdp_pd_test_dev.nports; ++i)
    {
        Port * p = &trdp_pd_test_dev.ports[i];
        /* write port data */
        if (p->type == PORT_PUSH || p->type == PORT_PULL)
        {
            p->err = tlp_put(trdp_pd_test_dev.apph, p->ph, (const UINT8 *)p->data, p->size);

            if (p->err != TRDP_NO_ERR)
            {
                LOG_E("PORT_PU-- %s", trdp_test_public_get_result_string(p->err));
            }
        }
        else if(p->type == PORT_SINK_PUSH)
        {
            if(p->err == TRDP_NO_ERR)
            {
                Port * p_send = &trdp_pd_test_dev.ports[0];

                uint32_t comm_id = 0, echo_data = 0, read_life_sign = 0;

                comm_id = trdp_pd_test_dev.pub_com_id;

                p_send->data[0] = comm_id;
                p_send->data[1] = comm_id >> 8;
                p_send->data[2] = comm_id >> 16;
                p_send->data[3] = comm_id >> 24;

                read_life_sign = (p->data[7] << 24) | (p->data[6] << 16) | (p->data[5] << 8) | p->data[4];
                LOG_I("read_life_sign %d %d %d %d %d %d", read_life_sign, last_life_signt, p->data[4], p->data[5], p->data[6], p->data[7]);
                if((read_life_sign - last_life_signt) == 1)
                {
                    echo_data = 0x55555555;
                    last_life_signt = read_life_sign;
                }
                else
                {
                    echo_data = 0xAAAAAAAA;
                }

                p_send->data[4] = echo_data;
                p_send->data[5] = echo_data >> 8;
                p_send->data[6] = echo_data >> 16;
                p_send->data[7] = echo_data >> 24;
            }
            else if(p->err == TRDP_TIMEOUT_ERR)
            {
                Port * p_send = &trdp_pd_test_dev.ports[0];

                uint32_t comm_id = 0, echo_data = 0;

                comm_id = trdp_pd_test_dev.pub_com_id;

                p_send->data[0] = comm_id;
                p_send->data[1] = comm_id >> 8;
                p_send->data[2] = comm_id >> 16;
                p_send->data[3] = comm_id >> 24;

                echo_data = 0xAAAAAAAA;

                p_send->data[4] = echo_data;
                p_send->data[5] = echo_data >> 8;
                p_send->data[6] = echo_data >> 16;
                p_send->data[7] = echo_data >> 24;

            }
        }
    }
}

/* --- poll received data ----------------------------------------------------*/

static void poll_data()
{
    int i;

    /* go through ports one-by-one */
    for (i = 0; i < trdp_pd_test_dev.nports; ++i)
    {
        Port * p = &trdp_pd_test_dev.ports[i];
        if (p->type == PORT_SINK || p->type == PORT_SINK_PUSH)
        {
            p->err = tlp_get(trdp_pd_test_dev.apph, p->sh, &trdp_pd_test_dev.pdi, p->data, &p->size);
        }
    }
}

static void trdp_pd_fcs_check_iut_thread(void *paramemter)
{
    static uint32_t current_ms = 0;
    /* trdp_pd_test loop */
    while (1)
    {   /* drive TRDP communications */
        tlc_process(trdp_pd_test_dev.apph, NULL, NULL);
        /* poll (receive) data */
        poll_data();
        /* process data every 1000 ms */
        if(trdp_test_timeout_ms(&current_ms, 1000))
        {
            process_data();
        }

        rt_thread_mdelay(1);
    }
}

/* --- trdp_pd_push_fcs_check_iut_test ------------------------------------------------------------------*/
void trdp_pd_push_fcs_check_iut_test(int argc, char * argv[])
{
    TRDP_ERR_T err;
    rt_thread_t tid;

    LOG_I("%s: (%s - %s)\n",
                          argv[0], __DATE__, __TIME__);

    memset(&trdp_pd_test_dev, 0, sizeof(trdp_pd_test_dev));

    if (argc < 2)
    {
        rt_kprintf("usage: %s <localip> <remoteip>\n", argv[0]);
        rt_kprintf("<localip>       .. own IP address (ie. 10.0.1.1)\n");
        rt_kprintf("<remoteip>      .. remote IP address (ie. 10.0.1.4)\n");
        return;
    }

    trdp_pd_test_dev.ip_cfg.srcip = vos_dottedIP(argv[1]);       /* 本地IP */
    trdp_pd_test_dev.ip_cfg.dstip = vos_dottedIP(argv[2]);       /* 远程IP */
    trdp_pd_test_dev.ip_cfg.pub_mcast = vos_dottedIP(TRDP_PUSH_FCS_CHECK_IUT_TEST_PUB_MCAST);   /* 发布者 组播地址 */
    trdp_pd_test_dev.ip_cfg.sub_mcast = vos_dottedIP(TRDP_PUSH_FCS_CHECK_IUT_TEST_SUB_MCAST);   /* 订阅者 组播地址 */
    trdp_pd_test_dev.pub_com_id = TRDP_PUSH_FCS_CHECK_IUT_TEST_PUB_COM_ID;         /* 发布者 commid */
    trdp_pd_test_dev.sub_com_id = TRDP_PUSH_FCS_CHECK_IUT_TEST_SUB_COM_ID;         /* 订阅者 commid */

    if (!trdp_pd_test_dev.ip_cfg.srcip || (trdp_pd_test_dev.ip_cfg.pub_mcast >> 28) != 0xE)
    {
        LOG_E("invalid ip arguments\n");
    }

    memset(&trdp_pd_test_dev.memcfg, 0, sizeof(TRDP_MEM_CONFIG_T));
    memset(&trdp_pd_test_dev.proccfg, 0, sizeof(TRDP_PROCESS_CONFIG_T));

    /* initialize TRDP protocol library */
    err = tlc_init(trdp_test_public_printLog, NULL, &trdp_pd_test_dev.memcfg);
    if (err != TRDP_NO_ERR)
    {
        LOG_E("tlc_init() failed, err: %d\n", err);
        return;
    }

    trdp_pd_test_dev.pdcfg.pfCbFunction = NULL;
    trdp_pd_test_dev.pdcfg.pRefCon = NULL;
    trdp_pd_test_dev.pdcfg.sendParam.qos = 5;
    trdp_pd_test_dev.pdcfg.sendParam.ttl = 64;
    trdp_pd_test_dev.pdcfg.flags = TRDP_FLAGS_NONE;
    trdp_pd_test_dev.pdcfg.timeout = 10000000;
    trdp_pd_test_dev.pdcfg.toBehavior = TRDP_TO_SET_TO_ZERO;
    trdp_pd_test_dev.pdcfg.port = 17224;

    /* open session */
    err = tlc_openSession(&trdp_pd_test_dev.apph, trdp_pd_test_dev.ip_cfg.srcip, 0, NULL, &trdp_pd_test_dev.pdcfg, NULL, &trdp_pd_test_dev.proccfg);
    if (err != TRDP_NO_ERR)
    {
        LOG_E("tlc_openSession() failed, err: %d\n", err);
        return;
    }
    /* generate ports configuration */
    set_gen_pub_config(&trdp_pd_test_dev);
    set_gen_sub_config(&trdp_pd_test_dev);

    setup_ports();
    vos_threadDelay(2000000);

    tid = rt_thread_create("trdp_pd_fcs_check_iut_thread",
                            trdp_pd_fcs_check_iut_thread, RT_NULL,
                        (1024 * 6), 20, 5);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(trdp_pd_push_fcs_check_iut_test, trdp_pd_push_fcs_check_iut_test, trdp pd push fcs check iut test);
#endif



