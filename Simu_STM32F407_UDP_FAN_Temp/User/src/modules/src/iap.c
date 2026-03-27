/**
 * @file iap.c
 * @author Primo Wu (wuxinyu@sae118.net)
 * @brief
 * @version 0.1
 * @date 2023-10-08
 *
 * @copyright Copyright (c) 2023 .
 *
 */
#include "iap.h"
#include "lan8720.h"
#include "logger.h"
#include "lwip_comm.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stmflash.h"
#include "udp_demo.h"
#include <string.h>

/*  Flash Partation Table (stm32f407zet6 1024K Flash)

    Section         Address          Size       Part Name
    ------------------------------------------------------
        0           0x8000000        16K        IAP Code
        1           0x8004000        16K        IAP Code
        2           0x8008000        16K        IAP Code
        3           0x800C000        16K        IAP param
    -------------------------------------------------------
        4           0x8010000        64K        APP Code
        5           0x8020000        128K       APP Code
        6           0x8040000        128K       APP Code
    -------------------------------------------------------
        7           0x8060000        128K       APP Param
    -------------------------------------------------------
        ...         ...              ...        ...

 */
// clang-format off
#define FLASH_BASE_ADDR 		0x08000000
#define SRAM_BASE_ADDR			0x20000000
 
#define FLASH_BOOT_CODE_ADDR        ADDR_FLASH_SECTOR_0
#define FLASH_BOOT_PARAM_ADDR       ADDR_FLASH_SECTOR_3
#define FLASH_APP_CODE_ADDR         ADDR_FLASH_SECTOR_4
#define FLASH_APP_PARAM_ADDR        ADDR_FLASH_SECTOR_7

#define CMD_ENTER_UPDATE            "ENTER_UPDATE"    
#define RET_ENTER_UPDATE            "ENTER_UPDATE_OK"

#define RET_PACK_PROG_DONE          "PACK_PROG_DONE"
#define RET_PACK_PROG_FAIL          "PACK_PROG_FAIL"

#define CMD_PROG_EXIT               "PROG_EXIT"
#define RET_PROG_DONE               "PROG_DONE"

#define CMD_IS_UPDATE               "IS_NEED_UPDATE"
#define ACK_IS_UPDATE               "IS_NEED_UPDATE_ACK"

#define CHECK_UDP_RECV() (udpRcvFlag)
#define CLEAR_UDP_RECV() \
    do { \
        udpRcvFlag = 0 ; \
    } while (0)

// clang-format on

//函数宏定义，简化函数调用
#define  udp_send_to_lastConnect(a,b)   udp_send_remote(a,b) 

/**
 * @brief 加载APP Code并跳转执行
 *
 */
void iap_reboot(void)
{
    log_info("Reset to bootloader...\r\n");
    HAL_NVIC_SystemReset();
}

/**
 * @brief Boot Code固件升级循环
 *
 */
void iap_boot_update_loop(void)
{
    log_info("Enter Boot update mode.");

    int ret = 0;
    int timeoutTick = 0;
    int startTick = 0;

    startTick = HAL_GetTick();

    while (1)
    {

        // 喂狗,防止烧录一半重启
        HAL_IWDG_Refresh(&iwdg);

        timeoutTick = HAL_GetTick() - startTick;
        if ((timeoutTick % 1000) == 0)
        {
            printf("[Update Mode]: waiting command...[%d]\n", timeoutTick / 1000);
            if (timeoutTick > 20000)
            {
                printf("Wait for update trigger timeout. Exit.\n");
                break;
            }
        }

        if (CHECK_UDP_RECV()) // 是否收到数据?
        {
            startTick = HAL_GetTick();

            HAL_Delay(10); // MCU收到包后等待10ms再处理，防止MCU跑太快，上位机逻辑错误
            CLEAR_UDP_RECV();
            if (recv_data_len == strlen(CMD_ENTER_UPDATE) &&
                memcmp(udp_recvbuf, CMD_ENTER_UPDATE, strlen(CMD_ENTER_UPDATE)) == 0) // 响应状态查询CMD
            {
                printf("Check is enter update mode.\n");
                udp_send_to_lastConnect(RET_ENTER_UPDATE, strlen(RET_ENTER_UPDATE));
                continue;
            }
            else if (recv_data_len == strlen(CMD_PROG_EXIT) &&
                     memcmp(udp_recvbuf, CMD_PROG_EXIT, strlen(CMD_PROG_EXIT)) == 0)
            {
                printf("Update done, exit.\n");
                udp_send_to_lastConnect(RET_PROG_DONE, strlen(RET_PROG_DONE));
                iap_reboot();
                break;
            }
            else
            {
                IAP_Header *header = (IAP_Header *)udp_recvbuf;
                if (recv_data_len <= sizeof(IAP_Header) ||
                    recv_data_len != sizeof(IAP_Header) + header->pack_len) // 数据长度不足，丢弃
                {
                    log_warn("Data pack len illegal, continue wait for cmd.");
                    continue;
                }

                if ((header->magic_num != IAP_MAGIC) || (header->pack_no > header->pack_total) ||
                    (header->pack_len > IAP_MAX_PACK_LEN)) // 数据头不合法，丢弃
                {
                    log_warn("Data pack header illegal, continue wait for cmd.");
                    udp_send_to_lastConnect(RET_PACK_PROG_FAIL,
                                            strlen(RET_PACK_PROG_FAIL)); // udp 回包，当前数据包不合法，烧录失败
                    continue;
                }
                else
                {
                    // 数据包合法，开始处理
                    uint32_t prog_addr = FLASH_BOOT_CODE_ADDR + (header->pack_no - 1) * IAP_MAX_PACK_LEN;
                    uint32_t *pack_data = (uint32_t *)(udp_recvbuf + sizeof(IAP_Header));
                    uint32_t pack_len = header->pack_len;
                    uint32_t write_times = (pack_len + 3) / 4; // 写入次数，每次写入4字节，不足4字节按4字节计算，向上取整
                    uint32_t pack_crc = header->data_crc;
                    uint32_t crc = 0;
                    log_debug("pack %d / %d , data len = %d, flash addr = 0x%08x, crc = 0x%08x", header->pack_no,
                              header->pack_total, pack_len, prog_addr, pack_crc);

                    if(header->crc_flag)
                    {
                        // CheckSum 校验，不用CRC,减小计算量
                        for (size_t i = 0; i < pack_len; i++)
                            crc += ((uint8_t*)pack_data)[i];
                        
                        if (crc != pack_crc) // 数据包CRC校验失败，丢弃
                        {
                            log_warn("Data pack crc = 0x%08x check failed, continue wait for cmd.",crc);
                            udp_send_to_lastConnect(RET_PACK_PROG_FAIL,
                                                    strlen(RET_PACK_PROG_FAIL)); // udp 回包，当前数据包校验失败，烧录失败
                            continue;
                        }
                        else{
                            log_info("Data pack crc = 0x%08x check success.",crc);
                        }
                    }
                    // 数据包CRC校验成功，开始烧录
                    ret = STMFLASH_Write_With_Erase(prog_addr, pack_data, write_times);
                    if (ret == 0)
                    {
                        // 发送烧录完成udp回包
                        udp_send_to_lastConnect(RET_PACK_PROG_DONE, strlen(RET_PACK_PROG_DONE));
                        continue;
                    }
                    else
                    {
                        log_warn("Page %d Program failed.", header->pack_no);
                        // 发送烧录失败udp回包
                        udp_send_to_lastConnect(RET_PACK_PROG_FAIL, strlen(RET_PACK_PROG_FAIL));
                        continue;
                    }
                
                }
            }
        }
    }
}

