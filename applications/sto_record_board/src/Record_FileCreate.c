/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : Record_DataProc.c
**@author: Created By Sunzq
**@date  : 2015.11.19
**@brief : None
********************************************************************************************/
#include "Record_FileCreate.h"
//#include "s25fl1.h"  //TODO(mingzhao)
#include "common.h"
//#include "FM25V05_Manage.h"   //TODO(mingzhao)
//#include "ETH_Manage.h"      //TODO(mingzhao)
//#include "Record_Selfcheck.h"      //TODO(mingzhao)
//#include "System_Vote.h"      //TODO(mingzhao)
//#include "s25fl256s.h"      //TODO(mingzhao)
#include <rtthread.h>

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "utils.h"

#include "sto_record_board.h"

#define DBG_TAG "FileCreat"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>


/* private macro definition -------------------------------------------------------------------- */
/* 写文件头标志 */
uint8_t u8_FileHead_Flag = 0u;
/* 写文件内容标志 */
uint8_t u8_Contant_Flag  = 0u;
/* 写公共信息包标志 */
uint8_t u8_Gonggongxinxi_Flag = 1U; 
/* 软件周期标志 */
uint8_t SoftWare_Cycle_Flag = 0U;
/* 读写flash临时缓存 */
uint8_t FLASH_SectorWriteRead_Buff[256U] = {0U};

uint8_t ETH_DAT[12] = { 0u };   //临时定义
uint32_t JILUGESHIBANBEN = 0x01020000;

/* 定义主控设备代码，默认：0x11--A模，0x12--B模 */
uint8_t g_ZK_DevCode = 0x11;
/* 定义显示器设备代码，默认：0x21--I系，0x22--II系 */
uint8_t g_XSQ_DevCode = 0x21;
/* 定义通信板设备代码，默认：0x31--I系，0x32--II系 */
uint8_t g_TX_DevCode = 0x31;
/* 定义接口板设备代码，默认：0x41--I系，0x42--II系 */
uint8_t g_JK_DevCode = 0x41;
/* 定义CEU设备代码，默认：0x71--I系，0x72--II系 */
uint8_t g_CEU_DevCode = 0x71;
/* 定义ECU板设备代码，默认：0x81--I系，0x82--II系 */
uint8_t g_ECU_DevCode = 0x81;

/* private type definition --------------------------------------------------------------------- */
/* EBV message. */
typedef struct tagEBV_Message
{
  /* Controlling message. ====================================================================== */
  /* ABV controlling flag. */
  uint8_t  ABV_CTRL_Flag;
  
  /* ABV controlling working condition. */
  uint8_t  ABV_CTRL_WC;
  
  /* ABV controlling voltage. */
  uint16_t ABV_CTRL_VOLT;
  
  /* ABV de-pressure. */
  uint8_t  ABV_Depressure;
  
  
  /* IBV controlling flag. */
  uint8_t  IBV_CTRL_Flag;
  
  /* IBV controlling working condition. */
  uint8_t  IBV_CTRL_WC;
  
  /* IBV controlling voltage. */
  uint16_t IBV_CTRL_VOLT;
  
  /* IBV de-pressure. */
  uint8_t  IBV_Depressure;
  
  
  /* Monitor message. ========================================================================== */
  /* ABV monitor working condition. */
  uint8_t  ABV_MNT_WC_I;
  
  /* ABV monitor voltage. */
  uint16_t ABV_MNT_VOLT_I;
  
  /* ABV fault flag. */
  uint16_t ABV_Fault_I;
  
  
  /* IBV monitor working condition. */
  uint8_t  IBV_MNT_WC_I;
  
  /* IBV monitor voltage. */
  uint16_t IBV_MNT_VOLT_I;
  
  /* IBV fault flag. */
  uint16_t IBV_Fault_I;
  
  /* ABV monitor working condition. */
  uint8_t  ABV_MNT_WC_II;
  
  /* ABV monitor voltage. */
  uint16_t ABV_MNT_VOLT_II;
  
  /* ABV fault flag. */
  uint16_t ABV_Fault_II;
  
  
  /* IBV monitor working condition. */
  uint8_t  IBV_MNT_WC_II;
  
  /* IBV monitor voltage. */
  uint16_t IBV_MNT_VOLT_II;
  
  /* IBV fault flag. */
  uint16_t IBV_Fault_II;
  
  
  /* Handle message. =========================================================================== */
  /* ABV handle working condition. */
  uint8_t  ABV_HAD_WC_I;
  
  /* ABV handle voltage. */
  uint16_t ABV_HAD_VOLT_I;
  
  /* IBR working condition. */
  uint8_t  IBR_HAD_WC_I;
  
  /* IBV handle working condition. */
  uint8_t  IBV_HAD_WC_I;
  
  /* IBV handle voltage. */
  uint16_t IBV_HAD_VOLT_I;
  
  
  /* ABV handle working condition. */
  uint8_t  ABV_HAD_WC_II;
  
  /* ABV handle voltage. */
  uint16_t ABV_HAD_VOLT_II;
  
  /* IBR working condition. */
  uint8_t  IBR_HAD_WC_II;
  
  /* IBV handle working condition. */
  uint8_t  IBV_HAD_WC_II;
  
  /* IBV handle voltage. */
  uint16_t IBV_HAD_VOLT_II;
  
} EBV_Message, *pEBV_Message;

/* High level real-time message. */
typedef struct tagHLRT_Message
{
  /* Speech tips. */
  uint16_t SpeechTips;
} HLRT_Message, *pHLRT_Message;

/* LKJ high level real-time message. */
typedef struct tagLKJ_HLRT_Message
{
  /* The state of monitor. */
  uint8_t  MonitorState;
} LKJ_HLRT_Message, *pLKJ_HLRT_Message;

/* LKJ low level real-time message. */
typedef struct tagLKJ_LLRT_Message
{
  /* Pipe pressure. */
  uint16_t PipePressure;
} LKJ_LLRT_Message, *pLKJ_LLRT_Message;

/* Recording message. */
typedef struct tagRecordingMessage
{
  /* EBV message. */
  EBV_Message  EBV_MSG;
  
  /* High level real-time message. */
  HLRT_Message HLRT_MSG;
  
  /* LKJ low level real-time message. */
  LKJ_LLRT_Message SLKJ_LLRT_MSG;
  
  /* LKJ high level real-time message. */
  LKJ_HLRT_Message SLKJ_HLRT_MSG;
} RecordingMessage, *pRecordingMessage;


/* private variable declaration ---------------------------------------------------------------- */
/* Flash写BUFFER */
WRITE_BUF write_buf = { 0u };

/* flash当前状态 */
//FLASH_STATE Flash_State = { 0u };

/* 公共信息结构体 */
SFile_Public s_file_public = { 0u };

/* 当前文件目录信息 */
SFile_Directory s_File_Directory = { 0u };

/* 文件头结构体 */
SFile_Head s_file_head = { 0u };

/* 当前写入地址 */
//static uint32_t Flash_Addr = 0u;

/* 记录文件体内容CRC校验值 */
uint32_t FileContent_CRC32 = 0xFFFFFFFF;
/* 记录事件数据包缓存 */
uint8_t u8_FFFE_Encode_buf[256];
uint16_t u16_FFFE_Encode_length = 0U;
uint32_t RecordEventPkt_CRC32 = 0U;

static char nulldata[8] = { 0u };
static uint8_t u8_Clear_Flag = 0u;

CAN_FRAME  Record_CanBuffer[150] = { 0u };
/* 在公共信息中更新的记录事项变量 */
static uint8_t C_jichexinhao[6]  = { 0U };
static uint8_t C_Lkjxiansu[2]  = { 0x00U, 0x00U };
static uint8_t C_Lkjsudu[2]  = { 0x00U, 0x00U };
static uint8_t C_gongzuozhuangtai = 0U;
static uint8_t C_gongzuomoshi = 0xFFU;
static uint8_t C_liecheguanyali[2]  = { 0x00U, 0x00U };
static uint8_t C_zhidonggangyali[2]  = { 0x00U, 0x00U };
static uint8_t C_jungangyali[2]  = { 0x00U, 0x00U };
static uint8_t C_CCUfuzhuzhuangtai[1]  = { 0U };

/* 12-June-2018, by Liang Zhen. */
/* Recording message. */
static RecordingMessage  msgRecording =\
{
  /* EBV message. */
  {
    /* Controlling message. ====================================================================== */
    /* ABV controlling flag. */
    0x00U,
    /* ABV controlling working condition. */
    0x00U,
    /* ABV controlling voltage. */
    0x0000U,
    /* ABV de-pressure. */
    0x00U,
    
    /* IBV controlling flag. */
    0x00U,
    /* IBV controlling working condition. */
    0x00U,
    /* IBV controlling voltage. */
    0x0000U,
    /* IBV de-pressure. */
    0x00U,
    
    /* Monitor message. ========================================================================== */
    /* ABV monitor working condition(I). */
    0x00U,
    /* ABV monitor voltage(I). */
    0x0000U,
    /* ABV fault flag(I). */
    0x0000U,
    
    /* IBV monitor working condition(I). */
    0x00U,
    /* IBV monitor voltage(I). */
    0x0000U,
    /* IBV fault flag(I). */
    0x0000U,
    
    /* ABV monitor working condition(II). */
    0x00U,
    /* ABV monitor voltage(II). */
    0x0000U,
    /* ABV fault flag(II). */
    0x0000U,
    
    /* IBV monitor working condition(II). */
    0x00U,
    /* IBV monitor voltage(II). */
    0x0000U,
    /* IBV fault flag(II). */
    0x0000U,
    
    /* Handle message. =========================================================================== */
    /* ABV handle working condition(I). */
    0x00U,
    /* ABV handle voltage(I). */
    0x0000U,
    /* IBR working condition(I). */
    0x00U,
    
    /* IBV handle working condition(I). */
    0x00U,
    /* IBV handle voltage(I). */
    0x0000U,
    
    /* ABV handle working condition(II). */
    0x00U,
    /* ABV handle voltage(II). */
    0x0000U,
    /* IBR working condition(II). */
    0x00U,
    
    /* IBV handle working condition(II). */
    0x00U,
    /* IBV handle voltage(II). */
    0x0000U,
  },
  
  /* High level real-time message. */
  {
    /* Speech tips. */
    0x0000U,
  },
  
  /* LKJ low level real-time message. */
  {
    /* Pipe pressure. */
    0x0000U,
  },
  
  /* LKJ high level real-time message. */
  {
    /* The state of monitor. */
    0x00U
  }
};



#if 1
/* private function declaration ---------------------------------------------------------------- */
static uint8_t Record_Condition_Judge(void);
static rt_err_t Creat_FileHead(S_CURRENT_FILE_INFO *current_file_info);
static void Get_Gonggongxinxi( void );
static void WriteFileContantPkt( uint8_t num1, uint8_t num2, uint8_t device_code, uint8_t *contant, uint8_t lenth );
static void WriteGonggongxinxiPkt( void );
static void Get_FileName(SFile_Directory *directory );
static void Get_FileContant( void );
static rt_err_t Init_FileDirectory(S_CURRENT_FILE_INFO *current_file_info);
static uint8_t Init_GonggongxinxiState( void );
static void Update_FileHead(void);
static sint32_t fm_write_record_file_head(S_CURRENT_FILE_INFO *current_file_info);
static sint32_t fm_modify_record_file_head(S_CURRENT_FILE_INFO *current_file_info);
static void Update_gongyoucanshu( void );

uint16_t FFFEEncode(uint8_t *u8p_SrcData, uint16_t u16_SrcLen, uint8_t *u8p_DstData);

/* 司机操作信息*/
static void RecordingDriverOperationMessage(void);
static void RecordingDMIOperationMessage(void);
static void RecordingDDUOperationMessage(void);
void RecordingLLevelMessage(void);
void RecordingSpeedDownMessage(void);
static void RecordingHoldOutVehicleMessage(void);
static void RecordingBrakeshoePressureMessage(void);
static void RecordingIsolateSTOMessage(void);

/* STO行程规划信息 */
static void RecordingFormPlanningMessage(void);
static void RecordingGuidSpeedMessage(void);
static void RecordingGuidConditonMessage(void);
static void RecordingGuidLevelMessage(void);
static void RecordingGuidSpeedLimitMessage(void);
static void RecordingStartOptimizeMessage(void);
static void RecordingOptimizeResultMessage(void);
static void RecordingOptimizeUncontrAreaMessage(void);
static void RecordingSoonerAndLaterMessage(void);

/* 获取STO控车信息 */
static void RecordingTrainControlMessage( void );
static void RecordingAllowAssistedDriveMessage( void );
static void RecordingWholeVehicleEnterAssistedDriveMessage( void );
static void RecordingEnterAssistedDriveMessage( void );
static void RecordingWholeVehicleExitAssistedDriveMessage( void );
static void RecordingExitAssistedDriveMessage( void );
static void RecordingAllowStartMessage( void );
static void RecordingControlTrainStartMessage( void );
static void RecordingControlTrainConditonMessage( void );
static void RecordingControlTrainLevelMessage( void );
static void RecordingEnterUncontrolAreaMessage( void );
static void RecordingExitUncontrolAreaMessage( void );
static void RecordingEnterPhaseSplitterMessage( void );
static void RecordingExitPhaseSplitterMessage( void );
static void RecordingThroughCommonMessage( void );
static void RecordingAirBrakeCommonMessage( void );
static void RecordingParkingPressurizeMessage( void );
static void RecordingTextPromptMessage( void );
static void RecordingVoicePromptMessage( void );
static void RecordingWorkingStateMessage( void );
static void RecordingWorkingModeMessage( void );
static void RecordingStaticTestMessage( void );
static void RecordingSendingControlMessage( void );
static void RecordingForcePumpAirMessage( void );

/* 获取机车制动系统信息 */
static void RecordingLocoBrakeMessage( void );
static void RecordingTrainPipePressureMessage(void);
static void RecordingBrakeCylinderPressureMessage(void);
static void RecordingMainAirPressureMessage(void);
static void RecordingEqualizReservoirPressureMessage(void);
static void RecordingBrakeHandleMessage(void);
static void RecordingChargingFlowMessage(void);
static void RecordingBCUStateMessage(void);
static void RecordingPenaltyBrakeMessage(void);
static void RecordingBCUAssistedDriveMessage(void);
static void RecordingBCUPermitConditionMessage(void);
static void RecordingBCUManufacturerMessage(void);
static void RecordingBCUErrorCodeMessage(void);

/* 获取机车牵引系统信息 */
static void RecordingLocoDrawnMessage( void );
static void RecordingPhysicalHandleMessage(void);
static void RecordingPhysicalConditionMessage(void);
static void RecordingIsolationMotorMessage(void);
static void RecordingLocoExertionMessage(void);
static void RecordingLocoRaceMessage(void);
static void RecordingLocoTaxiingMessage(void);
static void RecordingPantograghStatusMessage(void);
static void RecordingMainBreakerStatusMessage(void);
static void RecordingTractionBlockMessage(void);
static void RecordingElectricBlockMessage(void);
static void RecordingPrejudgeOffMessage(void);
static void RecordingForceOffMessage(void);
static void RecordingParkBrakeMessage(void);
static void RecordingSandingStateMessage(void);
static void RecordingElectropneumaticMessage(void);
static void RecordingCCUAuxiliaryStatusMessage(void);
static void RecordingCCUAllowTestMessage(void);
static void RecordingPrimaryVoltageMessage(void);
static void RecordingPrimaryCurrentMessage(void);
static void RecordingElectricKeyMessage(void);

/* 获取LKJ系统信息 */
static void RecordingLKJSystemMessage( void );
static void RecordingApplyRevealMessage(void);
static void RecordingReceiveRevealMessage(void);
static void RecordingRevealContentMessage(void);
static void RecordingDriverNum1Message(void);
static void RecordingDriverNum2Message(void);
static void RecordingRunPathMessage(void);
static void RecordingLKJDepartDirectionMessage(void);
static void RecordingTotalWeightMessage(void);
static void RecordingTotalLengthMessage(void);
static void RecordingVehiclesNumMessage(void);
static void RecordingLoadMessage(void);
static void RecordingPassengerTrainMessage(void);
static void RecordingHeavyTrainMessage(void);
static void RecordingEmptyTrainMessage(void);
static void RecordingNonTrafficTrainMessage(void);
static void RecordingSubstituteTrainMessage(void);
static void RecordingCabooseTrainMessage(void);
static void RecordingSpeedGradeMessage(void);

/* 列车运行信息 */
static void RecordingTrainOperationMessage( void );
static void RecordingRransitCenterMessage(void);
static void RecordingLKJModeMessage(void);
static void RecordingBenchmarkingMessage(void);
static void RecordingBranchLineSelectMessage(void);
static void RecordingSideLineSelectMessage(void);
static void RecordingLKJBrakeOutputMessage(void);
static void RecordingPassingSignalMessage(void);
static void RecordingCabSignalChangeMessage(void);
static void RecordingSpeedMessage(void);
static void RecordingLimitSpeedMessage(void);
static void RecordingPassingNeutralSectionMessage(void);
static void RecordingLineDataTerminationMessage(void);
static void RecordingDataErrorMessage(void);

/* 版本信息 */
static void RecordingVersionMessage( void );
static void RecordingSoftwareVersionMessage(void);
static void RecordingSoftwareVersionInconsistMessage(void);
static void RecordingSoftwareVersionMismatchMessage(void);
static void RecordingSTOBasicDataVersionMessage(void);
static void RecordingSTOBasicDataInconsistMessage(void);
static void RecordingSTOBasicDataMismatchMessage(void);
static void RecordingLKJBasicDataVersionMessage(void);

/* 开关机时间 */
static void RecordingSwitchTimeMessage( void );
void RecordingPowerOnMessage(void);
void RecordingPowerOffMessage(void);
static void RecordingDateChangeMessage(void);	

/* 插件自检信息 */
static void RecordingSelfCheckMessage( void );
static void RecordingZKSelfCheckMessage(void);
static void RecordingTX1SelfCheckMessage(void);
static void RecordingTX2SelfCheckMessage(void);
static void RecordingJLSelfCheckMessage(void);
static void RecordingWXTXSelfCheckMessage(void);
static void RecordingWJJKSelfCheckMessage(void);
static void RecordingDMISelfCheckMessage(void);
static void RecordingCEUSelfCheckMessage(void);
static void RecordingECUSelfCheckMessage(void);
static void RecordingCommunicatWithLKJMessage(void);
static void RecordingCommunicatWithCCUMessage(void);
static void RecordingCommunicatWithBCUMessage(void);
static void RecordingCommunicatWithCIRMessage(void);
static void RecordingCommunicatWithCEUMessage(void);
static void RecordingCommunicatWithECUMessage(void);

#endif

/********************************************************************************************
** @brief: RecordBoard_DataProc
** @param: null
********************************************************************************************/

void RecordBoard_FileCreate(void)
{
    S_CURRENT_FILE_INFO *current_file_info = file_manager.current_info;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };

    if (NULL == current_file_info)
    {
        LOG_E("current_file_info is null");
        return;
    }

    /* 1000ms循环检测 */
    static uint32_t Directory_time = 0u;
//    uint32_t File_time = 0u;
    static uint32_t File_time = 0u;
    struct stat stat_l;
    int32_t ret = 0;

    if (Common_BeTimeOutMN(&Directory_time, 1000u))
    {
        Init_FileDirectory(current_file_info);
    } /* end if */

    /* 100ms循环检测 */
    if (Common_BeTimeOutMN(&File_time, 100u))
    {
        Creat_FileHead(current_file_info);
        Get_FileContant();
    } /* end if */

    /* 公共信息发生变化或者周期标志到，将已有记录事件封包存入flash */
//    LOG_I("write_buf.pos %d", write_buf.pos);
    if ((Init_GonggongxinxiState() || SoftWare_Cycle_Flag) && (write_buf.pos > 40U))
    {
//        LOG_I("SoftWare_Cycle_Flag %d", SoftWare_Cycle_Flag);
        Update_gongyoucanshu();
        /* 添加数据包头及公共信息 */
        WriteGonggongxinxiPkt();
        /* 对数据包进行CRC校验 */
        RecordEventPkt_CRC32 = CRC32CCITT(write_buf.buf + 3U, write_buf.pos - 3U, 0xFFFFFFFF);

        /* 将校验值填入buf */
        memcpy(&(write_buf.buf[write_buf.pos]), &RecordEventPkt_CRC32, 4U);

        /* 对数据包内容进行FF-FE编码转换 */
        u16_FFFE_Encode_length = FFFEEncode(write_buf.buf + 3U, write_buf.pos + 4U - 3U, u8_FFFE_Encode_buf);

        /* 将编码后的数据填入buf */
        memcpy((write_buf.buf + 3U), u8_FFFE_Encode_buf, u16_FFFE_Encode_length);
        /* 更新buf包长度 */
        write_buf.buf[2U] = (uint8_t) (u16_FFFE_Encode_length + 5U);
        write_buf.buf[u16_FFFE_Encode_length + 3U] = 0xFF;
        write_buf.buf[u16_FFFE_Encode_length + 4U] = 0xFD;

        /* 写入文件 */
        snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, s_File_Directory.ch_file_name);
        if (FMAppendWrite(full_path, (const void *) write_buf.buf, (u16_FFFE_Encode_length + 5U)) < 0)
        {
            LOG_E("%s append len %d error", full_path, (u16_FFFE_Encode_length + 5U));
            return;
        }

        ret = stat(full_path, &stat_l);
        if(ret < 0)
        {
            LOG_E("can not stat file '%s'.error code: 0x%08x", full_path, ret);
            return;
        }
        /* 文件大小 */
        s_File_Directory.u32_file_size = stat_l.st_size;
        FMWriteDirFile(file_manager.latest_dir_file_info.file_name, (const void *) &s_File_Directory,
                sizeof(SFile_Directory));

        /* 计算写入内容的CRC值 用于更新文件头内文件内容CRC值*/
        FileContent_CRC32 = CRC32CCITT((uint8_t *) write_buf.buf, u16_FFFE_Encode_length + 5U, FileContent_CRC32);

        memset(&write_buf, 0u, sizeof(WRITE_BUF));
        memset(u8_FFFE_Encode_buf, 0u, sizeof(u8_FFFE_Encode_buf));

        /* 置位写公共信息包标志 */
        u8_Gonggongxinxi_Flag = 1U;

//        SoftWare_Cycle_Flag = 0;//TODO(mingzhao) 原本一代代码中没有将该标志清零
//        LOG_I("clear SoftWare_Cycle_Flag %d", SoftWare_Cycle_Flag);
    }
    else
    {
//        LOG_I("no enter SoftWare_Cycle_Flag %d write_buf.pos %d", SoftWare_Cycle_Flag, write_buf.pos);
    }
}


/********************************************************************************************
** @brief: Get_FlashState
** @param: null
********************************************************************************************/
#if 0 //TODO(mingzhoa)


void Init_FlashState(void)  //使用FMInitLatestFile替代 TODO(mingzhao)
{
    /* 读取FRAM的0地址数据放入Flash_State; */
    FLASH_STATE flash_state = { 0u }, flash_state_old = { 0u };

    while (1u)
    {
        FM25V05_Manage_ReadData(0u, (uint8_t *) &flash_state, sizeof(FLASH_STATE));

        printf("init %x %x %x\r\n", flash_state.u32_init_flag, flash_state.u32_file_count,
                flash_state.u32_sector_count);

        if (memcmp(&flash_state_old, &flash_state, sizeof(FLASH_STATE)))
        {
            memcpy(&flash_state_old, &flash_state, sizeof(FLASH_STATE));
        }
        else
        {
            memcpy(&Flash_State, &flash_state, sizeof(FLASH_STATE));
            break;
        } /* end if...else */

        Wait(1u);
    } /* end while */

    /* 判断FLASH是否初始化 */
    if (Flash_State.u32_init_flag != 0x05555559u)
//  if ( Flash_State.u32_init_flag != 0x05555552u )
    {
        printf("init the flash\r\n");
        S25FL256S_EraseChip();

        Flash_State.u32_init_flag = 0x05555559u;
        Flash_State.u32_sector_count = 0u;
        Flash_State.u32_file_count = 0u;
        Flash_State.u32_fram_start_addr = FRAM_BASE_ADDR;
        Flash_State.u32_fram_write_addr = FRAM_BASE_ADDR - sizeof(SFile_Directory);
        Flash_State.u32_flash_start_addr = FLASH_RecordFile_BASE_ADDR;
        Flash_State.u32_flash_write_addr = FLASH_RecordFile_BASE_ADDR - SECTOR_SIZE;

        /* 更新FLASH状态到FRAM的0地址 */
        FM25V05_Manage_WriteEnable();
        FM25V05_Manage_WriteData(0u, (uint8_t *) &Flash_State, sizeof(FLASH_STATE));

        /* 初始化写buf */
        memset(&write_buf, 0u, sizeof(WRITE_BUF));
        FM25V05_Manage_WriteEnable();
        FM25V05_Manage_WriteData( FRAM_FlashBuffer_BASE_ADDR, (uint8_t *) &write_buf, sizeof(WRITE_BUF));

        uint8_t dat_buff[1024] = { 0 };
        uint32_t i = 0;
        /* 初始化文件目录 */
        for (i = 0; i < 62; i++)
        {
            FM25V05_Manage_WriteEnable();
            FM25V05_Manage_WriteData( FRAM_FileDirectory_BASE_ADDR + i * 1024, dat_buff, 1024);
        }
    }
    else
    {
        if (Flash_State.u32_file_count != 0u)
        {
            /* 得到最后记录的车次信息 */
            FM25V05_Manage_ReadData(Flash_State.u32_fram_write_addr, (uint8_t *) &s_File_Directory,
                    sizeof(SFile_Directory));
//          printf("最后记录司机:%d\r\n",(uint16_t)s_File_Directory.ch_siji[0]
//                                  + ((uint16_t)s_File_Directory.ch_siji[1]<<8)
//                                  + ((uint16_t)s_File_Directory.ch_siji[2]<<16)
//                                  + ((uint16_t)s_File_Directory.ch_siji[3]<<24));
//          printf("最后记录车次:%d\r\n",(uint16_t)s_File_Directory.ch_checi[0]
//                                  + ((uint16_t)s_File_Directory.ch_checi[1]<<8)
//                                  + ((uint16_t)s_File_Directory.ch_checi[2]<<16)
//                                  + ((uint16_t)s_File_Directory.ch_checi[3]<<24));
//          printf("最后本补状态：%x\r\n",s_File_Directory.ch_benbuzhuangtai[0]);

//            Flash_Addr = s_File_Directory.u32_start_addr + s_File_Directory.u32_page_count * PAGE_SIZE; //TODO(mingzhoa)
            printf("Flash_Addr is 0x%08x\r\n", Flash_Addr);
            /* 得到上次丢失的数据 */
            FM25V05_Manage_ReadData( FRAM_FlashBuffer_BASE_ADDR, (uint8_t *) &write_buf, sizeof(WRITE_BUF));
            printf("得到上次丢失的位置： 0x%08x\r\n", write_buf.pos);
        } /* end if */
    } /* end if...else */
}

#endif

/**************************************************************************************************
功能：判断是否生成记录文件
参数：无
返回：1--生成记录文件
      0--不生成记录文件
***************************************************************************************************/
static uint8_t Record_Condition_Judge(void)
{
    uint8_t judge_resault = 0u;

    /* 文件大小大于20MB */
    if ( RECORD_FILE_MAN_SIZE <= s_File_Directory.u32_file_size)
    {
        LOG_W("file > 20MB");
        judge_resault |= 1u << 0u;
    }

    /* 车次号或车次扩充发生变化 */
    if ((memcmp(s_File_Directory.ch_checi, &CHECI, 3u) || memcmp(s_File_Directory.ch_checikuochong, &CHECIKUOCHONG, 4u))
            && memcmp(nulldata, &CHECI, 3u))
    {
        LOG_W("Che Ci Hao Change");
        judge_resault |= 1u << 1u;
    }

    /* 司机号发生变化 */
    if (memcmp(s_File_Directory.ch_siji, &SIJI1, 3u) && memcmp(nulldata, &SIJI1, 3u))
    {
        LOG_W("Si Ji Hao Change");
        judge_resault |= 1u << 2u;
    }

//  /* 本补状态发生变化 */
//  if( memcmp( s_File_Directory.ch_benbuzhuangtai, &LIECHESHUXING, 1u ) 
//		&& memcmp( nulldata, &LIECHESHUXING, 1u )	)
//  {
//    printf("\r\n 本补状态发生变化！\r\n");
//    judge_resault |= 1u<<3u;
//  }
    return judge_resault;
}

