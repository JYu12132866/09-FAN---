#include "lwip_comm.h"
#include "ethernetif.h"
#include "lan8720.h"
#include "lwip/api.h"
#include "lwip/autoip.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/ip_frag.h"
#include "lwip/lwip_sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/raw.h"
#include "lwip/snmp_msg.h"
#include "lwip/sockets.h"
#include "lwip/stats.h"
#include "lwip/tcp_impl.h"
#include "lwip/timers.h"
#include "lwip/udp.h"
#include "main.h"
#include "malloc.h"
#include "netif/etharp.h"
#include "pjlink.h"
#include "udp_demo.h"
#include <stdint.h>
#include <stdio.h>

#define ETH_LINK_FLAG 0x10

__lwip_dev lwipdev;      // lwip控制结构体
struct netif lwip_netif; // 定义一个全局的网络接口

extern uint32_t memp_get_memorysize(void); // 在memp.c里面定义
extern u8_t *memp_memory;                  // 在memp.c里面定义.
extern u8_t *ram_heap;                     // 在mem.c里面定义.
extern uint32_t EthStatus;

uint32_t TCPTimer = 0;   // TCP查询计时器
uint32_t ARPTimer = 0;   // ARP查询计时器
uint32_t lwip_localtime; // lwip本地时间计数器,单位:ms
extern char IP_UPDATE[20];

#if LWIP_DHCP
uint32_t DHCPfineTimer = 0;   // DHCP精细处理计时器
uint32_t DHCPcoarseTimer = 0; // DHCP粗糙处理计时器
#endif
extern uint8_t EthInitStatus;

// lwip中mem和memp的内存申请
// 返回值:0,成功;
//     其他,失败
uint8_t lwip_comm_mem_malloc(void)
{
    uint32_t mempsize;
    uint32_t ramheapsize;
    mempsize = memp_get_memorysize();         // 得到memp_memory数组大小
    memp_memory = mymalloc(SRAMIN, mempsize); // 为memp_memory申请内存
    ramheapsize = LWIP_MEM_ALIGN_SIZE(MEM_SIZE) + 2 * LWIP_MEM_ALIGN_SIZE(4 * 3) + MEM_ALIGNMENT; // 得到ram heap大小
    ram_heap = mymalloc(SRAMIN, ramheapsize); // 为ram_heap申请内存
    if (!memp_memory || !ram_heap)            // 有申请失败的
    {
        lwip_comm_mem_free();
        return 1;
    }
    return 0;
}
// lwip中mem和memp内存释放
void lwip_comm_mem_free(void)
{
    myfree(SRAMIN, memp_memory);
    myfree(SRAMIN, ram_heap);
}
// lwip 默认IP设置
// lwipx:lwip控制结构体指针
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
    uint32_t sn0, sn1;
    sn0 = *(uint32_t *)(0x1FFF7A10); // 获取STM32的唯一ID的前24位作为MAC地址后三字节
    sn1 = *(uint32_t *)(0x1FFF7A14); // 获取STM32的唯一ID的前24位作为MAC地址后三字节
    // MAC地址设置(高三字节固定为:2.0.0,低三字节用STM32唯一ID)
    lwipx->mac[0] = 2; // 高三字节(IEEE称之为组织唯一ID,OUI)地址固定为:2.0.0
    lwipx->mac[1] = 0;
    lwipx->mac[2] = 0;
    lwipx->mac[3] = (sn0 >> 16) & 0XFF; // 低三字节用STM32的唯一ID
    lwipx->mac[4] = (sn1 >> 8) & 0XFF;
    lwipx->mac[5] = sn0 & 0XFF;

    lwipx->ip[0] = Ethernet_Info[0];
    lwipx->ip[1] = Ethernet_Info[1];
    lwipx->ip[2] = Ethernet_Info[2];
    lwipx->ip[3] = Ethernet_Info[3];

    lwipx->netmask[0] = Ethernet_Info[4];
    lwipx->netmask[1] = Ethernet_Info[5];
    lwipx->netmask[2] = Ethernet_Info[6];
    lwipx->netmask[3] = Ethernet_Info[7];

    lwipx->gateway[0] = Ethernet_Info[8];
    lwipx->gateway[1] = Ethernet_Info[9];
    lwipx->gateway[2] = Ethernet_Info[10];
    lwipx->gateway[3] = Ethernet_Info[11];
    lwipx->dhcpstatus = 0; // 没有DHCP

    lwipx->remoteip[0] = Ethernet_Info[12];
    lwipx->remoteip[1] = Ethernet_Info[13];
    lwipx->remoteip[2] = Ethernet_Info[14];
    lwipx->remoteip[3] = Ethernet_Info[15];

    lwipx->UDP_Port[0] = Ethernet_Info[16];
    lwipx->UDP_Port[1] = Ethernet_Info[17];

    sprintf(IP_UPDATE, "%d.%d.%d.%d", Ethernet_Info[0], Ethernet_Info[1], Ethernet_Info[2], Ethernet_Info[3]);
}

