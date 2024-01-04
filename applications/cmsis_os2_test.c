/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-12-13     lvhan       the first version
 */
#include <cmsis_os2.h>

#define DBG_TAG "cmsisex"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

struct THREADARGDef
{
    uint32_t num;
};
static struct THREADARGDef cmsis_thread_argument = { .num = 0, };

osMessageQueueId_t mid_MsgQueue;                // message queue id

typedef struct
{                                // object data type
    uint8_t Buf[32];
    uint8_t Idx;
} MSGQUEUE_OBJ_t;

static int thread_is_run = 0;
static osThreadId_t cmsisos_thread = NULL;

static void msg_thread(void *argument)
{
    MSGQUEUE_OBJ_t msg;
    osStatus_t status;
    osThreadId_t thread_id;

    thread_id = osThreadGetId();
    LOG_D("thread '%s' %p", osThreadGetName(thread_id), thread_id);
    while (thread_is_run)
    {
        msg.Buf[0] = 0x55U;
        msg.Idx = osKernelGetTickCount();
        status = osMessageQueuePut(mid_MsgQueue, &msg, 0U, 0U);
        if (status != osOK)
        {
            LOG_E("osMessageQueuePut error");
        }
        else
        {
            LOG_D("osMessageQueuePut %d %d", msg.Buf[0], msg.Idx);
        }
        osDelay(500);
    }
    LOG_D("thread '%s' %p end", osThreadGetName(thread_id), thread_id);
}

static void cmsis_thread(void *argument)
{
    struct THREADARGDef *arg = (struct THREADARGDef *) argument;
    MSGQUEUE_OBJ_t msg;
    osStatus_t status;
    osThreadId_t thread_id;

    mid_MsgQueue = osMessageQueueNew(16, sizeof(MSGQUEUE_OBJ_t), NULL);
    if (mid_MsgQueue == NULL)
    {
        LOG_E("osMessageQueueNew error");
    }

    const osThreadAttr_t msg_thread_attr = { .name = "osmsg", .stack_size = 2048, .priority = osPriorityNormal, };
    thread_id = osThreadNew(msg_thread, NULL, &msg_thread_attr);
    if (thread_id == NULL)
    {
        LOG_E("osThreadNew error");
    }

    thread_id = osThreadGetId();
    LOG_D("thread '%s' %p", osThreadGetName(thread_id), thread_id);
    while (thread_is_run)
    {
        status = osMessageQueueGet(mid_MsgQueue, &msg, NULL, osWaitForever);
        if (status == osOK)
        {
            arg->num = osKernelGetTickCount();
            LOG_D("osMessageQueueGet %d %d", arg->num, msg.Idx);
        }
        else
        {
            LOG_E("osMessageQueueGet error");
        }
    }
    cmsisos_thread = NULL;
    LOG_D("thread '%s' %p end", osThreadGetName(thread_id), thread_id);
}

static void rt_cmsis_os2_example(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("Usage: cmsisostest [cmd]\n");
        rt_kprintf("       cmsisostest --start\n");
        rt_kprintf("       cmsisostest --stop\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--start") == 0)
        {
            if (cmsisos_thread == NULL)
            {
                const osThreadAttr_t cmsis_thread_attr = { .name = "osthread", .stack_size = 2048, .priority =
                        osPriorityNormal, };
                cmsisos_thread = osThreadNew(cmsis_thread, &cmsis_thread_argument, &cmsis_thread_attr);
                if (cmsisos_thread == NULL)
                {
                    rt_kprintf("osThreadNew error\n");
                }
                else
                {
                    thread_is_run = 1;
                }
            }
            else
            {
                rt_kprintf("thread already exists.\n");
            }
        }
        else if (rt_strcmp(argv[1], "--stop") == 0)
        {
            cmsisos_thread = NULL;
            thread_is_run = 0;
        }
        else
        {
            rt_kprintf("cmd does not exist!\n");
            rt_kprintf("Usage: cmsisostest [cmd]\n");
            rt_kprintf("       cmsisostest --start\n");
            rt_kprintf("       cmsisostest --stop\n");
        }
    }
}
#ifdef RT_USING_FINSH
#include <finsh.h>
#ifdef RT_USING_FINSH
MSH_CMD_EXPORT_ALIAS(rt_cmsis_os2_example, cmsisostest, cmsis os2 test);
#endif /* RT_USING_FINSH */
#endif /* RT_USING_FINSH */
