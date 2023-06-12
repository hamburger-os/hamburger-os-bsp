/*******************************************************
 *
 * @FileName: play.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 放音模块的实现.
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

/*******************************************************
 * 头文件
 *******************************************************/

#include "voice.h"
#include "file_manager.h"
#include "data.h"
#include "typedef.h"
#include "interf_enc.h"
#include "interf_dec.h"
#include "sp_dec.h"
#include "pcm.h"
#include "amr.h"
#include "delay.h"
#include "log.h"
#include "utils.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* 缓存提示音的最大数量 */
#define PLAY_FILE_NUMS 16
/* 文件名的最大长度 */
#define PLAY_FILE_NAME_LEN 64
/* 标准amr文件头部的大小 */
#define AMR_FILE_HEADER_LEN 6

#define EVENT_VOICE_DIR "/mnt/emmc/yysj/voice"                   /* 提示音存放路径 */
#define EVENT_VOICE_DUMP_START_ALL "BeginAll.amr"                /* 开始转储全部文件 */
#define EVENT_VOICE_DUMP_START_LAST "BeginNew.amr"               /* 开始转储最新文件 */
#define EVENT_VOICE_DUMP_END_LAST "EndNew.amr"                   /* 最新文件转储成功 */
#define EVENT_VOICE_DUMP_END_ALL "EndAll.amr"                    /* 全部文件转储成功 */
#define EVENT_VOICE_DUMP_FAIL "StoreFail.amr"                    /* 转储失败 */
#define EVENT_VOICE_DUMP_USB_FULL "UdiskFull.amr"                /* U盘已满 */
#define EVENT_VOICE_DUMP_USB_ILLEGAL "UdiskIllegalStoreFail.amr" /* 未鉴权U盘,转储失败 */
#define EVENT_VOICE_UPDATE_BEGIN "BeginUpdateApp.amr"            /* 开始更新程序 */
#define EVENT_VOICE_UPDATE_SUCCESS "UpdateAppOK.amr"             /* 更新程序成功 */
#define EVENT_VOICE_UPDATE_FAIL "UpdateAppERR.amr"               /* 更新程序失败 */
#define EVENT_VOICE_BEGIN_FORMAT_STORAGE "BeginFormatSD2.amr"    /* 开始擦除全部语音数据,请稍后 */
#define EVENT_VOICE_FINISH_FORMAT_STORAGE "EndFormatSD2.amr"     /* 全部语音数据擦除完成 */

/*******************************************************
 * 数据结构
 *******************************************************/

/* 提示音播放列表中的列表项数据结构 */
typedef struct _play_file_info_t
{
    char name[PLAY_FILE_NAME_LEN];
    sint32_t offset;
} play_file_info_t;

/* 提示音播放列表数据结构 */
typedef struct play_list_t
{
    play_file_info_t play_info[PLAY_FILE_NUMS];
    sint32_t head;
    sint32_t tail;
    pthread_mutex_t mutex;
} play_list_t;

/* 播放线程的播放状态 */
typedef enum _PlayDetailState
{
    PlayDetailState_Idle,                 /* 空闲状态. */
    PlayDetailState_PlayInit,             /* 初始化状态. */
    PlayDetailState_PlayReading,          /* 正在从文件中读取语音数据. */
    PlayDetailState_PlayRecordFinishRead, /* 从文件中读取完毕,但未能播放完毕. */
    PlayDetailState_PlayRecordFinishPlay, /* 播放完毕. */
} E_PlayDetailState;

/* 播放线程的播放模式 */
typedef enum _PlayMode
{
    PlayMode_Voice,  /* 播放录音模式. */
    PlayMode_Prompt, /* 播放提示音模式. */
} E_PlayMode;

/*******************************************************
 * 全局变量
 *******************************************************/

/* 播放提示音列表 */
static play_list_t *g_play_list = NULL;
/* 立即停止播放标志 */
static volatile bool g_stop_play = false;
/* 播放最后一条语音 */
static volatile bool g_play_voice = false;
/* AMR播放块大小 */
short g_block_size[16] =
    {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};