// LWIP初始化(LWIP启动的时候使用)
// 返回值:0,成功
//       1,内存错误
//       2,LAN8720初始化失败
//       3,网卡添加失败.

uint8_t lwip_comm_init(void)
{
    struct netif *netifInit; // 调用netif_add()函数时的返回值,用于判断网络初始化是否成功
    struct ip_addr ipaddr;   // ip地址
    struct ip_addr netmask;  // 子网掩码
    struct ip_addr gw;       // 默认网关
    if (ETH_Mem_Malloc())
        return 1; // 内存申请失败
    if (lwip_comm_mem_malloc())
        return 1; // 内存申请失败

    //   if(LAN8720_Init())return 2;			    //初始化LAN8720
    LAN8720_Init();                     // PHY配置错误也继续执行初始化Lwip
    lwip_init();                        // 初始化LWIP内核
    lwip_comm_default_ip_set(&lwipdev); // 设置默认IP等信息

#if LWIP_DHCP // 使用动态IP
    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;
#else // 使用静态IP
    IP4_ADDR(&ipaddr, lwipdev.ip[0], lwipdev.ip[1], lwipdev.ip[2], lwipdev.ip[3]);
    IP4_ADDR(&netmask, lwipdev.netmask[0], lwipdev.netmask[1], lwipdev.netmask[2], lwipdev.netmask[3]);
    IP4_ADDR(&gw, lwipdev.gateway[0], lwipdev.gateway[1], lwipdev.gateway[2], lwipdev.gateway[3]);
    printf("MAC=%02x.%02x.%02x.%02x.%02x.%02x\r\n", lwipdev.mac[0], lwipdev.mac[1], lwipdev.mac[2], lwipdev.mac[3], lwipdev.mac[4],
           lwipdev.mac[5]);
    printf("IP=%d.%d.%d.%d\r\n", lwipdev.ip[0], lwipdev.ip[1], lwipdev.ip[2], lwipdev.ip[3]);
    printf("NetMask=%d.%d.%d.%d\r\n", lwipdev.netmask[0], lwipdev.netmask[1], lwipdev.netmask[2], lwipdev.netmask[3]);
    printf("GateWay=%d.%d.%d.%d\r\n", lwipdev.gateway[0], lwipdev.gateway[1], lwipdev.gateway[2], lwipdev.gateway[3]);
    printf("RemoteIP=%d.%d.%d.%d\r\n", lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2],
           lwipdev.remoteip[3]);
    printf("UDPPort=%d\r\n", (lwipdev.UDP_Port[0] * 100 + lwipdev.UDP_Port[1]));

#endif
    netifInit = netif_add(&lwip_netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init,
                          &ethernet_input); // 向网卡列表中添加一个网口

#if LWIP_DHCP                // 如果使用DHCP的话
    lwipdev.dhcpstatus = 0;  // DHCP标记为0
    dhcp_start(&lwip_netif); // 开启DHCP服务
