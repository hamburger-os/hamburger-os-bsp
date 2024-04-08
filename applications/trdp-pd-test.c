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

/* --- globals ---------------------------------------------------------------*/

TRDP_MEM_CONFIG_T memcfg;
TRDP_APP_SESSION_T apph;
TRDP_PD_CONFIG_T pdcfg;
TRDP_PROCESS_CONFIG_T proccfg;

/* default addresses - overriden from command line */
TRDP_IP_ADDR_T srcip;
TRDP_IP_ADDR_T dstip;
TRDP_IP_ADDR_T mcast;

#define APP_VERSION         "1.0"
#define TRDP_PD_TEST_LOCALIP    "10.0.1.3"   /* 本地IP */
#define TRDP_PD_TEST_REMOTEIP   "10.0.1.2"   /* 远端IP */
#define TRDP_PD_TEST_MCAST      "239.255.55.1"   /* 组播 */

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

int size[3] = { 0, 256, TRDP_MAX_PD_DATA_SIZE };     /* small/medium/big dataset */
int period[2]  = { 100, 250 };      /* fast/slow cycle          */
unsigned cycle = 0;

Port ports[64];                     /* array of ports          */
int nports = 0;                     /* number of ports         */

#ifdef TSN_SUPPORT
    #define PORT_FLAGS TRDP_FLAGS_TSN
#else
    #define PORT_FLAGS TRDP_FLAGS_NONE
#endif

/***********************************************************************************************************************
 * PROTOTYPES
 */
void gen_push_ports_master(UINT32 comid, UINT32 echoid);
void gen_pull_ports_slave(UINT32 reqid, UINT32 repid);
void gen_push_ports_slave(UINT32 comid, UINT32 echoid);

/* --- generate PUSH ports ---------------------------------------------------*/

void gen_push_ports_master_one(UINT32 comid, UINT32 echoid)
{
    Port src, snk;
    int num = nports;
    UINT32  a, sz, per;
    
    LOG_I("- generating PUSH ports (master side) ... ");

    memset(&src, 0, sizeof(src));
    memset(&snk, 0, sizeof(snk));

    src.type = PORT_PUSH;
    snk.type = PORT_SINK_PUSH;
    snk.timeout = 4000000;         /* 4 secs timeout*/

    a = 0;

    src.comid = comid;
    snk.comid = echoid;
    /* dataset size */
    src.size = snk.size = (UINT32) size[1];
    /* period [usec] */
    src.cycle = (UINT32) 1000u * (UINT32)period[0];
    /* addresses */
    if (!a)
    {   /* unicast address 单点 */
        src.dst = snk.src = dstip;
        src.src = snk.dst = srcip;
    }
    else
    {   /* multicast address 组播 */
        src.dst = snk.dst = mcast;
        src.src = srcip;
        snk.src = dstip;
    }
    src.link = -1;
    /* add ports to config */
    ports[nports++] = src;
    ports[nports++] = snk;

    LOG_I("%u ports created", nports - num);
}

void gen_push_ports_master(UINT32 comid, UINT32 echoid)
{
    Port src, snk;
    int num = nports;
    UINT32  a, sz, per;

    LOG_I("- generating PUSH ports (master side) ... ");

    memset(&src, 0, sizeof(src));
    memset(&snk, 0, sizeof(snk));

    src.type = PORT_PUSH;
    snk.type = PORT_SINK_PUSH;
    snk.timeout = 4000000;         /* 4 secs timeout*/

    /* for unicast/multicast address */
    for (a = 0; a < 2; ++a)
    {   /* for all dataset sizes */
        for (sz = 1; sz < 3; ++sz)
        {   /* for all cycle periods */
            for (per = 0; per < 2; ++per)
            {   /* comid  */
                src.comid = comid + 100u *a+40*(per+1)+3*(sz+1);
                snk.comid = echoid + 100u *a+40*(per+1)+3*(sz+1);
                /* dataset size */
                src.size = snk.size = (UINT32) size[sz];
                /* period [usec] */
                src.cycle = (UINT32) 1000u * (UINT32)period[per];
                /* addresses */
                if (!a)
                {   /* unicast address */
                    src.dst = snk.src = dstip;
                    src.src = snk.dst = srcip;
                }
                else
                {   /* multicast address */
                    src.dst = snk.dst = mcast;
                    src.src = srcip;
                    snk.src = dstip;
                }
                src.link = -1;
                /* add ports to config */
                ports[nports++] = src;
                ports[nports++] = snk;
            }
        }
    }

    LOG_I("%u ports created", nports - num);
}

