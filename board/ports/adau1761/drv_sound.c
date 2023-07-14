/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2019-07-28     Ernest         the first version
 */

#include "board.h"

#include "adau17x1.h"
#include "drv_sound.h"
#include "drv_i2s.h"

#define DBG_TAG              "drv.sound"
#define DBG_LVL              DBG_LOG
#include <rtdbg.h>

#define CODEC_I2C_NAME  ADAU1761_I2C_DEV

#define TX_DMA_FIFO_SIZE (2048)
static uint8_t tx_fifo[TX_DMA_FIFO_SIZE];

#define RX_DMA_FIFO_SIZE (2048)
static uint8_t rx_fifo[RX_DMA_FIFO_SIZE];

struct stm32_audio
{
    struct rt_i2c_bus_device *i2c_bus;
    struct rt_audio_device audio;
    struct rt_audio_configure config;

    int replay_volume;
    uint8_t *tx_fifo;
    uint8_t *rx_fifo;
};
static struct stm32_audio _stm32_audio_play = {0};

static rt_err_t stm32_player_getcaps(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
    rt_err_t result = RT_EOK;
    struct stm32_audio *st_audio = (struct stm32_audio *)audio->parent.user_data;

    LOG_D("getcaps: main_type: %d, sub_type: %d", caps->main_type, caps->sub_type);

    switch (caps->main_type)
    {
    case AUDIO_TYPE_QUERY: /* query the types of hw_codec device */
    {
        switch (caps->sub_type)
        {
        case AUDIO_TYPE_QUERY:
            caps->udata.mask = AUDIO_TYPE_OUTPUT | AUDIO_TYPE_MIXER;
            break;

        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    case AUDIO_TYPE_OUTPUT: /* Provide capabilities of OUTPUT unit */
    {
        switch (caps->sub_type)
        {
        case AUDIO_DSP_PARAM:
            caps->udata.config.channels     = st_audio->config.channels;
            caps->udata.config.samplebits   = st_audio->config.samplebits;
            caps->udata.config.samplerate   = st_audio->config.samplerate;
            break;

        case AUDIO_DSP_SAMPLERATE:
            caps->udata.config.samplerate   = st_audio->config.samplerate;
            break;

        case AUDIO_DSP_CHANNELS:
            caps->udata.config.channels     = st_audio->config.channels;
            break;

        case AUDIO_DSP_SAMPLEBITS:
            caps->udata.config.samplebits     = st_audio->config.samplebits;
            break;

        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    case AUDIO_TYPE_INPUT: /* Provide capabilities of INTPUT unit */
    {
        switch (caps->sub_type)
        {
        case AUDIO_DSP_PARAM:
            caps->udata.config.channels     = st_audio->config.channels;
            caps->udata.config.samplebits   = st_audio->config.samplebits;
            caps->udata.config.samplerate   = st_audio->config.samplerate;
            break;

        case AUDIO_DSP_SAMPLERATE:
            caps->udata.config.samplerate   = st_audio->config.samplerate;
            break;

        case AUDIO_DSP_CHANNELS:
            caps->udata.config.channels     = st_audio->config.channels;
            break;

        case AUDIO_DSP_SAMPLEBITS:
            caps->udata.config.samplebits   = st_audio->config.samplebits;
            break;
        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    case AUDIO_TYPE_MIXER: /* report the Mixer Units */
    {
        switch (caps->sub_type)
        {
        case AUDIO_MIXER_QUERY:
            caps->udata.mask = AUDIO_MIXER_VOLUME | AUDIO_MIXER_LINE;
            break;

        case AUDIO_MIXER_VOLUME:
            caps->udata.value = st_audio->replay_volume;
            break;

        case AUDIO_MIXER_LINE:
            break;

        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    default:
        result = -RT_ERROR;
        break;
    }

    return result;
}

static rt_err_t  stm32_player_configure(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
    rt_err_t result = RT_EOK;
    struct stm32_audio *st_audio = (struct stm32_audio *)audio->parent.user_data;

    LOG_D("configure: main_type: %d, sub_type: %d", caps->main_type, caps->sub_type);

    switch (caps->main_type)
    {
    case AUDIO_TYPE_MIXER:
    {
        switch (caps->sub_type)
        {
        case AUDIO_MIXER_MUTE:
        {
            /* set mute mode */
            adau1761_set_volume(0);
            LOG_D("AUDIO_MIXER_MUTE");
            break;
        }

        case AUDIO_MIXER_VOLUME:
        {
            int volume = caps->udata.value;

            st_audio->replay_volume = volume;
            /* set mixer volume */
            adau1761_set_volume(volume);
            LOG_D("AUDIO_MIXER_VOLUME %d", volume);
            break;
        }

        default:
            result = -RT_ERROR;
            break;
        }

        break;
    }

    case AUDIO_TYPE_OUTPUT:
    {
        switch (caps->sub_type)
        {
        case AUDIO_DSP_PARAM:
        {
            struct rt_audio_configure config = caps->udata.config;

//            if (st_audio->config.samplerate != config.samplerate ||
//                    st_audio->config.samplebits != config.samplebits ||
//                    st_audio->config.channels != config.channels)
            {
                st_audio->config.samplerate = config.samplerate;
                st_audio->config.samplebits = config.samplebits;
                st_audio->config.channels = config.channels;

                adau1761_set_sampling_rate(config.samplerate);
                I2S_Samplerate_Set(config.samplerate);
                I2S_Samplebits_Set(config.samplebits);
                I2S_Channels_Set(config.channels);
                LOG_D("AUDIO_DSP_PARAM %d %d %d", config.samplerate, config.channels, config.samplebits);
            }
            break;
        }

        case AUDIO_DSP_SAMPLERATE:
        {
//            if (st_audio->config.samplerate != caps->udata.config.samplerate)
            {
                st_audio->config.samplerate = caps->udata.config.samplerate;
                adau1761_set_sampling_rate(caps->udata.config.samplerate);
                I2S_Samplerate_Set(caps->udata.config.samplerate);
                LOG_D("AUDIO_DSP_SAMPLERATE %d", caps->udata.config.samplerate);
            }
            break;
        }

        case AUDIO_DSP_CHANNELS:
        {
//            if (st_audio->config.channels != caps->udata.config.channels)
            {
                st_audio->config.channels = caps->udata.config.channels;
                I2S_Channels_Set(caps->udata.config.channels);
                LOG_D("AUDIO_DSP_CHANNELS %d", caps->udata.config.channels);
            }
            break;
        }

        case AUDIO_DSP_SAMPLEBITS:
        {
//            if (st_audio->config.samplebits != caps->udata.config.samplebits)
            {
                st_audio->config.samplebits = caps->udata.config.samplebits;
                I2S_Samplebits_Set(caps->udata.config.samplebits);
                LOG_D("AUDIO_DSP_SAMPLEBITS %d", caps->udata.config.samplebits);
            }
            break;
        }

        default:
            result = -RT_ERROR;
            break;
        }
        break;
    }

    case AUDIO_TYPE_INPUT:
    {
        switch (caps->sub_type)
        {
        case AUDIO_DSP_PARAM:
        {
            struct rt_audio_configure config = caps->udata.config;

//            if (st_audio->config.samplerate != config.samplerate ||
//                    st_audio->config.samplebits != config.samplebits ||
//                    st_audio->config.channels != config.channels)
            {
                st_audio->config.samplerate = config.samplerate;
                st_audio->config.channels   = config.channels;
                st_audio->config.samplebits = config.samplebits;

                adau1761_set_sampling_rate(config.samplerate);
                I2S_Samplerate_Set(config.samplerate);
                I2S_Samplebits_Set(config.samplebits);
                I2S_Channels_Set(config.channels);
                LOG_D("AUDIO_DSP_PARAM %d %d %d", config.samplerate, config.channels, config.samplebits);
            }
            break;
        }

        case AUDIO_DSP_SAMPLERATE:
        {
//            if (st_audio->config.samplerate != caps->udata.config.samplerate)
            {
                st_audio->config.samplerate = caps->udata.config.samplerate;
                adau1761_set_sampling_rate(caps->udata.config.samplerate);
                I2S_Samplerate_Set(caps->udata.config.samplerate);
                LOG_D("AUDIO_DSP_SAMPLERATE %d", caps->udata.config.samplerate);
            }
            break;
        }
        case AUDIO_DSP_CHANNELS:
        {
//            if (st_audio->config.channels != caps->udata.config.channels)
            {
                st_audio->config.channels = caps->udata.config.channels;
                I2S_Channels_Set(caps->udata.config.samplerate);
                LOG_D("AUDIO_DSP_CHANNELS %d", caps->udata.config.channels);
            }
            break;
        }

        case AUDIO_DSP_SAMPLEBITS:
        {
//            if (st_audio->config.samplebits != caps->udata.config.samplebits)
            {
                st_audio->config.samplebits = caps->udata.config.samplebits;
                I2S_Samplebits_Set(caps->udata.config.samplebits);
                LOG_D("AUDIO_DSP_SAMPLEBITS %d", caps->udata.config.samplebits);
            }
            break;
        }

        default:
            result = -RT_ERROR;
            break;
        }
        /* start record */
        I2S_Rec_Start(_stm32_audio_play.rx_fifo, RX_DMA_FIFO_SIZE/2);
        break;
    }

    default:
        break;
    }

    return result;
}

static void player_TxCpltCallback(void)
{
    if (_stm32_audio_play.audio.replay->activated == RT_TRUE)
    {
        rt_audio_tx_complete(&_stm32_audio_play.audio);
    }
}

static void player_TxHalfCpltCallback(void)
{
    if (_stm32_audio_play.audio.replay->activated == RT_TRUE)
    {
        rt_audio_tx_complete(&_stm32_audio_play.audio);
    }
}

static void player_RxCpltCallback(void)
{
    if (_stm32_audio_play.audio.record->activated == RT_TRUE)
    {
        rt_audio_rx_done(&(_stm32_audio_play.audio), &_stm32_audio_play.rx_fifo[0], RX_DMA_FIFO_SIZE / 2);
    }
}

static void player_RxHalfCpltCallback(void)
{
    if (_stm32_audio_play.audio.record->activated == RT_TRUE)
    {
        rt_audio_rx_done(&(_stm32_audio_play.audio), &_stm32_audio_play.rx_fifo[RX_DMA_FIFO_SIZE / 2], RX_DMA_FIFO_SIZE / 2);
    }
}

static rt_err_t stm32_player_init(struct rt_audio_device *audio)
{
    rt_err_t result = RT_EOK;

    /* initialize adau1761 */
    _stm32_audio_play.i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(CODEC_I2C_NAME);
    if (_stm32_audio_play.i2c_bus == RT_NULL)
    {
        LOG_E("Find device %s error", CODEC_I2C_NAME);
        return -RT_ERROR;
    }

    MX_I2S_Init();
    I2S_TxHalfCpltCallback_Set(player_TxHalfCpltCallback);
    I2S_TxCpltCallback_Set(player_TxCpltCallback);
    I2S_RxHalfCpltCallback_Set(player_RxHalfCpltCallback);
    I2S_RxCpltCallback_Set(player_RxCpltCallback);

    result = adau1761_init(_stm32_audio_play.i2c_bus);
    if (result != RT_EOK)
    {
        LOG_E("initialize adau1761 failed");
        return -RT_ERROR;
    }
    else
    {
        LOG_I("initialize adau1761 succeed");
    }

    return RT_EOK;
}

static rt_err_t stm32_player_start(struct rt_audio_device *audio, int stream)
{
    if (stream == AUDIO_STREAM_REPLAY)
    {
        adau1761_player_start();
        I2S_Play_Start(_stm32_audio_play.tx_fifo, TX_DMA_FIFO_SIZE/2);
        LOG_D("start replay");
    }
    else if (stream == AUDIO_STREAM_RECORD)
    {
        LOG_D("start record");
    }

    return RT_EOK;
}

static rt_err_t stm32_player_stop(struct rt_audio_device *audio, int stream)
{
    if (stream == AUDIO_STREAM_REPLAY)
    {
        adau1761_player_stop();
        I2S_Play_Stop();
        LOG_D("stop replay");
    }
    else if (stream == AUDIO_STREAM_RECORD)
    {
        I2S_Rec_Stop();
        LOG_D("stop record");
    }

    return RT_EOK;
}

static void stm32_player_buffer_info(struct rt_audio_device *audio, struct rt_audio_buf_info *info)
{
    /**
     *               TX_FIFO
     * +----------------+----------------+
     * |     block1     |     block2     |
     * +----------------+----------------+
     *  \  block_size  /
     */
    info->buffer = _stm32_audio_play.tx_fifo;
    info->total_size = TX_DMA_FIFO_SIZE;
    info->block_size = TX_DMA_FIFO_SIZE / 2;
    info->block_count = 2;
}
static struct rt_audio_ops _p_audio_ops =
{
    .getcaps     = stm32_player_getcaps,
    .configure   = stm32_player_configure,
    .init        = stm32_player_init,
    .start       = stm32_player_start,
    .stop        = stm32_player_stop,
    .transmit    = NULL,
    .buffer_info = stm32_player_buffer_info,
};

static int rt_hw_sound_init(void)
{
    /* default */
    _stm32_audio_play.config.samplerate = 8000;
    _stm32_audio_play.config.channels = 2;
    _stm32_audio_play.config.samplebits = 16;

    /* player */
    rt_memset(tx_fifo, 0, TX_DMA_FIFO_SIZE);
    _stm32_audio_play.tx_fifo = tx_fifo;

    /* record */
    rt_memset(rx_fifo, 0, RX_DMA_FIFO_SIZE);
    _stm32_audio_play.rx_fifo = rx_fifo;

    /* register sound device */
    _stm32_audio_play.audio.ops = &_p_audio_ops;
    rt_audio_register(&_stm32_audio_play.audio, "sound0", RT_DEVICE_FLAG_RDWR, &_stm32_audio_play);

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_hw_sound_init);
