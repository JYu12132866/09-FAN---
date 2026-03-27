#include "pjlink.h"
#include "error.h"
#include "logger.h"
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "md5.h"
#include "pjlink.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include <string.h>
#include "rt_param.h"

/* clang-format off */
#define PJLINK_PORT             4352    //PJLINK默认端口号
#define PJ_CMD_BUF_SIZE         256     //PJLINK命令缓冲区大小      
#define PJLINK_TCP_LINK_TIMEOUT 30000   //PJLINK TCP连接超时时间30s

/* 通知协议默认IP地址 */
 #define PJ_NOTIFY_IP_ADDR0                    192
 #define PJ_NOTIFY_IP_ADDR1                    168
 #define PJ_NOTIFY_IP_ADDR2                      0
 #define PJ_NOTIFY_IP_ADDR3                    100

/* PJLink 命令格式特殊字符定义 */
#define PJ_CMD_CLASS1           "%1"   //Class1命令
#define PJ_CMD_CLASS2           "%2"   //Class2命令
#define PJ_CMD_TERMINATOR       '\r'    //CR 0x0D
#define PJ_CMD_SP               ' '     //SP 0x20
#define PJ_CMD_QUERY            '?'     //0x3F 
#define PJ_CMD_ASSIGN           '='     //0x3D

/* PJLink 命令列表 */
#define PJ_CMD_POWR            "POWR"   //电源控制、查询指令
#define  PJ_CMD_INPT            "INPT"   //输入源切换、查询指令
#define PJ_CMD_AVMT            "AVMT"   //音频静音控制、查询指令
#define PJ_CMD_ERST            "ERST"   //错误状态查询指令
#define PJ_CMD_LAMP            "LAMP"   //灯泡状态查询指令
#define PJ_CMD_INST            "INST"   //输入切换列表查询指令
#define PJ_CMD_NAME            "NAME"   //设备名称查询指令
#define PJ_CMD_INF1            "INF1"   //设备厂商查询指令
#define PJ_CMD_INF2            "INF2"   //产品名称查询指令
#define PJ_CMD_INFO            "INFO"   //其它信息查询指令
#define PJ_CMD_CLSS            "CLSS"   //协议类型查询指令
#define PJ_CMD_SNUM            "SNUM"   //序列号查询指令
#define PJ_CMD_SVER            "SVER"   //软件版本查询指令
#define PJ_CMD_INNM            "INNM"   //输入终端名称查询指令
#define PJ_CMD_IRES            "IRES"   //输入分辨率查询指令
#define PJ_CMD_RRES            "RRES"   //推荐分辨率查询指令
#define PJ_CMD_FILT            "FILT"   //滤芯状态查询指令
#define PJ_CMD_RLMP            "RLMP"   //灯泡替换型号查询指令
#define PJ_CMD_RFLT            "RFLT"   //滤芯替换型号查询指令
#define PJ_CMD_SVOL            "SVOL"   //音量控制、查询指令
#define PJ_CMD_MVOL            "MVOL"   //麦克风音量控制、查询指令
#define PJ_CMD_FREZ            "FREZ"   //冻结控制、查询指令
#define PJ_CMD_AUTH            "PJLINK" //认证控制指令

/* PJLink 响应状态定义 */
#define PJ_RET_OK                       0X0         //正常响应
#define PJ_RET_UNDEFINED_CMD            0X01        //未定义指令
#define PJ_RET_OUT_OF_RANGE             0X02        //超出支持范围
#define PJ_RET_UNAVAIABLE_TIME          0X03        //不可用时间
#define PJ_RET_PROJECTOR_FAILURE        0X04        //投影机故障
// #define PJ_RET_STR_OK                   "OK"     
// #define PJ_RET_STR_UNDEFINED_CMD        "ERR1"
// #define PJ_RET_STR_OUT_OF_RANGE         "ERR2"
// #define PJ_RET_STR_UNAVAIABLE_TIME      "ERR3"
// #define PJ_RET_STR_PROJECTOR_FAILURE    "ERR4"
#define PJ_RET_STR_AUTH_FAIL            "ERRA"      //认证失败

// 错误字符串映射表
char errCodeStr[][5] = {
    {"OK"},
    {"ERR1"},
    {"ERR2"},
    {"ERR3"},
    {"ERR4"},
    {"ERRA"},
};

/* PJLink 命令格式定义 */
#define PJ_CLASS2_CMD_SEARCH            "%2SRCH\r"                                          //搜索协议：命令
#define PJ_CLASS2_CMD_SEARCH_ACK_FMT    "%%2ACKN=%02x:%02x:%02x:%02x:%02x:%02x\r"           //搜索协议：应答
#define PJ_CLASS2_SNC_LINKUP_STS_FMT    "%%2LKUP=%02x:%02x:%02x:%02x:%02x:%02x\r"           //状态通知协议：Linkup状态
#define PJ_CLASS2_SNC_ERR_STS_FMT       "%%2ERST=%1d%1d%1d%1d%1d%1d\r"                      //状态通知协议：错误状态
#define PJ_CLASS2_SNC_PWR_STS_FMT       "%%2POWR=%1d\r"                                     //状态通知协议：电源状态
#define PJ_CLASS2_SNC_INPT_STS_FMT      "%%2INPT=%c%c\r"                                    //状态通知协议：输入状态   

