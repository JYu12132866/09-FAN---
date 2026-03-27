#include "udp_demo.h"
#include "lwip/ip_addr.h"
#include "stm32f4xx_hal.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#include "ezxml.h"
#include "main.h"
#include "usart.h"

char udp_recvbuf[UDP_DEMO_RX_BUFSIZE]; 
int recv_data_len = 0;
extern char status_buf[XML_LEN];
extern char status_buf1[XML_LEN];
extern char UDP_Report[200];
extern char Report[40];

uint8_t LINK_UP = 0x12;
extern struct netif lwip_netif;
struct udp_pcb *udppcb;


uint8_t udpRcvFlag = 0;
uint8_t udpLinkFlag = 0 ;
struct ip_addr rmtipaddr;
u16_t rmtport;
struct ip_addr dst_udpIP;
u16_t dst_udpport;


void udp_demo_init(void)
{
	err_t err = ERR_OK;

	udppcb = udp_new();

	if (udppcb) 
	{
		IP4_ADDR(&rmtipaddr, lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3]);
		rmtport = lwipdev.UDP_Port[0] * 100 + lwipdev.UDP_Port[1];
		//	printf("udp_new. IP4_ADDR..\r\n");
		//	err=udp_connect(udppcb,&rmtipaddr,UDP_DEMO_PORT);
		if (err == ERR_OK)
		{
			err = udp_bind(udppcb, IP_ADDR_ANY, UDP_DEMO_PORT); 
			if (err == ERR_OK)									
			{
				printf("UDP:Connected\r\n");
				LINK_UP = 0x10;
				udp_recv(udppcb, udp_demo_recv, NULL); 
				udpLinkFlag = 1;					   
			}
		}
	}
}


void udp_demo_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	uint32_t data_len = 0;
	struct pbuf *q;
	if (p != NULL)
	{
		memset(udp_recvbuf, 0, UDP_DEMO_RX_BUFSIZE);
		for (q = p; q != NULL; q = q->next)			 
		{

			if (q->len > (UDP_DEMO_RX_BUFSIZE - data_len))
				memcpy(udp_recvbuf + data_len, q->payload, (UDP_DEMO_RX_BUFSIZE - data_len));
			else
				memcpy(udp_recvbuf + data_len, q->payload, q->len);
			data_len += q->len;
			recv_data_len = data_len;
			if (data_len > UDP_DEMO_RX_BUFSIZE)
				break; 
		}
		IP4_ADDR(&dst_udpIP, (u8_t)(addr->addr), (u8_t)(addr->addr >> 8), (u8_t)(addr->addr >> 16), (u8_t)(addr->addr >> 24));
		dst_udpport = port;
		udpRcvFlag =  1 ;
		pbuf_free(p);		
	}
}


void udp_send_data(struct udp_pcb *upcb, struct ip_addr* targetIp,  u16_t targetPort, u8_t* pBuf, int dataLen)
{
	struct pbuf *ptr;
	ptr = pbuf_alloc(PBUF_TRANSPORT, dataLen, PBUF_POOL); 
	if (ptr)
	{
		pbuf_take(ptr, pBuf, dataLen); 
		udp_sendto(upcb, ptr, targetIp, targetPort);
		pbuf_free(ptr); 
	}
	// printf("target IP=%d.%d.%d.%d\r\n", (u8_t)(targetIp->addr), (u8_t)(targetIp->addr >> 8), (u8_t)(targetIp->addr >> 16), (u8_t)(targetIp->addr >> 24));
	// printf("target port=%d\r\n", targetPort);
}

#if 0 
void udp_demo_sendready(struct udp_pcb *upcb)
{
	struct pbuf *ptr;

	//	ptr=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)tcp_demo_sendrdy),PBUF_POOL); //�����ڴ�
	if (ptr)
	{
		//	pbuf_take(ptr,(char*)tcp_demo_sendrdy,strlen((char*)tcp_demo_sendrdy)); //��tcp_demo_sendbuf�е����ݴ����pbuf�ṹ��
		udp_sendto(upcb, ptr, &rmtipaddr, rmtport);

		pbuf_free(ptr); // �ͷ��ڴ�
	}
}
#endif 


/*******************************************************
* @brief 
* 
* @param pbuf 
* @param dataLen 
*******************************************************/
void udp_send_remote(u8_t* pbuf, int dataLen)
{	
	udp_send_data(udppcb, &dst_udpIP, dst_udpport, pbuf, dataLen);
	// printf("Udp target ip: %d:%d\n",  dst_udpIP.addr, dst_udpport);
}
	


void udp_demo_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb);
	udp_remove(upcb);	   
	udpLinkFlag = 0; 
	//printf("udp_demo_connection_close...\r\n");
}

