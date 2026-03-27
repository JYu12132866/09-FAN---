/*******************************************************
* @file iap.h
* @author Primo Wu (wuxy101@avic.com)
* @brief 
* @version 1.0
* @date 2024-09-04
* 
* @copyright Copyright (c) 2024 .
* 
*******************************************************/
#pragma once
#include "stdint.h"

#define IAP_MAGIC           0X49415044   //'IAPD' -> IAP DATA
#define IAP_MAX_PACK_LEN    1024 


#pragma pack(1)
typedef struct 
{
    uint32_t    magic_num;
    uint8_t     pack_total;
    uint8_t     pack_no;
    uint32_t    pack_len;        
    uint32_t    data_crc;
    uint8_t    crc_flag;
    uint8_t    rsv[3];
}IAP_Header;

#pragma pack()

void iap_boot_update_loop(void);
