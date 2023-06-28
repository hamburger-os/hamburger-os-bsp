/*******************************************************
 *
 * @FileName: record.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 日志模块的实现
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include <file_manager.h>
#include <sys/stat.h>
#include "data.h"
#include "interf_enc.h"
#include "interf_dec.h"
#include "sp_dec.h"
#include "typedef.h"
#include "tax.h"
#include "utils.h"
#include "file_manager.h"
#include "pcm.h"
#include "amr.h"
#include "delay.h"
#include "voice.h"
#include "type.h"
#include "log.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* pcm缓存文件开关, 用于调试原始数据. */
#define DEBUG_PCM_FILE 0

/* 录音控制消息命令 */
#define REC_CTRL_MSG_INVALID 0
#define REC_CTRL_MSG_START 1
#define REC_CTRL_MSG_STOP 2

/* 录音线程向录音处理线程发送的命令 */
#define REC_MQ_NONE 0
#define REC_MQ_BEGIN 1
#define REC_MQ_END 2
#define REC_MQ_DATA 3

/*******************************************************
 * 全局变量
 *******************************************************/
typedef struct
{
    sint32_t cmd;      /* 录音命令 */
    sint32_t buf_size; /*缓冲区实际数据的大小*/
    /*缓冲区*/
    uint8_t buf[PCM_READ_BUFFER_SIZE / 2];
} rec_msg_t;

/*******************************************************
 * 全局变量
 *******************************************************/

/* 录音消息队列, 录音线程发送给录音处理线程, 包括开始录音处理线程/结束录音处理线程/数据等命令 */
static rt_mq_t rec_mq = RT_NULL;
/* 录音控制消息队列, 用于按键操作录音线程的启动/结束 */
static rt_mq_t rec_ctrl_mq = RT_NULL;

/*******************************************************
 * 函数声明
 *******************************************************/

/* 开始录音 */
static void record_beginrec(void);
/* 结束录音 */
static void record_endrec(void);
/* 录音线程 */
static void *record_thread(void *args);
/* 录音处理线程 */
static void *record_handler_thread(void *arg);

/*******************************************************
 *
 * @brief  开始录音
 *
 * @retval none
 *
 *******************************************************/
