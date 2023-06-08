/*******************************************************
 *
 * @FileName: log.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 文件管理模块的头文件
 *
 * Copyright (c) 2023 by thinker, All Rights Reserved.
 *
 *******************************************************/

#ifndef _FILE_MANAGER_H_
#define _FILE_MANAGER_H_

/*******************************************************
 * 头文件
 *******************************************************/

#include "data.h"
#include "common.h"
#include "type.h"

/*******************************************************
 * 宏定义
 *******************************************************/

/* 文件路径的相关宏 */
#define TARGET_DIR_NAME "/mnt/udisk/ud0p0/yysj/voice"     /* U盘中保存语音文件的路径 */
#define UPGRADE_FILE_NAME "/mnt/udisk/ud0p0/rtthread.rbl" /* 升级文件 */
#define YUYIN_PATH_NAME "/mnt/emmc/yysj"                  /* 语音文件在板子上的存储路径 */
#define YUYIN_BAK_PATH_NAME "/mnt/emmc/yysj/bak"          /* 语音文件在板子上的备份路径 */
#define NEW_FILE_NAME_CONF "latest_filename.conf"         /* 存放最新文件名的配置文件名称 */
#define LOG_FILE_PATH "/mnt/emmc/yysj/log"                /* 日志目录 */
#define LOG_FILE_NAME "/mnt/emmc/yysj/log/voice.log"      /* 日志文件 */
#define BUFFER_DIR "/mnt/emmc/yysj/buffer"                /* 缓存目录 */
#define YUYIN_STORAGE_MAX_SIZE (256 * 1024)               /* 板子存储器最大存储空间大小,单位: KB */

/* 录音文件的相关宏 */
#define PAGE_SIZE 512                                     /* 页大小 */
#define VOICE_VALID_LENGTH 27                             /* 每一包数据的有效个数,每一包的时间长度为20ms */
#define F_MODE (S_IRUSR | S_IWUSR)                        /* 目录 */
#define FILE_HEAD_FLAG "FILE_HEAD_FLAG|HENANSIWEI"        /* 文件头 */
#define VOICE_HEAD_FLAG "VOICE_HEAD_FLAG|HENANSIWEI"      /* 语音头 */

/* 版本相关的宏 */
#define PRG_VERISON 11 /* 程序版本 */
#define VERISON "LY-15录音板程序.\n版本:"__DATE__ \
                "\n" /* 版本 */

/* 路径最大长度 */
#define FILE_NAME_MAX_LEN 32

/*升序 */
#define SORT_UP 0x01
/*降序 */
#define SORT_DOWN 0x10

/*文件头 */
#define FILE_HEAD_FLAG_LEN 32      /* 文件头标志字段长度 */
#define TRAIN_ID_TYPE_LEN 4        /* 车次种类字段长度 */
#define TRAIN_ID_LEN 3             /* 车次字段长度 */
#define DRIVER_ID_LEN 3            /* 司机号字段长度 */
#define DATE_TIME_LEN 4            /* 年月日时分秒字段长度 */
#define KILOMETER_POST_LEN 3       /* 公里标字段长度 */
#define SPEED_LEN 3                /* 实速字段长度 */
#define STATION_LEN 2              /* 车站号字段长度 */
#define TOTAL_WEIGHT_LEN 2         /* 总重字段长度 */
#define ASSISTANT_DRIVER_LEN 3     /* 副司机字段长度 */
#define LOCOMOTIVE_NUM_LEN 2       /* 机车号字段长度 */
#define LOCOMOTIVE_TYPE_LEN 2      /* 机车型号字段长度 */
#define TOTAL_VOICE_NUM_LEN 2      /* 总语音条数字段长度 */
#define FILE_HEAD_RESERVE2_LEN 23  /* 保留字段长度 */
#define FILE_HEAD_RESERVE3_LEN 410 /* 保留字段长度 */

/*语音头 */
#define VOICE_HEAD_SIGNAL_MACHINE_ID_LEN 2 /* 信号机编号 */
#define VOICE_HEAD_PLAY_TIME_LEN 4         /* 播放时间 */
#define VOICE_HEAD_RESERVE2_LEN 10         /* 预留2 */
#define VOICE_HEAD_RESERVE3_LEN 411        /* 预留3 */