/*******************************************************
 * 函数声明
 *******************************************************/

/* 初始化提示音播放列表 */
static void init_play_list(play_list_t *list);
/* 在提示音播放列表中增加一个元素 */
static sint32_t push_play_list(play_list_t *list, char *name, sint32_t offset);
/* 从文件列表中读取要播放的文件 */
static sint32_t read_play_list(play_list_t *list, char *name, sint32_t *offset);
/* 获取提示音播放列表的大小 */
static sint32_t get_play_list_size(play_list_t *list);
/* 读取一帧的语音数据 */
static sint32_t read_frame_from_amr_file(sint32_t fd, uint8_t *out_buf, sint32_t *out_len, sint32_t *size);
/* 停止播放 */
static sint32_t play_stop_play(sint32_t fd, pcm_config_t *config);
/* 放音线程 */
static void *play_thread(void *args);

/*******************************************************
 * 函数变量
 *******************************************************/

/*******************************************************
 *
 * @brief  初始化提示音播放列表
 *
 * @param  *list: 提示音播放列表的数据指针
 * @retval None
 *
 *******************************************************/
static void init_play_list(play_list_t *list)
{
    memset(list, 0, sizeof(play_list_t));
    list->head = list->tail = 0;
    pthread_mutex_init(&list->mutex, NULL);
}

/*******************************************************
 *
 * @brief  在提示音播放列表中增加一个元素
 *
 * @param  *list: 提示音播放列表的数据指针
 * @param  *name: 文件名
 * @param  offset: 读取数据的开始位置
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
static sint32_t push_play_list(play_list_t *list, char *name, sint32_t offset)
{
    pthread_mutex_lock(&list->mutex);
    strcpy(list->play_info[list->head].name, name);
    list->play_info[list->head].offset = offset;
    list->head++;
    list->head %= PLAY_FILE_NUMS;
    pthread_mutex_unlock(&list->mutex);
    return 0;
}

/*******************************************************
 *
 * @brief  从文件列表中读取要播放的文件
 *
 * @param  *list: 文件列表数据指针
 * @param  *name: 文件名
 * @param  *offset: 从文件开始读取的位置
 * @retval -1:失败 0:成功
 *
 *******************************************************/
static sint32_t read_play_list(play_list_t *list, char *name, sint32_t *offset)
{
    pthread_mutex_lock(&list->mutex);
    if (list->head == list->tail)
    {
        pthread_mutex_unlock(&list->mutex);
        return (sint32_t)-1;
    }
    strcpy(name, list->play_info[list->tail].name);
    *offset = list->play_info[list->tail].offset;
    list->tail++;
    list->tail %= PLAY_FILE_NUMS;
    pthread_mutex_unlock(&list->mutex);
    return 0;
}

/*******************************************************
 *
 * @brief  获取提示音播放列表的大小
 *
 * @param  *list: 提示音播放列表的数据指针
 * @retval 提示音播放列表的大小
 *
 *******************************************************/
static sint32_t get_play_list_size(play_list_t *list)
{
    sint32_t ret = 0;

    pthread_mutex_lock(&list->mutex);
    if (list->head < list->tail)
        ret = list->head + PLAY_FILE_NUMS - list->tail;
    else
        ret = list->head - list->tail;
    pthread_mutex_unlock(&list->mutex);

    return ret;
}

/*******************************************************
 *
 * @brief  读取一帧的语音数据
 *
 * @param  Fd: AMR文件句柄
 * @param  *OutBuf: 读取的帧数据
 * @param  *OutLen: 读取帧数据的长度
 * @param  *Size: 语音数据总长度
 * @retval 0:成功 负数:失败
 *
 *******************************************************/