/* PJLink 错误查询状态定义 */
#define STS_NO_ERR                 0x00     //无错误
#define STS_WARNING                0x01     //警告
#define STS_ERR                    0x02     //错误

/* PJLink 电源状态查询定义 */
#define PWR_STS_OFF                0x00     // 关机，待机
#define PWR_STS_ON                 0x01     // 开机，亮灯
#define PWR_STS_COOLING            0x02     // 冷却中
#define PWR_STS_WARMING            0x03     // 启动中

/* PJLink 输入通道编号第一位定义 */
#define INPUT_1st_RGB_            '1'   //RGB
#define INPUT_1st_VIDEO           '2'   //VIDEO
#define INPUT_1st_DIGITAL         '3'   //DIGITAL
#define INPUT_1st_STORAGE         '4'   //STORAGE
#define INPUT_1st_NETWORK         '5'   //NETWORK
#define INPUT_1st_INTERNAL        '6'   //INTERNAL


/**
 * @brief 设备通道编号与PJLink通道编号映射结构
 * 
 */
typedef struct 
{
    uint8_t device_input_chnl_;     //设备支持的输入通道
    char pjlink_input_chnl_[2];     //PJLink输入通道编号
    char input_chnl_name_[8];       //输入通道名称 
} InputChnlMap;


/* PJLink 输入源通道范围定义 */
//NOTE: 由于Class1 和Class2 范围不同，为了兼容，在定义时候，使用Class1的范围 '1' ~ '9', 非必要不要超过这个范围
InputChnlMap pj_input_chnl_map[] = {
    {VIDEO_INPUT_SRC_VGA,       {'1','1'}, "VGA"    },
    {VIDEO_INPUT_SRC_DVI,       {'3','1'}, "DVI-D"  },
    {VIDEO_INPUT_SRC_HDMI_1,    {'3','2'}, "HDMI-1" },
#if (Projector_Model == LP92SLB)
    {VIDEO_INPUT_SRC_HDMI_2,    {'3','3'}, "HDMI-2" },
#endif
    {VIDEO_INPUT_SRC_DP_1,        {'3','4'}, "DP-1"     }, 
    {VIDEO_INPUT_SRC_DP_2,        {'3','5'}, "DP-2"     }, 
    {VIDEO_INPUT_SRC_HD_BASE,     {'3','6'}, "HD-BASE"  }, 
};

/* clang-format on */

// PJLink 认证器--------------------------------------------------------------

typedef struct
{
    uint8_t authSwitch;     // 认证开关
    uint8_t authPass;       // 认证通过标志
    int randomNumers;       // 随机数
    char password[32];      // 密码
    char EncryptedMsg[32];  // 加密后回复的消息
    uint32_t tcp_last_time; // TCP最后一次通信时间
} PJLink_Authenticator;
PJLink_Authenticator pjlink_authenticator = {
    .authSwitch = ON,
    .authPass = FALSE,
    .randomNumers = 0x498e4a67,
    .password = "GuohuaProjectorLink",
    .EncryptedMsg = "",
}; // PJLink认证器初始化

typedef struct
{
    uint8_t statusNotifyEn;          // 状态通知协议开关
    struct ip_addr pj_controller_ip; // PJLINK控制器IP地址
} PJLink_Manager;
PJLink_Manager pjlink_manager = {
    .statusNotifyEn = OFF,
    .pj_controller_ip = ((u32_t)((PJ_NOTIFY_IP_ADDR0) & 0xff) << 24) | ((u32_t)((PJ_NOTIFY_IP_ADDR1) & 0xff) << 16) |
                        ((u32_t)((PJ_NOTIFY_IP_ADDR2) & 0xff) << 8) | (u32_t)((PJ_NOTIFY_IP_ADDR3) & 0xff)
    // PJLINK控制器IP地址默认192.168.0.100, 可以通过搜索协议或TCP连接更新
};

/**
 * @brief 初始化PJLink认证器，生成加密后的消息
 *
 */
void initPJLinkAuthenticator(void)
{
    char msg[32];
    sprintf(msg, "%x%s", pjlink_authenticator.randomNumers, pjlink_authenticator.password);
    Compute_data_md5((unsigned char *)msg, strlen(msg), (unsigned char *)pjlink_authenticator.EncryptedMsg);
    log_info("EncryptedMsg: %s", pjlink_authenticator.EncryptedMsg);
}

// PJLink TCP Server - PJLink TCP服务器---------------------------------------

uint8_t PJ_TCP_data_len;
struct tcp_pcb *pj_tcp_server_pcb;
struct tcp_pcb *pj_tcp_client_pcb;
static u8_t pj_connectFlag = 0;
u16_t PJ_TCP_Server_PORT = PJLINK_PORT;
char pj_tcp_rx_buf[PJ_CMD_BUF_SIZE];
char pj_tcp_tx_buf[PJ_CMD_BUF_SIZE];
int pj_tcp_tx_msg_len = 0;
uint8_t pj_tcp_rx_flag = FALSE;

// 前置声明
void initPJLinkTcpServer(void);
err_t SendPJLinkTcpDataToClient(u8_t *buff, u16_t length);
void CheckForPJLinkConnection(void);
void ClosePJLinkTcp(struct tcp_pcb *pcb);
static err_t OnPJLinkTcpServerAccept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t OnPJLinkTcpServerRecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);