#endif

    if (netifInit == NULL)
        return 3; // 网卡添加失败
    else          // 网口添加成功后,设置netif为默认值,并且打开netif网口
    {
        netif_set_default(&lwip_netif); // 设置netif
        // printf("EthStatus=%d\r\n",EthStatus);
        if (EthStatus == (ETH_LINK_FLAG))
        {
            /* Set Ethernet link flag */
            LINK_UP = 0x31;
            netif_set_up(&lwip_netif); // 打开netif网
            lwip_netif.flags |= NETIF_FLAG_LINK_UP;
            printf("netif_set_up\r\n");
        }
        else
        {
            LINK_UP = 0x12;
            ETH_Stop();
            netif_set_down(&lwip_netif);
            printf("netif_set_failed\r\n");
            return 4; // 网
        }
        /* Set the link callback function, this function is called on change of link status*/
        netif_set_link_callback(&lwip_netif, ETH_link_callback);
    }
    return 0; // 操作OK.
}

// void Eth_Link_ITHandler_test(uint16_t PHYAddress)
// {
//     // uint16_t status;
//     // struct ip_addr ipaddr, netmask, gw;
//     if ((ETH_ReadPHYRegister(PHYAddress, PHY_BSR) & PHY_Linked_Status) != 0x00)
//     {
//         LAN8720_Init();
//     }
// }

void Eth_Link_ITHandler(uint16_t PHYAddress)
{
    uint16_t status;
    struct ip_addr ipaddr, netmask, gw;

    // NOTE: APP Code不再上电做lan8720和stm32
    // eth模块的初始化，由bootloader完成；lan8720初始化完成后此处才可以检查到link状态，根据link状态进行IP初始化和UDP初始化；
    // NOTE:
    // 若APPCode需要单独改造成为独立运行APP，则需要在主函数添加lan8720的初始化代码，否则此条件语句永远无法进入，网络无法初始化；
    if ((ETH_ReadPHYRegister(PHYAddress, PHY_BSR) & PHY_Linked_Status) != 0x00)
    {
        if (EthInitStatus != ETH_SUCCESS) // lwip未初始化
        {
            lwip_comm_init();
#ifdef USING_PJLINK
            initPJLink(); // 初始化PJLINK TCP  UDP  Server
#endif
        }
        else // lwip已初始化的情况
        {
            status = ETH_ReadPHYRegister(PHYAddress, PHY_BSR);
            if (status & (PHY_AutoNego_Complete | PHY_Linked_Status))
            {
                /*set link up for re link callbalk function*/
                if (LINK_UP == 0x10)
                    return;
                else
                {
                    printf("Link UP\r\n");
                    if (LINK_UP == 0x14)
                    {
                        lwip_comm_default_ip_set(&lwipdev);
                        IP4_ADDR(&ipaddr, lwipdev.ip[0], lwipdev.ip[1], lwipdev.ip[2], lwipdev.ip[3]);
                        IP4_ADDR(&netmask, lwipdev.netmask[0], lwipdev.netmask[1], lwipdev.netmask[2],
                                 lwipdev.netmask[3]);
                        IP4_ADDR(&gw, lwipdev.gateway[0], lwipdev.gateway[1], lwipdev.gateway[2], lwipdev.gateway[3]);
                        netif_set_addr(&lwip_netif, &ipaddr, &netmask, &gw);
                        netif_set_default(&lwip_netif);
                    }

                    LINK_UP = 0x21;
                    netif_set_up(&lwip_netif); // 打开netif网//
                    netif_set_link_up(&lwip_netif);
                    udp_demo_init(); // 初始化UDP 8080端口
#ifdef USING_PJLINK
                    initPJLink(); // 初始化PJLINK TCP  UDP  Server
#endif
                }
            }
            else
            {
                if (LINK_UP == 0x10)
                    printf("Link Down\r\n");
                LINK_UP = 0x12;
                netif_set_link_down(&lwip_netif);
                udp_demo_connection_close(udppcb); // 关闭UDP 8080端口
#ifdef USING_PJLINK
                closePJLink(); // 关闭PJLINK TCP UDP Server
#endif
                netif_set_down(&lwip_netif); //
                ETH_Mem_Free();
                lwip_comm_mem_free();
            }
        }
    }
    else if (LINK_UP == 0x10)
    {
        printf("Link Down\r\n");
        LINK_UP = 0x12;
        netif_set_link_down(&lwip_netif);
        udp_demo_connection_close(udppcb); // 关闭 UDP 8080端口
#ifdef USING_PJLINK
        closePJLink(); // 关闭PJLINK TCP UDP Server
#endif
        netif_set_down(&lwip_netif); //
        ETH_Mem_Free();
        lwip_comm_mem_free();
    }
}

