/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-09-13     zm       the first version
 */
#include "linklib/inc/if_file.h"

#define DBG_TAG "if_file"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include <rtthread.h>
#include <rtdevice.h>

FILE *if_file_open( const char *pfilename, const char *pmode )
{
    return NULL;
}

void if_file_close( FILE *fp )
{

}

uint32 if_file_read( FILE *fp, void *pbuf, uint32 len )
{
    return 0;
}

uint32 if_file_write( FILE *fp, void *pbuf, uint32 len )
{
    return 0;
}

BOOL if_file_seek( FILE *fp, uint32 offset, sint32 mode )
{
    return TRUE;
}

BOOL if_file_stat( const char *pfilename, struct stat *pbuf )
{
    return TRUE;
}

