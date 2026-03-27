/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "stm32f4xx_hal.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include "lwip_comm.h"
#include "lan8720.h"
#include "stmflash.h"
#include "misc.h"
#include "udp_demo.h"
#include "tcp_demo.h"
#include "error.h"
#include "ezxml.h"
#include "Motor.h"
// #include "PID.h"
#include "c381.h" //leo
#include "declare_gFunc.h"
#include "cmd_process.h"
#include "logger.h"
#include "modectl.h"
#include "rt_param.h"
#include "healthMgr.h"
#include "tcp_demo.h"
#include "pjlink.h"
#include "usart.h"
#include <fanctl.h>
#include "stm32f4xx_hal_iwdg.h"
#include "stm32f407xx.h"
#include "power_monitor.h"
#include <svc/uartCmdProc.h>
#include "it6635.h"
// #include "AD5241.h"
#include "delay3D.h"
#include "uart_protocal.h"
#include "util.h"



#if (projector_Model == LP92SLB || Projector_Model == NP72BN)
#include "it6635_import.h"
#endif

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

uint8_t Ethernet_Info[20];
char Work_Status[3] = {"0"};
char Source_Status[3] = {"0"};
char Temp_B_Status[8] = {"0"};
char Temp_B2_Status[8] = {"0"};
char Temp_G1_Status[8] = {"0"};
char Temp_G2_Status[8] = {"0"};
char Temp_R1_Status[8] = {"0"};
char Temp_R2_Status[8] = {"0"};
char Temp_E_Status[8] = {"0"};
char Temp_TEC_Status[8] = {"0"};
char Temp_DMD_Status[8] = {"0"};
char Fan2_Status[5] = {"0"};
char Fan1_Status[5] = {"0"};
char Fan1_check[80] = {"0"};
char Fan2_check[80] = {"0"};
char Tem_check[80] = {"0"};
char DLP_check[80] = {"0"};
char Mstar_check[80] = {"0"};
char Workhour_check[80] = {"0"};

// char I2C_check[80] = {"0"};
char IP_UPDATE[20] = {"0"};
char Warn_state[4] = {"1"};
uint8_t Warn_Type;
uint8_t I2C_CMD = 0;
int Work_Stat = 0; 

_Bool OE_lock;
signed char War_State;
// int Source_Stat = 0;
VIDEO_INPUT_SRC_ Source_Stat = VIDEO_INPUT_SRC_HDMI_1;
_Bool video_src_changed = TRUE;
_Bool it66352Init = FALSE;

uint16_t OE_movement;
// _Bool Sys_start = 0;
uint32_t secondCounter = 0;
// uint8_t FAN_DC = 0x60;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern struct ip_addr rmtipaddr;
// extern uint8_t udp_flag;
// extern char udp_recvbuf[20];
extern uint8_t FAN_Temp_Error[7];
extern char TCP_recvbuf[20];
extern uint8_t TCP_data_len;
// extern int XML;
extern struct netif lwip_netif;
extern uint8_t EthInitStatus;

uint8_t waitMstAlive = 0;
uint32_t waitMstStartTime = 0 ;

uint8_t mstar_work_on_oneshot_flag = 0;

IWDG_HandleTypeDef iwdg;


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SYSTEMTICK_PERIOD_MS 10

#define FLASH_LATENCY_1 ((uint8_t)0x0001)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// __IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint8_t i2cRxBuffer[2];

IAP_UPDATE_PARAM iapParam;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// void it6635_options_init(void)  //引入头文件存在类型问题，直接前置声明

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


#define HEARTBEAT_BLINK()   HAL_GPIO_TogglePin(MCU_LED_GPIO_Port, MCU_LED_Pin)

const char* ProjectorModelName[] = {
    "NoneModel",
    "LP72BSL;NP52SLC;NP52SL;",
    "LP92SLB",
};

