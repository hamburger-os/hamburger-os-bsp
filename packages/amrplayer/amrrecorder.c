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
#include <amrrecorder.h>
#include <sp_enc.h>

#define DBG_TAG "AMR_RECORDER"
#define DBG_LVL DBG_INFO

struct recorder
{
    rt_device_t device;
    struct amrrecord_info info;
    struct rt_event *event;
    struct rt_completion ack;
    rt_uint8_t *buffer;
    FILE *fp;
    rt_bool_t activated;
    uint32_t start;
};

enum RECORD_EVENT
{
    RECORD_EVENT_STOP = 0x01,
    RECORD_EVENT_START = 0x02,
};

#define WR_BUFFER_SIZE (320 * 2)

static struct recorder record;

static rt_err_t amrrecorder_open(struct recorder *record)
{
    rt_err_t result = RT_EOK;

    record->device = rt_device_find(AMRPLAYER_RECORD_DEVICE);
    if (record->device == RT_NULL)
    {
        LOG_E("device %s not find", AMRPLAYER_RECORD_DEVICE);
        return -RT_ERROR;
    }

    /* malloc internal buffer */
    record->buffer = rt_malloc(WR_BUFFER_SIZE);
    if (record->buffer == RT_NULL)
    {
        result = -RT_ENOMEM;
        LOG_E("malloc internal buffer for recorder failed");
        goto __exit;
    }
    rt_memset(record->buffer, 0, WR_BUFFER_SIZE);

    /* open file */
    record->fp = fopen(record->info.uri, "wb");
    if (record->fp == RT_NULL)
    {
        result = -RT_ERROR;
        LOG_E("open file %s failed", record->info.uri);
        goto __exit;
    }

    record->start = rt_tick_get_millisecond();
    /* open micphone device */
    result = rt_device_open(record->device, RT_DEVICE_OFLAG_RDONLY);
    if (result != RT_EOK)
    {
        result = -RT_ERROR;
        LOG_E("open %s device faield", AMRPLAYER_RECORD_DEVICE);
        goto __exit;
    }
    LOG_I("open %s device succeed", AMRPLAYER_RECORD_DEVICE);

    record->event = rt_event_create("amr_r", RT_IPC_FLAG_FIFO);
    if (record->event == RT_NULL)
    {
        result = -RT_ERROR;
        LOG_E("create event for amr recorder failed");
        goto __exit;
    }

    return RT_EOK;

__exit:
    if (record->buffer)
    {
        rt_free(record->buffer);
        record->buffer = RT_NULL;
    }

    if (record->fp)
    {
        fclose(record->fp);
        record->fp = RT_NULL;
    }

    if (record->device)
    {
        rt_device_close(record->device);
        record->device = RT_NULL;
    }

    if (record->event)
    {
        rt_event_delete(record->event);
        record->event = RT_NULL;
    }

    return result;
}

void amrrecorder_close(struct recorder *record)
{
    if (record->device)
    {
        rt_device_close(record->device);
        record->device = RT_NULL;
    }
    record->start = rt_tick_get_millisecond() - record->start;

    if (record->buffer)
    {
        rt_free(record->buffer);
        record->buffer = RT_NULL;
    }

    if (record->fp)
    {
        fclose(record->fp);
        record->fp = RT_NULL;
    }

    if (record->event)
    {
        rt_event_delete(record->event);
        record->event = RT_NULL;
    }
}

static void amrrecord_entry(void *parameter)
{
    rt_err_t result;
    rt_size_t size;
    struct rt_audio_caps caps;
    rt_uint32_t recv_evt, total_length = 0;
    unsigned char serial_data[32];
    /* pointer to encoder state structure */
    int *enstate;
    int dtx = 1;
    /* counters */
    int byte_counter, frames = 0, bytes = 0, i = 0;
    /* requested mode */
    enum Mode req_mode = MR102;

    result = amrrecorder_open(&record);
    if (result != RT_EOK)
    {
        LOG_E("open amr recorder failed. \n");
        return;
    }

    record.activated = RT_TRUE;

    /* set sampletate,channels, samplebits */
    caps.main_type = AUDIO_TYPE_INPUT;
    caps.sub_type = AUDIO_DSP_PARAM;
    caps.udata.config.samplerate = record.info.samplerate;
    caps.udata.config.channels = record.info.channels;
    caps.udata.config.samplebits = 16;
    rt_device_control(record.device, AUDIO_CTL_CONFIGURE, &caps);
    LOG_I("Information: samplerate %d, channels %d, samplebits %d", record.info.samplerate, record.info.channels, record.info.samplebits);
    LOG_I("record start, device %s, uri=%s", AMRPLAYER_RECORD_DEVICE, record.info.uri);

    // init encoder
    enstate = Encoder_Interface_init(dtx);

    /* write magic number to indicate single channel AMR file storage format */
    bytes = fwrite(AMR_MAGIC_NUMBER, sizeof(char), strlen(AMR_MAGIC_NUMBER), record.fp);
    while (1)
    {
        /* read raw data from sound device */
        size = rt_device_read(record.device, 0, record.buffer, WR_BUFFER_SIZE);

        for (i = 0; i < WR_BUFFER_SIZE / 2 - 1; i = i + 2)
        {
            record.buffer[i + 0] = record.buffer[i * 2 + 0];
            record.buffer[i + 1] = record.buffer[i * 2 + 1];
        }

        /* call encoder */
        size = Encoder_Interface_Encode(enstate, req_mode, record.buffer, serial_data, 0);
        if (size)
        {
            fwrite(serial_data, sizeof(char), size, record.fp);
            fflush(record.fp);
            total_length += size;
        }

        /* recive stop event */
        if (rt_event_recv(record.event, RECORD_EVENT_STOP,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_NO, &recv_evt) == RT_EOK)
        {
            /* close recorder */
            amrrecorder_close(&record);
            LOG_I("record end, length %d KB, use %d.%03d s", total_length / 1024, record.start / 1000, record.start % 1000);

            /* ack event */
            rt_completion_done(&record.ack);
            record.activated = RT_FALSE;

            break;
        }
    } // end while

    // destrory encoder
    Encoder_Interface_exit(enstate);

    return;
}

rt_err_t amrrecorder_start(struct amrrecord_info *info)
{
    if (record.activated != RT_TRUE)
    {
        rt_thread_t tid;

        if (record.info.uri)
            rt_free(record.info.uri);
        record.info.uri = rt_strdup(info->uri);

        record.info.samplerate = info->samplerate;
        record.info.channels = info->channels;
        record.info.samplebits = info->samplebits;

        tid = rt_thread_create("amr_r", amrrecord_entry, RT_NULL, 2048 * 200, 19, 20);
        if (tid)
            rt_thread_startup(tid);
    }

    return RT_EOK;
}

rt_err_t amrrecorder_stop(void)
{
    if (record.activated == RT_TRUE)
    {
        rt_completion_init(&record.ack);
        rt_event_send(record.event, RECORD_EVENT_STOP);
        rt_completion_wait(&record.ack, RT_WAITING_FOREVER);
    }

    return RT_EOK;
}
