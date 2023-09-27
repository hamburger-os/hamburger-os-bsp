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

#define ENABLE_FILE_DOWN 0

/* 网络连接标示 */
volatile uint8_t Socket_Connect_Flag = 0u;
volatile uint8_t Socket_Closed_Flag  = 1u;
volatile uint8_t Socket_Receive_Flag = 0u;

enum APPLY_CMD ApplyCmd = NONECMD;

static void Read_File(SFile_Directory directory, uint8_t ID );
static void Send_File( void );
#if 0
static void Get_FileName(SFile_Directory directory,uint8_t *file_name);
#endif
static void Send_FileDirectory( void );
static void ETH_RecProc( void );


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


#if ENABLE_FILE_DOWN
/********************************************************************************************
** @brief: RecordBoard_DownFile
** @param: null
********************************************************************************************/
extern void RecordBoard_FileDown( void )
{
    if ( Socket_Receive_Flag )
    {
        Socket_Receive_Flag = 0u;
        ETH_RecProc();
    } /*  end if */

    if ( !Socket_Closed_Flag && ApplyCmd == DIRECTORYCMD )
    {
        LOG_I( "A connect success" );
        ApplyCmd = NONECMD;
        Send_FileDirectory();
    }
    else if ( !Socket_Closed_Flag && ApplyCmd == FILECMD )
    {
        ApplyCmd = NONECMD;
        Send_File();
    }
    else if ( Socket_Closed_Flag )
    {
        LOG_I( "tcp close" );
        Socket_Closed_Flag = 0u;
        ApplyCmd = NONECMD;
    }
    else
    {
    } /* end if...else if......else */

    if ( Socket_Connect_Flag )
    {
        Socket_Connect_Flag = 0u;
        LOG_I( "Connected ack" );
        char ack_buf[5] = { 0x55u, 0xaau, 'A', 'C', 'K' };
        ETH_send( ack_buf, 5u );
    }
    else
    {
    } /* end if...else */
}


/********************************************************************************************
** @brief: Send_File
** @param: null
********************************************************************************************/

static void Send_File( void )
{
    ETH_REC *p;
    uint32_t i;
    SFile_Directory directory;
    S_FILE_MANAGER *fm = &file_manager;

    p = (ETH_REC *)UDPServerGetRcvDataBuf();

    file_info_t *p_file_list_head = NULL, *p_file = NULL;
    p_file_list_head = get_org_file_info(DIR_FILE_PATH_NAME);
    if(p_file_list_head != NULL)
    {
        p_file_list_head = sort_link(p_file_list_head, SORT_UP); /* 按照文件序号,由小到大排序 */
        p_file = p_file_list_head;
    }
    else
    {
        LOG_W("p_file_list_head = NULL line %d", __LINE__);
        return;
    }

    for ( i = 0u; i < p->file_count; i++ )
    {
        /* 无效ID */
        if ( p->ID[i] >= fm->latest_dir_file_info.dir_num )
        {
            return;
        } /* end if */

        /* 查找对应的目录 */
        while (p_file)
        {
            if(p_file->file_id == p->ID[i])
            {
                break;
            }
            p_file = p_file->next;
        }
        if(p_file != NULL)
        {
            /* 读目录信息 */
            FMReadDirFile(fm, p_file->dir_name, (void *)&directory, p_file->dir_file_size);  //size = sizeof( SFile_Directory )

            Read_File(directory, p->ID[i] );
        }
        else
        {
            LOG_E("can not find id %d dir file %d", p->ID[i], __LINE__);
            free_link(p_file_list_head);
            return;
        }
    } /* end for */
    free_link(p_file_list_head);
}

/********************************************************************************************
** @brief: copy_head_to_buf
** @param: null
********************************************************************************************/
static void copy_head_to_buf( char buf[], PACKAGE_HEAD *p_head )
{
    if ( ( NULL != buf ) && ( NULL != p_head ) )
    {
        buf[0] = p_head->file_num;
        buf[1] = p_head->file_count;
        buf[2] = p_head->lenth;

        memcpy( &buf[3],  ( char * )&p_head->total_package, 4u );
        memcpy( &buf[7],  ( char * )&p_head->current_package, 4u );
        memcpy( &buf[11], ( char * )&p_head->crc, 2u );
        memcpy( &buf[13], ( char * )&p_head->file_name[0], 24u );
        buf[37] = 0x00u;
    }
}


