/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-10-24     myshow       the first version
 */
#include <rtthread.h>

#include "Hdlc7c7eFrame.h"

#define DBG_TAG "hdlc  "
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

static uint8_t Hdlc7c7eFrame_sum(uint8_t *buf, uint16_t len)
{
    uint8_t sum = 0;
    for (uint16_t i = 0;i < len ; i++)
    {
        sum += buf[i];
    }

    return sum;
}

static uint8_t Hdlc7c7eFrame_code(uint8_t *buf, uint16_t len)
{
    LOG_D("  code: %d", len);
    LOG_HEX("      code", 16, buf, len);
    for (uint16_t i = 1;i < len - 1; )
    {
        if (buf[i] == HDLC7c7eFRAME_START || buf[i] == HDLC7c7eFRAME_END || buf[i] == HDLC7c7eFRAME_REPLACE)
        {
            rt_memmove(&buf[i + 1], &buf[i + 0], len - i);
            buf[i + 1] = buf[i + 0] & HDLC7c7eFRAME_COVER;
            buf[i + 0] = HDLC7c7eFRAME_REPLACE;

            len ++;
            i += 2;
        }
        else
        {
            i++;
        }
    }
    return len;
}

static uint8_t Hdlc7c7eFrame_uncode(uint8_t *buf, uint16_t len)
{
    LOG_D("uncode: %d", len);
    LOG_HEX("    uncode", 16, buf, len);
    for (uint16_t i = 1;i < len - 1; )
    {
        if (buf[i] == HDLC7c7eFRAME_REPLACE)
        {
            buf[i + 0] = (buf[i + 0] & 0xf0) | (buf[i + 1] & 0x0f);
            rt_memmove(&buf[i + 1], &buf[i + 2], len - i - 2);

            len --;
            i ++;
        }
        else
        {
            i++;
        }
    }
    return len;
}