/**
 * @brief 更新TCP连接时间
 *
 */
inline void refresh_tcp_link_time(void)
{
    pjlink_authenticator.tcp_last_time = HAL_GetTick();
}

/**
 * @brief 检测TCP连接时间是否超时
 *
 */
inline void check_tcp_link_time(void)
{
    if (HAL_GetTick() - pjlink_authenticator.tcp_last_time > PJLINK_TCP_LINK_TIMEOUT)
    {
        ClosePJLinkTcp(pj_tcp_client_pcb); // 关闭连接
    }
}

/*-------------------------------------------------*/
/*函数名：TCP服务器的初始化                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void initPJLinkTcpServer(void)
{
    pj_tcp_server_pcb = tcp_new();                                // 新建一个TCP控制块
    tcp_bind(pj_tcp_server_pcb, IP_ADDR_ANY, PJ_TCP_Server_PORT); // 绑定IP和端口号
    pj_tcp_server_pcb = tcp_listen(pj_tcp_server_pcb);            // 服务器进入监听
    tcp_accept(pj_tcp_server_pcb, OnPJLinkTcpServerAccept);       // 设置连接接入回调函数
    log_info("PJlink TCP server created, listen port: %d", PJLINK_PORT);
}

/*-------------------------------------------------*/
/*函数名：连接接入回调函数                         */
/*参  数：arg：传入参数                            */
/*参  数：pcb：TCP控制块                           */
/*参  数：err：错误值                              */
/*返回值：无                                       */
/*-------------------------------------------------*/
static err_t OnPJLinkTcpServerAccept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    pj_connectFlag = 1;
    pj_tcp_client_pcb = pcb;
    tcp_setprio(pcb, TCP_PRIO_MIN); // 设置回调函数优先级，当存在几个连接时特别重要,此函数必须调用*/
    tcp_recv(pcb, OnPJLinkTcpServerRecv); // 设置接收函数回调函数
    log_info("PJlink TCP server accept");

    pjlink_manager.pj_controller_ip = pcb->remote_ip;   //更新PJLINK控制器IP地址

    refresh_tcp_link_time();
    if (pjlink_authenticator.authSwitch == OFF) // 无需认证
    {
        char *noAuthMsg = "PJLINK 0\r";
        pjlink_authenticator.authPass = TRUE;                            // 认证通过标志
        SendPJLinkTcpDataToClient((u8_t *)noAuthMsg, strlen(noAuthMsg)); // 发送PJLINK 认证消息
    }
    else // 需要认证
    {
        char authMsg[64];
        sprintf(authMsg, "PJLINK 1 %x\r", pjlink_authenticator.randomNumers);
        SendPJLinkTcpDataToClient((u8_t *)authMsg, strlen(authMsg)); // 发送PJLINK 认证消息
    }

    return ERR_OK;
}

/*-------------------------------------------------*/
/*函数名：接收回调函数                             */
/*参  数：arg：传入参数                            */
/*参  数：pcb：TCP控制块                           */
/*参  数：p：数据缓冲区指针                        */
/*参  数：err：错误值                              */
/*返回值：无                                       */
/*-------------------------------------------------*/
static err_t OnPJLinkTcpServerRecv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    if (p != NULL) // p是数据缓冲区指针，不为空表示有数据到
    {
        tcp_recved(pcb, p->tot_len); // 接收数据
        memset(pj_tcp_rx_buf, 0, PJ_CMD_BUF_SIZE);
        memcpy(pj_tcp_rx_buf, p->payload, p->tot_len);
        PJ_TCP_data_len = p->tot_len;
        pbuf_free(p); // 释放缓冲区

        pj_tcp_rx_flag = TRUE;   // 设置TCP接收标志位
        refresh_tcp_link_time(); // 每次接收到TCP消息，则更新TCP连接时间
        log_info("PJLINK TCP RX: %s", pj_tcp_rx_buf);
    }
    return ERR_OK;
}

/**********************************************************
 * 函数功能：服务器发送消息给客户端的函数
 * 参数：		数据，数据的长度
 *	返回值：	是否发送成功
 ***********************************************************/
err_t SendPJLinkTcpDataToClient(u8_t *buff, u16_t length)
{
    err_t err;
    CheckForPJLinkConnection();
    if (pj_connectFlag == 1)
    {
        err = tcp_write(pj_tcp_client_pcb, buff, length, 1);
        log_info("TCP send data to client: %s", buff);
    }
    else
    {
        return ERR_CONN;
    }
    return err;
}

/******************************************************************
 *函数：  检查连接状态，若断开则重连
 *参数：  无
 *返回值：无
 *注：    此函数需以一定的时间调用，否则无法发挥作用
 *********************************************************************/
void CheckForPJLinkConnection(void)
{
    struct tcp_pcb *cpcb; // TCP控制块
    int connectFlag = 0;
    for (cpcb = tcp_active_pcbs; cpcb != NULL; cpcb = cpcb->next) // 轮询TCP活动列表
    {
        if ((cpcb->local_port == PJ_TCP_Server_PORT) &&
            (pj_tcp_client_pcb->state != CLOSED)) // 如果TCP_LOCAL_PORT端口指定的连接没有断开
        {
            connectFlag = 1; // 连接标志
            break;
        }
    }
    if (!connectFlag)
    {
        ClosePJLinkTcp(pj_tcp_client_pcb); // 关闭连接
    }
}