static void record_beginrec(void)
{
    char full_path[PATH_NAME_MAX_LEN];
    sint32_t ret = 0;

    /**
     * 1. 先检查是否需要重新生成新文件(0.新生成文件;1.追加语音)
     * 2. 如果是追加语音, 则以追加文件的方式打开文件, 写入语音头, 设置"正在录音标志".
     * 3. 如果是生成新文件, 则以建立文件的方式打开文件, 写入文件头, 再写入语音头,
     * 设置"正在录音标志". 更新latest_filename.conf文件.
     */

    g_cur_rec_file_info.channel = 1; /* 得到通道号 */
    if (fm_is_new() == 0)            /* 判断是否需要形成新的文件 */
    {
        /* 生成新文件 */
        sprintf(full_path, "%s/%s", YUYIN_PATH_NAME, g_cur_rec_file_info.filename);
        log_print(LOG_INFO, "create a new file '%s'. \n", full_path);
        ret = create_file(full_path);
        if (ret < 0)
        {
            log_print(LOG_ERROR, "create file error, ret=%d\n", ret);
        }
        else
        {
        }

        /* 打开文件 */
        g_cur_rec_file_info.fd = open(full_path, (int)((uint32_t)O_CREAT | (uint32_t)O_RDWR | (uint32_t)O_TRUNC), (uint32_t)F_MODE);
        if (g_cur_rec_file_info.fd < 0)
        {
            log_print(LOG_ERROR, "open file error, ret=%d\n", g_cur_rec_file_info.fd);
        }
        else
        {
        }

        if (g_cur_rec_file_info.fd > 0)
        {
            g_cur_rec_file_info.voices_num = (uint32_t)0;     /* 语音条数归零 */
            g_cur_rec_file_info.voice_index = (uint32_t)0;    /* 语音序号归零 */
            g_cur_rec_file_info.record_datalen = (uint32_t)0; /* 语音有效长度归零 */

            fm_write_file_head(g_cur_rec_file_info.fd); /* 写文件头 */
            g_cur_rec_file_info.new_voice_head_offset =
                lseek(g_cur_rec_file_info.fd, (off_t)0, SEEK_CUR); /* 得到当前文件偏移 */
            fm_write_voice_head(g_cur_rec_file_info.fd);           /* 写语音头 */

            log_print(LOG_INFO, "create a new file '%s', start record ... \n", full_path);
            /* 把g_cur_rec_file_info.filename写入latest_filename.conf文件中 */
            fm_write_name((char *)g_cur_rec_file_info.filename); /* 更新latest_filename.conf文件 */
        }
        else
        {
            log_print(LOG_ERROR, "error, can not create file: %s. \n", full_path);
        }
    }
    else /* 不需要生成新文件 */
    {
        /* 追加语言到文件末尾 */
        sprintf(full_path, "%s/%s", YUYIN_PATH_NAME, g_cur_rec_file_info.filename);
        log_print(LOG_INFO, "append data to file '%s'. ", full_path);
        g_cur_rec_file_info.fd = open(full_path, O_RDWR); /* 此处不能以APPEND方式打开,否则不能改写前面的内容 */

        if (g_cur_rec_file_info.fd > 0)
        {
            g_cur_rec_file_info.record_datalen = (uint32_t)0; /* 语音有效长度归零 */
            /* 先把文件指针移到最后 */
            g_cur_rec_file_info.new_voice_head_offset =
                lseek(g_cur_rec_file_info.fd, (off_t)0, SEEK_END);
            fm_write_voice_head(g_cur_rec_file_info.fd); /* 写入语音头 */
            log_print(LOG_INFO, "offset: 0x%x. \n", full_path, g_cur_rec_file_info.new_voice_head_offset);

            /* 通知tax箱录音开始 */
            tax_send_echo_event((uint8_t)IN_BEGIN, &g_tax40);
        }
        else
        {
            log_print(LOG_ERROR, "error, can not open file '%s'. \n", full_path);
        }
    }
}

/*******************************************************
 *
 * @brief  结束录音
 *
 * @retval none
 *
 *******************************************************/

static void record_endrec(void)
{
    uint32_t fill_len = 0;
    char fill_buf[PAGE_SIZE];

    memset(fill_buf, 0xFF, sizeof(fill_buf));
    if (true)
    {
        /* 如果当前正在录音, 则结束录音 */
        if (g_cur_rec_file_info.fd > 0)
        {
            /* 按512字节对其填充文件 */
            fill_len = g_cur_rec_file_info.record_datalen % (uint32_t)PAGE_SIZE;
            if (fill_len != (uint32_t)0)
            {
                /* 需要填充FF */
                fill_len = (uint32_t)PAGE_SIZE - fill_len;
                write(g_cur_rec_file_info.fd, fill_buf, fill_len);
                fsync(g_cur_rec_file_info.fd);
            }
            else
            {
            }
            g_cur_rec_file_info.voices_num++;

            pthread_mutex_lock(&g_mutex_voice_file);
            fm_modify_voice_head(g_cur_rec_file_info.fd); /* 修改语音头 */
            pthread_mutex_unlock(&g_mutex_voice_file);

            fm_modify_file_head(g_cur_rec_file_info.fd); /* 修改文件头 */

            /* 录音结束,关闭文件 */
            fsync(g_cur_rec_file_info.fd);
            close(g_cur_rec_file_info.fd);
            g_cur_rec_file_info.fd = -1;

            /* 这里不太对, 每帧语音数据20ms, 但是编码出来的长度不一定都为 27字节(VOICE_VALID_LENGTH) */
            log_print(LOG_INFO, "record end, total time: %ds. \n",
                      ((g_cur_rec_file_info.record_datalen / (uint32_t)VOICE_VALID_LENGTH) * (uint32_t)20) / (uint32_t)1000);
        }
        else
        {
        }

        /* 通知tax箱录音结束 */
        tax_send_echo_event((uint8_t)IN_END, &g_tax40);

        /* 释放存储空间 */
        free_space();
    }
    else
    {
    }
}
/*******************************************************
 *
 * @brief  录音线程
 *
 * @param  *args: 线程参数
 * @retval none
 *
 *******************************************************/