/********************************************************************************************
** @brief: Init_FileDirectory
** @param: null
********************************************************************************************/
static rt_err_t Init_FileDirectory(S_CURRENT_FILE_INFO *current_file_info)
{
    if (NULL == current_file_info || NULL == current_file_info->file_dir)
    {
        return -RT_EEMPTY;
    }

    static uint8_t CheCi_Count1 = 0u, CheCi_Count2 = 0u, Create_Flag = 0u;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };
    sint32_t ret = 0;
    struct stat stat_l;

    /* 确认新文件生成标记，置位开始记录文件标志 */
    if (Record_Condition_Judge())
    {
        CheCi_Count1 += 1u;
        CheCi_Count2 = 0u;
        u8_Contant_Flag = 0u;

//        LOG_I("车次标志1计数：%d",CheCi_Count1);
        if (CheCi_Count1 >= 3u)
        {
            LOG_I("生成新文件");
            /* 生成新文件之前，判断上一个文件是否记录完整 */
#if 1
            if (write_buf.pos > 40u)
            {
                LOG_W("上一个文件记录尚不完整");
                /* 对数据包进行CRC校验 */
                RecordEventPkt_CRC32 = CRC32CCITT(write_buf.buf + 3U, write_buf.pos - 3U, 0xFFFFFFFF); //一包最大255字节，记录事项内容最大249字节(除去CRC324字节，最大245字节)
                /* 将校验值填入buf */
                memcpy(&(write_buf.buf[write_buf.pos]), &RecordEventPkt_CRC32, 4U);
                /* 对数据包内容进行FF-FE编码转换 */
                u16_FFFE_Encode_length = FFFEEncode(write_buf.buf + 3U, write_buf.pos + 4U - 3U, u8_FFFE_Encode_buf);

                /* 将编码后的数据填入buf */
                memcpy((write_buf.buf + 3U), u8_FFFE_Encode_buf, u16_FFFE_Encode_length);
                /* 更新buf包长度 */
                write_buf.buf[2U] = (uint8_t) (u16_FFFE_Encode_length + 5U);    //不能包含包头、包尾，否则长度等于256时，u8的长度为0
                write_buf.buf[u16_FFFE_Encode_length + 3U] = 0xFF;
                write_buf.buf[u16_FFFE_Encode_length + 4U] = 0xFD;

                s_File_Directory.u32_over_flag = 1u;

                snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, s_File_Directory.ch_file_name);
                if (FMAppendWrite(full_path, (const void *) write_buf.buf, (u16_FFFE_Encode_length + 5U)) < 0)
                {
                    LOG_E("%s write pkt len %d error", full_path, (u16_FFFE_Encode_length + 5U));
                }

                ret = stat(full_path, &stat_l);
                if(ret < 0)
                {
                    LOG_E("can not stat file '%s'.error code: 0x%08x", full_path, ret);
                    return;
                }
                /* 文件大小 */
                s_File_Directory.u32_file_size = stat_l.st_size;
                FMWriteDirFile(file_manager.latest_dir_file_info.file_name, (const void *) &s_File_Directory,
                        sizeof(SFile_Directory));

                /* 计算写入内容的CRC值 用于更新文件头内文件内容CRC值*/
                FileContent_CRC32 = CRC32CCITT((uint8_t *) write_buf.buf, write_buf.pos, FileContent_CRC32);

                memset(&write_buf, 0u, sizeof(WRITE_BUF));
                memset(u8_FFFE_Encode_buf, 0u, sizeof(u8_FFFE_Encode_buf));

                Update_FileHead();

                /* 需要更新文件头内容*/
                snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME,
                        current_file_info->file_dir->ch_file_name);

                current_file_info->fd = open(full_path, O_RDWR);
                if (current_file_info->fd < 0)
                {
                    LOG_E("open %s error, fd=%d", full_path, current_file_info->fd);
                    return -RT_ERROR;
                }

                ret = fm_modify_record_file_head(current_file_info);
                if (ret < 0)
                {
                    LOG_E("fm_modify_record_file_head error");
                    return -RT_ERROR;
                }
                close(current_file_info->fd);

                /* 置位写公共信息包标志 */
                u8_Gonggongxinxi_Flag = 1U;

            } /* end if */
#endif 
            /* 初始化write buf */
            memset(&write_buf, 0u, sizeof(WRITE_BUF));
            memset(&s_File_Directory, 0, sizeof(SFile_Directory));

            Create_Flag = 1u;
            CheCi_Count1 = 0u;
//            LOG_I("update checihao!!!!");
            memcpy(s_File_Directory.ch_checi, &CHECI, 3u);
            memcpy(s_File_Directory.ch_checikuochong, &CHECIKUOCHONG, 4u);
            memcpy(s_File_Directory.ch_siji, &SIJI1, 3u);
            memcpy(s_File_Directory.ch_benbuzhuangtai, &LIECHESHUXING, 1u);
        } /* end if */
    }
    /* 继续上一个文件记录 */
    else if (memcmp(nulldata, &CHECIHAO, 3u) && memcmp(nulldata, &SIJIHAO1, 3u))
    {
        CheCi_Count1 = 0u;
        CheCi_Count2 += 1u;
        Create_Flag = 0u;
        if (CheCi_Count2 >= 3u)
        {
            u8_Contant_Flag = 1u;
            CheCi_Count2 = 0u;
        } /* end if */
    }
    else
    {
        Create_Flag = 0u;
        u8_Contant_Flag = 0u;
        CheCi_Count1 = 0u;
        CheCi_Count2 = 0u;
    } /* end if...else if...else */

    /* 开始生成新的文件目录 */
    if (Create_Flag == 1u)
    {
        Create_Flag = 0u;
        file_manager.latest_dir_file_info.dir_num += 1u;
        LOG_I("start create new file dir num %d", file_manager.latest_dir_file_info.dir_num);
        if (file_manager.latest_dir_file_info.dir_num > FILE_MAX_NUM)    //目录个数
        {
            if (fm_free_space() < 0)
            {
                LOG_E("fm_free_space error");
            }
        } /* end if */

        /* 生成新目录 */
        memcpy(s_File_Directory.ch_date, &TIME_NYR, 3u);
        memcpy(s_File_Directory.ch_time, &TIME_SFM, 3u);

        /* 计数方式采用先加1后用的方式，避免数据丢失 */
        s_File_Directory.u32_over_flag = 0u;
        s_File_Directory.u32_file_size = sizeof(SFile_Head);
        s_File_Directory.file_id = file_manager.latest_dir_file_info.dir_num;

        LOG_I("data %d %d %d %d", s_File_Directory.ch_date[0], s_File_Directory.ch_date[1], s_File_Directory.ch_date[2],
                s_File_Directory.ch_date[3]);
        Get_FileName(&s_File_Directory);  //有返回值     生成目录与记录文件名

        /* 打印记录文件名与目录文件名 */
        LOG_I("file：%s dir: %s", s_File_Directory.ch_file_name, file_manager.latest_dir_file_info.file_name);

        /* 1.生成新的记录文件 */
        char full_path[PATH_NAME_MAX_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, s_File_Directory.ch_file_name);
        LOG_I("creat record file: %s", full_path);
        ret = create_file(full_path);
        if (ret < 0)
        {
            LOG_E("create record %s error, ret=%d", full_path, ret);
        }

        /* 2.生成新的目录文件 */
        memset(full_path, 0, sizeof(full_path));
        snprintf(full_path, sizeof(full_path), "%s/%s", DIR_FILE_PATH_NAME,
                file_manager.latest_dir_file_info.file_name);
        LOG_I("creat dir file: %s", full_path);
        ret = create_file(full_path);
        if (ret < 0)
        {
            LOG_E("create %s error, ret=%d", file_manager.latest_dir_file_info.file_name, ret);
        }

        /* 3.把最新的目录文件信息写入配置文件 */
        FMWriteLatestInfo(LATEST_DIR_NAME_FILE_PATH_NAME, NEW_DIR_FILE_NAME_CONF, (const void *) &file_manager.latest_dir_file_info, sizeof(S_LATEST_DIR_FILE_INFO));

        /* 4.写入目录信息 */
        FMWriteDirFile(file_manager.latest_dir_file_info.file_name, (const void *) &s_File_Directory,
                sizeof(SFile_Directory));

#if 0
        /* 打开目录文件 */
        sint32_t fd = 0;
        fd = open(full_path, O_RDWR);
        if (fd < 0)
        {
            LOG_E("open %s error, ret=%d", full_path, fd);
            return -RT_ERROR;
        }

        /* 写入目录信息 */
        ret = write(fd, (char *)&s_File_Directory, sizeof(SFile_Directory));
        fsync(fd);
        close(fd);
        if(ret < 0)
        {
            return -RT_ERROR;
        }
#endif

#if 0
        LOG_I("che ci %d %d %d %d", s_File_Directory.ch_checi[0], s_File_Directory.ch_checi[1], s_File_Directory.ch_checi[2], s_File_Directory.ch_checi[3]);
        LOG_I("kuo chong %d %d %d %d", s_File_Directory.ch_checikuochong[0], s_File_Directory.ch_checikuochong[1], s_File_Directory.ch_checikuochong[2], s_File_Directory.ch_checikuochong[3]);
        LOG_I("si ji %d %d %d %d", s_File_Directory.ch_siji[0], s_File_Directory.ch_siji[1], s_File_Directory.ch_siji[2], s_File_Directory.ch_siji[3]);
        LOG_I("data %d %d %d %d", s_File_Directory.ch_date[0], s_File_Directory.ch_date[1], s_File_Directory.ch_date[2], s_File_Directory.ch_date[3]);
        LOG_I("time %d %d %d %d", s_File_Directory.ch_time[0], s_File_Directory.ch_time[1], s_File_Directory.ch_time[2], s_File_Directory.ch_time[3]);
        LOG_I("over %d", s_File_Directory.u32_over_flag);
        LOG_I("size %d", s_File_Directory.u32_file_size);
        LOG_I("name %s", s_File_Directory.ch_file_name);
        LOG_I("benbu %d", s_File_Directory.ch_benbuzhuangtai[0]);
        LOG_I("reserve %d %d %d", s_File_Directory.ch_reserve[0], s_File_Directory.ch_reserve[1], s_File_Directory.ch_reserve[2]);
        LOG_I("id %d", s_File_Directory.file_id);
        LOG_I("save %d", s_File_Directory.is_save);
        LOG_I("------------------------------------------");

        SFile_Directory read_file_dir;

        fd = open(full_path, O_RDWR);
        if (fd < 0)
        {
            LOG_E("open %s error, ret=%d", full_path, fd);
            return -RT_ERROR;
        }

        read(fd, (char *)&read_file_dir, sizeof(SFile_Directory));
        close(fd);

        LOG_I("che ci %d %d %d %d", read_file_dir.ch_checi[0], read_file_dir.ch_checi[1], read_file_dir.ch_checi[2], read_file_dir.ch_checi[3]);
        LOG_I("kuo chong %d %d %d %d", read_file_dir.ch_checikuochong[0], read_file_dir.ch_checikuochong[1], read_file_dir.ch_checikuochong[2], read_file_dir.ch_checikuochong[3]);
        LOG_I("si ji %d %d %d %d", read_file_dir.ch_siji[0], read_file_dir.ch_siji[1], read_file_dir.ch_siji[2], read_file_dir.ch_siji[3]);
        LOG_I("data %d %d %d %d", read_file_dir.ch_date[0], read_file_dir.ch_date[1], read_file_dir.ch_date[2], read_file_dir.ch_date[3]);
        LOG_I("time %d %d %d %d", read_file_dir.ch_time[0], read_file_dir.ch_time[1], read_file_dir.ch_time[2], read_file_dir.ch_time[3]);
        LOG_I("over %d", read_file_dir.u32_over_flag);
        LOG_I("size %d", read_file_dir.u32_file_size);
        LOG_I("name %s", read_file_dir.ch_file_name);
        LOG_I("benbu %d", read_file_dir.ch_benbuzhuangtai[0]);
        LOG_I("reserve %d %d %d", read_file_dir.ch_reserve[0], read_file_dir.ch_reserve[1], read_file_dir.ch_reserve[2]);
        LOG_I("id %d", read_file_dir.file_id);
        LOG_I("save %d", read_file_dir.is_save);

#endif
        u8_FileHead_Flag = 1u;
        return RT_EOK;
    }
    else
    {
        u8_FileHead_Flag = 0u;
    } /* end if...else */
    return RT_EOK;
}

/**************************************************************************************************
功能：更新文件头信息到FRAM缓存区
参数：无
返回：无
***************************************************************************************************/
static void Update_FileHead(void)
{
    /* 获取文件头 */
    s_file_head.ch_head_flag[0] = 0xb1;
    s_file_head.ch_head_flag[1] = 0xf1;

    memcpy(s_file_head.ch_jilugeshibanbenhao, &JILUGESHIBANBEN, 4u);
    /* 设置外设类型值 */
    s_file_head.ch_waisheleixing[0] |= (LKJCHANGJIA << 0u);
    s_file_head.ch_waisheleixing[0] |= (TCMSCHANGJIA << 3u);
    s_file_head.ch_waisheleixing[0] |= ((ZHIDONGJICHANGJIA & 0x01) << 7u);
    s_file_head.ch_waisheleixing[1] |= ((ZHIDONGJICHANGJIA & 0x0E) << 8u);
    s_file_head.ch_waisheleixing[1] |= (LIEWEICHANGJIA << 11u);
//    LOG_I("外设类型：%x %x", s_file_head.ch_waisheleixing[0], s_file_head.ch_waisheleixing[1]);

    memcpy(s_file_head.ch_create_time, &TIME_NYR, 3u);
    memcpy(&s_file_head.ch_create_time[3], &TIME_SFM, 3u);
    s_file_head.ch_yunxingjiaoluhao[0] = (SHUJUJIAOLU & 0x1f);
    memcpy(&s_file_head.ch_yunxingjiaoluhao[2], &JIANKONGJIAOLU, 1u);
    s_file_head.ch_LKJfachefangxiang[0] = LKJFACHEFANGXIANG;
    s_file_head.ch_LKJfachefangxiang[1] = (SHUJUJIAOLU & 0x60) >> 5u;
    memcpy(s_file_head.ch_chezhan, &CHEZHANMING, 12u );
    memcpy(s_file_head.ch_jichexinghao, &JICHEXINGHAO, 2u);

#if 0
    switch(JICHELEIXING)
    {
        case 0:
        s_file_head.ch_jicheleixing[0] = 0x01;
        break;
        case 1:
        s_file_head.ch_jicheleixing[0] = 0x02;
        break;
        default:
        s_file_head.ch_jicheleixing[0] = 0xFF;
        break;
    }
#else
    s_file_head.ch_jicheleixing[0] = 0x02;
#endif

    memcpy(s_file_head.ch_jichehao, &JICHEHAO, 2u);
    memcpy(s_file_head.ch_juduanhao, &JUDUANHAO, 2u );
    memcpy(s_file_head.ch_chezhongbiaoshi, &CHEZHONGBIAOSHI, 4u);
    memcpy(s_file_head.ch_checihao, &CHECIHAO, 3u);

    s_file_head.ch_liecheshuxing[0] |= (( LIECHESHUXING ^ 1u) & 0x01) << 1u;
    if (LIANGSHU < 5u)
        s_file_head.ch_liecheshuxing[0] |= 0x01;
    else
        s_file_head.ch_liecheshuxing[0] &= 0xFE;
    memcpy(s_file_head.ch_siji1, &SIJIHAO1, 3u);
    memcpy(s_file_head.ch_siji2, &SIJIHAO2, 3u);
    memcpy(s_file_head.ch_zongzhong, &ZONGZHONG, 2u);
    memcpy(s_file_head.ch_jichang, &JICHANG, 2u);
    memcpy(s_file_head.ch_liangshu, &LIANGSHU, 1u);
    /* 填写设备状态值 */
    if (Get_CPU_Type() == CPU_A)
    {
        s_file_head.ch_shebeizhuangtai[0] |= 0x80;

        if ((0x01 == BENXIZHUANGTAI) || (0x03 == BENXIZHUANGTAI))
            s_file_head.ch_shebeizhuangtai[0] |= 0x01;
    }
    else
    {
        s_file_head.ch_shebeizhuangtai[0] &= 0x7F;

        if ((0x01 == BENXIZHUANGTAI) || (0x03 == BENXIZHUANGTAI))
            s_file_head.ch_shebeizhuangtai[0] |= 0x02;
    }
    s_file_head.ch_shebeizhuangtai[0] |= 0x04;
//    LOG_I("获取文件头设备状态：%x",s_file_head.ch_shebeizhuangtai[0]);
    switch (GONGZUOZHUANGTAI)
    {
        case 0x00:  //人工驾驶
            s_file_head.ch_gongzuozhuangtai[0] = 0x01;
            break;
        case 0x01:  //指导驾驶
            s_file_head.ch_gongzuozhuangtai[0] = 0x02;
            break;
        case 0x02:  //辅助预置驾驶
            s_file_head.ch_gongzuozhuangtai[0] = 0x03;
            break;
        case 0x03:  //辅助驾驶
            s_file_head.ch_gongzuozhuangtai[0] = 0x04;
            break;
        case 0x04:  //退出辅助驾驶
            s_file_head.ch_gongzuozhuangtai[0] = 0x05;
            break;
        default:
            break;
    }
    memcpy(s_file_head.ch_Ajikongzhiruanjianbanben, &AJIKONGZHIRUANJIANBANBEN, 4u);
    memcpy(s_file_head.ch_Bjikongzhiruanjianbanben, &BJIKONGZHIRUANJIANBANBEN, 4u);
    memcpy(s_file_head.ch_AjiSTOjichushujubanben, &AJISTOJICHUSHUJUBANBENRIQI, 2u);
    memcpy(&s_file_head.ch_AjiSTOjichushujubanben[2], &AJISTOJICHUSHUJUBIANYIRIQI, 2u);
    memcpy(s_file_head.ch_BjiSTOjichushujubanben, &BJISTOJICHUSHUJUBANBENRIQI, 2u);
    memcpy(&s_file_head.ch_BjiSTOjichushujubanben[2], &BJISTOJICHUSHUJUBIANYIRIQI, 2u);
    memcpy(s_file_head.ch_AjiSTOkongzhicanshu, &AJISTOKONGZHICANSHUBANBENRIQI, 2u);
    memcpy(&s_file_head.ch_AjiSTOkongzhicanshu[2], &AJISTOKONGZHICANSHUBIANYIRIQI, 2u);
    memcpy(s_file_head.ch_BjiSTOkongzhicanshu, &BJISTOKONGZHICANSHUBANBENRIQI, 2u);
    memcpy(&s_file_head.ch_BjiSTOkongzhicanshu[2], &BJISTOKONGZHICANSHUBIANYIRIQI, 2u);
    memcpy(s_file_head.ch_AjiLKJshujubanben, &AJILKJSHUJUBANBEN, 4u );
    memcpy(s_file_head.ch_BjiLKJshujubanben, &BJILKJSHUJUBANBEN, 4u );
    memcpy(s_file_head.ch_AjiLKJshujushijian, &AJILKJSHUJUSHIJIAN, 4u );
    memcpy(s_file_head.ch_BjiLKJshujushijian, &BJILKJSHUJUSHIJIAN, 4u );

    memcpy(s_file_head.ch_wenjianneirongCRC, &FileContent_CRC32, 4u);
    /* CRC校验 */
    s_file_head.u32_CRC32 = CRC32CCITT((uint8_t *) &s_file_head, sizeof(SFile_Head) - 2u, 0xFFFFFFFF);

#if 0  //TODO(mingzhao)  文件头在记录文件中存储  原来为什么要存在fm
    /* 将文件头内容缓存到FRAM */
    FM25V05_Manage_WriteEnable();
    FM25V05_Manage_WriteData( FRAM_FileHead_BASE_ADDR, (uint8_t *) &s_file_head, sizeof(SFile_Head));
#endif
}


/**************************************************************************************************
功能：创建文件文件头信息
参数：无
返回：无
***************************************************************************************************/
static rt_err_t Creat_FileHead(S_CURRENT_FILE_INFO *current_file_info)
{
    if (NULL == current_file_info || NULL == current_file_info->file_dir)
    {
        return -RT_EEMPTY;
    }

    char full_path[PATH_NAME_MAX_LEN] = { 0 };
    sint32_t ret = -1;

    if (u8_FileHead_Flag)
    {
        LOG_I("create file head");
        u8_FileHead_Flag = 0u;
#if 0  //TODO(mingzhoa)  可以把Init_FileDirectory中创建文件的程序放在这里，原来裸机是在这里创建的  这个函数100ms执行一次  Init_FileDirectory函数1s执行一次
        /* 生成新的记录文件 */
        snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, current_file_info->file_dir->ch_file_name);
        LOG_I("Creat_FileHead %s", full_path);
        ret = create_file(full_path);
        if (ret < 0)
        {
            LOG_E("create %s error, ret=%d", full_path, ret);
            return -RT_ERROR;
        }

        current_file_info->fd = open(full_path, (int)((uint32_t)O_CREAT | (uint32_t)O_RDWR | (uint32_t)O_TRUNC), (uint32_t)F_MODE);
        if(current_file_info->fd < 0)
        {
            LOG_E("open %s error, fd=%d", full_path, current_file_info->fd);
            return -RT_ERROR;
        }
#endif

        snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME,
                current_file_info->file_dir->ch_file_name);
        current_file_info->fd = open(full_path, O_RDWR);
        if (current_file_info->fd < 0)
        {
            LOG_E("open %s error, fd=%d", full_path, current_file_info->fd);
            return -RT_ERROR;
        }

        ret = fm_write_record_file_head(current_file_info);
        if (ret < 0)
        {
            LOG_E("fm_write_file_head error");
            return -RT_ERROR;
        }

        current_file_info->new_record_head_offset = lseek(current_file_info->fd, (off_t) 0, SEEK_CUR);
        close(current_file_info->fd);

        /* 置位文件体记录标志 */
        u8_Contant_Flag = 1u;
        u8_Gonggongxinxi_Flag = 1u;
        u8_Clear_Flag = 1u;
    } /* end if */
    return RT_EOK;
}