static sint32_t read_frame_from_amr_file(sint32_t fd, uint8_t *out_buf, sint32_t *out_len, sint32_t *size)
{
    /**
     * AMR帧头格式,FT为编码模式,Q为帧质量指示器,如果为0表明帧被损坏. P为填充为设置为0.
     *
     *  7 6 5 4 3 2 1 0
     * +-+-+-+-+-+-+-+-+
     * |P|   FT  |Q|P|P|
     * +-+-+-+-+-+-+-+-+
     */

    sint32_t len;
    uint8_t FT, Q, frame_size;
    sint32_t data_len = *size;

    while (data_len > 0)
    {
        /* Read and find the mode byte */
        len = read(fd, out_buf, 1);
        if (len <= 0)
        {
            return (sint32_t)-1;
        }
        /* 判断是否为填充字节 */
        if (out_buf[0] == 0xff)
        {
            continue;
        }
#if 0
        rt_kprintf( "buff[0]:%x\n", out_buf[0]);
#endif
        Q = (out_buf[0] >> 2) & 0x01;
        FT = (out_buf[0] >> 3) & 0x0F;
        if ((Q != 0x01))
        {
            rt_kprintf("head error:%x\n", out_buf[0]);
            data_len--;
            continue;
        }

        frame_size = g_block_size[FT];
#if 0
        rt_kprintf( "fs:%x\ndata:", frame_size);
#endif
        /* Find the packet size */
        len = read(fd, out_buf + 1, frame_size);
#if 0
        sint32_t i;
        for (i = 0; i < frame_size; i++) {

            rt_kprintf( "%02x ", out_buf[i]);
        }
#endif

        if (len > 0)
        {
            data_len -= len;
        }
        if (len != frame_size)
        {
            return -2;
        }
        *out_len = len + 1;
        return 0;
    }
    return 0;
}

/*******************************************************
 *
 * @brief  停止播放
 *
 * @param  fd: 文件句柄
 * @param  *config: 配置的数据指针
 * @retval 0:成功 负数:失败
 *
 *******************************************************/
static sint32_t play_stop_play(sint32_t fd, pcm_config_t *config)
{
    sint32_t ret = 0;

    /* 关闭录音文件 */
    if (fd > 0)
    {
        ret = close(fd);
        if (ret < 0)
        {
            log_print(LOG_ERROR, "close error, ret=%d.\n", ret);
            ret |= (1 << 31);
        }
        fd = -1;
    }

    if (config->dev != NULL)
    {
        /* 关闭pcm设备 */
        ret = pcm_exit(config);
        if (ret < 0)
        {
            log_print(LOG_ERROR, "destory pcm device error, ret=%d. \n", ret);
            ret |= (1 << 31);
        }
    }
    /* 音频接口进入空闲状态 */
    log_print(LOG_INFO, "end play! \n");
    return ret;
}
/*******************************************************
 *
 * @brief  放音线程
 *
 * @param  *arg: 线程输入参数
 * @retval 线程返回值
 *
 *******************************************************/