void gen_push_ports_slave(UINT32 comid, UINT32 echoid)
{
    Port src, snk;
    int num = nports;
    UINT32 a, sz, per;

    LOG_I("- generating PUSH ports (slave side) ... ");

    memset(&src, 0, sizeof(src));
    memset(&snk, 0, sizeof(snk));

    src.type = PORT_PUSH;
    snk.type = PORT_SINK_PUSH;
    snk.timeout = 4000000;         /* 4 secs timeout */

    /* for unicast/multicast address */
    for (a = 0; a < 2; ++a)
    {   /* for all dataset sizes */
        for (sz = 1; sz < 3; ++sz)
        {   /* for all cycle periods */
            for (per = 0; per < 2; ++per)
            {   /* comid  */
                src.comid = echoid + 100*a+40*(per+1)+3*(sz+1);
                snk.comid = comid + 100*a+40*(per+1)+3*(sz+1);
                /* dataset size */
                src.size = snk.size = (UINT32) size[sz];
                /* period [usec] */
                src.cycle = (UINT32) (1000 * period[per]);
                /* addresses */
                if (!a)
                {   /* unicast address */
                    src.dst = snk.src =  dstip;
                    src.src = snk.dst =  srcip;
                }
                else
                {   /* multicast address */
                    src.dst = snk.dst = mcast;
                    src.src = srcip;
                    snk.src = dstip;
                }
                /* add ports to config */
                ports[nports++] = snk;
                src.link = nports - 1;
                ports[nports++] = src;
            }
        }
    }

    LOG_I("%u ports created", nports - num);
}

/* --- generate PULL ports ---------------------------------------------------*/

static void gen_pull_ports_master(UINT32 reqid, UINT32 repid)
{
    Port req, rep;
    int num = nports;
    UINT32  a, sz;

    LOG_I("- generating PULL ports (master side) ... ");

    memset(&req, 0, sizeof(req));
    memset(&rep, 0, sizeof(rep));

    req.type = PORT_REQUEST;
    rep.type = PORT_SINK;

    /* for unicast/multicast address */
    for (a = 0; a < 2; ++a)
    {   /* for all dataset sizes */
        for (sz = 0; sz < 2; ++sz)
        {   /* comid */
            req.comid = reqid + 100*a + 3*(sz+1);
            rep.comid = repid + 100*a + 3*(sz+1);
            /* dataset size */
            req.size = (UINT32) size[sz];
            rep.size = (UINT32) size[sz + 1];
            /* addresses */
            if (!a)
            {   /* unicast address */
                req.dst = dstip;
                req.src = srcip;
                req.rep = srcip;
                req.repid = rep.comid;
                rep.src = dstip;
                rep.dst = srcip;
            }
            else
            {   /* multicast address */
                req.dst = mcast;
                req.src = srcip;
                req.rep = mcast;
                req.repid = rep.comid;
                rep.dst = mcast;
                rep.src = dstip;
            }
            /* add ports to config */
            ports[nports++] = rep;
            req.link = nports - 1;
            ports[nports++] = req;
        }
    }

    LOG_I("%u ports created", nports - num);
}

void gen_pull_ports_slave(UINT32 reqid, UINT32 repid)
{
    Port req, rep;
    int num = nports;
    UINT32 a, sz;

    LOG_I("- generating PULL ports (slave side) ... ");

    memset(&req, 0, sizeof(req));
    memset(&rep, 0, sizeof(rep));

    req.type = PORT_SINK;
    rep.type = PORT_PULL;
    req.timeout = 4000000;      /* 4 secs timeout */

    /* for unicast/multicast address */
    for (a = 0; a < 2; ++a)
    {   /* for all dataset sizes */
        for (sz = 0; sz < 2; ++sz)
        {   /* comid */
            req.comid = reqid + 100*a + 3*(sz+1);
            rep.comid = repid + 100*a + 3*(sz+1);
            /* dataset size */
            req.size = (UINT32) size[sz];
            rep.size = (UINT32) size[sz + 1];
            /* addresses */
            if (!a)
            {   /* unicast address */
                req.dst = srcip;
                req.src = dstip;
                rep.src = srcip;
                rep.dst = 0;
            }
            else
            {   /* multicast address */
                req.dst = mcast;
                req.src = dstip;
                rep.src = srcip;
                rep.dst = 0;
            }
            /* add ports to config */
            ports[nports++] = req;
            rep.link = nports - 1;
            ports[nports++] = rep;
        }
    }

    LOG_I("%u ports created", nports - num);
}

