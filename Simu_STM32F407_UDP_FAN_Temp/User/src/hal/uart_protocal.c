#include "uart_protocal.h"
#include "usart.h"
#include <string.h>
#include "stmflash.h"
#include "main.h"
#include "logger.h"
#include "utils/inc/util.h"
#include "netRpcCall.h"
#include "rt_param.h"
#include "modectl.h"    

uint8_t uartMsgBuf[256];


// clang-format off
/**
 * @brief 
 * 
 *///
 
/*******************************************************
* @brief UART指令callback业务函数查找表
*        内部芯片间调用，实时性要求高，使用索引表，直接调用功能函数
*******************************************************/
cmdFuncLut cmdLut[] = {
    {0,               NULL,                  },
    {1,               NULL,                  },
    {2,               NULL,                  },
    {3,               NULL,                  },
    {4,               NULL,                  },
    {5,               NULL,                  },
    {6,               NULL,                  },
    {7,               NULL,                  },
    {8,               NULL,                  },
    {9,               NULL,                  },
    {10,              NULL,                  },
    {11,              NULL,                  },
    {12,              NULL,                  },
    {13,              NULL,                  },
    {14,              NULL,                  },
    {15,              NULL,                  },
    {16,              NULL,                  },
    {17,              NULL,                  },
    {18,              NULL,                  },
    {19,              NULL,                  },
    {20,              NULL,                  },
    {21,              NULL,                  },
    {22,              NULL,                  },
    {23,              NULL,                  },
    {24,              NULL,                  },
    {25,              NULL,                  },
    {26,              NULL,                  },
    {27,              NULL,                  },
    {28,              NULL,                  },
    {29,              NULL,                  },
    {30,              NULL,                  },
    {31,              NULL,                  },
    {32,              NULL,                  },
    {33,              NULL,                  },
    {34,              NULL,                  },
    {35,              NULL,                  },
    {36,              NULL,                  },
    {37,              NULL,                  },
    {38,              NULL,                  },
    {39,              NULL,                  },
    {40,              NULL,                  },
    {41,              NULL,                  },
    {42,              NULL,                  },
    {43,              NULL,                  },
    {44,              NULL,                  },
    {45,              NULL,                  },
    {46,              NULL,                  },
    {47,              NULL,                  },
    {48,              NULL,                  },
    {49,              NULL,                  },
    {50,              NULL,                  },
    {51,              NULL,                  },
    {52,              NULL,                  },
    {53,              NULL,                  },
    {54,              NULL,                  },
    {55,              NULL,                  },
    {56,              NULL,                  },
    {57,              NULL,                  },
    {58,              NULL,                  },
    {59,              NULL,                  },
    {60,              NULL,                  },
    {61,              NULL,                  },
    {62,              NULL,                  },
    {63,              NULL,                  },
    {64,              NULL,                  },
    {65,              NULL,                  },
    {66,              NULL,                  },
    {67,              NULL,                  },
    {68,              NULL,                  },
    {69,              NULL,                  },
    {70,              NULL,                  },
    {71,              NULL,                  },
    {72,              NULL,                  },
    {73,              NULL,                  },
    {74,              NULL,                  },
    {75,              NULL,                  },
    {76,              NULL,                  },
    {77,              NULL,                  },
    {78,              NULL,                  },
    {79,              NULL,                  },
    {80,              NULL,                  },
    {81,              NULL,                  },
    {MST_CMD_IP,      CmdFunc_IpUpdate,      },             //IP 更新业务处理函数
    {83,              NULL,                  },
    {84,              NULL,                  },
    {85,              NULL,                  },
    {86,              NULL,                  },
    {87,              NULL,                  },
    {88,              NULL,                  },
    {89,              NULL,                  },
    {90,              NULL,                  },
    {91,              NULL,                  },
    {92,              NULL,                  },
    {93,              NULL,                  },
    {94,              NULL,                  },
    {95,              NULL,                  },
    {96,              NULL,                  },
    {97,              NULL,                  },
    {98,              NULL,                  },
    {99,              NULL,                  },
    {100,             NULL,                  },
    {101,             NULL,                  },
    {102,             NULL,                  },
    {103,             NULL,                  },
    {104,             NULL,                  },
    {105,             NULL,                  },
    {106,             NULL,                  },
    {107,             NULL,                  },
    {108,             NULL,                  },
    {109,             NULL,                  },
    {110,             NULL,                  },
    {111,             NULL,                  },
    {112,             NULL,                  },
    {113,             NULL,                  },
    {114,             NULL,                  },
    {115,             NULL,                  },
    {116,             NULL,                  },
    {117,             NULL,                  },
    {118,             NULL,                  },
    {119,             NULL,                  },
    {120,             NULL,                  },
    {121,             NULL,                  },
    {122,             NULL,                  },
    {123,             NULL,                  },
    {124,             NULL,                  },
    {125,             NULL,                  },
    {126,             NULL,                  },
    {127,             NULL,                  },
    {128,             NULL,                  },
    {129,             NULL,                  },
    {130,             NULL,                  },
    {131,             NULL,                  },
    {132,             NULL,                  },
    {133,             NULL,                  },
    {134,             NULL,                  },
    {135,             NULL,                  },
    {136,             NULL,                  },
    {137,             NULL,                  },
    {138,             NULL,                  },
    {139,             NULL,                  },
    {140,             NULL,                  },
    {141,             NULL,                  },
    {142,             NULL,                  },
    {143,             NULL,                  },
    {144,             NULL,                  },
    {145,             NULL,                  },
    {146,             NULL,                  },
    {147,             NULL,                  },
    {148,             NULL,                  },
    {149,             NULL,                  },
    {150,             NULL,                  },
    {151,             NULL,                  },
    {152,             NULL,                  },
    {153,             NULL,                  },
    {154,             NULL,                  },
    {155,             NULL,                  },
    {156,             NULL,                  },
    {157,             NULL,                  },
    {158,             NULL,                  },
    {159,             NULL,                  },
    {160,             CmdFunc_ParamSync,     },    // 参数同步业务处理函数
    {161,             NULL,                  },
    {162,             NULL,                  },
    {163,             NULL,                  },
    {164,             NULL,                  },
    {165,             NULL,                  },
    {166,             NULL,                  },
    {167,             NULL,                  },
    {168,             NULL,                  },
    {169,             NULL,                  },
    {170,             NULL,                  },
    {171,             NULL,                  },
    {172,             NULL,                  },
    {173,             NULL,                  },
    {174,             NULL,                  },
    {175,             NULL,                  },
    {176,             NULL,                  },
    {177,             NULL,                  },
    {178,             NULL,                  },
    {179,             NULL,                  },
    {180,             NULL,                  },
    {181,             NULL,                  },
    {182,             NULL,                  },
    {183,             NULL,                  },
    {184,             NULL,                  },
    {185,             NULL,                  },
    {186,             NULL,                  },
    {MST_CMD_TRANSPARENT, CmdFunc_TransparentData, },       //透传数据处理函数
    {188,             NULL,                  },
    {189,             NULL,                  },
    {190,             NULL,                  },
    {191,             NULL,                  },
    {192,             NULL,                  },
    {193,             NULL,                  },
    {194,             NULL,                  },
    {195,             NULL,                  },
    {196,             NULL,                  },
    {197,             NULL,                  },
    {198,             NULL,                  },
    {199,             NULL,                  },
    {200,             NULL,                  },
    {201,             NULL,                  },
    {202,             NULL,                  },
    {203,             NULL,                  },
    {204,             NULL,                  },
    {205,             NULL,                  },
    {206,             NULL,                  },
    {207,             NULL,                  },
    {208,             NULL,                  },
    {209,             NULL,                  },
    {210,             NULL,                  },
    {211,             NULL,                  },
    {212,             NULL,                  },
    {213,             NULL,                  },
    {214,             NULL,                  },
    {215,             NULL,                  },
    {216,             NULL,                  },
    {217,             NULL,                  },
    {218,             NULL,                  },
    {219,             NULL,                  },
    {220,             NULL,                  },
    {221,             NULL,                  },
    {222,             NULL,                  },
    {223,             NULL,                  },
    {224,             NULL,                  },
    {225,             NULL,                  },
    {226,             NULL,                  },
    {227,             NULL,                  },
    {228,             NULL,                  },
    {229,             NULL,                  },
    {230,             NULL,                  },
    {231,             NULL,                  },
    {232,             NULL,                  },
    {233,             NULL,                  },
    {234,             NULL,                  },
    {235,             NULL,                  },
    {236,             NULL,                  },
    {237,             NULL,                  },
    {238,             NULL,                  },
    {239,             NULL,                  },
    {240,             NULL,                  },
    {241,             NULL,                  },
    {242,             NULL,                  },
    {243,             NULL,                  },
    {244,             NULL,                  },
    {245,             NULL,                  },
    {246,             NULL,                  },
    {247,             NULL,                  },
    {248,             NULL,                  },
    {249,             NULL,                  },
    {250,             NULL,                  },
    {251,             NULL,                  },
    {252,             NULL,                  },
    {253,             NULL,                  },
    {254,             NULL,                  },
    {255,             NULL,                  },
};
// clang-format on