static void *record_thread(void *args)
{
    rt_err_t ret = 0;
    sint32_t i = 0;
    pcm_config_t *p_config = NULL;
    sint32_t rec_ctrl_msg = 0;
    rec_msg_t rec_msg_data;

    /* 获取声卡配置信息 */
    p_config = pcm_get_config_instance();

    if ((rec_mq == RT_NULL) || (rec_ctrl_mq == RT_NULL))
    {
        log_print(LOG_INFO, "record thread start error.\n");
        return NULL;
    }
    else
    {
    }

    log_print(LOG_INFO, "record thread start ok\n");
    while (true)
    {
        /* 接收录音控制消息 */
        ret = rt_mq_recv(rec_ctrl_mq,
                         (void *)&rec_ctrl_msg,
                         (rt_size_t)sizeof(rec_ctrl_msg),
                         (rt_int32_t)RT_WAITING_FOREVER);
        if (ret != RT_EOK)
        {
            continue;
        }
        else
        {
        }

        if (rec_ctrl_msg == REC_CTRL_MSG_START)
        {
            /* 立即结束掉放音动作 */
            while (data_get_pcm_state() != PCM_STATE_IDLE)
            {
                play_stop();
                msleep((uint32_t)30);
            }

            /* 设置为录音状态 */
            data_set_pcm_state(PCM_STATE_RECORDING);

            /* 设置PCM设备参数 */
            p_config->mode = SOUND_MODE_CAPTURE;
            p_config->channels = CHANNEL_NUM;
            p_config->sample_rate = SMAPLE_RATE;
            p_config->format = SMAPLE_BITS;

            /* pcm设备初始化 */
            ret = pcm_init(p_config);
            if (ret < 0)
            {
                log_print(LOG_ERROR, "init pcm device error, error code is %d. \n", ret);
                continue;
            }
            else
            {
            }
#if DEBUG_PCM_FILE
            /* 如果文件存在,则删除文件 */
            if (access(BUFFER_DIR, 0) == 0)
            {
                unlink(BUFFER_DIR); /* 删除文件 */
            }
            else
            {
            }

            /* 创建文件 */
            ret = create_file(BUFFER_DIR);
            if (ret < 0)
            {
                log_print(LOG_ERROR, "create_dir error, error code is %d. \n", ret);
            }
            else
            {
            }

            /* 打开缓存文件 */
            sint32_t fd = open(BUFFER_DIR, O_RDWR);
            if (fd < 0)
            {
                log_print(LOG_ERROR, "open error! fd=%d src='%s' \n", fd, BUFFER_DIR);
                continue;
            }
            else
            {
            }
#else
            /* 继续运行录音处理线程 */
            rec_msg_data.cmd = REC_MQ_BEGIN;
            rec_msg_data.buf_size = 0;
            ret = rt_mq_send(rec_mq, &rec_msg_data, sizeof(rec_msg_data));
            if (ret != RT_EOK)
            {
                /* todo, 是否合理? */
                pcm_exit(p_config);
                continue;
            }
            else
            {
            }
#endif
            while (true)
            {
                /* 读取音频数据 */
                memset(p_config->p_buffer, 0, p_config->size);
                ret = pcm_read(p_config->dev, p_config->p_buffer, p_config->size);
                if (ret < 0)
                {
                    continue;
                }
                else
                {
                }
                /**
                 *  样本数据图示:
                 * +----------+-----------+----------+-----------+
                 * |  byte0   |  byte1    |  byte2   |  byte3    |
                 * +----------+-----------+----------+-----------+
                 * |     left channel     |    right  channel    |
                 * +----------+-----------+----------+-----------+
                 * | low byte | high byte | low byte | high byte |
                 * +----------+-----------+----------+-----------+
                 */
                /* 将右通道的值变为单通道 */
                for (i = 0; i < (ret / 2 - 1); i = i + 2)
                {
                    // p_config->p_buffer[i + 0] = p_config->p_buffer[i * 2 + 2];
                    // p_config->p_buffer[i + 1] = p_config->p_buffer[i * 2 + 3];

                    p_config->p_buffer[i + 0] = p_config->p_buffer[i * 2 + 0];
                    p_config->p_buffer[i + 1] = p_config->p_buffer[i * 2 + 1];
                }
#if DEBUG_PCM_FILE
                /* 写入文件缓存文件中 */
                ret = write(fd, p_config->buffer, ret / 2);
                if (ret < 0)
                {
                    log_print(LOG_ERROR, "write error. \n");
                }
                else
                {
                }
#endif
                /* 通知录音处理线程录音结束 */
                rec_msg_data.cmd = REC_MQ_DATA;
                /* 设置缓冲区大小 */
                rec_msg_data.buf_size = ret / 2;
                memset(rec_msg_data.buf, 0, p_config->size / 2);
                memcpy(rec_msg_data.buf, p_config->p_buffer, ret / 2);

                ret = rt_mq_send(rec_mq, &rec_msg_data, sizeof(rec_msg_data));
                if (ret != RT_EOK)
                {
                    /* rt_kprintf("*"); */
                }
                else
                {
                }
                /* 接收录音控制消息, 采用非阻塞方式. */
                ret = rt_mq_recv(rec_ctrl_mq, (void *)&rec_ctrl_msg, sizeof(sint32_t), (rt_int32_t)RT_WAITING_NO);
                if (ret != RT_EOK)
                {
                    continue;
                }
                else
                {
                }
                if (rec_ctrl_msg == REC_CTRL_MSG_STOP)
                {
                    break; /* 结束录音 */
                }
                else
                {
                }
            }
#if DEBUG_PCM_FILE
            /* 关闭缓存文件 */
            if (close(fd) < 0)
            {
                rt_kprintf("write record buffer error. \n");
            }
#endif

            /* 通知录音处理线程录音结束 */
            rec_msg_data.cmd = REC_MQ_END;
            rec_msg_data.buf_size = 0;
            ret = rt_mq_send(rec_mq, &rec_msg_data, sizeof(rec_msg_data));
            if (ret != RT_EOK)
            {
                rt_kprintf("send end msg error, ret:%d. \n", ret);
            }
            else
            {
            }

            /* pcm设备初始化 */
            ret = pcm_exit(p_config);
            if (ret < 0)
            {
                rt_kprintf("pcm exit error.\n", ret);
            }
            else
            {
            }
        }
        else
        {
        }
    }
    return NULL;
}
/*******************************************************
 *
 * @brief  录音处理线程
 *
 * @param  *arg: 线程输入参数
 * @retval void * 返回数据
 *
 *******************************************************/