/********************************************************************************************
** @brief: Send_File
** @param: null
********************************************************************************************/
static void Read_File(SFile_Directory directory, uint8_t ID )
{
    ETH_REC *p;

    uint32_t i = 0u, page_count = 0u;
    PACKAGE_HEAD package_head = { 0u };
    S_FILE_MANAGER *fm = &file_manager;
    char full_path[PATH_NAME_MAX_LEN] = { 0 };

    p = (ETH_REC *)UDPServerGetRcvDataBuf();
    /* 生成包头信息 */
    package_head.file_num   = ID;
    package_head.file_count = p->file_count;
    /*如果是当前记录文件,发送最后缓冲区数据 */
    if ( directory.u32_over_flag == 0u )
    {
        directory.u32_file_size += write_buf.pos;
        /*directory.page_count+=1;*/
    } /* end if */

    package_head.total_package = directory.u32_file_size / 100u\
                                    + ( ( directory.u32_file_size % 100u ) ? 1u : 0u );

    package_head.current_package = 0u;
    memcpy( package_head.file_name, directory.ch_file_name, FILE_NAME_MAX_NUM);

    LOG_I( "file name is %s", directory.ch_file_name );
	
//	uint32_t addr = directory.u32_start_addr;
	uint32_t addr = 0; //文件开始位置
	char read_buf[ 256u ] = { 0u }, send_buf[ 128u ] = { 0u };
	uint32_t SendBuf_pos = 28u, SendRest_size = 100u, ReadBuf_size = 0u, ReadRest_size = 0u;

	LOG_I( "read file" );
  
	page_count = directory.u32_file_size / 256u + ( ( directory.u32_file_size % 256u ) ? 1u : 0u );
  
    for ( i = 0u; i < page_count; i++ )
    {
        memset( read_buf, 0u, 256u );

        if ( ( i == page_count - 1u ) && ( directory.u32_over_flag == 0u ) )
        {
            memcpy( read_buf, write_buf.buf, 256u );
            ReadBuf_size = directory.u32_file_size % 256u;
            if ( ReadBuf_size == 0u )
            {
                ReadBuf_size = 256u;
            } /* end if */
        }
        else if ( i == page_count - 1u )
        {
//            /* 10-May-2018, by Liang Zhen. */  TODO(mingzhao)
//            #if 0
//            S25FL1D_Read( ( uint32_t * )&read_buf, sizeof( read_buf ), addr );
//            #else
//            S25FL256S_Read( ( uint32_t * )&read_buf, sizeof( read_buf ), addr );
//            #endif

            snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, directory.ch_file_name);
            if(FMReadFile(fm, full_path, addr, (void *)&read_buf, sizeof( read_buf )) < 0)
            {
                LOG_E("read %s error", full_path);
                return;
            }

            ReadBuf_size = directory.u32_file_size % 256u;
            if ( ReadBuf_size == 0u )
            {
                ReadBuf_size = 256u;
            } /* end if */
        }
        else
        {
//            /* 10-May-2018, by Liang Zhen. */   TODO(mingzhao)
//            #if 0
//            S25FL1D_Read( ( uint32_t * )&read_buf, sizeof( read_buf ), addr );
//            #else
//            S25FL256S_Read( ( uint32_t * )&read_buf, sizeof( read_buf ), addr );
//            #endif

            snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, directory.ch_file_name);
            if(FMReadFile(fm, full_path, addr, (void *)&read_buf, sizeof( read_buf )) < 0)
            {
                LOG_E("read %s error", full_path);
                return;
            }

            ReadBuf_size = 256u;
        } /* end if...else if...else */

        ReadRest_size = ReadBuf_size;
        /* 放入缓冲区 */
        /*
        SendRest_size=128-SendBuf_pos;
        printf("send rest size is %d,send pos is %d\r\n",SendRest_size,SendBuf_pos);
        */
        
        /* 17-August-2018, by Liang Zhen. */
//        static unsigned int uart_sync = 0U;
        
        while ( 1u )
        {
            if ( SendRest_size > ReadRest_size )
            {
                memcpy( &send_buf[ SendBuf_pos ],\
                        &read_buf[ ReadBuf_size - ReadRest_size ],\
                        ReadRest_size );

                SendBuf_pos   += ReadRest_size;
                SendRest_size -= ReadRest_size;
                memset( &send_buf[ SendBuf_pos ], 0u, SendRest_size );

                break;
            }
            else
            {
                memcpy( &send_buf[ SendBuf_pos ],\
                        &read_buf[ ReadBuf_size - ReadRest_size ],\
                        SendRest_size );

                /* 网口发送 */
                package_head.lenth = 100u;
                copy_head_to_buf( send_buf, &package_head );
                LOG_I( "package num is %d", package_head.current_package );
                ETH_send( send_buf, 128u );
                package_head.current_package += 1u;
                SendBuf_pos    = 28u;
                ReadRest_size -= SendRest_size;
                SendRest_size  = 100u;
            } /* end if...else */

//            /* 17-August-2018, by Liang Zhen. */  //TODO(mingzhao)
//            if( Common_BeTimeOutMN( &uart_sync, 1000U ) )
//            {
//                if( CPU_A == Get_CPU_Type() )
//                {
//                    UART1_Send( 5U );
//                    #if 0
//                    printf( "\r\n--> %d\r\n", uart_sync );
//                    #endif
//                } /* end if */
//            } /* end if */
        } /* end while */
        
        addr += FILE_PAGE_SIZE;
