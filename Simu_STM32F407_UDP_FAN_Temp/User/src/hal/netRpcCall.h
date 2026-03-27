#ifndef _NET_RPC_CALL_H
#define _NET_RPC_CALL_H

#include <stdint.h>

/**
 * @brief 网络RPC调用解析函数
 * 
 * @param rpcCmdStr JSON-RPC字符串
 * @return int 执行结果
 */
int netRpcParse(const char *rpcCmdStr);

/**
 * @brief MST芯片透传数据通知函数
 * 
 * @param data 透传数据指针
 * @param data_len 数据长度
 * @return int 执行结果
 */
int mstar_transparent_notify(uint8_t* data, uint8_t data_len);

#endif // _NET_RPC_CALL_H

