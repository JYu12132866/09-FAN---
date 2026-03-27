#ifndef UART_PROTOCAL_H_
#define UART_PROTOCAL_H_

#include <stdint.h>
#include <stdio.h>


#define RETVAL_OK           0
#define RETVAL_TIMEOUT      -1
#define RETVAL_PARAM_ERR    -2


#define CMDTYPE_GET     (1 << 7)
#define CMDTYPE_SET     0

#define CMD_ON              0xF1
#define CMD_OFF             0x1F
#define CMD_RESET           0XEE    
#define CMD_FOCUS_FUTHER    0XF1
#define CMD_FOCUS_CLOSER    0X1F
#define CMD_ZOOM_PLUS       0XF1 
#define CMD_ZOOM_MINUS      0X1F
#define CMD_SHIFT_UP        0X1F
#define CMD_SHIFT_DOWN      0XF1
#define CMD_SHIFT_LEFT      0X2F
#define CMD_SHIFT_RIGHT     0XF2

typedef int(*cmdFuncPtr)(uint8_t, uint8_t, void*);        //cmd callback函数指针(param: getset, paramLen, paramPtr)

/* 函数查找表结构定义 */
#pragma pack(1)
typedef struct 
{
    uint8_t     idx;
    cmdFuncPtr  func_cb; 
}cmdFuncLut;                    
#pragma pack()

/* MSTAR uart指令列表 */
typedef enum{
    MST_CMD_NONE                    = 0X00,
    MST_CMD_ACK,

    MST_CMD_POWER                   = 0X08,
    MST_CMD_MUTE,
    MST_CMD_FREEZE,
    
    MST_CMD_BACKGROUND              = 0x13,
    MST_CMD_FOCUS                   = 0X30,
    MST_CMD_ZOOM                    = 0X32,
    MST_CMD_SHIFT                   = 0X34,
    MST_CMD_LENS_LOCK               = 0X39,
    MST_CMD_BRIGHTNESS              = 0X43,
    MST_CMD_CONTRAST                = 0X44,
    MST_CMD_SHARPNESS               = 0X45,
    MST_CMD_GAMMA                   = 0X48,
    MST_CMD_IP                      = 0X52,
    MST_CMD_TESTPATTERN             = 0X60,
    MST_CMD_MODE_HI_ALT             = 0X62,

    MST_CMD_PARAM_SET               = 0xA0,    // 新协议，参数双向设置指令

    MST_CMD_LIGHT_LASER_SWITCH      = 0XA1,
    MST_CMD_LIGHT_IR_SWITCH,
    MST_CMD_LIGHT_IR_BRIGHTNESS,

    MST_CMD_INFO_MODEL              = 0XB0,
    MST_CMD_INFO_SERIAL,
    MST_CMD_INFO_FW_VERSION,
    MST_CMD_INFO_ERR,
    MST_CMD_INFO_HW_VERSION,

    /* 透传协议命令 */
    MST_CMD_TRANSPARENT             = 0XBB,    // 透传数据标识
    
    MST_CMD_FTY_WORKTIME            = 0XC1,
    MST_CMD_FTY_HSG,    

    MST_CMD_FTY_LASER_BRIGHT        = 0XCA,

    MST_CMD_MAX                     = 0xff,
}MST_CMD_;


typedef enum{
    PARAM_ID_DISPLAY_MODE = 1,                      //显示模式
    PARAM_ID_COLOR_SPACE ,                          //颜色模式 
    PARAM_ID_COLOR_TEMP,                            //色温
    PARAM_ID_LASER_EN,                              //激光开关
    PARAM_ID_LASER_BRIGHT,                          //激光亮度
    PARAM_ID_IR_EN,                                 //红外开关
    PARAM_ID_IR_BRIGHT,                             //红外亮度
    PARAM_ID_IR_ID,                                 //红外ID
    PARAM_ID_INSTALLATION_MODE,                     //安装模式anzhua
    PARAM_ID_HIGH_ALTITUDE,                         //高海拔
    PARAM_ID_INPUT_SOURCE,                          //输入源
    PARAM_ID_INPUT_COLOR_FORMAT,                    //输入颜色格式
    PARAM_ID_INPUT_COLOR_SPACE,                     //输入色域
    PARAM_ID_CONTRAST,                              //对比度
    PARAM_ID_SHARPNESS,                             //清晰度
    PARAM_ID_BRIGHTNESS,                            //背景亮度
    PARAM_ID_GAMMA,                                 //伽玛
    PARAM_ID_BACKGROUND_COLOR,                      //背景色
    PARAM_ID_LENS_LOCK,                             //镜头锁定
    PARAM_ID_ALARM_EN,                              //自检报警开关  
    PARAM_ID_ALARM_TIP,                             //告警提示开关
    PARAM_ID_NO_INPUT_POWEROFF,                     //无输入关机
    PARAM_ID_POWER_AUTO_BOOT,                       //上电自动开机
    PARAM_ID_ASPECT_RATIO,                          //宽高比
    PARAM_ID_LANGUAGE,                              //语言
    PARAM_ID_HSG_PRARM,                             //HSG颜色设置参数   
    PARAM_ID_RGB_GAIN,                              //RGB增益参数
    PARAM_ID_INDEP_COLOR,                           //独立颜色参数
    PARAM_ID_DISP_MODEL,                            //显示型号
    PARAM_ID_DISP_SN,                               //显示序列号
    PARAM_ID_DISP_DATE,                             //显示出厂日期
    PARAM_ID_LOCAL_IP_SET,                          //本机IP设置
    PARAM_ID_REPORT_IP_SET,                         //上报IP设置
    PARAM_ID_ECO_MODE_EN,                           //节能模式开关
    PARAM_ID_LASER_LEVEL,                           //激光亮度等级 R/G/B 0-1024
    PARAM_ID_MAX,
}PARAM_ID_;


int uartCmdParse(void);

/* 老版本协议接口，0XAA 包头 ----------------------------------------------------------------------------------------------*/

int appSendUartMsg(uint8_t* pData,  int dataLen, int needAck, uint8_t* ackBuf, int ackBufLen, int ackTimeout_ms);

int appSendUartRawData(uint8_t* pData,  int dataLen, int needAck, uint8_t* ackBuf, int ackBufLen, int ackTimeout_ms);
/* -------------------------------------------------------------------------------------------------------------------- */

/* 新版本协议接口，0XCC 包头 ----------------------------------------------------------------------------------------------*/

/* 异步发送函数, 发送后不等待响应, 直接返回 */
int appSendUartAsync(uint8_t cmd, uint8_t* pPayload, int payloadLen);

/* 同步发送函数, 发送后等待响应, 收到响应后返回 */
int appSendUartSync(uint8_t cmd, uint8_t* pPayload, int payloadLen, uint8_t* ackBuf, int ackBufLen, int ackTimeout_ms);

/* -------------------------------------------------------------------------------------------------------------------- */

/* 业务处理callback函数前置声明 */

int CmdFunc_IpUpdate(uint8_t get_set, uint8_t paramLen, void* param);

int CmdFunc_ParamSync(uint8_t get_set, uint8_t paramLen, void* param);

/* 透传数据处理函数前置声明 */
int CmdFunc_TransparentData(uint8_t get_set, uint8_t paramLen, void* param);

#endif // UART_PROTOCAL_H_