void print_fw_info(void)
{
    printf("\
*********************************************************************\n\
* (C) COPYRIGHT 2023 AVIC(GuoHua) Co.,Ltd. All rights reserved.     *\n\
* Application for projector main Control on stm32f407zet6           *\n\
* Author: Primo Wu                                                  *\n\
* Version: %d.%d.%d                                                    *\n\
* Compiled on %s at %s                               *\n\
*********************************************************************\n\
",VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, __DATE__, __TIME__);

    printf("Projector Model: %s\n", ProjectorModelName[Projector_Model]);
}


void ResetIP(void) // leo 20200731
{
  Ethernet_Info[0] = IP_ADDR0;
  Ethernet_Info[1] = IP_ADDR1;
  Ethernet_Info[2] = IP_ADDR2;
  Ethernet_Info[3] = IP_ADDR3;

  Ethernet_Info[4] = NETMASK_ADDR0;
  Ethernet_Info[5] = NETMASK_ADDR1;
  Ethernet_Info[6] = NETMASK_ADDR2;
  Ethernet_Info[7] = NETMASK_ADDR3;

  Ethernet_Info[8] = GW_ADDR0;
  Ethernet_Info[9] = GW_ADDR1;
  Ethernet_Info[10] = GW_ADDR2;
  Ethernet_Info[11] = GW_ADDR3;

  Ethernet_Info[12] = RemoteIP0;
  Ethernet_Info[13] = RemoteIP1;
  Ethernet_Info[14] = RemoteIP2;
  Ethernet_Info[15] = RemoteIP3;

  Ethernet_Info[16] = UDP_DEMO_PORT / 100;
  Ethernet_Info[17] = UDP_DEMO_PORT % 100;
}

