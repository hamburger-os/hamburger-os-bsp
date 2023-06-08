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
#include <amrhdr.h>
#include <amrplayer.h>
#include <rtdbg.h>
#include <stdio.h>
#include <sp_enc.h>

// #define DBG_TAG "amr_PLAYER"
#define DBG_LVL DBG_INFO

#define VOLUME_MIN (0)
#define VOLUME_MAX (100)

// #define WP_BUFFER_SIZE (2048)
#define WP_BUFFER_SIZE (320 * 2)
#define WP_VOLUME_DEFAULT (50)
#define WP_MSG_SIZE (10)
#define WP_THREAD_STATCK_SIZE (2048)
#define WP_THREAD_PRIORITY (15)

enum MSG_TYPE
{
    MSG_NONE = 0,
    MSG_START = 1,
    MSG_STOP = 2,
    MSG_PAUSE = 3,
    MSG_RESUME = 4,
};

enum PLAYER_EVENT
{
    PLAYER_EVENT_NONE = 0,
    PLAYER_EVENT_PLAY = 1,
    PLAYER_EVENT_STOP = 2,
    PLAYER_EVENT_PAUSE = 3,
    PLAYER_EVENT_RESUME = 4,
};

struct play_msg
{
    int type;
    void *data;
};

struct amrplayer
{
    int state;
    char *uri;
    char *buffer;
    rt_device_t device;
    rt_mq_t mq;
    rt_mutex_t lock;
    struct rt_completion ack;
    FILE *fp;
    int volume;
    uint32_t start;
};

static struct amrplayer player;

#if (DBG_LEVEL >= DBG_LOG)

static const char *state_str[] =
    {
        "STOPPED",
        "PLAYING",
        "PAUSED",
};

static const char *event_str[] =
    {
        "NONE",
        "PLAY"
        "STOP"
        "PAUSE"
        "RESUME"};

#endif

static void play_lock(void)
{
    rt_mutex_take(player.lock, RT_WAITING_FOREVER);
}

static void play_unlock(void)
{
    rt_mutex_release(player.lock);
}

static rt_err_t play_msg_send(struct amrplayer *player, int type, void *data)
{
    struct play_msg msg;

    msg.type = type;
    msg.data = data;

    return rt_mq_send(player->mq, &msg, sizeof(struct play_msg));
}

int amrplayer_play(char *uri)
{
    rt_err_t result = RT_EOK;

    rt_completion_init(&player.ack);

    play_lock();
    if (player.state != PLAYER_STATE_STOPED)
    {
        amrplayer_stop();
    }
    if (player.uri)
    {
        rt_free(player.uri);
    }
    player.uri = rt_strdup(uri);
    result = play_msg_send(&player, MSG_START, RT_NULL);
    rt_completion_wait(&player.ack, RT_WAITING_FOREVER);
    play_unlock();

    return result;
}

int amrplayer_stop(void)
{
    rt_err_t result = RT_EOK;

    rt_completion_init(&player.ack);

    play_lock();
    if (player.state != PLAYER_STATE_STOPED)
    {
        result = play_msg_send(&player, MSG_STOP, RT_NULL);
        rt_completion_wait(&player.ack, RT_WAITING_FOREVER);
    }
    play_unlock();

    return result;
}

int amrplayer_pause(void)
{
    rt_err_t result = RT_EOK;

    rt_completion_init(&player.ack);

    play_lock();
    if (player.state == PLAYER_STATE_PLAYING)
    {
        result = play_msg_send(&player, MSG_PAUSE, RT_NULL);
        rt_completion_wait(&player.ack, RT_WAITING_FOREVER);
    }
    play_unlock();

    return result;
}

int amrplayer_resume(void)
{
    rt_err_t result = RT_EOK;

    rt_completion_init(&player.ack);

    play_lock();
    if (player.state == PLAYER_STATE_PAUSED)
    {
        result = play_msg_send(&player, MSG_RESUME, RT_NULL);
        rt_completion_wait(&player.ack, RT_WAITING_FOREVER);
    }
    play_unlock();

    return result;
}

int amrplayer_volume_set(int volume)
{
    struct rt_audio_caps caps;

    if (volume < VOLUME_MIN)
        volume = VOLUME_MIN;
    else if (volume > VOLUME_MAX)
        volume = VOLUME_MAX;

    player.device = rt_device_find(AMRPLAYER_PLAY_DEVICE);
    if (player.device == RT_NULL)
        return -RT_ERROR;

    player.volume = volume;
    caps.main_type = AUDIO_TYPE_MIXER;
    caps.sub_type = AUDIO_MIXER_VOLUME;
    caps.udata.value = volume;

    // LOG_E("set volume = %d", volume);
    return rt_device_control(player.device, AUDIO_CTL_CONFIGURE, &caps);
}