static void *record_handler_thread(void *arg)
{
    sint32_t ret = 0;
    sint32_t *enstate;          /* 编码器状态指针 */
    sint32_t byte_counter;      /* 编码后字节数 */
    enum Mode req_mode = MR102; /* 编码模式 */
    uint8_t serial_data[AMR_FRAME_MAX_LEN];
    sint32_t dtx = 1;
    char encoder_buffer[AMR_FRAME_SRC_DATA_MAX_LEN]; /* 一帧的数据. */
    rec_msg_t rec_msg_data;
    sint32_t encoder_buffer_used = 0; /* 编码缓存区中数据的大小 */
    sint32_t src_buf_handled_len;     /* 已经处理的数据长度 */

    log_print(LOG_INFO, "record handler thread start ok\n");
    while (true)
    {
        /* 接收处理消息 */
        ret = rt_mq_recv(rec_mq, (void *)&rec_msg_data, sizeof(rec_msg_data), (rt_int32_t)RT_WAITING_FOREVER);
        if (ret != RT_EOK)
        {
            continue;
        }
        else
        {
        }
        /* 接收到的不是录音线程发送的开始命令, 则重新等待. */
        if (rec_msg_data.cmd != REC_MQ_BEGIN)
        {
            continue;
        }
        else
        {
        }

        /* 显示当前状态 */
        event_push_queue(EVENT_RECORD_START);

        /* 录音前的准备工作 */
        record_beginrec();
        encoder_buffer_used = 0;

        /* 初始化编码器 */
        enstate = Encoder_Interface_init(dtx);
        while (true)
        {
            /* 增加1s超时, 防止录音线程卡死, 不发送结束命令 */
            ret = rt_mq_recv(rec_mq, (void *)&rec_msg_data, sizeof(rec_msg_data), (rt_int32_t)1000);
            if (ret != RT_EOK)
            {
                break;
            }
            else
            {
            }

            if (rec_msg_data.cmd == REC_MQ_END) /* 接收结束命令 */
            {
                break;
            }
            else
            {
            }

            if (rec_msg_data.cmd == REC_MQ_DATA) /* 接收到数据 */
            {
                /* rt_kprintf("."); */
                src_buf_handled_len = 0;
                while (true)
                {
                    if ((rec_msg_data.buf_size - src_buf_handled_len) < (AMR_FRAME_SRC_DATA_MAX_LEN - encoder_buffer_used))
                    {
                        /* 消息队列中的剩余数据不足以填满一帧, 拷贝剩余数据到缓冲区中. */
                        memcpy(&encoder_buffer[encoder_buffer_used],
                               &rec_msg_data.buf[src_buf_handled_len],
                               rec_msg_data.buf_size - src_buf_handled_len);
                        encoder_buffer_used = rec_msg_data.buf_size - src_buf_handled_len;
                        break;
                    }
                    else
                    {
                        /* 消息队列中的剩余数据可以填满一帧,拷贝剩余数据到缓冲区中. */
                        memcpy(&encoder_buffer[encoder_buffer_used],
                               &rec_msg_data.buf[src_buf_handled_len],
                               AMR_FRAME_SRC_DATA_MAX_LEN - encoder_buffer_used);
                        src_buf_handled_len += AMR_FRAME_SRC_DATA_MAX_LEN - encoder_buffer_used;

                        /* 获取满帧, 进行编码, 并存储数据 */
                        memset(serial_data, 0, AMR_FRAME_MAX_LEN);

                        /* rt_tick_t rt_tick = rt_tick_get_millisecond(); */
                        /* 调用AMR编码器 */
                        byte_counter = Encoder_Interface_Encode(enstate, req_mode, (short *)encoder_buffer, serial_data, 0);

#if 0
                        sint32_t dec_mode = (serial_data[0] >> 3) & 0x000F; /* 获取编码的模式 */
                        sint32_t read_size = g_block_size[dec_mode];        /* 根据编码模式获取当前需要读取数据的大小 */
                        rt_kprintf("serial_data len:%d mode:%d len :%d \ndata: ", byte_counter, dec_mode, read_size);
#endif
#if 0
                        sint32_t i = 0;
                        for (i = 0; i < 32; i++)
                        {
                            rt_kprintf("0x%02x ", serial_data[i]);
                        }
                        rt_kprintf("\n");
#endif
                        /* 写入VSW文件 */
                        ret = write(g_cur_rec_file_info.fd, serial_data, byte_counter);
                        if (ret > 0)
                        {
                            g_cur_rec_file_info.record_datalen += ret;
                        }
                        /* 此处不应该调用fsync, 会降低此线程的处理速度. */
                        /* fsync(g_cur_rec_file_info.fd); */
                        /* rt_kprintf("%d ms\n", rt_tick_get_millisecond() - rt_tick); */
                        encoder_buffer_used = 0;
                    }
                }
            }
            else
            {
            }
        }

        /* 关闭编码器 */
        Encoder_Interface_exit(enstate);
        /* 结束录音 */
        record_endrec();

        /* 显示当前状态 */
        event_push_queue(EVENT_RECORD_END);
        data_set_pcm_state(PCM_STATE_IDLE);
    }

    return NULL;
}