//        if ( addr == FLASH_RecordFile_MAX_ADDR )
        if ( addr >= directory.u32_file_size )
        {
            addr = 0;   //该出去进行下一个文件了
        } /* end if */
    } /* end for */
	
    if ( SendBuf_pos != 28u )
    {
        LOG_I( "not full\r\n" );
        /* 网口发送 */
        package_head.lenth = SendBuf_pos - 28u;
        copy_head_to_buf( send_buf, &package_head );
        ETH_send( send_buf, 128u );
        package_head.current_package += 1u;
    } /*  end if */
}

static void Send_FileDirectory( void )
{
	ETH_SEND eth_buf = { 0u };
    S_FILE_MANAGER *fm = &file_manager;


    if ( 0u == fm->latest_dir_file_info.dir_num )
    {
        /* 组包头 */
        eth_buf.flag[0]  = 0xeeU;
        eth_buf.flag[1]  = 0xffU;
        eth_buf.file_sum    = 0u;
        eth_buf.package_sum = 0u;
        ETH_send( ( void* )&eth_buf, 128u );
    }
    else
    {
        /* 发送目录，每包发送6个 */
        uint8_t  package_sum = fm->latest_dir_file_info.dir_num / 6u + ( ( fm->latest_dir_file_info.dir_num % 6u ) ? 1u : 0u );
        uint8_t  packageID   = 0u, fileID = 0u, i = 0u;
        uint32_t addr = 0;
        SFile_Directory directory;

        /* 组包头 */
        eth_buf.flag[0]  = 0xeeU;
        eth_buf.flag[1]  = 0xffU;
        eth_buf.file_sum    = fm->latest_dir_file_info.dir_num;
        eth_buf.package_sum = package_sum;

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
            LOG_W("p_file_list_head = NULL %d",  __LINE__);
            return;
        }

        for ( packageID = 0u; packageID < package_sum; packageID++ )
        {
            if ( packageID < package_sum - 1u )
            {
                eth_buf.file_count = 6u;
                eth_buf.packageID  = packageID;
                for ( i = 0u; i < eth_buf.file_count; i++ )
                {
//                    FM25V05_Manage_ReadData( addr, ( uint8_t * )&directory, sizeof( SFile_Directory ) );
//                    addr += sizeof( SFile_Directory );
//                    if ( addr == FRAM_MAX_ADDR )
//                    {
//                        addr = FRAM_BASE_ADDR;
//                    } /* end if */


                    /* 查找对应的目录 */
                    while (p_file)
                    {
                        if(p_file->file_id == fileID)
                        {
                            break;
                        }
                        p_file = p_file->next;
                    }

                    if(p_file != NULL)
                    {
                        /* 读目录信息 */
                        FMReadDirFile(fm, p_file->dir_name, (void *)&directory, p_file->dir_file_size);  //size = sizeof( SFile_Directory )

                        memcpy( eth_buf.contant[i].name, directory.ch_file_name,  24u );
                        memcpy( eth_buf.contant[i].size, &directory.u32_file_size, 4u );
                        eth_buf.contant[i].fileID = fileID;
                        fileID++;
                    }
                    else
                    {
                        LOG_E("can not find id %d dir file %d", fileID, __LINE__);
                        free_link(p_file_list_head);
                        return;
                    }
                } /* end for */

                ETH_send( ( void* )&eth_buf, 128u );
            }
            /* 最后一包 */
            else
            {
                //*eth_buf.u32_file_count=Flash_State.u32_file_count%6;*/
                /* chengt by chengt----20171108 */
                eth_buf.file_count = ( fm->latest_dir_file_info.dir_num - packageID * 6u ) % 7u;

                eth_buf.packageID  = packageID;
                for ( i = 0u; i < eth_buf.file_count; i++ )
                {
//                    FM25V05_Manage_ReadData( addr, ( uint8_t * )&directory, sizeof( SFile_Directory ) );
//                    addr += sizeof( SFile_Directory );
//                    if ( addr == FRAM_MAX_ADDR )
//                    {
//                        addr = FRAM_BASE_ADDR;
//                    } /*  end if */

                    /* 查找对应的目录 */
                    while (p_file)
                    {
                        if(p_file->file_id == fileID)
                        {
                            break;
                        }
                        p_file = p_file->next;
                    }

                    if(p_file != NULL)
                    {
                        /* 读目录信息 */
                        FMReadDirFile(fm, p_file->dir_name, (void *)&directory, p_file->dir_file_size);  //size = sizeof( SFile_Directory )
                        memcpy( eth_buf.contant[i].name, directory.ch_file_name, 24u );
                        memcpy( eth_buf.contant[i].size, &directory.u32_file_size, 4u );
                        eth_buf.contant[i].fileID = fileID;
                        fileID++;
                    }
                    else
                    {
                        LOG_E("can not find id %d dir file %d", fileID, __LINE__);
                        free_link(p_file_list_head);
                        return;
                    }

                } /*  end for */

                if ( i != 6u )
                {
                    memset( &eth_buf.contant[i], 0u, ( 6 - i ) * sizeof( NAME_ID ) );
                } /* end if */

                ETH_send( ( void* )&eth_buf, 128u );
            } /*  end if...else */
        } /* end for */
        free_link(p_file_list_head);
    } /* end if...else */
}

