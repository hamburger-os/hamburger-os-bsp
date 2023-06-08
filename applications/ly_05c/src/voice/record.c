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
#include "voice.h"
#include "data.h"
#include "pcm.h"
#include "typedef.h"
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

/*******************************************************
 * 宏定义
 *******************************************************/

/*录音控制消息命令 */
#define REC_CTRL_MSG_INVALID 0
#define REC_CTRL_MSG_START 1
#define REC_CTRL_MSG_STOP 2

/* 录音线程向录音处理线程发送的命令 */
#define REC_MQ_NONE 0
#define REC_MQ_BEGIN 1
#define REC_MQ_END 2

/*******************************************************
 * 全局变量
 *******************************************************/

/* 录音消息队列 */
static rt_mq_t rec_mq = RT_NULL;
/* 录音控制消息队列 */
static rt_mq_t rec_ctrl_mq = RT_NULL;

/*******************************************************
 *
 * @brief  开始录音
 *
 * @param  channel: 通道数
 * @retval none
 *
 *******************************************************/
static void record_beginrec()
{
    char full_path[PATH_NAME_MAX_LEN];
    int ret = 0;

    /**
     * 1. 先检查是否需要重新生成新文件(0.新生成文件;1.追加语音)
     * 2. 如果是追加语音, 则以追加文件的方式打开文件, 写入语音头, 设置"正在录音标志".
     * 3. 如果是生成新文件, 则以建立文件的方式打开文件, 写入文件头, 再写入语音头, 设
     * 置"正在录音标志". 更新latest_filename.conf文件
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
        /*打开文件 */
        g_cur_rec_file_info.fd = open(full_path, O_CREAT | O_RDWR | O_TRUNC, F_MODE);
        if (g_cur_rec_file_info.fd < 0)
        {
            log_print(LOG_ERROR, "open file error, ret=%d\n", g_cur_rec_file_info.fd);
        }

        if (g_cur_rec_file_info.fd > 0)
        {
            g_cur_rec_file_info.voices_num = 0;     /* 语音条数归零 */
            g_cur_rec_file_info.voice_index = 0;    /* 语音序号归零 */
            g_cur_rec_file_info.record_datalen = 0; /* 语音有效长度归零 */

            fm_write_file_head(g_cur_rec_file_info.fd); /* 写文件头 */
            g_cur_rec_file_info.new_voice_head_offset =
                lseek(g_cur_rec_file_info.fd, 0, SEEK_CUR); /* 得到当前文件偏移 */
            fm_write_voice_head(g_cur_rec_file_info.fd);    /* 写语音头 */

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
        g_cur_rec_file_info.fd = open(full_path, O_RDWR); /*此处不能以APPEND方式打开,否则不能改写前面的内容 */

        if (g_cur_rec_file_info.fd > 0)
        {
            g_cur_rec_file_info.record_datalen = 0; /* 语音有效长度归零 */
            /* 先把文件指针移到最后 */
            g_cur_rec_file_info.new_voice_head_offset =
                lseek(g_cur_rec_file_info.fd, 0, SEEK_END);
            fm_write_voice_head(g_cur_rec_file_info.fd); /* 写入语音头 */
            log_print(LOG_INFO, "offset: 0x%x. \n", full_path, g_cur_rec_file_info.new_voice_head_offset);

            /* 此部分代码需要修改 */
            if (g_cur_rec_file_info.channel == 1)
            {
                tax_send_echo_event(IN_BEGIN, &g_tax40);
            }
            else
            {
                tax_send_echo_event(OUT_BEGIN, &g_tax40);
            }
        }
        else
        {
            log_print(LOG_ERROR, "error, can not open file '%s'. \n", full_path);
        }
    }
}

/*******************************************************
 *
 * @brief  结束放音
 *
 * @retval none
 *
 *******************************************************/