/*tax32 */
#define TAX32_TRAIN_NUM_TYPE_LEN 4       /* 车次种类标识符字段长度 */
#define TAX32_DRIVER_SECTION_LEN 2       /* 司机局段号字段长度 */
#define TAX32_BRAKE_CYLINDER_PRESS_LEN 2 /* 闸缸压力字段长度 */
#define TAX32_DIESEL_ENGINE_SPEED_LEN 2  /* 柴油机转速字段长度 */
#define TAX32_TOTAL_DISPLACEMENT_LEN 2   /* 累计位移字段长度 */

/*tax40 */
#define TAX40_SIGNAL_MACHINE_ID_LEN 2   /* 信号机编号字段长度 */
#define TAX40_TOTAL_LEN_LEN 2           /* 计长字段长度 */
#define TAX40_DRIVER_ID_LEN 2           /* 司机号字段长度 */
#define TAX40_ASSISTANT_DRIVER_ID_LEN 2 /* 副司机号字段长度 */
#define TAX40_TUBE_PRESS_LEN 2          /* 管压字段长度 */

/*******************************************************
 * 数据结构
 *******************************************************/

/* 当前录音文件信息 */
typedef struct __attribute__((packed)) /* 按照字节对齐*/
{
    char filename[FILE_NAME_MAX_LEN]; /* 最新的文件名 */
    int fd;                           /* 文件句柄 */
    unsigned int voices_num;          /* 文件中的语音条数 */
    int new_voice_head_offset;        /* 最新语音的偏移量 */
    unsigned int file_index;          /* 文件的序号 */
    unsigned int voice_index;         /* 语音序号 */
    int not_exsit;                    /* 为1表示录音板中还没有生成文件 */
    unsigned int record_datalen;      /* 录音数据长度 */
    int channel;                      /* 通道号 */
} current_rec_file_info_t;

/* 文件头结构体 */
typedef struct __attribute__((packed)) /* 按照字节对齐*/
{
    unsigned char file_head_flag[FILE_HEAD_FLAG_LEN];      /* 文件头标志 */
    unsigned int file_len;                                 /* 文件长度(单位页page) */
    unsigned int file_index;                               /* 文件序号 */
    unsigned char trainid_type[TRAIN_ID_TYPE_LEN];         /* 车次种类 */
    unsigned char train_id[TRAIN_ID_LEN];                  /* 车次 */
    unsigned char benbu_kehuo;                             /* 本/补、客/货 */
    unsigned char driver_id[DRIVER_ID_LEN];                /* 司机号 */
    unsigned char version;                                 /* 程序版本号 */
    unsigned char date_time[DATE_TIME_LEN];                /* 年月日时分秒 */
    unsigned char kilometer_post[KILOMETER_POST_LEN];      /* 公里标 */
    unsigned char speed[SPEED_LEN];                        /* 实速 */
    unsigned char real_road;                               /* 实际交路号 */
    unsigned char input_road;                              /* 输入交路号 */
    unsigned char station[STATION_LEN];                    /* 车站号 */
    unsigned char total_weight[TOTAL_WEIGHT_LEN];          /* 总重 */
    unsigned char assistant_driver[ASSISTANT_DRIVER_LEN];  /* 副司机 */
    unsigned char locomotive_num[LOCOMOTIVE_NUM_LEN];      /* 机车号 */
    unsigned char locomotive_type[LOCOMOTIVE_TYPE_LEN];    /* 机车型号 */
    unsigned char total_voice_number[TOTAL_VOICE_NUM_LEN]; /* 总语音条数 */
    unsigned char reserve2[FILE_HEAD_RESERVE2_LEN];        /* 保留 */
    unsigned char dump_flag;                               /* 转储标志 */
    unsigned char reserve3[FILE_HEAD_RESERVE3_LEN];        /* 保留 */
    unsigned char check_sum;                               /* 校验和 */
} file_head_t;

