#include "Record_FileDown.h"

#define DBG_TAG "file down"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>

#include "file_manager.h"
#include "Record_FileCreate.h"
#include "RecordErrorCode.h"
#include "Common.h"
#include "udp_comm.h"
#include "sto_record_board.h"
#include "utils.h"
#include "crc.h"
#include "record_ota.h"

#define ENABLE_FILE_DOWN 0

/* 网络连接标示 */
volatile uint8_t Socket_Connect_Flag = 0u;
volatile uint8_t Socket_Closed_Flag  = 1u;
volatile uint8_t Socket_Receive_Flag = 0u;

/* 13-November-2018, by Liang Zhen. */
/* private macro definition -------------------------------------------------------------------- */
/* Buffer size of RX. */
#define RX_BUFFER_SIZE      ( 1500U )
/* The maximum length of datagram. */
#define DGM_MAX_LENGTH      ( 512U )
/* The header length of datagram. */
#define DGM_HEADER_LENGTH   ( 16U )
#define PROTOCOL_LENGTH     ( DGM_MAX_LENGTH + DGM_HEADER_LENGTH )
/* The length of file segment. */
#define FILE_SEG_LENGTH     ( 508U )

/* Downloading command. */
#define DOWN_CMD_RQHS       ( "RQHS" )
#define DOWN_CMD_ACKR       ( "ACKR" )
#define DOWN_CMD_UPRB       ( "UPRB" )
#define DOWN_CMD_UPRF       ( "UPRF" )

//#define OTA_CMD_ROTA_START  (0x55)
//#define OTA_CMD_ROTA_STOP   (0xAA)


/* private type definition --------------------------------------------------------------------- */

/* State machine. */
typedef enum
{
    /* Idle. */
    DW_SM_IDLE = 0U,

    /* Assert brief. */
    DW_SM_ASSERT_BRIEF,
    /* Assert file. */
    DW_SM_ASSERT_FILE,

    /* Send brief. */
    DW_SM_SEND_BRIEF,
    /* Send file. */
    DW_SM_SEND_FILE,

    /* Ack of brief. */
    DW_SM_ACK_BRIEF,
    /* Ack of file. */
    DW_SM_ACK_FILE,

    /* RECORD OTA */
    DW_SM_OTA_MODE,

    /* Exception. */
    DW_SM_EXCEPTION,
} DOWNLOAD_STATE_MACHINE;

/* The structure of downloading file. */
typedef struct tagDownloadFileStruct
{
    /* Buffer of RX. */
    uint8_t  RX_Buffer[RX_BUFFER_SIZE];
    /* Buffer of TX. */
    uint8_t  TX_Buffer[PROTOCOL_LENGTH];
    /* Recieving status of UDP. */
    UDP_RECV_STATUS udp_recv;
    /* State machine. */
    DOWNLOAD_STATE_MACHINE smDownload;
    /* File number. */
    uint16_t FileNum;    /* 文件编号 */
    /* Brief amount. */
    uint32_t BriefAmount;
    /* Brief address of FRAM. */
    uint32_t BriefAddress;   //目录文件偏移  对应文件系统应该是目录文件编号
    /* Brief number. */
    uint32_t BriefNum;
    /* Ack time. */
    uint32_t AckTime;
    /* File size. */
    uint32_t FileSize;
    /* File address. */
    uint32_t FileAddr;   //文件内的偏移
    /* Datagram amount of file. */
    uint32_t DGM_Amount;
    /* Datagram number of file. */
    uint32_t DGM_Num;

//    RecordOTAMode ota_mode;      /* 0x55启动升级；0xaa关闭升级 */
    uint32_t ota_ack_index;  /* 报文序号 */
} DownloadFileStruct, *pDownloadFileStruct;


/* private constant definition ----------------------------------------------------------------- */
/* Resend limit. */
static uint32_t ResendLimit = 3U;
/* Ack timeout. */
static uint32_t AckTimeout = 450U;
/* File amount. */
static uint32_t FileAmount = 256U;


/* private variable declaration ---------------------------------------------------------------- */
/* The structure of downloading file. */
static DownloadFileStruct downloadFileInst;


/* private function declaration ---------------------------------------------------------------- */
static uint32_t AssertDownloadDatagram( UDP_RECV_STATUS status, uint8_t dgm[] );

static uint32_t Processing_DW_SM_IDLE( uint32_t status, DownloadFileStruct *dwf );
static uint32_t Processing_DW_SM_ASSERT_BRIEF( DownloadFileStruct *dwf );
static uint32_t Processing_DW_SM_ASSERT_FILE( DownloadFileStruct *dwf );
static uint32_t Processing_DW_SM_SEND_BRIEF( DownloadFileStruct *dwf );
static uint32_t Processing_DW_SM_SEND_FILE( DownloadFileStruct *dwf );
static uint32_t GetValidLength( uint32_t num, uint32_t amnt, uint32_t fsize );
static uint32_t Processing_DW_SM_ACK_BRIEF( uint32_t status, DownloadFileStruct *dwf );
static uint32_t AssertAckBrief( DownloadFileStruct *dwf );
static uint32_t Processing_DW_SM_ACK_FILE( uint32_t status, DownloadFileStruct *dwf );
static uint32_t AssertAckFile( DownloadFileStruct *dwf );
static uint32_t Processing_DW_SM_EXCEPTION( DownloadFileStruct *dwf );
static void Processing_DW_ACK_OTA( DownloadFileStruct *dwf );
static uint32_t Processing_DW_SM_OTA( DownloadFileStruct *dwf );