static void record_endrec(void)
{
    unsigned int fill_len = 0;
    char fill_buf[PAGE_SIZE] = {0};

    memset(fill_buf, 0xFF, sizeof(fill_buf));
    if (true)
    {
        /* 如果当前正在录音, 则结束录音 */
        if (g_cur_rec_file_info.fd > 0)
        {
            /* 按512字节对其填充文件 */
            fill_len = g_cur_rec_file_info.record_datalen % PAGE_SIZE;
            if (fill_len != 0)
            {
                /* 需要填充FF */
                fill_len = PAGE_SIZE - fill_len;
                write(g_cur_rec_file_info.fd, fill_buf, fill_len);
                fsync(g_cur_rec_file_info.fd);
            }
            g_cur_rec_file_info.voices_num++;

            pthread_mutex_lock(&g_mutex_voice_file);
            fm_modify_voice_head(g_cur_rec_file_info.fd); /* 修改语音头 */
            pthread_mutex_unlock(&g_mutex_voice_file);

            fm_modify_file_head(g_cur_rec_file_info.fd); /* 修改文件头 */

            fsync(g_cur_rec_file_info.fd);
            close(g_cur_rec_file_info.fd); /* 录音结束,关闭文件 */
            g_cur_rec_file_info.fd = -1;

            /* 这里不太对, 每帧语音数据20ms, 但是编码出来的长度不一定都为 27字节(VOICE_VALID_LENGTH) */
            log_print(LOG_INFO, "end, total time: %ds. \n",
                      ((g_cur_rec_file_info.record_datalen / VOICE_VALID_LENGTH) * 20) / 1000);
        }
        if (g_cur_rec_file_info.channel == 1)
        {
            tax_send_echo_event(IN_END, &g_tax40);
        }
        else
        {
            tax_send_echo_event(OUT_END, &g_tax40);
        }
        free_space();
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
    int ret = 0;
    int i = 0;
    pcm_config_t config;
    char buffer_filename[PATH_NAME_MAX_LEN];
    int rec_ctrl_msg = 0;
    int rec_msg = 0;

    if ((rec_mq == RT_NULL) | (rec_ctrl_mq == RT_NULL))
    {
        perror("record thread error.");
        return NULL;
    }

    log_print(LOG_INFO, "record thread start ...\n");
    while (true)
    {
        /*接收录音控制消息 */
        ret = rt_mq_recv(rec_ctrl_mq, &rec_ctrl_msg, sizeof(int), RT_WAITING_FOREVER);
        if (ret != RT_EOK)
            continue;

        if (rec_ctrl_msg == REC_CTRL_MSG_START)
        {
            /* 立即结束掉放音动作 */
            while (data_get_pcm_state() != PCM_STATE_IDLE)
            {
                play_stop();
                msleep(30);
            }

            /* 设置为录音状态 */
            data_set_pcm_state(PCM_STATE_RECORDING);

            /* 设置PCM设备参数 */
            config.mode = SOUND_MODE_CAPTURE;
            config.channels = CHANNEL_NUM;
            config.sample_rate = SMAPLE_RATE;
            config.format = SMAPLE_BITS;

            /* pcm设备初始化 */
            ret = pcm_init(&config);
            if (ret < 0)
            {
                log_print(LOG_ERROR, "pcm_init error, error code is %d. \n", ret);
                continue;
            }
            /* 如果文件存在,则删除文件 */
            if (access(BUFFER_DIR, 0) == 0)
            {
                unlink(BUFFER_DIR); /* 删除文件 */
            }

            /* 创建文件 */
            ret = create_file(BUFFER_DIR);
            if (ret < 0)
            {
                log_print(LOG_ERROR, "create_dir error, error code is %d. \n", ret);
            }

            /*打开缓存文件 */
            int fd = open(BUFFER_DIR, O_RDWR);
            if (fd < 0)
            {
                log_print(LOG_ERROR, "open error! fd=%d src='%s' \n", fd, buffer_filename);
                continue;
            }
            // 继续运行录音处理线程
            rec_msg = REC_MQ_BEGIN;
            rt_mq_send(rec_mq, &rec_msg, sizeof(int));
            if (ret != RT_EOK)
            {
                // todo, ???
                continue;
            }

            while (true)
            {
                /* 读取音频数据 */
                memset(config.buffer, 0, config.size);
                ret = pcm_read(config.dev, config.buffer, config.size);
                if (ret == -EPIPE)
                {
                    printf("overrun occurred. \n");
                    continue;
                }
                /**
                 * 样本数据图示:
                 * +----------+-----------+----------+-----------+
                 * |  byte0   |  byte1    |  byte2   |  byte3    |
                 * +----------+-----------+----------+-----------+
                 * |     left channel     |    right  channel    |
                 * +----------+-----------+----------+-----------+
                 * | low byte | high byte | low byte | high byte |
                 * +----------+-----------+----------+-----------+
                 */
                /* 将右通道的值放到左通道 */
                for (i = 0; i < ret / 2 - 1; i = i + 2)
                {
                    config.buffer[i + 0] = config.buffer[i * 2 + 2];
                    config.buffer[i + 1] = config.buffer[i * 2 + 3];
                }
                /* 写入文件缓存文件中 */
                ret = write(fd, config.buffer, ret / 2);
                if (ret < 0)
                {
                    log_print(LOG_ERROR, "write error. \n");
                }
                /* 接收录音控制消息, 采用非阻塞方式. */
                ret = rt_mq_recv(rec_ctrl_mq, &rec_ctrl_msg, sizeof(int), RT_WAITING_NO);
                if (ret != RT_EOK)
                {
                    continue;
                }
                if (rec_ctrl_msg == REC_CTRL_MSG_STOP)
                {
                    break; /* 结束录音 */
                }
            }

            /* 关闭缓存文件 */
            if (close(fd) < 0)
            {
                log_print(LOG_ERROR, "write error. \n");
            }

            // 通知录音处理线程录音结束
            rec_msg = REC_MQ_END;
            rt_mq_send(rec_mq, &rec_msg, sizeof(int));

            /* pcm设备初始化 */
            ret = pcm_exit(&config);
            if (ret < 0)
            {
                return NULL;
            }
        }
        msleep(30);
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
    int ret = 0;
    int *enstate;               /* 编码器状态指针 */
    int byte_counter;           /* 编码后字节数 */
    enum Mode req_mode = MR102; /* 编码模式 */
    unsigned char serial_data[AMR_FRAME_MAX_LEN];
    int dtx = 1, fd = 0;
    char buffer_filename[PATH_NAME_MAX_LEN];
    char encoder_buffer[AMR_FRAME_SRC_DATA_MAX_LEN]; /*一帧的数据. */
    uint32_t rec_msg;
    int encoder_buffer_used = 0;

    while (true)
    {
        /*接收处理消息 */
        ret = rt_mq_recv(rec_mq, &rec_msg, sizeof(uint32_t), RT_WAITING_FOREVER);
        if (ret != RT_EOK)
            continue;

        // 接收到的不是录音线程发送的开始命令, 则重新等待.
        if (rec_msg != REC_MQ_BEGIN)
            continue;

        /*打开缓存文件 */
        // sprintf(buffer_filename, "%s/%u", BUFFER_DIR, (unsigned int)msg);
        if ((fd = open(BUFFER_DIR, O_RDONLY)) < 0)
        {
            log_print(LOG_ERROR, "error, can not open file '%s'.\n", buffer_filename);
            continue;
        }

        /* 录音前的准备工作 */
        record_beginrec();

        /* 初始化编码器 */
        enstate = Encoder_Interface_init(dtx);
        while ((ret = read(fd, &encoder_buffer[encoder_buffer_used], (AMR_FRAME_SRC_DATA_MAX_LEN - encoder_buffer_used))) > 0)
        {
            if (ret == 0) // 读取到0个字节
            {
                ret = rt_mq_recv(rec_mq, &rec_msg, sizeof(int), RT_WAITING_NO);
                if ((ret == RT_EOK) && (REC_MQ_NONE == REC_MQ_END))
                {
                    // 录音结束.
                    break;
                }
                else
                {
                    // 还在录音, 但是缓存文件中数据读取完毕, 等待20ms.
                    msleep(20);
                }
            }
            else if (ret < 0) // 读取数据错误
            {
                // 读取缓存文件出现错误, 直接结束录音.
                break;
            }
            else // 读取到数据, 但未满一帧.
            {
                encoder_buffer_used += ret;
                if (encoder_buffer_used < AMR_FRAME_SRC_DATA_MAX_LEN)
                {
                    // 还在录音, 还未读满足一帧, 并且缓存文件中有数据, 则继续读取语音数据
                    continue;
                }
            }

            memset(serial_data, 0, AMR_FRAME_MAX_LEN);

            /* 调用AMR编码器 */
            byte_counter = Encoder_Interface_Encode(enstate, req_mode, (short *)encoder_buffer, serial_data, 0);
#if 0
            int dec_mode = (serial_data[0] >> 3) & 0x000F; /* 获取编码的模式 */
            int read_size = g_block_size[dec_mode];        /* 根据编码模式获取当前需要读取数据的大小 */
            printf( "serial_data len:%d mode:%d len :%d \ndata: ", byte_counter, dec_mode, read_size);
#endif
#if 0
            int i = 0;
            for (i = 0; i < 32; i++)
            {
                printf( "0x%02x ", serial_data[i]);
            }
            printf( "\n");
#endif
            /* 写入VSW文件 */
            ret = write(g_cur_rec_file_info.fd, serial_data, byte_counter);
            if (ret > 0)
            {
                g_cur_rec_file_info.record_datalen += ret;
            }
            fsync(g_cur_rec_file_info.fd);

            encoder_buffer_used = 0;
        }

        /* 关闭编码器 */
        Encoder_Interface_exit(enstate);
        /*结束录音 */
        record_endrec();
        /*关闭缓存文件 */
        close(fd);
        /*删除缓存文件 */
        unlink(buffer_filename);

        data_set_pcm_state(PCM_STATE_IDLE);
    }
    return NULL;
}

/*******************************************************
 *
 * @brief  开始录制语音
 *
 * @retval none
 *
 *******************************************************/

void record_start_record(void)
{
    rt_err_t ret;
    int msg = REC_CTRL_MSG_START;

    ret = rt_mq_send(rec_ctrl_mq, &msg, sizeof(int));
    if (ret != RT_EOK)
    {
        log_print(LOG_ERROR, "rt_mq_send error. \n");
    }
}
/*******************************************************
 *
 * @brief  停止录制语音
 *
 * @retval none
 *
 *******************************************************/
void record_stop_record(void)
{
    rt_err_t ret;
    int msg = REC_CTRL_MSG_STOP;

    ret = rt_mq_send(rec_ctrl_mq, &msg, sizeof(int));
    if (ret != RT_EOK)
    {
        log_print(LOG_ERROR, "rt_mq_send error. \n");
    }
}

/*******************************************************
 *
 * @brief  录音模块初始化
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/

int record_init(void)
{
    int ret = 0;
    pthread_t reccord_tid;
    struct pthread_attr pthread_attr_t;

    /*初始化消息队列 */
    rec_mq = rt_mq_create("rec_mq", sizeof(uint32_t), 256, RT_IPC_FLAG_FIFO);
    if (rec_mq == RT_NULL)
        return -1;
    rec_ctrl_mq = rt_mq_create("rec_ctrl_mq", sizeof(uint32_t), 256, RT_IPC_FLAG_FIFO);
    if (rec_ctrl_mq == RT_NULL)
        return -1;

    /* 初始化录音线程 */
    pthread_attr_init(&pthread_attr_t);
    pthread_attr_t.stacksize = 1024 * 10;
    pthread_attr_t.schedparam.sched_priority = 20;
    ret = pthread_create(&reccord_tid, &pthread_attr_t, (void *)record_thread, NULL);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "pthread_create error\n ");
        return -1;
    }

    /* 初始化录音处理线程 */
    pthread_attr_init(&pthread_attr_t);
    pthread_attr_t.stacksize = 1024 * 30;
    pthread_attr_t.schedparam.sched_priority = 21;
    ret = pthread_create(&reccord_tid, &pthread_attr_t, (void *)record_handler_thread, NULL);
    if (ret < 0)
    {
        log_print(LOG_ERROR, "pthread_create error\n ");
        return -1;
    }
    return 0;
}