/**
 * @brief  应用层协议发包函数
 * 
 * @param pData         payload 数据包[cmd][data]
 * @param dataLen       payload 数据包长度
 * @param needAck       是否需要响应
 * @param ackBuf        响应包buffer
 * @param ackBufLen     响应包buffer长度
 * @param ackTimeout_ms 超时时间
 * @return int      >0: 成功,返回包长度  -1: 参数错误  -2:超时   -3:ack数据错误  -4: buffer长度不够 
 */
int appSendUartMsg(uint8_t* pData,  int dataLen, int needAck, uint8_t* ackBuf, int ackBufLen, int ackTimeout_ms)
{
    if(dataLen > 252) return -1;   // 参数检查
    if(pData == NULL) return -1;

    // 协议层组包
    uartMsgBuf[0] = 0XAA;
    uartMsgBuf[1] = dataLen;
    memcpy(&uartMsgBuf[2], pData, dataLen);
    uartMsgBuf[dataLen + 2] = simple_checksum(pData,dataLen);
    int msgLen = dataLen + 3 ; 
    // 发送msg  [GET/SET] [CMD] [DATA] [...]
    HAL_UART_Transmit(&huart1, uartMsgBuf, msgLen, 0x100);
    
    // 打印，用于debug
    printf("\n");
    printf("appSendUartMsg:\n");
    for (size_t i = 0; i < msgLen; i++)
    {
        printf("%02x,", uartMsgBuf[i]);
    }
    printf("\n");

    // 检查ack包
	if(needAck)
    {
        int old_systick = HAL_GetTick();
        while (!g_uart1_ack_flag)
        {	
            if( HAL_GetTick() - old_systick > ackTimeout_ms)
            {
                log_warn(" -> cmd [%d] ack timeout.", pData[0]);
                return  -2;     //time out 
            }
        }
        // ACK包格式 [ACK][CMD][RET_DATA][...]
        //检查ack cmd 和 响应的发送CMD是否正确
        uint16_t sendCmd = (((uint16_t*)pData)[0] & 0x7fff);                            //发送的CMD
        uint16_t ackCmd = ((*((uint16_t*)(&g_uart1_rx_ack_buf[0]))) & 0x7fff);          //响应CMD
        uint16_t retCmd = ((*((uint16_t*)(&g_uart1_rx_ack_buf[2]))) & 0x7fff);          //返回发送的CMD值
        if((ackCmd != MST_CMD_ACK) || (retCmd != sendCmd) )
        {
            log_warn(" -> cmd [%d] ack data err.",pData[0]);
            log_warn("ack_cmd:0x%04X, send_cmd:0x%04X, ret_cmd:0x%04X",ackCmd, sendCmd, retCmd);
            return -3;          // ack err
        }
        //根据输入指针确认是否需要返回ack数据用于应用层处理
        if( ackBuf != NULL )
        {
            if(ackBufLen < g_uart1_ack_len - 4 )
                return -4;
            //ACK 格式：  [ACK] [CMD] [DATA][...]
            memcpy(ackBuf, &g_uart1_rx_ack_buf[4], g_uart1_ack_len - 4); 
        }
        g_uart1_ack_flag = 0;
	    return g_uart1_ack_len -4;
    }	
    else 
    {
        return 0 ;
    }
}