/**************************************************************************************************
(^_^) Function Name : ThreadFileDownload.
(^_^) Brief         : The thread of downloading recording file.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : none.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
void ThreadFileDownload( void )
{
    /* 1. Assert datagram. */
    uint32_t dgm_status = AssertDownloadDatagram( downloadFileInst.udp_recv,\
                                                    downloadFileInst.RX_Buffer );

    if ( dgm_status )
    {
        if ( UDP_RECV_NOTEMPTY == downloadFileInst.udp_recv )
        {
            memset( downloadFileInst.RX_Buffer, 0, RX_BUFFER_SIZE );

            downloadFileInst.udp_recv = UDP_RECV_EMPTY;
        } /* end if */
    } /* end if */
    
    /* 2. Assert state machine. */
    switch ( ( uint32_t )downloadFileInst.smDownload )
    {
        case DW_SM_IDLE :
            Processing_DW_SM_IDLE( dgm_status, &downloadFileInst );
            break;

        case DW_SM_ASSERT_BRIEF :
//            LOG_I("DW_SM_ASSERT_BRIEF");
            Processing_DW_SM_ASSERT_BRIEF( &downloadFileInst );
            break;

        case DW_SM_ASSERT_FILE :
//            LOG_I("DW_SM_ASSERT_FILE");
            Processing_DW_SM_ASSERT_FILE( &downloadFileInst );
            break;

        case DW_SM_SEND_BRIEF :
//            LOG_I("DW_SM_SEND_BRIEF");
            Processing_DW_SM_SEND_BRIEF( &downloadFileInst );
            break;

        case DW_SM_SEND_FILE :
//            LOG_I("DW_SM_SEND_FILE");
            Processing_DW_SM_SEND_FILE( &downloadFileInst );
            break;

        case DW_SM_ACK_BRIEF :
//            LOG_I("DW_SM_ACK_BRIEF");
            Processing_DW_SM_ACK_BRIEF( dgm_status, &downloadFileInst );
            break;

        case DW_SM_ACK_FILE :
//            LOG_I("DW_SM_ACK_FILE");
            Processing_DW_SM_ACK_FILE( dgm_status, &downloadFileInst );
            break;

        case DW_SM_EXCEPTION :
//            LOG_I("DW_SM_EXCEPTION");
            Processing_DW_SM_EXCEPTION( &downloadFileInst );
            break;

        case DW_SM_OTA_MODE:
            Processing_DW_SM_OTA(&downloadFileInst);
            break;
        default :
//            LOG_I("default");
            downloadFileInst.smDownload = DW_SM_IDLE;
            break;
    } /* end switch */
} /* end function ThreadFileDownload */


/**************************************************************************************************
(^_^) Function Name : DownloadFileInit.
(^_^) Brief         : Initializing download file structure.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : none.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t DownloadFileInit( void )
{
    uint32_t exit_code = 0U;

    /* 1. Clear structure. */
    memset( &downloadFileInst, 0U, sizeof( DownloadFileStruct ) );

    /* 2. State machine. */
    downloadFileInst.smDownload = DW_SM_IDLE;

    /* 3. Recieving status of UDP. */
    downloadFileInst.udp_recv = UDP_RECV_EMPTY;

    return exit_code;
} /* end function DownloadFileInit */


/**************************************************************************************************
(^_^) Function Name : GetDownloadDatagram.
(^_^) Brief         : Get downloading datagram.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : dgm  --> the pointer of datagram.
(^_^)                 size --> the size of datagram.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t GetDownloadDatagram( uint8_t dgm[], uint32_t size )
{
    uint32_t exit_code = 0U;

    /* 1. Assert arguments. */
    if ( NULL == dgm )
    {
        exit_code = 1U;
    }
    else if ( ( 0U == size ) || ( size > RX_BUFFER_SIZE ) )
    {
        exit_code = 2U;
    }
    else
    {
        /* 2. Copy datagram. */
        if ( downloadFileInst.udp_recv != UDP_RECV_EMPTY )
        {
            exit_code = 3U;
        }
        else
        {
            memcpy( downloadFileInst.RX_Buffer, dgm, size );

            downloadFileInst.udp_recv = UDP_RECV_NOTEMPTY;
        } /* end if...else */
    } /* end if...else if...else */

    return exit_code;
} /* end function GetDownloadDatagram */


/**************************************************************************************************
(^_^) Function Name : AssertDownloadDatagram.
(^_^) Brief         : Assert downloading file datagram.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : status --> recieving status of UDP.
(^_^)                 dgm --> the pointer of recieving buffer.
(^_^) Return        :   0 --> the datagram is valid.
(^_^)                 > 0 --> the datagram is invalid.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t AssertDownloadDatagram( UDP_RECV_STATUS status, uint8_t dgm[] )
{
    uint32_t exit_code = 0U;
    
    /* 1. Assert arguments. */
    if ( UDP_RECV_EMPTY == status )
    {
        exit_code = 1U;
    }
    else if ( NULL == dgm )
    {
        exit_code = 2U;
    }
    else
    {
        uint16_t dgm_len = ( uint16_t )( ( uint32_t )dgm[12] << 8U )\
                            + ( uint16_t )dgm[13];

        if ( dgm_len > DGM_MAX_LENGTH )
        {
            exit_code = 3U;
        }
        else
        {
            uint16_t dgm_crc = ( uint16_t )( ( uint32_t )dgm[14+dgm_len] << 8U )\
                            + ( uint16_t )dgm[15+dgm_len];

            uint16_t chk_crc = Common_CRC16( dgm, dgm_len + 14U );

            if ( ( uint32_t )dgm_crc ^ ( uint32_t )chk_crc )
            {
                exit_code = 4U;
            }
            else
            {
                exit_code = 0U;
            } /* end if...else */
        } /* end if...else */
    } /* end if...else if...else */

    return exit_code;
} /* end function AssertDownloadDatagram */


