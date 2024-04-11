/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-10     zm       the first version
 */

#include "trdp_test_public.h"
#include "trdp_if_light.h"
#include "vos_thread.h"
#include "vos_utils.h"

#define DBG_TAG "trdp_pd_topology_counter_iut_test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/select.h>

#define TRDP_PUSH_TOPOLOGY_COUNTER_SYSTEM_TEST_PORT_NUM (6)

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
    TRDP_IP_ADDR_T sub_src;
} TRDP_PD_TEST_IP_CFG;

typedef struct {
    TRDP_PD_TEST_IP_CFG ip_cfg;
    uint32_t pub_commid;
    uint32_t sub_commid;

    uint32_t load_etbTopoCnt;
    uint32_t load_opTrnTopoCnt;

    uint32_t send_etbTopoCnt;
    uint32_t send_opTrnTopoCnt;

    Port ports[TRDP_PUSH_TOPOLOGY_COUNTER_SYSTEM_TEST_PORT_NUM];   /* array of ports          */
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
    src.comid = dev->pub_commid;
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
    snk.comid = dev->sub_commid;
    /* dataset size */
    snk.size = 1024;

    snk.src = dev->ip_cfg.sub_src;
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
                trdp_pd_test_dev.send_etbTopoCnt,                  /* topo counter     */
                trdp_pd_test_dev.send_opTrnTopoCnt,
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
    uint32_t comm_id = 0;
    uint32_t index = 0;

    /* go through ports one-by-one */
    for (i = 0; i < trdp_pd_test_dev.nports; ++i)
    {
        Port * p = &trdp_pd_test_dev.ports[i];
        /* write port data */
        if (p->type == PORT_PUSH || p->type == PORT_PULL)
        {
            comm_id = trdp_pd_test_dev.pub_commid;

            p->data[0] = comm_id;
            p->data[1] = comm_id >> 8;
            p->data[2] = comm_id >> 16;
            p->data[3] = comm_id >> 24;

            p->data[4] = p->sign;
            p->data[5] = p->sign >> 8;
            p->data[6] = p->sign >> 16;
            p->data[7] = p->sign >> 24;
            p->sign++;

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
                rt_kprintf("---recv data:\n");
                for(index = 0; index < p->size; index++)
                {
                    rt_kprintf("%x ", p->data[index]);
                }
                rt_kprintf("\n");

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

static void trdp_pd_set_topology_counter_test(TRDP_PD_TEST *dev)
{
    /*   local etbTopoCnt set    */
    if(dev->load_etbTopoCnt == 11223344)
    {
        dev->load_etbTopoCnt = 0x11223344;
    }
    else if(dev->load_etbTopoCnt == 22334455)
    {
        dev->load_etbTopoCnt = 0x22334455;
    }

    /*   local opTrnTopoCnt set    */
    if(dev->load_opTrnTopoCnt == 55667788)
    {
        dev->load_opTrnTopoCnt = 0x55667788;
    }
    else if(dev->load_opTrnTopoCnt == 66778899)
    {
        dev->load_opTrnTopoCnt = 0x66778899;
    }

    /*   send etbTopoCnt set    */
    if(dev->send_etbTopoCnt == 11223344)
    {
        dev->send_etbTopoCnt = 0x11223344;
    }

    /*   send opTrnTopoCnt set    */
    if(dev->send_opTrnTopoCnt == 55667788)
    {
        dev->send_opTrnTopoCnt = 0x55667788;
    }
}

static void trdp_pd_topolpgy_counter_system_thread(void *paramemter)
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

/* --- trdp_pd_push_topology_counter_system_test ------------------------------------------------------------------*/
void trdp_pd_push_topology_counter_system_test(int argc, char * argv[])
{
    TRDP_ERR_T err;
    rt_thread_t tid;

    LOG_I("%s: (%s - %s)\n",
                          argv[0], __DATE__, __TIME__);

    memset(&trdp_pd_test_dev, 0, sizeof(trdp_pd_test_dev));

    if (argc < 10)
    {
        rt_kprintf("usage: %s <localip> <pub  mcast> <sub mcast> <pub comm id> <sub comm id> <load etbTopoCnt> <load opTrnTopoCnt> <send etbTopoCnt> <send opTrnTopoCnt>\n", argv[0]);
        rt_kprintf("<localip>       .. own IP address (ie. 10.0.1.1)\n");
        rt_kprintf("<remoteip>      .. remote IP address (ie. 10.0.1.2)\n");
        rt_kprintf("<pub mcast>     .. multicast group address (ie. 239.255.3.15)\n");
        rt_kprintf("<sub mcast>     .. multicast group address (ie. 239.255.3.6)\n");
        rt_kprintf("<pub comm id>   .. pub comm id (ie. 315)\n");
        rt_kprintf("<sub comm id>   .. sub comm id (ie. 306)\n");
        rt_kprintf("<load etbTopoCnt>    .. etbTopoCnt (ie. 11223344)\n");
        rt_kprintf("<load opTrnTopoCnt>  .. opTrnTopoCnt (ie. 55667788)\n");
        rt_kprintf("<send etbTopoCnt>    .. etbTopoCnt (ie. 0)\n");
        rt_kprintf("<send opTrnTopoCnt>  .. opTrnTopoCnt (ie. 0)\n");
        return;
    }

    trdp_pd_test_dev.ip_cfg.srcip = vos_dottedIP(argv[1]);       /* 本地IP */
    trdp_pd_test_dev.ip_cfg.sub_src = vos_dottedIP(argv[2]);       /* 远程IP */
    trdp_pd_test_dev.ip_cfg.pub_mcast = vos_dottedIP(argv[3]);   /* 发布者 组播地址 */
    trdp_pd_test_dev.ip_cfg.sub_mcast = vos_dottedIP(argv[4]);   /* 订阅者 组播地址 */
    trdp_pd_test_dev.pub_commid = vos_dottedIP(argv[5]);         /* 发布者 commid */
    trdp_pd_test_dev.sub_commid = vos_dottedIP(argv[6]);         /* 订阅者 commid */
    trdp_pd_test_dev.load_etbTopoCnt = vos_dottedIP(argv[7]);
    trdp_pd_test_dev.load_opTrnTopoCnt = vos_dottedIP(argv[8]);
    trdp_pd_test_dev.send_etbTopoCnt = vos_dottedIP(argv[9]);
    trdp_pd_test_dev.send_opTrnTopoCnt = vos_dottedIP(argv[10]);

    trdp_pd_set_topology_counter_test(&trdp_pd_test_dev);

    LOG_I("send_etbTopoCnt %x %x %x %x",
            trdp_pd_test_dev.send_etbTopoCnt, trdp_pd_test_dev.send_opTrnTopoCnt,
            trdp_pd_test_dev.load_etbTopoCnt, trdp_pd_test_dev.load_opTrnTopoCnt);

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

    tlc_setETBTopoCount(trdp_pd_test_dev.apph, trdp_pd_test_dev.load_etbTopoCnt);
    tlc_setOpTrainTopoCount(trdp_pd_test_dev.apph, trdp_pd_test_dev.load_opTrnTopoCnt);
    vos_threadDelay(2000000);

    tid = rt_thread_create("trdp_pd_topolpgy_counter_system_thread",
                            trdp_pd_topolpgy_counter_system_thread, RT_NULL,
                        (1024 * 6), 20, 5);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(trdp_pd_push_topology_counter_system_test, trdp_pd_push_topology_counter_system_test, trdp pd push topology counter system test);
#endif