/*******************************************************
 *
 * @brief  开始录制语音
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/

sint32_t record_start_record(void)
{
    rt_err_t ret;
    sint32_t msg = REC_CTRL_MSG_START;

    if (data_get_pcm_state() == PCM_STATE_RECORDING)
    {
        return (sint32_t)-1;
    }
    else
    {
    }

    ret = rt_mq_send(rec_ctrl_mq, &msg, sizeof(sint32_t));
    if (ret != RT_EOK)
    {
        log_print(LOG_ERROR, "send record start mq error. \n");
    }
    else
    {
    }
    return 0;
}
/*******************************************************
 *
 * @brief  停止录制语音
 *
 * @retval 0:成功 -1:事变
 *
 *******************************************************/
sint32_t record_stop_record(void)
{
    rt_err_t ret;
    sint32_t msg = REC_CTRL_MSG_STOP;

    if (data_get_pcm_state() != PCM_STATE_RECORDING)
    {
        return (sint32_t)-1;
    }
    else
    {
    }
    ret = rt_mq_send(rec_ctrl_mq, &msg, sizeof(sint32_t));
    if (ret != RT_EOK)
    {
        log_print(LOG_ERROR, "send record stop mq error. \n");
    }
    else
    {
    }
    return 0;
}

/*******************************************************
 *
 * @brief  录音模块初始化
 *
 * @retval  =0:成功
 *          <0:失败
 *
 *******************************************************/