/**************************************************************************************************
(^_^) Function Name : Processing_DW_SM_IDLE.
(^_^) Brief         : Processing idle state machine.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : status --> the status of datagram.
(^_^)                 dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_DW_SM_IDLE( uint32_t status, DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;
    
    /* 1. Assert arguments. */
    if ( status )
    {
        exit_code = 1U;
    }
    else if ( NULL == dwf )
    {
        exit_code = 2U;
    }
    else
    {
        /* 2. Assert command. */
        uint32_t hs = strncmp( DOWN_CMD_RQHS, ( const char * )&dwf->RX_Buffer[0], 4U );
        
        if ( hs )
        {
            exit_code = 3U;
        }
        else
        {
            uint32_t sign = strncmp( HS_RB, ( const char * )&dwf->RX_Buffer[16], 2U );
            LOG_I("IDLE udp %c %c %d", dwf->RX_Buffer[16], dwf->RX_Buffer[17], sign);
            if ( sign )
            {
                sign = strncmp( HS_RF, ( const char * )&dwf->RX_Buffer[16], 2U );

                if ( sign )
                {
                    sign = strncmp( HS_APP, ( const char * )&dwf->RX_Buffer[16], 2U );
                    if(sign)
                    {
                        exit_code = 4U;
                    }
                    else
                    {
                        dwf->smDownload = DW_SM_OTA_MODE;
                        exit_code = 0U;

                        if(PC_OTA_START_I == dwf->RX_Buffer[14] && PC_OTA_START_II == dwf->RX_Buffer[15])
                        {
                            RecordOTASetMode(RecordOTAModeUpdata);
                            Processing_DW_ACK_OTA(dwf);
                        }
                        else if(PC_OTA_OVER_I == dwf->RX_Buffer[14] && PC_OTA_OVER_II == dwf->RX_Buffer[15])
                        {
                            RecordOTASetMode(RecordOTAModeNormal);
                            Processing_DW_ACK_OTA(dwf);
                        }
                        else
                        {
                            dwf->smDownload = DW_SM_EXCEPTION;
                            RecordOTASetMode(RecordOTAModeNormal);
                            exit_code = 5U;
                        }
                    }
                }
                else
                {
                    /* Request file. */
                    dwf->smDownload = DW_SM_ASSERT_FILE;

                    dwf->FileNum = ( uint16_t )( ( uint32_t )dwf->RX_Buffer[18] << 8U )\
                                    + ( uint16_t )dwf->RX_Buffer[19];

                    exit_code = 0U;
                } /* end if...else */
            }
            else
            {
                /* Request brief. */
                dwf->smDownload = DW_SM_ASSERT_BRIEF;
                exit_code = 0U;
            } /* end if...else */
        } /* end if...else */

        memset( dwf->RX_Buffer, 0, RX_BUFFER_SIZE );

        dwf->udp_recv = UDP_RECV_EMPTY;
    } /* end if...else */
    
    return exit_code;
} /* end function Processing_DW_SM_IDLE */


/**************************************************************************************************
(^_^) Function Name : Processing_DW_SM_ASSERT_BRIEF.
(^_^) Brief         : Processing assert brief state machine.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_DW_SM_ASSERT_BRIEF( DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;
    S_FILE_MANAGER *fm = &file_manager;

    /* 1. Assert argument. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        dwf->BriefAmount = fm->latest_dir_file_info.dir_num;

        if ( 0U == dwf->BriefAmount )
        {
            dwf->smDownload = DW_SM_EXCEPTION;

            exit_code = 2U;
        }
        else
        {
//            dwf->BriefAddress = 0;//Flash_State.u32_fram_start_addr;  //TODO(mingzhao)
            dwf->BriefAddress  = 1;

            dwf->BriefNum = 1U;

            dwf->smDownload = DW_SM_SEND_BRIEF;

            exit_code = 0U;
        } /* end if...else */
    } /* end if...else */

    return exit_code;
} /* end function Processing_DW_SM_ASSERT_BRIEF */


/**************************************************************************************************
(^_^) Function Name : Processing_DW_SM_ASSERT_FILE.
(^_^) Brief         : Processing assert file state machine.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_DW_SM_ASSERT_FILE( DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;
    S_FILE_MANAGER *fm = &file_manager;

    /* 1. Assert argument. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        if ( ( 0U == dwf->FileNum ) || ( ( uint32_t )dwf->FileNum > FileAmount ) )
        {
            dwf->smDownload = DW_SM_EXCEPTION;

            exit_code = 2U;
        }
        else
        {
            SFile_Directory dir;
            /* 读取目录信息 */
            file_info_t *p_file_list_head = NULL, *p_file = NULL;
            p_file_list_head = get_org_file_info(DIR_FILE_PATH_NAME);
            if(p_file_list_head != NULL)
            {
                p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序 */
                p_file = p_file_list_head;
            }
            else
            {
                exit_code = 2U;
                LOG_W("p_file_list_head = NULL %d",  __LINE__);
                return exit_code;
            }

            /* 查找对应的目录 */
            while (p_file)
            {
//                if(p_file->file_id == ( ( uint32_t )dwf->FileNum - 1U ))
                if(p_file->file_id == ( ( uint32_t )dwf->FileNum))
                {
                    break;
                }
                p_file = p_file->next;
            }
            if(p_file != NULL)
            {
                /* 读目录信息 */
                FMReadDirFile(fm, p_file->dir_name, (void *)&dir, p_file->dir_file_size);  //size = sizeof( SFile_Directory )
            }
            else
            {
                exit_code = 4U;
                LOG_E("can not find id %d dir file %d", ( ( uint32_t )dwf->FileNum), __LINE__);
                free_link(p_file_list_head);
                return exit_code;
            }

            free_link(p_file_list_head);


            dwf->FileSize = dir.u32_file_size;
//            dwf->FileAddr = dir.u32_start_addr;
            dwf->FileAddr = 0;

            dwf->DGM_Amount = dir.u32_file_size / FILE_SEG_LENGTH;
            if ( dir.u32_file_size % FILE_SEG_LENGTH )
            {
                dwf->DGM_Amount++;
            } /* end if */

            /* 21-November-2018, by Liang Zhen. */