/*-------------------------------------------------*/
/*函数名：关闭连接函数                             */
/*参  数：pcb：TCP控制块                           */
/*返回值：无                                       */
/*-------------------------------------------------*/
void ClosePJLinkTcp(struct tcp_pcb *pcb)
{
    tcp_abort(pcb);         // 关闭连接
    tcp_arg(pcb, NULL);     // 移除回调函数
    tcp_recv(pcb, NULL);    // 移除回调函数
    tcp_sent(pcb, NULL);    // 移除回调函数
    tcp_err(pcb, NULL);     // 移除回调函数
    tcp_poll(pcb, NULL, 0); // 移除回调函数

    pj_connectFlag = 0;                    // TCP连接标志位
    pjlink_authenticator.authPass = FALSE; // 认证通过标志清除

    log_info("PJLINK TCP client closed");
}

// PJLink TCP Server - PJLink TCP服务器

// PJLink UDP Server - PJLink UDP服务器---------------------------------------

/* clang-format off */
struct udp_pcb *pj_udp_pcb;             // UDP控制块
char pj_udp_rx_buf[PJ_CMD_BUF_SIZE];    // UDP接收数据缓冲区
char pj_udp_tx_buf[PJ_CMD_BUF_SIZE];    // UDP发送数据缓冲区
int pj_udp_rx_len = 0;                  // UDP接收数据长度
uint8_t pj_udp_rx_flag = FALSE;         // UDP标志位

/* clang-format on */

// 前置声明
void pjlink_udp_rx_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
void PJLinkProcessUDPCmd(struct ip_addr *addr);
/**
 * @brief PJLINK UDP Server 初始化函数
 *
 */
void initPJLinkUdpServer(void)
{
    err_t err = ERR_OK;
    pj_udp_pcb = udp_new();

    if (pj_udp_pcb) // 创建成功
    {
        if (err == ERR_OK)
        {
            err = udp_bind(pj_udp_pcb, IP_ADDR_ANY, PJLINK_PORT); // 绑定本地IP地址与端口号
            if (err == ERR_OK)                                    // 绑定完成
            {
                log_info("PJlink UDP server created, port: %d", PJLINK_PORT);
                udp_recv(pj_udp_pcb, pjlink_udp_rx_callback, NULL);

                ip_set_option(pj_udp_pcb, SOF_BROADCAST);
            }
        }
    }
}

/**
 * @brief UDP接收回调函数
 *
 * @param arg
 * @param upcb
 * @param p
 * @param addr
 * @param port
 */
void pjlink_udp_rx_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
    uint32_t data_len = 0;
    struct pbuf *q;
    if (p != NULL)
    {
        memset(pj_udp_rx_buf, 0, PJ_CMD_BUF_SIZE); // 数据接收缓冲区清零
        for (q = p; q != NULL; q = q->next)        // 遍历完整个pbuf链表
        {

            if (q->len > (PJ_CMD_BUF_SIZE - data_len))
                memcpy(pj_udp_rx_buf + data_len, q->payload, (PJ_CMD_BUF_SIZE - data_len)); // 拷贝数据
            else
                memcpy(pj_udp_rx_buf + data_len, q->payload, q->len);
            data_len += q->len;
            pj_udp_rx_len = data_len;
            if (data_len > PJ_CMD_BUF_SIZE)
                break; // 超出TCP客户端接收数组,跳出
        }
        // IP4_ADDR(&pj_controller_ip, (u8_t)(addr->addr), (u8_t)(addr->addr >> 8), (u8_t)(addr->addr >> 16),
        //          (u8_t)(addr->addr >> 24));
        // pj_udp_rx_flag = TRUE; // 设置UDP标志位
        log_info("PJLINK UDP RX: %s", pj_udp_rx_buf);
        PJLinkProcessUDPCmd(addr); // 处理UDP命令
        pbuf_free(p);          // 释放内存
    }
}

/**
 * @brief PJLink UDP发送函数
 *
 * @param pBuf      发送数据缓冲区
 * @param data_len  发送数据长度
 */
void pjlink_udp_send(uint8_t *pBuf, int data_len)
{
    struct pbuf *ptr;

    ptr = pbuf_alloc(PBUF_TRANSPORT, data_len, PBUF_POOL); // 申请内存
    if (ptr)
    {
        pbuf_take(ptr, (void *)(pBuf), data_len);
        udp_sendto(pj_udp_pcb, ptr, &pjlink_manager.pj_controller_ip, PJLINK_PORT); // 发送数据
        pbuf_free(ptr);                                                             // 释放内存
    }
}

/**
 * @brief 关闭UDP服务器
 *
 */
void closePJLinkUDPServer(void)
{
    udp_disconnect(pj_udp_pcb); // 断开UDP连接
    udp_remove(pj_udp_pcb);     // 移除UDP控制块
    log_info("PJlink UDP server closed");
}

// PJLink 业务函数------------------------------------------------------------

#pragma pack(1)
// PJLink命令结构体抽象
typedef struct
{
    char class[2];
    char command[4];
    char separator;
    uint8_t data[128 + 1];
} PJ_CMD_STRUCT;
#pragma pack()

