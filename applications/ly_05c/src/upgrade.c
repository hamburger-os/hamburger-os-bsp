/*******************************************************
 *
 * @FileName: upgrade.c
 * @Date: 2023-05-24 16:06:26
 * @Author: ccy
 * @Description: 升级模块
 * 
 * Copyright (c) 2023 by thinker, All Rights Reserved. 
 *
*******************************************************/


/*******************************************************
 * 头文件
*******************************************************/
 
#include <ota_from_file.h>
#include <stdio.h>
#include <event.h>

/*******************************************************
 * 宏定义
*******************************************************/
 
/*升级状态标志文件 */
#define UPGRADE_STATA_FILE "/mnt/emmc/yysj/upgrade_state"
/*升级状态标志文件读写最大缓冲区大小 */
#define UPGRADE_STATA_MAX_LEN 16

/*******************************************************
 * 数据结构
*******************************************************/

/*升级状态标志 */
typedef enum _UpgradeState
{
    UPGRADE_STATE_NOT = 0,       /*非升级状态 */
    UPGRADE_STATE_UPGRADING = 1, /*升级中 */
    UPGRADE_STATE_INVALID = 2,   /*非有效标志 */
} E_UPGRADE_STATE;

/*******************************************************
 *
 * @brief  函数描述
 *
 * @param  int update: 0:非升级状态 1:升级状态
 * @retval int 0:成功 -1:失败
 * 
*******************************************************/
static int upgrade_set_flag(E_UPGRADE_STATE state)
{
    FILE *fp = NULL;
    int ret = 0;

    /*打开文件 */
    fp = fopen(UPGRADE_STATA_FILE, "w+");
    if (fp == NULL)
        return -1;

    /*写入标志 */
    fprintf(fp, "%d", state);
    if (ret < 0)
        return -1;

    /*关闭文件 */
    ret = fclose(fp);
    if (ret < 0)
        return -1;

    return 0;
}
/*******************************************************
 *
 * @brief  获取升级状态标志
 *
 * @retval E_UPGRADE_STATE 升级状态标志
 *
*******************************************************/
static E_UPGRADE_STATE upgrade_get_flag(void)
{
    FILE *fp = NULL;
    char buff[UPGRADE_STATA_MAX_LEN] = {0};
    char *p = NULL;
    int ret = 0;

    fp = fopen(UPGRADE_STATA_FILE, "r+");
    if (fp == NULL)
        return UPGRADE_STATE_INVALID;

    p = fgets(buff, UPGRADE_STATA_MAX_LEN, fp);
    if (p == NULL)
        return UPGRADE_STATE_INVALID;

    ret = fclose(fp);
    if (ret < 0)
        return UPGRADE_STATE_INVALID;

    return (buff[0] == '1') ? UPGRADE_STATE_UPGRADING : UPGRADE_STATE_NOT;
}
/*******************************************************
 *
 * @brief  重定义升级函数
 *
 * @param  OtaHandleTypeDef type: 升级过程中,发生的消息类型.
 * @retval void 无
 * 
 *******************************************************/
void ota_from_file_handle(OtaHandleTypeDef type)
{
    E_UPGRADE_STATE state = UPGRADE_STATE_NOT;

    switch (type)
    {
    case OTA_HANDLE_START: /*开始更新 */
        upgrade_set_flag(UPGRADE_STATE_UPGRADING);
        event_push_queue(EVENT_UPDATE_BEGIN);
        sleep(2);/*此处延迟, 主要为播放提示音留出时间. */
        break;
    case OTA_HANDLE_FINISH: /*完成更新 */
        break;
    case OTA_HANDLE_LOADED: /*download 分区内的镜像文件已经被装载.  */
        state = upgrade_get_flag();
        if (state == UPGRADE_STATE_UPGRADING)
        {
            upgrade_set_flag(UPGRADE_STATE_NOT);
            event_push_queue(EVENT_UPDATE_SUCCESS);
        }
        break;
    case OTA_HANDLE_FAILED: /*download 分区内的镜像文件没有被装载. */
        state = upgrade_get_flag();
        if (state == UPGRADE_STATE_UPGRADING)
        {
            upgrade_set_flag(UPGRADE_STATE_NOT);
            event_push_queue(EVENT_UPDATE_FAIL);
        }
        break;
    default:
        break;
    }
}