#if 0
            dwf->DGM_Num = 1U;
#if 0
            LOG_I( "$$ 2, dwf->FileNum=%d, dwf->FileSize=%d $$",\
                            dwf->FileNum, dwf->FileSize );
#endif
            dwf->smDownload = DW_SM_SEND_FILE;
#else
            if ( 0U == dwf->DGM_Amount )
            {
                dwf->smDownload = DW_SM_EXCEPTION;
                exit_code = 3U;
            }
            else
            {
                dwf->DGM_Num = 1U;
#if 0
                LOG_I( "$$ 2, dwf->FileNum=%d, dwf->FileSize=%d $$",\
                                dwf->FileNum, dwf->FileSize );
#endif
                dwf->smDownload = DW_SM_SEND_FILE;
            } /* end if...else */
#endif
        } /* end if...else */
    } /* end if...else */

    return exit_code;
} /* end function Processing_DW_SM_ASSERT_FILE */


/**************************************************************************************************
(^_^) Function Name : Processing_DW_SM_SEND_BRIEF.
(^_^) Brief         : Processing send brief state machine.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_DW_SM_SEND_BRIEF( DownloadFileStruct *dwf )
{
  uint32_t exit_code = 0U;
  
    /* 1. Assert argument. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        /* 2. Assert brief amount. */
        if ( 0U == dwf->BriefAmount )
        {
            dwf->smDownload = DW_SM_EXCEPTION;

            exit_code = 2U;
        }
        else
        {
            memset( dwf->TX_Buffer, 0U, PROTOCOL_LENGTH );
//            LOG_I( "$$ 1, dwf->BriefAmount=%d, dwf->BriefNum=%d $$", dwf->BriefAmount, dwf->BriefNum );
            LOG_I( "dwf->BriefAmount=%d, BriefNum=%d", dwf->BriefAmount, dwf->BriefNum );
            /* 2.1 Loading command. */
            memcpy( dwf->TX_Buffer, DOWN_CMD_UPRB, 4U );
            /* 2.2 Loading datagram amount. */
            dwf->TX_Buffer[4] = ( uint8_t )( dwf->BriefAmount >> 24U );
            dwf->TX_Buffer[5] = ( uint8_t )( dwf->BriefAmount >> 16U );
            dwf->TX_Buffer[6] = ( uint8_t )( dwf->BriefAmount >> 8U );
            dwf->TX_Buffer[7] = ( uint8_t )( dwf->BriefAmount >> 0U );
            /* 2.3 Loading datagram number. */
            dwf->TX_Buffer[8] = ( uint8_t )( dwf->BriefNum >> 24U );
            dwf->TX_Buffer[9] = ( uint8_t )( dwf->BriefNum >> 16U );
            dwf->TX_Buffer[10] = ( uint8_t )( dwf->BriefNum >> 8U );
            dwf->TX_Buffer[11] = ( uint8_t )( dwf->BriefNum >> 0U );
            /* 2.4 Loading datagram length. */
            /* 4-December-2018, by Liang Zhen. */
#if 0
            dwf->TX_Buffer[12] = 0x00U;
            dwf->TX_Buffer[13] = 0x14U;
#else
            dwf->TX_Buffer[12] = 0x00U;
            dwf->TX_Buffer[13] = 0x20U;
#endif
            /* 2.5 Loading brief number. */
            dwf->TX_Buffer[14] = ( uint8_t )( ( dwf->BriefNum & 0xFFFFU ) >> 8U );
            dwf->TX_Buffer[15] = ( uint8_t )( dwf->BriefNum >> 0U );
            /* 2.5 Loading file name. */
            SFile_Directory fd;
            file_info_t file_info;
            S_FILE_MANAGER *fm = &file_manager;

            if(FMGetIndexFile(fm, DIR_FILE_PATH_NAME, dwf->BriefAddress, &file_info) < 0)
            {
                exit_code = 3U;
                LOG_E("FMGetIndexFile id %d error line %d", dwf->BriefAddress, __LINE__);
                return exit_code;
            }
            else
            {
                if(FMReadDirFile(fm, file_info.dir_name, (void *)&fd, file_info.dir_file_size) < 0)//size = sizeof( SFile_Directory )
                {
                    exit_code = 3U;
                    LOG_E("FMReadDirFile id %d name %s error", dwf->BriefAddress, file_info.dir_name);
                    return exit_code;
                }
            }
            memcpy( &dwf->TX_Buffer[16], fd.ch_file_name, 16U + 8U);
#if 0
            LOG_I("send file name %s ", fd.ch_file_name);
#endif

            /* 4-December-2018, by Liang Zhen. */
#if 0
            /* 2.6 Loading CRC-16. */
            uint16_t crc = Common_CRC16( dwf->TX_Buffer, 34U );
            dwf->TX_Buffer[34] = ( uint8_t )( ( uint32_t )crc >> 8U );
            dwf->TX_Buffer[35] = ( uint8_t )( ( uint32_t )crc >> 0U );
            /* 2.7 Sending datagram. */
            udp_send_datagram( ( char * )dwf->TX_Buffer, 36U );
#else
            /* Loading file size. */
            dwf->TX_Buffer[34 + 8U] = ( uint8_t )( fd.u32_file_size >> 24U );
            dwf->TX_Buffer[35 + 8U] = ( uint8_t )( fd.u32_file_size >> 16U );
            dwf->TX_Buffer[36 + 8U] = ( uint8_t )( fd.u32_file_size >> 8U );
            dwf->TX_Buffer[37 + 8U] = ( uint8_t )( fd.u32_file_size >> 0U );
            /* 2.6 Loading CRC-16. */
            uint16_t crc = Common_CRC16( dwf->TX_Buffer, 38U + 8U );
            dwf->TX_Buffer[38 + 8U] = ( uint8_t )( ( uint32_t )crc >> 8U );
            dwf->TX_Buffer[39 + 8U] = ( uint8_t )( ( uint32_t )crc >> 0U );
            /* 2.7 Sending datagram. */
            UDPServerSendData( ( const void * )dwf->TX_Buffer, 40U + 8U );
#endif
            /* 2.8 Update ack time. */
            dwf->smDownload = DW_SM_ACK_BRIEF;
            dwf->AckTime = rt_tick_get();
        } /* end if...else */
    } /* end if...else */
  
    return exit_code;
} /* end function Processing_DW_SM_SEND_BRIEF */


