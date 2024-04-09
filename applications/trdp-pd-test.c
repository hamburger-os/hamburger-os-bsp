/**********************************************************************************************************************/
/**
 * @file            trdp-pd-test.c
 *
 * @brief           Test application for TRDP PD
 *
 * @details
 *
 * @note            Project: TCNOpen TRDP prototype stack
 *
 * @author          Petr Cvachou?ek, UniControls
 *
 * @remarks This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 *          If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *          Copyright UniControls, 2013. All rights reserved.
 *
 * $Id: trdp-pd-test.c 2205 2020-08-20 12:20:43Z bloehr $
 *
 *      A� 2020-05-04: Ticket #330: Extend TRDP_PDTest with TSN support
 *      A� 2019-11-11: Ticket #290: Add support for Virtualization on Windows
 *      BL 2018-06-20: Ticket #184: Building with VS 2015: WIN64 and Windows threads (SOCKET instead of INT32)
 *      BL 2018-03-06: Ticket #101 Optional callback function on PD send
 *      BL 2017-06-30: Compiler warnings, local prototypes added
 *
 */

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/select.h>

#include "trdp_if_light.h"
#include "vos_thread.h"
#include "vos_utils.h"

#define DBG_TAG "trdp_pd_test"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>


#define APP_VERSION         "1.0"

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
    Port ports[64];                     /* array of ports          */
    uint32_t nports;                    /* number of ports         */
    TRDP_PD_INFO_T pdi;

    TRDP_MEM_CONFIG_T memcfg;
    TRDP_APP_SESSION_T apph;
    TRDP_PD_CONFIG_T pdcfg;
    TRDP_PROCESS_CONFIG_T proccfg;
} TRDP_PD_TEST;

int size[3] = { 0, 256, TRDP_MAX_PD_DATA_SIZE };     /* small/medium/big dataset */
int period[2]  = { 100, 250 };      /* fast/slow cycle          */
unsigned cycle = 0;

static TRDP_PD_TEST trdp_pd_test_dev;

#ifdef TSN_SUPPORT
    #define PORT_FLAGS TRDP_FLAGS_TSN
#else
    #define PORT_FLAGS TRDP_FLAGS_NONE
#endif

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
    src.size = 256;
    /* period [usec] */
    src.cycle = (UINT32) 1000u * (UINT32)100; /* 100ms */

    /* unicast address 单点 */
    src.dst = dev->ip_cfg.dstip;
    src.src = dev->ip_cfg.srcip;

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

    memset(&snk, 0, sizeof(snk));

    snk.type = PORT_SINK_PUSH;
    snk.timeout = 4000000;         /* 4 secs timeout*/
    snk.comid = dev->sub_commid;
    /* dataset size */
    snk.size = 256;

    snk.src = dev->ip_cfg.dstip;
    snk.dst = dev->ip_cfg.srcip;

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

/* --- convert trdp error code to string -------------------------------------*/

static const char * get_result_string(int ret)
{
    static char buf[128];

    switch (ret)
    {
    case TRDP_NO_ERR:
        return "TRDP_NO_ERR (no error)";
    case TRDP_PARAM_ERR:
        return "TRDP_PARAM_ERR (parameter missing or out of range)";
    case TRDP_INIT_ERR:
        return "TRDP_INIT_ERR (call without valid initialization)";
    case TRDP_NOINIT_ERR:
        return "TRDP_NOINIT_ERR (call with invalid handle)";
    case TRDP_TIMEOUT_ERR:
        return "TRDP_TIMEOUT_ERR (timeout)";
    case TRDP_NODATA_ERR:
        return "TRDP_NODATA_ERR (non blocking mode: no data received)";
    case TRDP_SOCK_ERR:
        return "TRDP_SOCK_ERR (socket error / option not supported)";
    case TRDP_IO_ERR:
        return "TRDP_IO_ERR (socket IO error, data can't be received/sent)";
    case TRDP_MEM_ERR:
        return "TRDP_MEM_ERR (no more memory available)";
    case TRDP_SEMA_ERR:
        return "TRDP_SEMA_ERR semaphore not available)";
    case TRDP_QUEUE_ERR:
        return "TRDP_QUEUE_ERR (queue empty)";
    case TRDP_QUEUE_FULL_ERR:
        return "TRDP_QUEUE_FULL_ERR (queue full)";
    case TRDP_MUTEX_ERR:
        return "TRDP_MUTEX_ERR (mutex not available)";
    case TRDP_NOSESSION_ERR:
        return "TRDP_NOSESSION_ERR (no such session)";
    case TRDP_SESSION_ABORT_ERR:
        return "TRDP_SESSION_ABORT_ERR (Session aborted)";
    case TRDP_NOSUB_ERR:
        return "TRDP_NOSUB_ERR (no subscriber)";
    case TRDP_NOPUB_ERR:
        return "TRDP_NOPUB_ERR (no publisher)";
    case TRDP_NOLIST_ERR:
        return "TRDP_NOLIST_ERR (no listener)";
    case TRDP_CRC_ERR:
        return "TRDP_CRC_ERR (wrong CRC)";
    case TRDP_WIRE_ERR:
        return "TRDP_WIRE_ERR (wire error)";
    case TRDP_TOPO_ERR:
        return "TRDP_TOPO_ERR (invalid topo count)";
    case TRDP_COMID_ERR:
        return "TRDP_COMID_ERR (unknown comid)";
    case TRDP_STATE_ERR:
        return "TRDP_STATE_ERR (call in wrong state)";
    case TRDP_APP_TIMEOUT_ERR:
        return "TRDP_APPTIMEOUT_ERR (application timeout)";
    case TRDP_UNKNOWN_ERR:
        return "TRDP_UNKNOWN_ERR (unspecified error)";
    }
    sprintf(buf, "unknown error (%d)", ret);
    return buf;
}