/* 语音头信息 */
typedef struct __attribute__((packed)) /* 按照字节对齐*/
{
    char voice_head_flag[FILE_HEAD_FLAG_LEN];                          /* 语音头标志 */
    unsigned int voice_length;                                         /* 语音长度(单位页page) */
    unsigned char trainid_type[TRAIN_ID_TYPE_LEN];                     /* 车次种类 */
    unsigned char train_id[TRAIN_ID_LEN];                              /* 车次 */
    unsigned char benbu_kehuo;                                         /* 本/补、客/货 */
    unsigned char driver_id[DRIVER_ID_LEN];                            /* 司机号 */
    unsigned char version;                                             /* 程序版本号,新增 */
    unsigned char date_time[DATE_TIME_LEN];                            /* 年月日时分秒 */
    unsigned char kilometer_post[KILOMETER_POST_LEN];                  /* 公里标 */
    unsigned char speed[SPEED_LEN];                                    /* 实速 */
    unsigned char real_road;                                           /* 实际交路号 */
    unsigned char input_road;                                          /* 输入交路号 */
    unsigned char station[STATION_LEN];                                /* 车站号 */
    unsigned char total_weight[TOTAL_WEIGHT_LEN];                      /* 总重 */
    unsigned char assistant_driver[ASSISTANT_DRIVER_LEN];              /* 副司机 */
    unsigned char locomotive_num[LOCOMOTIVE_NUM_LEN];                  /* 机车号 */
    unsigned char locomotive_type[LOCOMOTIVE_TYPE_LEN];                /* 机车型号 */
    unsigned char channel;                                             /* 通道号 */
    unsigned int valid_data_length;                                    /* 有效数据长度(单位字节) */
    unsigned int voice_index;                                          /* 语音序号 */
    unsigned char voice_flag;                                          /* bit0:为0表示该条语音已经通过按键放过音,为1表示没有放过音. */
    unsigned char voice_play_time[VOICE_HEAD_PLAY_TIME_LEN];           /* 播放时间 */
    unsigned char locomotive_signal_type;                              /* 机车信号类型 */
    unsigned char signal_machine_id[VOICE_HEAD_SIGNAL_MACHINE_ID_LEN]; /* 信号机编号 */
    unsigned char signal_machine_type;                                 /* 信号机种类 */
    unsigned char monitor_state;                                       /* 监控状态 */
    unsigned char reserve2[VOICE_HEAD_RESERVE2_LEN];                   /* Change 15 to 10 */
    unsigned char reserve3[VOICE_HEAD_RESERVE3_LEN];                   /* 预留3 */
    unsigned char check_sum;                                           /* 校验和 */
} voice_head_t;

/*TAX本板数据1 */
typedef struct __attribute__((packed)) /* 按照字节对齐*/
{
    unsigned char board_addr;                                              /* 本板地址 */
    unsigned char feature_code;                                            /* 特征码 */
    unsigned char flag;                                                    /* 标志 */
    unsigned char version;                                                 /* 版本号 */
    unsigned char monitor_state;                                           /* 监控状态 */
    unsigned char station_ext;                                             /* 车站号 车站号扩充字节 */
    unsigned char train_num_type[TAX32_TRAIN_NUM_TYPE_LEN];                /* 车次种类标识符 */
    unsigned char driver_id;                                               /* 司机号扩充字节 */
    unsigned char assistant_driver;                                        /* 副司机号扩充字节 */
    unsigned char driver_section_id[TAX32_DRIVER_SECTION_LEN];             /* 司机局段号 */
    unsigned char locomotive_type;                                         /* 机车型号扩充字节 */
    unsigned char real_road;                                               /* 实际交路号 */
    unsigned char brake_cylinder_pressure[TAX32_BRAKE_CYLINDER_PRESS_LEN]; /* 闸缸压力 */
    unsigned char brake_output;                                            /* 制动输出 */
    unsigned char diesel_engine_speed[TAX32_DIESEL_ENGINE_SPEED_LEN];      /* 柴油机转速/原边电流 */
    unsigned char total_displacement[TAX32_TOTAL_DISPLACEMENT_LEN];        /* 累计位移 */
    unsigned char local_branch;                                            /* 本分区支线 */
    unsigned char local_branch_lateral;                                    /* 本分区侧线 */
    unsigned char front_branch;                                            /* 前方分区支线 */
    unsigned char front_branch_lateral;                                    /* 前方分区侧线 */
    unsigned char benbu_kehuo;                                             /* 本/补、客/货 */
    unsigned char train_id[TRAIN_ID_LEN];                                  /* 车次 */
    unsigned char check_sum;                                               /* 检查和 */
} tax32_t;