/**************************************************************************************************
(^_^) Function Name : Processing_DW_SM_SEND_FILE.
(^_^) Brief         : Processing send file state machine.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_DW_SM_SEND_FILE( DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;
    char full_path[TEMP_PATH_NAME_MAX_LEN] = { 0 };
  
    /* 1. Assert argument. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        /* 2. Assert datagram. */
        if ( ( 0U == dwf->DGM_Num ) || ( dwf->DGM_Num > dwf->DGM_Amount ) )
        {
            exit_code = 2U;
        }
        else
        {
            memset( dwf->TX_Buffer, 0U, PROTOCOL_LENGTH );

#if 0
            LOG_I( "$$ 1, dwf->DGM_Amount=%d, dwf->DGM_Num=%d $$",\
            dwf->DGM_Amount, dwf->DGM_Num );
#endif

            /* 2.1 Loading command. */
            memcpy( dwf->TX_Buffer, DOWN_CMD_UPRF, 4U );
            /* 2.2 Loading datagram amount. */
            dwf->TX_Buffer[4] = ( uint8_t )( dwf->DGM_Amount >> 24U );
            dwf->TX_Buffer[5] = ( uint8_t )( dwf->DGM_Amount >> 16U );
            dwf->TX_Buffer[6] = ( uint8_t )( dwf->DGM_Amount >> 8U );
            dwf->TX_Buffer[7] = ( uint8_t )( dwf->DGM_Amount >> 0U );
            /* 2.3 Loading datagram number. */
            dwf->TX_Buffer[8] = ( uint8_t )( dwf->DGM_Num >> 24U );
            dwf->TX_Buffer[9] = ( uint8_t )( dwf->DGM_Num >> 16U );
            dwf->TX_Buffer[10] = ( uint8_t )( dwf->DGM_Num >> 8U );
            dwf->TX_Buffer[11] = ( uint8_t )( dwf->DGM_Num >> 0U );
            /* 2.4 Loading datagram length. */
            dwf->TX_Buffer[12] = ( uint8_t )( DGM_MAX_LENGTH >> 8U );
            dwf->TX_Buffer[13] = ( uint8_t )( DGM_MAX_LENGTH >> 0U );
            /* 2.5 Loading file number. */
            dwf->TX_Buffer[14] = ( uint8_t )( ( uint32_t )dwf->FileNum >> 8U );
            dwf->TX_Buffer[15] = ( uint8_t )( ( uint32_t )dwf->FileNum >> 0U );
            /* 2.6 Loading valid length. */
            uint32_t len = GetValidLength( dwf->DGM_Num, dwf->DGM_Amount, dwf->FileSize );

            dwf->TX_Buffer[16] = ( uint8_t )( len >> 8U );
            dwf->TX_Buffer[17] = ( uint8_t )( len >> 0U );
            /* 2.7 Loading file info. */

            SFile_Directory dir_info;
            file_info_t file_info;
            S_FILE_MANAGER *fm = &file_manager;

//            if(FMGetIndexFile(fm, DIR_FILE_PATH_NAME, (dwf->FileNum - 1), &file_info) < 0)
            if(FMGetIndexFile(fm, DIR_FILE_PATH_NAME, (dwf->FileNum), &file_info) < 0)
            {
                exit_code = 3U;
//                LOG_E("FMGetIndexFile id %d error line %d", (dwf->FileNum - 1), __LINE__);
                LOG_E("FMGetIndexFile id %d error line %d", (dwf->FileNum), __LINE__);
                return exit_code;
            }
            else
            {
                if(FMReadDirFile(fm, file_info.dir_name, (void *)&dir_info, file_info.dir_file_size) < 0)//size = sizeof( SFile_Directory )
                {
                    exit_code = 3U;
                    LOG_E("FMReadDirFile id %d name %s error line %d", dwf->BriefAddress, file_info.dir_name, __LINE__);
                    return exit_code;
                }
            }

            rt_snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, dir_info.ch_file_name);
            if(FMReadFile(fm, full_path, dwf->FileAddr, (void *)&dwf->TX_Buffer[18], len) < 0)
            {
                exit_code =4U;
                LOG_E("read %s error line %d", full_path, __LINE__);
                return exit_code;
            }

            /* 2.8 Loading CRC-16. */
            uint16_t crc = Common_CRC16( dwf->TX_Buffer, PROTOCOL_LENGTH - 2U );
            dwf->TX_Buffer[526] = ( uint8_t )( ( uint32_t )crc >> 8U );
            dwf->TX_Buffer[527] = ( uint8_t )( ( uint32_t )crc >> 0U );
            /* 2.9 Sending datagram. */
            UDPServerSendData( ( char * )dwf->TX_Buffer, PROTOCOL_LENGTH );
            /* 2.8 Update ack time. */
            dwf->smDownload = DW_SM_ACK_FILE;
            dwf->AckTime = rt_tick_get();
        } /* end if...else */
    } /* end if...else */
  
    return exit_code;
} /* end function Processing_DW_SM_SEND_FILE */


