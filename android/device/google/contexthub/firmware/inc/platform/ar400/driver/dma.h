/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 DMA module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : dma.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-7-1
* Descript: DMA controller public header.
* Update  : date                auther      ver     notes
*           2012-7-1 16:54:34   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __DMA_H__
#define __DMA_H__

/*
*********************************************************************************************************
*                                       DMA MEMORY COPY
*
* Description:  copy 'src' to 'dest' use DMA.
*
* Arguments  :  dest    : the dest buffer;
*               src     : the source buffer;
*               len     : the byte size of data to copy.
*
* Returns    :  OK if copy succeeded, others if failed.
*********************************************************************************************************
*/
#if DMA_USED
s32 dma_memcpy(void *dest, void *src, u32 len);
#endif

#endif  //__DMA_H__
