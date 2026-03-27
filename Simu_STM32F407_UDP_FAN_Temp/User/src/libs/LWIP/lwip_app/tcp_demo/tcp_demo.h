//https://github.com/will4906/Stm32TcpServer/tree/590107b2052d5ea7ecc08d46bf48dc82020a2b4c/User/drivers/tcp

#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"

#define CHECK_FOR_CONNECTION_PERIOD		20000					//

extern struct tcp_pcb *tcp_active_pcbs;//todo:？进一步检查这个结构的用途

void InitTcpServer(void);
err_t SendTcpDataToClient(u8_t *buff, u16_t length);
void CheckForConnection(void);
void CloseTcp(struct tcp_pcb *pcb);

#endif