/**************************************************************************************************
(^_^) Function Name : GetValidLength.
(^_^) Brief         : Get valid length of datagram.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 14-November-2018.
(^_^) Parameter     : num   --> datagram number.
(^_^)                 amnt  --> datagram amount.
(^_^)                 fsize --> file size.
(^_^) Return        : the valid length of datagram.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t GetValidLength( uint32_t num, uint32_t amnt, uint32_t fsize )
{
    uint32_t len = 0U;
    
    if ( ( 0U == num ) || ( num > amnt ) )
    {
        len = 0U;
    }
    else if ( 0U == fsize )
    {
        len = 0U;
    }
    else
    {
        uint32_t diff = amnt - num;

        if ( diff )
        {
            len = FILE_SEG_LENGTH;
        }
        else
        {
            len = fsize - ( amnt - 1U ) * FILE_SEG_LENGTH;
        } /* end if...else */
    } /* end if...else if...else */

    return len;
} /* end function GetValidLength */


/**************************************************************************************************
(^_^) Function Name : Processing_DW_SM_ACK_BRIEF.
(^_^) Brief         : Processing ack brief state machine.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : status --> the status of datagram.
(^_^)                 dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_DW_SM_ACK_BRIEF( uint32_t status, DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;

    /* 1. Assert arguments. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        if ( Common_BeTimeOutMN( &dwf->AckTime, AckTimeout ) )
        {
            dwf->smDownload = DW_SM_EXCEPTION;

            exit_code = 2U;
        }
        else
        {
            if ( status )
            {
                exit_code = 3U;
            }
            else
            {
                exit_code = AssertAckBrief( dwf );

                memset( dwf->RX_Buffer, 0, RX_BUFFER_SIZE );

                dwf->udp_recv = UDP_RECV_EMPTY;
            } /* end if...else */
        } /* end if...else */
    } /* end if...else */

    return exit_code;
} /* end function Processing_DW_SM_ACK_BRIEF */


/**************************************************************************************************
(^_^) Function Name : AssertAckBrief.
(^_^) Brief         : Assert ack of brief.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t AssertAckBrief( DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;
    static uint32_t resend_cnt = 0U;
  
    /* 1. Assert argument. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        uint32_t ack = strncmp( DOWN_CMD_ACKR, ( const char * )dwf->RX_Buffer, 4U );
        
        if ( ack )
        {
            ack = strncmp( DOWN_CMD_RQHS, ( const char * )&dwf->RX_Buffer[0], 4U );

            if ( ack )
            {
                exit_code = 2U;
            }
            else
            {
                if ( strncmp( HS_RB, ( const char * )&dwf->RX_Buffer[16], 2U ) )
                {
                    exit_code = 4U;
                }
                else
                {
                    /* Request brief. */
                    dwf->smDownload = DW_SM_ASSERT_BRIEF;
                } /* end if...else */
            } /* end if...else */
        }
        else
        {
            if ( 0U == strncmp( ACK_CF, ( const char * )&dwf->RX_Buffer[16], 2U ) )
            {
                resend_cnt++;
                if ( resend_cnt >= ResendLimit )
                {
                    dwf->smDownload = DW_SM_EXCEPTION;
                }
                else
                {
                    dwf->smDownload = DW_SM_SEND_BRIEF;
                } /* end if...else */
            }
            else if ( 0U == strncmp( ACK_NS, ( const char * )&dwf->RX_Buffer[16], 2U ) )
            {
                uint32_t tsn = ( ( uint32_t )dwf->RX_Buffer[20] << 24U )\
                                + ( ( uint32_t )dwf->RX_Buffer[21] << 16U )\
                                + ( ( uint32_t )dwf->RX_Buffer[22] << 8U )\
                                + ( ( uint32_t )dwf->RX_Buffer[23] << 0U );
                                uint32_t fsn = ( ( uint32_t )dwf->RX_Buffer[24] << 24U )\
                                + ( ( uint32_t )dwf->RX_Buffer[25] << 16U )\
                                + ( ( uint32_t )dwf->RX_Buffer[26] << 8U )\
                                + ( ( uint32_t )dwf->RX_Buffer[27] << 0U );

                if ( ( fsn > tsn ) && ( dwf->BriefAmount >= fsn ) )
                {
                    dwf->BriefNum = tsn + 1U;
                    dwf->BriefAddress -= ( fsn - tsn );// * sizeof( SFile_Directory );
                    dwf->BriefAddress += 1;
                    dwf->smDownload = DW_SM_SEND_BRIEF;
//                    LOG_I("brieNum %d address %d line %d", dwf->BriefNum, dwf->BriefAddress, __LINE__);
                } /* end if */
            }
            else if ( 0U == strncmp( ACK_RS, ( const char * )&dwf->RX_Buffer[16], 2U ) )
            {
                resend_cnt = 0U;

                dwf->BriefNum++;

                if ( dwf->BriefAmount >= dwf->BriefNum )
                {
                    dwf->BriefAddress += 1;//sizeof( SFile_Directory );
                    dwf->smDownload = DW_SM_SEND_BRIEF;
//                    LOG_I("brief address %d, num %d, line %d", dwf->BriefAddress, dwf->BriefNum, __LINE__);
                }
                else
                {
                    dwf->smDownload = DW_SM_IDLE;
                } /* end if...else */
            }
            else if ( 0U == strncmp( ACK_RT, ( const char * )&dwf->RX_Buffer[16], 2U ) )
            {
                resend_cnt++;
                if ( resend_cnt >= ResendLimit )
                {
                    dwf->smDownload = DW_SM_EXCEPTION;
                }
                else
                {
                    dwf->smDownload = DW_SM_SEND_BRIEF;
                } /* end if...else */
            }
            else
            {
                /* Unknow sign. */
                exit_code = 3U;
            } /* end if...else */
        } /* end if...else */
    } /* end if...else */
  
    return exit_code;
} /* end function AssertAckBrief */