/**
 * @brief  应用层协议发包函数
 * 
 * @param pData         payload 数据包[cmd][data]
 * @param dataLen       payload 数据包长度
 * @param needAck       是否需要响应
 * @param ackBuf        响应包buffer
 * @param ackBufLen     响应包buffer长度
 * @param ackTimeout_ms 超时时间
 * @return int      >0: 成功,返回包长度  -1: 参数错误  -2:超时   -3:ack数据错误  -4: buffer长度不够 
 */
int appSendUartRawData(uint8_t* pData,  int dataLen, int needAck, uint8_t* ackBuf, int ackBufLen, int ackTimeout_ms)
{
    if(dataLen > 255) return -1;   // 参数检查
    if(pData == NULL) return -1;

    HAL_UART_Transmit(&huart1, pData, dataLen, 0x100);

    // 检查ack包
	if(needAck)
    {
        int old_systick = HAL_GetTick();
        while (!g_uart1_ack_flag)
        {	
            if( HAL_GetTick() - old_systick > ackTimeout_ms)
            {
                log_warn(" -> cmd [%d] ack timeout.", pData[0]);
                return  -2;     //time out 
            }
        }

        //根据输入指针确认是否需要返回ack数据用于应用层处理
        if( ackBuf != NULL )
        {
            if(ackBufLen < g_uart1_ack_len - 4 )
                return -4;
            //ACK 格式：  [ACK] [CMD] [DATA][...]
            memcpy(ackBuf, &g_uart1_rx_ack_buf[4], g_uart1_ack_len - 4); 
        }
        g_uart1_ack_flag = 0;
	    return g_uart1_ack_len -4;
    }	
    else 
    {
        return 0 ;
    }
}