/**************************************************************************************************
功能：获取公共信息
参数：无
返回：无
***************************************************************************************************/
static void Get_Gonggongxinxi(void)
{
    /* 公共信息更新 */
    memcpy(s_file_public.ch_time, &TIME_SFM, 3u);
    memcpy(s_file_public.ch_juli, &JULI, 2u);
    memcpy(s_file_public.ch_licheng, &LICHENG, 3u);
#if 0
    memcpy( s_file_public.ch_jichexinhao, &JICHEXINHAO, 1u );
#else
    uint16_t u16_xinhaojizhuangtai = 0u;
    u16_xinhaojizhuangtai = (uint16_t) JICHEXINHAO + ((uint16_t) (*(&JICHEXINHAO + 1)) << 8u);

    switch (u16_xinhaojizhuangtai & 0xFF)
    {
        case 0x01:    //绿灯
            s_file_public.ch_jichexinhao[0] = 0x03;
            break;
        case 0x02:    //绿黄
            s_file_public.ch_jichexinhao[0] = 0x04;
            break;
        case 0x04:    //黄灯
            s_file_public.ch_jichexinhao[0] = 0x06;
            break;
        case 0x08:    //黄2灯
            s_file_public.ch_jichexinhao[0] = 0x08;
            break;
        case 0x10:    //双黄
            s_file_public.ch_jichexinhao[0] = 0x0B;
            break;
        case 0x20:    //红黄
            s_file_public.ch_jichexinhao[0] = 0x0D;
            break;
        case 0x40:    //红灯
            s_file_public.ch_jichexinhao[0] = 0x0E;
            break;
        case 0x80:    //白灯
            s_file_public.ch_jichexinhao[0] = 0x14;
            break;
        default:
            break;
    }

    static char last_ch_jichexinhao = 0;
    if (last_ch_jichexinhao != s_file_public.ch_jichexinhao[0])
    {
        LOG_I("change public xin xi ji che xin hao：%x", s_file_public.ch_jichexinhao[0]);
        last_ch_jichexinhao = s_file_public.ch_jichexinhao[0];
    }
#endif
    memcpy(s_file_public.ch_xiansu, &XIANSU, 2u);
    memcpy(s_file_public.ch_LKJsudu, &LKJSUDU, 2u);
    /* 工作状态 */
    switch (GONGZUOZHUANGTAI)
    {
        case 0x00:  //人工驾驶
            s_file_public.ch_gongzuozhuangtai[0] = 0x01;
            break;
        case 0x01:  //指导驾驶
            s_file_public.ch_gongzuozhuangtai[0] = 0x02;
            break;
        case 0x02:  //辅助预置驾驶
            s_file_public.ch_gongzuozhuangtai[0] = 0x03;
            break;
        case 0x03:  //辅助驾驶
            s_file_public.ch_gongzuozhuangtai[0] = 0x04;
            break;
        case 0x04:  //退出辅助驾驶
            s_file_public.ch_gongzuozhuangtai[0] = 0x05;
            break;
        default:
            break;
    }
    memcpy(s_file_public.ch_gongzuomoshi, &GONGZUOMOSHI, 1u);

    /* 系统状态 */
    if (1u == (JINGGAOBIAOZHI & 0xC0) >> 6u)   //I端有权
        s_file_public.ch_xitongzhuangtai[0] |= 0x01;
    else if (2u == (JINGGAOBIAOZHI & 0xC0) >> 6u)   //II端有权
        s_file_public.ch_xitongzhuangtai[0] |= 0x02;
    else
        s_file_public.ch_xitongzhuangtai[0] |= 0x00;

    if (Get_CPU_Type() == CPU_A)
    {
        s_file_public.ch_xitongzhuangtai[0] |= 0x40;   //I系记录

        if ((0x01 == BENXIZHUANGTAI) || (0x03 == BENXIZHUANGTAI))
            s_file_public.ch_xitongzhuangtai[0] |= 0x04;    //I系为主
    }
    else
    {
        s_file_public.ch_xitongzhuangtai[0] &= 0xBF;   //II系记录

        if ((0x01 == BENXIZHUANGTAI) || (0x03 == BENXIZHUANGTAI))
            s_file_public.ch_xitongzhuangtai[0] |= 0x08;    //II系为主
    }

    /* 机车发挥工况 */
    if ((0x03 == GONGZUOZHUANGTAI) || (0x04 == GONGZUOZHUANGTAI))   //自动驾驶
    {
        memcpy(s_file_public.ch_jichefahuishoubingjiwei, &JICHEFAHUISHOUBINGJIWEI, 1u);
        switch (JICHEFAHUIGONGKUANG)
        {
            case 0x01:
                s_file_public.ch_jichefahuigongkuang[0] = 0x01;
                break;
            case 0x02:
                s_file_public.ch_jichefahuigongkuang[0] = 0x00;
                break;
            case 0x04:
                s_file_public.ch_jichefahuigongkuang[0] = 0x02;
                break;
            default:
                s_file_public.ch_jichefahuigongkuang[0] = 0x07;
                break;
        }
    }
    else    //人工驾驶
    {
        memcpy(s_file_public.ch_jichefahuishoubingjiwei, &WULISHOUBINGJIWEI, 1u);
        switch (WULIJICHEGONGKUANG)
        {
            case 0x01:
                s_file_public.ch_jichefahuigongkuang[0] = 0x01;
                break;
            case 0x02:
                s_file_public.ch_jichefahuigongkuang[0] = 0x00;
                break;
            case 0x04:
                s_file_public.ch_jichefahuigongkuang[0] = 0x02;
                break;
            default:
                s_file_public.ch_jichefahuigongkuang[0] = 0x07;
                break;
        }
    }
    /* CCU速度 */
    s_file_public.ch_CCUsudu[0] = *(&CCUSUDU + 1u);
    s_file_public.ch_CCUsudu[1] = CCUSUDU;

    /* 牵引制动力 */
#if 0
    memcpy( s_file_public.ch_qianyinzhidong, &QIANYINZHIDONG, 2u );
#else
    uint16_t qianyinzhidongli = 0;
    if (1u == CHONGLIANCHE)
    {
        /* 处理牵引制动力数值 */
        qianyinzhidongli = (uint16_t) (axle1_jicheli_T37 & 0x7F) + (uint16_t) (axle2_jicheli_T38 & 0x7F)
                + (uint16_t) (axle3_jicheli_T39 & 0x7F) + (uint16_t) (axle4_jicheli_T40 & 0x7F)
                + (uint16_t) (axle5_jicheli_T41 & 0x7F) + (uint16_t) (axle6_jicheli_T42 & 0x7F);
        /* 处理牵引制动力类型 */
        if (axle1_jicheli_T37 & 0x7F)
        {
            qianyinzhidongli |= ((axle1_jicheli_T37 & 0x80) << 8u);
        }
        else if (axle2_jicheli_T38 & 0x7F)
        {
            qianyinzhidongli |= ((axle2_jicheli_T38 & 0x80) << 8u);
        }
        else if (axle3_jicheli_T39 & 0x7F)
        {
            qianyinzhidongli |= ((axle3_jicheli_T39 & 0x80) << 8u);
        }
        else if (axle4_jicheli_T40 & 0x7F)
        {
            qianyinzhidongli |= ((axle4_jicheli_T40 & 0x80) << 8u);
        }
        else if (axle5_jicheli_T41 & 0x7F)
        {
            qianyinzhidongli |= ((axle5_jicheli_T41 & 0x80) << 8u);
        }
        else if (axle6_jicheli_T42 & 0x7F)
        {
            qianyinzhidongli |= ((axle6_jicheli_T42 & 0x80) << 8u);
        }
        else
            qianyinzhidongli |= (0u << 15u);
    }
    else if (2u == CHONGLIANCHE)
    {
        /* 处理牵引制动力数值 */
        qianyinzhidongli = (uint16_t) (axle1_jicheli_T37 & 0x7F) + (uint16_t) (axle2_jicheli_T38 & 0x7F)
                + (uint16_t) (axle3_jicheli_T39 & 0x7F) + (uint16_t) (axle4_jicheli_T40 & 0x7F)
                + (uint16_t) (axle5_jicheli_T41 & 0x7F) + (uint16_t) (axle6_jicheli_T42 & 0x7F)
                + (uint16_t) (axle1_CLjicheli_T75 & 0x7F) + (uint16_t) (axle2_CLjicheli_T76 & 0x7F)
                + (uint16_t) (axle3_CLjicheli_T77 & 0x7F) + (uint16_t) (axle4_CLjicheli_T78 & 0x7F)
                + (uint16_t) (axle5_CLjicheli_T79 & 0x7F) + (uint16_t) (axle6_CLjicheli_T80 & 0x7F);

        /* 处理牵引制动力类型 */
        if (axle1_jicheli_T37 & 0x7F)
        {
            qianyinzhidongli |= ((axle1_jicheli_T37 & 0x80) << 8u);
        }
        else if (axle2_jicheli_T38 & 0x7F)
        {
            qianyinzhidongli |= ((axle2_jicheli_T38 & 0x80) << 8u);
        }
        else if (axle3_jicheli_T39 & 0x7F)
        {
            qianyinzhidongli |= ((axle3_jicheli_T39 & 0x80) << 8u);
        }
        else if (axle4_jicheli_T40 & 0x7F)
        {
            qianyinzhidongli |= ((axle4_jicheli_T40 & 0x80) << 8u);
        }
        else if (axle5_jicheli_T41 & 0x7F)
        {
            qianyinzhidongli |= ((axle5_jicheli_T41 & 0x80) << 8u);
        }
        else if (axle6_jicheli_T42 & 0x7F)
        {
            qianyinzhidongli |= ((axle6_jicheli_T42 & 0x80) << 8u);
        }
        else if (axle1_CLjicheli_T75 & 0x7F)
        {
            qianyinzhidongli |= ((axle1_CLjicheli_T75 & 0x80) << 8u);
        }
        else if (axle2_CLjicheli_T76 & 0x7F)
        {
            qianyinzhidongli |= ((axle2_CLjicheli_T76 & 0x80) << 8u);
        }
        else if (axle3_CLjicheli_T77 & 0x7F)
        {
            qianyinzhidongli |= ((axle3_CLjicheli_T77 & 0x80) << 8u);
        }
        else if (axle4_CLjicheli_T78 & 0x7F)
        {
            qianyinzhidongli |= ((axle4_CLjicheli_T78 & 0x80) << 8u);
        }
        else if (axle5_CLjicheli_T79 & 0x7F)
        {
            qianyinzhidongli |= ((axle5_CLjicheli_T79 & 0x80) << 8u);
        }
        else if (axle6_CLjicheli_T80 & 0x7F)
        {
            qianyinzhidongli |= ((axle6_CLjicheli_T80 & 0x80) << 8u);
        }
        else
            qianyinzhidongli |= (0u << 15u);
    }
    else
    {
        qianyinzhidongli = 1u;
    }

    memcpy(s_file_public.ch_qianyinzhidong, &qianyinzhidongli, 2u);
//    LOG_I("重联车标志：%x",CHONGLIANCHE);
//    LOG_I("牵引制动力记录值：%d",(s_file_public.ch_qianyinzhidong[0] + ((uint16_t)s_file_public.ch_qianyinzhidong[1] << 8)));
#endif

    s_file_public.ch_liecheguanyali[0] = (*(&LIECHEGUANYALI + 1));
    s_file_public.ch_liecheguanyali[1] = LIECHEGUANYALI;
    s_file_public.ch_zhidonggangyali[0] = (*(&ZHIDONGGANGYALI + 1));
    s_file_public.ch_zhidonggangyali[1] = ZHIDONGGANGYALI;
    s_file_public.ch_junfenggangyali[0] = (*(&JUNFENGGANGYALI + 1));
    s_file_public.ch_junfenggangyali[1] = JUNFENGGANGYALI;
    memcpy(s_file_public.ch_chaizhuandianliu, &CHAIZHUANDIANLIU, 2u);
}

/*****************************************************************************************
功能：同步更新公共信息与记录事项共有参数
参数：无
返回：无
******************************************************************************************/
static void Update_gongyoucanshu(void)
{
    /* 更新记录事项中共有参数 */
    memcpy(C_Lkjxiansu, &XIANSU, 2u);
    memcpy(C_Lkjsudu, &LKJSUDU, 2u);

    switch (GONGZUOZHUANGTAI)
    {
        case 0x00:  //人工驾驶
            C_gongzuozhuangtai = 0x01;
            break;
        case 0x01:  //指导驾驶
            C_gongzuozhuangtai = 0x02;
            break;
        case 0x02:  //辅助预置驾驶
            C_gongzuozhuangtai = 0x03;
            break;
        case 0x03:  //辅助驾驶
            C_gongzuozhuangtai = 0x04;
            break;
        case 0x04:  //退出辅助驾驶
            C_gongzuozhuangtai = 0x05;
            break;
        default:
            break;
    }
    C_gongzuomoshi = GONGZUOMOSHI;

    C_liecheguanyali[0] = (*(&LIECHEGUANYALI + 1));
    C_liecheguanyali[1] = LIECHEGUANYALI;
    C_zhidonggangyali[0] = (*(&ZHIDONGGANGYALI + 1));
    C_zhidonggangyali[1] = ZHIDONGGANGYALI;
    C_jungangyali[0] = (*(&JUNFENGGANGYALI + 1));
    C_jungangyali[1] = JUNFENGGANGYALI;
    memcpy(C_jichexinhao, &JICHEXINHAO, 2u);
}
/*****************************************************************************************
功能：组织公共信息包并写入buf
参数：无
返回：无
******************************************************************************************/
static void WriteGonggongxinxiPkt(void)
{
    /* 获取公共信息 */
    Get_Gonggongxinxi();

    /* 填包头标识 */
    write_buf.buf[0] = 0xFF;
    write_buf.buf[1] = 0xFE;
    /* 填包长 */
    write_buf.buf[2] = 0x2A;   //本包数据初始值，打包编码后更新该值
    /* 填公共信息内容 */
    write_buf.buf[3] = 0xD0;
    write_buf.buf[4] = 0x01;   //公共信息类别
    write_buf.buf[5] = 0x22;   //公共信息事项包长度

    memcpy(write_buf.buf + 6, &s_file_public, sizeof(SFile_Public));
//    LOG_I("write gong gong xin xi pos：%d", write_buf.pos);
//    LOG_I("gong gong time %d.%d.%d", s_file_public.ch_time[0], s_file_public.ch_time[1], s_file_public.ch_time[2]);
#if 0
    if(write_buf.pos == 0)
    write_buf.pos += 40U;
#endif		
}

/*******************************************************************************************
功能：判断公共信息是否发生变化
参数：无
返回：1 --> 公共信息发生变化
      0 --> 公共信息无变化
********************************************************************************************/
static uint8_t Init_GonggongxinxiState(void)
{
    uint8_t u8_GonggongxinxiState = 0u;
    static SFile_Public s_file_Public_old = { 0u };

    /* 获取当前公共信息 */
    Get_Gonggongxinxi();

    if (memcmp(&s_file_Public_old, &s_file_public, sizeof(SFile_Public)))    //公共信息发生变化
    {
        /* 更新公共信息内容 */
        memcpy(&s_file_Public_old, &s_file_public, sizeof(SFile_Public));
        u8_GonggongxinxiState = 1u;
    }
    else
    {
        u8_GonggongxinxiState = 0u;
    } /* end if...else */
    return u8_GonggongxinxiState;
}

/**********************************************************************************************
功能：组织记录事项包
参数：num1        --> 记录事项类别第一字节.
      num1        --> 记录事项类别第二字节.
      device_code --> 形成事件设备号.
      contant     --> 事件内容.
      lenth       --> 事件长度.
返回：无.
***********************************************************************************************/
static void WriteFileContantPkt(uint8_t num1, uint8_t num2, uint8_t device_code, uint8_t *contant, uint8_t lenth)
{
    static uint32_t rest_size = 255u;
    uint8_t contant_size = 0u;
    char file_contant[204u];
    char full_path[PATH_NAME_MAX_LEN] = { 0 };
    struct stat stat_l;
    int32_t ret = 0;

    file_contant[0u] = num1;
    file_contant[1u] = num2;
    file_contant[2u] = lenth;
    file_contant[3u] = device_code;

    LOG_I("记录事项代码：%x %x", num1, num2);
    memcpy(file_contant + 4U, contant, lenth);

    /* 放入缓冲区 */
    rest_size = 255U - 2 - 4 - write_buf.pos - 1;    //256字节减去包尾标识2字节、CRC32校验值4字节及公共信息包长度40字节

    contant_size = lenth + 4u;

    while (1u)
    {
//        LOG_I("rest_size %d contant_size %d", rest_size, contant_size);
        if (rest_size >= contant_size)
        {
            memcpy(&(write_buf.buf[write_buf.pos]), &file_contant[lenth + 4U - contant_size], contant_size);
//            LOG_I("写记录事项位置：%d", write_buf.pos);
            write_buf.pos += contant_size;
            rest_size -= contant_size;

            if(fm_free_fram_space(&file_manager) < 0)
            {
                LOG_E("fm_free_fram_space error");
            }
            else
            {
                if(FMWriteTmpFile(&file_manager, (const void *)&write_buf, sizeof(WRITE_BUF)) < 0)
                {
                    LOG_E("write record pos %d", write_buf.pos);
                }
            }
//            LOG_I("写记录事项222位置：%d   剩余空间：%d   内容长度：%d", write_buf.pos, rest_size, contant_size);
            break;
        }
        else /* 缓存区存满 */
        {
            Update_gongyoucanshu();
            /* 添加数据包头及公共信息 */
            WriteGonggongxinxiPkt();
            /* 对数据包进行CRC校验 */
            RecordEventPkt_CRC32 = CRC32CCITT(write_buf.buf + 3U, write_buf.pos - 3U, 0xFFFFFFFF); //一包最大255字节，记录事项内容最大249字节(除去CRC324字节，最大245字节)
            /* 将校验值填入buf */
            memcpy(&(write_buf.buf[write_buf.pos]), &RecordEventPkt_CRC32, 4U);
            //      printf( "编码前数据长度： %d\r\n", write_buf.pos+4 );
            /* 对数据包内容进行FF-FE编码转换 */
            u16_FFFE_Encode_length = FFFEEncode(write_buf.buf + 3U, write_buf.pos + 4U - 3U, u8_FFFE_Encode_buf);

            /* 将编码后的数据填入buf */
            memcpy((write_buf.buf + 3U), u8_FFFE_Encode_buf, u16_FFFE_Encode_length);
            /* 更新buf包长度 */
            write_buf.buf[2U] = (uint8_t) (u16_FFFE_Encode_length + 5U); //不能包含包头、包尾，否则长度等于256时，u8的长度为0   5 = 包头 2 + 长度 + 1 + 包尾 2
            write_buf.buf[u16_FFFE_Encode_length + 3U] = 0xFF;
            write_buf.buf[u16_FFFE_Encode_length + 4U] = 0xFD;
//            LOG_I("FFFE_Encode_length： %d", u16_FFFE_Encode_length);

            snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, s_File_Directory.ch_file_name);
            if (FMAppendWrite(full_path, (const void *) write_buf.buf, (u16_FFFE_Encode_length + 5U)) < 0)
            {
                LOG_E("%s write pkt len %d error", full_path, (u16_FFFE_Encode_length + 5U));
            }

            ret = stat(full_path, &stat_l);
            if(ret < 0)
            {
                LOG_E("can not stat file '%s'.error code: 0x%08x", full_path, ret);
                return;
            }
            /* 文件大小 */
            s_File_Directory.u32_file_size = stat_l.st_size;
            FMWriteDirFile(file_manager.latest_dir_file_info.file_name, (const void *) &s_File_Directory,
                    sizeof(SFile_Directory));

            /* 计算写入内容的CRC值 用于更新文件头内文件内容CRC值*/
            FileContent_CRC32 = CRC32CCITT((uint8_t *) write_buf.buf, u16_FFFE_Encode_length + 5U, FileContent_CRC32);

            memset(&write_buf, 0u, sizeof(WRITE_BUF));
            memset(u8_FFFE_Encode_buf, 0u, sizeof(u8_FFFE_Encode_buf));

            /*write_buf.pos=0;*/
            rest_size = 255U - 2 - 4 - 40 - 1;
            write_buf.pos = 40U;
            /* 置位写公共信息包标志 */
            u8_Gonggongxinxi_Flag = 1U;
        } /* end if...else */
    } /* end while */
}

/**********************************************************************************************
功能：获取记录事件
参数：无
返回：无
***********************************************************************************************/
static void Get_FileContant(void)
{
    if (u8_Contant_Flag)
    {
        static uint32_t time_count = 0u;

#if 0		
        /* 组织公共信息包并写入FRAM */
        if( u8_Gonggongxinxi_Flag )
        {
            WriteGonggongxinxiPkt();
            u8_Gonggongxinxi_Flag = 0U;
        }
#else
        if (write_buf.pos == 0)
            write_buf.pos += 40U;
#endif    

        /* clear check flag */
        if (u8_Clear_Flag)
        {
            LOG_I("clear check flag");

            /* 版本信息 */
            RecordingVersionMessage();

            /* 插件自检信息 */
            RecordingSelfCheckMessage();

            u8_Clear_Flag = 0U;
        } /* end if */
        /* 司机操作信息*/
        RecordingDriverOperationMessage();

        /* STO行程规划信息 */
        RecordingFormPlanningMessage();

        /* 获取STO控车信息 */
        RecordingTrainControlMessage();

        /* 获取机车制动系统信息 */
        RecordingLocoBrakeMessage();

        /* 获取机车牵引系统信息 */
        RecordingLocoDrawnMessage();

        /* 获取LKJ系统信息 */
        RecordingLKJSystemMessage();

        /* 列车运行信息 */
        RecordingTrainOperationMessage();

        /* 开关机时间 */
        RecordingSwitchTimeMessage();

        /* 以下信息1S检测一次 */
        if (Common_BeTimeOutMN(&time_count, 1000u))
        {
            /* 版本信息 */
            RecordingVersionMessage();
            /* 插件自检信息 */
            RecordingSelfCheckMessage();
        } /* end if */

    } /* end if */
}


/**********************************************
功能：获取文件名
参数：addr  -->  文件地址
返回：无
***********************************************/
#if 1
/*
 *
 * 需要保证上一个文件大于等于20M或没有文件，才能调用该接口创建文件*/
static void Get_FileName(SFile_Directory *directory)
{
//    char filename[FILE_NAME_MAX_NUM] = {0};

    /* 记录文件名：车次_司机号_日期_序号 */
    /* 日期：年月日 */
//    snprintf(directory->ch_file_name, FILE_NAME_MAX_NUM,
//            "%d%d%d%d_%d%d%d%d_%d%d%d_%d",
//            directory->ch_checi[0], directory->ch_checi[1], directory->ch_checi[2], directory->ch_checi[3],
//            directory->ch_siji[0], directory->ch_siji[1], directory->ch_siji[2], directory->ch_siji[3],
//            directory->ch_date[0], directory->ch_date[1], directory->ch_date[2],
//            directory->file_id);

    snprintf(directory->ch_file_name, FILE_NAME_MAX_NUM,
            "%d%d%d%d_%d%d%d_%d.DSW",
            directory->ch_checi[0], directory->ch_checi[1], directory->ch_checi[2], directory->ch_checi[3],
            directory->ch_date[0], directory->ch_date[1], directory->ch_date[2],
            directory->file_id);

    /* 目录文件    dir_车次_日期_序号 */
    snprintf(file_manager.latest_dir_file_info.file_name, FILE_NAME_MAX_NUM,
            "dir_%d%d%d%d_%d%d%d_%d",
            directory->ch_checi[0], directory->ch_checi[1], directory->ch_checi[2], directory->ch_checi[3],
            directory->ch_date[0], directory->ch_date[1], directory->ch_date[2],
            directory->file_id);
    memcpy(directory->ch_dir_name, file_manager.latest_dir_file_info.file_name, FILE_NAME_MAX_NUM);

    LOG_I("dir: %s record: %s", file_manager.latest_dir_file_info.file_name, directory->ch_file_name);
}

#else
static sint32_t Get_FileName(SFile_Directory *directory )
{
    uint32_t i = 0u, j = 0u;
    uint32_t num, divisor = 0u;
    uint8_t u8_lastfilename_num = 0u;
    sint32_t ret = 0;

    SFile_Directory s_lastfile_directory = { 0U };

    /* 车次扩充 */
    for (i = 0u; i < 4u; i++)
    {
        if (directory->ch_checikuochong[i] != 0x20u \
                && directory->ch_checikuochong[i] != 0x00u)
        {
            directory->ch_file_name[j] = directory->ch_checikuochong[i];
            j++;
        } /* end if */
    } /* end for */
    /* 车次号 */
    memcpy(&num, directory->ch_checi, 4u);

    if (num >= 100000u)
    {
        num = 99999u;
    } /* end if */

    if (num >= 10000u)
    {
        i = 5u;
        divisor = 10000u;
    }
    else if (num >= 1000u)
    {
        i = 4u;
        divisor = 1000u;
    }
    else if (num >= 100u)
    {
        i = 3u;
        divisor = 100u;
    }
    else if (num >= 10u)
    {
        i = 2u;
        divisor = 10u;
    }
    else
    {
        i = 1u;
        divisor = 1u;
    } /* end if...else if......else */

    for (; i > 0u; i--)
    {
        directory->ch_file_name[j] = num / divisor + 48u;    //将十进制数字转换为ASCII码字符
        num = num % divisor;
        j++;
        divisor = divisor / 10u;
    } /* end for */

    /* 司机号 */
    memcpy(&num, directory->ch_siji, 4u);

    if (num >= 10000000u)
    {
        num = 9999999u;
    } /* end if */

    if (num >= 1000000u)
    {
        i = 7u;
        divisor = 1000000u;
    }
    else if (num >= 100000u)
    {
        i = 6u;
        divisor = 100000u;
    }
    else if (num >= 10000u)
    {
        i = 5u;
        divisor = 10000u;
    }
    else if (num >= 1000u)
    {
        i = 4u;
        divisor = 1000u;
    }
    else if (num >= 100u)
    {
        i = 3u;
        divisor = 100u;
    }
    else if (num >= 10u)
    {
        i = 2u;
        divisor = 10u;
    }
    else
    {
        i = 1u;
        divisor = 1u;
    } /* end if...else if......else */

    for (; i > 0u; i--)
    {
        directory->ch_file_name[j] = num / divisor + 48u;    //将十进制数字转换为ASCII码字符
        num = num % divisor;
        j++;
        divisor = divisor / 10u;
    } /* end for */

#if 0    //TODO(mingzhoa)   这段代码会导致出现EOT
    /* 本补状态占用本字节高7位 */
#if 0
    directory->ch_file_name[ j ] = LIECHESHUXING << 1U;
#else
    directory->ch_file_name[j] = (directory->ch_benbuzhuangtai[0] & 0x03) << 1U;
//    LOG_I("num1:%s  %d", directory->ch_benbuzhuangtai, directory->ch_benbuzhuangtai);
//    LOG_I("num2:%s  %d", (directory->ch_benbuzhuangtai[0] & 0x03), (directory->ch_benbuzhuangtai[0] & 0x03));
//    LOG_I("num3:%s", directory->ch_file_name);
#endif
#endif
    LOG_I("ch file name %s", directory->ch_file_name);
    /* 还没有目录文件 */
    if(0 == strcmp(file_manager.latest_dir_file_info.file_name, FIRST_LATEST_DIR_NAME_NULL))
    {
        snprintf(file_manager.latest_dir_file_info.file_name, FILE_NAME_MAX_NUM, "%s_dir_%d", directory->ch_file_name, directory->file_id);
        LOG_I("dir name %s", file_manager.latest_dir_file_info.file_name);
    }
    else
    {
        /* 文件序号 */
        if(FMReadDirFile(file_manager.latest_dir_file_info.file_name, &s_lastfile_directory) < 0)
        {
            return -1;
        }

        /* 如果上个文件大于等于20M，则本文件名在上一个的基础上加序号 */
        if( s_lastfile_directory.u32_file_size >= RECORD_FILE_MAN_SIZE )
        {
            for (i = 0u; i < 24u; i++)
            {
                if ('.' == s_lastfile_directory.ch_file_name[i])
                {
                    u8_lastfilename_num = ((s_lastfile_directory.ch_file_name[i - 3] - 48u) & 0x01) * 100
                            + (s_lastfile_directory.ch_file_name[i - 2] - 48u) * 10
                            + (s_lastfile_directory.ch_file_name[i - 1] - 48u) * 1;
                    break;
                }
            }

            /* 如果上个文件大于等于20MB，则读取上一个文件的文件名，在上个文件名基础上序号加1 */
            if (s_lastfile_directory.u32_file_size >= RECORD_FILE_MAN_SIZE)
            {
                u8_lastfilename_num += 1u;
                /* 文件序号 */
                directory->file_id = u8_lastfilename_num;
                directory->ch_file_name[j] = directory->ch_file_name[j] | (u8_lastfilename_num / 100) + 48u;
                j++;
                directory->ch_file_name[j++] = (u8_lastfilename_num % 100) / 10 + 48u;
                directory->ch_file_name[j++] = (u8_lastfilename_num % 10) / 1 + 48u;
            }
            else
            {
                /* 文件序号 */
                directory->ch_file_name[j] = directory->ch_file_name[j] | 0u + 48u;
                j++;
                directory->ch_file_name[j++] = 0u + 48u;
                directory->ch_file_name[j++] = 1u + 48u;
            }

            /* 后缀符号‘.’ */
            directory->ch_file_name[j++] = '.';
            /* 日期 */
        //  directory->ch_file_name[ j++ ] = directory->ch_date[ 0u ] / 10u + 48u;
        //  directory->ch_file_name[ j++ ] = directory->ch_date[ 0u ] % 10u + 48u;
            directory->ch_file_name[j++] = directory->ch_date[1u] / 10u + 48u;
            directory->ch_file_name[j++] = directory->ch_date[1u] % 10u + 48u;
            directory->ch_file_name[j++] = directory->ch_date[2u] / 10u + 48u;
            directory->ch_file_name[j++] = directory->ch_date[2u] % 10u + 48u;

            if (j < 24u)
            {
                for (i = j; i < 24u; i++)
                {
                    directory->ch_file_name[i] = 0u;
                } /*  end for */
            } /* end if */

            /* 更新目录的文件名 */
            snprintf(file_manager.latest_dir_file_info.file_name, FILE_NAME_MAX_NUM, "%s_dir_%d", directory->ch_file_name, directory->file_id);
            LOG_I("dir name %s", file_manager.latest_dir_file_info.file_name);
        }
    }
    return 0;
}
#endif

/**********************************************
***********************************************
功能：获取司机操作信息
参数：无
返回：无
***********************************************/
static void RecordingDriverOperationMessage( void )
{
  /* DMI操作信息 */
  RecordingDMIOperationMessage();
  /* DDU操作信息 */
  RecordingDDUOperationMessage();
  /* L等级信息 */
  RecordingLLevelMessage();
  /* 速度下浮值信息 */
  RecordingSpeedDownMessage();
  /* 关门车信息 */
  RecordingHoldOutVehicleMessage();
  /* 换算闸瓦压力信息 */
  RecordingBrakeshoePressureMessage();
  /* 隔离STO信息 */
  RecordingIsolateSTOMessage();
	
} /* end function RecordingDriverOperationMessage */

/**********************************************
功能：记录DMI操作信息
参数：无
返回：无
***********************************************/
static void RecordingDMIOperationMessage(void)
{
    static uint8_t C_contant_datI[5] = { 0 };
    static uint8_t C_contant_datII[5] = { 0 };

    if (1u == ((JINGGAOBIAOZHI & 0xC0) >> 6u))   //I端有权
    {
        if ( MINGLINGHAOI && (0x01 != MINGLINGHAOI))
        {
//            LOG_I("...I端显示器命令号变化\r\n");
            C_contant_datI[0] = MINGLINGHAOI;
            memcpy(&C_contant_datI[1], &MINGLINGNEIRONGI, 4U);
            WriteFileContantPkt(0xA0, 0x01, 0x21, C_contant_datI, 5);
            MINGLINGHAOI = 0u;
        }
    }
    else if (2u == ((JINGGAOBIAOZHI & 0xC0) >> 6u))   //II端有权
    {
        if ( MINGLINGHAOII && (0x01 != MINGLINGHAOII))
        {
//            LOG_I("...II端显示器命令号变化：%x\r\n", MINGLINGHAOII);
            C_contant_datII[0] = MINGLINGHAOII;
            memcpy(&C_contant_datII[1], &MINGLINGNEIRONGII, 4U);
            WriteFileContantPkt(0xA0, 0x01, 0x22, C_contant_datII, 5);
            MINGLINGHAOII = 0u;
        }
    }
    else
    {
        //do nothing
    }
} /* end function RecordingDMIOperationMessage */

/**********************************************
功能：记录DDU操作信息
参数：无
返回：无
***********************************************/
static void RecordingDDUOperationMessage( void )
{
// static uint8_t C_DDUcaozuo[5] = {0};

//  if(MINGLINGHAO)
//  {
//    C_DDUcaozuo[0] = MINGLINGHAO;
//    memcpy( &C_DDUcaozuo[1], &MINGLINGNEIRONG, 4U);

//    WriteFileContantPkt( 0xA0, 0x02, g_XSQ_DevCode, C_DDUcaozuo, 5 );    
//  } 
} /* end function RecordingDDUOperationMessage */

/**********************************************
功能：记录L等级信息
参数：无
返回：无
***********************************************/
void RecordingLLevelMessage(void)
{
    static uint8_t C_Ldengji[2] = { 0U };

    if (C_Ldengji[1] != LDENGJI)
    {
        C_Ldengji[0] = LDENGJI;
//        LOG_I("L等级值：%x", LDENGJI);
        WriteFileContantPkt(0xA0, 0x03, g_ZK_DevCode, C_Ldengji, 2U);
        C_Ldengji[1] = LDENGJI;
    }
} /* end function RecordingLLevelMessage */