/*
    函数功能：给指定网卡更新ip 掩码 网关 （不需要更新的还是原先数值就好）
    函数参数：
        netif：需要更新这些的网卡
        ip：需要更新的ip
        mask：更新的掩码
        gw：更新的网关
    返回值：无
    注释：本人亲测可以使用，代码成功
 
*/
void ip_setting_update(uint8_t* ip, uint8_t* mask, uint8_t* gw)
{
    ip_addr_t ip_update;
    ip_addr_t mask_update;
    ip_addr_t gw_update;
 
    //转换
    IP4_ADDR(&ip_update, ip[0], ip[1], ip[2], ip[3]);
    IP4_ADDR(&mask_update, mask[0], mask[1], mask[2], mask[3]);
    IP4_ADDR(&gw_update, gw[0], gw[1], gw[2], gw[3]);
 
    //禁用网卡
    netif_set_down(&lwip_netif);
    
    //1.分别设置
    netif_set_gw(&lwip_netif, &ip_update);        //重新设置网关地址
    netif_set_netmask(&lwip_netif, &mask_update); //重新设置子网掩码
    netif_set_ipaddr(&lwip_netif, &gw_update);    //重新设置IP地址
 
    //重新使能网卡
    netif_set_up(&lwip_netif);

    printf("IP=%d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
    printf("NetMask=%d.%d.%d.%d\r\n", mask[0], mask[1], mask[2], mask[3]);
    printf("GateWay=%d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
    return ;
}


void lwipIPUpdate(void) // leo 20200731
{
  struct ip_addr ipaddr, netmask, gw;
  lwip_comm_default_ip_set(&lwipdev);
  IP4_ADDR(&ipaddr, lwipdev.ip[0], lwipdev.ip[1], lwipdev.ip[2], lwipdev.ip[3]);
  IP4_ADDR(&netmask, lwipdev.netmask[0], lwipdev.netmask[1], lwipdev.netmask[2], lwipdev.netmask[3]);
  IP4_ADDR(&gw, lwipdev.gateway[0], lwipdev.gateway[1], lwipdev.gateway[2], lwipdev.gateway[3]);
  printf("MAC=%02x.%02x.%02x.%02x.%02x.%02x\r\n", lwipdev.mac[0], lwipdev.mac[1], lwipdev.mac[2], lwipdev.mac[3], lwipdev.mac[4], lwipdev.mac[5]);
  printf("IP=%d.%d.%d.%d\r\n", lwipdev.ip[0], lwipdev.ip[1], lwipdev.ip[2], lwipdev.ip[3]);
  printf("NetMask=%d.%d.%d.%d\r\n", lwipdev.netmask[0], lwipdev.netmask[1], lwipdev.netmask[2], lwipdev.netmask[3]);
  printf("GateWay=%d.%d.%d.%d\r\n", lwipdev.gateway[0], lwipdev.gateway[1], lwipdev.gateway[2], lwipdev.gateway[3]);
  printf("RemoteIP=%d.%d.%d.%d\r\n", lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3]);
  printf("UDPPort=%d\r\n", (lwipdev.UDP_Port[0] * 100 + lwipdev.UDP_Port[1]));

  if (LINK_UP == 0x10)
  {
    udp_demo_connection_close(udppcb);
    //closePJLink();
    netif_set_down(&lwip_netif); // down
    LINK_UP = 0x12;
    netif_set_addr(&lwip_netif, &ipaddr, &netmask, &gw);
    netif_set_default(&lwip_netif);
    netif_set_up(&lwip_netif);
  }
  else
    LINK_UP = 0x14;

  // udp_demo_init();
}

/**
 * @brief 设置IP配置,拆分成二进制数存储进flash,并更新ip
 * @param ip IP地址
 * @param netmask 掩码
 * @param gw 网关
 * @return 0:成功, -1:失败
 */
int set_ip_config(char* ip, char* netmask, char* gw)
{
    uint8_t ip_temp[12] = {0} ;
    memcpy(ip_temp, Ethernet_Info, 12); // 备份原始IP配置

    char buf[16]; // 临时缓冲区，避免破坏原始字符串

    // 只更新传入的参数，其他参数保持不变
    if(ip != NULL) {
        strcpy(buf, ip);
        ip_temp[0] = atoi(strtok(buf, "."));
        ip_temp[1] = atoi(strtok(NULL, "."));
        ip_temp[2] = atoi(strtok(NULL, "."));
        ip_temp[3] = atoi(strtok(NULL, "."));
    }
    if(netmask != NULL){    
        strcpy(buf, netmask);
        ip_temp[4] = atoi(strtok(buf, "."));
        ip_temp[5] = atoi(strtok(NULL, "."));
        ip_temp[6] = atoi(strtok(NULL, "."));
        ip_temp[7] = atoi(strtok(NULL, "."));

    }
    if(gw != NULL){
        strcpy(buf, gw);
        ip_temp[8] = atoi(strtok(buf, "."));
        ip_temp[9] = atoi(strtok(NULL, "."));
        ip_temp[10] = atoi(strtok(NULL, "."));
        ip_temp[11] = atoi(strtok(NULL, "."));
    }
    memcpy(Ethernet_Info, ip_temp, 12);
    memcpy(rt_param.ip_addr, ip_temp, 12);
    save_rt_param_to_flash();

    STMFLASH_Write(ADDR_FLASH_SECTOR_6, (uint32_t *)Ethernet_Info, 5);
    lwipIPUpdate();
    return 0 ;
}

/**
 * @brief 获取IP配置
 * @param ip IP地址
 * @param netmask 掩码
 * @param gw 网关
 * @return 0:成功, -1:失败
 */
int get_ip_config(char* ip, char* netmask, char* gw)
{
    sprintf(ip, "%d.%d.%d.%d", Ethernet_Info[0], Ethernet_Info[1], Ethernet_Info[2], Ethernet_Info[3]);
    sprintf(netmask, "%d.%d.%d.%d", Ethernet_Info[4], Ethernet_Info[5], Ethernet_Info[6], Ethernet_Info[7]);
    sprintf(gw, "%d.%d.%d.%d", Ethernet_Info[8], Ethernet_Info[9], Ethernet_Info[10], Ethernet_Info[11]);
    return 0;
}


/**
 * @brief 设置上报目标地址IP设置
 * @param ip 上报目标地址IP
 * @param port 上报目标地址PORT
 * @return 0:成功, -1:失败
 * @return 
 */
int set_report_ip(char* ip, int port)
{
    char buf[16]; // 临时缓冲区，避免破坏原始字符串
    strcpy(buf, ip);
    Ethernet_Info[12] = atoi(strtok(buf, "."));
    Ethernet_Info[13] = atoi(strtok(NULL, "."));
    Ethernet_Info[14] = atoi(strtok(NULL, "."));
    Ethernet_Info[15] = atoi(strtok(NULL, "."));
    Ethernet_Info[16] = port / 100;
    Ethernet_Info[17] = port % 100;

	lwipdev.remoteip[0] = Ethernet_Info[12];
    lwipdev.remoteip[1] = Ethernet_Info[13];
    lwipdev.remoteip[2] = Ethernet_Info[14];
    lwipdev.remoteip[3] = Ethernet_Info[15];

    lwipdev.UDP_Port[0] = Ethernet_Info[16];
    lwipdev.UDP_Port[1] = Ethernet_Info[17];
	
    
    rt_param.report_ip[0] = Ethernet_Info[12];
    rt_param.report_ip[1] = Ethernet_Info[13];
    rt_param.report_ip[2] = Ethernet_Info[14];
    rt_param.report_ip[3] = Ethernet_Info[15];
    rt_param.report_port = Ethernet_Info[16] * 100 + Ethernet_Info[17];
    save_rt_param_to_flash();


	IP4_ADDR(&rmtipaddr, lwipdev.remoteip[0], lwipdev.remoteip[1], lwipdev.remoteip[2], lwipdev.remoteip[3]);
	rmtport = lwipdev.UDP_Port[0] * 100 + lwipdev.UDP_Port[1];

    STMFLASH_Write(ADDR_FLASH_SECTOR_6, (uint32_t *)Ethernet_Info, 5); // 更新flash
	return 0;
}


/**
 * @brief 获取上报目标地址IP设置
 * @param ip 上报目标地址IP
 * @param port 上报目标地址PORT
 * @return 0:成功, -1:失败
 */
int get_report_ip(char* ip, int* port){
    sprintf(ip, "%d.%d.%d.%d", Ethernet_Info[12], Ethernet_Info[13], Ethernet_Info[14], Ethernet_Info[15]);
    *port = Ethernet_Info[16] * 100 + Ethernet_Info[17];
    return 0;
}



/**
 * @brief 开机运行时一次性同步信息到Mstar, 防止Mstar开机时，信息未同步
 * 
 */
void mstar_workon_info_sync_oneshot(void)
{
    log_info("hw_version Info sync --> Mstar");
    /**同步板卡硬件版本 */
    uint8_t msg_data[10] = {MST_CMD_INFO_HW_VERSION, CMDTYPE_SET, read_hw_version(), 0, 0, 0, 0, 0, 0, 0};
    appSendUartMsg(msg_data, 10, false, NULL, 0, 0);    

    HAL_Delay(100);

    /** 同步 所有参数信息 */
    rt_param_sync_to_mstar();
    log_info("All params Info sync --> Mstar");
    return ;
}

void loop_task_100ms()
{
    /* 1. 移轴马达控制 100ms 检测周期*/
    if (!OE_lock && motor_nsleep && Work_Stat) // motor movement
    {
        // calculate shift positioin
        {
            OE_Shift_calculate();
            OE_Shift_AVE();
        }
        if (OE_Shift_Type == OE_Shift_RST)
        {
            OE_Shift_Reset();
            //				 printf ("OE_Shift_Reset\r\n");
        }
        else
        {
            OE_Shift_limit();
            OE_Shift_Handler();

            if (OE_timer >= OE_timer_cnt && OE_Shift_Type) // 1s //motor stop without IR cmd
            {
                OE_timer = 0;
                if (OE_movement == 0)
                {
                    if (OE_Shift_Type == OE_Shift_UD)
                        OE_ShiftUD_stop();
                    else if (OE_Shift_Type == OE_Shift_LR)
                        OE_ShiftLR_stop();
                    else if (OE_Shift_Type == OE_Zoom)
                        OE_Zoom_stop();
                    else if (OE_Shift_Type == OE_Focus)
                        OE_Focus_stop();
                    OE_Value_clean();
                    printf("OE_stop\r\n");
                }
                else
                    motor_FZ++;

                OE_movement = 0;
            }
            else
                OE_timer++;
        }
    }

}

void loop_task_500ms()
{
    /* 1. 心跳灯 500ms 周期*/
    HEARTBEAT_BLINK();

    /* 2. 网络插入状态检测 500ms 周期*/
    if (LINK_UP != 0)
        Eth_Link_ITHandler(LAN8720_PHY_ADDRESS);

    /* 3. 异步方式，数据保存flash */
    do_save_rt_param_to_flash();
}

void loop_task_1000ms()
{
    /* 1. Mstar 开机后，仅执行一次的任务 */
    if(mstar_work_on_oneshot_flag == 1)
    {
        mstar_work_on_oneshot_flag = 0;
        
        // 1.1: 同步信息到Mstar
        mstar_workon_info_sync_oneshot();

        // 1.2: mstar 开机后，配置3D延时芯片
    #if (Projector_Model == LP72BSL_NP52SLC_NP52SL)
        if(is_3d_delay_supported())
            init_delay3D();
        else
            log_warn("3D delay function not supported on current hardware version\n");
    #endif

    }
    
    /* 2. 温度检测， 风扇PWM调整，温度异常检测，风扇异常检测 （5s 周期） */
    if ((secondCounter % 5) == 0)
    {
        if (Work_Stat) // 上电工作状态，i2c 命令正确
        {
            int fan_err = 0;
            int temp_err = 0;
            // 检测风扇是否被锁定
            for (size_t i = 1; i <= 8; i++)
            {
                Fan_LOCKED_check(i);
            }
            fan_err = Fan_Error_warn();
            // 获取温度
            Get_Temperature();
            // 检测温度是否异常
            temp_err = Tempture_Erorr_Test();
            // 根据温度调整风扇PWM
            fan_pwm_auto_set(ADS_Temp_Data, sizeof(ADS_Temp_Data) / sizeof(float));
            // 判断是否错误, 存在错误发送，则调用PJLINK发送错误状态
            if (fan_err || temp_err)
            {
                PJLink_StatusNotify_ErrorStatus(fan_err, temp_err, 0, 0, 0, 0);
            }
        }
    }

    /* 3. 周期XML上报 Time_UP 周期） */
    if ((secondCounter % rt_param.health_report_period) == 0 ) // //30s XML report
    {
        if (LINK_UP == 0x10 && rt_param.health_report_en)
        {
            if(Work_Stat == 0 && rt_param.xml_standby_report_en == 0){
                ; // DO NOTHING, 待机状态下不进行XML上报
            }
            else{
                if (rt_param.health_report_format)
                    StateReportTask(STATE_REPORT_MODE_ACTIVE, STATE_REPORT_TYPE_XML);
                else
                    StateReportTask(STATE_REPORT_MODE_ACTIVE, STATE_REPORT_TYPE_BYTE);
            }

        }
    }

    /* 4. 网络开机时等待mstar启动保活机制 */
    if(waitMstAlive)
    {
        //记录开始时间
        if(waitMstStartTime == 0 )                  
            waitMstStartTime = secondCounter;
        int timeClap = secondCounter - waitMstStartTime ;
        printf("waiting Mstar ready time: %ds\n",timeClap);
        
        if(Work_Stat == 1 )     //检测到开机
        {
            printf("Mstar ready at %d s.\n", timeClap);
            waitMstAlive = 0;
            waitMstStartTime = 0;  
        }
        else                    //没有检测到开机
        {
            if(timeClap >  45)  // 6s 开机 + 10秒wathdog超时时间，重新掉电，MSTAR进入待机状态， 再预留4秒后，发送重新开机指令
            { 
                printf("Mstar not ready, reboot mstar.\n");
                HAL_UART_Transmit(&huart1, (uint8_t *)"{PWR}", 5, 5000);  // 重新发送开机命令
                //记录情况
                rt_param.debug_area[0] ++;
                save_rt_param_to_flash();
                waitMstStartTime = 0;                                     // 清理开始时间
                waitMstAlive = 0 ;      //每收到一条开机命令，任务只执行一次
            }
        }
    }

    /* 5. 秒计数器递增 */
    power_monitor_tick();
    secondCounter++;
    return ;
}



/* USER CODE END 0 */

/******************************************************************************/
/*                                                                            */
/*    The application entry point                                             */
/*                                                                            */
/******************************************************************************/
int main(void)
{
    /* USER CODE BEGIN 1 */
    
    // Flash中断向量表重定位+使能全局中断
    RESET_IRQ_VEC_TAB_BASE(); 
    __enable_irq(); 

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();

    // 优先配置uart，用于打印系统信息=============================================
    MX_USART1_UART_Init();
    MX_USART3_UART_Init();

#if (Projector_Model == LP72BSL_NP52SLC_NP52SL)
    // 读取硬件版本
    read_hw_version();
#endif

    // 打印软件版本信息
    print_fw_info();

    // 读取Flash配置*************************************************************
    printf("***********************************************\n");
    printf("READ CONFIG FROM FLASH.\n");

    // 读取IAP更新参数
    STMFLASH_Read(FLASH_IAP_PARAM_ADDR, (uint32_t *)(&iapParam), sizeof(iapParam) / 4);
    if(iapParam.iapUpdateFlag != 0 ) //若不为0，则Clear Flag（包含升级完成，以及第一次烧录两种场景）; 
    {
        printf("IAP Update success! Clear Flag\n");
        iapParam.iapUpdateFlag = 0;
        STMFLASH_Write_With_Erase(FLASH_IAP_PARAM_ADDR, (uint32_t*)(&iapParam), sizeof(iapParam) / 4);  //更新数据
    }

    // 读取网络配置
    STMFLASH_Read(ADDR_FLASH_SECTOR_6, (uint32_t *)Ethernet_Info, 9);
    if ((Ethernet_Info[1] == 0xFF) && (Ethernet_Info[3] == 0xFF) && (Ethernet_Info[9] == 0xFF) &&
        (Ethernet_Info[11] == 0xFF) && (Ethernet_Info[13] == 0xFF) && (Ethernet_Info[15] == 0xFF) &&
        (Ethernet_Info[16] == 0xFF) && (Ethernet_Info[17] == 0xFF))
    {
        ResetIP(); // leo 20211221
        STMFLASH_Write(ADDR_FLASH_SECTOR_6, (uint32_t *)Ethernet_Info, 9);
        printf("Default_IP\r\n");
    }
    // 读取其他配置
    read_rt_param_from_flash();

    printf("***********************************************\n");

    // 兼容不同型号的马达，独立函数初始化Timer参数。
    initTimerPeroid();

// 继续配置其他外设========================================================
#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
    MX_SPI2_Init();
    MX_TIM9_Init();
#endif

    MX_DMA_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();
    MX_TIM1_Init();
    MX_TIM4_Init();

    MX_TIM3_Init();
    MX_TIM2_Init();
    MX_TIM6_Init();
    /* USER CODE BEGIN 2  rg*/
    Timer_Base_enable();
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_ConvertedValue, ADC_NUMOFCHANNEL);
    HAL_TIM_Base_Start_IT(&htim6);
    Timer_Pwm_enable();

    // 外设初始化完成，初始化工作模式，风扇开始运转
    modectl_init_mode(rt_param.work_mode);

    #if 0 
    // MCU与MSATR uart通信接收中断开启
    //  if((HAL_UART_Receive_IT(&huart1, &rx_temp, 1)) == HAL_OK)
    if ((HAL_UART_Receive_IT(&huart1, (uint8_t *)g_uart1_rx_buf, 18)) == HAL_OK) 
        printf("Ethernet<->Mstar\n");

    // 开启UART3 接收中断
    HAL_UART_Receive_IT(&huart3, (uint8_t *)uart3_rx_buf, 1);
    #endif

    // 开启I2C1接收中断
    HAL_I2C_Slave_Receive_IT(&hi2c1, (uint8_t *)i2cRxBuffer, 2);
     
    // 设置iwatch dog 定时器, 超时周期 8s
    // 公式： rlr = 4095(max), iwdgclk = 32Khz(def), prer = 64 (selfdef)
    // 公式： Tout = (prer x rlr ) / iwdgclk = 8 s
    iwdg.Instance = IWDG;
    iwdg.Init.Prescaler = IWDG_PRESCALER_128;
    iwdg.Init.Reload = 4095;
    HAL_IWDG_Init(&iwdg);
    log_info("IWDG_Init done, Tout = 8s");

#define MSTCMD_CHECK_STATE "XXX" /* 检查Mstar工作状态指令 */
    // 发送检查状态指令，保证若MCU在运行时通过watchDog自动复位，可以继续获取到正确的状态
    HAL_UART_Transmit(&huart1, MSTCMD_CHECK_STATE, strlen(MSTCMD_CHECK_STATE), 1000);


    /* USER CODE END 2 */
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    // 主循环=================================================================
    log_info("Enter main loop");
    while (1)
    {
        // Round-robin + ISR架构

        /* 100ms 周期任务处理段 */
        if (loopFlag_100ms)
        {
            loopFlag_100ms = 0;
            loop_task_100ms();
        }
       
        /* 500ms 周期任务处理段 */
        if (loopFlag_500ms)
        {
            loopFlag_500ms = 0;
            loop_task_500ms();
        }

        /* 1000ms 周期任务处理段 */
        if (loopFlag_1000ms)
        {
            loopFlag_1000ms = 0;
            loop_task_1000ms();

            // 喂狗
            HAL_IWDG_Refresh(&iwdg);
        }

        // lwip周期任务，ARP清理，DHCP处理等
        lwip_periodic_handle();
        // I2C指令包处理
        I2C_CMD_Process();
        // 网络指令包处理
        Netif_CMD_Process();
#ifdef USING_PJLINK
        PJLinkThread(); // 测试，处理PJLink指令
#endif

        uartCmdProcTask();
    
        if (Work_Stat == 1) // Mstar Ready,外设已供电,再进行IT66352 和 C381 控制
        {     
            /* 循环等待报警信息上报指令 */
            Report_Warning();
#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
            /* C381 Loop */
            stwp();
            // it66352控制=================================================
            // it66352上电由mstar控制，每次机器软重启都需要重新初始化该芯片；
            if(FALSE == it66352Init)
            {
                HAL_GPIO_WritePin(ITE_RST_GPIO_Port, ITE_RST_Pin, GPIO_PIN_SET);
                it6635_options_init();
                printf("IT6635 init\n");
                it66352Init = TRUE;
            }
            if (video_src_changed) // 视频源切换,切换HDMI输入端口
            {
                video_src_changed = FALSE;
                /* 为与OSD界面显示的HDMI通道映射关系保持一致，对应HDMI1和HDMI2的port互换 */
                if (Source_Stat == VIDEO_INPUT_SRC_HDMI_1)
                {
                    printf("Change HDMI port: 1\n");
                    it6635_set_active_port(2); // 芯片HDMI2端口,对应OSD显示HDMI1端口
                }
                else if (Source_Stat == VIDEO_INPUT_SRC_HDMI_2)
                {
                    printf("Change HDMI port: 2\n");
                    it6635_set_active_port(1); // 芯片HDMI1端口,对应OSD显示HDMI2端口
                }
                else // 输入不是HDMI,则active port 设置为0
                {
                    it6635_set_active_port(0);
                }
            }
            if (loopFlag_10ms)
            {
                loopFlag_10ms = 0;
                it6635_dev_loop();
            }
#endif
        }
        /* USER CODE END WHILE */
    }
    /* USER CODE BEGIN 3 */
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  printf("Error_Handlers\r\n");
  //  __disable_irq();
  //  while (1)
  //  {
  //  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