/* 新版本协议接口实现，0XCC 包头 ***************************************************************/

/**
 * @brief  异步发送函数(0xCC协议), 发送后不等待响应, 直接返回
 * 
 * @param pData     发送数据包 [cmd][payload...]
 * @param dataLen   数据包长度 (cmd + payload)
 * @return int      0: 成功  <0: 失败
 */
int appSendUartAsync(uint8_t cmd, uint8_t* pPayload, int payloadLen)
{
    if (pPayload == NULL || payloadLen < 1) return RETVAL_PARAM_ERR;
    if (payloadLen > 251) return RETVAL_PARAM_ERR;  // 251byte payload max

    // 组装 0xCC 协议包: [header][cmd][payloadLen][payload][checksum]
    uartMsgBuf[0] = 0xDD;
    uartMsgBuf[1] = cmd;
    uartMsgBuf[2] = payloadLen;
    if (payloadLen > 0) {
        memcpy(&uartMsgBuf[3], pPayload, payloadLen);
    }
    uartMsgBuf[3 + payloadLen] = simple_checksum(pPayload, payloadLen);
    int msgLen = 4 + payloadLen;  // header + cmd + length + payload + checksum

    // 发送数据
    HAL_UART_Transmit(&huart1, uartMsgBuf, msgLen, 0x100);
    // 打印调试信息
    print_mem_hexdump("uart1 tx", uartMsgBuf, msgLen);
    return RETVAL_OK;
}

/**
 * @brief  同步发送函数(0xCC协议), 发送后等待响应, 收到响应后返回
 * 
 * @param pData         发送数据包 [cmd][payload...]
 * @param dataLen       数据包长度 (cmd + payload)
 * @param ackBuf        响应数据buffer (仅payload部分)
 * @param ackBufLen     响应buffer长度
 * @param ackTimeout_ms 超时时间(ms)
 * @return int          >0: 成功,返回响应payload长度  <0: 失败
 */