/**********************************************
功能：记录速度下浮值信息
参数：无
返回：无
***********************************************/
void RecordingSpeedDownMessage( void )
{
  static uint8_t C_Suduxiafuzhi[2] = { 0U };

  if( C_Suduxiafuzhi[1] != SUDUXIAFUZHI )
  {
    C_Suduxiafuzhi[0] = SUDUXIAFUZHI;
    WriteFileContantPkt( 0xA0, 0x04, g_ZK_DevCode, C_Suduxiafuzhi, 2U );
    C_Suduxiafuzhi[1] = SUDUXIAFUZHI;    
  } 
} /* end function RecordingSpeedDownMessage */

/**********************************************
功能：记录关门车信息
参数：无
返回：无
***********************************************/
static void RecordingHoldOutVehicleMessage(void)
{
    static uint8_t C_Guanmencheshuliang[2] = { 0U };

    if (C_Guanmencheshuliang[1] != GUANMENCHE)
    {
        C_Guanmencheshuliang[0] = GUANMENCHE;
//        LOG_I("关门车：%x", GUANMENCHE);
        WriteFileContantPkt(0xA0, 0x05, g_ZK_DevCode, C_Guanmencheshuliang, 2U);
        C_Guanmencheshuliang[1] = GUANMENCHE;
    }
} /* end function RecordingHoldOutVehicleMessage */

/**********************************************
功能：记录换算闸瓦压力信息
参数：无
返回：无
***********************************************/
static void RecordingBrakeshoePressureMessage(void)
{
    static uint8_t C_Zawayali[4] = { 0U };

    if (memcmp(&C_Zawayali[2], &HUANSUANZHAWAYALI, 2U))
    {
        memcpy(C_Zawayali, &HUANSUANZHAWAYALI, 2U);
//        LOG_I("闸瓦压力：%x %x", C_Zawayali[0], C_Zawayali[1]);
        WriteFileContantPkt(0xA0, 0x06, g_ZK_DevCode, C_Zawayali, 4u);

        memcpy(&C_Zawayali[2], &HUANSUANZHAWAYALI, 2U);
    } /* end if */
} /* end function RecordingBrakeshoePressureMessage */

/**********************************************
功能：记录隔离STO信息
参数：无
返回：无
***********************************************/
static void RecordingIsolateSTOMessage(void)
{
    static uint8_t C_geliSTO = 0U;

    if (C_geliSTO != GELISTO)
    {
//        LOG_I("...生成隔离STO事项...");
        C_geliSTO = GELISTO;
        WriteFileContantPkt(0xA0, 0x07, g_ZK_DevCode, &C_geliSTO, 1u);
    } /* end if */

} /* end function RecordingIsolateSTOMessage */


/**********************************************
***********************************************
功能：获取STO行程规划信息
参数：无
返回：无
***********************************************/
static void RecordingFormPlanningMessage( void )
{
  /* 指导速度信息 */
  RecordingGuidSpeedMessage();
  /* 指导级位信息 */
  RecordingGuidLevelMessage();
	/* 指导工况信息 */
  RecordingGuidConditonMessage();
  /* 指导限速信息 */
  RecordingGuidSpeedLimitMessage();
  /* 开始优化信息 */
  RecordingStartOptimizeMessage();
  /* 优化结果信息 */
  RecordingOptimizeResultMessage();
  /* 优化不可控区信息 */
  RecordingOptimizeUncontrAreaMessage();
  /* 早晚点信息 */
  RecordingSoonerAndLaterMessage();
} /* end function RecordingFormPlanningMessage */

/**********************************************
功能：获取指导速度信息
参数：无
返回：无
***********************************************/
static void RecordingGuidSpeedMessage(void)
{
    static uint8_t C_zhidaosudu[2] = { 0x00U, 0x00U };
    uint16_t zhidaosudu_New = 0u, zhidaosudu_Old = 0u;

    zhidaosudu_New = (uint16_t) (*(&ZHIDAOSUDU + 1)) + (uint16_t) (ZHIDAOSUDU << 8);
    zhidaosudu_Old = (uint16_t) C_zhidaosudu[0] + (uint16_t) (C_zhidaosudu[1] << 8);

//    LOG_I("指导速度：%d----- %d\r\n", zhidaosudu_New, zhidaosudu_Old);
    /* 判断指导速度变化是否大于等于0.1km/h */
    if (1U <= abs(zhidaosudu_New - zhidaosudu_Old))
    {
        C_zhidaosudu[0] = *(&ZHIDAOSUDU + 1);
        C_zhidaosudu[1] = ZHIDAOSUDU;
//        LOG_I("指导速度：%d", (C_zhidaosudu[0] + ((uint16_t )C_zhidaosudu[1] << 8)));
        WriteFileContantPkt(0xA1, 0x01, g_ZK_DevCode, C_zhidaosudu, 2u);
    } /* end if */
} /* end function RecordingGuidSpeedMessage */

/**********************************************
功能：获取指导工况信息
参数：无
返回：无
***********************************************/
static void RecordingGuidConditonMessage(void)
{
    static uint8_t C_zhidaogongkuang = 0U;
    uint8_t zhidaogongkuangzhi = 0u;

    zhidaogongkuangzhi = ZHIDAOGONGKUANG;
    if (memcmp(&C_zhidaogongkuang, &ZHIDAOGONGKUANG, 1U))
    {
        switch (zhidaogongkuangzhi)
        {
            case 1:
                C_zhidaogongkuang = 0x01;
                break;
            case 2:
                C_zhidaogongkuang = 0x00;
                break;
            case 3:
                C_zhidaogongkuang = 0x02;
                break;
            default:
                C_zhidaogongkuang = 0xff;
                break;
        }
//        LOG_I("指导工况：%d",C_zhidaogongkuang);
        WriteFileContantPkt(0xA1, 0x02, g_ZK_DevCode, &C_zhidaogongkuang, 1u);
        memcpy(&C_zhidaogongkuang, &ZHIDAOGONGKUANG, 1U);
    } /* end if */
} /* end function RecordingGuidConditonMessage */

/**********************************************
功能：获取指导级位信息
参数：无
返回：无
***********************************************/
static void RecordingGuidLevelMessage(void)
{
    static uint8_t C_zhidaojiwei = 0U;

    /* 判断指导级位是否发生变化 */
    if (C_zhidaojiwei != ZHIDAOJIWEI)
    {
//        LOG_I("指导级位：%d  %d", C_zhidaojiwei, ZHIDAOJIWEI);
        C_zhidaojiwei = ZHIDAOJIWEI;
        WriteFileContantPkt(0xA1, 0x03, g_ZK_DevCode, &C_zhidaojiwei, 1u);
    } /* end if */
} /* end function RecordingGuidLevelMessage */

/**********************************************
功能：获取指导限速信息
参数：无
返回：无
***********************************************/
static void RecordingGuidSpeedLimitMessage(void)
{
    static uint8_t C_zhidaoxiansu[2] = { 0x00U, 0x00U };

    /* 判断指导限速是否变化 */
    if (memcmp(C_zhidaoxiansu, &ZHIDAOXIANSU, 2U))
    {
        memcpy(C_zhidaoxiansu, &ZHIDAOXIANSU, 2U);
//        LOG_I("指导限速：%x %x", C_zhidaoxiansu[0], C_zhidaoxiansu[1]);
        WriteFileContantPkt(0xA1, 0x04, g_ZK_DevCode, C_zhidaoxiansu, 2u);
    } /* end if */
} /* end function RecordingGuidSpeedLimitMessage */

/**********************************************
功能：获取开始优化信息
参数：无
返回：无
***********************************************/
static void RecordingStartOptimizeMessage(void)
{
    static uint8_t C_kaishiyouhua = 0U;
    uint8_t youhuabiaozhi = 0u;

    youhuabiaozhi = ((*(&YOUHUAJIEGUO + 1u)) & 0xc0) >> 6u;
    if ((C_kaishiyouhua != youhuabiaozhi) && (0x01 == youhuabiaozhi))
    {
//        LOG_I("...生成开始优化事项...");
        WriteFileContantPkt(0xA1, 0x05, g_ZK_DevCode, &C_kaishiyouhua, 0u);
    } /* end if */
    C_kaishiyouhua = youhuabiaozhi;
} /* end function RecordingStartOptimizeMessage */

/**********************************************
功能：获取优化结果信息
参数：无
返回：无
***********************************************/
static void RecordingOptimizeResultMessage( void )
{
    static uint8_t C_youhuajieguo[4] = { 0U };
    uint8_t youhuabiaozhi = 0u;
    uint8_t youhuajieguo = 0u;

    youhuabiaozhi = ((*(&YOUHUAJIEGUO + 1u)) & 0xc0) >> 6u;
    youhuajieguo = ((*(&YOUHUAJIEGUO + 1u)) & 0x30) >> 4u;

    if ((C_youhuajieguo[3] != youhuabiaozhi) && (2u == youhuabiaozhi))
    {
//        LOG_I("...生成优化结果事项...优化结果：%x", youhuajieguo);
//        LOG_I("优化耗时：%d", YOUHUAHAOSHI + ((*(&YOUHUAHAOSHI+1)) << 8));
        C_youhuajieguo[0] = youhuajieguo;
        if (0x01 == youhuajieguo)  //优化成功
            memcpy(&C_youhuajieguo[1], &YOUHUAHAOSHI, 2U);
        else
            memset(&C_youhuajieguo[1], 0u, 2U);
        WriteFileContantPkt(0xA1, 0x06, g_ZK_DevCode, C_youhuajieguo, 3u);
    } /* end if */
    C_youhuajieguo[3] = youhuabiaozhi;
} /* end function RecordingOptimizeResultMessage */

/**********************************************
功能：获取优化不可控区信息
参数：无
返回：无
***********************************************/
static void RecordingOptimizeUncontrAreaMessage( void )
{

} /* end function RecordingOptimizeUncontrAreaMessage */

/**********************************************
功能：获取早晚点信息
参数：无
返回：无
***********************************************/
static void RecordingSoonerAndLaterMessage( void )
{
    static uint8_t C_zaowandian[4] = { 0x00, 0x00U, 0x00U, 0x00U };
    static uint8_t guozhanzhongxinzhi = 0u;
    uint8_t zaowandianbiaozhizhi = 0x00;
    uint32_t biaozhunshijian = 0u, yujishijian = 0u, zaowandianshijian = 0u;

    memcpy(&biaozhunshijian, &DAOZHANBIAOZHUNSHIJIAN, 3U);
    memcpy(&yujishijian, &YUJIDAOZHANSHIJIAN, 3U);

    if (yujishijian >= biaozhunshijian)
        zaowandianshijian = (yujishijian - biaozhunshijian);
    else
        zaowandianshijian = (biaozhunshijian - yujishijian);

#if 0
    switch(ZAOWANDIANBIAOZHI & 0x03)
    {
        case 0x00:  //晚点
        zaowandianbiaozhizhi = 0x02;
        break;
        case 0x01://正点
        zaowandianbiaozhizhi = 0x00;
        break;
        default:
        break;
    }
#else
    zaowandianbiaozhizhi = (ZAOWANDIANBIAOZHI & 0x03);
#endif
//    LOG_I("早晚点时间：%d - %d = %d", yujishijian, biaozhunshijian, zaowandianshijian);
    if ((0x00 == guozhanzhongxinzhi) && GUOZHANZHONGXIN && (0x80 == LKJGONGZUOMOSHI))
    {
//        LOG_I("...生成早晚点事项...");
        C_zaowandian[0] = zaowandianbiaozhizhi;
        C_zaowandian[1] = zaowandianshijian & 0xff;
        C_zaowandian[2] = (zaowandianshijian >> 8) & 0xff;

        WriteFileContantPkt(0xA1, 0x08, g_ZK_DevCode, C_zaowandian, 3u);
    } /* end if */
    guozhanzhongxinzhi = GUOZHANZHONGXIN;
} /* end function RecordingSoonerAndLaterMessage */


/**********************************************
***********************************************
功能：获取STO控车信息
参数：无
返回：无
***********************************************/
static void RecordingTrainControlMessage( void )
{
  /* 获取允许辅助驾驶信息 */
  RecordingAllowAssistedDriveMessage();
  /* 获取整车进入辅助驾驶信息 */
  RecordingWholeVehicleEnterAssistedDriveMessage();
  /* 获取进入辅助驾驶信息 */
  RecordingEnterAssistedDriveMessage();
  /* 获取整车退出辅助驾驶信息 */
  RecordingWholeVehicleExitAssistedDriveMessage();
  /* 获取退出辅助驾驶信息 */
  RecordingExitAssistedDriveMessage();
  /* 获取允许启动信息 */
  RecordingAllowStartMessage();
  /* 获取控制列车启动信息 */
  RecordingControlTrainStartMessage();
  /* 获取控车工况信息 */
  RecordingControlTrainConditonMessage();
  /* 获取控车级位信息 */
  RecordingControlTrainLevelMessage();
  /* 获取进不可控区域信息 */
  RecordingEnterUncontrolAreaMessage();
  /* 获取出不可控区域信息 */
  RecordingExitUncontrolAreaMessage();
  /* 获取进分相区信息 */
  RecordingEnterPhaseSplitterMessage();
  /* 获取出分相区信息 */
  RecordingExitPhaseSplitterMessage();
  /* 获取贯通指令信息 */
  RecordingThroughCommonMessage();
  /* 获取空气制动指令信息 */
  RecordingAirBrakeCommonMessage();
  /* 获取停车保压信息 */
  RecordingParkingPressurizeMessage();
  /* 获取文本提示信息 */
  RecordingTextPromptMessage();
  /* 获取声音提示信息 */
  RecordingVoicePromptMessage();
  /* 获取工作状态变化信息 */
  RecordingWorkingStateMessage();
  /* 获取工作模式变化信息 */
  RecordingWorkingModeMessage();
  /* 获取静态测试信息 */
  RecordingStaticTestMessage();
  /* 获取控制撒沙信息 */
  RecordingSendingControlMessage();
  /* 获取强制泵风信息 */
  RecordingForcePumpAirMessage();
} /* end function RecordingTrainControlMessage */

/**********************************************
功能：获取允许辅助驾驶信息
参数：无
返回：无
***********************************************/
static void RecordingAllowAssistedDriveMessage( void )
{
    static uint8_t C_yunxufuzhujiashi = 0U;

    if ((0u == C_yunxufuzhujiashi) && (0x01 == ((Enter_Autopilot_S1 & 0x08) >> 3u)))
    {
//        LOG_I("允许辅助驾驶");
        WriteFileContantPkt(0xA1, 0x51, g_ZK_DevCode, &C_yunxufuzhujiashi, 0u);
    } /* end if */
    C_yunxufuzhujiashi = ((Enter_Autopilot_S1 & 0x08) >> 3u);
} /* end function RecordingAllowAssistedDriveMessage */

/**********************************************
功能：获取整车进入辅助驾驶信息
参数：无
返回：无
***********************************************/
static void RecordingWholeVehicleEnterAssistedDriveMessage( void )
{
    static uint8_t C_zhengchujinrufuzhujiashi = 0U;

    if ((0u == C_zhengchujinrufuzhujiashi) && ((Enter_Autopilot_S1 & 0x03) == 0x03))
    {
//        LOG_I("整车进入辅助驾驶");
        WriteFileContantPkt(0xA1, 0x52, g_ZK_DevCode, &C_zhengchujinrufuzhujiashi, 0u);
    } /* end if */
    C_zhengchujinrufuzhujiashi = (Enter_Autopilot_S1 & 0x03);
} /* end function RecordingWholeVehicleEnterAssistedDriveMessage */

/**********************************************
功能：获取进入辅助驾驶信息
参数：无
返回：无
***********************************************/
static void RecordingEnterAssistedDriveMessage(void)
{
    static uint8_t C_jinrufuzhujiashi = 0U;

    if ((0u == C_jinrufuzhujiashi) && (0x01 == ((Enter_Autopilot_S1 & 0x10) >> 4u)))
    {
//        LOG_I("进入辅助驾驶");
        WriteFileContantPkt(0xA1, 0x53, g_ZK_DevCode, &C_jinrufuzhujiashi, 0u);
    } /* end if */
    C_jinrufuzhujiashi = (Enter_Autopilot_S1 & 0x10) >> 4u;
} /* end function RecordingEnterAssistedDriveMessage */

/**********************************************
功能：获取整车退出辅助驾驶信息
参数：无
返回：无
***********************************************/
static void RecordingWholeVehicleExitAssistedDriveMessage(void)
{
    static uint8_t C_zhengchetuichufuzhujiashi = 0U;

    if ((0u == C_zhengchetuichufuzhujiashi) && (0x01 == ((Enter_Autopilot_S1 & 0x04) >> 2u)))
    {
//        LOG_I("整车退出辅助驾驶");
        WriteFileContantPkt(0xA1, 0x54, g_ZK_DevCode, &C_zhengchetuichufuzhujiashi, 0u);
    } /* end if */
    C_zhengchetuichufuzhujiashi = (Enter_Autopilot_S1 & 0x04) >> 2u;
} /* end function RecordingWholeVehicleExitAssistedDriveMessage */

/**********************************************
功能：获取退出辅助驾驶信息
参数：无
返回：无
***********************************************/
static void RecordingExitAssistedDriveMessage(void)
{
    static uint8_t C_tuichufuzhujiashi[4] = { 0U };
    static uint32_t tuichuyuanyin_old = 0u, tuichuyuanyin_new = 0u;

    memcpy(&tuichuyuanyin_new, &TUICHUFUZHUJIASHI, 4u);

    if ((0u == tuichuyuanyin_old) && tuichuyuanyin_new)
    {
//        LOG_I("...生成退出辅助驾驶事项:%x...", tuichuyuanyin_new);
        memcpy(C_tuichufuzhujiashi, &tuichuyuanyin_new, 4u);
        WriteFileContantPkt(0xA1, 0x55, g_ZK_DevCode, C_tuichufuzhujiashi, 4u);
    } /* end if */
    memcpy(&tuichuyuanyin_old, &tuichuyuanyin_new, 4u);
} /* end function RecordingExitAssistedDriveMessage */

/**********************************************
功能：获取允许启动信息
参数：无
返回：无
***********************************************/
static void RecordingAllowStartMessage(void)
{
    static uint8_t C_yunxuqidong = 0U;

    if ((0u == C_yunxuqidong) && YUNXUQIDONG)
    {
        WriteFileContantPkt(0xA1, 0x56, g_ZK_DevCode, &C_yunxuqidong, 0u);
    } /* end if */
    C_yunxuqidong = YUNXUQIDONG;
} /* end function RecordingAllowStartMessage */

/**********************************************
功能：获取控制列车启动信息
参数：无
返回：无
***********************************************/
static void RecordingControlTrainStartMessage(void)
{
    static uint8_t C_kongzhiliecheqidong = 0U;

    if ((0u == C_kongzhiliecheqidong) && KONGZHILIECHEQIDONG)
    {
        WriteFileContantPkt(0xA1, 0x57, g_ZK_DevCode, &C_kongzhiliecheqidong, 0u);
    } /* end if */
    C_kongzhiliecheqidong = KONGZHILIECHEQIDONG;
} /* end function RecordingControlTrainStartMessage */

/**********************************************
功能：获取控车工况信息
参数：无
返回：无
***********************************************/
static void RecordingControlTrainConditonMessage(void)
{
    static uint8_t C_kongchegongkuang = 0U;
    uint8_t kongchegongkuangzhi = 0u;

    switch (KONGCHEGONGKUANG & 0x0f)
    {
        case 0:   //保留
            break;
        case 1:   //牵引
            kongchegongkuangzhi = 0x01;
            break;
        case 2:   //零位
            kongchegongkuangzhi = 0x00;
            break;
        case 3:   //制动
            kongchegongkuangzhi = 0x02;
            break;
        default:  //无效
            kongchegongkuangzhi = 0xff;
            break;
    }
    if (memcmp(&C_kongchegongkuang, &kongchegongkuangzhi, 1U))
    {
        C_kongchegongkuang = kongchegongkuangzhi;
        WriteFileContantPkt(0xA1, 0x58, g_ZK_DevCode, &C_kongchegongkuang, 1u);
    } /* end if */
} /* end function RecordingControlTrainConditonMessage */

/**********************************************
功能：获取控车级位信息
参数：无
返回：无
***********************************************/
static void RecordingControlTrainLevelMessage(void)
{
    static uint8_t C_kongchejiwei = 0U;

    if (memcmp(&C_kongchejiwei, &KONGCHEJIWEI, 1U))
    {
        memcpy(&C_kongchejiwei, &KONGCHEJIWEI, 1U);
//        LOG_I("控制级位值：%d", C_kongchejiwei);
        WriteFileContantPkt(0xA1, 0x59, g_ZK_DevCode, &C_kongchejiwei, 1u);
    } /* end if */
} /* end function RecordingControlTrainLevelMessage */

/**********************************************
功能：获取进不可控区域信息
参数：无
返回：无
***********************************************/
static void RecordingEnterUncontrolAreaMessage(void)
{
    static uint8_t C_jinbukongquyu = 0U;

    if ((0U == C_jinbukongquyu) && (KEKONGBIAOZHI))
    {
//        LOG_I("...生成进不可控区域事项...");
        WriteFileContantPkt(0xA1, 0x60, g_ZK_DevCode, &C_jinbukongquyu, 0u);
    } /* end if */
    C_jinbukongquyu = KEKONGBIAOZHI;
} /* end function RecordingEnterUncontrolAreaMessage */

/**********************************************
功能：获取出不可控区域信息
参数：无
返回：无
***********************************************/
static void RecordingExitUncontrolAreaMessage(void)
{
    static uint8_t C_chubukongquyu = 0U;

    if ((0x01 == C_chubukongquyu) && (0u == KEKONGBIAOZHI))
    {
//        LOG_I("...生成出不可控区域事项...\r\n");
        WriteFileContantPkt(0xA1, 0x61, g_ZK_DevCode, &C_chubukongquyu, 0u);
    } /* end if */
    C_chubukongquyu = KEKONGBIAOZHI;
} /* end function RecordingExitUncontrolAreaMessage */

/**********************************************
功能：获取进分相区信息
参数：无
返回：无
***********************************************/
static void RecordingEnterPhaseSplitterMessage(void)
{
    static uint16_t C_fenxianghuilingzhi = 0U;
    uint8_t C_jinfenxiangqu = 0u;
    uint16_t fenxianghuilingzhi = 0u;

    memcpy(&fenxianghuilingzhi, &FENXIANGHUILING, 2U);

    if ((C_fenxianghuilingzhi) && (0u == fenxianghuilingzhi))
    {
//        LOG_I("...生成进分相区事项...");
        WriteFileContantPkt(0xA1, 0x62, g_ZK_DevCode, &C_jinfenxiangqu, 0u);
    } /* end if */
    C_fenxianghuilingzhi = fenxianghuilingzhi;
} /* end function RecordingEnterPhaseSplitterMessage */

/**********************************************
功能：获取出分相区信息
参数：无
返回：无
***********************************************/
static void RecordingExitPhaseSplitterMessage(void)
{
    static uint16_t C_chufenxianghuilingzhi = 0xFF;
    uint8_t C_chufenxiangqu = 0u;
    uint16_t chufenxianghuilingzhi = 0u;

    memcpy(&chufenxianghuilingzhi, &FENXIANGHUILING, 2U);

    if ((0u == C_chufenxianghuilingzhi) && (chufenxianghuilingzhi))
    {
//        LOG_I("...生成出分相区事项...");
        WriteFileContantPkt(0xA1, 0x63, g_ZK_DevCode, &C_chufenxiangqu, 0u);
    } /* end if */
    C_chufenxianghuilingzhi = chufenxianghuilingzhi;
} /* end function RecordingExitPhaseSplitterMessage */

/**********************************************
功能：获取贯通指令信息
参数：无
返回：无
***********************************************/
static void RecordingThroughCommonMessage(void)
{
    static uint8_t C_Guantongzhiling[2] = { 0x0U };
    uint8_t guantongbiaozhi = 0u;

    guantongbiaozhi = ((GUANTONGJIEGUO & 0x0c) >> 2u);
    if ((C_Guantongzhiling[0] != guantongbiaozhi) && guantongbiaozhi)
    {
        if (0x01 == guantongbiaozhi)  //贯通开始
        {
            C_Guantongzhiling[0] = 0x01;
            C_Guantongzhiling[1] = 0x00;
        }
        else if (0x02 == guantongbiaozhi)  //贯通结束
        {
            C_Guantongzhiling[0] = 0x02;
            C_Guantongzhiling[1] = (GUANTONGJIEGUO & 0x03);
        }
        else   //无效
        {
            C_Guantongzhiling[0] = 0x00;
            C_Guantongzhiling[1] = 0x00;
        }

        WriteFileContantPkt(0xA1, 0x64, g_ZK_DevCode, C_Guantongzhiling, 2u);
    } /* end if */
} /* end function RecordingThroughCommonMessage */

/**********************************************
功能：获取空气制动指令信息
参数：无
返回：无
***********************************************/
static void RecordingAirBrakeCommonMessage(void)
{
    uint8_t C_Kongqizhidong[6] = { 0x0U };
    static uint8_t kongzhizhuangtai_old = 0u;

    if (kongzhizhuangtai_old != KONGZHIZHUANGTAI)
    {
//        LOG_I("...生成空气制动事项...");
        C_Kongqizhidong[0] = (*(&JUNFENGGANGYALI + 1));
        C_Kongqizhidong[1] = JUNFENGGANGYALI;
        C_Kongqizhidong[2] = (*(&ZHIDONGGANGYALI + 1));
        C_Kongqizhidong[3] = ZHIDONGGANGYALI;
        C_Kongqizhidong[4] = XIAOZHACEHUAN;
        C_Kongqizhidong[5] = KONGZHIZHUANGTAI;
        WriteFileContantPkt(0xA1, 0x65, g_ZK_DevCode, C_Kongqizhidong, 6u);
        kongzhizhuangtai_old = KONGZHIZHUANGTAI;
    } /* end if */
} /* end function RecordingAirBrakeCommonMessage */

/**********************************************
功能：获取停车保压信息
参数：无
返回：无
***********************************************/
static void RecordingParkingPressurizeMessage(void)
{
    static uint8_t C_Tingchebaoya[6] = { 0x0U };

    if ((0u == C_Tingchebaoya[5]) && (TINGCHEBAOYA))
    {
//        LOG_I("...生成停车保压事项...");
        C_Tingchebaoya[0] = (*(&JUNFENGGANGYALI + 1));
        C_Tingchebaoya[1] = JUNFENGGANGYALI;
        C_Tingchebaoya[2] = (*(&ZHIDONGGANGYALI + 1));
        C_Tingchebaoya[3] = ZHIDONGGANGYALI;
        C_Tingchebaoya[4] = XIAOZHACEHUAN;
        WriteFileContantPkt(0xA1, 0x66, g_ZK_DevCode, C_Tingchebaoya, 5u);
    } /* end if */
    C_Tingchebaoya[5] = TINGCHEBAOYA;
} /* end function RecordingParkingPressurizeMessage */

/**********************************************
功能：获取文本提示信息
参数：无
返回：无
***********************************************/
static void RecordingTextPromptMessage(void)
{
    static uint8_t C_Wenbentishibiaozhi[2] = { 0x0U };

    if (memcmp(C_Wenbentishibiaozhi, &WENBENTISHI, 2U) && WENBENTISHI)
    {
        memcpy(C_Wenbentishibiaozhi, &WENBENTISHI, 2U);

        WriteFileContantPkt(0xA1, 0x67, g_ZK_DevCode, C_Wenbentishibiaozhi, 2u);
    } /* end if */
} /* end function RecordingTextPromptMessage */

/**********************************************
功能：获取声音提示信息
参数：无
返回：无
***********************************************/
static void RecordingVoicePromptMessage( void )
{
//  static uint8_t C_yuyindaima[2]  = { 0x00U, 0x00U };

//  if ( memcmp( C_yuyindaima, &YUYINDAIMA, 2U ) )
//  {
//		printf("。。。生成声音提示事项。。。\r\n");
//		memcpy( C_yuyindaima, &YUYINDAIMA, 2U);
//		WriteFileContantPkt( 0xA1, 0x68, g_ZK_DevCode, C_yuyindaima, 2u ); 
//	} /* end if */	
} /* end function RecordingVoicePromptMessage */

