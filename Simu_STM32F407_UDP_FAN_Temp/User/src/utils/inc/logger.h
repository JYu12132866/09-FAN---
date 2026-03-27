/**
 * @file logger.h
 * @author Primo Wu (wuxinyu@sae118.net)
 * @brief 
 * @version 0.1
 * @date 2023-10-08
 * 
 * @copyright Copyright (c) 2023 Smartsens.
 * 
 */

#ifndef APP_PUBLIC_LOGGER_H_
#define APP_PUBLIC_LOGGER_H_

#define __DEBUG    //日志模块总开关，注释掉将关闭日志输出

#ifdef __DEBUG
    #define LOG_DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(format, ...)
#endif

typedef enum LOG_LEVEL{    
    LOG_LEVEL_OFF=0,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_ALL,
}LOG_LEVEL_;

extern LOG_LEVEL_  logger_level;  //Log打印级别设定


/**
 * @brief 初始化Logger模块，遍历flash中log空间的每一片log，找到最后一片log的地址；同时设置打印级别
 * 
 * @param level 打印级别
 */
void Init_Logger(LOG_LEVEL_ level);
/**
 * @brief Flash新增Log记录
 * 
 * @param format 格式化字符串
 */
void Update_Log_To_Flash(const char *format, ...);
/**
 * @brief 读出Flash中的数据，并打印到调试UART
 * 
 */
void ShowLogs_To_Terminal(void);
/**
 * @brief Log模块测试函数
 * 
 */
void test_log_module(void);

#define log_fatal(format, ...) \
    do { \
        if(logger_level>=LOG_LEVEL_FATAL){\
            LOG_DEBUG("#F:%s L:%d " format "\n", __FILE__, __LINE__, ##__VA_ARGS__ );\
            Update_Log_To_Flash("#F:%s L:%d " format "\n", __FILE__, __LINE__, ##__VA_ARGS__ );\
        }\
    } while (0)

#define log_err(format, ...) \
    do { \
         if(logger_level>=LOG_LEVEL_ERR)\
           LOG_DEBUG("##  ERR!  @ FILE:%s LINE:%d " format "\n",\
                     __FILE__, __LINE__, ##__VA_ARGS__ );\
    } while (0)

#define log_warn(format, ...) \
    do { \
         if(logger_level>=LOG_LEVEL_WARN)\
           LOG_DEBUG("#   WARN  @ " format "\n", ##__VA_ARGS__ );\
    } while (0)

#define log_info(format, ...) \
    do { \
         if(logger_level>=LOG_LEVEL_INFO)\
           LOG_DEBUG("[INFO]  "format"\n",##__VA_ARGS__ );\
    } while (0)

#define log_debug(format, ...) \
    do { \
         if(logger_level>=LOG_LEVEL_ALL)\
           LOG_DEBUG("[DEBUG] "format"\n",##__VA_ARGS__ );\
    } while (0)

#endif 