static void *play_thread(void *args)
{
    sint32_t ret = 0;
    sint32_t *destate = NULL;
    uint8_t analysis[AMR_FRAME_MAX_LEN]; /* 需要分析的数据 */
    sint32_t read_size = 0;
    sint32_t play_size = 0;
    char play_file_name[PLAY_FILE_NAME_LEN];
    E_PCM_STATE pcm_state = PCM_STATE_IDLE;                     /* PCM接口状态 */
    E_PlayDetailState play_detail_state = PlayDetailState_Idle; /* 播放线程的播放状态 */
    E_PlayMode play_mode = PlayMode_Voice;                      /* 播放模式 */
    sint32_t play_fd = 0;                                       /* 播放文件句柄 */
    pcm_config_t *config = NULL;                                /* 放音模块配置 */
    sint32_t play_offset;                                       /* 播放语音的偏移量 */
    struct stat sb;
    sint32_t i = 0;

    /* 分配内存空间 */
    config = (pcm_config_t *)malloc(sizeof(pcm_config_t));
    if (config == NULL)
    {
        log_print(LOG_ERROR, "error, config pointer is null. \n");
        return NULL;
    }
    memset(config, 0, sizeof(pcm_config_t));
    /* 初始化解码器 */
    destate = Decoder_Interface_init();

    log_print(LOG_INFO, "play thread start ...\n");
    while (true)
    {
        /* 其他模块给出命令(g_stop_play 或者 g_play_voice), 需要立即结束放音. */
        if ((g_stop_play == true) || (g_play_voice == true))
        {
            g_stop_play = false;
            ret = play_stop_play(play_fd, config);
            if (ret < 0)
            {
                log_print(LOG_ERROR, "error, play_stop_play error, ret=%d.\n", ret);
            }
            data_set_pcm_state(PCM_STATE_IDLE);
            play_detail_state = PlayDetailState_Idle;
        }

        /* 获取当前PCM的状态 */
        pcm_state = data_get_pcm_state();
        /* 不处于播放状态,则休眠. */
        if (pcm_state == PCM_STATE_RECORDING)
        {
            msleep((uint32_t)30);
            continue;
        }
        switch (play_detail_state)
        {
        case PlayDetailState_Idle:    /* 空闲状态 */
            if (g_play_voice == true) /* 需要播放最后一条语音 */
            {
                g_play_voice = false;
                play_detail_state = PlayDetailState_PlayInit;
                play_mode = PlayMode_Voice;
            }
            else if (get_play_list_size(g_play_list) > 0) /* 提示音播放列表不为空, 则开始播放提示音. */
            {
                play_detail_state = PlayDetailState_PlayInit;
                play_mode = PlayMode_Prompt;
            }
            else /* 提示音播放列表为空, 休眠30ms. */
            {
                msleep((uint32_t)30);
            }
            break;
        case PlayDetailState_PlayInit: /* 初始化状态 */
            data_set_pcm_state(PCM_STATE_PLAYING);
            if (play_mode == PlayMode_Voice) /* 播放录音文件 */
            {
                /* 设置PCM设备参数 */
                config->mode = SOUND_MODE_PLAY;    /* 设置放音模式 */
                config->channels = CHANNEL_NUM;    /* 设置通道数为2 */
                config->sample_rate = SMAPLE_RATE; /* 设置采样率为8000Hz */
                config->format = SMAPLE_BITS;      /* 设置样本数据的大小 */

                /* pcm设备初始化 */
                ret = pcm_init(config);
                if (ret < 0)
                {
                    break;
                }

                /* 读取当前放音信息 */
                sprintf(play_file_name, "%s/%s", YUYIN_PATH_NAME, g_cur_rec_file_info.filename);
                play_offset = g_cur_rec_file_info.new_voice_head_offset + PAGE_SIZE;

                /* 打开录音文件 */
                play_fd = open(play_file_name, O_RDWR | O_CREAT);
                if (play_fd > 0)
                {
                    /* 修改头文件 */
                    pthread_mutex_lock(&g_mutex_voice_file);
                    fm_modify_play_flag(play_fd);
                    pthread_mutex_unlock(&g_mutex_voice_file);

                    fsync(play_fd);
                    lseek(play_fd, play_offset, SEEK_SET);

                    /* 计算偏移量 */
                    memset(&sb, 0, sizeof(struct stat));
                    ret = fstat(play_fd, &sb);
                    play_size = sb.st_size - (g_cur_rec_file_info.new_voice_head_offset + PAGE_SIZE);

                    /* 记录当前信息 */
                    log_print(LOG_INFO,
                              "begin play voice: %s, offset: %x, data len: %x. \n",
                              play_file_name, play_offset, play_size);
                }
            }
            else /* 播放提示音 */
            {
                /* 从提示音提示音播放列表中取出需要播放的文件 */
                if (read_play_list(g_play_list, play_file_name, &play_offset) == 0)
                {
                    /* 设置PCM设备参数 */
                    config->mode = SOUND_MODE_PLAY;    /* 设置放音模式 */
                    config->channels = CHANNEL_NUM;    /* 设置通道数为1 */
                    config->sample_rate = SMAPLE_RATE; /* 设置采样率为8000Hz */
                    config->format = SMAPLE_BITS;      /* 设置样本数据的大小 */
                    /* pcm设备初始化 */
                    ret = pcm_init(config);
                    if (ret < 0)
                    {
                        log_print(LOG_ERROR, "init pcm device error. \n");
                        break;
                    }

                    play_fd = open(play_file_name, O_RDONLY);
                    if (play_fd > 0)
                    {
                        lseek(play_fd, play_offset, SEEK_SET);
                        if (fstat(play_fd, &sb) == 0)
                        {
                            play_size = sb.st_size - play_offset;
                        }
                        log_print(LOG_INFO, "begin play : %s, offset: %d, data len: %d.\n",
                                  play_file_name, play_offset, play_size);
                    }
                    else
                    {
                        log_print(LOG_ERROR, "error, can not open %s. \n", play_file_name);
                        play_detail_state = PlayDetailState_PlayRecordFinishPlay;
                        break;
                    }
                }
            }
            play_detail_state = PlayDetailState_PlayReading;
            break;
        case PlayDetailState_PlayReading: /* 播放状态,读取语音数据中,并同时播放 */
            ret = read_frame_from_amr_file(play_fd, analysis, &read_size, &play_size);
            if (ret == 0) /* 读取一帧数据成功 */
            {
                /* 清空PCM缓冲区 */
                memset(config->p_buffer, 0, config->size);
                /* 调用解码器 */
                Decoder_Interface_Decode(destate, analysis, (Word16 *)config->p_buffer, 0);

                /* 将单通道的数据变为双通道 */
                for (i = config->size / 2 - 2; i >= 0; i = i - 2)
                {
                    config->p_buffer[i * 2 + 0] = config->p_buffer[i + 0];
                    config->p_buffer[i * 2 + 2] = config->p_buffer[i + 0];
                    config->p_buffer[i * 2 + 1] = config->p_buffer[i + 1];
                    config->p_buffer[i * 2 + 3] = config->p_buffer[i + 1];
                }
                /* 写音频数据到PCM设备 */
                while ((ret = pcm_write(config->dev, config->p_buffer, config->size)) < 0)
                {
                    msleep((uint32_t)2);
                    if (ret < 0)
                    {
                        log_print(LOG_ERROR, "write pcm error.\n");
                    }
                }
                play_detail_state = PlayDetailState_PlayReading;
            }
            else /* 读取一帧数据失败,表明读取AMR语音数据完毕. */
            {
                play_detail_state = PlayDetailState_PlayRecordFinishRead;
            }
            break;
        case PlayDetailState_PlayRecordFinishRead: /* 播放状态,从文件中读取完毕,但未能播放完毕. */
            if (config->dev > 0)
            {
                play_detail_state = PlayDetailState_PlayRecordFinishPlay;
            }
            msleep((uint32_t)20);
            break;
        case PlayDetailState_PlayRecordFinishPlay: /* 播放完毕. */
            ret = play_stop_play(play_fd, config);
            if (ret < 0)
            {
                log_print(LOG_ERROR, "stop play error, ret=%d.\n", ret);
            }
            event_push_queue(EVENT_PLAY_END);
            play_detail_state = PlayDetailState_Idle;
            data_set_pcm_state(PCM_STATE_IDLE);
            break;
        default:
            break;
        }
    } /* while */

    log_print(LOG_DEBUG, "fatal error, play thread end.\n");
    Decoder_Interface_Decode(destate, analysis, (Word16 *)config->p_buffer, 0);
    return NULL;
}
/*******************************************************
 *
 * @brief  播放最后一条录音
 *
 * @retval None
 *
 *******************************************************/
