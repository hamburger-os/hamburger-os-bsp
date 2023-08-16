/**************************************************************************************************
(^_^) File Name  : RecordErrorCode.c
(^_^) Brief      : It is the error code recording source file of project.
(^_^) Author     : Liang Zhen.
(^_^) Date       : 4-September-2018.
(^_^) Version    : V1.0.0.1
(^_^) Revision   : 14-January-2019, by Liang Zhen.
(^_^) Company    : 北京思维鑫科信息技术有限公司.
(^_^) Copyright  : All rights reserved.
(^_^) Platform   : ATSAME70Q21.
(^_^) Disclaimer : none.
(^_^) Note       : Before using the file, please read comments carefully.
**************************************************************************************************/


/* include ------------------------------------------------------------------------------------- */
#include "RecordErrorCode.h"
#include "Record_FileCreate.h"
#include <string.h>


/* private macro definition -------------------------------------------------------------------- */
/* The maximum size of datagram buffer. */
/* 22-October-2018, by Liang Zhen. */
#if 0
#define DGM_BUFFER_SIZE     ( TCP_SERVER_RX_BUFSIZE )
#else
#define DGM_BUFFER_SIZE     ( 1500U )
#endif

/* The maximum length of datagram. */
/* 26-October-2018, by Liang Zhen. */
#if 0
#define DGM_MAX_LEN         ( 64U )
#else
#define DGM_MAX_LEN         ( 512U )
#endif
/* The maximum length of protocol. */
#define PRTC_MAX_LEN        ( DGM_MAX_LEN + 16U )
/* The specification length of datagram. */
#define DGM_SPEC_LEN        ( 36U )

/* The command of datagram. */
/* 24-October-2018, by Liang Zhen. */
#if 0
#define DGM_CMD_RQEC        ( "RQEC" )
#define DGM_CMD_DGCF        ( "DGCF" )
#define DGM_CMD_DGNS        ( "DGNS" )
#define DGM_CMD_DGRS        ( "DGRS" )
#else
#define DGM_CMD_RQHS        ( "RQHS" )
#define DGM_CMD_ACKR        ( "ACKR" )
#endif
#define DGM_CMD_UPEC        ( "UPEC" )

/* The address of SPI flash used for recording error code. */
/* 0x10000 --> 64K. */
#define BLOCK_SIZE          ( 0x10000U )
/* 0x100 --> 256-bytes. */
#define FLASH_PAGE_SIZE     ( 0x100U )
/* 0x100000 --> 1M. */
/* 0x300000 --> 3M. */
#define EC_BASE_ADDR        ( 0x300000U )
/* 0x320000 --> 3.125M. */
#define EC_END_ADDR         ( 0x320000U - 1U )
#if ( EC_BASE_ADDR % BLOCK_SIZE )
  #error "The based address of error code is invalid."
#endif
#if ( ( EC_END_ADDR + 1U ) % BLOCK_SIZE )
  #error "The end address of error code is invalid."
#endif
#if ( EC_BASE_ADDR >= EC_END_ADDR )
  #error "The end address of error code can not be less than based address."
#endif

/* The maximum length of error code recording. */
#define EC_MAX_LEN          ( 128U )
#define EC_DatPkt_MAXLEN    ( 66U )
/* The status of error code. */
#define ERROR_FALSE         ( 'F' )
#define ERROR_TRUE          ( 'T' )

/* The amount of error code for uploading. */
#define EC_UPLOAD_AMNT      ( 360U )
#if ( ( EC_END_ADDR - EC_BASE_ADDR + 1U ) < ( EC_UPLOAD_AMNT * EC_MAX_LEN ) )
  #error "The maximum bytes of error code can not be greater than address arangement."
#endif

/* Timeout of ACK. */
/* 14-November-2018, by Liang Zhen. */
#if 0
/* 500ms. */
#define TIMEOUT_ACK         ( 500U )
#else
/* 450ms. */
#define TIMEOUT_ACK         ( 450U )
#endif

/* The limit of resending. */
#define RESEND_LIMIT        ( 3U )

/* The reboot flag of error code. */
#define EC_REBOOT_FLAG      ( 0U )


/* private type definition --------------------------------------------------------------------- */
/* The state machine of error code. */
typedef enum
{
  /* Idle. */
  EC_SM_IDLE = 0U,
  
  /* Assert. */
  EC_SM_ASSERT,
  
  /* Sending. */
  EC_SM_SEND,
  
  /* Ack. */
  EC_SM_ACK,
  
  /* Exception. */
  EC_SM_EXCEPTION
} EC_STATE_MACHINE;

/* The structure of error code. */
typedef struct tagErrorCodeInitStruct
{
  /* Department catagory. */
  char DPRT[10];
  
  /* The flag of new datagram. */
  LOCKER LockFlag;
  
  /* The buffer of datagram. */
  uint8_t  DGM_Buffer[DGM_BUFFER_SIZE];
  
  /* The buffer of sending. */
  uint8_t  SendBuffer[PRTC_MAX_LEN];
  
  /* The state machine of error code. */
  EC_STATE_MACHINE EC_SM;
  
  /* Year, month, day, hour, minute, second. */
  uint32_t Year;
  uint8_t  Month;
  uint8_t  Day;
  uint8_t  Hour;
  uint8_t  Minute;
  uint8_t  Second;
  
  /* The address of error code for writing flash. */
  uint32_t WriteAddr;
  /* The address of error code for reading flash. */
  uint32_t ReadAddr;
  /* The total size of datagram. */
  uint32_t DGM_TotalSize;
  /* Datagram amount. */
  uint32_t DGM_AMNT;
  /* Datagram number. */
  uint32_t DGM_NUM;
  /* The time of ACK. */
  uint32_t ACK_Time;
  
  /* HMB error flag. */
  uint16_t EF_HMB_A;
  uint16_t EF_HMB_B; 
//  uint16_t EF_HMB_I_A;
//  uint16_t EF_HMB_I_B;
//  uint16_t EF_HMB_II_A;
//  uint16_t EF_HMB_II_B;
  /* HRB error flag. */
  uint16_t EF_HRB;
  /* HCB error flag. */
  uint16_t EF_HCB;
  /* HIB error flag. */
  uint16_t EF_HIB;
  /* DMI error flag. */
  uint16_t EF_DMI_I;
  uint16_t EF_DMI_II;
  /* CEU error flag. */
  uint16_t EF_CEU_I;
  uint16_t EF_CEU_II;
  /* EBV error flag. */
  uint16_t EF_EBV_I;
  uint16_t EF_EBV_II;
} ErrorCodeInitStruct, *pErrorCodeInitStruct;

/* The error code distribution structure. */
typedef struct tagErrorCodeDistribution
{
  /* CPU. */
  CPU_TYPE cpu;
  
  /* Specification. */
  char spec[DGM_SPEC_LEN];
  
  /* offset. */
  uint32_t offset;
  
  /* The old error code. */
  uint16_t ec_old;
  
  /* The new error code. */
  uint16_t ec_new;
  
  /* Side. */
  DEVICE_SIDE side;
} ErrorCodeDistribution, *pErrorCodeDistribution;


/* private variable declaration ---------------------------------------------------------------- */
/* The instance of error code. */
static ErrorCodeInitStruct ErrorCodeInitInst =\
{
  /* Department catagory. */
  {
    0x00U
  },
  
  /* The flag of new datagram. */
  UNLOCK,
  
  /* The buffer of datagram. */
  {
    0x00U
  },
  
  /* The buffer of sending. */
  {
    0x00U
  },
  
  /* The state machine of error code. */
  EC_SM_IDLE,
};


/* private function declaration ---------------------------------------------------------------- */
static uint32_t GetWriteAddress( uint32_t base, uint32_t end );
static uint32_t AssertPage( uint8_t page[] );
static uint32_t GetAddressFromBlock( uint32_t start );
static uint32_t PageTimeCompare( TIME_METHOD method, uint8_t page[], uint8_t cmp[] );