/**************************************************************************************************
(^_^) Function Name : Processing_DW_SM_ACK_FILE.
(^_^) Brief         : Processing ack file state machine.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : status --> the status of datagram.
(^_^)                 dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_DW_SM_ACK_FILE( uint32_t status, DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;

    /* 1. Assert arguments. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        if ( Common_BeTimeOutMN( &dwf->AckTime, AckTimeout ) )
        {
            dwf->smDownload = DW_SM_EXCEPTION;
#if 0
            LOG_I( "$$ AckTimeout, dwf->DGM_Num=%d $$", dwf->DGM_Num );
#endif
            exit_code = 2U;
        }
        else
        {
            if ( status )
            {
                exit_code = 3U;
            }
            else
            {
                exit_code = AssertAckFile( dwf );

                memset( dwf->RX_Buffer, 0, RX_BUFFER_SIZE );

                dwf->udp_recv = UDP_RECV_EMPTY;
            } /* end if...else */
        } /* end if...else */
    } /* end if...else */

    return exit_code;
} /* end function Processing_DW_SM_ACK_FILE */


/**************************************************************************************************
(^_^) Function Name : AssertAckFile.
(^_^) Brief         : Assert ack of file.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t AssertAckFile( DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;
    static uint32_t resend_cnt = 0U;
    
    /* 1. Assert argument. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        uint32_t ack = strncmp( DOWN_CMD_ACKR, ( const char * )dwf->RX_Buffer, 4U );

        if ( ack )
        {
            ack = strncmp( DOWN_CMD_RQHS, ( const char * )dwf->RX_Buffer, 4U );

            if ( ack )
            {
                exit_code = 2U;
#if 0
                LOG_I( "$$ Not DOWN_CMD_RQHS $$" );
#endif
            }
            else
            {
                if ( strncmp( HS_RF, ( const char * )&dwf->RX_Buffer[16], 2U ) )
                {
                    exit_code = 4U;
#if 0
                    LOG_I( "$$ Not HS_RF $$" );
#endif
                }
                else
                {
                    /* Request file. */
                    dwf->smDownload = DW_SM_ASSERT_FILE;

                    dwf->FileNum = ( uint16_t )( ( uint32_t )dwf->RX_Buffer[18] << 8U )\
                                    + ( uint16_t )dwf->RX_Buffer[19];
                } /* end if...else */
            } /* end if...else */
        }
        else
        {
            if ( 0U == strncmp( ACK_CF, ( const char * )&dwf->RX_Buffer[16], 2U ) )
            {
                resend_cnt++;
#if 0
                LOG_I( "$$ ACK_CF $$" );
#endif
                if ( resend_cnt >= ResendLimit )
                {
                    dwf->smDownload = DW_SM_EXCEPTION;
                }
                else
                {
                    dwf->smDownload = DW_SM_SEND_FILE;
                } /* end if...else */
            }
            else if ( 0U == strncmp( ACK_NS, ( const char * )&dwf->RX_Buffer[16], 2U ) )
            {
                uint32_t tsn = ( ( uint32_t )dwf->RX_Buffer[20] << 24U )\
                                + ( ( uint32_t )dwf->RX_Buffer[21] << 16U )\
                                + ( ( uint32_t )dwf->RX_Buffer[22] << 8U )\
                                + ( ( uint32_t )dwf->RX_Buffer[23] << 0U );
                uint32_t fsn = ( ( uint32_t )dwf->RX_Buffer[24] << 24U )\
                                + ( ( uint32_t )dwf->RX_Buffer[25] << 16U )\
                                + ( ( uint32_t )dwf->RX_Buffer[26] << 8U )\
                                + ( ( uint32_t )dwf->RX_Buffer[27] << 0U );
#if 0
                LOG_I( "$$ ACK_NS $$" );
#endif
                if ( ( fsn > tsn ) && ( dwf->DGM_Amount >= fsn ) )
                {
                    dwf->DGM_Num = tsn + 1U;
                    dwf->FileAddr -= ( fsn - tsn ) * FILE_SEG_LENGTH;
                    dwf->smDownload = DW_SM_SEND_FILE;
                } /* end if */
            }
            else if ( 0U == strncmp( ACK_RS, ( const char * )&dwf->RX_Buffer[16], 2U ) )
            {
                resend_cnt = 0U;

                dwf->DGM_Num++;

                if ( dwf->DGM_Amount >= dwf->DGM_Num )
                {
                    dwf->FileAddr += FILE_SEG_LENGTH;
                    dwf->smDownload = DW_SM_SEND_FILE;
                }
                else
                {
                    dwf->smDownload = DW_SM_IDLE;
                } /* end if...else */
            }
            else if ( 0U == strncmp( ACK_RT, ( const char * )&dwf->RX_Buffer[16], 2U ) )
            {
                resend_cnt++;
#if 0
                LOG_I( "$$ ACK_RT $$" );
#endif
                if ( resend_cnt >= ResendLimit )
                {
                    dwf->smDownload = DW_SM_EXCEPTION;
                }
                else
                {
                    dwf->smDownload = DW_SM_SEND_FILE;
                } /* end if...else */
            }
            else
            {
                /* Unknown sign. */
                exit_code = 3U;
#if 0
                LOG_I( "$$ Unknown $$" );
#endif
            } /* end if...else */
        } /* end if...else */
    } /* end if...else */

    return exit_code;
} /* end function AssertAckFile */


/**************************************************************************************************
(^_^) Function Name : Processing_DW_SM_EXCEPTION.
(^_^) Brief         : Processing exception state machine.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 13-November-2018.
(^_^) Parameter     : dwf --> the pointer of downloading file structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_DW_SM_EXCEPTION( DownloadFileStruct *dwf )
{
    uint32_t exit_code = 0U;

    /* 1. Assert argument. */
    if ( NULL == dwf )
    {
        exit_code = 1U;
    }
    else
    {
        dwf->smDownload = DW_SM_IDLE;
    } /* end if...else */

    return exit_code;
} /* end function Processing_DW_SM_EXCEPTION */