/* TAX本板数据2 */
typedef struct __attribute__((packed)) /* 按照字节对齐*/
{
    unsigned char board_addr;                                      /* 本板地址 */
    unsigned char feature_code;                                    /* 特征码 */
    unsigned char detect_unit_code;                                /* 检测单元代号 */
    unsigned char date[DATE_TIME_LEN];                             /* 年、月、日、时、分、秒 */
    unsigned char speed[SPEED_LEN];                                /* 实速 */
    unsigned char locomotive_signal_type;                          /* 机车信号 */
    unsigned char locomotive_condition;                            /* 机车工况 */
    unsigned char signal_machine_id[TAX40_SIGNAL_MACHINE_ID_LEN];  /* 信号机编号 */
    unsigned char signal_machine_type;                             /* 信号机种类 */
    unsigned char kilometer_post[KILOMETER_POST_LEN];              /* 公里标 */
    unsigned char total_weight[TOTAL_WEIGHT_LEN];                  /* 总重 */
    unsigned char total_len[TAX40_TOTAL_LEN_LEN];                  /* 计长 */
    unsigned char train_num;                                       /* 辆数 */
    unsigned char benbu_kehuo;                                     /* 本/补、客/货 */
    unsigned char train_id[TRAIN_ID_LEN];                          /* 车次 */
    unsigned char section_id;                                      /* 区段号(交路号) */
    unsigned char station_ext;                                     /* 车站号 */
    unsigned char driver_id[TAX40_DRIVER_ID_LEN];                  /* 司机号 */
    unsigned char assistant_driver[TAX40_ASSISTANT_DRIVER_ID_LEN]; /* 副司机号 */
    unsigned char locomotive_num[LOCOMOTIVE_NUM_LEN];              /* 机车号 */
    unsigned char locomotive_type;                                 /* 机车型号 */
    unsigned char tube_press[TAX40_TUBE_PRESS_LEN];                /* 管压 */
    unsigned char device_state;                                    /* 装置状态 */
    unsigned char reserve;                                         /* 预留 */
    unsigned char check_sum;                                       /* 检查和2 */
} tax40_t;

/*******************************************************
 * 全局变量
 *******************************************************/
/* 保护锁, 用于保护g_tax32, g_tax40 */
extern pthread_mutex_t g_mutex_voice_file;
/* 第一段tax信息 */
extern tax32_t g_tax32;
/* 第二段tax信息 */
extern tax40_t g_tax40;
/* 当前录音文件的信息 */
extern current_rec_file_info_t g_cur_rec_file_info;

/*******************************************************
 * 函数声明
 *******************************************************/
/*******************************************************
 *
 * @brief  释放空间
 *
 * @param  none
 * @retval 0:成功 <0:失败
 *
 *******************************************************/
int free_space(void);
/*******************************************************
 *
 * @brief   初始化最新的文件信息,把最新文件的信息放到全局变量中.
 *          如果文件是在上次录音时候掉电,还要把文件的信息补全.
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_init_latest_file(void);

/*******************************************************
 *
 * @brief  监控信息, 判断是否需要生成新的文件,把相应信息放
 *         到CurrentRecFileInfo结构体中保存.
 *
 * @param  *args: 传输参数
 * @retval  0:产生新文件 1:追加文件
 *
 *******************************************************/
int fm_is_new(void);

/*******************************************************
 *
 * @brief  写入文件头.
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_write_file_head(int fd);
/*******************************************************
 *
 * @brief  写入语音头.
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_write_voice_head(int fd);

/*******************************************************
 *
 * @brief  修正语音头.其中要更改:
 *              1.语音长度 ( voice_length, 单位页page );
 *              2.语音的有效长度( valid_data_length );
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_modify_voice_head(int fd);
/*******************************************************
 *
 * @brief  修正文件头. 其中要更改:
 *         1.文件长度 (file_len, 单位页page)
 *         2.语音总条数 (total_voice_number, 单位页page)
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_modify_file_head(int fd);

/*******************************************************
 *
 * @brief  把当前录音文件名写入latest_filename.conf文件中
 *
 * @param  *name: 当前录音文件名
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_write_name(char *name);

/*******************************************************
 *
 * @brief  修正语音头,增加语音已放音标识和时间.
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_modify_play_flag(int fd);

/*******************************************************
 *
 * @brief  修正语音头,增加语音已放音标识和时间.
 *
 * @param  fd: 文件的句柄
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_get_file_name(char *name, int bak_flag);

/*******************************************************
 *
 * @brief  文件管理模块初始化
 *
 * @retval 0:成功 -1:失败
 *
 *******************************************************/
int fm_init(void);

#endif