/**********************************************
功能：获取工作状态变化信息
参数：无
返回：无
***********************************************/
static void RecordingWorkingStateMessage(void)
{
//  static uint8_t C_gongzuozhuangtai = 0U;
    uint8_t gongzuozhuangtaizhi = 0u;

    switch (GONGZUOZHUANGTAI)
    {
        case 0x00:  //人工驾驶
            gongzuozhuangtaizhi = 0x01;
            break;
        case 0x01:  //指导驾驶
            gongzuozhuangtaizhi = 0x02;
            break;
        case 0x02:  //辅助预置驾驶
            gongzuozhuangtaizhi = 0x03;
            break;
        case 0x03:  //辅助驾驶
            gongzuozhuangtaizhi = 0x04;
            break;
        case 0x04:  //退出辅助驾驶
            gongzuozhuangtaizhi = 0x05;
            break;
        default:
            break;
    }

    if (C_gongzuozhuangtai != gongzuozhuangtaizhi)
    {
        C_gongzuozhuangtai = gongzuozhuangtaizhi;
//        LOG_I("生成工作状态事项：%x", gongzuozhuangtaizhi);
        WriteFileContantPkt(0xA1, 0x69, g_ZK_DevCode, &C_gongzuozhuangtai, 1u);
    } /* end if */
} /* end function RecordingWorkingStateMessage */

/**********************************************
功能：获取工作模式变化信息
参数：无
返回：无
***********************************************/
static void RecordingWorkingModeMessage(void)
{
//  static uint8_t C_gongzuomoshi = 0xFFU;

    if (C_gongzuomoshi != GONGZUOMOSHI)
    {
        C_gongzuomoshi = GONGZUOMOSHI;
//        LOG_I("工作模式变化：%x", GONGZUOMOSHI);
        WriteFileContantPkt(0xA1, 0x70, g_ZK_DevCode, &C_gongzuomoshi, 1u);
    } /* end if */
} /* end function RecordingWorkingModeMessage */

/**********************************************
功能：获取静态测试信息
参数：无
返回：无
***********************************************/
static void RecordingStaticTestMessage(void)
{
    static uint8_t C_jingtaiceshizhuangtai = 0U;
    uint8_t ceshizhuangtaizhi = 0xff;

    switch (JINGTAICESHIZHUANGTAI)
    {
        case 0x01:
            ceshizhuangtaizhi = 0x00;
            break;
        case 0x02:
            ceshizhuangtaizhi = 0x01;
            break;
        case 0x04:
            ceshizhuangtaizhi = 0x02;
            break;
        case 0x08:
            ceshizhuangtaizhi = 0x03;
            break;
        case 0x10:
            ceshizhuangtaizhi = 0x04;
            break;
        case 0x20:
            ceshizhuangtaizhi = 0x05;
            break;
        case 0x40:
            ceshizhuangtaizhi = 0x06;
            break;
        default:
            break;
    }

    if (C_jingtaiceshizhuangtai != ceshizhuangtaizhi)
    {
        C_jingtaiceshizhuangtai = ceshizhuangtaizhi;
        WriteFileContantPkt(0xA1, 0x71, g_ZK_DevCode, &C_jingtaiceshizhuangtai, 1U);
    } /* end if */
} /* end function RecordingStaticTestMessage */

/**********************************************
功能：获取控制撒沙信息
参数：无
返回：无
***********************************************/
static void RecordingSendingControlMessage(void)
{
    static uint8_t C_kongzhisasha = 0U;

    if ((0u == C_kongzhisasha) && KONGZHISASHA)
    {
//        LOG_I("...生成控制撒沙事项...");
        WriteFileContantPkt(0xA1, 0x72, g_ZK_DevCode, &C_kongzhisasha, 0U);
    } /* end if */
    C_kongzhisasha = KONGZHISASHA;
} /* end function RecordingSendingControlMessage */

/**********************************************
功能：获取强制泵风信息
参数：无
返回：无
***********************************************/
static void RecordingForcePumpAirMessage(void)
{
    static uint8_t C_qiangzhibengfeng = 0U;

    if( (0u == C_qiangzhibengfeng) && QIANGZHIBENGFENG )
    {
//        LOG_I(...生成控制泵风事项...");
        WriteFileContantPkt( 0xA1, 0x73, g_ZK_DevCode, &C_qiangzhibengfeng, 0U );
    } /* end if */
    C_qiangzhibengfeng = QIANGZHIBENGFENG;
} /* end function RecordingForcePumpAirMessage */

/**********************************************
***********************************************
功能：获取机车制动系统信息
参数：无
返回：无
***********************************************/
static void RecordingLocoBrakeMessage( void )
{
  /* 获取列车管压力信息 */
  RecordingTrainPipePressureMessage();
  /* 获取制动缸压力信息 */
  RecordingBrakeCylinderPressureMessage();
  /* 获取总风缸压力信息 */
  RecordingMainAirPressureMessage();
  /* 获取均缸压力信息 */
  RecordingEqualizReservoirPressureMessage();
  /* 获取制动手柄信息 */
  RecordingBrakeHandleMessage();
  /* 获取充风流量信息 */
  RecordingChargingFlowMessage();
  /* 获取BCU状态信息 */
  RecordingBCUStateMessage();
  /* 获取惩罚制动信息 */
  RecordingPenaltyBrakeMessage();
  /* 获取BCU辅助驾驶信息 */
  RecordingBCUAssistedDriveMessage();
  /* 获取BCU允许条件信息 */
  RecordingBCUPermitConditionMessage();
  /* 获取BCU厂家信息*/
  RecordingBCUManufacturerMessage();
  /* 获取BCU故障信息 */
  RecordingBCUErrorCodeMessage();
} /* end function RecordingLocomotiveBrakeMessage */

/**********************************************
功能：获取列车管压力信息
参数：无
返回：无
***********************************************/
static void RecordingTrainPipePressureMessage(void)
{
//  static uint8_t C_liecheguanyali[2]  = { 0x00U, 0x00U };
    uint16_t Pressure_Dvalue = 0U;    //压力差值，单位kPa
    uint16_t liecheguanyali_New = 0u, liecheguanyali_Old = 0u;

    liecheguanyali_New = ((uint16_t) LIECHEGUANYALI << 8u) + (uint16_t) (*(&LIECHEGUANYALI + 1));
    liecheguanyali_Old = (uint16_t) C_liecheguanyali[0] + (uint16_t) (C_liecheguanyali[1] << 8);

    if ((0x03 == GONGZUOZHUANGTAI) || (0x04 == GONGZUOZHUANGTAI))   //自动驾驶
        Pressure_Dvalue = (16 * 3U);
    else
        Pressure_Dvalue = (16 * 10U);

    if (Pressure_Dvalue <= abs(liecheguanyali_New - liecheguanyali_Old))
    {
//        LOG_I("列车管压力：%d --- %d", liecheguanyali_Old, liecheguanyali_New);
        C_liecheguanyali[0] = (*(&LIECHEGUANYALI + 1));
        C_liecheguanyali[1] = LIECHEGUANYALI;
        WriteFileContantPkt(0xA2, 0x01, g_ZK_DevCode, C_liecheguanyali, 0u);
    } /* end if */
} /* end function RecordingTrainPipePressureMessage */

/**********************************************
功能：获取制动缸压力信息
参数：无
返回：无
***********************************************/
static void RecordingBrakeCylinderPressureMessage(void)
{
//  static uint8_t C_zhidonggangyali[2]  = { 0x00U, 0x00U };
    uint16_t Pressure_Dvalue = 0U;    //压力差值，单位kPa
    uint16_t zhidonggangyali_New = 0u, zhidonggangyali_Old = 0u;

    zhidonggangyali_New = ((uint16_t) ZHIDONGGANGYALI << 8u) + (uint16_t) (*(&ZHIDONGGANGYALI + 1));
    zhidonggangyali_Old = (uint16_t) C_zhidonggangyali[0] + (uint16_t) (C_zhidonggangyali[1] << 8);

    if ((0x03 == GONGZUOZHUANGTAI) || (0x04 == GONGZUOZHUANGTAI))   //自动驾驶
        Pressure_Dvalue = (16 * 3U);
    else
        Pressure_Dvalue = (16 * 10U);

    if (Pressure_Dvalue <= abs(zhidonggangyali_New - zhidonggangyali_Old))
    {
        C_zhidonggangyali[0] = (*(&ZHIDONGGANGYALI + 1));
        C_zhidonggangyali[1] = ZHIDONGGANGYALI;
//        LOG_I("制动缸压力：%x", zhidonggangyali_New);
        WriteFileContantPkt(0xA2, 0x02, g_ZK_DevCode, C_zhidonggangyali, 0u);
    } /* end if */
} /* end function RecordingBrakeCylinderPressureMessage */

/**********************************************
功能：获取总风缸压力信息
参数：无
返回：无
***********************************************/
static void RecordingMainAirPressureMessage(void)
{
    static uint8_t C_zongfenggangyali[2] = { 0x00U, 0x00U };
    uint16_t zongfenggangyali_New = 0u, zongfenggangyali_Old = 0u;

    zongfenggangyali_New = (uint16_t) (ZONGFENGGANGYALI << 8u) + (uint16_t) (*(&ZONGFENGGANGYALI + 1));
    zongfenggangyali_Old = (uint16_t) C_zongfenggangyali[0] + (uint16_t) (C_zongfenggangyali[1] << 8);

    /* 总风缸压力变化大于50kPa */
    if ((16 * 50) <= abs(zongfenggangyali_New - zongfenggangyali_Old))
    {
        C_zongfenggangyali[0] = (*(&ZONGFENGGANGYALI + 1));
        C_zongfenggangyali[1] = ZONGFENGGANGYALI;
        WriteFileContantPkt(0xA2, 0x03, g_ZK_DevCode, C_zongfenggangyali, 2u);
    } /* end if */
} /* end function RecordingMainAirPressureMessage */

/**********************************************
功能：获取均缸压力信息
参数：无
返回：无
***********************************************/
static void RecordingEqualizReservoirPressureMessage(void)
{
//  static uint8_t C_jungangyali[2]  = { 0x00U, 0x00U };
    uint16_t jungangyali_New = 0u, jungangyali_Old = 0u;

    jungangyali_New = ((uint16_t) JUNFENGGANGYALI << 8u) + (uint16_t) (*(&JUNFENGGANGYALI + 1));
    jungangyali_Old = (uint16_t) C_jungangyali[0] + (uint16_t) (C_jungangyali[1] << 8);
    /* 均缸压力变化大于20kPa */
    if ((16 * 20U) <= abs(jungangyali_New - jungangyali_Old))
    {
        C_jungangyali[0] = (*(&JUNFENGGANGYALI + 1));
        C_jungangyali[1] = JUNFENGGANGYALI;
        WriteFileContantPkt(0xA2, 0x04, g_ZK_DevCode, C_jungangyali, 0u);
    } /* end if */
} /* end function RecordingEqualizReservoirPressureMessage */

/**********************************************
功能：获取制动手柄信息
参数：无
返回：无
***********************************************/
static void RecordingBrakeHandleMessage(void)
{
    static uint8_t C_BCUzhidongshoubing[3] = { 0U };
    uint8_t rec = 0U;

    if (C_BCUzhidongshoubing[0] != ZIZHIDONGSHOUBING)
    {
        C_BCUzhidongshoubing[0] = ZIZHIDONGSHOUBING;
        rec |= 1U;
    }
    if (C_BCUzhidongshoubing[1] != DANDUZHIDONGSHOUBING)
    {
        C_BCUzhidongshoubing[1] = DANDUZHIDONGSHOUBING;
        rec |= 1U;
    }
    if (C_BCUzhidongshoubing[2] != XIAOZHACEHUAN)
    {
        C_BCUzhidongshoubing[2] = XIAOZHACEHUAN;
        rec |= 1U;
    }

    if (rec)
    {
//        LOG_I("...生成制动手柄变化事项...");
        WriteFileContantPkt(0xA2, 0x05, 0x92, C_BCUzhidongshoubing, 3u);
    } /* end if */
} /* end function RecordingBrakeHandleMessage */

/**********************************************
功能：获取充风流量信息
参数：无
返回：无
***********************************************/
static void RecordingChargingFlowMessage(void)
{
    static uint8_t C_BCUchongfengliuliang[2] = { 0x00U, 0x00U };
    uint16_t chongfengliuliang_New = 0u, chongfengliuliang_Old = 0u;

    chongfengliuliang_New = (uint16_t) (BCUCHONGFENGLIULIANG << 8) + (uint16_t) (*(&BCUCHONGFENGLIULIANG + 1));
    chongfengliuliang_Old = (uint16_t) C_BCUchongfengliuliang[0] + (uint16_t) (C_BCUchongfengliuliang[1] << 8);

//    LOG_I("充风流量值：%d-----%d", chongfengliuliang_Old, chongfengliuliang_New);
    /* 流量变化大于0.1m3/min */
    if (100U <= abs(chongfengliuliang_New - chongfengliuliang_Old))
    {
        C_BCUchongfengliuliang[0] = (*(&BCUCHONGFENGLIULIANG + 1));
        C_BCUchongfengliuliang[1] = BCUCHONGFENGLIULIANG;
        WriteFileContantPkt(0xA2, 0x06, g_ZK_DevCode, C_BCUchongfengliuliang, 2u);
    } /* end if */
} /* end function RecordingChargingFlowMessage */

/**********************************************
功能：获取BCU状态信息
参数：无
返回：无
***********************************************/
static void RecordingBCUStateMessage(void)
{
    static uint8_t C_BCUzhuangtai[1] = { 0U };

    if (C_BCUzhuangtai[0] != BCUZHUANGTAI)
    {
        C_BCUzhuangtai[0] = BCUZHUANGTAI;
//        LOG_I("...BCU状态变化...");
        WriteFileContantPkt(0xA2, 0x07, g_ZK_DevCode, C_BCUzhuangtai, 1u);
    } /* end if */
} /* end function RecordingBCUStateMessage */

/**********************************************
功能：获取惩罚制动信息
参数：无
返回：无
***********************************************/
static void RecordingPenaltyBrakeMessage(void)
{
    static uint8_t C_BCUchengfazhidong = 0U;

    if ((0u == C_BCUchengfazhidong) && BCUCHENGFAZHIDONG)
    {
//        LOG_I("...生成惩罚制动事项...");
        WriteFileContantPkt(0xA2, 0x08, g_ZK_DevCode, &C_BCUchengfazhidong, 0u);
    } /* end if */
    C_BCUchengfazhidong = BCUCHENGFAZHIDONG;
} /* end function RecordingPenaltyBrakeMessage */

/**********************************************
功能：获取BCU辅助驾驶信息
参数：无
返回：无
***********************************************/
static void RecordingBCUAssistedDriveMessage(void)
{
    static uint8_t C_BCUfuzhujiashizhuangtai[1] = { 0U };

    if (C_BCUfuzhujiashizhuangtai[0] != BCUFUZHUJIASHIZHUANGTAI)
    {
        C_BCUfuzhujiashizhuangtai[0] = BCUFUZHUJIASHIZHUANGTAI;
        WriteFileContantPkt(0xA2, 0x09, g_ZK_DevCode, C_BCUfuzhujiashizhuangtai, 1u);
    } /* end if */
} /* end function RecordingBCUAssistedDriveMessage */

/**********************************************
功能：获取BCU允许条件信息
参数：无
返回：无
***********************************************/
static void RecordingBCUPermitConditionMessage(void)
{
    static uint8_t C_BCUyunxutiaojian[1] = { 0U };

    if (memcmp(C_BCUyunxutiaojian, &BCUYUNXUTIAOJIAN, 1U))
    {
        memcpy(C_BCUyunxutiaojian, &BCUYUNXUTIAOJIAN, 1U);

        WriteFileContantPkt(0xA2, 0x10, g_ZK_DevCode, C_BCUyunxutiaojian, 1u);
    } /* end if */
} /* end function RecordingBCUPermitConditionMessage */

/**********************************************
功能：获取BCU厂家信息
参数：无
返回：无
***********************************************/
static void RecordingBCUManufacturerMessage(void)
{
    static uint8_t C_BCUchangjia[1] = { 0U };

    if (memcmp(C_BCUchangjia, &BCUCHANGJIA, 1U))
    {
        memcpy(C_BCUchangjia, &BCUCHANGJIA, 1U);

        WriteFileContantPkt(0xA2, 0x11, g_ZK_DevCode, C_BCUchangjia, 1u);
    } /* end if */
} /* end function RecordingBCUManufacturerMessage */

/**********************************************
功能：获取BCU故障信息
参数：无
返回：无
***********************************************/
static void RecordingBCUErrorCodeMessage(void)
{
    static uint8_t C_BCUguzhangdaima[4] = { 0U };

    if (memcmp(C_BCUguzhangdaima, &BCUGUZHANGDAIMA, 4U))
    {
        memcpy(C_BCUguzhangdaima, &BCUGUZHANGDAIMA, 4U);

        WriteFileContantPkt(0xA2, 0x12, g_CEU_DevCode, C_BCUguzhangdaima, 4u);
    } /* end if */
} /* end function RecordingBCUErrorCodeMessage */


/**********************************************
***********************************************
功能：获取机车牵引系统信息
参数：无
返回：无
***********************************************/
static void RecordingLocoDrawnMessage( void )
{
  /* 获取物理手柄信息 */
  RecordingPhysicalHandleMessage();
  /* 获取物理机车工况信息 */
  RecordingPhysicalConditionMessage();
  /* 获取电机隔离信息 */
  RecordingIsolationMotorMessage();
  /* 获取机车发挥力信息 */
  RecordingLocoExertionMessage();
  /* 获取机车空转信息 */
  RecordingLocoRaceMessage();
  /* 获取机车滑行信息 */
  RecordingLocoTaxiingMessage();
  /* 获取受电弓状态信息 */
  RecordingPantograghStatusMessage();
  /* 获取主断状态信息 */
  RecordingMainBreakerStatusMessage();
  /* 获取牵引封锁信息 */
  RecordingTractionBlockMessage();
  /* 获取电制封锁信息 */
  RecordingElectricBlockMessage();
  /* 获取预断有效信息 */
  RecordingPrejudgeOffMessage();
  /* 获取强断有效信息 */
  RecordingForceOffMessage();
  /* 获取停放制动信息 */
  RecordingParkBrakeMessage();
  /* 获取撒沙状态信息 */
  RecordingSandingStateMessage();
  /* 获取空电联合信息 */
  RecordingElectropneumaticMessage();
  /* 获取CCU辅助状态信息 */
  RecordingCCUAuxiliaryStatusMessage();
  /* 获取CCU允许测试信息 */
  RecordingCCUAllowTestMessage();
  /* 获取原边电压信息 */
  RecordingPrimaryVoltageMessage();
  /* 获取原边电流信息 */
  RecordingPrimaryCurrentMessage();
  /* 获取电钥匙信息 */
  RecordingElectricKeyMessage();
} /* end function RecordingLocoDrawnMessage */

/**********************************************
功能：获取物理手柄信息
参数：无
返回：无
***********************************************/
static void RecordingPhysicalHandleMessage(void)
{
    static uint8_t C_wulishoubingjiwei[1] = { 0U };

    if (memcmp(C_wulishoubingjiwei, &WULISHOUBINGJIWEI, 1U))
    {
        memcpy(C_wulishoubingjiwei, &WULISHOUBINGJIWEI, 1U);
//        LOG_I("物理手柄级位变化：%d", WULISHOUBINGJIWEI);
        WriteFileContantPkt(0xA3, 0x01, g_CEU_DevCode, C_wulishoubingjiwei, 1u);
    } /* end if */
} /* end function RecordingPhysicalHandleMessage */

/**********************************************
功能：获取物理机车工况信息
参数：无
返回：无
***********************************************/
static void RecordingPhysicalConditionMessage(void)
{
    static uint8_t C_wulijichegongkuang[1] = { 0U };
    uint8_t wuligongkuangzhi = 0xFF;

    switch (WULIJICHEGONGKUANG)
    {
        case 1:
            wuligongkuangzhi = 0x01;
            break;
        case 2:
            wuligongkuangzhi = 0x00;
            break;
        case 4:
            wuligongkuangzhi = 0x02;
            break;
        default:
            break;
    }

    if (C_wulijichegongkuang[0] != wuligongkuangzhi)
    {
        C_wulijichegongkuang[0] = wuligongkuangzhi;
        WriteFileContantPkt(0xA3, 0x02, g_CEU_DevCode, C_wulijichegongkuang, 1u);
    } /* end if */
} /* end function RecordingPhysicalConditionMessage */

/**********************************************
功能：获取电机隔离信息
参数：无
返回：无
***********************************************/
static void RecordingIsolationMotorMessage(void)
{
    static uint8_t C_dianjigeli[2] = { 0U };

    if ((C_dianjigeli[0] != DIANJIGELIZHUANGTAI) || (C_dianjigeli[1] != (JICHELIKEYONGBILI + CLJICHELIKEYONGBILI)))
    {
        C_dianjigeli[0] = DIANJIGELIZHUANGTAI;
        C_dianjigeli[1] = (JICHELIKEYONGBILI + CLJICHELIKEYONGBILI);
        WriteFileContantPkt(0xA3, 0x03, g_ZK_DevCode, C_dianjigeli, 2u);
    } /* end if */
} /* end function RecordingIsolationMotorMessage */

/**********************************************
功能：获取机车发挥力信息
参数：无
返回：无
***********************************************/
static void RecordingLocoExertionMessage(void)
{
//  static uint8_t C_jichefahuili[6]  = { 0U };
//  static uint16_t C_gezhouheli_New = 0U, C_gezhouheli_Old = 0U;
//	
//	if( 1u == CHONGLIANCHE)    //单机
//	{
//	  C_gezhouheli_New = (uint16_t)(ZHOU1LI & 0x7F) + (uint16_t)(ZHOU2LI & 0x7F) + (uint16_t)(ZHOU3LI & 0x7F)\
//	                   + (uint16_t)(ZHOU4LI & 0x7F) + (uint16_t)(ZHOU5LI & 0x7F) + (uint16_t)(ZHOU6LI & 0x7F);
//		C_gezhouheli_New = ( C_gezhouheli_New * JICHELIKEYONGBILI ) / 100.0;
//	}
//	else if(2u == CHONGLIANCHE)    //重联
//	{
//	  C_gezhouheli_New = (( (uint16_t)(ZHOU1LI & 0x7F) + (uint16_t)(ZHOU2LI & 0x7F) + (uint16_t)(ZHOU3LI & 0x7F)\
//	                      + (uint16_t)(ZHOU4LI & 0x7F) + (uint16_t)(ZHOU5LI & 0x7F) + (uint16_t)(ZHOU6LI & 0x7F))\
//		                     * JICHELIKEYONGBILI  / 100.0 )
//		                 + (( (uint16_t)(axle1_CLjicheli_T75 & 0x7F) + (uint16_t)(axle2_CLjicheli_T76 & 0x7F) + (uint16_t)(axle3_CLjicheli_T77 & 0x7F)\
//	                      + (uint16_t)(axle4_CLjicheli_T78 & 0x7F) + (uint16_t)(axle5_CLjicheli_T79 & 0x7F) + (uint16_t)(axle6_CLjicheli_T80 & 0x7F))\
//		                     * CLJICHELIKEYONGBILI  / 100.0 );	
//	}
//	else
//	{
//		C_gezhouheli_New = 0U;
//	}
//	
//	if ( 50U <= abs( C_gezhouheli_New  - C_gezhouheli_Old ) )
//  {
//		printf("机车发挥力：%d---%d\r\n",C_gezhouheli_Old,C_gezhouheli_New);
//		C_gezhouheli_Old = C_gezhouheli_New;
//		
//		C_jichefahuili[0] = ZHOU1LI;
//    C_jichefahuili[1] = ZHOU2LI;
//    C_jichefahuili[2] = ZHOU3LI;
//    C_jichefahuili[3] = ZHOU4LI;
//    C_jichefahuili[4] = ZHOU5LI;
//    C_jichefahuili[5] = ZHOU6LI;
//  
//		WriteFileContantPkt( 0xA3, 0x04, g_JK_DevCode, C_jichefahuili, 6u ); 
//	} /* end if */
  
} /* end function RecordingLocoExertionMessage */

/**********************************************
功能：获取机车空转信息
参数：无
返回：无
***********************************************/
static void RecordingLocoRaceMessage(void)
{
    static uint8_t C_jichekongzhuan[7] = { 0U };

    if (C_jichekongzhuan[0] != JICHEKONGZHUAN)
    {
        C_jichekongzhuan[0] = JICHEKONGZHUAN;
        C_jichekongzhuan[1] = ZHOU1LI;
        C_jichekongzhuan[2] = ZHOU2LI;
        C_jichekongzhuan[3] = ZHOU3LI;
        C_jichekongzhuan[4] = ZHOU4LI;
        C_jichekongzhuan[5] = ZHOU5LI;
        C_jichekongzhuan[6] = ZHOU6LI;
//        LOG_I("...生成机车空转事项...");
        WriteFileContantPkt(0xA3, 0x06, g_JK_DevCode, C_jichekongzhuan, 7u);
    } /* end if */
} /* end function RecordingLocoRaceMessage */

/**********************************************
功能：获取机车滑行信息
参数：无
返回：无
***********************************************/
static void RecordingLocoTaxiingMessage(void)
{
    static uint8_t C_jichehuaxing[7] = { 0U };

    if (C_jichehuaxing[0] != JICHEHUAXING)
    {
        C_jichehuaxing[0] = JICHEHUAXING;
        C_jichehuaxing[1] = ZHOU1LI;
        C_jichehuaxing[2] = ZHOU2LI;
        C_jichehuaxing[3] = ZHOU3LI;
        C_jichehuaxing[4] = ZHOU4LI;
        C_jichehuaxing[5] = ZHOU5LI;
        C_jichehuaxing[6] = ZHOU6LI;
//        LOG_I("...生成机车滑行事项...");
        WriteFileContantPkt(0xA3, 0x07, g_JK_DevCode, C_jichehuaxing, 7u);
    } /* end if */
} /* end function RecordingLocoTaxiingMessage */

/**********************************************
功能：获取受电弓状态信息
参数：无
返回：无
***********************************************/
static void RecordingPantograghStatusMessage(void)
{
    static uint8_t C_shoudiangongzhuangtai = 0U;

    if (C_shoudiangongzhuangtai != SHOUDIANGONGZHUANGTAI)
    {
//        LOG_I("...生成受电弓状态事项...");
        C_shoudiangongzhuangtai = SHOUDIANGONGZHUANGTAI;

        WriteFileContantPkt(0xA3, 0x08, g_ZK_DevCode, &C_shoudiangongzhuangtai, 1U);
    } /* end if */
} /* end function RecordingPantograghStatusMessage */

/**********************************************
功能：获取主断状态信息
参数：无
返回：无
***********************************************/
static void RecordingMainBreakerStatusMessage(void)
{
    static uint8_t C_zhuduanzhuangtai = 0U;

    if (C_zhuduanzhuangtai != ZHUDUANZHUANGTAI)
    {
//        LOG_I("...生成主断状态事项...");
        C_zhuduanzhuangtai = ZHUDUANZHUANGTAI;

        WriteFileContantPkt(0xA3, 0x09, g_ZK_DevCode, &C_zhuduanzhuangtai, 1U);
    } /* end if */
} /* end function RecordingMainBreakerStatusMessage */

/**********************************************
功能：获取牵引封锁信息
参数：无
返回：无
***********************************************/
static void RecordingTractionBlockMessage(void)
{
    static uint8_t C_qianyinfengsuo[1] = { 0U };

    if (C_qianyinfengsuo[0] != QIANYINFENGSUO)
    {
//        LOG_I("...生成牵引封锁事项...");
        C_qianyinfengsuo[0] = QIANYINFENGSUO;

        WriteFileContantPkt(0xA3, 0x10, g_ZK_DevCode, C_qianyinfengsuo, 1U);
    } /* end if */
} /* end function RecordingTractionBlockadeMessage */

