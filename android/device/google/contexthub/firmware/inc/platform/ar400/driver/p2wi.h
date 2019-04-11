/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 p2wi module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : p2wi.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-18
* Descript: p2wi controller public header.
* Update  : date                auther      ver     notes
*           2012-5-18 9:55:00   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __P2WI_H__
#define __P2WI_H__

/*
*********************************************************************************************************
*                                       INITIALIZE P2WI
*
* Description:  initialize p2wi module.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize p2wi succeeded, others if failed.
*********************************************************************************************************
*/
s32 p2wi_init(void);

/*
*********************************************************************************************************
*                                       EXIT P2WI
*
* Description:  exit p2wi module.
*
* Arguments  :  none.
*
* Returns    :  OK if exit p2wi succeeded, others if failed.
*********************************************************************************************************
*/
s32 p2wi_exit(void);

/*
*********************************************************************************************************
*                                           READ DATA BY P2WI
*
* Description:  read data by p2wi.
*
* Arguments  :  addr    : the point of address, size is specified by len.
*               data    : the point of data, size is specified by len.
*               len     : the length of data.
*
* Returns    :  OK if read data succeeded, others if failed.
*********************************************************************************************************
*/
s32 p2wi_read(u8 *addr, u8 *data, u32 len);

/*
*********************************************************************************************************
*                                           WRITE DATA BY P2WI
*
* Description:  write data by p2wi.
*
* Arguments  :  addr    : the point of address, size is specified by len.
*               data    : the point of data, size is specified by len.
*               len     : the length of data.
*
* Returns    :  OK if write data succeeded, others if failed.
*********************************************************************************************************
*/
s32 p2wi_write(u8 *addr, u8 *data, u32 len);

s32 p2wi_read_block_data(struct message *pmessage);
s32 p2wi_write_block_data(struct message *pmessage);
s32 p2wi_bits_ops_sync(struct message *pmessage);

#endif  //__P2WI_H__