int appSendUartSync(uint8_t cmd, uint8_t* pPayload, int payloadLen, uint8_t* ackBuf, int ackBufLen, int ackTimeout_ms)
{
    if (pPayload == NULL || payloadLen < 1) return RETVAL_PARAM_ERR;
    if (payloadLen > 251) return RETVAL_PARAM_ERR;

    // 设置ACK等待上下文
    g_waitAckCtx.cmd = cmd;
    g_waitAckCtx.ackBuf = ackBuf;
    g_waitAckCtx.ackBufLen = ackBufLen;
    g_waitAckCtx.ackFlag = 0;
    g_waitAckCtx.ackLen = 0;
    g_waitAckCtx.waitAckFlag = 1;  // 最后置位，启动等待

    // 组装 0xCC 协议包: [header][cmd][length][payload][checksum]
    uartMsgBuf[0] = 0xDD;
    uartMsgBuf[1] = cmd;
    uartMsgBuf[2] = payloadLen;
    if (payloadLen > 0) {
        memcpy(&uartMsgBuf[3], pPayload, payloadLen);
    }
    uartMsgBuf[3 + payloadLen] = simple_checksum(pPayload, payloadLen);
    int msgLen = 4 + payloadLen;

    // 发送数据
    HAL_UART_Transmit(&huart1, uartMsgBuf, msgLen, 0x100);
    // 打印调试信息
    print_mem_hexdump("uart1 tx", uartMsgBuf, msgLen);

    // 等待响应
    int old_systick = HAL_GetTick();
    while (!g_waitAckCtx.ackFlag)
    {
        if (HAL_GetTick() - old_systick > ackTimeout_ms)
        {
            g_waitAckCtx.waitAckFlag = 0;  // 清除等待标志
            log_warn(" -> cmd [0x%02X] ack timeout.", cmd);
            return RETVAL_TIMEOUT;
        }
    }

    // 收到响应，清除等待标志
    g_waitAckCtx.waitAckFlag = 0;
    
    return g_waitAckCtx.ackLen;
}


/* uart消息处理业务函数，注册进LUT中 ****************************************************************/

/**
 * @brief uart指令业务callback函数， IP参数更新
 * 
 * @param param     参数指针
 * @return int      执行结果
 */
int CmdFunc_IpUpdate(uint8_t get_set, uint8_t paramLen, void* param)
{
    memcpy(Ethernet_Info, param, 18);
    STMFLASH_Write(ADDR_FLASH_SECTOR_6, (uint32_t *)Ethernet_Info, 5);
    lwipIPUpdate();
    HAL_UART_Transmit(&huart1, (uint8_t *)"Ucn", 3, 500);

    rt_param.ip_addr[0] = Ethernet_Info[0];
    rt_param.ip_addr[1] = Ethernet_Info[1];
    rt_param.ip_addr[2] = Ethernet_Info[2];
    rt_param.ip_addr[3] = Ethernet_Info[3];
    rt_param.mask[0] = Ethernet_Info[4];
    rt_param.mask[1] = Ethernet_Info[5];
    rt_param.mask[2] = Ethernet_Info[6];
    rt_param.mask[3] = Ethernet_Info[7];
    rt_param.gateway[0] = Ethernet_Info[8];
    rt_param.gateway[1] = Ethernet_Info[9];
    rt_param.gateway[2] = Ethernet_Info[10];
    rt_param.gateway[3] = Ethernet_Info[11];
    rt_param.report_ip[0] = Ethernet_Info[12];
    rt_param.report_ip[1] = Ethernet_Info[13];
    rt_param.report_ip[2] = Ethernet_Info[14];
    rt_param.report_ip[3] = Ethernet_Info[15];
    rt_param.report_port = Ethernet_Info[16] * 100 + Ethernet_Info[17];
    save_rt_param_to_flash();

    return 0;
}

/**
 * @brief uart指令业务callback函数， 透传数据处理
 * 
 * @param get_set   获取/设置标识（透传中不使用）
 * @param param     透传数据指针
 * @return int      执行结果
 */
int CmdFunc_TransparentData(uint8_t get_set, uint8_t paramLen, void* param)
{   
    // 调用透传通知函数，将数据发送给上位机
    mstar_transparent_notify(param, paramLen);
    return 0;
}


/**
 * @brief uart指令业务callback函数， 接收Mstar 发来的参数同步消息
 * 
 * @param get_set   获取/设置标识（参数同步中不使用）
 * @param paramLen  参数长度
 * @param param     参数指针
 * @return int      执行结果
 */
