#include "logger.h"
#include "stmflash.h"
#include "string.h"
#include "malloc.h"
#include "stdio.h"
#include <stdlib.h>
#include <stdarg.h>

#define LOG_SECTOR_ADDR     ADDR_FLASH_SECTOR_7 //扇区7起始地址, 0x08060000
#define LOG_SECTOR_SIZE     0x20000             //128KByte
#define LOG_MAX_NUMS        2048                //最大log条数
#define SINGLE_LOG_SIZE     64                  //单条log数据块大小
#define SINGLE_LOG_TEXT_LEN 63                  //log文本长度
#define RELOAD_LOG_NUMS     20                  //log空间已满时，重新缓存并加载的log条数

#define LOG_ITEM_USED_FLAG      0x00
#define LOG_ITEM_UNUSED_FLAG    0xff

#pragma pack(1)
typedef struct {
    
    uint8_t used_flag;      //log空间已使用标志，0xff表示未使用，0x00表示已使用
    uint8_t log_text[63];   //log文本
}LOG_ITEM;
#pragma pack()

LOG_LEVEL_  logger_level = LOG_LEVEL_ALL;  //Log打印级别设定
volatile int latest_log_idx = 0;


/**
 * @brief 读出单条log数据的使用标志 
 * 
 * @return uint8_t 
 */
inline uint8_t read_log_used_flag(int idx)
{
    return STMFLASH_ReadByte(LOG_SECTOR_ADDR + idx*SINGLE_LOG_SIZE);
}

/**
 * @brief 读出单条log数据
 * 
 * @param idx  log空间索引号
 * @param p_log_item log数据存放地址
 */
inline void read_log_item(int idx, LOG_ITEM *p_log_item)
{
    STMFLASH_Read(LOG_SECTOR_ADDR + idx * SINGLE_LOG_SIZE, (uint32_t *)p_log_item, sizeof(LOG_ITEM)/4);
}

/**
 * @brief 写入单条log数据，STMFLASH_Write 内部自动判断Flash是否需要擦除
 * 
 * @param addr 
 * @param p_log_item log数据存放地址
 */
inline void write_log_item(int idx, LOG_ITEM *p_log_item)
{
    STMFLASH_Write(LOG_SECTOR_ADDR + idx * SINGLE_LOG_SIZE, (uint32_t *)p_log_item, sizeof(LOG_ITEM)/4);
}

void Init_Logger(LOG_LEVEL_ level)
{
    uint8_t used_flag = 0xff;
    for (size_t i = 0; i < LOG_MAX_NUMS; i++)
    {
        used_flag = read_log_used_flag(i);
        if (used_flag == LOG_ITEM_USED_FLAG)
        {
            latest_log_idx = i;
            break;
        }
    }
    printf("[LOG]Flash log latest idx = %d\n", latest_log_idx);
    logger_level = level;//设置打印级别
}

void Update_Log_To_Flash(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    static LOG_ITEM log_item = {0};

    //格式化字符串
    int chars_to_write = vsnprintf((char*)(log_item.log_text), SINGLE_LOG_TEXT_LEN, format, args);
    va_end(args);

    if (chars_to_write < 0) {
        // 格式化错误，打印警告
        printf("[Log]Log formatting error.\n");
        return;
    }
    //字符串太长，无法完全放入缓冲区，打印警告
    if (chars_to_write >= SINGLE_LOG_TEXT_LEN) {
        log_item.log_text[SINGLE_LOG_TEXT_LEN-1] = '\0'; //越界则截断，末尾补'\0';
        printf("[Log]Log message is too long and will be truncated.\n");
    }
    log_item.used_flag = LOG_ITEM_USED_FLAG; //新的一条Log数据储存完成

    //若Log存满，需要清理并重新缓存
    if(latest_log_idx >= LOG_MAX_NUMS) 
    {
        latest_log_idx = 0;
        uint8_t *tmp_buf = (uint8_t*)mymalloc(SRAMIN, SINGLE_LOG_SIZE * RELOAD_LOG_NUMS);
        if( tmp_buf == NULL ) //若malloc出错，则不再缓存，写入一条出错Log
        {
            LOG_ITEM malloc_err_log = {
                .used_flag = 0,
                .log_text = "[LOG]Log reload ram malloc fail!\n",
            };
            write_log_item(latest_log_idx, &malloc_err_log); 
            latest_log_idx++;
        }
        else //缓存旧Log共RELOAD_LOG_NUMS条
        {
            for (size_t i = 0; i < RELOAD_LOG_NUMS; i++)
            {
                read_log_item(LOG_MAX_NUMS-(20-i)-1, (LOG_ITEM*)(tmp_buf + i * sizeof(LOG_ITEM)));   // 1024-1-(20-i)
            }
            //先读后写，写入时自动擦除正片sector
            for (size_t i = 0; i < RELOAD_LOG_NUMS; i++)
            {
                write_log_item(latest_log_idx, (LOG_ITEM*)(tmp_buf + i * sizeof(LOG_ITEM)));
                latest_log_idx++;
            }   
        }
    }
    //保存新的一条Log数据进入Flash
    write_log_item(latest_log_idx, &log_item);
    latest_log_idx++;
}

void ShowLogs_To_Terminal(void)
{
    LOG_ITEM show_log = {0};

    for (size_t i = 0; i < latest_log_idx; i++)
    {
        read_log_item( i, &show_log);
        printf("[LOG]Flash_log[%d]:%s", i, show_log.log_text);
    }
}

void test_log_module(void)
{
    while(1)
  {
    //测试循环写入
    for (size_t i = 0; i < 2068; i++)
    {
      log_fatal("fetal log %d",i);
    }
    //测试数据长度超出64
      log_fatal("-123456789-123456789-123456789-123456789-123456789-123456789-123456789");
    //读出所有Log确认是否正常工作
    ShowLogs_To_Terminal();
    while(1)
    {
    }
  }
}




