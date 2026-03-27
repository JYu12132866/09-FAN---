#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H

#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//UDP 测试代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   

//网络Link状态机
#define NET_SM_LINK_DOWN	0x12	//硬件网络连接断开
#define NET_SM_LINK_UP		0x21	//硬件已连接，协议栈与应用层待初始化
#define NET_SM_LINK_ON		0x10	//网络连接正常，可正常收发UDP数据包
#define NET_SM_IP_CHG	    0x14	//网络连接状态改变 


#define UDP_DEMO_RX_BUFSIZE		2000	//定义udp最大接收数据长度 
extern int recv_data_len;
// extern uint8_t udp_flag;
extern uint8_t udpRcvFlag;
extern uint8_t udpLinkFlag;
extern char udp_recvbuf[UDP_DEMO_RX_BUFSIZE];
extern uint8_t LINK_UP;
extern struct udp_pcb *udppcb; 
 

extern struct ip_addr dst_udpIP;        //UDP接收远端IP记录
extern u16_t dst_udpport;               //UDP接收远端PORT记录
extern struct ip_addr rmtipaddr;        //主动信息上报目标IP
extern u16_t rmtport;                   //主动信息上报目标port


void udp_demo_init(void);
void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
// void udp_demo_senddata(struct udp_pcb *upcb);
// void udp_demo_sendready(struct udp_pcb *upcb);
// void udp_send_data(uint8_t* pBuf, int data_len);
void udp_send_data(struct udp_pcb *upcb, struct ip_addr* targetIp,  u16_t targetPort, u8_t* pBuf, int dataLen);
void udp_send_remote(u8_t* pbuf, int dataLen);
void udp_demo_connection_close(struct udp_pcb *upcb);

#endif