int8_t Hdlc7c7eFrame_init(Hdlc7c7eFrame *hdlc)
{
    hdlc->id_count = 0;
    hdlc->rx_len = 0;

    hdlc->tx_mutex = rt_mutex_create("tx_mutex", RT_IPC_FLAG_PRIO);
    if (hdlc->tx_mutex == RT_NULL)
    {
        LOG_E("mutex creat error!");
        return -RT_ENOMEM;
    }
    hdlc->rx_mutex = rt_mutex_create("rx_mutex", RT_IPC_FLAG_PRIO);
    if (hdlc->rx_mutex == RT_NULL)
    {
        LOG_E("mutex creat error!");
        return -RT_ENOMEM;
    }
    hdlc->rx_buffer = rt_malloc(hdlc->maxlen);
    if (hdlc->rx_buffer == RT_NULL)
    {
        LOG_E("buffer creat error!");
        return -RT_ENOMEM;
    }

    if (hdlc->init(hdlc) != RT_EOK)
    {
        LOG_E("init error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}
RTM_EXPORT(Hdlc7c7eFrame_init);

int8_t Hdlc7c7eFrame_close(Hdlc7c7eFrame *hdlc)
{
    rt_mutex_delete(hdlc->tx_mutex);
    rt_mutex_delete(hdlc->rx_mutex);
    rt_free(hdlc->rx_buffer);

    if (hdlc->close(hdlc) != RT_EOK)
    {
        LOG_E("close error!");
        return -RT_ERROR;
    }
    return RT_EOK;
}
RTM_EXPORT(Hdlc7c7eFrame_close);

uint16_t Hdlc7c7eFrame_type(Hdlc7c7eFrame *hdlc, uint8_t dst_addr, uint8_t type, const uint8_t *buffer, uint16_t size)
{
    rt_mutex_take(hdlc->tx_mutex, RT_WAITING_FOREVER);
    uint8_t framebuff[(size + sizeof(Hdlc7c7eFrameHead) + sizeof(Hdlc7c7eFrameEnd))*2];
    Hdlc7c7eFrameHead head = {
            .start = HDLC7c7eFRAME_START,
            .src_addr = hdlc->addr,
            .dst_addr = dst_addr,
            .len = sizeof(framebuff)/2 - 2,
            .type = type,
            .id = hdlc->id_count++,
    };
    rt_memcpy(framebuff, &head, sizeof(Hdlc7c7eFrameHead));
    rt_memcpy(&framebuff[sizeof(Hdlc7c7eFrameHead)], buffer, size);
    Hdlc7c7eFrameEnd end = {
            .sum = Hdlc7c7eFrame_sum(&framebuff[1], head.len - 1),
            .end = HDLC7c7eFRAME_END,
    };
    rt_memcpy(&framebuff[sizeof(Hdlc7c7eFrameHead) + size], &end, sizeof(Hdlc7c7eFrameEnd));

    size = Hdlc7c7eFrame_code(framebuff, sizeof(framebuff)/2);
    LOG_D("   cmd: %d", size);
    LOG_HEX("       cmd", 16, framebuff, size);

    if (hdlc->write(hdlc, framebuff, size) != size)
    {
        LOG_E("cmd error!");
        size = 0;
    }
    rt_mutex_release(hdlc->tx_mutex);
    return size;
}
RTM_EXPORT(Hdlc7c7eFrame_type);

uint16_t Hdlc7c7eFrame_ack(Hdlc7c7eFrame *hdlc, uint8_t dst_addr, Hdlc7c7eFrame_Ack ack, uint8_t id, uint8_t type, const uint8_t *buffer, uint16_t size)
{
    rt_mutex_take(hdlc->tx_mutex, RT_WAITING_FOREVER);
    uint8_t framebuff[(size + sizeof(Hdlc7c7eFrameHead) + sizeof(Hdlc7c7eFrameEnd))*2];
    Hdlc7c7eFrameHead head = {
            .start = HDLC7c7eFRAME_START,
            .src_addr = hdlc->addr,
            .dst_addr = dst_addr,
            .len = sizeof(framebuff)/2 - 2,
            .type = type | ack,
            .id = id,
    };
    rt_memcpy(framebuff, &head, sizeof(Hdlc7c7eFrameHead));
    rt_memcpy(&framebuff[sizeof(Hdlc7c7eFrameHead)], buffer, size);
    Hdlc7c7eFrameEnd end = {
            .sum = Hdlc7c7eFrame_sum(&framebuff[1], head.len - 1),
            .end = HDLC7c7eFRAME_END,
    };
    rt_memcpy(&framebuff[sizeof(Hdlc7c7eFrameHead) + size], &end, sizeof(Hdlc7c7eFrameEnd));

    size = Hdlc7c7eFrame_code(framebuff, sizeof(framebuff)/2);
    LOG_D("   ack: %d", size);
    LOG_HEX("       ack", 16, framebuff, size);

    if (hdlc->write(hdlc, framebuff, size) != size)
    {
        LOG_E("ack error!");
        size = 0;
    }
    rt_mutex_release(hdlc->tx_mutex);
    return size;
}
RTM_EXPORT(Hdlc7c7eFrame_ack);

uint16_t Hdlc7c7eFrame_get_buffer_len(Hdlc7c7eFrameHead *head)
{
    return head->len + 2 - sizeof(Hdlc7c7eFrameHead) - sizeof(Hdlc7c7eFrameEnd);
}
RTM_EXPORT(Hdlc7c7eFrame_get_buffer_len);

int8_t Hdlc7c7eFrame_accept(Hdlc7c7eFrame *hdlc, uint16_t size)
{
    rt_mutex_take(hdlc->rx_mutex, RT_WAITING_FOREVER);
    uint16_t nblk = ((size % hdlc->maxlen) > 0)?(size/hdlc->maxlen + 1):(size/hdlc->maxlen);
    LOG_D("  nblk: %d %d %d", nblk, size, hdlc->maxlen);
    while (nblk > 0)
    {
        uint16_t nsize = 0;
        if (nblk == 1)
        {
            nsize = (size % hdlc->maxlen == 0)?(hdlc->maxlen):(size % hdlc->maxlen);
        }
        else
        {
            nsize = hdlc->maxlen;
        }

        if (hdlc->rx_len + nsize > hdlc->maxlen)
        {
            LOG_E("Buffer size limit exceeded %d/%d", hdlc->rx_len + nsize, hdlc->maxlen);
            hdlc->rx_len = 0;
        }
        uint16_t rx_length = hdlc->read(hdlc, hdlc->rx_buffer + hdlc->rx_len, nsize);
        if (rx_length > 0)
        {
            LOG_D("accept: %d/%d", rx_length, nsize);
            LOG_HEX("    accept", 16, hdlc->rx_buffer + hdlc->rx_len, rx_length);
            hdlc->rx_len += rx_length;

            if (hdlc->rx_len >= sizeof(Hdlc7c7eFrameHead) + sizeof(Hdlc7c7eFrameEnd))
            {
                LOG_D("   get: %d", hdlc->rx_len);
                LOG_HEX("       get", 16, hdlc->rx_buffer, hdlc->rx_len);

                uint16_t mode = 0;
                int16_t indexS = -1;
                int16_t indexE = -1;
                uint16_t hdlcLength = 0;
                for (uint16_t n = 0; n < hdlc->rx_len; n++)
                {
                    uint8_t data8 = hdlc->rx_buffer[n];

                    Hdlc7c7eFrameHead *head = NULL;
                    Hdlc7c7eFrameEnd *end = NULL;
                    uint8_t *buffer = NULL;

                    if (data8 == HDLC7c7eFRAME_START)
                    {
                        mode = 1;
                        indexS = n;
                        LOG_D("indexS: %d", indexS);
                    }
                    else if (mode == 1 && data8 == HDLC7c7eFRAME_END)
                    {
                        mode = 0;
                        indexE = n;
                        LOG_D("indexE: %d", indexE);

                        hdlcLength = indexE + 1 - indexS;
                        if (hdlcLength >= sizeof(Hdlc7c7eFrameHead) + sizeof(Hdlc7c7eFrameEnd))
                        {
                            hdlcLength = Hdlc7c7eFrame_uncode(&hdlc->rx_buffer[indexS], hdlcLength);
                            LOG_D("buffer: %d", hdlcLength);
                            LOG_HEX("    buffer", 16, &hdlc->rx_buffer[indexS], hdlcLength);
                            head = (Hdlc7c7eFrameHead *)&hdlc->rx_buffer[indexS];
                            LOG_D("  head: %x %x %x %d %x %d", head->start, head->src_addr, head->dst_addr, head->len, head->type, head->id);
                            if (head->len == hdlcLength - 2)
                            {
                                buffer = &hdlc->rx_buffer[indexS + sizeof(Hdlc7c7eFrameHead)];
                                end = (Hdlc7c7eFrameEnd *)&hdlc->rx_buffer[indexS + hdlcLength - sizeof(Hdlc7c7eFrameEnd)];
                                LOG_D("   end: %x %x", end->sum, end->end);
                                uint8_t sum = Hdlc7c7eFrame_sum((uint8_t *)head + 1, head->len - 1);
                                if (end->sum == sum)
                                {
                                    if (head->len > HDLC7c7eFRAME_MAX_LEN)
                                    {
                                        LOG_E("Please set a longer maximum length %d/%d.", head->len, HDLC7c7eFRAME_MAX_LEN);
                                    }
                                    if (head->dst_addr == hdlc->addr)
                                    {
                                        hdlc->rx_indicate(hdlc, head, buffer, end);
                                    }
                                }
                                else
                                {
                                    LOG_D("   sum: error %d %d", end->sum, sum);
                                }
                            }
                            else
                            {
                                LOG_D("   len: error %d %d", head->len, hdlcLength - 2);
                            }
                        }
                    }
                }
                uint16_t lessindex = indexS;
                if (indexS == -1 && indexE == -1)
                {
                    lessindex = hdlc->rx_len;
                }
                else if (indexE != -1)
                {
                    lessindex = (indexE > indexS)?(indexE + 1):(indexS);
                }
                uint16_t lessLength = hdlc->rx_len - lessindex;
                rt_memcpy(hdlc->rx_buffer, &hdlc->rx_buffer[lessindex], lessLength);
                hdlc->rx_len = lessLength;
                LOG_D("  less: %d", hdlc->rx_len);
                LOG_HEX("      less", 16, hdlc->rx_buffer, hdlc->rx_len);
            }
            else
            {
                LOG_D(" rxlen: error %d %d", hdlc->rx_len, sizeof(Hdlc7c7eFrameHead) + sizeof(Hdlc7c7eFrameEnd));
            }
        }

        nblk -- ;
    }
    rt_mutex_release(hdlc->rx_mutex);

    return 0;
}
RTM_EXPORT(Hdlc7c7eFrame_accept);