sint32_t record_init(void)
{
    sint32_t ret = 0;
    pthread_t reccord_tid;
    struct pthread_attr pthread_attr_data;

    /* 初始化消息队列 */
    rec_mq = rt_mq_create("rec_mq", sizeof(rec_msg_t), 8, RT_IPC_FLAG_FIFO);
    if (rec_mq == RT_NULL)
    {
        return (sint32_t)-1;
    }
    else
    {
    }
    rec_ctrl_mq = rt_mq_create("rec_ctrl_mq", sizeof(uint32_t), 8, RT_IPC_FLAG_FIFO);
    if (rec_ctrl_mq == RT_NULL)
    {

        return (sint32_t)-2;
    }
    else
    {
    }

    /* 初始化录音线程 */
    pthread_attr_init(&pthread_attr_data);
    pthread_attr_data.stacksize = 1024 * 10;
    pthread_attr_data.schedparam.sched_priority = 20;
    ret = pthread_create(&reccord_tid, &pthread_attr_data, (void *)record_thread, NULL);
    pthread_attr_destroy(&pthread_attr_data);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "pthread_create error\n ");
        return (sint32_t)-3;
    }
    else
    {
    }

    /* 初始化录音处理线程 */
    pthread_attr_init(&pthread_attr_data);
    pthread_attr_data.stacksize = 1024 * 30;
    pthread_attr_data.schedparam.sched_priority = 21;
    ret = pthread_create(&reccord_tid, &pthread_attr_data, (void *)record_handler_thread, NULL);
    pthread_attr_destroy(&pthread_attr_data);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "pthread_create error\n ");
        return (sint32_t)-4;
    }
    else
    {
    }
    return 0;
}