/**
 * @brief PJLINK网络服务器初始化函数
 *
 */
void initPJLink(void)
{
    initPJLinkAuthenticator();
    initPJLinkUdpServer();
    initPJLinkTcpServer();
    PJLink_StatusNotify_LinkupStatus();
}

/**
 * @brief PJLINK网络服务关闭函数
 *
 */
void closePJLink(void)
{
    closePJLinkUDPServer();
    if(pj_tcp_client_pcb!=NULL)
        ClosePJLinkTcp(pj_tcp_client_pcb);
    pjlink_authenticator.authPass = FALSE;
}

/**
 * @brief PJLink消息构建函数
 * 
 * @param msg 
 * @param command 
 * @param separator 
 * @param data 
 * @param data_len 
 */
void construct_message(char *msg, char class, char *command, char separator, char *data, int data_len)
{
    msg[0] = '%';
    msg[1] = class;
    msg[2] = command[0];
    msg[3] = command[1];
    msg[4] = command[2];
    msg[5] = command[3];
    msg[6] = separator;
    memcpy(&msg[7], data, data_len);
    msg[7 + data_len] = PJ_CMD_TERMINATOR;

    pj_tcp_tx_msg_len = 8 + data_len;
}

/**
 * @brief 检测电源状态
 *
 * @return int 电源状态， ON/OFF
 */
inline int check_pwr_sts()
{
    return Work_Stat == OFF ? PWR_STS_OFF : PWR_STS_ON;
}

/**
 * @brief 电源控制函数
 *
 * @param power
 * @return int
 */
int pjlink_power_ctrl(PJ_CMD_STRUCT *pj_cmd)
{
    int ret = 0;

    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0) // 非1类命令,返回未定义
    {
        return PJ_RET_UNDEFINED_CMD;
    }

    if (pj_cmd->data[0] == '0') // 关机
    {
        ret = HAL_UART_Transmit(&huart1, (uint8_t *)"Upw", 3, 5000); // 关机
    }
    else if (pj_cmd->data[0] == '1') // 开机
    {
        ret = HAL_UART_Transmit(&huart1, (uint8_t *)"{PWR}", 5, 5000); // 开机
    }
    else
    {
        return PJ_RET_OUT_OF_RANGE;
    }
    return ret == HAL_OK ? PJ_RET_OK : PJ_RET_PROJECTOR_FAILURE;
}

/**
 * @brief 电源状态查询函数
 *
 * @return int
 */
int pjlink_power_query(PJ_CMD_STRUCT *pj_cmd)
{
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0) // 非1类命令,返回未定义
    {
        return PJ_RET_UNDEFINED_CMD;
    }

    // HACK:目前无法实现严格的power off/warming/power on/cooling状态查询, 使用Mstar的work on 状态代替
    char power_sts_str;
    // 构建回复消息
    sprintf(&power_sts_str, "%d", check_pwr_sts());
    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, &power_sts_str, 1);
    return PJ_RET_OK;
}

/**
 * @brief 输入切换函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_input_switch(PJ_CMD_STRUCT *pj_cmd)
{
    int ret = 0;
    char input_chnl = pj_cmd->data[0];
    char input_chnl_2 = pj_cmd->data[1];
    /* 检查指令CLASS是否支持 */
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0 && memcmp(pj_cmd->class, PJ_CMD_CLASS2, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }
    /* 检查指令是否可生效, 仅power on状态可生效 */
    if (check_pwr_sts() != PWR_STS_ON)
    {
        return PJ_RET_UNAVAIABLE_TIME;
    }

    for (int i = 0; i < sizeof(pj_input_chnl_map) / sizeof(InputChnlMap); i++)
    {
        if (pj_input_chnl_map[i].pjlink_input_chnl_[0] == input_chnl &&
            pj_input_chnl_map[i].pjlink_input_chnl_[1] == input_chnl_2)
        {
            char msg[3] = {'s', 'r', pj_input_chnl_map[i].device_input_chnl_};

            ret = HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg), 5000);
            return ret == HAL_OK ? PJ_RET_OK : PJ_RET_PROJECTOR_FAILURE; // 查找到合法输入通道,发送切换指令
        }
    }
    return PJ_RET_OUT_OF_RANGE; // 未找到合法输入通道
}

/**
 * @brief 输入通道状态查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_input_query(PJ_CMD_STRUCT *pj_cmd)
{
    // int ret = 0;
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0 && memcmp(pj_cmd->class, PJ_CMD_CLASS2, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }
    /* 检查指令是否可生效, 仅power on状态可生效 */
    if (check_pwr_sts() != PWR_STS_ON)
    {
        return PJ_RET_UNAVAIABLE_TIME;
    }

    for (int i = 0; i < sizeof(pj_input_chnl_map) / sizeof(InputChnlMap); i++)
    {
        if (pj_input_chnl_map[i].device_input_chnl_ == Source_Stat)
        {
            construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN,
                              pj_input_chnl_map[i].pjlink_input_chnl_, 2);
            return PJ_RET_OK;
        }
    }
    return PJ_RET_PROJECTOR_FAILURE; // 未找到合法输入通道, 投影机故障
}