static void Processing_DW_ACK_OTA( DownloadFileStruct *dwf )
{
    uint32_t expectation_index = 0;
    RecordOTAMode current_ota_mode = RecordOTAModeNormal;

    memset( dwf->TX_Buffer, 0U, PROTOCOL_LENGTH );
    memcpy( dwf->TX_Buffer, DOWN_CMD_ACKR, 4U );

    /* 报文总数 */
    dwf->BriefAmount = 1;
    dwf->TX_Buffer[4] = ( uint8_t )( dwf->BriefAmount >> 24U );
    dwf->TX_Buffer[5] = ( uint8_t )( dwf->BriefAmount >> 16U );
    dwf->TX_Buffer[6] = ( uint8_t )( dwf->BriefAmount >> 8U );
    dwf->TX_Buffer[7] = ( uint8_t )( dwf->BriefAmount >> 0U );

    /* 报文序号 */
    dwf->BriefAmount = 1;
    dwf->TX_Buffer[8] = ( uint8_t )( dwf->BriefNum >> 24U );
    dwf->TX_Buffer[9] = ( uint8_t )( dwf->BriefNum >> 16U );
    dwf->TX_Buffer[10] = ( uint8_t )( dwf->BriefNum >> 8U );
    dwf->TX_Buffer[11] = ( uint8_t )( dwf->BriefNum >> 0U );

    /* 报文长度 */
    dwf->TX_Buffer[12] = 0x00U;
    dwf->TX_Buffer[13] = 0x0EU;

    /* 报文内容 */
    dwf->TX_Buffer[14] = 0;
    dwf->TX_Buffer[15] = 0;

    current_ota_mode = RecordOTAGetMode();

//    if(RecordOTAModeUpdata == dwf->ota_mode || RecordOTAModeUpdataAgain == dwf->ota_mode)
    if(RecordOTAModeUpdata == current_ota_mode || RecordOTAModeUpdataAgain == current_ota_mode)
    {
        dwf->TX_Buffer[14] = PC_OTA_START_I;
        dwf->TX_Buffer[15] = PC_OTA_START_II;
    }
    else
    {
        dwf->TX_Buffer[14] = PC_OTA_OVER_I;
        dwf->TX_Buffer[15] = PC_OTA_OVER_II;
    }

    memcpy( &dwf->TX_Buffer[16], ACK_RS, 2U );//16 17

    memcpy( &dwf->TX_Buffer[18], HS_APP, 2U );//18 19

    /*  */
    dwf->ota_ack_index += 1;
    memcpy( &dwf->TX_Buffer[20], &dwf->ota_ack_index, 4U );//20 23

    expectation_index = dwf->ota_ack_index + 1;
    memcpy( &dwf->TX_Buffer[24], &expectation_index, 4U );//24 27

    /* crc */
    uint16_t crc = Common_CRC16( dwf->TX_Buffer, 28U );
    dwf->TX_Buffer[28] = ( uint8_t )( ( uint32_t )crc >> 8U );
    dwf->TX_Buffer[29] = ( uint8_t )( ( uint32_t )crc >> 0U );

//    LOG_HEX("udp ack", 16, dwf->TX_Buffer, 30);
    /* 应答报文 */
    UDPServerSendData( ( const void * )dwf->TX_Buffer, 30 );
}

static uint32_t Processing_DW_SM_OTA( DownloadFileStruct *dwf )
{
    uint32_t sign = 0;

//    RecordOTASetMode(dwf->ota_mode);
//    if(RecordOTAModeNormal == dwf->ota_mode)
//    {
//        dwf->smDownload = DW_SM_IDLE;
//    }

    if(RecordOTAGetMode() == RecordOTAModeNormal)
    {
        dwf->smDownload = DW_SM_IDLE;
    }

    sign = strncmp( HS_APP, ( const char * )&dwf->RX_Buffer[16], 2U );
    if(0 == sign)
    {
        dwf->smDownload = DW_SM_OTA_MODE;

        if(PC_OTA_START_I == dwf->RX_Buffer[14] && PC_OTA_START_II == dwf->RX_Buffer[15])
        {
//            dwf->ota_mode = RecordOTAModeUpdataAgain;
            RecordOTASetMode(RecordOTAModeUpdataAgain);

            memset( dwf->RX_Buffer, 0, RX_BUFFER_SIZE );
            dwf->udp_recv = UDP_RECV_EMPTY;
            Processing_DW_ACK_OTA(dwf);
//            LOG_I("2-start ota");
        }
        else if(PC_OTA_OVER_I == dwf->RX_Buffer[14] && PC_OTA_OVER_II == dwf->RX_Buffer[15])
        {
//            dwf->ota_mode = RecordOTAModeNormal;
            RecordOTASetMode(RecordOTAModeNormal);

            memset( dwf->RX_Buffer, 0, RX_BUFFER_SIZE );
            dwf->udp_recv = UDP_RECV_EMPTY;
            Processing_DW_ACK_OTA(dwf);
//            LOG_I("2-stop ota");
        }
        else
        {
            dwf->smDownload = DW_SM_EXCEPTION;

            memset( dwf->RX_Buffer, 0, RX_BUFFER_SIZE );
            dwf->udp_recv = UDP_RECV_EMPTY;

//            dwf->ota_mode = RecordOTAModeNormal;
            RecordOTASetMode(RecordOTAModeNormal);
        }
    }

    return 0;
}

/**************************************************************************************************
(^_^) Function Name : 
(^_^) Brief         : 
(^_^) Author        : 
(^_^) Date          : 
(^_^) Parameter     : 
(^_^) Return        : 
(^_^) Hardware      : 
(^_^) Software      : 
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/


