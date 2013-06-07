/* 7zDecode.h */

#ifndef __7Z_DECODE_H
#define __7Z_DECODE_H

#include "Types.h"

#include <stdio.h>

//#include "7zItem.h"
//#include "7zAlloc.h"
/*
#ifdef _LZMA_IN_CB
#include "7zIn.h"
#endif


SZ_RESULT SzDecode(const CFileSize *packSizes, const CFolder *folder,
    #ifdef _LZMA_IN_CB
    ISzInStream *stream,
    #else
    const Byte *inBuffer,
    #endif
    Byte *outBuffer, size_t outSize, 
    size_t *outSizeProcessed, ISzAlloc *allocMain);
*/

unsigned char *LZMADecode_C ( unsigned char *inBuffer, size_t inSize, size_t *outSizeProcessed, int *result );

#endif