void play_voice(void)
{
    g_play_voice = true;
}

/*******************************************************
 *
 * @brief  立即停止播放录音
 *
 * @retval None
 *
 *******************************************************/
void play_stop(void)
{
    g_stop_play = true;
}

/*******************************************************
 *
 * @brief  播放语音事件
 *
 * @param  event: 事件
 * @retval 无
 *
 *******************************************************/
void play_event(E_EVENT event)
{
    char event_path[PATH_NAME_MAX_LEN];
    char *filename = NULL;

    memset(event_path, 0, sizeof(event_path));
    switch (event)
    {
    case EVENT_DUMP_START_ALL: /* 开始转储全部文件 */
        filename = EVENT_VOICE_DUMP_START_ALL;
        break;
    case EVENT_DUMP_START_LAST: /* 开始转储最新文件 */
        filename = EVENT_VOICE_DUMP_START_LAST;
        break;
    case EVENT_DUMP_END_LAST: /* 最新文件转储成功 */
        filename = EVENT_VOICE_DUMP_END_LAST;
        break;
    case EVENT_DUMP_END_ALL: /* 全部文件转储成功 */
        filename = EVENT_VOICE_DUMP_END_ALL;
        break;
    case EVENT_DUMP_FAIL: /* 转储失败 */
        filename = EVENT_VOICE_DUMP_FAIL;
        break;
    case EVENT_DUMP_USB_FULL: /* U盘已满 */
        filename = EVENT_VOICE_DUMP_USB_FULL;
        break;
    case EVENT_DUMP_USB_ILLEGAL: /* 未鉴权U盘,转储失败 */
        filename = EVENT_VOICE_DUMP_USB_ILLEGAL;
        break;
    case EVENT_PLAY_LAST: /* 开始回放最后一条语音 */
        break;
    case EVENT_TAX_COMM_NORMAL: /* TAX通信正常 */
        break;
    case EVENT_TAX_COMM_ERROR: /* TAX通信失败 */
        break;
    case EVENT_RECORD_START: /* 开始录音 */
        break;
    case EVENT_RECORD_END: /* 录音结束 */
        break;
    case EVENT_UPDATE_BEGIN: /* 开始更新程序 */
        filename = EVENT_VOICE_UPDATE_BEGIN;
        break;
    case EVENT_UPDATE_SUCCESS: /* 更新程序成功 */
        filename = EVENT_VOICE_UPDATE_SUCCESS;
        break;
    case EVENT_UPDATE_FAIL: /* 更新程序失败 */
        filename = EVENT_VOICE_UPDATE_FAIL;
        break;
    case EVENT_BEGIN_FORMAT_STORAGE: /* 开始擦除全部语音数据,请稍后 */
        filename = EVENT_VOICE_BEGIN_FORMAT_STORAGE;
        break;
    case EVENT_FINISH_FORMAT_STORAGE: /* 全部语音数据擦除完成 */
        filename = EVENT_VOICE_FINISH_FORMAT_STORAGE;
        break;
    default:
        break;
    }
    if (filename == NULL)
        return;

    sprintf(event_path, "%s/%s", EVENT_VOICE_DIR, filename);
    push_play_list(g_play_list, event_path, AMR_FILE_HEADER_LEN);
}
/*******************************************************
 *
 * @brief  放音模块初始化
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
sint32_t play_init(void)
{
    sint32_t ret = 0;
    pthread_t play_tid;
    pthread_attr_t pthread_attr_data;

    /* 分配内存空间 */
    g_play_list = (play_list_t *)malloc(sizeof(play_list_t));
    if (g_play_list == NULL)
        return (sint32_t)-1;
    /* 初始化播放音列表 */
    init_play_list(g_play_list);

    /* 创建放音线程 */
    pthread_attr_init(&pthread_attr_data);
    pthread_attr_data.stacksize = 1024 * 5;
    pthread_attr_data.schedparam.sched_priority = 22;
    ret = pthread_create(&play_tid, &pthread_attr_data, (void *)play_thread, NULL);
    pthread_attr_destroy(&pthread_attr_data);
    if (ret < 0)
    {
        log_print(LOG_DEBUG, "create play thread error.\n ");
        return (sint32_t)-1;
    }

    return 0;
}
