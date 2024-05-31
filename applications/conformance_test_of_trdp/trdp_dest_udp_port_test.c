/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-09     zm       the first version
 */

#include "trdp_test_public.h"
#include "trdp_if_light.h"
#include "vos_thread.h"
#include "vos_utils.h"

#define DBG_TAG "trdp_pd_dest_udp_port_test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/select.h>

#define TRDP_PD_DEST_UDP_PORT_TEST_PORT_NUM (6)

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
//    unsigned char data[TRDP_MAX_PD_DATA_SIZE];       /* data buffer                          */
    UINT32 data[TRDP_MAX_PD_DATA_SIZE];       /* data buffer                          */
    uint32_t sign;
    int link;                       /* index of linked port (echo or subscribe) */
} Port;

typedef struct {
    /* default addresses - overriden from command line */
    TRDP_IP_ADDR_T srcip;
    TRDP_IP_ADDR_T dstip;
    TRDP_IP_ADDR_T mcast;
} TRDP_PD_TEST_IP_CFG;

typedef struct {
    TRDP_PD_TEST_IP_CFG ip_cfg;

    uint32_t pub_commid;
    uint32_t sub_commid;
    uint16_t port;

    Port ports[TRDP_PD_DEST_UDP_PORT_TEST_PORT_NUM];   /* array of ports          */
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
    src.dst = dev->ip_cfg.mcast;
    src.src = dev->ip_cfg.srcip;
#endif

    src.link = -1;

    /* add ports to config */
    dev->ports[dev->nports++] = src;
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
#if PORT_FLAGS == TRDP_FLAGS_TSN
        comPrams.vlan = 1;
        comPrams.tsn = TRUE;
#endif

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

    /* go through ports one-by-one */
    for (i = 0; i < trdp_pd_test_dev.nports; ++i)
    {
        Port * p = &trdp_pd_test_dev.ports[i];
        /* write port data */
        if (p->type == PORT_PUSH || p->type == PORT_PULL)
        {
            if (p->link == -1)
            {   /* data generator */
                memset(p->data, 0, p->size);
                p->data[0] = 301;
                p->data[1] = p->sign;
                p->sign+=1;
            }

            p->err = tlp_put(trdp_pd_test_dev.apph, p->ph, (const UINT8 *)p->data, p->size);

//            LOG_I("send commit id %d %s", p->comid, types[p->type]);
//            for(index = 0; index < p->size; index++)
//            {
//                rt_kprintf("%x ", p->data[index]);
//            }
//            rt_kprintf("\n");
            if (p->err != TRDP_NO_ERR)
            {
                LOG_E("PORT_PU-- %s", trdp_test_public_get_result_string(p->err));
            }
        }
    }
}

static void trdp_pd_dest_udp_port_test_thread(void *paramemter)
{
    static uint32_t current_ms = 0;
    /* trdp_pd_test loop */
    while (1)
    {   /* drive TRDP communications */
        tlc_process(trdp_pd_test_dev.apph, NULL, NULL);

        /* process data every 1000 ms */
        if(trdp_test_timeout_ms(&current_ms, 1000))
        {
            process_data();
        }

        rt_thread_mdelay(1);
    }
}

/* --- trdp_pd_dest_udp_port_test ------------------------------------------------------------------*/
void trdp_pd_dest_udp_port_test(int argc, char * argv[])
{
    TRDP_ERR_T err;
    rt_thread_t tid;

    LOG_I("%s: (%s - %s)\n",
                          argv[0], __DATE__, __TIME__);

    if (argc < 3)
    {
        rt_kprintf("usage: %s <localip> <mcast> <pub comm id>\n", argv[0]);
        rt_kprintf("<localip>       .. own IP address (ie. 10.0.1.1)\n");
        rt_kprintf("<mcast>         .. multicast group address (ie. 239.255.3.1)\n");
        rt_kprintf("<port>          .. udp port (ie. 17000/17224)\n");
        return;
    }

    trdp_pd_test_dev.ip_cfg.srcip = vos_dottedIP(argv[1]);       /* 本地IP */
    trdp_pd_test_dev.ip_cfg.mcast = vos_dottedIP(argv[2]);       /* 组播地址 */
    trdp_pd_test_dev.port = vos_dottedIP(argv[3]);               /* 端口号 */

    if (!trdp_pd_test_dev.ip_cfg.srcip || (trdp_pd_test_dev.ip_cfg.mcast >> 28) != 0xE)
    {
        LOG_E("invalid ip arguments\n");
    }

    memset(&trdp_pd_test_dev.memcfg, 0, sizeof(TRDP_MEM_CONFIG_T));
    memset(&trdp_pd_test_dev.proccfg, 0, sizeof(TRDP_PROCESS_CONFIG_T));

    trdp_pd_test_dev.pub_commid = 301;

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
    trdp_pd_test_dev.pdcfg.port = trdp_pd_test_dev.port;

    /* open session */
    err = tlc_openSession(&trdp_pd_test_dev.apph, trdp_pd_test_dev.ip_cfg.srcip, 0, NULL, &trdp_pd_test_dev.pdcfg, NULL, &trdp_pd_test_dev.proccfg);
    if (err != TRDP_NO_ERR)
    {
        LOG_E("tlc_openSession() failed, err: %d\n", err);
        return;
    }

    /* generate ports configuration */
    set_gen_pub_config(&trdp_pd_test_dev);

    setup_ports();
    vos_threadDelay(2000000);

    tid = rt_thread_create("trdp_pd_dest_udp_port_test_thread",
                            trdp_pd_dest_udp_port_test_thread, RT_NULL,
                        (1024 * 6), 20, 5);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(trdp_pd_dest_udp_port_test, trdp_pd_dest_udp_port_test, trdp pd dest udp port test);
#endif