int CmdFunc_ParamSync(uint8_t get_set, uint8_t paramLen, void* param)
{
    PARAM_ID_ id = (PARAM_ID_)((uint8_t*)param)[0];
    uint8_t *pParam = (uint8_t*)param;
	
    char ip_str[20] = {0};
    char ip_mask_str[20] = {0};
    char ip_gateway_str[20] = {0};
    char report_ip_str[20] = {0};

    switch(id)
    {
        case PARAM_ID_DISPLAY_MODE:
            rt_param.display_mode = pParam[1];
            break;
        case PARAM_ID_COLOR_SPACE:
            rt_param.color_space = pParam[1];
            break;
        case PARAM_ID_COLOR_TEMP:
            rt_param.color_temp = pParam[1];
            break;
        case PARAM_ID_LASER_EN:
            rt_param.laser_en = pParam[1];
            break;
        case PARAM_ID_LASER_BRIGHT:
            rt_param.laser_lvl = pParam[1];
            break;
        case PARAM_ID_IR_EN:
            rt_param.ir_en = pParam[1];
            break;
        case PARAM_ID_IR_BRIGHT:
            rt_param.ir_lvl = pParam[1];
            break;
        case PARAM_ID_IR_ID:
            rt_param.ir_id = pParam[1];
        case PARAM_ID_INSTALLATION_MODE:
            rt_param.installation_mode = pParam[1];
            break;
        case PARAM_ID_HIGH_ALTITUDE:
            rt_param.high_altitude = pParam[1];
            modectl_switch_mode( (pParam[1] == 1) ? MODECTL_CMD_HI_ALT_ON : MODECTL_CMD_HI_ALT_OFF);
            break;
        case PARAM_ID_INPUT_SOURCE:
            rt_param.input_source = pParam[1];
#if (Projector_Model == LP92SLB)
            if(rt_param.input_source == VIDEO_INPUT_SRC_HDMI_1 || rt_param.input_source == VIDEO_INPUT_SRC_HDMI_2)
            {
                rt_param.video_in_chan = rt_param.input_source;
                video_src_changed = TRUE;
            }
#endif
            break;
        case PARAM_ID_INPUT_COLOR_FORMAT:
            rt_param.input_color_format = pParam[1];
            break;
        case PARAM_ID_INPUT_COLOR_SPACE:
            rt_param.input_color_space = pParam[1];
            break;
        case PARAM_ID_CONTRAST:
            rt_param.contrast = pParam[1];
            break;
        case PARAM_ID_SHARPNESS:
            rt_param.sharpness = pParam[1];
            break;
        case PARAM_ID_BRIGHTNESS:
            rt_param.brightness = pParam[1];
            break;
        case PARAM_ID_GAMMA:
            rt_param.gamma = pParam[1];
            break;
        case PARAM_ID_BACKGROUND_COLOR:
            rt_param.background_color = pParam[1];
            break;
        case PARAM_ID_LENS_LOCK:
            rt_param.lens_lock = pParam[1];
            OE_lock = (rt_param.lens_lock == 1) ? TRUE : FALSE;
            break;
        case PARAM_ID_ALARM_EN:
            rt_param.alarm_en = pParam[1];
            break;
        case PARAM_ID_ALARM_TIP:
            rt_param.alarm_tip = pParam[1];
            break;
        case PARAM_ID_NO_INPUT_POWEROFF:
            rt_param.no_input_poweroff = pParam[1];
            break;
        case PARAM_ID_POWER_AUTO_BOOT:
            rt_param.power_auto_boot = pParam[1];
            break;
        case PARAM_ID_ASPECT_RATIO:
            rt_param.aspect_ratio = pParam[1];
            break;
        case PARAM_ID_LANGUAGE:
            rt_param.language = pParam[1];
            break;
        case PARAM_ID_HSG_PRARM:
            memcpy(rt_param.HSG_param, pParam, 21);
            break;
        case PARAM_ID_RGB_GAIN:
            memcpy(rt_param.RGB_gain, pParam, 3);
            break;
        case PARAM_ID_INDEP_COLOR:
            memcpy(rt_param.indep_color, pParam, 6);
            break;
        case PARAM_ID_DISP_MODEL:
            strcpy(rt_param.display_model, (char*)pParam);
            break;
        case PARAM_ID_DISP_SN:
            strcpy(rt_param.display_sn, (char*)pParam);
            break;
        case PARAM_ID_DISP_DATE:
            strcpy(rt_param.display_date, (char*)pParam);
            break;
        case PARAM_ID_LOCAL_IP_SET:
            rt_param.ip_addr[0] = pParam[0];
            rt_param.ip_addr[1] = pParam[1];
            rt_param.ip_addr[2] = pParam[2];
            rt_param.ip_addr[3] = pParam[3];
            rt_param.mask[0] = pParam[4];
            rt_param.mask[1] = pParam[5];
            rt_param.mask[2] = pParam[6];
            rt_param.mask[3] = pParam[7];
            rt_param.gateway[0] = pParam[8];
            rt_param.gateway[1] = pParam[9];
            rt_param.gateway[2] = pParam[10];
            rt_param.gateway[3] = pParam[11];
            
            sprintf(ip_str, "%d.%d.%d.%d", rt_param.ip_addr[0], rt_param.ip_addr[1], rt_param.ip_addr[2], rt_param.ip_addr[3]);
            sprintf(ip_mask_str, "%d.%d.%d.%d", rt_param.mask[0], rt_param.mask[1], rt_param.mask[2], rt_param.mask[3]);
            sprintf(ip_gateway_str, "%d.%d.%d.%d", rt_param.gateway[0], rt_param.gateway[1], rt_param.gateway[2], rt_param.gateway[3]);
            set_ip_config(ip_str, ip_mask_str, ip_gateway_str);

            break;
        case PARAM_ID_REPORT_IP_SET:
            rt_param.report_ip[0] = pParam[0];
            rt_param.report_ip[1] = pParam[1];
            rt_param.report_ip[2] = pParam[2];
            rt_param.report_ip[3] = pParam[3];
            rt_param.report_port = pParam[4] * 100 + pParam[5];

            sprintf(report_ip_str, "%d.%d.%d.%d", rt_param.report_ip[0], rt_param.report_ip[1], rt_param.report_ip[2], rt_param.report_ip[3]);

            set_report_ip(report_ip_str, rt_param.report_port);
            
            break;
        case PARAM_ID_ECO_MODE_EN:
            rt_param.eco_mode_en = pParam[1];
            modectl_switch_mode((pParam[1] == 1 )? MODECTL_CMD_ECO_ON : MODECTL_CMD_ECO_OFF);
            break;
        case PARAM_ID_LASER_LEVEL:
            rt_param.laser_bright[0] = pParam[0];
            rt_param.laser_bright[1] = pParam[1];
            rt_param.laser_bright[2] = pParam[2];
            break;
        default:
            break;
    } 
    save_rt_param_to_flash();
    return 0;
}