/**********************************************
功能：获取电制封锁信息
参数：无
返回：无
***********************************************/
static void RecordingElectricBlockMessage(void)
{
    static uint8_t C_dianzhifengsuo[1] = { 0U };

    if (C_dianzhifengsuo[0] != DIANZHIFEGNSUO)
    {
//        LOG_I("...生成电制封锁事项...");
        C_dianzhifengsuo[0] = DIANZHIFEGNSUO;
        WriteFileContantPkt(0xA3, 0x11, g_ZK_DevCode, C_dianzhifengsuo, 1U);
    } /* end if */
} /* end function RecordingElectricBlockMessage */

/**********************************************
功能：获取预断有效信息
参数：无
返回：无
***********************************************/
static void RecordingPrejudgeOffMessage(void)
{
    static uint8_t C_yuduanxinhao = 0U;

//    LOG_I("预断值：%x", YUDUANYOUXIAO);
    if ((C_yuduanxinhao == 0U) && ( YUDUANYOUXIAO == 1U))
    {
//        LOG_I("...生成预断事项...");
        WriteFileContantPkt(0xA3, 0x12, g_ZK_DevCode, &C_yuduanxinhao, 0U);
    } /* end if */
    C_yuduanxinhao = YUDUANYOUXIAO;
} /* end function RecordingPrejudgeMessage */

/**********************************************
功能：获取强断有效信息
参数：无
返回：无
***********************************************/
static void RecordingForceOffMessage(void)
{
    static uint8_t C_qiangduanxinhao = 0U;

    if ((C_qiangduanxinhao == 0U) && ( QIANGDUANHOUXIAO == 1U))
    {
//        LOG_I("...生成强断事项...");
        WriteFileContantPkt(0xA3, 0x13, g_ZK_DevCode, &C_qiangduanxinhao, 0U);
    } /* end if */
    C_qiangduanxinhao = QIANGDUANHOUXIAO;
} /* end function RecordingForceOffMessage */

/**********************************************
功能：获取停放制动信息
参数：无
返回：无
***********************************************/
static void RecordingParkBrakeMessage(void)
{
    static uint8_t C_tingfangzhidong = 0U;

    if (C_tingfangzhidong != TINGFANGZHIDONG)
    {
//        LOG_I("...生成停放制动事项...");
        C_tingfangzhidong = TINGFANGZHIDONG;

        WriteFileContantPkt(0xA3, 0x14, g_ZK_DevCode, &C_tingfangzhidong, 1U);
    } /* end if */
} /* end function RecordingParkBrakeMessage */

/**********************************************
功能：获取撒沙状态信息
参数：无
返回：无
***********************************************/
static void RecordingSandingStateMessage(void)
{
    static uint8_t C_sashazhuangtai = 0U;

    if (C_sashazhuangtai != SASHAZHUANGTAI)
    {
//        LOG_I("...生成撒沙状态事项...");
        C_sashazhuangtai = SASHAZHUANGTAI;

        WriteFileContantPkt(0xA3, 0x15, g_ZK_DevCode, &C_sashazhuangtai, 1U);
    } /* end if */
} /* end function RecordingSandingStateMessage */

/**********************************************
功能：获取空电联合信息
参数：无
返回：无
***********************************************/
static void RecordingElectropneumaticMessage(void)
{
    static uint8_t C_kongdianlianhezhuangtai[1] = { 0U };

    if (C_kongdianlianhezhuangtai[0] != KONGDIANLIANHEZHUANGTAI)
    {
//        LOG_I("...生成空电联合事项...");
        C_kongdianlianhezhuangtai[0] = KONGDIANLIANHEZHUANGTAI;

        WriteFileContantPkt(0xA3, 0x16, g_ZK_DevCode, C_kongdianlianhezhuangtai, 1U);
    } /* end if */
} /* end function RecordingElectropneumaticMessage */

/**********************************************
功能：获取CCU辅助状态信息
参数：无
返回：无
***********************************************/
static void RecordingCCUAuxiliaryStatusMessage(void)
{
//  static uint8_t C_CCUfuzhuzhuangtai[1]  = { 0U };
    uint8_t CCUfuzhuzhuangtaizhi = 0u;

    CCUfuzhuzhuangtaizhi = (CCUFUZHUZHUANGTAI & 0x07);
    if (C_CCUfuzhuzhuangtai[0] != CCUfuzhuzhuangtaizhi)
    {
//        LOG_I("CCU辅助状态：%x  %x", C_CCUfuzhuzhuangtai[0], CCUfuzhuzhuangtaizhi);
        C_CCUfuzhuzhuangtai[0] = CCUfuzhuzhuangtaizhi;
        WriteFileContantPkt(0xA3, 0x17, g_ZK_DevCode, C_CCUfuzhuzhuangtai, 1U);
    } /* end if */
} /* end function RecordingCCUAuxiliaryStatusMessage */

/**********************************************
功能：获取CCU允许测试信息
参数：无
返回：无
***********************************************/
static void RecordingCCUAllowTestMessage(void)
{
    static uint8_t C_CCUyunxuceshi[1] = { 0U };

    if (C_CCUyunxuceshi[0] != CCUYUNXUCESHI)
    {
        C_CCUyunxuceshi[0] = CCUYUNXUCESHI;

        WriteFileContantPkt(0xA3, 0x18, g_ZK_DevCode, C_CCUyunxuceshi, 1U);
    } /* end if */
} /* end function RecordingCCUAllowTestMessage */

/**********************************************
功能：获取原边电压信息
参数：无
返回：无
***********************************************/
static void RecordingPrimaryVoltageMessage(void)
{
    static uint8_t C_yuanbiandianya[2] = { 0x00U, 0x00U };
    uint16_t yuanbiandianya_New = 0u, yuanbiandianya_Old = 0u;

    yuanbiandianya_New = (uint16_t) (YUANBIANDIANYA << 8u) + (uint16_t) (*(&YUANBIANDIANYA + 1));
    yuanbiandianya_Old = (uint16_t) C_yuanbiandianya[0] + (uint16_t) (C_yuanbiandianya[1] << 8);

    /* 原边电压变化大于3kv */
    if (3000U <= abs(yuanbiandianya_New - yuanbiandianya_Old))
    {
        C_yuanbiandianya[0] = (*(&YUANBIANDIANYA + 1));
        C_yuanbiandianya[1] = YUANBIANDIANYA;
        WriteFileContantPkt(0xA3, 0x19, g_JK_DevCode, C_yuanbiandianya, 2u);
    } /* end if */
} /* end function RecordingPrimaryVoltageMessage */

/**********************************************
功能：获取原边电流信息
参数：无
返回：无
***********************************************/
static void RecordingPrimaryCurrentMessage(void)
{
    static uint8_t C_yuanbiandianliu[2] = { 0x00U, 0x00U };
    uint16_t yuanbiandianliu_New = 0u, yuanbiandianliu_Old = 0u;

    yuanbiandianliu_New = (uint16_t) (YUANBIANDIANLIU << 8u) + (uint16_t) (*(&YUANBIANDIANLIU + 1));
    yuanbiandianliu_Old = (uint16_t) C_yuanbiandianliu[0] + (uint16_t) (C_yuanbiandianliu[1] << 8);

    /* 原边电流变化大于20A */
    if (20U <= abs(yuanbiandianliu_New - yuanbiandianliu_Old))
    {
        C_yuanbiandianliu[0] = (*(&YUANBIANDIANLIU + 1));
        C_yuanbiandianliu[1] = YUANBIANDIANLIU;
        WriteFileContantPkt(0xA3, 0x20, g_JK_DevCode, C_yuanbiandianliu, 2u);
    } /* end if */
} /* end function RecordingPrimaryCurrentMessage */

/**********************************************
功能：获取电钥匙信息
参数：无
返回：无
***********************************************/
static void RecordingElectricKeyMessage(void)
{
    static uint8_t C_dianyaoshizhuangtai = 0U;

    if (C_dianyaoshizhuangtai != DIANYAOSHIZHUANGTAI)
    {
        C_dianyaoshizhuangtai = DIANYAOSHIZHUANGTAI;

        WriteFileContantPkt(0xA3, 0x21, g_ZK_DevCode, &C_dianyaoshizhuangtai, 1U);
    } /* end if */
} /* end function RecordingElectricKeyMessage */


/**********************************************
***********************************************
功能：获取LKJ系统信息
参数：无
返回：无
***********************************************/
static void RecordingLKJSystemMessage( void )
{
/* 获取申请揭示信息 */
  RecordingApplyRevealMessage();
/* 获取接收揭示信息 */
  RecordingReceiveRevealMessage();
/* 获取揭示内容信息 */
  RecordingRevealContentMessage();
/* 获取司机号1变化信息 */
  RecordingDriverNum1Message();
/* 获取司机号2变化信息 */
  RecordingDriverNum2Message();
/* 获取运行路径变化信息 */
  RecordingRunPathMessage();
/* 获取LKJ发车方向信息 */
  RecordingLKJDepartDirectionMessage();
/* 获取总重信息 */
  RecordingTotalWeightMessage();
/* 获取计长信息 */
  RecordingTotalLengthMessage();
/* 获取辆数信息 */
  RecordingVehiclesNumMessage();
/* 获取载重信息 */
  RecordingLoadMessage();
/* 获取客车信息 */
  RecordingPassengerTrainMessage();
/* 获取重车信息 */
  RecordingHeavyTrainMessage();
/* 获取空车信息 */
  RecordingEmptyTrainMessage();
/* 获取非运用车信息*/
  RecordingNonTrafficTrainMessage();
/* 获取代客车信息 */
  RecordingSubstituteTrainMessage();
/* 获取守车信息 */
  RecordingCabooseTrainMessage();
/* 获取车速等级信息*/
  RecordingSpeedGradeMessage();		
} /* end function RecordingLKJSystemMessage */

/**********************************************
功能：获取申请揭示信息
参数：无
返回：无
***********************************************/
static void RecordingApplyRevealMessage( void )
{

} /* end function RecordingApplyRevealMessage */

/**********************************************
功能：获取接收揭示信息
参数：无
返回：无
***********************************************/
static void RecordingReceiveRevealMessage(void)
{
    static uint8_t C_jieshoujieshi[3] = { 0U };
    static uint16_t jieshi_tmp_new = 0u, jieshi_tmp_old = 0u;

    memcpy(&jieshi_tmp_new, &HUOQUJIESHITIAOSHU, 2U);

    if ((jieshi_tmp_old != jieshi_tmp_new) && ((jieshi_tmp_new & 0x7FFF) < 0x7FFF))
    {
        if (0x0000 == (jieshi_tmp_new & 0x8000))  //揭示成功
        {
            C_jieshoujieshi[0] = 0u;
            C_jieshoujieshi[1] = (uint8_t) (jieshi_tmp_new & 0x7FFF);
            C_jieshoujieshi[2] = (uint8_t) ((jieshi_tmp_new & 0x7FFF) >> 8);
        }
        else  //揭示失败
        {
            C_jieshoujieshi[0] = 1u;
            C_jieshoujieshi[1] = 0u;
            C_jieshoujieshi[2] = 0u;
        }

        WriteFileContantPkt(0xA4, 0x02, g_ZK_DevCode, C_jieshoujieshi, 3U);
        memcpy(&jieshi_tmp_old, &jieshi_tmp_new, 2U);
    } /* end if */
} /* end function RecordingReceiveRevealMessage */

/**********************************************
功能：获取揭示内容信息
参数：无
返回：无
***********************************************/
static void RecordingRevealContentMessage( void )
{

} /* end function RecordingRevealContentMessage */

/**********************************************
功能：获取司机号1变化信息
参数：无
返回：无
***********************************************/
static void RecordingDriverNum1Message(void)
{
    static uint8_t C_sijihao1[4] = { 0U };

    if (memcmp(C_sijihao1, &SIJIHAO1, 3U))
    {
        memcpy(C_sijihao1, &SIJIHAO1, 3U);

        WriteFileContantPkt(0xA4, 0x04, g_ZK_DevCode, C_sijihao1, 4U);
    } /* end if */
} /* end function RecordingDriverNum1Message */

/**********************************************
功能：获取司机号2变化信息
参数：无
返回：无
***********************************************/
static void RecordingDriverNum2Message(void)
{
    static uint8_t C_sijihao2[4] = { 0U };

    if (memcmp(C_sijihao2, &SIJIHAO2, 3U))
    {
        memcpy(C_sijihao2, &SIJIHAO2, 3U);

        WriteFileContantPkt(0xA4, 0x05, g_ZK_DevCode, C_sijihao2, 4U);
    } /* end if */
} /* end function RecordingDriverNum2Message */

/**********************************************
功能：获取运行路径变化信息
参数：无
返回：无
***********************************************/
static void RecordingRunPathMessage(void)
{
    static uint8_t C_yunxinglujing[4] = { 0U };

    if ((C_yunxinglujing[0] != (SHUJUJIAOLU & 0x1F)) || (C_yunxinglujing[2] != JIANKONGJIAOLU))
    {
        C_yunxinglujing[0] = (SHUJUJIAOLU & 0x1F);
        C_yunxinglujing[2] = JIANKONGJIAOLU;
        WriteFileContantPkt(0xA4, 0x06, g_ZK_DevCode, C_yunxinglujing, 4U);
    } /* end if */
} /* end function RecordingRunPathMessage */

/**********************************************
功能：获取LKJ发车方向信息
参数：无
返回：无
***********************************************/
static void RecordingLKJDepartDirectionMessage(void)
{
    static uint8_t C_fachefangxiang[4] = { 0U };

    if ((C_fachefangxiang[0] != LKJFACHEFANGXIANG) || (C_fachefangxiang[1] != (SHUJUJIAOLU & 0x60) >> 5u))
    {
        C_fachefangxiang[0] = LKJFACHEFANGXIANG;
        C_fachefangxiang[1] = (SHUJUJIAOLU & 0x60) >> 5u;
//        LOG_I("LKJ发车方向：%d", C_fachefangxiang[0] + (C_fachefangxiang[1] << 8));
        WriteFileContantPkt(0xA4, 0x07, g_ZK_DevCode, C_fachefangxiang, 4U);
    } /* end if */
} /* end function RecordingLKJDepartDirectionMessage */

/**********************************************
功能：获取总重信息
参数：无
返回：无
***********************************************/
static void RecordingTotalWeightMessage(void)
{
    static uint8_t C_zongzhong[2] = { 0U };

    if (memcmp(C_zongzhong, &ZONGZHONG, 2U))
    {
        memcpy(C_zongzhong, &ZONGZHONG, 2U);

        WriteFileContantPkt(0xA4, 0x08, g_ZK_DevCode, C_zongzhong, 2U);
    } /* end if */
} /* end function RecordingTotalWeightMessage */

/**********************************************
功能：获取计长信息
参数：无
返回：无
***********************************************/
static void RecordingTotalLengthMessage(void)
{
    static uint8_t C_jichang[2] = { 0U };

    if (memcmp(C_jichang, &JICHANG, 2U))
    {
        memcpy(C_jichang, &JICHANG, 2U);

        WriteFileContantPkt(0xA4, 0x09, g_ZK_DevCode, C_jichang, 2U);
    } /* end if */
} /* end function RecordingTotalLengthMessage */

/**********************************************
功能：获取辆数信息
参数：无
返回：无
***********************************************/
static void RecordingVehiclesNumMessage(void)
{
    static uint8_t C_liangshu[1] = { 0U };

    if (memcmp(C_liangshu, &LIANGSHU, 1U))
    {
        memcpy(C_liangshu, &LIANGSHU, 1U);

        WriteFileContantPkt(0xA4, 0x10, g_ZK_DevCode, C_liangshu, 1U);
    } /* end if */
} /* end function RecordingVehiclesNumMessage */

/**********************************************
功能：获取载重信息
参数：无
返回：无
***********************************************/
static void RecordingLoadMessage(void)
{
    static uint8_t C_zaizhong[2] = { 0U };

    if (memcmp(C_zaizhong, &ZAIZHONG, 2U))
    {
        memcpy(C_zaizhong, &ZAIZHONG, 2U);

        WriteFileContantPkt(0xA4, 0x11, g_ZK_DevCode, C_zaizhong, 2U);
    } /* end if */
} /* end function RecordingLoadMessage */

/**********************************************
功能：获取客车信息
参数：无
返回：无
***********************************************/
static void RecordingPassengerTrainMessage(void)
{
    static uint8_t C_keche[1] = { 0U };

    if (memcmp(C_keche, &KECHE, 1U))
    {
        memcpy(C_keche, &KECHE, 1U);

        WriteFileContantPkt(0xA4, 0x12, g_ZK_DevCode, C_keche, 1U);
    } /* end if */
} /* end function RecordingPassengerTrainMessage */

/**********************************************
功能：获取重车信息
参数：无
返回：无
***********************************************/
static void RecordingHeavyTrainMessage(void)
{
    static uint8_t C_zhongche[1] = { 0U };

    if (memcmp(C_zhongche, &ZHONGCHE, 1U))
    {
        memcpy(C_zhongche, &ZHONGCHE, 1U);

        WriteFileContantPkt(0xA4, 0x13, g_ZK_DevCode, C_zhongche, 1U);
    } /* end if */
} /* end function RecordingHeavyTrainMessage */

/**********************************************
功能：获取空车信息
参数：无
返回：无
***********************************************/
static void RecordingEmptyTrainMessage(void)
{
    static uint8_t C_kongche[1] = { 0U };

    if (memcmp(C_kongche, &KONGCHE, 1U))
    {
        memcpy(C_kongche, &KONGCHE, 1U);

        WriteFileContantPkt(0xA4, 0x14, g_ZK_DevCode, C_kongche, 1U);
    } /* end if */
} /* end function RecordingEmptyTrainMessage */

/**********************************************
功能：获取非运用车信息
参数：无
返回：无
***********************************************/
static void RecordingNonTrafficTrainMessage(void)
{
    static uint8_t C_feiyunyongche[1] = { 0U };

    if (memcmp(C_feiyunyongche, &FEIYUNYONGCHE, 1U))
    {
        memcpy(C_feiyunyongche, &FEIYUNYONGCHE, 1U);

        WriteFileContantPkt(0xA4, 0x15, g_ZK_DevCode, C_feiyunyongche, 1U);
    } /* end if */
} /* end function RecordingNonTrafficTrainMessage */

/**********************************************
功能：获取代客车信息
参数：无
返回：无
***********************************************/
static void RecordingSubstituteTrainMessage(void)
{
    static uint8_t C_daikeche[1] = { 0U };

    if (memcmp(C_daikeche, &DAIKECHE, 1U))
    {
        memcpy(C_daikeche, &DAIKECHE, 1U);

        WriteFileContantPkt(0xA4, 0x16, g_ZK_DevCode, C_daikeche, 1U);
    } /* end if */
} /* end function RecordingSubstituteTrainMessage */

/**********************************************
功能：获取守车信息
参数：无
返回：无
***********************************************/
static void RecordingCabooseTrainMessage(void)
{
    static uint8_t C_shouche[1] = { 0U };

    if (memcmp(C_shouche, &SHOUCHE, 1U))
    {
        memcpy(C_shouche, &SHOUCHE, 1U);

        WriteFileContantPkt(0xA4, 0x17, g_ZK_DevCode, C_shouche, 1U);
    } /* end if */
} /* end function RecordingCabooseTrainMessage */

/**********************************************
功能：获取车速等级信息
参数：无
返回：无
***********************************************/
static void RecordingSpeedGradeMessage(void)
{
    static uint8_t C_chesudengji[2] = { 0U };

    if (C_chesudengji[1] != CHESUDENGJI)
    {
//		LOG_I("车速等级值：%x",CHESUDENGJI);
        C_chesudengji[0] = C_chesudengji[1];
        C_chesudengji[1] = CHESUDENGJI;
        WriteFileContantPkt(0xA4, 0x18, g_ZK_DevCode, C_chesudengji, 2U);
    } /* end if */
} /* end function RecordingSpeedGradeMessage */


/**********************************************
***********************************************
功能：获取列车运行信息
参数：无
返回：无
***********************************************/
static void RecordingTrainOperationMessage( void )
{
  /* 获取过站中心信息 */
  RecordingRransitCenterMessage();
  /* 获取LKJ工作模式信息 */
  RecordingLKJModeMessage();
  /* 获取开车对标信息 */
  RecordingBenchmarkingMessage();
  /* 获取支线选择信息 */
  RecordingBranchLineSelectMessage();
  /* 获取侧线选择信息 */
  RecordingSideLineSelectMessage();
  /* 获取LKJ制动输出信息 */
  RecordingLKJBrakeOutputMessage();
  /* 获取过信号机信息 */
  RecordingPassingSignalMessage();
  /* 获取机车信号变化信息 */
  RecordingCabSignalChangeMessage();
  /* 获取速度信息 */
  RecordingSpeedMessage();
  /* 获取限速信息 */
  RecordingLimitSpeedMessage();
  /* 获取过分相信息 */
  RecordingPassingNeutralSectionMessage();
  /* 获取线路数据终止信息 */
  RecordingLineDataTerminationMessage();
  /* 获取数据故障信息 */
  RecordingDataErrorMessage();
} /* end function RecordingTrainOperationMessage */

/**********************************************
功能：获取过站中心信息
参数：无
返回：无
***********************************************/
static void RecordingRransitCenterMessage(void)
{
    static uint8_t C_guozhanzhongxin[17] = { 0U };

    if ((0u == C_guozhanzhongxin[16]) && GUOZHANZHONGXIN && (0x80 == LKJGONGZUOMOSHI))
    {
        C_guozhanzhongxin[0] = LKJFACHEFANGXIANG;
        C_guozhanzhongxin[1] = (SHUJUJIAOLU & 0x60) >> 5u;
        memcpy(&C_guozhanzhongxin[4], &CHEZHANMING, 12U );
        WriteFileContantPkt(0xA4, 0x50, g_ZK_DevCode, C_guozhanzhongxin, 16U);
    } /* end if */
    C_guozhanzhongxin[16] = GUOZHANZHONGXIN;
} /* end function RecordingRransitCenterMessage */

/**********************************************
功能：获取LKJ工作模式信息
参数：无
返回：无
***********************************************/
static void RecordingLKJModeMessage( void )
{
 static uint8_t C_Lkjgongzuomoshi[2]  = { 0U };
 uint8_t lkjgongzuomoshizhi = 0u;
 
 #if 0
 switch(LKJGONGZUOMOSHI)
 {
	 case 0x01:  //出段/调监状态
		 lkjgongzuomoshizhi = 2u;
		 break;
	 case 0x02:  //运行方向向后
		 break;
	 case 0x04:  //非本务控制
		 lkjgongzuomoshizhi = 7u;
		 break;
	 case 0x08:  //入段
		 lkjgongzuomoshizhi = 2u;
		 break;
	 case 0x10:  //降级工作
		 lkjgongzuomoshizhi = 4u;
		 break;
	 case 0x20:  //平面调车
	 case 0x40:  //调车控制
     lkjgongzuomoshizhi = 3u;
  	 break;
	 case 0x80:  //通常工作
		 lkjgongzuomoshizhi = 5u;
		 break;
	 default:
		 break;	 
 }
 #else
    lkjgongzuomoshizhi = LKJGONGZUOMOSHI;
 #endif
    if (C_Lkjgongzuomoshi[1] != lkjgongzuomoshizhi)
    {
        C_Lkjgongzuomoshi[0] = C_Lkjgongzuomoshi[1];
        C_Lkjgongzuomoshi[1] = lkjgongzuomoshizhi;
        WriteFileContantPkt(0xA4, 0x51, g_ZK_DevCode, C_Lkjgongzuomoshi, 2U);
    } /* end if */
} /* end function RecordingLKJModeMessage */

/**********************************************
功能：获取开车对标信息
参数：无
返回：无
***********************************************/
static void RecordingBenchmarkingMessage(void)
{
    static uint8_t C_kaicheduibiao[1] = { 0U };

    if ((C_kaicheduibiao[0] == 0U) && ( KAICHEDUIBIAO == 1U))
    {
//        LOG_I("...生成开车对标事项...");
        WriteFileContantPkt(0xA4, 0x52, g_ZK_DevCode, C_kaicheduibiao, 0U);
    } /* end if */
    C_kaicheduibiao[0] = KAICHEDUIBIAO;
} /* end function RecordingBenchmarkingMessage */

/**********************************************
功能：获取支线选择信息
参数：无
返回：无
***********************************************/
static void RecordingBranchLineSelectMessage(void)
{
    static uint8_t C_zhixianhao[1] = { 0U };

    if (memcmp(C_zhixianhao, &ZHIXIANHAO, 1U))
    {
        memcpy(C_zhixianhao, &ZHIXIANHAO, 1U);
        WriteFileContantPkt(0xA4, 0x53, g_ZK_DevCode, C_zhixianhao, 1U);
    } /* end if */
} /* end function RecordingBranchLineSelectMessage */

/**********************************************
功能：获取侧线选择信息
参数：无
返回：无
***********************************************/
static void RecordingSideLineSelectMessage(void)
{
    static uint8_t C_cexianhao[1] = { 0U };

    if (memcmp(C_cexianhao, &CEXIANHAO, 1U) && (0u != CEXIANHAO) && (126u != CEXIANHAO) && (127u != CEXIANHAO))
    {
        memcpy(C_cexianhao, &CEXIANHAO, 1U);
        WriteFileContantPkt(0xA4, 0x54, g_ZK_DevCode, C_cexianhao, 1U);
    } /* end if */
} /* end function RecordingBranchLineSelectMessage */

/**********************************************
功能：获取LKJ制动输出信息
参数：无
返回：无
***********************************************/
static void RecordingLKJBrakeOutputMessage(void)
{
    static uint8_t C_Lkjzhidongshuchu[2] = { 0U };

    if (memcmp(C_Lkjzhidongshuchu, &ZHIDONGSHUCHU, 2U))
    {
        memcpy(C_Lkjzhidongshuchu, &ZHIDONGSHUCHU, 2U);
        WriteFileContantPkt(0xA4, 0x55, g_ZK_DevCode, C_Lkjzhidongshuchu, 2U);
    } /* end if */
} /* end function RecordingLKJBrakeOutputMessage */

/**********************************************
功能：获取过信号机信息
参数：无
返回：无
***********************************************/
static void RecordingPassingSignalMessage(void)
{
    static uint8_t C_guoxinhaoji[9 + 4] = { 0U };
    uint8_t xinhaojizhonglei = 0u;

    switch ((XINHAOJILEIXING & 0xf0) >> 4u)
    {
        case 0:  //备用
            break;
        case 1:  //进出站
            break;
        case 2:  //出站
            xinhaojizhonglei = 5u;
            break;
        case 3:  //进站
            xinhaojizhonglei = 2u;
            break;
        case 4:  //通过
            xinhaojizhonglei = 6u;
            break;
        case 5:  //预告
            xinhaojizhonglei = 9u;
            break;
        case 6:
            xinhaojizhonglei = 7u;
            break;
        case 7:
            xinhaojizhonglei = 10u;
            break;
        case 8:    //未定义
            break;
        case 9:
            xinhaojizhonglei = 9u;
            break;
        case 10:
            xinhaojizhonglei = 9u;
            break;
        default:
            break;
    }

    if ((C_guoxinhaoji[0] != xinhaojizhonglei) || (memcmp(&C_guoxinhaoji[7], &XINHAOJIBIANHAO, 2U)))
    {
        memcpy(&C_guoxinhaoji[1], &XINHAOJIBIANHAOZIFUTOU, 6U );

        if(2u == C_guoxinhaoji[0])
        {
            C_guoxinhaoji[9] = QIANFANGCHEZHANHAO;
            C_guoxinhaoji[10] = *(&QIANFANGCHEZHANHAO + 1u);
        }
        else
        {
            C_guoxinhaoji[9] = LKJFACHEFANGXIANG;
            C_guoxinhaoji[10] = (SHUJUJIAOLU & 0x60) >> 5u;
        }
        WriteFileContantPkt( 0xA4, 0x56, g_ZK_DevCode, C_guoxinhaoji, 9U+4U );
        C_guoxinhaoji[0] = xinhaojizhonglei;
        memcpy( &C_guoxinhaoji[7], &XINHAOJIBIANHAO, 2U );
    } /* end if */
} /* end function RecordingLPassingSignalMessage */

