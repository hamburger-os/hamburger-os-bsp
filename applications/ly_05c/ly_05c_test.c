/*******************************************************
 *
 * @FileName: ly_05c_test.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: ly-05c产品测试模块
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/
#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>
#include <stdio.h>
#include <event.h>
#include <optparse.h>
#include <stdlib.h>

#include "file_manager.h"
#include "tax.h"
#include "usb.h"
#include "voice.h"
#include "log.h"

/*******************************************************
 * 数据结构
 *******************************************************/

enum LY_05C_ACTTION
{
    LY_05C_ACTION_HELP = 0,
    LY_05C_ACTION_START_RECORD = 1,
    LY_05C_ACTION_END_RECORD = 2,
    LY_05C_ACTION_PLAY_VOICE = 3,
    LY_05C_ACTION_PLAY_EVENT = 4,
    LY_05C_ACTION_USB_COPY = 5,
};

struct ly_05c_args
{
    sint32_t action;
};

/* 播放参数结构体 */
static struct optparse_long opts[] = {
    {"help", 'h', OPTPARSE_NONE},
    {"record", 'r', OPTPARSE_NONE},
    {"stop record", 's', OPTPARSE_NONE},
    {"play voice", 'p', OPTPARSE_NONE},
    {"play event", 'e', OPTPARSE_NONE},
    {"usb", 'u', OPTPARSE_NONE},
    {"usb format", 'f', OPTPARSE_NONE},
    {NULL, 0, OPTPARSE_NONE}};
/*******************************************************
 * 数据结构
 *******************************************************/

/*******************************************************
 *
 * @brief  初始化提示音播放列表
 *
 * @param  argc: 输入参数的个数
 * @param  argv: 输入参数
 * @param  play_args: 播放参数
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
sint32_t ly_05c_args_prase(sint32_t argc, char *argv[], struct ly_05c_args *play_args)
{
    sint32_t ch;
    sint32_t option_index;
    struct optparse options;
    rt_uint8_t action_cnt = 0;
    sint32_t result = RT_EOK;

    if (argc == 1)
    {
        play_args->action = LY_05C_ACTION_HELP;
        return RT_EOK;
    }

    /* Parse cmd */
    optparse_init(&options, argv);
    while ((ch = optparse_long(&options, opts, &option_index)) != -1)
    {
        switch (ch)
        {
        case 'h': /* 帮助 */
            play_args->action = LY_05C_ACTION_HELP;
            action_cnt++;
            break;
        case 'r': /* 录音 */
            play_args->action = LY_05C_ACTION_START_RECORD;
            action_cnt++;
            break;

        case 's': /* 停止录音 */
            play_args->action = LY_05C_ACTION_END_RECORD;
            action_cnt++;
            break;

        case 'p': /* 放音 */
            play_args->action = LY_05C_ACTION_PLAY_VOICE;
            action_cnt++;
            break;

        case 'e': /* 播放提示音 */
            play_args->action = LY_05C_ACTION_PLAY_EVENT;
            action_cnt++;
            break;

        case 'u': /* U盘转储 */
            play_args->action = LY_05C_ACTION_USB_COPY;
            action_cnt++;
            break;

        default:
            result = -RT_EINVAL;
            break;
        }
    }

    if (action_cnt > 1)
    {
        result = -RT_EINVAL;
    }

    return result;
}

/*******************************************************
 *
 * @brief  ly-05c测试程序入口
 *
 * @param  argc: 输入参数的个数
 * @param  argv: 输入参数
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
sint32_t ly_05c_test(sint32_t argc, char *argv[])
{
    sint32_t ret = RT_EOK;
    struct ly_05c_args play_args = {0};

    ret = ly_05c_args_prase(argc, argv, &play_args);
    if (ret != RT_EOK)
    {
        return ret;
    }

    switch (play_args.action)
    {
    case LY_05C_ACTION_HELP:
        break;

    case LY_05C_ACTION_START_RECORD:
        record_start_record();
        sleep(2);
        record_stop_record();
        break;

    case LY_05C_ACTION_END_RECORD:
        record_stop_record();
        break;

    case LY_05C_ACTION_PLAY_VOICE:
        play_voice();
        break;

    case LY_05C_ACTION_PLAY_EVENT:
        play_event(EVENT_DUMP_START_ALL);
        break;
    case LY_05C_ACTION_USB_COPY:
        break;

    default:
        ret = -RT_ERROR;
        break;
    }

    return RT_EOK;
}

MSH_CMD_EXPORT_ALIAS(ly_05c_test, ly_05c_test, ly_05c test interface);