/* --- test data processing --------------------------------------------------*/

static void process_data(void)
{
    int i = 0;
    unsigned n;
    TRDP_COM_PARAM_T comPrams = TRDP_PD_DEFAULT_SEND_PARAM;
#if PORT_FLAGS == TRDP_FLAGS_TSN
    comPrams.vlan = 1;
    comPrams.tsn = TRUE;
#endif
    static uint32_t expect_seqCount = 0;
    int k = 0;

    /* go through ports one-by-one */
    for (i = 0; i < trdp_pd_test_dev.nports; ++i)
    {
        Port * p = &trdp_pd_test_dev.ports[i];
        /* write port data */
        if (p->type == PORT_PUSH || p->type == PORT_PULL)
        {
            if (p->link == -1)
            {   /* data generator */
                unsigned o = cycle % 128;
                memset(p->data, '_', p->size);
                if (o < p->size)
                {
                    snprintf((char *) p->data + o, p->size - o,
                        "<%s/%ld.%ld.%ld.%ld->%ld.%ld.%ld.%ld/%ldms/%ldb:%d>",
                        p->type == PORT_PUSH ? "Pd" : "Pp",
                        (p->src >> 24) & 0xff, (p->src >> 16) & 0xff,
                        (p->src >> 8) & 0xff, p->src & 0xff,
                        (p->dst >> 24) & 0xff, (p->dst >> 16) & 0xff,
                        (p->dst >> 8) & 0xff, p->dst & 0xff,
                        p->cycle / 1000, p->size, cycle);
                }
            }
            else
            {   /* echo data from incoming port, replace all '_' by '~' */
                unsigned char * src = trdp_pd_test_dev.ports[p->link].data;
                unsigned char * dst = p->data;
                for (n = p->size; n; --n, ++src, ++dst)
                    *dst = (*src == '_') ? '~' : *src;
            }
#if PORT_FLAGS == TRDP_FLAGS_TSN
            if (p->type == PORT_PUSH)
            {
                p->err = tlp_putImmediate(apph, p->ph, p->data, p->size, 0);
            }
            else
            {
                p->err = tlp_put(apph, p->ph, p->data, p->size);
            }
#else
            p->err = tlp_put(trdp_pd_test_dev.apph, p->ph, p->data, p->size);

#endif

//            LOG_I("send commit id %d %s", p->comid, types[p->type]);
//            for(index = 0; index < p->size; index++)
//            {
//                rt_kprintf("%x ", p->data[index]);
//            }
//            rt_kprintf("\n");
            if (p->err != TRDP_NO_ERR)
            {
                LOG_E("PORT_PU-- %s", get_result_string(p->err));
            }
        }
        else if (p->type == PORT_REQUEST)
        {
            unsigned o = cycle % 128;
            memset(p->data, '_', p->size);
            if (o < p->size)
            {
                snprintf((char *) p->data + o, p->size - o,
                    "<Pr/%ld.%ld.%ld.%ld->%ld.%ld.%ld.%ld/%ldms/%ldb:%d>",
                    (p->src >> 24) & 0xff, (p->src >> 16) & 0xff,
                    (p->src >> 8) & 0xff, p->src & 0xff,
                    (p->dst >> 24) & 0xff, (p->dst >> 16) & 0xff,
                    (p->dst >> 8) & 0xff, p->dst & 0xff,
                    p->cycle / 1000, p->size, cycle);
            }

            p->err = tlp_request(trdp_pd_test_dev.apph, trdp_pd_test_dev.ports[p->link].sh, 0u, p->comid, 0u, 0u,
                p->src, p->dst, 0, PORT_FLAGS, &comPrams, p->data, p->size,
                p->repid, p->rep);

//            LOG_I("recv commit id %d %s", p->comid, types[p->type]);
//            for(index = 0; index < p->size; index++)
//            {
//                rt_kprintf("%x ", p->data[index]);
//            }
//            rt_kprintf("\n");
            if (p->err != TRDP_NO_ERR)
            {
                LOG_E("PORT_REQUEST-- %s", get_result_string(p->err));
            }
        }
        else if(p->type == PORT_SINK_PUSH)
        {
            if(p->err == TRDP_NO_ERR)
            {
                if(trdp_pd_test_dev.pdi.seqCount == expect_seqCount)
                {
                    rt_kprintf("***recv seqcount %d etbTopoCnt %d opTrnTopoCnt %d size %d***\n",
                            trdp_pd_test_dev.pdi.seqCount, trdp_pd_test_dev.pdi.etbTopoCnt, trdp_pd_test_dev.pdi.opTrnTopoCnt, p->size);
                    for(k = 0; k < p->size; k++)
                    {
                        rt_kprintf("%x ", p->data[k]);
                    }
                    rt_kprintf("\n");
                    expect_seqCount++;
                }
            }
            memset(&trdp_pd_test_dev.pdi, 0, sizeof(TRDP_PD_INFO_T));
        }
    }
    /* increment cycle counter  */
    ++cycle;
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

static void printLog(
    void        *pRefCon,
    VOS_LOG_T   category,
    const CHAR8 *pTime,
    const CHAR8 *pFile,
    UINT16      line,
    const CHAR8 *pMsgStr)
{
    (void) pRefCon;

    const char *catStr[] = {"Error:", "Warning:", "Info:", "Debug:", "User:"};
    CHAR8       *pF = strrchr(pFile, VOS_DIR_SEP);

    rt_kprintf("%s %s:%d %s",
           catStr[category],
           (pF == NULL)? "" : pF + 1,
           line,
           pMsgStr);
}

static void trdp_pd_test_thread(void *paramemter)
{
    unsigned tick = 0;

    /* trdp_pd_test loop */
    while (1)
    {   /* drive TRDP communications */
        tlc_process(trdp_pd_test_dev.apph, NULL, NULL);
        /* poll (receive) data */
        poll_data();
        /* process data every 500 msec */
        if (!(++tick % 50))
            process_data();

        rt_thread_mdelay(10);
    }
}

/* --- trdp_pd_test ------------------------------------------------------------------*/
void trdp_pd_test(int argc, char * argv[])
{
    TRDP_ERR_T err;
    rt_thread_t tid;

    LOG_I("%s: Version %s (%s - %s)\n",
                          argv[0], APP_VERSION, __DATE__, __TIME__);

    if (argc < 4)
    {
        rt_kprintf("usage: %s <localip> <remoteip> <mcast> <pub comm id> <sub comm id>\n", argv[0]);
        rt_kprintf("<localip>       .. own IP address (ie. 10.0.1.3)\n");
        rt_kprintf("<remoteip>      .. remote peer IP address (ie. 10.0.1.2)\n");
        rt_kprintf("<mcast>         .. multicast group address (ie. 239.255.55.1)\n");
        rt_kprintf("<mcast>         .. multicast group address (ie. 239.255.55.1)\n");
        rt_kprintf("<pub comm id>   .. pub comm id (ie. 21761)\n");
        rt_kprintf("<sub comm id>   .. pub comm id (ie. 20000)\n");
        return;
    }

    trdp_pd_test_dev.ip_cfg.srcip = vos_dottedIP(argv[1]);       /* 本地IP */
    trdp_pd_test_dev.ip_cfg.dstip = vos_dottedIP(argv[2]);       /* 远端IP */
    trdp_pd_test_dev.ip_cfg.mcast = vos_dottedIP(argv[3]);       /* 组播地址 */
    trdp_pd_test_dev.pub_commid = vos_dottedIP(argv[4]);         /* 发布者 commid */
    trdp_pd_test_dev.sub_commid = vos_dottedIP(argv[5]);         /* 订阅者 commid  */

    if (!trdp_pd_test_dev.ip_cfg.srcip || !trdp_pd_test_dev.ip_cfg.dstip || (trdp_pd_test_dev.ip_cfg.mcast >> 28) != 0xE)
    {
        LOG_E("invalid ip arguments\n");
    }

    memset(&trdp_pd_test_dev.memcfg, 0, sizeof(TRDP_MEM_CONFIG_T));
    memset(&trdp_pd_test_dev.proccfg, 0, sizeof(TRDP_PROCESS_CONFIG_T));

    /* initialize TRDP protocol library */
    err = tlc_init(printLog, NULL, &trdp_pd_test_dev.memcfg);
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

    tid = rt_thread_create("trdp_pd_test",
                         trdp_pd_test_thread, RT_NULL,
                        (1024 * 6), 20, 5);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(trdp_pd_test, trdp_pd_test, trdp pd test);
#endif

/* ---------------------------------------------------------------------------*/