/**********************************************
功能：获取机车信号变化信息
参数：无
返回：无
***********************************************/
static void RecordingCabSignalChangeMessage(void)
{
//  static uint8_t C_jichexinhao[2]  = { 0U };
//    LOG_I("机车信号代码：%x %x", *(&JICHEXINHAODAIMA + 1), JICHEXINHAODAIMA);
    if (memcmp(C_jichexinhao, &JICHEXINHAODAIMA, 2U))
    {
        memcpy(C_jichexinhao, &JICHEXINHAODAIMA, 2U);
        WriteFileContantPkt(0xA4, 0x57, g_ZK_DevCode, C_jichexinhao, 2U);
    } /* end if */
} /* end function RecordingCabSignalChangeMessage */

/**********************************************
功能：获取速度信息
参数：无
返回：无
***********************************************/
static void RecordingSpeedMessage( void )
{
// static uint8_t C_Lkjsudu[2]  = { 0x00U, 0x00U };
    uint16_t lkjsudu_New = 0u, lkjsudu_Old = 0u;

    lkjsudu_New = (uint16_t) LKJSUDU + (uint16_t) ((*(&LKJSUDU + 1)) << 8);
    lkjsudu_Old = (uint16_t) C_Lkjsudu[0] + (uint16_t) (C_Lkjsudu[1] << 8);
    /* 速度变化大于等于1Km/h */
//    LOG_I("\r\nLKJ速度：%d", lkjsudu_New);
    if (1U <= abs(lkjsudu_New - lkjsudu_Old))
    {
        memcpy(C_Lkjsudu, &LKJSUDU, 2U);
//        LOG_I("\r\nLKJ速度：%d", lkjsudu_New);
        WriteFileContantPkt(0xA4, 0x58, g_ZK_DevCode, C_Lkjsudu, 0u);
    } /* end if */
} /* end function RecordingSpeedMessage */

/**********************************************
功能：获取限速信息
参数：无
返回：无
***********************************************/
static void RecordingLimitSpeedMessage(void)
{
// static uint8_t C_Lkjxiansu[2]  = { 0x00U, 0x00U };
    uint16_t lkjxiansu_New = 0u, lkjxiansu_Old = 0u;

    lkjxiansu_New = (uint16_t) LKJXIANSU + (uint16_t) ((*(&LKJXIANSU + 1)) << 8);
    lkjxiansu_Old = (uint16_t) C_Lkjxiansu[0] + (uint16_t) (C_Lkjxiansu[1] << 8);
    /* 速度变化大于等于1Km/h */
    if (1U <= abs(lkjxiansu_New - lkjxiansu_Old))
    {
        memcpy(C_Lkjxiansu, &LKJXIANSU, 2U);

        WriteFileContantPkt(0xA4, 0x59, g_ZK_DevCode, C_Lkjxiansu, 2u);
    } /* end if */
} /* end function RecordingLimitSpeedMessage */

/**********************************************
功能：获取过分相信息
参数：无
返回：无
***********************************************/
static void RecordingPassingNeutralSectionMessage(void)
{
    static uint8_t C_guofenxiangbiaozhi = 0U;
    uint8_t guofenxiangzhi = 0u;

    guofenxiangzhi = (GUOFENXIANG & 0x01);
    if ((C_guofenxiangbiaozhi == 0U) && (guofenxiangzhi == 1U))
    {
//        LOG_I("...生成过分相事项...");
        WriteFileContantPkt(0xA4, 0x60, g_ZK_DevCode, &C_guofenxiangbiaozhi, 0U);
    } /* end if */
    C_guofenxiangbiaozhi = guofenxiangzhi;
} /* end function RecordingPassingNeutralSectionMessage */

/**********************************************
功能：获取线路数据终止信息
参数：无
返回：无
***********************************************/
static void RecordingLineDataTerminationMessage( void )
{

} /* end function RecordingLineDataTerminationMessage */

/**********************************************
功能：获取数据故障信息
参数：无
返回：无
***********************************************/
static void RecordingDataErrorMessage(void)
{
    static uint8_t C_Shujucuowuleixing[1] = { 0U };

    if ((0x01 == C_Shujucuowuleixing[0]) && (0u == SHUJUGUZHANG))
    {
//        LOG_I("...生成数据故障事项...");
        WriteFileContantPkt(0xA4, 0x62, g_ZK_DevCode, C_Shujucuowuleixing, 1U);
    } /* end if */
    C_Shujucuowuleixing[0] = SHUJUGUZHANG;
} /* end function RecordingDataErrorMessage */

/**********************************************
***********************************************
功能：获取版本信息
参数：无
返回：无
***********************************************/
static void RecordingVersionMessage( void )
{
  /* 获取插件软件版本信息 */
  RecordingSoftwareVersionMessage();
  /* 获取软件版本不一致信息 */
  RecordingSoftwareVersionInconsistMessage();
  /* 获取软件版本不匹配信息 */
  RecordingSoftwareVersionMismatchMessage();
  /* 获取STO基础数据版本信息 */
  RecordingSTOBasicDataVersionMessage();
  /* 获取STO基础数据不一致信息 */
  RecordingSTOBasicDataInconsistMessage();
  /* 获取STO基础数据不匹配信息 */
  RecordingSTOBasicDataMismatchMessage();
  /* 获取LKJ基础数据版本信息 */
  RecordingLKJBasicDataVersionMessage();	
} /* end function RecordingVersionMessage */

