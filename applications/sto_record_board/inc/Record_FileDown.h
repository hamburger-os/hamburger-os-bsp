#ifndef __RECORD_FILEDOWN_
#define __RECORD_FILEDOWN_

#include "common.h"

typedef struct {
    uint8_t name[24];
    uint8_t size[4];
    uint8_t fileID;
} NAME_ID;

typedef struct {
    uint8_t flag[2];
    /* 文件总数 */
    uint8_t file_sum;
    /* 本包文件个数 */
    uint8_t file_count;
    /* 总包数 */
    uint8_t package_sum;
    uint8_t packageID;
    uint8_t crc16[2];
    NAME_ID contant[6];
} ETH_SEND;

typedef struct {
    uint8_t flag[2];
    uint8_t file_count;
    uint8_t crc16[2];
    uint8_t ID[195];
} ETH_REC;

enum APPLY_CMD
{
    NONECMD,
    DIRECTORYCMD,
    FILECMD,
};

extern volatile uint8_t Socket_Connect_Flag;
extern volatile uint8_t Socket_Closed_Flag;
extern volatile uint8_t Socket_Receive_Flag;

extern void RecordBoard_FileDown(void);


/* 13-November-2018, by Liang Zhen. */
/* include ------------------------------------------------------------------------------------- */


/* public macro definition --------------------------------------------------------------------- */


/* public type definition ---------------------------------------------------------------------- */


/* public function declaration ----------------------------------------------------------------- */
void ThreadFileDownload( void );
uint32_t DownloadFileInit( void );
uint32_t GetDownloadDatagram( uint8_t dgm[], uint32_t size );


#endif

