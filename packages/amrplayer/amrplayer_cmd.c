/*
 * Copyright (c) 2006-2030, Hnhinker Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Date           Author       Notes
 * 2023-03-29     ccy    first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <optparse.h>
#include <amrplayer.h>
#include <stdlib.h>

enum amrplayer_ACTTION
{
    amrplayer_ACTION_HELP   = 0,
    amrplayer_ACTION_START  = 1,
    amrplayer_ACTION_STOP   = 2,
    amrplayer_ACTION_PAUSE  = 3,
    amrplayer_ACTION_RESUME = 4,
    amrplayer_ACTION_VOLUME = 5,
    amrplayer_ACTION_DUMP   = 6,
};

struct amrplay_args
{
    int action;
    char *uri;
    int volume;
};

static const char *state_str[] =
{
    "STOPPED",
    "PLAYING",
    "PAUSED",
};

static struct optparse_long opts[] =
{
    {"help", 'h', OPTPARSE_NONE    },       /* 帮助 */
    {"start", 's', OPTPARSE_REQUIRED},      /* 播放 */
    {"stop", 't', OPTPARSE_NONE    },       /* 停止 */
    {"pause", 'p', OPTPARSE_NONE    },      /* 暂停 */
    {"resume", 'r', OPTPARSE_NONE    },     /* 恢复 */
    {"volume", 'v', OPTPARSE_REQUIRED},     /* 音量 */
    {"dump",   'd', OPTPARSE_NONE    },     /* 状态 */
    { NULL,  0,  OPTPARSE_NONE    }
};

static void usage(void)
{
    rt_kprintf("usage: amrplay [option] [target] ...\n\n");
    rt_kprintf("usage options:\n");
    rt_kprintf("  -h,     --help                     Print defined help message.\n");
    rt_kprintf("  -s URI, --start=URI                Play amr music with URI(local files).\n");
    rt_kprintf("  -t,     --stop                     Stop playing music.\n");
    rt_kprintf("  -p,     --pause                    Pause the music.\n");
    rt_kprintf("  -r,     --resume                   Resume the music.\n");
    rt_kprintf("  -v lvl, --volume=lvl               Change the volume(0~100).\n");
    rt_kprintf("  -d,     --dump                     Dump play relevant information.\n");
}

static void dump_status(void)
{
    rt_kprintf("\namrplayer status:\n");
    rt_kprintf("uri     - %s\n", amrplayer_uri_get());
    rt_kprintf("status  - %s\n", state_str[amrplayer_state_get()]);
    rt_kprintf("volume  - %d\n", amrplayer_volume_get());
}

int amrplay_args_prase(int argc, char *argv[], struct amrplay_args *play_args)
{
    int ch;
    int option_index;
    struct optparse options;
    rt_uint8_t action_cnt = 0;
    rt_err_t result = RT_EOK;

    if (argc == 1)
    {
        play_args->action = amrplayer_ACTION_HELP;
        return RT_EOK;
    }

    /* Parse cmd */
    optparse_init(&options, argv);
    while ((ch = optparse_long(&options, opts, &option_index)) != -1)
    {
        switch (ch)
        {
        case 'h':   /* 帮助 */
            play_args->action = amrplayer_ACTION_HELP;
            break;

        case 's':   /* 播放 */
            play_args->action = amrplayer_ACTION_START;
            play_args->uri = options.optarg;
            action_cnt++;
            break;

        case 't':   /* 停止 */
            play_args->action = amrplayer_ACTION_STOP;
            action_cnt++;
            break;

        case 'p':   /* 暂停 */
            play_args->action = amrplayer_ACTION_PAUSE;
            action_cnt++;
            break;

        case 'r':   /* 恢复 */
            play_args->action = amrplayer_ACTION_RESUME;
            action_cnt++;
            break;

        case 'v':   /* 音量 */
            play_args->action = amrplayer_ACTION_VOLUME;
            play_args->volume = (options.optarg == RT_NULL) ? (-1) : atoi(options.optarg);
            action_cnt++;
            break;

        case 'd':   /* 信息 */
            play_args->action = amrplayer_ACTION_DUMP;
            break;

        default:
            result = -RT_EINVAL;
            break;
        }
    }

    /* 判断 播放 暂停 停止 恢复 移动 命令是否多次使用 不能共存使用 */
    if (action_cnt > 1)
    {
        rt_kprintf("START STOP PAUSE RESUME parameter can't be used at the same time.\n");
        result = -RT_EINVAL;
    }

    return result;
}

int amr_player(int argc, char *argv[])
{
    int result = RT_EOK;
    struct amrplay_args play_args = {0};

    result = amrplay_args_prase(argc, argv, &play_args);
    if (result != RT_EOK)
    {
        usage();
        return result;
    }

    switch (play_args.action)
    {
    case amrplayer_ACTION_HELP:
        usage();
        break;

    case amrplayer_ACTION_START:
        amrplayer_play(play_args.uri);
        break;

    case amrplayer_ACTION_STOP:
        amrplayer_stop();
        break;

    case amrplayer_ACTION_PAUSE:
        amrplayer_pause();
        break;

    case amrplayer_ACTION_RESUME:
        amrplayer_resume();
        break;

    case amrplayer_ACTION_VOLUME:
        amrplayer_volume_set(play_args.volume);
        break;

    case amrplayer_ACTION_DUMP:
        dump_status();
        break;

    default:
        result = -RT_ERROR;
        break;
    }

    return result;
}

MSH_CMD_EXPORT_ALIAS(amr_player, amrplay, play amr music);