static void RecordErrorCode( void );
static uint32_t WriteErrorCodeToFlash( char error_stat, uint8_t error_code, const char *error_spec );
static uint32_t ErrorCode_HMB( void );
static void ErrorCode_HMB_Processing( ErrorCodeDistribution *ecd );
#if EC_REBOOT_FLAG
static uint32_t ErrorCode_HMB_Reboot( ErrorCodeDistribution *ecd );
#endif
static uint32_t ErrorCode_HMB_CAN_1( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_CAN_2( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_LKJ_CAN_1( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_LKJ_CAN_2( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_485_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_485_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_RAM( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_Flash( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_Network( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_BasedData( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HMB_DP( ErrorCodeDistribution *ecd );
  
static uint32_t ErrorCode_HIB( void );
static void ErrorCode_HIB_Processing( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HIB_ICAN_1( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HIB_ICAN_2( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HIB_ECAN_1( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HIB_ECAN_2( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HIB_485_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HIB_485_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HIB_RAM( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HIB_Flash( ErrorCodeDistribution *ecd );

static uint32_t ErrorCode_HRB( void );
static void ErrorCode_HRB_Processing( ErrorCodeDistribution *ecd );
#if EC_REBOOT_FLAG
static uint32_t ErrorCode_HRB_Reboot( ErrorCodeDistribution *ecd );
#endif
static uint32_t ErrorCode_HRB_CAN_1( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HRB_CAN_2( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HRB_485_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HRB_485_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HRB_RAM( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HRB_Flash( ErrorCodeDistribution *ecd );

static uint32_t ErrorCode_HCB( void );
static void ErrorCode_HCB_Processing( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HCB_ICAN_1( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HCB_ICAN_2( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HCB_ECAN_1( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HCB_ECAN_2( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HCB_485_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HCB_485_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HCB_RAM( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_HCB_Flash( ErrorCodeDistribution *ecd );

static uint32_t ErrorCode_DMI( void );
static void ErrorCode_DMI_Processing( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_DMI_CAN_1( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_DMI_CAN_2( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_DMI_485_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_DMI_485_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_DMI_RAM( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_DMI_Flash( ErrorCodeDistribution *ecd );

static uint32_t ErrorCode_CEU( void );
static void ErrorCode_CEU_Processing( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_CEU_CAN_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_CEU_CAN_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_CEU_485_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_CEU_485_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_CEU_Relay( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_CEU_FRAM( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_CEU_DAC( ErrorCodeDistribution *ecd );

static uint32_t ErrorCode_EBV( void );
static void ErrorCode_EBV_Processing( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_CAN_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_CAN_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_485_A( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_485_B( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_Relay( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_FRAM( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_DAC_Selfcheck( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_DAC_Output( ErrorCodeDistribution *ecd );
static uint32_t ErrorCode_EBV_5V_Power( ErrorCodeDistribution *ecd );

static uint32_t UploadErrorCode( void );
static uint32_t AssertDatagram( LOCKER lock, uint8_t dgm[] );
static uint32_t Processing_EC_SM_IDLE( uint32_t avai, ErrorCodeInitStruct *inst );
static uint32_t Processing_EC_SM_ASSERT( ErrorCodeInitStruct *inst );
static uint32_t Processing_EC_SM_SEND( ErrorCodeInitStruct *inst );
static uint32_t Processing_EC_SM_ACK( uint32_t avai, ErrorCodeInitStruct *inst );
static uint32_t ACK_DatagramProcessing( ErrorCodeInitStruct *inst );
static uint32_t Processing_EC_SM_EXCEPTION( ErrorCodeInitStruct *inst );
static uint32_t MoveToBackwardAddress( uint32_t tnum, uint32_t fnum, uint32_t *addr );
static uint32_t MoveToForwardAddress( uint32_t size, uint32_t *addr );
static uint32_t LoadDatagramData( ErrorCodeInitStruct *inst, uint32_t size );
static uint16_t GetDatagramLength( uint32_t num, uint32_t amnt, uint32_t size );
static uint32_t GetDatagramAmount( uint32_t size );
static uint32_t GetDatagramTotalSize( uint32_t base );
static uint8_t Get_XOR_Byte( uint8_t array[], uint32_t size );

//static uint32_t WriteErrorCodePkt( char stat, uint8_t code, const char *spec, uint8_t *err );
/**************************************************************************************************
(^_^) Function Name : ThreadErrorCode.
(^_^) Brief         : The thread of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 4-September-2018.
(^_^) Parameter     : none.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
void ThreadErrorCode( void )
{
  /* 1. Recording error code. */
  RecordErrorCode();
  
  /* 2. Uploading error code. */
  UploadErrorCode();
} /* end function ThreadErrorCode */


/**************************************************************************************************
(^_^) Function Name : ErrorCodeInit.
(^_^) Brief         : Initializing error code structure.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 5-September-2018.
(^_^) Parameter     : dprt --> department catagory.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
void ErrorCodeInit( const char *dprt )
{
  /* 1. Department catagory. */
  memset( ErrorCodeInitInst.DPRT, 0U, sizeof( ErrorCodeInitInst.DPRT ) );
  memcpy( ErrorCodeInitInst.DPRT, dprt, strlen( dprt ) );
  
  /* 2. The flag of new datagram. */
  ErrorCodeInitInst.LockFlag = UNLOCK;
  
  /* 3. The buffer of datagram. */
  memset( ErrorCodeInitInst.DGM_Buffer, 0U, DGM_BUFFER_SIZE );
  
  /* 4. The buffer of sending. */
  memset( ErrorCodeInitInst.SendBuffer, 0U, PRTC_MAX_LEN );
  
  /* 5. The state machine of error code. */
  ErrorCodeInitInst.EC_SM  = EC_SM_IDLE;
  
  /* 6. Year, month, day, hour, minute, second. */
  ErrorCodeInitInst.Year   = 0U;
  ErrorCodeInitInst.Month  = 0x00U;
  ErrorCodeInitInst.Day    = 0x00U;
  ErrorCodeInitInst.Hour   = 0x00U;
  ErrorCodeInitInst.Minute = 0x00U;
  ErrorCodeInitInst.Second = 0x00U;
  
  /* 7. The address of error code for writing flash. */
  #if 0
    S25FL256S_Erase64KBlock( EC_BASE_ADDR );
  #endif
  ErrorCodeInitInst.WriteAddr = GetWriteAddress( EC_BASE_ADDR, EC_END_ADDR );
  #if 0
    printf( "\r\nThe next writing address of error code is 0x%x\r\n",\
            ErrorCodeInitInst.WriteAddr );
  #endif
  
  /* 8. The address of error code for reading flash. */
  ErrorCodeInitInst.ReadAddr = 0U;
  
  /* 9. The total size of datagram. */
  ErrorCodeInitInst.DGM_TotalSize = 0U;
  /* 10. Datagram amount. */
  ErrorCodeInitInst.DGM_AMNT = 0U;
  /* 11. Datagram number. */
  ErrorCodeInitInst.DGM_NUM  = 0U;
  /* 12. The time of ACK. */
  ErrorCodeInitInst.ACK_Time = 0U;
  
  /* 13. HMB error flag. */
  ErrorCodeInitInst.EF_HMB_A = 0x0000U;
  ErrorCodeInitInst.EF_HMB_B = 0x0000U;
  /* 14. HRB error flag. */
  ErrorCodeInitInst.EF_HRB   = 0x0000U;
  /* 15. HCB error flag. */
  ErrorCodeInitInst.EF_HCB   = 0x0000U;
  /* 16. HIB error flag. */
  ErrorCodeInitInst.EF_HIB   = 0x0000U;
  /* 17. DMI error flag. */
  ErrorCodeInitInst.EF_DMI_I  = 0x0000U;
  ErrorCodeInitInst.EF_DMI_II = 0x0000U;
  /* 18. CEU error flag. */
  ErrorCodeInitInst.EF_CEU_I  = 0x0000U;
  ErrorCodeInitInst.EF_CEU_II = 0x0000U;
  /* 19. EBV error flag. */
  ErrorCodeInitInst.EF_EBV_I  = 0x0000U;
  ErrorCodeInitInst.EF_EBV_II = 0x0000U;
} /* end function ErrorCodeInit */


/**************************************************************************************************
(^_^) Function Name : GetWriteAddress.
(^_^) Brief         : Get the address of error code for writing to SPI flash.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 5-September-2018.
(^_^) Parameter     : base --> based address.
(^_^)                 end  --> end address.
(^_^) Return        : The address of error code for writing to SPI flash.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t GetWriteAddress( uint32_t base, uint32_t end )
{
  uint32_t addr = EC_BASE_ADDR;
  uint32_t b_addr = base;
  uint32_t e_addr = end;
  
  /* 1. Based address. */
  if ( EC_BASE_ADDR > base )
  {
    b_addr = EC_BASE_ADDR;
  } /* end if */
  
  /* 2. End address. */
  if ( end > EC_END_ADDR )
  {
    e_addr = EC_END_ADDR;
  } /* end if */
  
  /* 3. Searching block whick is not full. */
  uint8_t  tmp[FLASH_PAGE_SIZE];
  uint8_t  cmp[22] = { "9999-99-99 99:99:99  " };
  uint32_t all_full = 0U;
  
  for ( uint32_t i = b_addr; i <= e_addr; i += BLOCK_SIZE )
  {
    /* 3.1 Read last page of block. */
    S25FL256S_Read( ( uint32_t * )tmp, FLASH_PAGE_SIZE, i + BLOCK_SIZE - FLASH_PAGE_SIZE );
    
    /* 3.2 Assert page which is full or not. */
    if ( !AssertPage( tmp ) )
    {
      #if 0
        printf( "\r\nThe block address = 0x%x\r\n", i );
      #endif
      
      /* 3.3 Get the next address for writing error code from block. */
      addr = GetAddressFromBlock( i );
      /* Clear all full flag. */
      all_full = 0U;
      
      break;
    }
    else
    {
      /* 3.4 Compare time. */
      if ( PageTimeCompare( TIME_OLDEST, tmp, cmp ) )
      {
        all_full = i;
      } /* end if */
    } /* end if...else */
  } /* end for */
  
  /* 4. Assert all blocks whick are full or not. */
  if ( all_full )
  {
    addr = all_full;
    
    #if 0
      printf( "\r\nAll blocks are full, address = 0x%x\r\n", addr );
    #endif
  } /* end if */
  
  return addr;
} /* end function GetWriteAddress */


/**************************************************************************************************
(^_^) Function Name : AssertPage.
(^_^) Brief         : Assert page.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 5-September-2018.
(^_^) Parameter     : page --> page buffer.
(^_^) Return        :   0  --> the page is null.
(^_^)                 > 0  --> the page is not null.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t AssertPage( uint8_t page[] )
{
  uint32_t exit_code = 1U;
  
  /* 1. Assert argument. */
  if ( NULL == page )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t ec[EC_MAX_LEN];
  
    memset( ec, 0xFFU, EC_MAX_LEN );
    
    #if 0
      for ( uint32_t j = 0U; j < EC_MAX_LEN; j++ )
      {
        printf( "%x ", ec[j] );
      } /* end for */
      printf( "\r\n" );
    #endif
    
    for ( uint32_t j = 0u; j < FLASH_PAGE_SIZE; j += EC_MAX_LEN )
    {
      if ( 0U == memcmp( ec, &page[j], EC_MAX_LEN ) )
      {
        exit_code = 0U;
        
        break;
      } /* end if */
    } /* end for */
  } /* end if...else */
  
  return exit_code;
} /* end function AssertPage */


/**************************************************************************************************
(^_^) Function Name : GetAddressFromBlock.
(^_^) Brief         : Get next address from block.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 5-September-2018.
(^_^) Parameter     : start --> the start address of block.
(^_^) Return        : Next address of sector.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t GetAddressFromBlock( uint32_t start )
{
  uint32_t offt = 0U;
  /* cnt = log2(BLOCK_SIZE). */
  uint32_t cnt  = 16U;
  uint32_t s_a  = 0U;
  uint32_t e_a  = BLOCK_SIZE;
  uint8_t  ec[EC_MAX_LEN];
  uint8_t  null[EC_MAX_LEN];
  
  memset( ec, 0U, EC_MAX_LEN );
  memset( null, 0xFFU, EC_MAX_LEN );
  
  do
  {
    offt = ( s_a + e_a ) / 2U;
    
    if ( ( offt % EC_MAX_LEN ) || ( EC_MAX_LEN > offt ) )
    {
      offt = 0U;
      break;
    } /* end if */
    
    #if 0
      printf( "offt = 0x%x\r\n", offt );
      printf( "s_a = 0x%x\r\n", s_a );
      printf( "e_a = 0x%x\r\n", e_a );
      printf( "cnt = %d\r\n", cnt );
    #endif
    
    /* Read the recording of error code. */
    S25FL256S_Read( ( uint32_t * )ec, EC_MAX_LEN, offt + start );
    
    /* Assert the recording of error code. */
    if ( 0U == memcmp( null, ec, EC_MAX_LEN ) )
    {
      e_a = offt;
      
      /* Read the former recording of error code. */
      S25FL256S_Read( ( uint32_t * )ec, EC_MAX_LEN, offt + start - EC_MAX_LEN );
      
      /* Assert the former recording of error code. */
      if ( memcmp( null, ec, EC_MAX_LEN ) )
      {
        break;
      }
      else
      {
        if ( EC_MAX_LEN == offt )
        {
          offt = 0U;
          
          break;
        } /* end if */
      } /* end if...else */
    }
    else
    {
      s_a = offt;
    } /* end if...else */
    
    cnt--;
  } while ( cnt > 0U );
  
  return ( offt + start );
} /* end function GetAddressFromBlock */


/**************************************************************************************************
(^_^) Function Name : PageTimeCompare.
(^_^) Brief         : Compare the time with page.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : method --> refer to TIME_METHOD.
(^_^)                 page   --> page buffer.
(^_^)                 cmp    --> be compared time buffer.
(^_^) Return        :   0    --> origined time is not change.
(^_^)                 > 0    --> new time was put to buffer.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t PageTimeCompare( TIME_METHOD method, uint8_t page[], uint8_t cmp[] )
{
  uint32_t exit_code = 0U;
  
  /* 1. Assert arguments. */
  if ( NULL == page )
  {
    exit_code = 0U;
  }
  else if ( NULL == cmp )
  {
    exit_code = 0U;
  }
  else
  {
    uint32_t break_code = 0U;
    
    /* 2. Assert time method. */
    switch ( method )
    {
      case TIME_OLDEST : 
        for ( uint32_t i = 0U; i < FLASH_PAGE_SIZE; i += EC_MAX_LEN )
        {
          break_code = 0U;
          
          /* Compare time. */
          for ( uint32_t j = 0U; j < 21U; j++ )
          {
            if ( page[i + 2U + j] < cmp[j] )
            {
              break_code = 1U;
              
              memcpy( cmp, &page[i + 2U], 21U );
              
              break;
            }
            else if ( page[i + 2U + j] > cmp[j] )
            {
              break;
            }
            else
            {
            } /* end if...else if...else */
          } /* end for */
          
          /* Assert break code. */
          if ( break_code )
          {
            exit_code = break_code;
            
            break;
          } /* end if */
        } /* end for */
        break;
      
      case TIME_NEWEST : 
        for ( uint32_t i = 0U; i < FLASH_PAGE_SIZE; i += EC_MAX_LEN )
        {
          break_code = 0U;
          
          /* Compare time. */
          for ( uint32_t j = 0U; j < 21U; j++ )
          {
            if ( page[i + 2U + j] > cmp[j] )
            {
              break_code = 1U;
              
              memcpy( cmp, &page[i + 2U], 21U );
              
              break;
            }
            else if ( page[i + 2U + j] < cmp[j] )
            {
              break;
            }
            else
            {
            } /* end if...else if...else */
          } /* end for */
          
          /* Assert break code. */
          if ( break_code )
          {
            exit_code = break_code;
            
            break;
          } /* end if */
        } /* end for */
        break;
      
      default : 
        exit_code = 0U;
        break;
    } /* end switch */
  } /* end if...else */
  
  return exit_code;
} /* end function PageTimeCompare */


/**************************************************************************************************
(^_^) Function Name : RecordErrorCode.
(^_^) Brief         : Recording error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 5-September-2018.
(^_^) Parameter     : none.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static void RecordErrorCode( void )
{
  /* 1. HMB(host main board). */
  ErrorCode_HMB();
  
  /* 2. HIB(host interface board). */
  ErrorCode_HIB();
  
  /* 3. HRB(host recording board). */
  ErrorCode_HRB();
  
  /* 4. HCB(host communication board). */
  ErrorCode_HCB();
  
  /* 5. DMI(). */
  ErrorCode_DMI();
  
  /* 6. CEU(controlling execution unit). */
  ErrorCode_CEU();
  
  /* 7. EBV(electronic brake valve). */
  ErrorCode_EBV();
} /* end function RecordErrorCode */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorCodeDate.
(^_^) Brief         : Updating the date of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : date --> new date.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : 
(^_^) Software      : 
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t UpdateErrorCodeDate( uint8_t date[] )
{
  uint32_t exit_code = 0U;
  
  if ( NULL == date )
  {
    exit_code = 1U;
  }
  else
  {
    ErrorCodeInitInst.Year  = ( uint32_t )date[0] + 2000U;
    ErrorCodeInitInst.Month = date[1];
    ErrorCodeInitInst.Day   = date[2];
  } /* end if...else */
  
  return exit_code;
} /* end function UpdateErrorCodeDate */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorCodeTime.
(^_^) Brief         : Updating the time of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : time --> new date.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : 
(^_^) Software      : 
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t UpdateErrorCodeTime( uint8_t time[] )
{
  uint32_t exit_code = 0U;
  
  if ( NULL == time )
  {
    exit_code = 1U;
  }
  else
  {
    ErrorCodeInitInst.Hour   = ( uint8_t )( ( uint32_t )time[0] & 0x1FU );
    ErrorCodeInitInst.Minute = ( uint8_t )( ( uint32_t )time[1] & 0x3FU );
    ErrorCodeInitInst.Second = ( uint8_t )( ( uint32_t )time[2] & 0x3FU );
  } /* end if...else */
  
  return exit_code;
} /* end function UpdateErrorCodeTime */


/**********************************************
功能：组织故障履历记录事件包并写入flash
参数：error_stat --> 'F':false, 'T':true, others:invalid.
      error_code --> device code.
      error_spec --> specification of error code.
返回：0  --> operation successful.
    > 0  --> operation failed.
***********************************************/ 
static uint32_t WriteErrorCodeToFlash( char error_stat, uint8_t error_code, const char *error_spec )
{
  uint32_t exit_code = 0U;
//  uint8_t err_code_packet[EC_MAX_LEN] = {0U};
	uint8_t err_code_packet[72] = {0U};
  uint8_t err_code_Date[EC_DatPkt_MAXLEN] = {0U};
  uint8_t u8_Encode_Buff[EC_DatPkt_MAXLEN+1] = {0U};
  uint16_t u16_Encode_Length = 0U;
  uint32_t CRCR32_Dat = 0u;
  uint8_t index = 0;
  
  uint32_t len_spec  = strlen( error_spec );
  
  /* 1. 判断参数. */
  if ( ( ERROR_FALSE != error_stat ) && ( ERROR_TRUE != error_stat ) )
  {
    exit_code = 1U;
  }
  else if ( NULL == error_code ) 
  {
    exit_code = 2U;
  }
  else if ( ( NULL == error_spec ) || ( len_spec > 36U ) )
  {
    exit_code = 3U;
  }
  else
  {  
    memset( err_code_Date, 0U, EC_MAX_LEN );
    
    /* 2. 组织故障履历事件包. */
		err_code_Date[index++] = 0xE1;
    err_code_Date[index++] = 0x99;		
    err_code_Date[index++] = len_spec + 23U;    //长度
		err_code_Date[index++] = error_code;    //设备号
		err_code_Date[index++] = error_stat;    //故障状态
    
    /* Get data & time. */
    err_code_Date[index++] = ( ErrorCodeInitInst.Year % 10000U ) / 1000U + '0';
    err_code_Date[index++] = ( ErrorCodeInitInst.Year % 1000U ) / 100U + '0';
    err_code_Date[index++] = ( ErrorCodeInitInst.Year % 100U ) / 10U + '0';
    err_code_Date[index++] = ( ErrorCodeInitInst.Year % 10U ) + '0';
    err_code_Date[index++] = '-';
    err_code_Date[index++] = ( ErrorCodeInitInst.Month % 100U ) / 10U + '0';
    err_code_Date[index++] = ( ErrorCodeInitInst.Month % 10U ) + '0';
    err_code_Date[index++] = '-';
    err_code_Date[index++] = ( ErrorCodeInitInst.Day % 100U ) / 10U + '0';
    err_code_Date[index++] = ( ErrorCodeInitInst.Day % 10U ) + '0';
    err_code_Date[index++] = ' ';
    err_code_Date[index++] = ( ErrorCodeInitInst.Hour % 100U ) / 10U + '0';
    err_code_Date[index++] = ( ErrorCodeInitInst.Hour % 10U ) + '0';
    err_code_Date[index++] = ':';
    err_code_Date[index++] = ( ErrorCodeInitInst.Minute % 100U ) / 10U + '0';
    err_code_Date[index++] = ( ErrorCodeInitInst.Minute % 10U ) + '0';
    err_code_Date[index++] = ':';
    err_code_Date[index++] = ( ErrorCodeInitInst.Second % 100U ) / 10U + '0';
    err_code_Date[index++] = ( ErrorCodeInitInst.Second % 10U ) + '0';
    err_code_Date[index++] = ' ';
    err_code_Date[index++] = ' ';
    #if 0
      s_ErrorCodeInitInst.Second++;
      if ( s_ErrorCodeInitInst.Second >= 60U )
      {
        s_ErrorCodeInitInst.Second = 0U;
        s_ErrorCodeInitInst.Minute++;
        
        if ( s_ErrorCodeInitInst.Minute >= 60U )
        {
          s_ErrorCodeInitInst.Minute = 0U;
          s_ErrorCodeInitInst.Hour++;
          
          if ( s_ErrorCodeInitInst.Hour >= 24U )
          {
            s_ErrorCodeInitInst.Hour = 0U;
            s_ErrorCodeInitInst.Day++;
            
            if ( s_ErrorCodeInitInst.Day > 31U )
            {
              s_ErrorCodeInitInst.Day = 1U;
              s_ErrorCodeInitInst.Month++;
            } /* end if */
          } /* end if */
        } /* end if */
      } /* end if...else */
    #endif
    
    /* Load specification. */
    memcpy( &err_code_Date[index], error_spec, len_spec );
		index += 	len_spec;
    
    /* Load CRC32 code. */
    CRCR32_Dat = CRC32CCITT( err_code_Date, index, 0xFFFFFFFF );
    err_code_Date[index++] = (uint8_t)(CRCR32_Dat & 0xff);
    err_code_Date[index++] = (uint8_t)((CRCR32_Dat>>8) & 0xff);
    err_code_Date[index++] = (uint8_t)((CRCR32_Dat>>16) & 0xff);
    err_code_Date[index++] = (uint8_t)((CRCR32_Dat>>24) & 0xff);    
    
    /* 3. FF-FE协议编码. */
    u16_Encode_Length = FFFEEncode(err_code_Date, index, u8_Encode_Buff);
      
    /* 4. 组织故障履历记录事件. */
    err_code_packet[0] = 0xFF;
    err_code_packet[1] = 0xFE; 
    err_code_packet[2] = u16_Encode_Length + 5U;    //故障包实际长度
//		err_code_packet[2] = len_spec + 35U;    //故障包实际长度
    memcpy (err_code_packet + 3U,u8_Encode_Buff,u16_Encode_Length);
    err_code_packet[3U + u16_Encode_Length] = 0xFF;
    err_code_packet[4U + u16_Encode_Length] = 0xFD;        
    
    /* 5. 写入falsh. */
    if ( 0U == ( ErrorCodeInitInst.WriteAddr % BLOCK_SIZE ) )
    {
      S25FL256S_Erase64KBlock( ErrorCodeInitInst.WriteAddr );
      #if 1
        printf( "\r\nErasing block, address = 0x%x\r\n", ErrorCodeInitInst.WriteAddr );
      #endif
      /* Delay 500ms. */
      Wait( 500U );
    } /* end if */
    
		uint32_t i;
//    printf( "\r\n故障履历记录写入地址：0x%x\r\n", ErrorCodeInitInst.WriteAddr );
//		printf( "\r\n故障履历:%s\r\n", err_code_packet );
		
    S25FL256S_Write( ( uint32_t * )err_code_packet, (5U + u16_Encode_Length), ErrorCodeInitInst.WriteAddr, 0U );
    
    /* 6. Increasing next address. */
    ErrorCodeInitInst.WriteAddr += EC_MAX_LEN;
    
    if ( ErrorCodeInitInst.WriteAddr > EC_END_ADDR )
    {
      ErrorCodeInitInst.WriteAddr = EC_BASE_ADDR;
    } /* end if */
  } /* end if...else if...else */
  
  return exit_code;   
} /* end function WriteErrorCodeToFlash */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB.
(^_^) Brief         : Recording the error code of HMB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : none.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> exception occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB( void )
{
  uint32_t exit_code = 0U;
  static uint16_t err_a = 0x0000U;
  static uint16_t err_b = 0x0000U;
  static uint32_t timeout = 0U;
  static const char cpu_a[] = { "A模主控" };
  static const char cpu_b[] = { "B模主控" };
  
  /* 1. Assert timeout. */
  if ( Common_BeTimeOutMN( &timeout, 500U ) )
  {
    /* 2. HMB_I_A. */
    if ( ( uint32_t )err_a ^ ( uint32_t )ErrorCodeInitInst.EF_HMB_A )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ErrorCodeInitInst.DPRT );
      uint32_t len_cpu  = strlen( cpu_a );
      
      memcpy( ec_dist.spec, ErrorCodeInitInst.DPRT, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], cpu_a, len_cpu );
      
      ec_dist.cpu    = CPU_A;
      ec_dist.offset = len_dprt + len_cpu;
      ec_dist.ec_old = err_a;
      ec_dist.ec_new = ErrorCodeInitInst.EF_HMB_A;
      ErrorCode_HMB_Processing( &ec_dist );
      
      err_a = ErrorCodeInitInst.EF_HMB_A;
    } /* end if */
    
    /* 3. HMB_I_B. */
    if ( ( uint32_t )err_b ^ ( uint32_t )ErrorCodeInitInst.EF_HMB_B )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ErrorCodeInitInst.DPRT );
      uint32_t len_cpu  = strlen( cpu_b );
      
      memcpy( ec_dist.spec, ErrorCodeInitInst.DPRT, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], cpu_b, len_cpu );
      
      ec_dist.cpu    = CPU_B;
      ec_dist.offset = len_dprt + len_cpu;
      ec_dist.ec_old = err_b;
      ec_dist.ec_new = ErrorCodeInitInst.EF_HMB_B;
      ErrorCode_HMB_Processing( &ec_dist );
      
      err_b = ErrorCodeInitInst.EF_HMB_B;
    } /* end if */   
  }    
  else
  {
    exit_code = 1U;
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorFlag_HMB.
(^_^) Brief         : Updating the error flag of HMB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : cpu --> refer to CPU_TYPE.
(^_^)                 err --> error flag.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t UpdateErrorFlag_HMB( CPU_TYPE cpu, uint8_t err[] )
{
  uint32_t exit_code = 0U;
  
  if ( NULL == err )
  {
    exit_code = 1U;
  }
  else
  {
    switch ( cpu )
    {
      case CPU_A : 
        ErrorCodeInitInst.EF_HMB_A = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                                 + ( uint16_t )err[0];
        break;
      
      case CPU_B : 
        ErrorCodeInitInst.EF_HMB_B = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                                 + ( uint16_t )err[0];
        break;
      
      default : 
        exit_code = 2U;
        break;
    } /* end switch */
  } /* end if...else */
  
  return exit_code;
} /* end function UpdateErrorFlag_HMB */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_Processing.
(^_^) Brief         : Processing the error code of HMB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static void ErrorCode_HMB_Processing( ErrorCodeDistribution *ecd )
{
  /* 1. Assert argument. */
  if ( NULL == ecd )
  {
    return ;
  }
  else
  {
    /* 2. Reboot flag. */
    #if EC_REBOOT_FLAG
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_Reboot( ecd );
    #endif
    
    /* 3. CAN1. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_CAN_1( ecd );
    
    /* 4. CAN2. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_CAN_2( ecd );
    
    /* 5. LKJ CAN1. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_LKJ_CAN_1( ecd );
    
    /* 6. LKJ CAN2. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_LKJ_CAN_2( ecd );
    
    /* 7. RS485A. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_485_A( ecd );
    
    /* 8. RS485B. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_485_B( ecd );
    
    /* 9. RAM. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_RAM( ecd );
    
    /* 10. Flash. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_Flash( ecd );
    
    /* 11. Network. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_Network( ecd );
    
    /* 12. Based data. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_BasedData( ecd );
    
    /* 13. DianPai(Unknown english word). */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HMB_DP( ecd );
  } /* end if...else */
} /* end function ErrorCode_HMB_Processing */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_Reboot.
(^_^) Brief         : Recording the reboot event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
#if EC_REBOOT_FLAG
static uint32_t ErrorCode_HMB_Reboot( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hmb_reboot_T[] = { "启动中" };
  static const char hmb_reboot_F[] = { "启动完成" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0001U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0001U )
    {
      memcpy( &ecd->spec[ecd->offset], hmb_reboot_T, strlen( hmb_reboot_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hmb_reboot_F, strlen( hmb_reboot_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == Get_CPU_Type() )
    {
      /* CPU B. */
      if ( CPU_B == ecd->cpu )
      {
        WriteErrorCodeToFlash( state, "D000", ecd->spec );
      }
      /* CPU A. */
      else
      {
        WriteErrorCodeToFlash( state, "C000", ecd->spec );
      } /* end if...else */
    }
    /* Department I. */
    else
    {
      /* CPU B. */
      if ( CPU_B == ecd->cpu )
      {
        WriteErrorCodeToFlash( state, "B000", ecd->spec );
      }
      /* CPU A. */
      else
      {
        WriteErrorCodeToFlash( state, "A000", ecd->spec );
      } /* end if...else */
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_Reboot */
#endif


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_CAN_1.
(^_^) Brief         : Recording the CAN1 event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_CAN_1( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hmb_can_1_T[] = { "内CAN1故障" };
  static const char hmb_can_1_F[] = { "内CAN1故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0002U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0002U )
    {
      memcpy( &ecd->spec[ecd->offset], hmb_can_1_T, strlen( hmb_can_1_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hmb_can_1_F, strlen( hmb_can_1_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == Get_CPU_Type() )
    {
      /* CPU B. */
      if ( CPU_B == ecd->cpu )
      {
        WriteErrorCodeToFlash( state, 0x14, ecd->spec );
      }
      /* CPU A. */
      else
      {
        WriteErrorCodeToFlash( state, 0x13, ecd->spec );
      } /* end if...else */
    }
    /* Department I. */
    else
    {
      /* CPU B. */
      if ( CPU_B == ecd->cpu )
      {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
      } /* end if...else */
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_CAN_1 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_CAN_2.
(^_^) Brief         : Recording the CAN2 event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_CAN_2( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hmb_can_2_T[] = { "内CAN2故障" };
  static const char hmb_can_2_F[] = { "内CAN2故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0004U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0004U )
    {
      memcpy( &ecd->spec[ecd->offset], hmb_can_2_T, strlen( hmb_can_2_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hmb_can_2_F, strlen( hmb_can_2_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_CAN_2 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_LKJ_CAN_1.
(^_^) Brief         : Recording the LKJ CAN 1 event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_LKJ_CAN_1( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char lkj_can_1_T[] = { "LKJ CAN1故障" };
  static const char lkj_can_1_F[] = { "LKJ CAN1故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0008U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0008U )
    {
      memcpy( &ecd->spec[ecd->offset], lkj_can_1_T, strlen( lkj_can_1_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], lkj_can_1_F, strlen( lkj_can_1_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_LKJ_CAN_1 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_LKJ_CAN_2.
(^_^) Brief         : Recording the LKJ CAN 2 event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_LKJ_CAN_2( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char lkj_can_2_T[] = { "LKJ CAN2故障" };
  static const char lkj_can_2_F[] = { "LKJ CAN2故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0010U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0010U )
    {
      memcpy( &ecd->spec[ecd->offset], lkj_can_2_T, strlen( lkj_can_2_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], lkj_can_2_F, strlen( lkj_can_2_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_LKJ_CAN_2 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_485_A.
(^_^) Brief         : Recording the RS485A event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_485_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485A故障" };
  static const char rs485_F[] = { "RS485A故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0020U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0020U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_485_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_485_B.
(^_^) Brief         : Recording the RS485B event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_485_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485B故障" };
  static const char rs485_F[] = { "RS485B故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0040U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0040U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_485_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_RAM.
(^_^) Brief         : Recording the RAM event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_RAM( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ram_T[] = { "RAM故障" };
  static const char ram_F[] = { "RAM故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0080U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0080U )
    {
      memcpy( &ecd->spec[ecd->offset], ram_T, strlen( ram_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ram_F, strlen( ram_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_RAM */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_Flash.
(^_^) Brief         : Recording the flash event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_Flash( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char flash_T[] = { "FLASH故障" };
  static const char flash_F[] = { "FLASH故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0100U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0100U )
    {
      memcpy( &ecd->spec[ecd->offset], flash_T, strlen( flash_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], flash_F, strlen( flash_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_Flash */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_Network.
(^_^) Brief         : Recording the network event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_Network( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char net_T[] = { "网络故障" };
  static const char net_F[] = { "网络故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0200U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0200U )
    {
      memcpy( &ecd->spec[ecd->offset], net_T, strlen( net_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], net_F, strlen( net_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_Network */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_BasedData.
(^_^) Brief         : Recording the based data event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_BasedData( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char bd_T[] = { "基础数据故障" };
  static const char bd_F[] = { "基础数据故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0400U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0400U )
    {
      memcpy( &ecd->spec[ecd->offset], bd_T, strlen( bd_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], bd_F, strlen( bd_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_BasedData */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HMB_DP.
(^_^) Brief         : Recording the dianpai event of HMB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 22-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HMB_DP( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char bd_T[] = { "点排数据故障" };
  static const char bd_F[] = { "点排数据故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0800U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0800U )
    {
      memcpy( &ecd->spec[ecd->offset], bd_T, strlen( bd_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], bd_F, strlen( bd_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
      /* Department II. */
      if ( CPU_B == Get_CPU_Type() )
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x14, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x13, ecd->spec );
        } /* end if...else */
      }
      /* Department I. */
      else
      {
        /* CPU B. */
        if ( CPU_B == ecd->cpu )
        {
          WriteErrorCodeToFlash( state, 0x12, ecd->spec );
        }
        /* CPU A. */
        else
        {
          WriteErrorCodeToFlash( state, 0x11, ecd->spec );
        } /* end if...else */
      } /* end if...else */ 
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HMB_DP */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB.
(^_^) Brief         : Recording the error code of HIB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : none.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> exception occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB( void )
{
  uint32_t exit_code = 0U;
  static uint16_t err = 0x0000U;
  static uint32_t timeout = 0U;
  static const char hib[] = { "接口插件" };
  
  /* 1. Assert timeout. */
  if ( Common_BeTimeOutMN( &timeout, 500U ) )
  {
    if ( ( uint32_t )err ^ ( uint32_t )ErrorCodeInitInst.EF_HIB  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ErrorCodeInitInst.DPRT );
      uint32_t len_hib  = strlen( hib );
      
      memcpy( ec_dist.spec, ErrorCodeInitInst.DPRT, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], hib, len_hib );
      
      ec_dist.cpu    = Get_CPU_Type();
      ec_dist.offset = len_dprt + len_hib;
      ec_dist.ec_old = err;
      ec_dist.ec_new = ErrorCodeInitInst.EF_HIB;
      
      ErrorCode_HIB_Processing( &ec_dist );
      
      err = ErrorCodeInitInst.EF_HIB;
    } /* end if */
  }
  else
  {
    exit_code = 1U;
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorFlag_HIB.
(^_^) Brief         : Updating the error flag of HIB(host interface board).
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : err --> the new error flag of HIB.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t UpdateErrorFlag_HIB( uint8_t err[] )
{
  uint32_t exit_code = 0U;
  
  /* 1. Assert argument. */
  if ( NULL == err )
  {
    exit_code = 1U;
  }
  else
  {
    ErrorCodeInitInst.EF_HIB = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                           + ( uint16_t )err[0];
  } /* end if...else */
  
  return exit_code;
} /* end function UpdateErrorFlag_HIB */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_Processing.
(^_^) Brief         : Processing the error code of HIB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static void ErrorCode_HIB_Processing( ErrorCodeDistribution *ecd )
{
  /* 1. Assert argument. */
  if ( NULL == ecd )
  {
    return ;
  }
  else
  {
    /* 2. Reboot. */
    
    /* 3. Internal CAN 1. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HIB_ICAN_1( ecd );
    
    /* 4. Internal CAN 2. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HIB_ICAN_2( ecd );
    
    /* 5. External CAN 1. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HIB_ECAN_1( ecd );
    
    /* 6. External CAN 2. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HIB_ECAN_2( ecd );
    
    /* 7. RS485A. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HIB_485_A( ecd );
    
    /* 8. RS485B. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HIB_485_B( ecd );
    
    /* 9. RAM. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HIB_RAM( ecd );
    
    /* 10. Flash. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HIB_Flash( ecd );
  } /* end if...else */
} /* end function ErrorCode_HIB_Processing */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_ICAN_1.
(^_^) Brief         : Recording the internal CAN 1 event of HIB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB_ICAN_1( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hib_can_1_T[] = { "内CAN1故障" };
  static const char hib_can_1_F[] = { "内CAN1故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0002U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
      
    if ( ecd->ec_new & 0x0002U )
    {
      memcpy( &ecd->spec[ecd->offset], hib_can_1_T, strlen( hib_can_1_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hib_can_1_F, strlen( hib_can_1_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x42, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x41, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB_ICAN_1 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_ICAN_2.
(^_^) Brief         : Recording the internal CAN 2 event of HIB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB_ICAN_2( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hib_can_2_T[] = { "内CAN2故障" };
  static const char hib_can_2_F[] = { "内CAN2故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0004U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0004U )
    {
      memcpy( &ecd->spec[ecd->offset], hib_can_2_T, strlen( hib_can_2_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hib_can_2_F, strlen( hib_can_2_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x42, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x41, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB_ICAN_2 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_ECAN_1.
(^_^) Brief         : Recording the external CAN 1 event of HIB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB_ECAN_1( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hib_can_1_T[] = { "外CAN1故障" };
  static const char hib_can_1_F[] = { "外CAN1故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0008U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
      
    if ( ecd->ec_new & 0x0008U )
    {
      memcpy( &ecd->spec[ecd->offset], hib_can_1_T, strlen( hib_can_1_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hib_can_1_F, strlen( hib_can_1_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x42, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x41, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB_ECAN_1 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_ECAN_2.
(^_^) Brief         : Recording the external CAN 2 event of HIB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB_ECAN_2( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hib_can_2_T[] = { "外CAN2故障" };
  static const char hib_can_2_F[] = { "外CAN2故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0010U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
      
    if ( ecd->ec_new & 0x0010U )
    {
      memcpy( &ecd->spec[ecd->offset], hib_can_2_T, strlen( hib_can_2_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hib_can_2_F, strlen( hib_can_2_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x42, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x41, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB_ECAN_2 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_485_A.
(^_^) Brief         : Recording the RS485 A event of HIB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB_485_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485A故障" };
  static const char rs485_F[] = { "RS485A故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0020U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0020U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x42, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x41, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB_485_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_485_B.
(^_^) Brief         : Recording the RS485 B event of HIB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB_485_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485B故障" };
  static const char rs485_F[] = { "RS485B故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0040U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0040U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x42, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x41, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB_485_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_RAM.
(^_^) Brief         : Recording the RAM event of HIB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB_RAM( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ram_T[] = { "RAM故障" };
  static const char ram_F[] = { "RAM故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0080U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0080U )
    {
      memcpy( &ecd->spec[ecd->offset], ram_T, strlen( ram_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ram_F, strlen( ram_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x42, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x41, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB_RAM */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HIB_Flash.
(^_^) Brief         : Recording the flash event of HIB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HIB_Flash( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char flash_T[] = { "FLASH故障" };
  static const char flash_F[] = { "FLASH故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0100U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0100U )
    {
      memcpy( &ecd->spec[ecd->offset], flash_T, strlen( flash_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], flash_F, strlen( flash_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x42, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x41, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HIB_Flash */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB.
(^_^) Brief         : Recording the error code of HRB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : none.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> exception occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HRB( void )
{
  uint32_t exit_code = 0U;
  static uint16_t err = 0x0000U;
  static uint32_t timeout = 0U;
  static const char hrb[] = { "记录插件" };
  
  /* 1. Assert timeout. */
  if ( Common_BeTimeOutMN( &timeout, 500U ) )
  {
    if ( ( uint32_t )err ^ ( uint32_t )ErrorCodeInitInst.EF_HRB  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ErrorCodeInitInst.DPRT );
      uint32_t len_hrb  = strlen( hrb );
      
      memcpy( ec_dist.spec, ErrorCodeInitInst.DPRT, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], hrb, len_hrb );
      
      ec_dist.cpu    = Get_CPU_Type();
      ec_dist.offset = len_dprt + len_hrb;
      ec_dist.ec_old = err;
      ec_dist.ec_new = ErrorCodeInitInst.EF_HRB;
      
      ErrorCode_HRB_Processing( &ec_dist );
      
      err = ErrorCodeInitInst.EF_HRB;
    } /* end if */
  }
  else
  {
    exit_code = 1U;
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HRB */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorFlag_HRB.
(^_^) Brief         : Updating the error flag of HRB(host recording board).
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : err --> the new error flag of HRB.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
void UpdateErrorFlag_HRB( uint16_t err )
{
  ErrorCodeInitInst.EF_HRB = err;
  #if 0
    printf( "\r\nEF_HRB=0x%04x\r\n", ErrorCodeInitInst.EF_HRB );
  #endif
} /* end function UpdateErrorFlag_HRB */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB_Processing.
(^_^) Brief         : Processing the error code of HRB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static void ErrorCode_HRB_Processing( ErrorCodeDistribution *ecd )
{
  /* 1. Assert argument. */
  if ( NULL == ecd )
  {
    return ;
  }
  else
  {
    /* 2. Reboot. */
    #if EC_REBOOT_FLAG
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HRB_Reboot( ecd );
    #endif
    
    /* 3. CAN1. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HRB_CAN_1( ecd );
    
    /* 4. CAN2. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HRB_CAN_2( ecd );
    
    /* 5. CEU CAN1. */
    
    /* 6. CEU CAN2. */
    
    /* 7. RS485A. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HRB_485_A( ecd );
    
    /* 8. RS485B. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HRB_485_B( ecd );
    
    /* 9. RAM. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HRB_RAM( ecd );
    
    /* 10. Flash. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HRB_Flash( ecd );
  } /* end if...else */
} /* end function ErrorCode_HRB_Processing */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB_Reboot.
(^_^) Brief         : Recording the reboot event of HRB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
#if 0
static uint32_t ErrorCode_HRB_Reboot( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hrb_reboot_T[] = { "启动中" };
  static const char hrb_reboot_F[] = { "启动完成" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0001U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0001U )
    {
      memcpy( &ecd->spec[ecd->offset], hrb_reboot_T, strlen( hrb_reboot_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hrb_reboot_F, strlen( hrb_reboot_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x62, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x61, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HRB_Reboot */
#endif


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB_CAN_1.
(^_^) Brief         : Recording the CAN 1 event of HRB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HRB_CAN_1( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hrb_can_1_T[] = { "内CAN1故障" };
  static const char hrb_can_1_F[] = { "内CAN1故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0002U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0002U )
    {
      memcpy( &ecd->spec[ecd->offset], hrb_can_1_T, strlen( hrb_can_1_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hrb_can_1_F, strlen( hrb_can_1_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x62, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x61, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HRB_CAN_1 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB_CAN_2.
(^_^) Brief         : Recording the CAN 2 event of HRB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HRB_CAN_2( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hrb_can_2_T[] = { "内CAN2故障" };
  static const char hrb_can_2_F[] = { "内CAN2故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0004U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0004U )
    {
      memcpy( &ecd->spec[ecd->offset], hrb_can_2_T, strlen( hrb_can_2_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hrb_can_2_F, strlen( hrb_can_2_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x62, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x61, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HRB_CAN_2 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB_485_A.
(^_^) Brief         : Recording the RS485 A event of HRB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HRB_485_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485A故障" };
  static const char rs485_F[] = { "RS485A故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0020U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0020U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x62, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x61, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HRB_485_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB_485_B.
(^_^) Brief         : Recording the RS485 B event of HRB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HRB_485_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485B故障" };
  static const char rs485_F[] = { "RS485B故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0040U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0040U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x62, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x61, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HRB_485_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB_RAM.
(^_^) Brief         : Recording the RAM event of HRB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HRB_RAM( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ram_T[] = { "RAM故障" };
  static const char ram_F[] = { "RAM故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0080U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0080U )
    {
      memcpy( &ecd->spec[ecd->offset], ram_T, strlen( ram_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ram_F, strlen( ram_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x62, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x61, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HRB_RAM */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HRB_Flash.
(^_^) Brief         : Recording the flash event of HRB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HRB_Flash( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char flash_T[] = { "FLASH故障" };
  static const char flash_F[] = { "FLASH故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0100U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0100U )
    {
      memcpy( &ecd->spec[ecd->offset], flash_T, strlen( flash_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], flash_F, strlen( flash_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x62, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x61, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HRB_Flash */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB.
(^_^) Brief         : Recording the error code of HCB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : none.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> exception occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB( void )
{
  uint32_t exit_code = 0U;
  static uint16_t err = 0x0000U;
  static uint32_t timeout = 0U;
  static const char hcb[] = { "通信插件" };
  
  /* 1. Assert timeout. */
  if ( Common_BeTimeOutMN( &timeout, 500U ) )
  {
    if ( ( uint32_t )err ^ ( uint32_t )ErrorCodeInitInst.EF_HCB  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ErrorCodeInitInst.DPRT );
      uint32_t len_hcb  = strlen( hcb );
      
      memcpy( ec_dist.spec, ErrorCodeInitInst.DPRT, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], hcb, len_hcb );
      
      ec_dist.cpu    = Get_CPU_Type();
      ec_dist.offset = len_dprt + len_hcb;
      ec_dist.ec_old = err;
      ec_dist.ec_new = ErrorCodeInitInst.EF_HCB;
      
      ErrorCode_HCB_Processing( &ec_dist );
      
      err = ErrorCodeInitInst.EF_HCB;
    } /* end if */
  }
  else
  {
    exit_code = 1U;
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorFlag_HCB.
(^_^) Brief         : Updating the error flag of HCB(host communication board).
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : err --> the new error flag of HCB.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t UpdateErrorFlag_HCB( uint8_t err[] )
{
  uint32_t exit_code = 0U;
  
  /* 1. Assert argument. */
  if ( NULL == err )
  {
    exit_code = 1U;
  }
  else
  {
    ErrorCodeInitInst.EF_HCB = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                           + ( uint16_t )err[0];
  } /* end if...else */
  
  return exit_code;
} /* end function UpdateErrorFlag_HCB */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_Processing.
(^_^) Brief         : Processing the error code of HCB.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static void ErrorCode_HCB_Processing( ErrorCodeDistribution *ecd )
{
  /* 1. Assert argument. */
  if ( NULL == ecd )
  {
    return ;
  }
  else
  {
    /* 2. Reboot. */
    
    /* 3. Internal CAN 1. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HCB_ICAN_1( ecd );
    
    /* 4. Internal CAN 2. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HCB_ICAN_2( ecd );
    
    /* 5. External CAN 1. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HCB_ECAN_1( ecd );
    
    /* 6. External CAN 2. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HCB_ECAN_2( ecd );
    
    /* 7. RS485A. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HCB_485_A( ecd );
    
    /* 8. RS485B. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HCB_485_B( ecd );
    
    /* 9. RAM. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HCB_RAM( ecd );
    
    /* 10. Flash. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_HCB_Flash( ecd );
  } /* end if...else */
} /* end function ErrorCode_HCB_Processing */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_ICAN_1.
(^_^) Brief         : Recording the internal CAN 1 event of HCB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB_ICAN_1( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hcb_can_1_T[] = { "内CAN1故障" };
  static const char hcb_can_1_F[] = { "内CAN1故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0002U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
      
    if ( ecd->ec_new & 0x0002U )
    {
      memcpy( &ecd->spec[ecd->offset], hcb_can_1_T, strlen( hcb_can_1_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hcb_can_1_F, strlen( hcb_can_1_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x32, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x31, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB_ICAN_1 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_ICAN_2.
(^_^) Brief         : Recording the internal CAN 2 event of HCB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB_ICAN_2( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hcb_can_2_T[] = { "内CAN2故障" };
  static const char hcb_can_2_F[] = { "内CAN2故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0004U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0004U )
    {
      memcpy( &ecd->spec[ecd->offset], hcb_can_2_T, strlen( hcb_can_2_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hcb_can_2_F, strlen( hcb_can_2_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x32, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x31, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB_ICAN_2 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_ECAN_1.
(^_^) Brief         : Recording the external CAN 1 event of HCB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB_ECAN_1( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hcb_can_1_T[] = { "外CAN1故障" };
  static const char hcb_can_1_F[] = { "外CAN1故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0008U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
      
    if ( ecd->ec_new & 0x0008U )
    {
      memcpy( &ecd->spec[ecd->offset], hcb_can_1_T, strlen( hcb_can_1_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hcb_can_1_F, strlen( hcb_can_1_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x32, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x31, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB_ECAN_1 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_ECAN_2.
(^_^) Brief         : Recording the external CAN 2 event of HCB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB_ECAN_2( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char hcb_can_2_T[] = { "外CAN2故障" };
  static const char hcb_can_2_F[] = { "外CAN2故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0010U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
      
    if ( ecd->ec_new & 0x0010U )
    {
      memcpy( &ecd->spec[ecd->offset], hcb_can_2_T, strlen( hcb_can_2_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], hcb_can_2_F, strlen( hcb_can_2_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x32, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x31, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB_ECAN_2 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_485_A.
(^_^) Brief         : Recording the RS485 A event of HCB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB_485_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485A故障" };
  static const char rs485_F[] = { "RS485A故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0020U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0020U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x32, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x31, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB_485_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_485_B.
(^_^) Brief         : Recording the RS485 B event of HCB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB_485_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485B故障" };
  static const char rs485_F[] = { "RS485B故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0040U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0040U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x32, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x31, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB_485_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_RAM.
(^_^) Brief         : Recording the RAM event of HCB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB_RAM( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ram_T[] = { "RAM故障" };
  static const char ram_F[] = { "RAM故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0080U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0080U )
    {
      memcpy( &ecd->spec[ecd->offset], ram_T, strlen( ram_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ram_F, strlen( ram_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x32, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x31, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB_RAM */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_HCB_Flash.
(^_^) Brief         : Recording the flash event of HCB of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_HCB_Flash( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char flash_T[] = { "FLASH故障" };
  static const char flash_F[] = { "FLASH故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0100U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0100U )
    {
      memcpy( &ecd->spec[ecd->offset], flash_T, strlen( flash_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], flash_F, strlen( flash_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Department II. */
    if ( CPU_B == ecd->cpu )
    {
      WriteErrorCodeToFlash( state, 0x32, ecd->spec );
    }
    /* Department I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x31, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_HCB_Flash */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_DMI.
(^_^) Brief         : Recording the error code of DMI.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : none.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> exception occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_DMI( void )
{
  uint32_t exit_code = 0U;
  static uint16_t err_i   = 0x0000U;
  static uint16_t err_ii  = 0x0000U;
  static uint32_t timeout = 0U;
  static const char dmi_i[]  = { "I端" };
  static const char dmi_ii[] = { "II端" };
  static const char dmi[]    = { "显示器" };
  
  /* 1. Assert timeout. */
  if ( Common_BeTimeOutMN( &timeout, 500U ) )
  {
    if ( ( uint32_t )err_i ^ ( uint32_t )ErrorCodeInitInst.EF_DMI_I  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( dmi_i );
      uint32_t len_dmi  = strlen( dmi );
      
      memcpy( ec_dist.spec, dmi_i, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], dmi, len_dmi );
      
      ec_dist.side   = DEV_SIDE_I;
      ec_dist.offset = len_dprt + len_dmi;
      ec_dist.ec_old = err_i;
      ec_dist.ec_new = ErrorCodeInitInst.EF_DMI_I;
      
      ErrorCode_DMI_Processing( &ec_dist );
      
      err_i = ErrorCodeInitInst.EF_DMI_I;
    } /* end if */
    
    if ( ( uint32_t )err_ii ^ ( uint32_t )ErrorCodeInitInst.EF_DMI_II  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( dmi_ii );
      uint32_t len_dmi  = strlen( dmi );
      
      memcpy( ec_dist.spec, dmi_ii, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], dmi, len_dmi );
      
      ec_dist.side   = DEV_SIDE_II;
      ec_dist.offset = len_dprt + len_dmi;
      ec_dist.ec_old = err_ii;
      ec_dist.ec_new = ErrorCodeInitInst.EF_DMI_II;
      
      ErrorCode_DMI_Processing( &ec_dist );
      
      err_ii = ErrorCodeInitInst.EF_DMI_II;
    } /* end if */
  }
  else
  {
    exit_code = 1U;
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_DMI */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorFlag_DMI.
(^_^) Brief         : Updating the error flag of DMI.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : side --> refer to DEVICE_SIDE.
(^_^)                 err  --> error flag.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t UpdateErrorFlag_DMI( DEVICE_SIDE side, uint8_t err[] )
{
  uint32_t exit_code = 0U;
  
  if ( NULL == err )
  {
    exit_code = 1U;
  }
  else
  {
    switch ( side )
    {
      case DEV_SIDE_I : 
        ErrorCodeInitInst.EF_DMI_I = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                                 + ( uint16_t )err[0];
        break;
      
      case DEV_SIDE_II : 
        ErrorCodeInitInst.EF_DMI_II = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                                  + ( uint16_t )err[0];
        break;
      
      default : 
        exit_code = 2U;
        break;
    } /* end switch */
  } /* end if...else */
  
  return exit_code;
} /* end function UpdateErrorFlag_DMI */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_DMI_Processing.
(^_^) Brief         : Processing the error code of DMI.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static void ErrorCode_DMI_Processing( ErrorCodeDistribution *ecd )
{
  /* 1. Assert argument. */
  if ( NULL == ecd )
  {
    return ;
  }
  else
  {
    /* 2. Reboot. */
    
    /* 3. CAN 1. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_DMI_CAN_1( ecd );
    
    /* 4. CAN 2. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_DMI_CAN_2( ecd );
    
    /* 5. Reserved. */
    
    /* 6. Reserved. */
    
    /* 7. RS485A. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_DMI_485_A( ecd );
    
    /* 8. RS485B. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_DMI_485_B( ecd );
    
    /* 9. RAM. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_DMI_RAM( ecd );
    
    /* 10. Flash. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_DMI_Flash( ecd );
  } /* end if...else */
} /* end function ErrorCode_DMI_Processing */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_DMI_CAN_1.
(^_^) Brief         : Recording the CAN 1 event of DMI of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_DMI_CAN_1( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char dmi_can_1_T[] = { "CAN1故障" };
  static const char dmi_can_1_F[] = { "CAN1故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0002U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0002U )
    {
      memcpy( &ecd->spec[ecd->offset], dmi_can_1_T, strlen( dmi_can_1_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], dmi_can_1_F, strlen( dmi_can_1_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x22, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x21, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_DMI_CAN_1 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_DMI_CAN_2.
(^_^) Brief         : Recording the CAN 2 event of DMI of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_DMI_CAN_2( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char dmi_can_2_T[] = { "CAN2故障" };
  static const char dmi_can_2_F[] = { "CAN2故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0004U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0004U )
    {
      memcpy( &ecd->spec[ecd->offset], dmi_can_2_T, strlen( dmi_can_2_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], dmi_can_2_F, strlen( dmi_can_2_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x22, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x21, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_DMI_CAN_2 */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_DMI_485_A.
(^_^) Brief         : Recording the RS485 A event of DMI of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_DMI_485_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485A故障" };
  static const char rs485_F[] = { "RS485A故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0020U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0020U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x22, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x21, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_DMI_485_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_DMI_485_B.
(^_^) Brief         : Recording the RS485 B event of DMI of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_DMI_485_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485B故障" };
  static const char rs485_F[] = { "RS485B故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0040U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0040U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x22, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x21, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_DMI_485_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_DMI_RAM.
(^_^) Brief         : Recording the RAM event of DMI of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_DMI_RAM( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ram_T[] = { "RAM故障" };
  static const char ram_F[] = { "RAM故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0080U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0080U )
    {
      memcpy( &ecd->spec[ecd->offset], ram_T, strlen( ram_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ram_F, strlen( ram_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x22, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x21, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_DMI_RAM */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_DMI_Flash.
(^_^) Brief         : Recording the flash event of DMI of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 25-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_DMI_Flash( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char flash_T[] = { "FLASH故障" };
  static const char flash_F[] = { "FLASH故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0100U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0100U )
    {
      memcpy( &ecd->spec[ecd->offset], flash_T, strlen( flash_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], flash_F, strlen( flash_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x22, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x21, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_DMI_Flash */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU.
(^_^) Brief         : Recording the error code of CEU.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : none.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> exception occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_CEU( void )
{
  uint32_t exit_code = 0U;
  static uint16_t err_i   = 0x0000U;
  static uint16_t err_ii  = 0x0000U;
  static uint32_t timeout = 0U;
  static const char ceu_i[]  = { "I端" };
  static const char ceu_ii[] = { "II端" };
  static const char ceu[]    = { " CEU " };
  
  /* 1. Assert timeout. */
  if ( Common_BeTimeOutMN( &timeout, 500U ) )
  {
    if ( ( uint32_t )err_i ^ ( uint32_t )ErrorCodeInitInst.EF_CEU_I  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ceu_i );
      uint32_t len_ceu  = strlen( ceu );
      
      memcpy( ec_dist.spec, ceu_i, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], ceu, len_ceu );
      
      ec_dist.side   = DEV_SIDE_I;
      ec_dist.offset = len_dprt + len_ceu;
      ec_dist.ec_old = err_i;
      ec_dist.ec_new = ErrorCodeInitInst.EF_CEU_I;
      
      ErrorCode_CEU_Processing( &ec_dist );
      
      err_i = ErrorCodeInitInst.EF_CEU_I;
    } /* end if */
    
    if ( ( uint32_t )err_ii ^ ( uint32_t )ErrorCodeInitInst.EF_CEU_II  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ceu_ii );
      uint32_t len_ceu  = strlen( ceu );
      
      memcpy( ec_dist.spec, ceu_ii, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], ceu, len_ceu );
      
      ec_dist.side   = DEV_SIDE_II;
      ec_dist.offset = len_dprt + len_ceu;
      ec_dist.ec_old = err_ii;
      ec_dist.ec_new = ErrorCodeInitInst.EF_CEU_II;
      
      ErrorCode_CEU_Processing( &ec_dist );
      
      err_ii = ErrorCodeInitInst.EF_CEU_II;
    } /* end if */
  }
  else
  {
    exit_code = 1U;
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_CEU */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorFlag_CEU.
(^_^) Brief         : Updating the error flag of CEU.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : side --> refer to DEVICE_SIDE.
(^_^)                 err  --> error flag.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t UpdateErrorFlag_CEU( DEVICE_SIDE side, uint8_t err[] )
{
  uint32_t exit_code = 0U;
  
  if ( NULL == err )
  {
    exit_code = 1U;
  }
  else
  {
    switch ( side )
    {
      case DEV_SIDE_I : 
        ErrorCodeInitInst.EF_CEU_I = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                                 + ( uint16_t )err[0];
        break;
      
      case DEV_SIDE_II : 
        ErrorCodeInitInst.EF_CEU_II = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                                  + ( uint16_t )err[0];
        break;
      
      default : 
        exit_code = 2U;
        break;
    } /* end switch */
  } /* end if...else */
  
  return exit_code;
} /* end function UpdateErrorFlag_CEU */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU_Processing.
(^_^) Brief         : Processing the error code of CEU.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static void ErrorCode_CEU_Processing( ErrorCodeDistribution *ecd )
{
  /* 1. Assert argument. */
  if ( NULL == ecd )
  {
    return ;
  }
  else
  {
    /* 2. Reserved. */
    
    /* 3. CAN B. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_CEU_CAN_B( ecd );
    
    /* 4. CAN A. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_CEU_CAN_A( ecd );
    
    /* 5. RS485A.*/
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_CEU_485_A( ecd );
    
    /* 6. RS485B.*/
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_CEU_485_B( ecd );
    
    /* 7. Relay. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_CEU_Relay( ecd );
    
    /* 8. Flash. */
    
    /* 9. SRAM. */
    
    /* 10. FRAM. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_CEU_FRAM( ecd );
    
    /* 11. DAC. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_CEU_DAC( ecd );
  } /* end if...else */
} /* end function ErrorCode_CEU_Processing */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU_CAN_A.
(^_^) Brief         : Recording the CAN A event of CEU of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_CEU_CAN_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ceu_can_a_T[] = { "CANA故障" };
  static const char ceu_can_a_F[] = { "CANA故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0004U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0004U )
    {
      memcpy( &ecd->spec[ecd->offset], ceu_can_a_T, strlen( ceu_can_a_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ceu_can_a_F, strlen( ceu_can_a_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x72, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x71, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_CEU_CAN_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU_CAN_B.
(^_^) Brief         : Recording the CAN B event of CEU of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_CEU_CAN_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ceu_can_b_T[] = { "CANB故障" };
  static const char ceu_can_b_F[] = { "CANB故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0002U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0002U )
    {
      memcpy( &ecd->spec[ecd->offset], ceu_can_b_T, strlen( ceu_can_b_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ceu_can_b_F, strlen( ceu_can_b_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x72, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x71, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_CEU_CAN_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU_485_A.
(^_^) Brief         : Recording the RS485 A event of CEU of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_CEU_485_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485A故障" };
  static const char rs485_F[] = { "RS485A故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0008U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0008U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x72, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x71, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_CEU_485_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU_485_B.
(^_^) Brief         : Recording the RS485 B event of CEU of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_CEU_485_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485B故障" };
  static const char rs485_F[] = { "RS485B故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0010U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0010U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x72, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x71, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_CEU_485_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU_Relay.
(^_^) Brief         : Recording the relay event of CEU of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_CEU_Relay( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char relay_T[] = { "继电器故障" };
  static const char relay_F[] = { "继电器故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0020U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0020U )
    {
      memcpy( &ecd->spec[ecd->offset], relay_T, strlen( relay_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], relay_F, strlen( relay_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x72, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x71, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_CEU_Relay */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU_FRAM.
(^_^) Brief         : Recording the FRAM event of CEU of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_CEU_FRAM( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char fram_T[] = { "FRAM故障" };
  static const char fram_F[] = { "FRAM故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0100U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0100U )
    {
      memcpy( &ecd->spec[ecd->offset], fram_T, strlen( fram_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], fram_F, strlen( fram_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x72, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x71, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_CEU_FRAM */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_CEU_DAC.
(^_^) Brief         : Recording the DAC event of CEU of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_CEU_DAC( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char dac_T[] = { "DAC自检故障" };
  static const char dac_F[] = { "DAC自检故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0200U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0200U )
    {
      memcpy( &ecd->spec[ecd->offset], dac_T, strlen( dac_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], dac_F, strlen( dac_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x72, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x71, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_CEU_DAC */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV.
(^_^) Brief         : Recording the error code of EBV.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : none.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> exception occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV( void )
{
  uint32_t exit_code = 0U;
  static uint16_t err_i   = 0x0000U;
  static uint16_t err_ii  = 0x0000U;
  static uint32_t timeout = 0U;
  static const char ebv_i[]  = { "I端" };
  static const char ebv_ii[] = { "II端" };
  static const char ebv[]    = { " ECU " };
  
  /* 1. Assert timeout. */
  if ( Common_BeTimeOutMN( &timeout, 500U ) )
  {
    if ( ( uint32_t )err_i ^ ( uint32_t )ErrorCodeInitInst.EF_EBV_I  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ebv_i );
      uint32_t len_ebv  = strlen( ebv );
      
      memcpy( ec_dist.spec, ebv_i, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], ebv, len_ebv );
      
      ec_dist.side   = DEV_SIDE_I;
      ec_dist.offset = len_dprt + len_ebv;
      ec_dist.ec_old = err_i;
      ec_dist.ec_new = ErrorCodeInitInst.EF_EBV_I;
      
      ErrorCode_EBV_Processing( &ec_dist );
      
      err_i = ErrorCodeInitInst.EF_EBV_I;
    } /* end if */
    
    if ( ( uint32_t )err_ii ^ ( uint32_t )ErrorCodeInitInst.EF_EBV_II  )
    {
      ErrorCodeDistribution ec_dist;
      
      memset( ec_dist.spec, 0U, DGM_SPEC_LEN );
      
      uint32_t len_dprt = strlen( ebv_ii );
      uint32_t len_ebv  = strlen( ebv );
      
      memcpy( ec_dist.spec, ebv_ii, len_dprt );
      memcpy( &ec_dist.spec[len_dprt], ebv, len_ebv );
      
      ec_dist.side   = DEV_SIDE_II;
      ec_dist.offset = len_dprt + len_ebv;
      ec_dist.ec_old = err_ii;
      ec_dist.ec_new = ErrorCodeInitInst.EF_EBV_II;
      
      ErrorCode_EBV_Processing( &ec_dist );
      
      err_ii = ErrorCodeInitInst.EF_EBV_II;
    } /* end if */
  }
  else
  {
    exit_code = 1U;
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV */


/**************************************************************************************************
(^_^) Function Name : UpdateErrorFlag_EBV.
(^_^) Brief         : Updating the error flag of EBV.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : side --> refer to DEVICE_SIDE.
(^_^)                 err  --> error flag.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t UpdateErrorFlag_EBV( DEVICE_SIDE side, uint8_t err[] )
{
  uint32_t exit_code = 0U;
  
  if ( NULL == err )
  {
    exit_code = 1U;
  }
  else
  {
    switch ( side )
    {
      case DEV_SIDE_I : 
        ErrorCodeInitInst.EF_EBV_I = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                                 + ( uint16_t )err[0];
        break;
      
      case DEV_SIDE_II : 
        ErrorCodeInitInst.EF_EBV_II = ( uint16_t )( ( uint32_t )err[1] << 8U )\
                                                  + ( uint16_t )err[0];
        break;
      
      default : 
        exit_code = 2U;
        break;
    } /* end switch */
  } /* end if...else */
  
  return exit_code;
} /* end function UpdateErrorFlag_EBV */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_Processing.
(^_^) Brief         : Processing the error code of EBV.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static void ErrorCode_EBV_Processing( ErrorCodeDistribution *ecd )
{
  /* 1. Assert argument. */
  if ( NULL == ecd )
  {
    return ;
  }
  else
  {
    /* 2. Reserved. */
    
    /* 3. CAN B. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_CAN_B( ecd );
    
    /* 4. CAN A. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_CAN_A( ecd );
    
    /* 5. RS485A.*/
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_485_A( ecd );
    
    /* 6. RS485B.*/
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_485_B( ecd );
    
    /* 7. Relay. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_Relay( ecd );
    
    /* 8. Flash. */
    
    /* 9. SRAM. */
    
    /* 10. FRAM. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_FRAM( ecd );
    
    /* 11. DAC self-checking. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_DAC_Selfcheck( ecd );
    
    /* 12. DAC output. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_DAC_Output( ecd );
    
    /* 13. 5V power. */
    memset( &ecd->spec[ecd->offset], 0U, DGM_SPEC_LEN - ecd->offset );
    ErrorCode_EBV_5V_Power( ecd );
  } /* end if...else */
} /* end function ErrorCode_EBV_Processing */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_CAN_A.
(^_^) Brief         : Recording the CAN A event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_CAN_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ebv_can_a_T[] = { "CANA故障" };
  static const char ebv_can_a_F[] = { "CANA故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0004U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0004U )
    {
      memcpy( &ecd->spec[ecd->offset], ebv_can_a_T, strlen( ebv_can_a_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ebv_can_a_F, strlen( ebv_can_a_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_CAN_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_CAN_B.
(^_^) Brief         : Recording the CAN B event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_CAN_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char ebv_can_b_T[] = { "CANB故障" };
  static const char ebv_can_b_F[] = { "CANB故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0002U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0002U )
    {
      memcpy( &ecd->spec[ecd->offset], ebv_can_b_T, strlen( ebv_can_b_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], ebv_can_b_F, strlen( ebv_can_b_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_CAN_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_485_A.
(^_^) Brief         : Recording the RS485 A event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_485_A( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485A故障" };
  static const char rs485_F[] = { "RS485A故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0008U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0008U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_485_A */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_485_B.
(^_^) Brief         : Recording the RS485 B event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_485_B( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char rs485_T[] = { "RS485B故障" };
  static const char rs485_F[] = { "RS485B故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0010U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0010U )
    {
      memcpy( &ecd->spec[ecd->offset], rs485_T, strlen( rs485_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], rs485_F, strlen( rs485_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_485_B */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_Relay.
(^_^) Brief         : Recording the relay event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_Relay( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char relay_T[] = { "继电器故障" };
  static const char relay_F[] = { "继电器故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0020U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0020U )
    {
      memcpy( &ecd->spec[ecd->offset], relay_T, strlen( relay_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], relay_F, strlen( relay_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_Relay */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_FRAM.
(^_^) Brief         : Recording the FRAM event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_FRAM( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char fram_T[] = { "FRAM故障" };
  static const char fram_F[] = { "FRAM故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0100U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0100U )
    {
      memcpy( &ecd->spec[ecd->offset], fram_T, strlen( fram_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], fram_F, strlen( fram_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_FRAM */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_DAC_Selfcheck.
(^_^) Brief         : Recording the DAC self-checking event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_DAC_Selfcheck( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char dac_T[] = { "DAC自检故障" };
  static const char dac_F[] = { "DAC自检故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0200U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0200U )
    {
      memcpy( &ecd->spec[ecd->offset], dac_T, strlen( dac_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], dac_F, strlen( dac_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_DAC_Selfcheck */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_DAC_Output.
(^_^) Brief         : Recording the DAC output event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_DAC_Output( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char dac_T[] = { "DAC输出故障" };
  static const char dac_F[] = { "DAC输出故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0400U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0400U )
    {
      memcpy( &ecd->spec[ecd->offset], dac_T, strlen( dac_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], dac_F, strlen( dac_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_DAC_Output */


/**************************************************************************************************
(^_^) Function Name : ErrorCode_EBV_5V_Power.
(^_^) Brief         : Recording the 5V power event of EBV of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 26-September-2018.
(^_^) Parameter     : ecd --> the structure pointer of error code distribution.
(^_^) Return        :   0 --> the new event of error code occured.
(^_^)                 > 0 --> the new event of error code not occured.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ErrorCode_EBV_5V_Power( ErrorCodeDistribution *ecd )
{
  uint32_t exit_code = 0U;
  /* The specification of error code. */
  static const char power_T[] = { "5V电源故障" };
  static const char power_F[] = { "5V电源故障恢复" };
  
  if ( NULL == ecd )
  {
    exit_code = 1U;
  }
  else if ( 0U == ( ( ( uint32_t )ecd->ec_old ^ ( uint32_t )ecd->ec_new ) & 0x0800U ) )
  {
    exit_code = 2U;
  }
  else
  {
    uint8_t  state = 0x00U;
    
    if ( ecd->ec_new & 0x0800U )
    {
      memcpy( &ecd->spec[ecd->offset], power_T, strlen( power_T ) );
      
      state = ERROR_TRUE;
    }
    else
    {
      memcpy( &ecd->spec[ecd->offset], power_F, strlen( power_F ) );
      
      state = ERROR_FALSE;
    } /* end if...else */
    
    /* Side II. */
    if ( DEV_SIDE_II == ecd->side )
    {
      WriteErrorCodeToFlash( state, 0x82, ecd->spec );
    }
    /* Side I. */
    else
    {
      WriteErrorCodeToFlash( state, 0x81, ecd->spec );
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function ErrorCode_EBV_5V_Power */


/**************************************************************************************************
(^_^) Function Name : UploadErrorCode.
(^_^) Brief         : Uploading error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 5-September-2018.
(^_^) Parameter     :   0 --> datagram is valid.
(^_^)                 > 0 --> datagram is invalid.
(^_^) Return        : none.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t UploadErrorCode( void )
{
  uint32_t exit_code = 0U;
  
  /* 1. Assert datagram. */
  /* 13-November-2018, by Liang Zhen. */
  #if 0
  if ( AssertDatagram( ErrorCodeInitInst.LockFlag, ErrorCodeInitInst.DGM_Buffer ) )
  {
    if ( LOCK == ErrorCodeInitInst.LockFlag )
    {
      memset( ErrorCodeInitInst.DGM_Buffer, 0U, DGM_BUFFER_SIZE );
      
      ErrorCodeInitInst.LockFlag = UNLOCK;
    } /* end if */
    
    exit_code = 1U;
  } /* end if */
  #else
    uint32_t status = AssertDatagram( ErrorCodeInitInst.LockFlag,\
                                      ErrorCodeInitInst.DGM_Buffer );
    
    if ( status )
    {
      if ( LOCK == ErrorCodeInitInst.LockFlag )
      {
        memset( ErrorCodeInitInst.DGM_Buffer, 0U, DGM_BUFFER_SIZE );
        
        ErrorCodeInitInst.LockFlag = UNLOCK;
      } /* end if */
      
      exit_code = 1U;
    } /* end if */
  #endif
  
  /* 2. Assert state machine of error code. */
  switch ( ErrorCodeInitInst.EC_SM )
  {
    case EC_SM_IDLE : 
      Processing_EC_SM_IDLE( status, &ErrorCodeInitInst );
      break;
    
    case EC_SM_ASSERT : 
      Processing_EC_SM_ASSERT( &ErrorCodeInitInst );
      break;
    
    case EC_SM_SEND : 
      Processing_EC_SM_SEND( &ErrorCodeInitInst );
      break;
    
    case EC_SM_ACK : 
      Processing_EC_SM_ACK( status, &ErrorCodeInitInst );
      break;
    
    case EC_SM_EXCEPTION : 
      Processing_EC_SM_EXCEPTION( &ErrorCodeInitInst );
      break;
    
    default : 
      exit_code = 2U;
      ErrorCodeInitInst.EC_SM = EC_SM_IDLE;
      break;
  } /* end switch */

  return exit_code;
} /* end function UploadErrorCode */


/**************************************************************************************************
(^_^) Function Name : GetNewDatagram.
(^_^) Brief         : Get new datagram.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 4-September-2018.
(^_^) Parameter     : dgm --> datagram buffer pointer.
(^_^)                 len --> the length of datagram.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
uint32_t GetNewDatagram( uint8_t dgm[], uint32_t len )
{
  uint32_t exit_code = 0U;
  
  /* 1. Assert arguments. */
  if ( NULL == dgm )
  {
    exit_code = 1U;
  }
  else if ( ( 0U == len ) || ( len > DGM_BUFFER_SIZE ) )
  {
    exit_code = 2U;
  }
  else
  {
    if ( UNLOCK == ErrorCodeInitInst.LockFlag )
    {
      memcpy( ErrorCodeInitInst.DGM_Buffer, dgm, len );
      
      ErrorCodeInitInst.LockFlag = LOCK;
    }
    else
    {
      exit_code = 3U;
    } /* end if...else */
  } /* end if...else if...else */
  
  return exit_code;
} /* end function GetNewDatagram */


/**************************************************************************************************
(^_^) Function Name : AssertDatagram.
(^_^) Brief         : Assert the datagram of error code.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 5-September-2018.
(^_^) Parameter     : lock --> the lock status of datagram.
(^_^)                 dgm  --> the datagram of error code.
(^_^) Return        :   0  --> the datagram is valid.
(^_^)                 > 0  --> the datagram is invalid.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t AssertDatagram( LOCKER lock, uint8_t dgm[] )
{
  uint32_t exit_code = 0U;
  
  if ( UNLOCK == lock )
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
    
    if ( dgm_len > DGM_MAX_LEN )
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
        #if 0
        printf( "\r\n## 5 ##\r\n" );
        #endif
      }
      else
      {
        exit_code = 0U;
      } /* end if...else */
    } /* end if...else */
  } /* end if...else if...else */
  
  return exit_code;
} /* end function AssertDatagram */


/**************************************************************************************************
(^_^) Function Name : SM_IDLE_Processing.
(^_^) Brief         : Processing EC_SM_IDLE.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 5-September-2018.
(^_^) Parameter     : avai --> the datagram is available or not.
(^_^)                 inst --> the instance of error code structure.
(^_^) Return        :   0 --> operation successful.
(^_^)                 > 0 --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_EC_SM_IDLE( uint32_t avai, ErrorCodeInitStruct *inst )
{
  uint32_t exit_code = 0U;
  
  if ( avai )
  {
    exit_code = 1U;
  }
  else if ( NULL == inst )
  {
    exit_code = 2U;
  }
  else
  {
    /* 24-October-2018, by Liang Zhen. */
    #if 0
    if ( strncmp( ( char * )inst->DGM_Buffer, DGM_CMD_RQEC, 4U ) )
    #else
    if ( strncmp( DGM_CMD_RQHS, ( char * )inst->DGM_Buffer, 4U ) )
    #endif
    {
      exit_code = 3U;
    }
    else
    {
      /* 24-October-2018, by Liang Zhen. */
      #if 0
      inst->EC_SM = EC_SM_ASSERT;
      
      exit_code = 0U;
      #else
      if ( strncmp( HS_EC, ( char * )&inst->DGM_Buffer[16], 2U ) )
      {
        exit_code = 4U;
      }
      else
      {
        inst->EC_SM = EC_SM_ASSERT;
        
        exit_code = 0U;
      } /* end if...else */
      #endif
    } /* end if...else */
    
    memset( inst->DGM_Buffer, 0U, DGM_BUFFER_SIZE );
    
    inst->LockFlag = UNLOCK;
  } /* end if...else */
  
  return exit_code;
} /* end function Processing_EC_SM_IDLE */


/**************************************************************************************************
(^_^) Function Name : Processing_EC_SM_ASSERT.
(^_^) Brief         : Processing EC_SM_ASSERT.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : inst --> the instance of error code structure.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_EC_SM_ASSERT( ErrorCodeInitStruct *inst )
{
  uint32_t exit_code = 0U;
  
  /* 1. Assert arguments. */
  if ( NULL == inst )
  {
    exit_code = 1U;
  }
  else
  {
    /* 2. Initializing instance. */
    if ( ( inst->WriteAddr < EC_BASE_ADDR ) || ( inst->WriteAddr > EC_END_ADDR ) )
    {
      /* 2.1 EC_SM = EC_SM_EXCEPTION. */
      inst->EC_SM = EC_SM_EXCEPTION;
      
      exit_code = 2U;
    }
    else
    {
      /* 2.2 ReadAddr = WriteAddr - EC_MAX_LEN. */
      inst->ReadAddr = inst->WriteAddr;
      
      /* 2.3 Get the total size of datagram. */
      inst->DGM_TotalSize = GetDatagramTotalSize( inst->ReadAddr );
      
      /* 2.4 Set the amount of datagram, the number of datagram. */
      inst->DGM_AMNT = GetDatagramAmount( inst->DGM_TotalSize );
      
      /* 2.5 Assert reading address. */
      uint32_t offset = inst->ReadAddr - EC_BASE_ADDR;
      
      if ( offset >= EC_MAX_LEN )
      {
        inst->ReadAddr -= EC_MAX_LEN;
      }
      else
      {
        inst->ReadAddr = EC_BASE_ADDR;
      } /* end if...else */
      
      /* 24-October-2018, by Liang Zhen. */
      #if 0
        inst->DGM_NUM = 0U;
      #else
        inst->DGM_NUM = 1U;
      #endif
      
      /* 2.6 EC_SM  = EC_SM_SEND. */
      inst->EC_SM   = EC_SM_SEND;
      
      #if 1
        printf( "\r\n--> 1, raddr=%x, waddr=%x\r\n", inst->ReadAddr, inst->WriteAddr );
        printf( "\r\n--> 2, tsize=%d, amnt=%d\r\n", inst->DGM_TotalSize, inst->DGM_AMNT );
      #endif
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function Processing_EC_SM_ASSERT */


/**************************************************************************************************
(^_^) Function Name : Processing_EC_SM_SEND.
(^_^) Brief         : Processing EC_SM_SEND.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : inst --> the instance of error code structure.
(^_^) Return        :   0 --> waiting acknowledgement.
(^_^)                 > 0 --> operation abort.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_EC_SM_SEND( ErrorCodeInitStruct *inst )
{
  uint32_t exit_code = 0U;
  
  if ( NULL == inst )
  {
    exit_code = 1U;
  }
  else
  {
    /* 24-October-2018, by Liang Zhen. */
    #if 0
    if ( inst->DGM_NUM >= inst->DGM_AMNT )
    #else
    if ( ( 0U == inst->DGM_NUM ) || ( inst->DGM_NUM > inst->DGM_AMNT ) )
    #endif
    {
      inst->EC_SM = EC_SM_EXCEPTION;
      exit_code = 2U;
    }
    else
    {
      uint32_t len = GetDatagramLength( inst->DGM_NUM, inst->DGM_AMNT, inst->DGM_TotalSize );
      #if 0
      printf( "\r\n--> 3, len=%d,DGM_NUM=%d\r\n", len, inst->DGM_NUM );
      #endif
      memset( inst->SendBuffer, 0U, PRTC_MAX_LEN );
			#if 0
      printf( "\r\n--> 4, raddr=%x\r\n", inst->ReadAddr );
      #endif
      LoadDatagramData( inst, len );
      /* 14-January-2019, by Liang Zhen. */
      #if 0
      ETH_send( ( char * )inst->SendBuffer, len + 16U );
      #else
      udp_send_datagram( ( char * )inst->SendBuffer, len + 16U );
      #endif
      
      MoveToForwardAddress( len, &inst->ReadAddr );
      #if 0
      printf( "\r\n--> 4, raddr=%x\r\n", inst->ReadAddr );
      #endif
      
      memset( inst->DGM_Buffer, 0U, DGM_BUFFER_SIZE );
      inst->LockFlag = UNLOCK;
      inst->EC_SM    = EC_SM_ACK;
      inst->ACK_Time = rt_tick_get();
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function Processing_EC_SM_SEND */


/**************************************************************************************************
(^_^) Function Name : Processing_EC_SM_ACK.
(^_^) Brief         : Processing EC_SM_ACK.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 6-September-2018.
(^_^) Parameter     : avai --> the datagram is available or not.
(^_^)                 inst --> the instance of error code structure.
(^_^) Return        :   0  --> acknowledgement is success.
(^_^)                 > 0  --> acknowledgement is fail.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_EC_SM_ACK( uint32_t avai, ErrorCodeInitStruct *inst )
{
  uint32_t exit_code = 0U;
  
  /* 1. Assert arguments. */
  if ( NULL == inst )
  {
    exit_code = 1U;
  }
  else
  {
    if ( Common_BeTimeOutMN( &inst->ACK_Time, TIMEOUT_ACK ) )
    {
      inst->EC_SM = EC_SM_EXCEPTION;
      #if 0
      printf( "\r\n## 6 ##\r\n" );
      #endif
      exit_code = 2U;
    }
    else
    {
      if ( avai )
      {
        exit_code = 3U;
      }
      else
      {
        exit_code = ACK_DatagramProcessing( inst );
        
        memset( inst->DGM_Buffer, 0U, DGM_BUFFER_SIZE );
        
        inst->LockFlag = UNLOCK;
      } /* end if...else */
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function Processing_EC_SM_ACK */


/**************************************************************************************************
(^_^) Function Name : ACK_DatagramProcessing.
(^_^) Brief         : Processing the datagram of acknowledgement.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 7-September-2018.
(^_^) Parameter     : inst --> the instance of error code structure.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t ACK_DatagramProcessing( ErrorCodeInitStruct *inst )
{
  uint32_t exit_code = 0U;
  static uint32_t resend = 0U;
  
  if ( NULL == inst )
  {
    exit_code = 4U;
  }
  else
  {
    /* 24-October-2018, by Liang Zhen. */
    #if 0
    if ( 0U == strncmp( ( char * )inst->DGM_Buffer, DGM_CMD_DGCF, 4U ) )
    {
      resend++;
      #if 0
      printf( "\r\n## 3 ##\r\n" );
      #endif
      if ( resend >= RESEND_LIMIT )
      {
        inst->EC_SM = EC_SM_EXCEPTION;
      }
      else
      {
        inst->EC_SM = EC_SM_SEND;
        MoveToBackwardAddress( 0U, 2U, &inst->ReadAddr );
      } /* end if...else */
    }
    else if ( 0U == strncmp( ( char * )inst->DGM_Buffer, DGM_CMD_DGNS, 4U ) )
    {
      uint32_t tnum = ( ( uint32_t )inst->DGM_Buffer[14] << 24U )\
                    + ( ( uint32_t )inst->DGM_Buffer[15] << 16U )\
                    + ( ( uint32_t )inst->DGM_Buffer[16] << 8U )\
                    + ( ( uint32_t )inst->DGM_Buffer[17] << 0U );
      uint32_t fnum = ( ( uint32_t )inst->DGM_Buffer[18] << 24U )\
                    + ( ( uint32_t )inst->DGM_Buffer[19] << 16U )\
                    + ( ( uint32_t )inst->DGM_Buffer[20] << 8U )\
                    + ( ( uint32_t )inst->DGM_Buffer[21] << 0U );
      
      MoveToBackwardAddress( tnum, fnum, &inst->ReadAddr );
      #if 0
      printf( "\r\n## 4 ##\r\n" );
      #endif
      if ( ( tnum >= inst->DGM_AMNT )\
        || ( fnum >= inst->DGM_AMNT )\
        || ( tnum > fnum ) )
      {
        inst->EC_SM = EC_SM_EXCEPTION;
        exit_code = 5U;
      }
      else
      {
        inst->EC_SM = EC_SM_SEND;
        /* The next number is greater 1 than 'tnum'. */
        inst->DGM_NUM = tnum + 1U;
      } /* end if...else */
    }
    else if ( 0U == strncmp( ( char * )inst->DGM_Buffer, DGM_CMD_DGRS, 4U ) )
    {
      resend = 0U;
      inst->DGM_NUM++;
      #if 0
      printf( "\r\n## 1 ##\r\n" );
      #endif
      if ( inst->DGM_NUM >= inst->DGM_AMNT )
      {
        inst->EC_SM = EC_SM_IDLE;
      }
      else
      {
        inst->EC_SM = EC_SM_SEND;
      } /* end if...else */
    }
    #else
    if ( 0U == strncmp( DGM_CMD_ACKR, ( char * )inst->DGM_Buffer, 4U ) )
    {
      if ( 0U == strncmp( ACK_CF, ( char * )&inst->DGM_Buffer[16], 2U ) )
      {
        resend++;
        #if 0
          printf( "\r\n## 3 ##\r\n" );
        #endif
        if ( resend >= RESEND_LIMIT )
        {
          inst->EC_SM = EC_SM_EXCEPTION;
        }
        else
        {
          inst->EC_SM = EC_SM_SEND;
          MoveToBackwardAddress( 0U, 2U, &inst->ReadAddr );
        } /* end if...else */
      }
      else if ( 0U == strncmp( ACK_NS, ( char * )&inst->DGM_Buffer[16], 2U ) )
      {
        uint32_t tnum = ( ( uint32_t )inst->DGM_Buffer[20] << 24U )\
                      + ( ( uint32_t )inst->DGM_Buffer[21] << 16U )\
                      + ( ( uint32_t )inst->DGM_Buffer[22] << 8U )\
                      + ( ( uint32_t )inst->DGM_Buffer[23] << 0U );
        uint32_t fnum = ( ( uint32_t )inst->DGM_Buffer[24] << 24U )\
                      + ( ( uint32_t )inst->DGM_Buffer[25] << 16U )\
                      + ( ( uint32_t )inst->DGM_Buffer[26] << 8U )\
                      + ( ( uint32_t )inst->DGM_Buffer[27] << 0U );
        
        MoveToBackwardAddress( tnum, fnum, &inst->ReadAddr );
        #if 0
          printf( "\r\n## 4 ##, tnum=%08x, fnum=%08x\r\n", tnum, fnum );
        #endif
        if ( ( tnum > inst->DGM_AMNT )\
          || ( fnum > inst->DGM_AMNT )\
          || ( tnum > fnum ) )
        {
          /* 25-October-2018, by Liang Zhen. */
          #if 0
            inst->EC_SM = EC_SM_EXCEPTION;
          #endif
          
          exit_code = 5U;
        }
        else
        {
          inst->EC_SM = EC_SM_SEND;
          /* The next number is greater 1 than 'tnum'. */
          inst->DGM_NUM = tnum + 1U;
        } /* end if...else */
      }
      else if ( 0U == strncmp( ACK_RS, ( char * )&inst->DGM_Buffer[16], 2U ) )
      {
        resend = 0U;
        inst->DGM_NUM++;
        #if 0
          printf( "\r\n## 1 ##\r\n" );
        #endif
        if ( inst->DGM_NUM > inst->DGM_AMNT )
        {
          inst->EC_SM = EC_SM_IDLE;
        }
        else
        {
          inst->EC_SM = EC_SM_SEND;
        } /* end if...else */
      }
      else if ( 0U == strncmp( ACK_RT, ( char * )&inst->DGM_Buffer[16], 2U ) )
      {
        inst->EC_SM = EC_SM_SEND;
        #if 0
          printf( "\r\n## 7 ##, inst->DGM_NUM=%d, inst->DGM_AMNT=%d\r\n", inst->DGM_NUM, inst->DGM_AMNT );
        #endif
        MoveToBackwardAddress( 0U, 2U, &inst->ReadAddr );
      }
      else
      {
        exit_code = 7U;
      } /* end if...else if......else */
    }
    else if ( 0U == strncmp( DGM_CMD_RQHS, ( char * )inst->DGM_Buffer, 4U ) )
    {
      #if 0
        printf( "\r\n## 8 ##, %c%c\r\n", inst->DGM_Buffer[16], inst->DGM_Buffer[17] );
      #endif
      
      if ( 0U == strncmp( HS_EC, ( char * )&inst->DGM_Buffer[16], 2U ) )
      {
        inst->EC_SM = EC_SM_ASSERT;
      }
      else
      {
        exit_code = 8U;
      } /* end if...else */
    }
    #endif
    else
    {
      exit_code = 6U;
      #if 0
      printf( "\r\n## 2 ##\r\n" );
      #endif
    } /* end if...else if......else */
  } /* end if...else */
  
  return exit_code;
} /* end function ACK_DatagramProcessing */


/**************************************************************************************************
(^_^) Function Name : Processing_EC_SM_EXCEPTION.
(^_^) Brief         : Processing EC_SM_EXCEPTION.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 7-September-2018.
(^_^) Parameter     : inst --> the instance of error code structure.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t Processing_EC_SM_EXCEPTION( ErrorCodeInitStruct *inst )
{
  uint32_t exit_code = 0U;
  
  if ( NULL == inst )
  {
    exit_code = 1U;
  }
  else
  {
    inst->EC_SM = EC_SM_IDLE;
  } /* end if...else */
  
  return exit_code;
} /* end function Processing_EC_SM_EXCEPTION */


/**************************************************************************************************
(^_^) Function Name : MoveToBackwardAddress.
(^_^) Brief         : Move to the backward address according to datagram number.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 7-September-2018.
(^_^) Parameter     : tnum --> the last datagram number.
(^_^)                 fnum --> the incorrect datagram number.
(^_^)                 addr --> reading address.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t MoveToBackwardAddress( uint32_t tnum, uint32_t fnum, uint32_t *addr )
{
  uint32_t exit_code = 0U;
  
  if ( tnum > fnum )
  {
    exit_code = 1U;
  }
  else if ( NULL == addr )
  {
    exit_code = 2U;
  }
  else
  {
    uint32_t diff = fnum - tnum;
    
    if ( diff < 2U )
    {
      exit_code = 3U;
    }
    else
    {
      uint32_t new_addr = *addr + ( diff - 1U ) * DGM_MAX_LEN;
      
      if ( new_addr > EC_END_ADDR )
      {
        *addr = ( new_addr - EC_END_ADDR + 1U ) + EC_BASE_ADDR;
      }
      else
      {
        *addr = new_addr;
      } /* end if...else */
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function MoveToBackwardAddress */


/**************************************************************************************************
(^_^) Function Name : MoveToForwardAddress.
(^_^) Brief         : Move to the forward address according to datagram number.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 7-September-2018.
(^_^) Parameter     : size --> the size of moving address.
(^_^)                 addr --> reading address.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t MoveToForwardAddress( uint32_t size, uint32_t *addr )
{
  uint32_t exit_code = 0U;
  
  if ( 0U == size )
  {
    exit_code = 1U;
  }
  else if ( NULL == addr )
  {
    exit_code = 2U;
  }
  else
  {
    if ( *addr < EC_BASE_ADDR )
    {
      exit_code = 3U;
    }
    else
    {
      uint32_t offset = *addr - EC_BASE_ADDR;
      
      if ( size > offset )
      {
        *addr = ( EC_END_ADDR + 1U ) - ( size - offset );
      }
      else
      {
        *addr -= size;
      } /* end if...else */
    } /* end if...else */
  } /* end if...else */
  
  return exit_code;
} /* end function MoveToForwardAddress */


/**************************************************************************************************
(^_^) Function Name : LoadDatagramData.
(^_^) Brief         : Loading datagram data.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 7-September-2018.
(^_^) Parameter     : inst --> the instance of error code structure.
(^_^)                 size --> the size of datagram which will be sent to client.
(^_^) Return        :   0  --> operation successful.
(^_^)                 > 0  --> operation failed.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t LoadDatagramData( ErrorCodeInitStruct *inst, uint32_t size )
{
  uint32_t exit_code = 0U;
  	
  if ( NULL == inst )
  {
//		printf("\r\n@@@@@@@@@ NULL == inst @@@@@@@@@\r\n");
    exit_code = 1U;
  }
  else if ( inst->ReadAddr % EC_MAX_LEN )
  {
//		printf("\r\n@@@@@@@@@ iexit_code = 2U %d %d @@@@@@@@@\r\n",(inst->ReadAddr - EC_BASE_ADDR),EC_MAX_LEN);
    exit_code = 2U;
  }
  else if ( ( 0U == size ) || ( size % EC_MAX_LEN ) )
  {
    exit_code = 3U;
  }
  else
  { //printf("\r\nFFFFFFFFFFFFFFFFFFFFFF\r\n");
    inst->SendBuffer[0] = 'U';
    inst->SendBuffer[1] = 'P';
    inst->SendBuffer[2] = 'E';
    inst->SendBuffer[3] = 'C';
    
    inst->SendBuffer[4] = ( uint8_t )( inst->DGM_AMNT >> 24U );
    inst->SendBuffer[5] = ( uint8_t )( inst->DGM_AMNT >> 16U );
    inst->SendBuffer[6] = ( uint8_t )( inst->DGM_AMNT >> 8U );
    inst->SendBuffer[7] = ( uint8_t )( inst->DGM_AMNT >> 0U );
    
    inst->SendBuffer[8] = ( uint8_t )( inst->DGM_NUM >> 24U );
    inst->SendBuffer[9] = ( uint8_t )( inst->DGM_NUM >> 16U );
    inst->SendBuffer[10] = ( uint8_t )( inst->DGM_NUM >> 8U );
    inst->SendBuffer[11] = ( uint8_t )( inst->DGM_NUM >> 0U );
    
    inst->SendBuffer[12] = ( uint8_t )( size >> 8U );
    inst->SendBuffer[13] = ( uint8_t )( size >> 0U );
    
    uint32_t addr = inst->ReadAddr;
    
    for ( uint32_t i = 0U; i < size; i += EC_MAX_LEN )
    {
      S25FL256S_Read( ( uint32_t * )( &inst->SendBuffer[i + 14] ), EC_MAX_LEN, addr );
      
      if ( addr > EC_BASE_ADDR )
      {
        addr -= EC_MAX_LEN;
      }
      else
      {
        addr  = ( EC_END_ADDR + 1U ) - EC_MAX_LEN;
      } /* end if...else */
    } /* end for */
    
    uint16_t crc = Common_CRC16( inst->SendBuffer, size + 14U );
    
    inst->SendBuffer[size + 14] = ( uint8_t )( ( uint32_t )crc >> 8U );
    inst->SendBuffer[size + 15] = ( uint8_t )crc;
  } /* end if...else if...else */
  
  return exit_code;
} /* end function LoadDatagramData */


/**************************************************************************************************
(^_^) Function Name : GetDatagramLength.
(^_^) Brief         : Get the length of datagram.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 7-September-2018.
(^_^) Parameter     : num  --> datagram number.
(^_^)                 amnt --> datagram amount.
(^_^)                 size --> total size of datagram.
(^_^) Return        : The length of datagram.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint16_t GetDatagramLength( uint32_t num, uint32_t amnt, uint32_t size )
{
  uint16_t len = 0x0000U;
  
  if ( 0U == amnt )
  {
    len = 0x0000U;
  }
  else
  {
    if ( ( amnt - 1U ) > num )
    {
      len = ( uint16_t )DGM_MAX_LEN;
    }
    else
    {
      len = ( uint16_t )( size - ( ( amnt - 1U ) * DGM_MAX_LEN ) );
    } /* end if...else */
  } /* end if...else */
  
  return len;
} /* end function GetDatagramLength */


/**************************************************************************************************
(^_^) Function Name : Get_XOR_Byte.
(^_^) Brief         : Get the xor value of array.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 10-September-2018.
(^_^) Parameter     : array --> array buffer's pointer.
(^_^)                 size  --> the size of array.
(^_^) Return        : The xor value of array.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint8_t Get_XOR_Byte( uint8_t array[], uint32_t size )
{
  uint8_t xor_tmp = 0x00U;
  
  /* 1. Assert arguments. */
  if ( NULL == array )
  {
    xor_tmp = 0x00U;
  }
  else if ( 0U == size )
  {
    xor_tmp = 0xFFU;
  }
  else
  {
    for ( uint32_t i = 0U; i < size; i++ )
    {
      xor_tmp ^= array[i];
    } /* end for */
  } /* end if...else if...else */
  
  return xor_tmp;
} /* end function Get_XOR_Byte */


/**************************************************************************************************
(^_^) Function Name : GetDatagramAmount.
(^_^) Brief         : Get the amount of datagram.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 11-September-2018.
(^_^) Parameter     : size --> the total size of datagram.
(^_^) Return        :   0  --> operation failed.
(^_^)                 > 0  --> operation successful.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t GetDatagramAmount( uint32_t size )
{
  uint32_t amnt = 0U;
  
  /* 1. Assert argument. */
  if ( 0U == size )
  {
    amnt = 0U;
  }
  else
  {
    /* 2. Get the amount of datagram. */    
    amnt = size / DGM_MAX_LEN;
    
    if ( size % DGM_MAX_LEN )
    {
      amnt++;
    } /* end if */
  } /* end if...else */
  
  return amnt;
} /* end function GetDatagramAmount */


/**************************************************************************************************
(^_^) Function Name : GetDatagramTotalSize.
(^_^) Brief         : Get the total size of datagram.
(^_^) Author        : Liang Zhen.
(^_^) Date          : 11-September-2018.
(^_^) Parameter     : base --> the based address of reading recordings of error code.
(^_^) Return        :   0  --> operation failed.
(^_^)                 > 0  --> operation successful.
(^_^) Hardware      : none.
(^_^) Software      : none.
(^_^) Note          : Before using the function, please read comments carefully.
**************************************************************************************************/
static uint32_t GetDatagramTotalSize( uint32_t base )
{
  uint32_t size = 0U;
  
  /* 1. Assert argument. */
  if ( ( base < EC_BASE_ADDR ) || ( base > EC_END_ADDR ) )
  {
    size = 0U;
  }
  else
  {
    /* 2. Get the amount of datagram. */
    uint32_t offset = base - EC_BASE_ADDR;
    
    if ( offset < ( EC_UPLOAD_AMNT * EC_MAX_LEN ) )
    {
      uint8_t tmp[FLASH_PAGE_SIZE];
      
      S25FL256S_Read( ( uint32_t * )tmp, FLASH_PAGE_SIZE, EC_END_ADDR + 1U - FLASH_PAGE_SIZE );
      
      if ( AssertPage( tmp ) )
      {
        size = EC_UPLOAD_AMNT * EC_MAX_LEN;
      }
      else
      {
        size = offset;
      } /* end if...else */
    }
    else
    {
      size = EC_UPLOAD_AMNT * EC_MAX_LEN;
    } /* end if...else */
  } /* end if...else */
  
  return size;
} /* end function GetDatagramTotalSize */


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