/**
 * @brief 输入通道列表查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_input_list_query(PJ_CMD_STRUCT *pj_cmd)
{
    // int ret = 0;
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0 && memcmp(pj_cmd->class, PJ_CMD_CLASS2, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }

    char input_chnl_list[95];
    int chnl_num = sizeof(pj_input_chnl_map) / sizeof(InputChnlMap);
    for (int i = 0; i < chnl_num; i++) // 填充列表
    {
        input_chnl_list[i * 3] = pj_input_chnl_map[i].pjlink_input_chnl_[0];
        input_chnl_list[i * 3 + 1] = pj_input_chnl_map[i].pjlink_input_chnl_[1];
        input_chnl_list[i * 3 + 2] = PJ_CMD_SP;
    }
    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, input_chnl_list,
                      chnl_num * 3 - 1); // 最后1byte SP不要
    return PJ_RET_OK;
}

/**
 * @brief 异常状态查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_err_status_query(PJ_CMD_STRUCT *pj_cmd)
{
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0) // 非1类命令,返回未定义
    {
        return PJ_RET_UNDEFINED_CMD;
    }
    // 构建回复消息
    uint8_t fan_err = 0;
    uint8_t lamp_err = 0;
    uint8_t temp_err = 0;
    uint8_t cover_err = 0;
    uint8_t filter_err = 0;
    uint8_t other_err = 0;

    fan_err = check_fan_err();
    temp_err = check_temp_err();
    // TODO: 其它异常检测暂不支持，返回0， 未来考虑支持

    char err_status[6] = {0};
    sprintf(err_status, "%d%d%d%d%d%d", fan_err, lamp_err, temp_err, cover_err, filter_err, other_err);
    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, err_status, 6);
    return PJ_RET_OK;
}

/**
 * @brief 投影机名称查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_projector_name_query(PJ_CMD_STRUCT *pj_cmd)
{
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }
    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, hw_info.projector_name,
                      strlen(hw_info.projector_name));
    return PJ_RET_OK;
}

/**
 * @brief 厂商名称查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_manufacture_name_query(PJ_CMD_STRUCT *pj_cmd)
{
    // int ret = 0;
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }

    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, hw_info.menufacture_name,
                      strlen(hw_info.menufacture_name));
    return PJ_RET_OK;
}

/**
 * @brief 产品名称查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_product_name_query(PJ_CMD_STRUCT *pj_cmd)
{
    // int ret = 0;
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }
    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, hw_info.product_name,
                      strlen(hw_info.product_name));
    return PJ_RET_OK;
}

/**
 * @brief 协议类查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_class_query(PJ_CMD_STRUCT *pj_cmd)
{
    // int ret = 0;
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS1, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }
    char class = '2';
    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, &class, 1);
    return PJ_RET_OK;
}

/**
 * @brief 设备序列号查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_serial_num_query(PJ_CMD_STRUCT *pj_cmd)
{
    // int ret = 0;
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS2, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }
    // HACK: 未定义序列号,返回虚假序列号
    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, (char*)(hw_info.serial_num),
                      strlen((char*)(hw_info.serial_num)));
    return PJ_RET_OK;
}

/**
 * @brief 软件版本查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_software_version_query(PJ_CMD_STRUCT *pj_cmd)
{
    // int ret = 0;
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS2, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }
    char software_version[8];
    sprintf(software_version, "V%d.%d.%d", (int)VERSION_MAJOR,
            (int)VERSION_MINOR, (int)VERSION_PATCH);
    construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, software_version,
                      strlen(software_version));
    return PJ_RET_OK;
}

/**
 * @brief 输入终端名称查询函数
 *
 * @param pj_cmd
 * @return int
 */
int pjlink_input_terminal_name_query(PJ_CMD_STRUCT *pj_cmd)
{
    // int ret = 0;
    if (memcmp(pj_cmd->class, PJ_CMD_CLASS2, 2) != 0)
    {
        return PJ_RET_UNDEFINED_CMD; // 非1\2类命令,返回未定义
    }

    uint8_t chnl_num = sizeof(pj_input_chnl_map) / sizeof(InputChnlMap);
    for (size_t i = 0; i < chnl_num; i++)
    {
        if (pj_cmd->data[1] == pj_input_chnl_map[i].pjlink_input_chnl_[0] &&
            pj_cmd->data[2] == pj_input_chnl_map[i].pjlink_input_chnl_[1])
        {
            construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN,
                              pj_input_chnl_map[i].input_chnl_name_, strlen(pj_input_chnl_map[i].input_chnl_name_));
            return PJ_RET_OK;
        }
    }
    return PJ_RET_OUT_OF_RANGE;
}

/**
 * @brief TCP命令处理, PJLINK控制协议
 *
 */