int amrplayer_volume_get(void)
{
    return player.volume;
}

int amrplayer_state_get(void)
{
    return player.state;
}

char *amrplayer_uri_get(void)
{
    return player.uri;
}

static rt_err_t amrplayer_open(struct amrplayer *player)
{
    rt_err_t result = RT_EOK;
    struct rt_audio_caps caps;
    int ret = 0;
    char magic[8];
    int dtx = 1;
    int i = 0;

    /* find device */
    player->device = rt_device_find(AMRPLAYER_PLAY_DEVICE);
    if (player->device == RT_NULL)
    {
        LOG_E("device %s not find", AMRPLAYER_PLAY_DEVICE);
        return -RT_ERROR;
    }

    /* open file */
    player->fp = fopen(player->uri, "rb");
    if (player->fp == RT_NULL)
    {
        LOG_E("open file %s failed\n", player->uri);
        result = -RT_ERROR;
        goto __exit;
    }

    /* read and verify magic number */
    ret = fread(magic, sizeof(char), strlen(AMR_MAGIC_NUMBER), player->fp);
    if (ret != strlen(AMR_MAGIC_NUMBER))
    {
        return -1;
    }
    if (strncmp(magic, AMR_MAGIC_NUMBER, strlen(AMR_MAGIC_NUMBER)))
    {
        LOG_E("%s%s\n", "Invalid magic number: ", magic);
        fclose(player->fp);
        return -1;
    }

    caps.main_type = AUDIO_TYPE_OUTPUT;
    caps.sub_type = AUDIO_DSP_PARAM;
    caps.udata.config.samplerate = 8000;
    caps.udata.config.channels = 1;
    caps.udata.config.samplebits = 16;
    /* read amrfile header information from file */
    LOG_I("Information: samplerate %d, channels %d, samplebits %d.\n",
           caps.udata.config.samplerate, caps.udata.config.channels, caps.udata.config.samplebits);

    player->start = rt_tick_get_millisecond();
    /* open sound device */
    result = rt_device_open(player->device, RT_DEVICE_OFLAG_WRONLY);
    if (result != RT_EOK)
    {
        LOG_E("open %s device faield", AMRPLAYER_PLAY_DEVICE);
        goto __exit;
    }
    LOG_I("open amrplayer, device %s", AMRPLAYER_PLAY_DEVICE);

    /* set sampletate,channels, samplebits */
    rt_device_control(player->device, AUDIO_CTL_CONFIGURE, &caps);

    return RT_EOK;

__exit:
    if (player->fp)
    {
        fclose(player->fp);
        player->fp = RT_NULL;
    }

    if (player->device)
    {
        rt_device_close(player->device);
        player->device = RT_NULL;
    }

    return result;
}

static void amrplayer_close(struct amrplayer *player)
{
    if (player->device)
    {
        rt_device_close(player->device);
        player->device = RT_NULL;
    }
    player->start = rt_tick_get_millisecond() - player->start;

    if (player->fp)
    {
        fclose(player->fp);
        player->fp = RT_NULL;
    }
    LOG_I("close amrplayer");
}

static int amrplayer_event_handler(struct amrplayer *player, int timeout)
{
    int event;
    rt_err_t result;
    struct play_msg msg;
#if (DBG_LEVEL >= DBG_LOG)
    rt_uint8_t last_state;
#endif

    result = rt_mq_recv(player->mq, &msg, sizeof(struct play_msg), timeout);
    if (result != RT_EOK)
    {
        event = PLAYER_EVENT_NONE;
        return event;
    }

#if (DBG_LEVEL >= DBG_LOG)
    last_state = player->state;
#endif

    switch (msg.type)
    {
    case MSG_START: // 开始
        event = PLAYER_EVENT_PLAY;
        player->state = PLAYER_STATE_PLAYING;
        break;

    case MSG_STOP: // 停止
        event = PLAYER_EVENT_STOP;
        player->state = PLAYER_STATE_STOPED;
        break;

    case MSG_PAUSE: // 暂停
        event = PLAYER_EVENT_PAUSE;
        player->state = PLAYER_STATE_PAUSED;
        break;

    case MSG_RESUME: // 恢复
        event = PLAYER_EVENT_RESUME;
        player->state = PLAYER_STATE_PLAYING;
        break;

    default:
        event = PLAYER_EVENT_NONE;
        break;
    }

    rt_completion_done(&player->ack);

#if (DBG_LEVEL >= DBG_LOG)
    LOG_I("EVENT:%s, STATE:%s -> %s", event_str[event], state_str[last_state], state_str[player->state]);
#endif

    return event;
}

