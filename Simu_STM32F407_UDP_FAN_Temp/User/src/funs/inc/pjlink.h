#ifndef __PJLINK_H
#define __PJLINK_H

#include "main.h"
#include <stdint.h>
#include <stdio.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip_comm.h"

extern struct tcp_pcb *tcp_active_pcbs;

void PJLinkThread(void);
void initPJLink(void);
void closePJLink(void);

void PJLink_StatusNotify_InputStatus(uint8_t input);
void PJLink_StatusNotify_PowerStatus(uint8_t power_status);
void PJLink_StatusNotify_ErrorStatus(uint8_t fan_err, uint8_t lamp_err, uint8_t temp_err, uint8_t cover_err,
                                     uint8_t filter_err, uint8_t other_err);
void PJLink_StatusNotify_LinkupStatus(void);

#endif // __PJLINK_H