void PJLinkProcessTCPCmd(uint8_t *pCmd, int cmd_len)
{
    pj_tcp_rx_flag = FALSE;
    uint8_t errcode = PJ_RET_OK;
    // char ret_str[4] = {0};
    PJ_CMD_STRUCT *pj_cmd = (PJ_CMD_STRUCT *)pCmd;
    log_info("Process PJLink command: %s", pj_tcp_rx_buf);
    /* 查询指令 SP + '?' */
    if (pj_cmd->separator == PJ_CMD_SP && pj_cmd->data[0] == PJ_CMD_QUERY && pCmd[cmd_len - 1] == PJ_CMD_TERMINATOR)
    {
        if (memcmp(pj_cmd->command, PJ_CMD_POWR, strlen(PJ_CMD_POWR)) == 0) // 电源状态查询指令
        {
            errcode = pjlink_power_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_INPT, strlen(PJ_CMD_INPT)) == 0) // 输入源查询指令
        {
            errcode = pjlink_input_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_AVMT, strlen(PJ_CMD_AVMT)) == 0) // 静音命令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_ERST, strlen(PJ_CMD_ERST)) == 0) // 异常状态查询函数
        {
            errcode = pjlink_err_status_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_LAMP, strlen(PJ_CMD_LAMP)) == 0) // 灯泡状态查询函数
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_INST, strlen(PJ_CMD_INST)) == 0) // 输入切换列表查询函数
        {
            errcode = pjlink_input_list_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_NAME, strlen(PJ_CMD_NAME)) == 0) // 设备名称查询函数
        {
            errcode = pjlink_projector_name_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_INF1, strlen(PJ_CMD_INF1)) == 0) // 设备厂商查询函数
        {
            errcode = pjlink_manufacture_name_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_INF2, strlen(PJ_CMD_INF2)) == 0) // 产品名称查询函数
        {
            errcode = pjlink_product_name_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_INFO, strlen(PJ_CMD_INFO)) == 0) // 其它信息查询函数
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_CLSS, strlen(PJ_CMD_CLSS)) == 0) // 其它信息查询函数
        {
            errcode = pjlink_class_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_SNUM, strlen(PJ_CMD_SNUM)) == 0) // 设备序列号查询函数
        {
            errcode = pjlink_serial_num_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_SVER, strlen(PJ_CMD_SVER)) == 0) // 软件版本查询函数
        {
            errcode = pjlink_software_version_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_INNM, strlen(PJ_CMD_INNM)) == 0) // 输入终端名称查询函数
        {
            errcode = pjlink_input_terminal_name_query(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_IRES, strlen(PJ_CMD_IRES)) == 0) // 输入分辨率查询命令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_RRES, strlen(PJ_CMD_RRES)) == 0) // 推荐分辨率查询命令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_FILT, strlen(PJ_CMD_FILT)) == 0) // 过滤器使用时间查询命令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_RLMP, strlen(PJ_CMD_RLMP)) == 0) // 灯泡更换型号查询命令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_RFLT, strlen(PJ_CMD_RFLT)) == 0) // 过滤器更换型号查询命令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_FREZ, strlen(PJ_CMD_FREZ)) == 0) // 冻结屏幕查询指令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else // 未定义的命令
        {
            log_warn("undefined PJLink command");
            errcode = PJ_RET_UNDEFINED_CMD;
        }
        // 查询指令格式不统一，数据不定长，在各功能函数中构建正常响应包，否则在这里构建异常响应包
        if (errcode != PJ_RET_OK)
        {
            construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, errCodeStr[errcode], 4);
            // 多元运算符，将ret映射为对应字符串
        }
    }
    /* 控制指令 SP */
    else if (pj_cmd->separator == PJ_CMD_SP && pCmd[cmd_len - 1] == PJ_CMD_TERMINATOR)
    {
        if (memcmp(pj_cmd->command, PJ_CMD_POWR, strlen(PJ_CMD_POWR)) == 0) // 电源控制指令
        {
            errcode = pjlink_power_ctrl(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_INPT, strlen(PJ_CMD_INPT)) == 0) // 输入源切换指令
        {
            errcode = pjlink_input_switch(pj_cmd);
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_AVMT, strlen(PJ_CMD_AVMT)) == 0) // 静音命令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 静音命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_SVOL, strlen(PJ_CMD_SVOL)) == 0) // 扬声器音量调节指令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_MVOL, strlen(PJ_CMD_MVOL)) == 0) // 麦克风音量调节指令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else if (memcmp(pj_cmd->command, PJ_CMD_FREZ, strlen(PJ_CMD_FREZ)) == 0) // 冻结屏幕指令
        {
            errcode = PJ_RET_UNDEFINED_CMD; // TODO: 命令暂不支持，返回未定义，所有数据结构均未定义
        }
        else
        {
            log_warn("undefined PJLink command");
            errcode = PJ_RET_UNDEFINED_CMD;
        }
        // 控制指令响应包格式统一，因此统一在这里构建
        if (errcode == PJ_RET_OK)
            construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, errCodeStr[errcode], 2);
        else
            construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, errCodeStr[errcode], 4);
    }
    else // 命令格式错误
    {
        log_warn("illegal PJLink command");
        errcode = PJ_RET_UNDEFINED_CMD;
        construct_message(pj_tcp_tx_buf, pj_cmd->class[1], pj_cmd->command, PJ_CMD_ASSIGN, errCodeStr[errcode], 4);
    }

    // 发送响应
    SendPJLinkTcpDataToClient((u8_t *)pj_tcp_tx_buf, pj_tcp_tx_msg_len);
    memset(pj_tcp_tx_buf, 0, PJ_CMD_BUF_SIZE); // 清空发送指令
    return;
}

/**
 * @brief UDP命令处理， PJLINK搜索协议
 *
 */
