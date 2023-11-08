/*******************************************************************************************
 file information
 ********************************************************************************************
 **@file  : support_filesys.h
 **@author: Created By Chengt
 **@date  : 2022.04.16
 **@brief : Implement the function interfaces of support_filesys
 ********************************************************************************************/

#ifndef _SUPPORT_FILESYS_H
#define _SUPPORT_FILESYS_H

#include <stdio.h>
#include "common.h"

extern sint32 support_fileSys_readFile(const uint8 *filePath, const uint8 *fileName, const void *buf, uint32 len,
        uint32 offset);
extern sint32 support_fileSys_writeFile(const uint8 *filePath, const uint8 *fileName, const void *buf, uint32 len);
extern sint32 support_fileSys_getFileSize(uint8 *filePathName, uint8 *filePath, uint8 *fileName);

#endif
/**************************************end file*********************************************/