/* --- setup ports -----------------------------------------------------------*/

static void setup_ports()
{
    int i;

    LOG_I("setup_ports:");
    /* setup ports one-by-one */
    for (i = 0; i < nports; ++i)
    {
        Port * p = &ports[i];
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
                apph,               /* session handle */
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
                apph,               /* session handle */
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
                apph,               /* session handle */
                ports[p->link].sh,  /* related subscribe handle */
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
                apph,               /* session handle   */
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
                apph,               /* session handle   */
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
    int i = 0, index = 0;
    unsigned n;
    TRDP_COM_PARAM_T comPrams = TRDP_PD_DEFAULT_SEND_PARAM;
#if PORT_FLAGS == TRDP_FLAGS_TSN
    comPrams.vlan = 1;
    comPrams.tsn = TRUE;
#endif

    /* go through ports one-by-one */
    for (i = 0; i < nports; ++i)
    {
        Port * p = &ports[i];
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
                unsigned char * src = ports[p->link].data;
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
            p->err = tlp_put(apph, p->ph, p->data, p->size);

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

            p->err = tlp_request(apph, ports[p->link].sh, 0u, p->comid, 0u, 0u,
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
    }
    /* increment cycle counter  */
    ++cycle;
}

/* --- poll received data ----------------------------------------------------*/

static void poll_data()
{
    TRDP_PD_INFO_T pdi;
    int i;
    int k = 0;
    static uint32_t expect_seqCount = 0;

    /* go through ports one-by-one */
    for (i = 0; i < nports; ++i)
    {
        Port * p = &ports[i];
        UINT32 size = p->size;
        if (p->type == PORT_SINK || p->type == PORT_SINK_PUSH)
        {
            p->err = tlp_get(apph, p->sh, &pdi, p->data, &size);

            if(p->err == TRDP_NO_ERR)
            {
                if(pdi.seqCount == expect_seqCount)
                {
                    rt_kprintf("!!!%d\n", pdi.seqCount);
                    for(k = 0; k < size; k++)
                    {
                        rt_kprintf("%x ", p->data[k]);
                    }
                    rt_kprintf("\n");
                    expect_seqCount++;
                }
            }
            memset(&pdi, 0, sizeof(TRDP_PD_INFO_T));
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

    /* main test loop */
    while (1)
    {   /* drive TRDP communications */
        tlc_process(apph, NULL, NULL);
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

    srcip = vos_dottedIP(TRDP_PD_TEST_LOCALIP);
    dstip = vos_dottedIP(TRDP_PD_TEST_REMOTEIP);
    mcast = vos_dottedIP(TRDP_PD_TEST_MCAST);

#ifdef SIM
    if (argc < 6)
    {
        printf("In simulation mode an extra last argument is required <Unike thread name>\n");
        return 1;
    }
    vos_setTimeSyncPrefix(argv[5]);

    if (!SimSetHostIp(argv[1]))
        printf("Failed to set sim host IP.");
#endif

    if (!srcip || !dstip || (mcast >> 28) != 0xE)
    {
        LOG_E("invalid ip arguments\n");
    }

    memset(&memcfg, 0, sizeof(memcfg));
    memset(&proccfg, 0, sizeof(proccfg));

    /* initialize TRDP protocol library */
    err = tlc_init(printLog, NULL, &memcfg);
    if (err != TRDP_NO_ERR)
    {
        LOG_E("tlc_init() failed, err: %d\n", err);
    }
#ifdef SIM
    vos_threadRegister("main", TRUE);
#endif
    pdcfg.pfCbFunction = NULL;
    pdcfg.pRefCon = NULL;
    pdcfg.sendParam.qos = 5;
    pdcfg.sendParam.ttl = 64;
    pdcfg.flags = TRDP_FLAGS_NONE;
    pdcfg.timeout = 10000000;
    pdcfg.toBehavior = TRDP_TO_SET_TO_ZERO;
    pdcfg.port = 17224;

    /* open session */
    err = tlc_openSession(&apph, srcip, 0, NULL, &pdcfg, NULL, &proccfg);
    if (err != TRDP_NO_ERR)
    {
        LOG_E("tlc_openSession() failed, err: %d\n", err);
    }

    /* generate ports configuration */
#if 0
    gen_push_ports_master(10000, 20000);
    gen_push_ports_slave(10000, 20000);
    gen_pull_ports_master(30000, 40000);
    gen_pull_ports_slave(30000, 40000);
#else
    gen_push_ports_master_one(21761, 20000);
#endif
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