static void amrplayer_entry(void *parameter)
{
    rt_err_t result = RT_EOK;
    rt_int32_t size;
    int event;
    unsigned char analysis[32];
    enum Mode dec_mode;
    int read_size;
    int ret = 0;
    int *destate = NULL;
    short block_size[16] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};
    int var;
    int i = 0;

    player.buffer = rt_malloc(WP_BUFFER_SIZE);
    if (player.buffer == RT_NULL)
        return;
    rt_memset(player.buffer, 0, WP_BUFFER_SIZE);

    player.mq = rt_mq_create("amr_p", sizeof(struct play_msg), 10, RT_IPC_FLAG_FIFO);
    if (player.mq == RT_NULL)
        goto __exit;

    player.lock = rt_mutex_create("amr_p", RT_IPC_FLAG_FIFO);
    if (player.lock == RT_NULL)
        goto __exit;

    player.volume = WP_VOLUME_DEFAULT;
    /* set volume */
    amrplayer_volume_set(player.volume);

    while (1)
    {
        /* wait play event forever */
        event = amrplayer_event_handler(&player, RT_WAITING_FOREVER);
        if (event != PLAYER_EVENT_PLAY)
            continue;

        /* open amrplayer */
        result = amrplayer_open(&player);
        if (result != RT_EOK)
        {
            player.state = PLAYER_STATE_STOPED;
            LOG_E("open amr player failed");
            continue;
        }

        // Decoder_Interface_init
        destate = Decoder_Interface_init();
        LOG_I("play start, device %s, uri=%s", AMRPLAYER_PLAY_DEVICE, player.uri);
        while (1)
        {
            event = amrplayer_event_handler(&player, RT_WAITING_NO);
            switch (event)
            {
            case PLAYER_EVENT_NONE:
            {
                // clear analysis
                memset(analysis, 0, sizeof(analysis));
                // read the first byte
                size = fread(analysis, sizeof(unsigned char), 1, player.fp);
                if (size <= 0)
                {
                    LOG_I("end of file on input.\n");
                    player.state = PLAYER_STATE_STOPED;
                    break;
                }
                dec_mode = (analysis[0] >> 3) & 0x000F; // get the decode mode
                read_size = block_size[dec_mode];       // get the size of the frame
                ret = fread(&analysis[1], sizeof(char), read_size, player.fp);
                if (ret < 0)
                {
                    LOG_E("fread error. \n");
                }

                // clear buffer
                memset(player.buffer, 0, WP_BUFFER_SIZE);

                /* call decoder */
                Decoder_Interface_Decode(destate, analysis, player.buffer, 0);

                for (i = WP_BUFFER_SIZE / 2 - 2; i >= 0; i = i - 2)
                {
                    player.buffer[i * 2 + 0] = player.buffer[i + 0];
                    player.buffer[i * 2 + 2] = player.buffer[i + 0];
                    player.buffer[i * 2 + 1] = player.buffer[i + 1];
                    player.buffer[i * 2 + 3] = player.buffer[i + 1];
                }

                /*witte data to sound device*/
                ret = rt_device_write(player.device, 0, player.buffer, WP_BUFFER_SIZE);
                if (ret != WP_BUFFER_SIZE)
                {
                    LOG_E("rt_device_write error: %d\n", ret);
                }
                break;
            }
            case PLAYER_EVENT_PAUSE:
            {
                /* wait resume or stop event forever */
                event = amrplayer_event_handler(&player, RT_WAITING_FOREVER);
                LOG_I("play pause");
            }
            default:
                break;
            }
            if (player.state == PLAYER_STATE_STOPED)
                break;
        }

        /* close amrplayer */
        amrplayer_close(&player);
        LOG_I("play end, use %d.%03d s", player.start / 1000, player.start % 1000);
    }

__exit:
    if (player.buffer)
    {
        rt_free(player.buffer);
        player.buffer = RT_NULL;
    }

    if (player.mq)
    {
        rt_mq_delete(player.mq);
        player.mq = RT_NULL;
    }

    if (player.lock)
    {
        rt_mutex_delete(player.lock);
        player.lock = RT_NULL;
    }
}

int amrplayer_init(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("amr_p",
                           amrplayer_entry,
                           RT_NULL,
                           WP_THREAD_STATCK_SIZE * 2,
                           WP_THREAD_PRIORITY, 10);
    if (tid)
        rt_thread_startup(tid);

    return RT_EOK;
}

INIT_APP_EXPORT(amrplayer_init);
