/*******************************************************************************************
                                file information
********************************************************************************************
**@file  : if_file.h
**@author: Created By Chengt
**@date  : 2023.09.06
**@brief : Implement the function interfaces of if_file
********************************************************************************************/
#ifndef _IF_FILE_H
#define _IF_FILE_H

#include <stdio.h>
#include "common.h"

#include <sys/stat.h>

extern FILE *if_file_open( const char *pfilename, const char *pmode );
extern void if_file_close( FILE *fp );
extern uint32 if_file_read( FILE *fp, void *pbuf, uint32 len );
extern uint32 if_file_write( FILE *fp, void *pbuf, uint32 len );
extern BOOL if_file_seek( FILE *fp, uint32 offset, sint32 mode );
extern BOOL if_file_stat( const char *pfilename, struct stat *pbuf );

#endif
/*******************************************end file*******************************************************/
