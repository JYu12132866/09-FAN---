#ifndef __LAN8720_H
#define __LAN8720_H
#include "main.h"
#include "stm32f4x7_eth.h"
#include "lwip/netif.h"		


#define LAN8720_PHY_ADDRESS  	0x00				//LAN8720 PHY地址
#define LAN8720_RST() 		   	(HAL_GPIO_WritePin(ETH_PHY_RST_GPIO_Port, ETH_PHY_RST_Pin, GPIO_PIN_RESET)) 
#define LAN8720_SET() 		   	(HAL_GPIO_WritePin(ETH_PHY_RST_GPIO_Port, ETH_PHY_RST_Pin, GPIO_PIN_SET)) 
	 
#define ETH_INIT_FLAG           0x01 /* Ethernet Init Flag */
#define ETH_LINK_FLAG           0x10 /* Ethernet Link Flag */


// extern ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB];/* Ethernet Rx MA Descriptor */
// extern ETH_DMADESCTypeDef  DMATxDscrTab[ETH_TXBUFNB];/* Ethernet Tx DMA Descriptor */
// extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; /* Ethernet Receive Buffer */
// extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; 

extern ETH_DMADESCTypeDef *DMARxDscrTab;			/* Ethernet Rx MA Descriptor */
extern ETH_DMADESCTypeDef *DMATxDscrTab;			/* Ethernet Tx DMA Descriptor */
extern uint8_t *Rx_Buff; 							/* Ethernet Receive Buffer */
extern uint8_t *Tx_Buff; 							/* Ethernet Transmit Buffer */
extern ETH_DMADESCTypeDef  *DMATxDescToSet;			//DMA发送描述符追踪指针
extern ETH_DMADESCTypeDef  *DMARxDescToGet; 		//DMA接收描述符追踪指针 
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;	//DMA最后接收到的帧信息指针
 
void ETH_IT_ENABLE(void);
void ETH_IT_DISABLE(void);

uint8_t LAN8720_Init(void);
uint8_t LAN8720_Get_Speed(void);
uint8_t ETH_MACDMA_Config(void);
FrameTypeDef ETH_Rx_Packet(void);
uint8_t ETH_Tx_Packet(uint16_t FrameLength);
uint32_t ETH_GetCurrentTxBuffer(void);
uint8_t ETH_Mem_Malloc(void);
void ETH_Mem_Free(void);
void ETH_link_callback(struct netif *netif);
void ETH_Broadcast_Set(int flag);

#endif 