/**********************************************
功能：获取插件软件版本信息
参数：无
返回：无
***********************************************/
static void RecordingSoftwareVersionMessage(void)
{
    static uint8_t C_zk_I_A_bb[9] = { 0x11U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_zk_I_B_bb[9] = { 0x12U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_zk_II_A_bb[9] = { 0x13U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_zk_II_B_bb[9] = { 0x14U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_xsq_I_bb[9] = { 0x21U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_xsq_II_bb[9] = { 0x22U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_tx_I_bb[9] = { 0x31U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_tx_II_bb[9] = { 0x32U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };

    static uint8_t C_wjjk_I_bb[9] = { 0x51U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_wjjk_II_bb[9] = { 0x52U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_jl_bb[9] = { 0x61U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_wxtx_bb[9] = { 0x64U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_CEU_I_bb[9] = { 0x71U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_CEU_II_bb[9] = { 0x72U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_ECU_I_bb[9] = { 0x81U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_ECU_II_bb[9] = { 0x82U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_CCU_bb[9] = { 0x81U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_BCU_bb[9] = { 0x82U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };

    if (CPU_A == Get_CPU_Type())
    {
        /* 主控插件版本 */
        if (memcmp(&C_zk_I_A_bb[1], &ZK_I_A_BB, 4u))
        {
            memcpy(&C_zk_I_A_bb[5], &C_zk_I_A_bb[1], 4u);
            memcpy(&C_zk_I_A_bb[1], &ZK_I_A_BB, 4u);
            WriteFileContantPkt(0xA5, 0x01, 0x11, C_zk_I_A_bb, 9u);
        } /* end if */
        if (memcmp(&C_zk_I_B_bb[1], &ZK_I_B_BB, 4u))
        {
            memcpy(&C_zk_I_B_bb[5], &C_zk_I_B_bb[1], 4u);
            memcpy(&C_zk_I_B_bb[1], &ZK_I_B_BB, 4u);
            WriteFileContantPkt(0xA5, 0x01, 0x12, C_zk_I_B_bb, 9u);
        } /* end if */

        /* 微机接口版本 */
        if (memcmp(&C_wjjk_I_bb[1], &WJJK_I_BB, 4u))
        {
            memcpy(&C_wjjk_I_bb[5], &C_wjjk_I_bb[1], 4u);
            memcpy(&C_wjjk_I_bb[1], &WJJK_I_BB, 4u);
            WriteFileContantPkt(0xA5, 0x01, 0x51, C_wjjk_I_bb, 9u);
        } /* end if */
        /* 通信插件版本 */
        if (memcmp(&C_tx_I_bb[1], &TX_I_BB, 4u))
        {
            memcpy(&C_tx_I_bb[5], &C_tx_I_bb[1], 4u);
            memcpy(&C_tx_I_bb[1], &TX_I_BB, 4u);
            WriteFileContantPkt(0xA5, 0x01, 0x31, C_tx_I_bb, 9u);
        } /* end if */
    }
    if (CPU_B == Get_CPU_Type())
    {
        /* 主控插件版本 */
        if (memcmp(&C_zk_II_A_bb[1], &ZK_II_A_BB, 4u))
        {
            memcpy(&C_zk_II_A_bb[5], &C_zk_II_A_bb[1], 4u);
            memcpy(&C_zk_II_A_bb[1], &ZK_II_A_BB, 4u);
            WriteFileContantPkt(0xA5, 0x01, 0x13, C_zk_II_A_bb, 9u);
        } /* end if */
        if (memcmp(&C_zk_II_B_bb[1], &ZK_II_B_BB, 4u))
        {
            memcpy(&C_zk_II_B_bb[5], &C_zk_II_B_bb[1], 4u);
            memcpy(&C_zk_II_B_bb[1], &ZK_II_B_BB, 4u);
            WriteFileContantPkt(0xA5, 0x01, 0x14, C_zk_II_B_bb, 9u);
        } /* end if */

        /* 微机接口版本 */
        if (memcmp(&C_wjjk_II_bb[1], &WJJK_II_BB, 4u))
        {
            memcpy(&C_wjjk_II_bb[5], &C_wjjk_II_bb[1], 4u);
            memcpy(&C_wjjk_II_bb[1], &WJJK_II_BB, 4u);
            WriteFileContantPkt(0xA5, 0x01, 0x52, C_wjjk_II_bb, 9u);
        } /* end if */
        /* 通信插件版本 */
        if (memcmp(&C_tx_II_bb[1], &TX_II_BB, 4u))
        {
            memcpy(&C_tx_II_bb[5], &C_tx_II_bb[1], 4u);
            memcpy(&C_tx_II_bb[1], &TX_II_BB, 4u);
            WriteFileContantPkt(0xA5, 0x01, 0x32, C_tx_II_bb, 9u);
        } /* end if */
    }

    /* 记录插件版本 */
    if (memcmp(&C_jl_bb[1], &JL_BB, 4u ) )
    {
        memcpy( &C_jl_bb[5], &C_jl_bb[1], 4u );
        memcpy( &C_jl_bb[1], &JL_BB, 4u );
        WriteFileContantPkt( 0xA5, 0x01, 0x61, C_jl_bb, 9u );
    } /* end if */

    /* 显示器版本 */
    if (memcmp(&C_xsq_I_bb[1], &XSQ_I_BB, 4u))
    {
        memcpy(&C_xsq_I_bb[5], &C_xsq_I_bb[1], 4u);
        memcpy(&C_xsq_I_bb[1], &XSQ_I_BB, 4u);
        WriteFileContantPkt(0xA5, 0x01, 0x21, C_xsq_I_bb, 9u);
    } /* end if */
    if (memcmp(&C_xsq_II_bb[1], &XSQ_II_BB, 4u))
    {
        memcpy(&C_xsq_II_bb[5], &C_xsq_II_bb[1], 4u);
        memcpy(&C_xsq_II_bb[1], &XSQ_II_BB, 4u);
        WriteFileContantPkt(0xA5, 0x01, 0x22, C_xsq_II_bb, 9u);
    } /* end if */

    /* 无线通信插件版本 */
    if (memcmp(&C_wxtx_bb[1], &WXTX_BB, 4u ) )
    {
        memcpy( &C_wxtx_bb[5], &C_wxtx_bb[1], 4u );
        memcpy( &C_wxtx_bb[1], &WXTX_BB, 4u );
        WriteFileContantPkt( 0xA5, 0x01, 0x64, C_wxtx_bb, 9u );
    } /* end if */

    /* CEU版本 */
    if (memcmp(&C_CEU_I_bb[1], &CEU_I_BB, 4u ) )
    {
        memcpy( &C_CEU_I_bb[5], &C_CEU_I_bb[1], 4u );
        memcpy( &C_CEU_I_bb[1], &CEU_I_BB, 4u );
        WriteFileContantPkt( 0xA5, 0x01, 0x71, C_CEU_I_bb, 9u );
    } /* end if */
    if (memcmp(&C_CEU_II_bb[1], &CEU_II_BB, 4u ) )
    {
        memcpy( &C_CEU_II_bb[5], &C_CEU_II_bb[1], 4u );
        memcpy( &C_CEU_II_bb[1], &CEU_II_BB, 4u );
        WriteFileContantPkt( 0xA5, 0x01, 0x72, C_CEU_II_bb, 9u );
    } /* end if */

    /* ECU版本 */
    if (memcmp(&C_ECU_I_bb[1], &ECU_I_BB, 4u ) )
    {
        memcpy( &C_ECU_I_bb[5], &C_ECU_I_bb[1], 4u );
        memcpy( &C_ECU_I_bb[1], &ECU_I_BB, 4u );
        WriteFileContantPkt( 0xA5, 0x01, 0x81, C_ECU_I_bb, 9u );
    } /* end if */
    if (memcmp(&C_ECU_II_bb[1], &ECU_II_BB, 4u ) )
    {
        memcpy( &C_ECU_II_bb[5], &C_ECU_II_bb[1], 4u );
        memcpy( &C_ECU_II_bb[1], &ECU_II_BB, 4u );
        WriteFileContantPkt( 0xA5, 0x01, 0x82, C_ECU_II_bb, 9u );
    } /* end if */

    /* CCU版本 */
    if (memcmp(&C_CCU_bb[1], &CCU_BB, 4u ) )
    {
        memcpy( &C_CCU_bb[5], &C_CCU_bb[1], 4u );
        memcpy( &C_CCU_bb[1], &CCU_BB, 4u );
        WriteFileContantPkt( 0xA5, 0x01, 0x91, C_CCU_bb, 9u );
    } /* end if */

    /* BCU版本 */
    if (memcmp(&C_BCU_bb[1], &BCU_BB, 4u ) )
    {
        memcpy( &C_BCU_bb[5], &C_BCU_bb[1], 4u );
        memcpy( &C_BCU_bb[1], &BCU_BB, 4u );
        WriteFileContantPkt( 0xA5, 0x01, 0x92, C_BCU_bb, 9u );
    } /* end if */
} /* end function RecordingSoftwareVersionMessage */

/**********************************************
功能：获取软件版本不一致信息
参数：无
返回：无
***********************************************/
static void RecordingSoftwareVersionInconsistMessage(void)
{
    static uint8_t C_ruanjianbanbenbuyizhi = 0xFFU;

    if ((C_ruanjianbanbenbuyizhi == 0U) && ( RUANJIANBANBENBUYIZHI == 1U))
    {
//        LOG_I("...生成软件版本不一致事项...");
        WriteFileContantPkt(0xA5, 0x02, g_ZK_DevCode, &C_ruanjianbanbenbuyizhi, 0U);
    } /* end if */
    C_ruanjianbanbenbuyizhi = RUANJIANBANBENBUYIZHI;
} /* end function RecordingSoftwareVersionInconsistMessage */

/**********************************************
功能：获取软件版本不匹配信息
参数：无
返回：无
***********************************************/
static void RecordingSoftwareVersionMismatchMessage(void)
{

} /* end function RecordingSoftwareVersionMismatchMessage */

/**********************************************
功能：获取STO基础数据版本信息
参数：无
返回：无
***********************************************/
static void RecordingSTOBasicDataVersionMessage(void)
{
    static uint8_t C_stoA_jichushuju_bb[9] = { 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
    static uint8_t C_stoB_jichushuju_bb[9] = { 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };

    // A模
    if (memcmp(&C_stoA_jichushuju_bb[1], &AJISTOJICHUSHUJUBANBENRIQI, 2u)
            || memcmp(&C_stoA_jichushuju_bb[3], &AJISTOJICHUSHUJUBIANYIRIQI, 2u))
    {
        if (CPU_A == Get_CPU_Type())  //I系记录板A模
            C_stoA_jichushuju_bb[0] = 0x01;
        else if (CPU_B == Get_CPU_Type())  //II系记录板A模
            C_stoA_jichushuju_bb[0] = 0x02;
        else
            C_stoA_jichushuju_bb[0] = 0x0;

        memcpy(&C_stoA_jichushuju_bb[5], &C_stoA_jichushuju_bb[1], 2u);
        memcpy(&C_stoA_jichushuju_bb[7], &C_stoA_jichushuju_bb[3], 2u);
        memcpy(&C_stoA_jichushuju_bb[1], &AJISTOJICHUSHUJUBANBENRIQI, 2u);
        memcpy(&C_stoA_jichushuju_bb[3], &AJISTOJICHUSHUJUBIANYIRIQI, 2u);

        WriteFileContantPkt(0xA5, 0x04, g_ZK_DevCode, C_stoA_jichushuju_bb, 9u);
    } /* end if */

    // B模
    if (memcmp(&C_stoB_jichushuju_bb[1], &BJISTOJICHUSHUJUBANBENRIQI, 2u)
            || memcmp(&C_stoB_jichushuju_bb[3], &BJISTOJICHUSHUJUBIANYIRIQI, 2u))
    {
        if (CPU_A == Get_CPU_Type())  //I系记录板B模
            C_stoB_jichushuju_bb[0] = 0x03;
        else if (CPU_B == Get_CPU_Type())  //II系记录板B模
            C_stoB_jichushuju_bb[0] = 0x04;
        else
            C_stoB_jichushuju_bb[0] = 0x0;

        memcpy(&C_stoB_jichushuju_bb[5], &C_stoB_jichushuju_bb[1], 2u);
        memcpy(&C_stoB_jichushuju_bb[7], &C_stoB_jichushuju_bb[3], 2u);
        memcpy(&C_stoB_jichushuju_bb[1], &BJISTOJICHUSHUJUBANBENRIQI, 2u);
        memcpy(&C_stoB_jichushuju_bb[3], &BJISTOJICHUSHUJUBIANYIRIQI, 2u);

        WriteFileContantPkt(0xA5, 0x04, g_ZK_DevCode, C_stoB_jichushuju_bb, 9u);
    } /* end if */
} /* end function RecordingSTOBasicDataVersionMessage */

/**********************************************
功能：获取STO基础数据不一致信息
参数：无
返回：无
***********************************************/
static void RecordingSTOBasicDataInconsistMessage(void)
{
//  static uint8_t C_sto_jichushuju_bb  = 0U ;
  
//  if ( memcmp( &AJISTOJICHUSHUJUBANBENRIQI, &BJISTOJICHUSHUJUBANBENRIQI, 2U )
//		|| memcmp( &AJISTOJICHUSHUJUBIANYIRIQI, &BJISTOJICHUSHUJUBIANYIRIQI, 2U ))
//  {
//		C_sto_jichushuju_bb = g_ZK_DevCode;
//    WriteFileContantPkt( 0xA5, 0x05, g_ZK_DevCode, &C_sto_jichushuju_bb, 1u ); 
//  } 

} /* end function RecordingSTOBasicDataInconsistMessage */

/**********************************************
功能：获取STO基础数据不匹配信息
参数：无
返回：无
***********************************************/
static void RecordingSTOBasicDataMismatchMessage(void)
{

} /* end function RecordingSTOBasicDataMismatchMessage */

/**********************************************
功能：获取LKJ基础数据版本信息
参数：无
返回：无
***********************************************/
static void RecordingLKJBasicDataVersionMessage(void)
{
//  static uint8_t C_lkjA_shuju_bb[9]     = { 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
//  static uint8_t C_lkjB_shuju_bb[9]     = { 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };
  
//  if ( memcmp( &C_lkjA_shuju_bb[1], &AJILKJSHUJUBANBEN, 4u ) )
//  {
//    memcpy( &C_lkjA_shuju_bb[5], &C_lkjA_shuju_bb[1], 4u );
//    memcpy( &C_lkjA_shuju_bb[1], &AJILKJSHUJUBANBEN, 4u );
//    WriteFileContantPkt( 0xA5, 0x07, 0x61, C_lkjA_shuju_bb, 9u );
//  } /* end if */
//  
//  if ( memcmp( &C_lkjB_shuju_bb[1], &BJILKJSHUJUBANBEN, 4u ) )
//  {
//    memcpy( &C_lkjB_shuju_bb[5], &C_lkjB_shuju_bb[1], 4u );
//    memcpy( &C_lkjB_shuju_bb[1], &BJILKJSHUJUBANBEN, 4u );
//    WriteFileContantPkt( 0xA5, 0x07, 0x61, C_lkjB_shuju_bb, 9u );
//  } /* end if */ 
} /* end function RecordingLKJBasicDataVersionMessage */


/**********************************************
***********************************************
功能：获取插件自检信息
参数：无
返回：无
***********************************************/
static void RecordingSelfCheckMessage( void )
{
  /* 获取主控插件自检信息 */
  RecordingZKSelfCheckMessage();
  /* 获取通信1插件自检信息 */
  RecordingTX1SelfCheckMessage();
  /* 获取通信2插件自检信息 */
  RecordingTX2SelfCheckMessage();
  /* 获取记录插件自检信息 */
  RecordingJLSelfCheckMessage();
  /* 获取无线通信插件自检信息 */
  RecordingWXTXSelfCheckMessage();
  /* 获取微机接口自检信息 */
  RecordingWJJKSelfCheckMessage();
  /* 获取显示器自检信息 */
  RecordingDMISelfCheckMessage();
//  /* 获取CEU自检信息 */
//  RecordingCEUSelfCheckMessage();
//  /* 获取ECU自检信息 */
//  RecordingECUSelfCheckMessage();
  /* 获取与LKJ通信状态信息 */
  RecordingCommunicatWithLKJMessage();
  /* 获取与CCU通信状态信息 */
  RecordingCommunicatWithCCUMessage();
  /* 获取与BCU通信状态信息 */
  RecordingCommunicatWithBCUMessage();
  /* 获取与CIR通信状态信息 */
  RecordingCommunicatWithCIRMessage();
//  /* 获取与CEU通信状态信息 */
//  RecordingCommunicatWithCEUMessage();
//  /* 获取与ECU通信状态信息 */
//  RecordingCommunicatWithECUMessage();
} /* end function RecordingSelfCheckMessage */

/**********************************************
功能：获取主控插件自检信息
参数：无
返回：无
***********************************************/
static void RecordingZKSelfCheckMessage(void)
{
    static uint8_t C_zkzj[3] = { 0x00U, 0x00U, 0x00U };

    /* 判断自检状态是否发生变化 */
    if (memcmp(&C_zkzj[1], &ZK_ZJ, 2U))
    {
        memcpy(&C_zkzj[1], &ZK_ZJ, 2U);

        /* 判断自检为正常或异常 */
        if (NORMAL == ZK_ZJ)
        {
            if (CPU_A == Get_CPU_Type())  //I系
            {
                if (g_ZK_DevCode == 0x11)  //A模
                    C_zkzj[0] = 0x01;
                else
                    C_zkzj[0] = 0x03;
            }
            else if (CPU_B == Get_CPU_Type())  //II系
            {
                if (g_ZK_DevCode == 0x11)  //A模
                    C_zkzj[0] = 0x02;
                else
                    C_zkzj[0] = 0x04;
            }
            else
                C_zkzj[0] = 0x0;

            WriteFileContantPkt(0xA6u, 0x10u, g_ZK_DevCode, C_zkzj, 3u);
        }
        else
        {
            if (CPU_A == Get_CPU_Type())  //I系
            {
                if (g_ZK_DevCode == 0x11)  //A模
                    C_zkzj[0] = 0x01;
                else
                    C_zkzj[0] = 0x03;
            }
            else if (CPU_B == Get_CPU_Type())  //II系
            {
                if (g_ZK_DevCode == 0x11)  //A模
                    C_zkzj[0] = 0x02;
                else
                    C_zkzj[0] = 0x04;
            }
            else
                C_zkzj[0] = 0x0;
            WriteFileContantPkt(0xA6u, 0x01u, g_ZK_DevCode, C_zkzj, 3u);
        }
    } /* end if */
} /* end function RecordingZKSelfCheckMessage */

/**********************************************
功能：获取通信1插件自检信息
参数：无
返回：无
***********************************************/
static void RecordingTX1SelfCheckMessage(void)
{
    static uint8_t C_tx1zj[3] = { 0x02U, 0x00U, 0x00U };

    /* 判断自检状态是否发生变化 */
    if (memcmp(&C_tx1zj[1], &TX1_ZJ, 2U))
    {
        memcpy(&C_tx1zj[1], &TX1_ZJ, 2U);

        /* 判断自检为正常或异常 */
        if (NORMAL == TX1_ZJ)
        {
            if (CPU_A == Get_CPU_Type())  //I系
            {
                C_tx1zj[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x11u, 0x31, C_tx1zj, 3u);
            }
            else if (CPU_B == Get_CPU_Type())  //II系
            {
                C_tx1zj[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x11u, 0x32, C_tx1zj, 3u);
            }
            else
                C_tx1zj[0] = 0x00;
        }
        else
        {
            if (CPU_A == Get_CPU_Type())  //I系
            {
                C_tx1zj[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x02u, 0x31, C_tx1zj, 3u);
            }
            else if (CPU_B == Get_CPU_Type())  //II系
            {
                C_tx1zj[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x02u, 0x32, C_tx1zj, 3u);
            }
            else
                C_tx1zj[0] = 0x00;
        }
    } /* end if */
} /* end function RecordingTX1SelfCheckMessage */

/**********************************************
功能：获取通信2插件自检信息
参数：无
返回：无
***********************************************/
static void RecordingTX2SelfCheckMessage(void)
{

} /* end function RecordingTX2SelfCheckMessage */

/**********************************************
功能：获取记录插件自检信息
参数：无
返回：无
***********************************************/
static void RecordingJLSelfCheckMessage(void)
{
    static uint8_t C_jlzj[3] = { 0x03U, 0x00U, 0x00U };

    /* 判断自检状态是否发生变化 */
    if (memcmp(&C_jlzj[1], &JL_ZJ, 2U))
    {
        memcpy(&C_jlzj[1], &JL_ZJ, 2U);

        /* 判断自检为正常或异常 */
        if (NORMAL == JL_ZJ)
        {
            if (Get_CPU_Type() == CPU_A)
            {
                /* I系记录 0x61 */
                C_jlzj[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x13u, 0x61, C_jlzj, 3u);
            }
            else
            {
                /* II系记录 0x62 */
                C_jlzj[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x13u, 0x62, C_jlzj, 3u);
            }
        }
        else
        {
            if (Get_CPU_Type() == CPU_A)
            {
                /* I系记录 0x61 */
                C_jlzj[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x04u, 0x61, C_jlzj, 3u);
            }
            else
            {
                /* II系记录 0x62 */
                C_jlzj[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x04u, 0x62, C_jlzj, 3u);
            }
        }
    } /* end if */
} /* end function RecordingJLSelfCheckMessage */

/**********************************************
功能：获取无线通信插件自检信息
参数：无
返回：无
***********************************************/
static void RecordingWXTXSelfCheckMessage(void)
{
    static uint8_t C_wxtxzj[3] = { 0x00U, 0x00U, 0x00U };

    /* 判断自检状态是否发生变化 */
    if (memcmp(&C_wxtxzj[1], &WXTX_ZJ, 2U ) )
    {
        memcpy( &C_wxtxzj[1], &WXTX_ZJ, 2U);

        /* 判断自检为正常或异常 */
        if( NORMAL == WXTX_ZJ)
        {
            /* 无线通信 0x64 */
            C_wxtxzj[0] = 0x01;
            WriteFileContantPkt( 0xA6u, 0x14u, 0x64, C_wxtxzj, 3u );
        }
        else
        {
            /* 无线通信 0x64 */
            C_wxtxzj[0] = 0x01;
            WriteFileContantPkt( 0xA6u, 0x05u, 0x64, C_wxtxzj, 3u );
        }
    } /* end if */
} /* end function RecordingWXTXSelfCheckMessage */

/**********************************************
功能：获取微机接口自检信息
参数：无
返回：无
***********************************************/
static void RecordingWJJKSelfCheckMessage(void)
{
    static uint8_t C_wjjkzj[3] = { 0x00U, 0x00U, 0x00U };

    /* 判断自检状态是否发生变化 */
    if (memcmp(&C_wjjkzj[1], &WJJK_ZJ, 2U))
    {
        memcpy(&C_wjjkzj[1], &WJJK_ZJ, 2U);

        /* 判断自检为正常或异常 */
        if (NORMAL == WJJK_ZJ)
        {
            if (Get_CPU_Type() == CPU_A)
            {
                /* I系微机接口板 0x51 */
                C_wjjkzj[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x15u, 0x51, C_wjjkzj, 3u);
            }
            else
            {
                /* II系微机接口板 0x52 */
                C_wjjkzj[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x15u, 0x52, C_wjjkzj, 3u);
            }
        }
        else
        {
            if (Get_CPU_Type() == CPU_A)
            {
                /* I系微机接口板 0x51 */
                C_wjjkzj[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x06u, 0x51, C_wjjkzj, 3u);
            }
            else
            {
                /* II系微机接口板 0x52 */
                C_wjjkzj[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x06u, 0x52, C_wjjkzj, 3u);
            }
        }
    } /* end if */
} /* end function RecordingWJJKSelfCheckMessage */

/**********************************************
功能：获取显示器自检信息
参数：无
返回：无
***********************************************/
static void RecordingDMISelfCheckMessage(void)
{
    static uint8_t C_xsqzj1[3] = { 0x01U, 0x00U, 0x00U };
    static uint8_t C_xsqzj2[3] = { 0x02U, 0x00U, 0x00U };

    /* 判断I端显示器自检状态是否发生变化 */
    if (memcmp(&C_xsqzj1[1], &XSQ1_ZJ, 2U))
    {
        memcpy(&C_xsqzj1[1], &XSQ1_ZJ, 2U);

        /* 判断自检为正常或异常 */
        if (NORMAL == XSQ1_ZJ)
        {
            if (Get_CPU_Type() == CPU_A)
            {
                C_xsqzj1[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x16u, 0x21, C_xsqzj1, 3u);
            }
            else
            {
                C_xsqzj1[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x16u, 0x22, C_xsqzj1, 3u);
            }
        }
        else
        {
            if (Get_CPU_Type() == CPU_A)
            {
                C_xsqzj1[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x07u, 0x21, C_xsqzj1, 3u);
            }
            else
            {
                C_xsqzj1[0] = 0x01;
                WriteFileContantPkt(0xA6u, 0x07u, 0x22, C_xsqzj1, 3u);
            }
        }
    } /* end if */

    /* 判断II端显示器自检状态是否发生变化 */
    if (memcmp(&C_xsqzj2[1], &XSQ2_ZJ, 2U))
    {
        memcpy(&C_xsqzj2[1], &XSQ2_ZJ, 2U);

        /* 判断自检为正常或异常 */
        if (NORMAL == XSQ2_ZJ)
        {
            if (Get_CPU_Type() == CPU_A)
            {
                C_xsqzj2[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x16u, 0x21, C_xsqzj2, 3u);
            }
            else
            {
                C_xsqzj2[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x16u, 0x22, C_xsqzj2, 3u);
            }
        }
        else
        {
            if (Get_CPU_Type() == CPU_A)
            {
                C_xsqzj2[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x07u, 0x21, C_xsqzj2, 3u);
            }
            else
            {
                C_xsqzj2[0] = 0x02;
                WriteFileContantPkt(0xA6u, 0x07u, 0x22, C_xsqzj2, 3u);
            }
        }
    } /* end if */
} /* end function RecordingDMISelfCheckMessage */

/**********************************************
功能：获取CEU自检信息
参数：无
返回：无
***********************************************/
static void RecordingCEUSelfCheckMessage(void)
{
  /* 自检故障 */
	/* 故障恢复*/
} /* end function RecordingCEUSelfCheckMessage */

/**********************************************
功能：获取ECU自检信息
参数：无
返回：无
***********************************************/
static void RecordingECUSelfCheckMessage(void)
{
  /* 自检故障 */
	/* 故障恢复 */
} /* end function RecordingECUSelfCheckMessage */

/**********************************************
功能：获取与LKJ通信状态信息
参数：无
返回：无
***********************************************/
static void RecordingCommunicatWithLKJMessage(void)
{
    static uint8_t C_lkj2zj = 0u;
    uint8_t lkj2zjzhi = 0u;

    /* 判断通信间隔 标志 */
    if (C_lkj2zj != LKJ_COMMUNICATION)
    {
        C_lkj2zj = LKJ_COMMUNICATION;

        /* 判断通信状态为异常、中断或恢复 */
        if (RESUME == LKJ_COMMUNICATION)    //通信恢复
        {
//            LOG_I("...LKJ通信恢复...");
            if (Get_CPU_Type() == CPU_A)
            {
                lkj2zjzhi = 0x01;
                WriteFileContantPkt(0xA6, 0x31, g_ZK_DevCode, &lkj2zjzhi, 1u);
            }
            else
            {
                lkj2zjzhi = 0x02;
                WriteFileContantPkt(0xA6, 0x31, g_ZK_DevCode, &lkj2zjzhi, 1u);
            }
        }
        else   //通信中断
        {
//            LOG_I("...LKJ通信中断...");
            if (Get_CPU_Type() == CPU_A)
            {
                lkj2zjzhi = 0x01;
                WriteFileContantPkt(0xA6, 0x25, g_ZK_DevCode, &lkj2zjzhi, 1u);
            }
            else
            {
                lkj2zjzhi = 0x02;
                WriteFileContantPkt(0xA6, 0x25, g_ZK_DevCode, &lkj2zjzhi, 1u);
            }
        }
    } /* end if */
} /* end function RecordingCommunicatWithLKJMessage */

/**********************************************
功能：获取与CCU通信状态信息
参数：无
返回：无
***********************************************/
static void RecordingCommunicatWithCCUMessage(void)
{
    static uint8_t C_ccu2zj = 0u;
    uint8_t ccu2zjzhi = 0u;

    /* 判断通信间隔 标志 */
    if (C_ccu2zj != CCU_COMMUNICATION)
    {
        C_ccu2zj = CCU_COMMUNICATION;

        /* 判断通信状态为异常、中断或恢复 */
        if (RESUME == CCU_COMMUNICATION)
        {
//            LOG_I("...CCU通信恢复...");
            if (Get_CPU_Type() == CPU_A)
            {
                ccu2zjzhi = 0x01;
                WriteFileContantPkt(0xA6, 0x32, g_ZK_DevCode, &ccu2zjzhi, 1u);
            }
            else
            {
                ccu2zjzhi = 0x02;
                WriteFileContantPkt(0xA6, 0x32, g_ZK_DevCode, &ccu2zjzhi, 1u);
            }
        }
        else
        {
//            LOG_I("...CCU通信中断...");
            if (Get_CPU_Type() == CPU_A)
            {
                ccu2zjzhi = 0x01;
                WriteFileContantPkt(0xA6, 0x26, g_ZK_DevCode, &ccu2zjzhi, 1u);
            }
            else
            {
                ccu2zjzhi = 0x02;
                WriteFileContantPkt(0xA6, 0x26, g_ZK_DevCode, &ccu2zjzhi, 1u);
            }
        }
    } /* end if */
} /* end function RecordingCommunicatWithCCUMessage */

/**********************************************
功能：获取与BCU通信状态信息
参数：无
返回：无
***********************************************/
static void RecordingCommunicatWithBCUMessage(void)
{
    static uint8_t C_bcu2zj = 0u;
    uint8_t bcu2zjzhi = 0u;

    /* 判断通信间隔 标志 */
    if (C_bcu2zj != BCU_COMMUNICATION)
    {
        C_bcu2zj = BCU_COMMUNICATION;

        /* 判断通信状态为异常、中断或恢复 */
        if (RESUME == BCU_COMMUNICATION)
        {
//            LOG_I("...BCU通信恢复...");
            if (Get_CPU_Type() == CPU_A)
            {
                bcu2zjzhi = 0x01;
                WriteFileContantPkt(0xA6, 0x33, g_ZK_DevCode, &bcu2zjzhi, 1u);
            }
            else
            {
                bcu2zjzhi = 0x02;
                WriteFileContantPkt(0xA6, 0x33, g_ZK_DevCode, &bcu2zjzhi, 1u);
            }
        }
        else
        {
//            LOG_I("...BCU通信中断...");
            if (Get_CPU_Type() == CPU_A)
            {
                bcu2zjzhi = 0x01;
                WriteFileContantPkt(0xA6, 0x27, g_ZK_DevCode, &bcu2zjzhi, 1u);
            }
            else
            {
                bcu2zjzhi = 0x02;
                WriteFileContantPkt(0xA6, 0x27, g_ZK_DevCode, &bcu2zjzhi, 1u);
            }
        }
    } /* end if */
} /* end function RecordingCommunicatWithBCUMessage */

/**********************************************
功能：获取与CIR通信状态信息
参数：无
返回：无
***********************************************/
static void RecordingCommunicatWithCIRMessage(void)
{
    static uint8_t C_cir2zj = 0u;
    uint8_t cir2zjzhi = 0u;

    /* 判断通信间隔 标志 */
    if (C_cir2zj != CIR_COMMUNICATION)
    {
        C_cir2zj = CIR_COMMUNICATION;

        /* 判断通信状态为异常、中断或恢复 */
        if (0u == CIR_COMMUNICATION)
        {
//            LOG_I("...CIR通信恢复...");
            if (Get_CPU_Type() == CPU_A)
            {
                cir2zjzhi = 0x01;
                WriteFileContantPkt(0xA6, 0x34, g_ZK_DevCode, &cir2zjzhi, 1u);
            }
            else
            {
                cir2zjzhi = 0x02;
                WriteFileContantPkt(0xA6, 0x34, g_ZK_DevCode, &cir2zjzhi, 1u);
            }
        }
        else
        {
//            LOG_I("...CIR通信中断...");
            if (Get_CPU_Type() == CPU_A)
            {
                cir2zjzhi = 0x01;
                WriteFileContantPkt(0xA6, 0x28, g_ZK_DevCode, &cir2zjzhi, 1u);
            }
            else
            {
                cir2zjzhi = 0x02;
                WriteFileContantPkt(0xA6, 0x28, g_ZK_DevCode, &cir2zjzhi, 1u);
            }
        }
    } /* end if */
} /* end function RecordingCommunicatWithCIRMessage */

/**********************************************
功能：获取与CEU通信状态信息
参数：无
返回：无
***********************************************/
static void RecordingCommunicatWithCEUMessage(void)
{
//  static uint8_t C_ceu2zj;
  
//  /* 判断通信间隔 标志 */
//  if ( C_ceu2zj != CEU_COMMUNICATION ) 
//  {
//		C_ceu2zj = CEU_COMMUNICATION;
//    
//    /* 判断通信状态为异常、中断或恢复 */
//    if( RESUME == CEU_COMMUNICATION )
//    {
//		  WriteFileContantPkt( 0xA6, 0x35, 0x01, &C_ceu2zj, 1u );       
//    }
//    else if( ABNORMAL == CEU_COMMUNICATION )
//    {
//		  WriteFileContantPkt( 0xA6, 0x23, 0x01, &C_ceu2zj, 1u );             
//    }
//    else if( SUSPEND == CEU_COMMUNICATION )
//    {
//		  WriteFileContantPkt( 0xA6, 0x29, 0x01, &C_ceu2zj, 1u );      
//    }
//    else
//    {
//      /* 超出取值范围，不做处理 */
//    }
//	} /* end if */
} /* end function RecordingCommunicatWithCEUMessage */

/**********************************************
功能：获取与ECU通信状态信息
参数：无
返回：无
***********************************************/
static void RecordingCommunicatWithECUMessage(void)
{
//  static uint8_t C_ecu2zj;
  
//  /* 判断通信间隔 标志 */
//  if ( C_ecu2zj != ECU_COMMUNICATION ) 
//  {
//		C_ecu2zj = ECU_COMMUNICATION;
//    
//    /* 判断通信状态为异常、中断或恢复 */
//    if( RESUME == ECU_COMMUNICATION )
//    {
//		  WriteFileContantPkt( 0xA6, 0x36, 0x01, &C_ecu2zj, 1u );       
//    }
//    else if( ABNORMAL == ECU_COMMUNICATION )
//    {
//		  WriteFileContantPkt( 0xA6, 0x24, 0x01, &C_ecu2zj, 1u );             
//    }
//    else if( SUSPEND == ECU_COMMUNICATION )
//    {
//		  WriteFileContantPkt( 0xA6, 0x30, 0x01, &C_ecu2zj, 1u );      
//    }
//    else
//    {
//      /* 超出取值范围，不做处理 */
//    }
//	} /* end if */
} /* end function RecordingCommunicatWithECUMessage */

/**********************************************
***********************************************
功能：获取开关机时间信息
参数：无
返回：无
***********************************************/
static void RecordingSwitchTimeMessage( void )
{
    /* 获取日期变化信息 */
    RecordingDateChangeMessage();
} /* end function RecordingSwitchTimeMessage */

/**********************************************
功能：获取记录插件开机信息
参数：无
返回：无
***********************************************/
void RecordingPowerOnMessage(void)
{
    static uint8_t C_waisheleixing[2] = { 0x0u };

    memcpy( &C_waisheleixing, &s_file_head.ch_waisheleixing, 2U);
    WriteFileContantPkt( 0xA7, 0x01, 0x61, C_waisheleixing, 2u );
} /* end function RecordingPowerOnMessage */

/**********************************************
功能：获取记录插件关机信息
参数：无
返回：无
***********************************************/
void RecordingPowerOffMessage(void)
{
    static uint8_t C_waisheleixing[2] = { 0x0u };

    memcpy( &C_waisheleixing, &s_file_head.ch_waisheleixing, 2U);
    WriteFileContantPkt( 0xA7, 0x02, 0x61, C_waisheleixing, 1u );
} /* end function RecordingPowerOffMessage */

/**********************************************
功能：获取日期变化信息
参数：无
返回：无
***********************************************/
static void RecordingDateChangeMessage(void)
{
    static uint8_t C_riqi[3] = { 0x0u };

    if (memcmp(C_riqi, &TIME_NYR, 3U))
    {
        memcpy(C_riqi, &TIME_NYR, 3U);
        WriteFileContantPkt(0xA7, 0x03, g_ZK_DevCode, C_riqi, 3u);
    }
} /* end function RecordingDateChangeMessage */

/**********************************************
***********************************************
功能：获取升级维护信息
参数：无
返回：无
***********************************************/
static void RecordingUpgradeMessage( void )
{

} /* end function RecordingUpgradeMessage */

/**********************************************
***********************************************
功能：获取调试信息
参数：无
返回：无
***********************************************/
static void RecordingDebugMessage( void )
{

} /* end function RecordingDebugMessage */


/**************************************************************************************************
(^_^) Function Name : Update_ABV_ControllingMessage.
(^_^) Brief         : Updating automatic brake valve controlling message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 12-June-2018.
(^_^) Parameter     : msg --> message pointer.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_ABV_ControllingMessage( uint8_t msg[] )
{
    msgRecording.EBV_MSG.ABV_CTRL_Flag  = msg[0];

    msgRecording.EBV_MSG.ABV_CTRL_WC    = msg[1];

    msgRecording.EBV_MSG.ABV_CTRL_VOLT  = ( uint16_t )( ( uint32_t )msg[3] << 8U )\
                                                    + ( uint16_t )msg[2];

    msgRecording.EBV_MSG.ABV_Depressure = msg[4];
} /* end function Update_ABV_ControllingMessage */


/**************************************************************************************************
(^_^) Function Name : Update_IBV_ControllingMessage.
(^_^) Brief         : Updating independent brake valve controlling message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 12-June-2018.
(^_^) Parameter     : msg --> message pointer.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_IBV_ControllingMessage( uint8_t msg[] )
{
    msgRecording.EBV_MSG.IBV_CTRL_Flag  = msg[0];

    msgRecording.EBV_MSG.IBV_CTRL_WC    = msg[1];

    msgRecording.EBV_MSG.IBV_CTRL_VOLT  = ( uint16_t )( ( uint32_t )msg[3] << 8U )\
                                                    + ( uint16_t )msg[2];

    msgRecording.EBV_MSG.IBV_Depressure = msg[4];
} /* end function Update_IBV_ControllingMessage */

/**************************************************************************************************
(^_^) Function Name : Update_ABV_MonitorMessage.
(^_^) Brief         : Updating automatic brake valve monitor message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : msg     --> message pointer.
(^_^)                 low3bit --> low 3-bit of EBV CAN priority.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_ABV_MonitorMessage(uint8_t msg[], uint8_t low3bit)
{
    if (0x01U == low3bit)
    {
        msgRecording.EBV_MSG.ABV_MNT_WC_I = msg[0];

        msgRecording.EBV_MSG.ABV_MNT_VOLT_I = (uint16_t) ((uint32_t) msg[2] << 8U)\
                                            + (uint16_t) msg[1];

        msgRecording.EBV_MSG.ABV_Fault_I = (uint16_t) ((uint32_t) msg[4] << 8U)\
                                            + (uint16_t) msg[3];
    }
    else if (0x05U == low3bit)
    {
        msgRecording.EBV_MSG.ABV_MNT_WC_II = msg[0];

        msgRecording.EBV_MSG.ABV_MNT_VOLT_II = (uint16_t) ((uint32_t) msg[2] << 8U)\
                                            + (uint16_t) msg[1];

        msgRecording.EBV_MSG.ABV_Fault_II = (uint16_t) ((uint32_t) msg[4] << 8U)\
                                            + (uint16_t) msg[3];
    }
    else
    {
    } /* end if...else if...else */
} /* end function Update_ABV_MonitorMessage */


/**************************************************************************************************
(^_^) Function Name : Update_IBV_MonitorMessage.
(^_^) Brief         : Updating independent brake valve monitor message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : msg --> message pointer.
(^_^)                 low3bit --> low 3-bit of EBV CAN priority.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_IBV_MonitorMessage( uint8_t msg[], uint8_t low3bit )
{
  if ( 0x01U == low3bit )
  {
    msgRecording.EBV_MSG.IBV_MNT_WC_I   = msg[0];
    
    msgRecording.EBV_MSG.IBV_MNT_VOLT_I = ( uint16_t )( ( uint32_t )msg[2] << 8U )\
                                                      + ( uint16_t )msg[1];
    
    msgRecording.EBV_MSG.IBV_Fault_I    = ( uint16_t )( ( uint32_t )msg[4] << 8U )\
                                                      + ( uint16_t )msg[3];
  }
  else if ( 0x05U == low3bit )
  {
    msgRecording.EBV_MSG.IBV_MNT_WC_II   = msg[0];
    
    msgRecording.EBV_MSG.IBV_MNT_VOLT_II = ( uint16_t )( ( uint32_t )msg[2] << 8U )\
                                                       + ( uint16_t )msg[1];
    
    msgRecording.EBV_MSG.IBV_Fault_II    = ( uint16_t )( ( uint32_t )msg[4] << 8U )\
                                                       + ( uint16_t )msg[3];
  }
  else
  {
  } /* end if...else if...else */
} /* end function Update_IBV_MonitorMessage */

/**************************************************************************************************
(^_^) Function Name : Update_ABV_HandleMessage.
(^_^) Brief         : Updating automatic brake valve handle message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : msg     --> message pointer.
(^_^)                 low3bit --> low 3-bit of EBV CAN priority.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_ABV_HandleMessage( uint8_t msg[], uint8_t low3bit )
{
  if ( 0x02U == low3bit )
  {
    msgRecording.EBV_MSG.ABV_HAD_WC_I   = msg[0];
    
    msgRecording.EBV_MSG.ABV_HAD_VOLT_I = ( uint16_t )( ( uint32_t )msg[2] << 8U )\
                                                      + ( uint16_t )msg[1];
    
    msgRecording.EBV_MSG.IBR_HAD_WC_I   = msg[3];
  }
  else if ( 0x06U == low3bit )
  {
    msgRecording.EBV_MSG.ABV_HAD_WC_II   = msg[0];
    
    msgRecording.EBV_MSG.ABV_HAD_VOLT_II = ( uint16_t )( ( uint32_t )msg[2] << 8U )\
                                                       + ( uint16_t )msg[1];
    
    msgRecording.EBV_MSG.IBR_HAD_WC_II   = msg[3];
  }
  else
  {
  } /* end if...else if...else */
} /* end function Update_ABV_HandleMessage */


/**************************************************************************************************
(^_^) Function Name : Update_IBV_HandleMessage.
(^_^) Brief         : Updating independent brake valve handle message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-June-2018.
(^_^) Parameter     : msg     --> message pointer.
(^_^)                 low3bit --> low 3-bit of EBV CAN priority.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_IBV_HandleMessage( uint8_t msg[], uint8_t low3bit )
{
  if ( 0x02U == low3bit )
  {
    msgRecording.EBV_MSG.IBV_HAD_WC_I   = msg[0];
    
    msgRecording.EBV_MSG.IBV_HAD_VOLT_I = ( uint16_t )( ( uint32_t )msg[2] << 8U )\
                                                      + ( uint16_t )msg[1];
  }
  else if ( 0x06U == low3bit )
  {
    msgRecording.EBV_MSG.IBV_HAD_WC_II   = msg[0];
    
    msgRecording.EBV_MSG.IBV_HAD_VOLT_II = ( uint16_t )( ( uint32_t )msg[2] << 8U )\
                                                       + ( uint16_t )msg[1];
  }
  else
  {
  } /* end if...else if...else */
} /* end function Update_IBV_HandleMessage */

/**************************************************************************************************
(^_^) Function Name : Update_LKJ_LLRT_Message.
(^_^) Brief         : Updating low-level real-time message of LKJ.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 14-June-2018.
(^_^) Parameter     : none.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_LKJ_LLRT_Message( uint8_t msg[] )
{
  msgRecording.SLKJ_LLRT_MSG.PipePressure = ( uint16_t )( ( uint32_t )msg[1] << 8U )\
                                                       + ( uint16_t )msg[0];
} /* end function Update_HLRT_SpeechTipsMessage */


/**************************************************************************************************
(^_^) Function Name : Update_LKJ_HLRT_Message.
(^_^) Brief         : Updating high-level real-time message of LKJ.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 28-September-2018.
(^_^) Parameter     : none.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_LKJ_HLRT_Message( uint8_t msg[] )
{
  msgRecording.SLKJ_HLRT_MSG.MonitorState = msg[7];
} /* end function Update_LKJ_HLRT_Message */

/**************************************************************************************************
(^_^) Function Name : Update_HLRT_SpeechTipsMessage.
(^_^) Brief         : Updating high-level real-time speech tips message.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 14-June-2018.
(^_^) Parameter     : none.
(^_^) Return        : none.
(^_^) Harware       : none.
(^_^) Software      : none.
(^_^) Note          : Before using the program, you should read comments carefully.
**************************************************************************************************/
void Update_HLRT_SpeechTipsMessage( uint8_t msg[] )
{
  msgRecording.HLRT_MSG.SpeechTips = ( uint16_t )( ( uint32_t )msg[1] << 8U )\
                                                 + ( uint16_t )msg[0];
} /* end function Update_HLRT_SpeechTipsMessage */


/**********************************************************************************************
功能：对数据包进行FF—FE编码
参数：*u8p_SrcData,     ：编码前数组
      u16_SrcLen        ：编码前数组长度
      *u8p_DstData      ：编码后数组
返回：u16_DstP: 编码后数组长度
***********************************************************************************************/
uint16_t FFFEEncode(uint8_t *u8p_SrcData, uint16_t u16_SrcLen, uint8_t *u8p_DstData)
{
    bool bFindNextFF;
    uint8_t u8_NextFFPos;

    uint16_t u16_SrcP, u16_DstP, u16_FFPosP;

    u16_SrcP = 0;
    u16_DstP = 0;

    while (u16_SrcP < u16_SrcLen)
    {
        u8p_DstData[u16_DstP] = u8p_SrcData[u16_SrcP];
        if (u8p_SrcData[u16_SrcP] != 0xFF)
        {
            u16_SrcP++;
            u16_DstP++;
            continue;
        }

        //找到第一个FF
        u16_SrcP++;  //指向FF后面的字符
        u16_DstP++; //预留一个字节，写入下一个FF的偏移
        u16_FFPosP = u16_DstP;  //存储下一个FF的相对位置偏移
        u16_DstP++; //指向后面的需要写入字符的位置

        u8_NextFFPos = 1;
        bFindNextFF = false;
        while (u8_NextFFPos <= 0xFC && u16_SrcP < u16_SrcLen)
        {
            if (u8p_SrcData[u16_SrcP] != 0xFF)
            {
                u8p_DstData[u16_DstP++] = u8p_SrcData[u16_SrcP++];
                u8_NextFFPos++;
            }
            else
            {
                u8p_DstData[u16_FFPosP] = u8_NextFFPos;
                u16_SrcP++;
                bFindNextFF = true;
                break;
            }
        }

        //如果在下面252个数据内未找到下一个FF，或者到数据结束未找到下一个FF
        if (!bFindNextFF)
        {
            u8p_DstData[u16_FFPosP] = 0;
        }
    }

    return u16_DstP;
}

//写文件头
static sint32_t fm_write_record_file_head(S_CURRENT_FILE_INFO *current_file_info)
{
    sint32_t ret;

    if (NULL == current_file_info || NULL == current_file_info->file_head)
    {
        return -1;
    }
    Update_FileHead();
    LOG_I("fm_write_record_file_head");
    ret = write(current_file_info->fd, (char *) current_file_info->file_head, sizeof(SFile_Head));
    fsync(current_file_info->fd);
    return ret;
}

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
static sint32_t fm_modify_record_file_head(S_CURRENT_FILE_INFO *current_file_info)
{
    Update_FileHead();

    lseek(current_file_info->fd, 0, SEEK_SET);
    write(current_file_info->fd, (char *)&s_file_head, sizeof(SFile_Head));

    return 0;
}

#if 0  //用该测试代码时，需要将RECORD_FILE_MAN_SIZE定义改小
static char test_write[RECORD_FILE_MAN_SIZE];
static void ChangeRecord_Condition_Judge(int argc, char **argv)
{
    static char che_ci[4];
    static char si_ji[4];
    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: changerecord [cmd]\n");
        rt_kprintf("       changerecord --checi\n");
        rt_kprintf("       changerecord --siji\n");
        rt_kprintf("       changerecord --wf\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--checi") == 0)
        {
            CHECI += 1;
            LOG_I("change che ci %d", CHECI);
        }
        else if (rt_strcmp(argv[1], "--siji") == 0)
        {
            SIJI1 += 1;

            LOG_I("change si ji %d", SIJI1);
        }
        else if (rt_strcmp(argv[1], "--wf") == 0)
        {
            char full_path[PATH_NAME_MAX_LEN] = {0};

            memset(test_write, 0xaa, RECORD_FILE_MAN_SIZE);
            snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, s_File_Directory.ch_file_name);
            LOG_I("test:%s", full_path);
            if(FMAppendWrite(full_path, (const void *)test_write, RECORD_FILE_MAN_SIZE) < 0)
            {
                LOG_E("test:%s write error", full_path);
            }
        }
        else
        {
            rt_kprintf("Usage: changerecord [cmd]\n");
            rt_kprintf("       changerecord --checi\n");
            rt_kprintf("       changerecord --siji\n");
            rt_kprintf("       changerecord --wf\n");
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(ChangeRecord_Condition_Judge, changerecord, Change Record);
#endif /* RT_USING_FINSH */
#endif

#if 1  //打印
static void FileCreatInfo(int argc, char **argv)
{
//    static char che_ci[4];
//    static char si_ji[4];
    if (argc != 2 && argc != 3)
    {
        rt_kprintf("Usage: fileinfo [cmd]\n");
        rt_kprintf("       fileinfo --pos\n");
        rt_kprintf("       fileinfo --flag\n");
    }
    else
    {
        if (rt_strcmp(argv[1], "--pos") == 0)
        {
            LOG_I("pos %d", write_buf.pos);
        }
        else if(rt_strcmp(argv[1], "--flag") == 0)
        {
            LOG_I("SoftWare_Cycle_Flag %d", SoftWare_Cycle_Flag);
        }
        else
        {
            rt_kprintf("Usage: fileinfo [cmd]\n");
            rt_kprintf("       fileinfo --pos\n");
            rt_kprintf("       fileinfo --flag\n");
        }
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
    MSH_CMD_EXPORT_ALIAS(FileCreatInfo, fileinfo, File Info);
#endif /* RT_USING_FINSH */
#endif

