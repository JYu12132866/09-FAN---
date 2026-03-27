#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include <stdbool.h>


uint8_t simple_checksum(uint8_t* buffer, int len);

/**
 * @brief 以十六进制格式打印内存数据（带标签）
 * @param tag   指示词/标签名
 * @param buf   数据缓冲区指针
 * @param len   数据长度
 */
void print_mem_hexdump(const char* tag, const void* buf, int len);

bool isVaildIp(const char* ip);

#endif