static void ETH_RecProc( void )
{
    ETH_REC *p;

    p = (ETH_REC *)UDPServerGetRcvDataBuf();

    /* 28-September-2018, by Liang Zhen. */
//    GetNewDatagram( tcp_server_recvbuf, TCP_SERVER_RX_BUFSIZE );  //TODO(mingzhao)

    if ( p->flag[0] != 0xeeU && p->flag[1] != 0xffU )
    {
        return;
    } /* end if */

    if ( p->file_count == 0u )
    {
        ApplyCmd = DIRECTORYCMD;
    }
    else
    {
        ApplyCmd = FILECMD;
    } /* end if...else */
}

#endif
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
            LOG_I("UDP_RECV_NOTEMPTY");
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

            if ( sign )
            {
                sign = strncmp( HS_RF, ( const char * )&dwf->RX_Buffer[16], 2U );

                if ( sign )
                {
                    exit_code = 4U;
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
            dwf->BriefAddress = 0;//Flash_State.u32_fram_start_addr;  //TODO(mingzhao)

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
//            uint32_t dir_addr = Flash_State.u32_fram_start_addr\
//                                    + ( ( uint32_t )dwf->FileNum - 1U ) * sizeof( SFile_Directory );
//            SFile_Directory dir;
//
//            FM25V05_Manage_ReadData( dir_addr, ( uint8_t * )&dir, sizeof( SFile_Directory ) );

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
                LOG_W("p_file_list_head = NULL %d",  __LINE__);
                return;
            }

            /* 查找对应的目录 */
            while (p_file)
            {
                if(p_file->file_id == ( ( uint32_t )dwf->FileNum - 1U ))
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
                LOG_E("can not find id %d dir file %d", ( ( uint32_t )dwf->FileNum - 1U ), __LINE__);
                free_link(p_file_list_head);
                return;
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
#if 0
            LOG_I( "$$ 1, dwf->BriefAmount=%d, dwf->BriefNum=%d $$",\
                                dwf->BriefAmount, dwf->BriefNum );
#endif
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
#if 1
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

            if(FMGetIndexFile(fm, DIR_FILE_PATH_NAME, (dwf->FileNum - 1), &file_info) < 0)
            {
                exit_code = 3U;
                LOG_E("FMGetIndexFile id %d error line %d", (dwf->FileNum - 1), __LINE__);
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

            char full_path[PATH_NAME_MAX_LEN] = { 0 };

            snprintf(full_path, sizeof(full_path), "%s/%s", RECORD_FILE_PATH_NAME, dir_info.ch_file_name);
            if(FMReadFile(fm, full_path, dwf->FileAddr, (void *)&dwf->TX_Buffer[18], len) < 0)
            {
                LOG_E("read %s error line %d", full_path, __LINE__);
                return;
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
                    dwf->smDownload = DW_SM_SEND_BRIEF;
                    LOG_I("brieNum %d address %d line %d", dwf->BriefNum, dwf->BriefAddress, __LINE__);
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
                    LOG_I("brief address %d, num %d, line %d", dwf->BriefAddress, dwf->BriefNum, __LINE__);
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