// 当接收到数据后调用
void lwip_pkt_handle(void)
{
    // 从网络缓冲区中读取接收到的数据包并将其发送给LWIP处理
    ethernetif_input(&lwip_netif);
}

// LWIP轮询任务
void lwip_periodic_handle()
{
#if LWIP_TCP
    // 每250ms调用一次tcp_tmr()函数
    if (lwip_localtime - TCPTimer >= TCP_TMR_INTERVAL)
    {
        TCPTimer = lwip_localtime;
        tcp_tmr();
    }
#endif
    // ARP每5s周期性调用一次
    if ((lwip_localtime - ARPTimer) >= ARP_TMR_INTERVAL)
    {
        ARPTimer = lwip_localtime;
        etharp_tmr();
    }

#if LWIP_DHCP // 如果使用DHCP的话
    // 每500ms调用一次dhcp_fine_tmr()
    if (lwip_localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
    {
        DHCPfineTimer = lwip_localtime;
        dhcp_fine_tmr();
        if ((lwipdev.dhcpstatus != 2) && (lwipdev.dhcpstatus != 0XFF))
        {
            lwip_dhcp_process_handle(); // DHCP处理
        }
    }

    // 每60s执行一次DHCP粗糙处理
    if (lwip_localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
    {
        DHCPcoarseTimer = lwip_localtime;
        dhcp_coarse_tmr();
    }
#endif
}

void LwIP_Periodic_Handle(uint32_t localtime)
{
#if LWIP_TCP
    /* TCP periodic process every 250 ms */
    if (localtime - TCPTimer >= TCP_TMR_INTERVAL)
    {
        TCPTimer = localtime;
        tcp_tmr();
    }
#endif

    /* ARP periodic process every 5s */
    if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)
    {
        ARPTimer = localtime;
        etharp_tmr();
    }

#ifdef USE_DHCP
    /* Fine DHCP periodic process every 500ms */
    if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
    {
        DHCPfineTimer = localtime;
        dhcp_fine_tmr();
        if ((DHCP_state != DHCP_ADDRESS_ASSIGNED) && (DHCP_state != DHCP_TIMEOUT) && (DHCP_state != DHCP_LINK_DOWN))
        {
            /* toggle LED1 to indicate DHCP on-going process */
            STM_EVAL_LEDToggle(LED1);

            /* process DHCP state machine */
            LwIP_DHCP_Process_Handle();
        }
    }

    /* DHCP Coarse periodic process every 60s */
    if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
    {
        DHCPcoarseTimer = localtime;
        dhcp_coarse_tmr();
    }

#endif
}

// 如果使能了DHCP
#if LWIP_DHCP

// DHCP处理任务
void lwip_dhcp_process_handle(void)
{
    uint32_t ip = 0, netmask = 0, gw = 0;
    switch (lwipdev.dhcpstatus)
    {
    case 0: // 开启DHCP
        dhcp_start(&lwip_netif);
        lwipdev.dhcpstatus = 1; // 等待通过DHCP获取到的地址
        printf("正在查找DHCP服务器,请稍等...........\r\n");
        break;
    case 1: // 等待获取到IP地址
    {
        ip = lwip_netif.ip_addr.addr;      // 读取新IP地址
        netmask = lwip_netif.netmask.addr; // 读取子网掩码
        gw = lwip_netif.gw.addr;           // 读取默认网关

        if (ip != 0) // 正确获取到IP地址的时候
        {
            lwipdev.dhcpstatus = 2; // DHCP成功
            printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n", lwipdev.mac[0], lwipdev.mac[1],
                   lwipdev.mac[2], lwipdev.mac[3], lwipdev.mac[4], lwipdev.mac[5]);
            // 解析出通过DHCP获取到的IP地址
            lwipdev.ip[3] = (uint8_t)(ip >> 24);
            lwipdev.ip[2] = (uint8_t)(ip >> 16);
            lwipdev.ip[1] = (uint8_t)(ip >> 8);
            lwipdev.ip[0] = (uint8_t)(ip);
            printf("通过DHCP获取到IP地址..............%d.%d.%d.%d\r\n", lwipdev.ip[0], lwipdev.ip[1], lwipdev.ip[2],
                   lwipdev.ip[3]);
            // 解析通过DHCP获取到的子网掩码地址
            lwipdev.netmask[3] = (uint8_t)(netmask >> 24);
            lwipdev.netmask[2] = (uint8_t)(netmask >> 16);
            lwipdev.netmask[1] = (uint8_t)(netmask >> 8);
            lwipdev.netmask[0] = (uint8_t)(netmask);
            printf("通过DHCP获取到子网掩码............%d.%d.%d.%d\r\n", lwipdev.netmask[0], lwipdev.netmask[1],
                   lwipdev.netmask[2], lwipdev.netmask[3]);
            // 解析出通过DHCP获取到的默认网关
            lwipdev.gateway[3] = (uint8_t)(gw >> 24);
            lwipdev.gateway[2] = (uint8_t)(gw >> 16);
            lwipdev.gateway[1] = (uint8_t)(gw >> 8);
            lwipdev.gateway[0] = (uint8_t)(gw);
            printf("通过DHCP获取到的默认网关..........%d.%d.%d.%d\r\n", lwipdev.gateway[0], lwipdev.gateway[1],
                   lwipdev.gateway[2], lwipdev.gateway[3]);
        }
        else if (lwip_netif.dhcp->tries > LWIP_MAX_DHCP_TRIES) // 通过DHCP服务获取IP地址失败,且超过最大尝试次数
        {
            lwipdev.dhcpstatus = 0XFF; // DHCP超时失败.
            // 使用静态IP地址
            IP4_ADDR(&(lwip_netif.ip_addr), lwipdev.ip[0], lwipdev.ip[1], lwipdev.ip[2], lwipdev.ip[3]);
            IP4_ADDR(&(lwip_netif.netmask), lwipdev.netmask[0], lwipdev.netmask[1], lwipdev.netmask[2],
                     lwipdev.netmask[3]);
            IP4_ADDR(&(lwip_netif.gw), lwipdev.gateway[0], lwipdev.gateway[1], lwipdev.gateway[2], lwipdev.gateway[3]);
            printf("DHCP服务超时,使用静态IP地址!\r\n");
            printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n", lwipdev.mac[0], lwipdev.mac[1],lwipdev.mac[2], lwipdev.mac[3], lwipdev.mac[4], lwipdev.mac[5]);
            printf("静态IP地址........................%d.%d.%d.%d\r\n", lwipdev.ip[0], lwipdev.ip[1], lwipdev.ip[2],lwipdev.ip[3]);
            printf("子网掩码..........................%d.%d.%d.%d\r\n", lwipdev.netmask[0], lwipdev.netmask[1],lwipdev.netmask[2], lwipdev.netmask[3]);
            printf("默认网关..........................%d.%d.%d.%d\r\n", lwipdev.gateway[0], lwipdev.gateway[1],lwipdev.gateway[2], lwipdev.gateway[3]);
        }
    }
    break;
    default:
        break;
    }
}
#endif