/* 总接口*****************************************************************************************/

/**
 * @brief uart指令解析函数，解析指令并执行callback业务函数
 * 
 * @return int 
 */
int uartCmdParse(void)
{

    if(g_uart1_cmd_type == 0XDD) // 0xCC协议
    {
        uint8_t cmd = g_uart1_cmd_cmd;
        uint8_t* pPayload = g_uart1_rx_cmd_buf;
        int payloadLen = g_uart1_cmd_len;
        
        if(cmdLut[cmd].func_cb!=NULL)
            cmdLut[cmd].func_cb(0, payloadLen, pPayload);             //执行callback函数
        else    
            log_warn("UART CMD [%d] empty.", cmd);

        return 0;
    }
    else if(g_uart1_cmd_type == 0XAA)
    {
        uint8_t getset = ((((uint16_t*)g_uart1_rx_cmd_buf)[0] & 0x8000) >> 8);
        uint16_t cmdIdx  = ((uint16_t*)g_uart1_rx_cmd_buf)[0] & 0x00ff;
        
        // get param ptr and param len
        uint8_t* paramPtr = &g_uart1_rx_cmd_buf[2];
        uint8_t paramLen = g_uart1_cmd_len - 2;
         
        // execute callback function
        if(cmdLut[cmdIdx].func_cb!=NULL)
            cmdLut[cmdIdx].func_cb(getset, paramLen, paramPtr);             //执行callback函数
        else    
            log_warn("UART CMD [%d] empty.", cmdIdx);
        return 0;
    }
		return 0;
}