void PJLinkProcessUDPCmd(struct ip_addr *addr)
{
    // 查找命令响应
    if (memcmp(pj_udp_rx_buf, PJ_CLASS2_CMD_SEARCH, strlen(PJ_CLASS2_CMD_SEARCH)) == 0)
    {
        /* 记录PJLINK Controller IP */
        IP4_ADDR(&pjlink_manager.pj_controller_ip, (u8_t)(addr->addr), (u8_t)(addr->addr >> 8),
                 (u8_t)(addr->addr >> 16), (u8_t)(addr->addr >> 24));
        // 使能状态通知
        pjlink_manager.statusNotifyEn = ON;
        sprintf(pj_udp_tx_buf, PJ_CLASS2_CMD_SEARCH_ACK_FMT, lwipdev.mac[0], lwipdev.mac[1], lwipdev.mac[2],
                lwipdev.mac[3], lwipdev.mac[4], lwipdev.mac[5]);
        // 发送搜索应答
        pjlink_udp_send((uint8_t *)pj_udp_tx_buf, 25);
    }
    else
    {
        // 发送错误应答
        log_warn("PJLink UDP command illegal, no response.");
    }
}

/**
 * @brief 状态通知协议，Linkup状态
 *
 */
void PJLink_StatusNotify_LinkupStatus(void)
{
    if (pjlink_manager.statusNotifyEn == OFF)
        return;
    sprintf(pj_udp_tx_buf, PJ_CLASS2_SNC_LINKUP_STS_FMT, lwipdev.mac[0], lwipdev.mac[1], lwipdev.mac[2], lwipdev.mac[3],
            lwipdev.mac[4], lwipdev.mac[5]);
    pjlink_udp_send((uint8_t *)pj_udp_tx_buf, strlen(pj_udp_tx_buf));
}

/**
 * @brief 状态通知协议，错误状态
 *
 */
void PJLink_StatusNotify_ErrorStatus(uint8_t fan_err, uint8_t lamp_err, uint8_t temp_err, uint8_t cover_err,
                                     uint8_t filter_err, uint8_t other_err)
{
    if (pjlink_manager.statusNotifyEn == OFF)
        return;
    sprintf(pj_udp_tx_buf, PJ_CLASS2_SNC_ERR_STS_FMT, fan_err, lamp_err, temp_err, cover_err, filter_err, other_err);
    pjlink_udp_send((uint8_t *)pj_udp_tx_buf, strlen(pj_udp_tx_buf));
}

/**
 * @brief 状态通知协议，电源状态
 *
 */
void PJLink_StatusNotify_PowerStatus(uint8_t power_status)
{
    if (pjlink_manager.statusNotifyEn == OFF)
        return;
    // uint8_t power_status = Work_Stat; // 0:关机 1:开机

    sprintf(pj_udp_tx_buf, PJ_CLASS2_SNC_PWR_STS_FMT, power_status);
    pjlink_udp_send((uint8_t *)pj_udp_tx_buf, strlen(pj_udp_tx_buf));
}

/**
 * @brief 状态通知协议，输入状态
 *
 */
void PJLink_StatusNotify_InputStatus(uint8_t input)
{
    if (pjlink_manager.statusNotifyEn == OFF)
        return;
    int chnl_num = sizeof(pj_input_chnl_map) / sizeof(InputChnlMap);
    for (int i = 0; i < chnl_num; i++)
    {
        if (pj_input_chnl_map[i].device_input_chnl_ == input)
        {
            sprintf(pj_udp_tx_buf, PJ_CLASS2_SNC_INPT_STS_FMT, pj_input_chnl_map[i].pjlink_input_chnl_[0],
                    pj_input_chnl_map[i].pjlink_input_chnl_[1]);
            pjlink_udp_send((uint8_t *)pj_udp_tx_buf, strlen(pj_udp_tx_buf));
            return;
        }
    }
}

/**
 * @brief PJLink命令处理函数
 *
 */
void PJLinkThread(void)
{
    if (pj_tcp_rx_flag == TRUE) // 处理TCP命令
    {
        if (pjlink_authenticator.authPass == FALSE) // 还未进行认证
        {
            /* 密码比对 */
            if (memcmp(pjlink_authenticator.EncryptedMsg, pj_tcp_rx_buf, 32) == 0)
            {
                pjlink_authenticator.authPass = TRUE;
                // 认证通过，继续处理本条消息内紧跟着的命令
                PJLinkProcessTCPCmd((uint8_t *)(pj_tcp_rx_buf + 32), PJ_TCP_data_len - 32);
            }
            else // 回复认证失败
            {
                pjlink_authenticator.authPass = FALSE;
                sprintf(pj_tcp_tx_buf, "%s%c%s\r", PJ_CMD_AUTH, PJ_CMD_SP, PJ_RET_STR_AUTH_FAIL);
                SendPJLinkTcpDataToClient((u8_t *)pj_tcp_tx_buf, strlen(pj_tcp_tx_buf));
            }
        }
        else // 已认证通过，直接处理命令
        {
            PJLinkProcessTCPCmd((uint8_t *)pj_tcp_rx_buf, PJ_TCP_data_len);
        }
    }

    if (pj_connectFlag == 1) // TCP连接正常,则进行超时检测
    {
        check_tcp_link_time();
    }
}

// PJLink 业务函数
