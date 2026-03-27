#include "cmd_process.h"
#include "main.h"
#include "udp_demo.h"
#include "stmflash.h"
#include "usart.h"
#include "error.h"
#include <string.h>
#include "logger.h"
#include "tim.h"
#include "fanctl.h"
#include "c381.h"
#include "set.h"
#include "adc.h" 
#include "rt_param.h"
#include "it6635_import.h"
#include "lan8720.h"
#include "iap.h"


#include "hal/uart_protocal.h"
#include "libs/cJSON/cJSON.h"
#include "hal/netRpcCall.h"
#include "drivers/inc/AD5241.h"

//  #define NET_CMD_POWER_UP "PWR"
//  #define NET_CMD_POWER_DOWN "PWO"


// clang-format off
/* MCU <--> MSTAR ����궨�� */
/* display mode */
#define MSTCMD_DISP_MODE_2D_NORMAL        "UAm"    /* ��ʾģʽ 2D��ͨ    */
#define MSTCMD_DISP_MODE_3D_SYNC_INT      "UBm"    /* ��ʾģʽ 3D�ڲ�ͬ��*/
#define MSTCMD_DISP_MODE_3D_SYNC_EXT      "UCm"    /* ��ʾģʽ 3D�ⲿͬ��*/
#define MSTCMD_DISP_MODE_2D_LOW_DRAG      "UDm"    /* ��ʾģʽ 2D����Ӱ  */
#define MSTCMD_DISP_MODE_2D_LOW_LAT       "UEm"    /* ��ʾģʽ 2D���ӳ�  */

/* Low drag mode */
#define MSTCMD_LOW_DRG_MODE_0             "UG0"    /* ����Ӱģʽģʽ 0  45ms    �ӳ� */
#define MSTCMD_LOW_DRG_MODE_1             "UG1"    /* ����Ӱģʽģʽ 1  15-30ms �ӳ� ���ź����������ϲ 50m DP�ߴ�����˸���⣩*/
#define MSTCMD_LOW_DRG_MODE_2             "UG2"    /* ����Ӱģʽģʽ 2  27ms    �ӳ�*/

/* IR remote ID */
#define MSTCMD_IR_ID_0                    "UM0"    /* ң����ID 0 */
#define MSTCMD_IR_ID_1                    "UM1"    /* ң����ID 1 */
#define MSTCMD_IR_ID_2                    "UM2"    /* ң����ID 2 */
#define MSTCMD_IR_ID_3                    "UM3"    /* ң����ID 3 */
#define MSTCMD_IR_ID_4                    "UM4"    /* ң����ID 4 */
#define MSTCMD_IR_ID_5                    "UM5"    /* ң����ID 5 */
#define MSTCMD_IR_ID_6                    "UM6"    /* ң����ID 6 */
#define MSTCMD_IR_ID_7                    "UM7"    /* ң����ID 7 */
#define MSTCMD_IR_ID_8                    "UM8"    /* ң����ID 8 */
#define MSTCMD_IR_ID_9                    "UM9"    /* ң����ID 9 */

/* IR remote code */
#define MSTCMD_IRREMOTE_0                 "UL0"    /* ң���� 0 */
#define MSTCMD_IRREMOTE_1                 "UL1"    /* ң���� 1 */
#define MSTCMD_IRREMOTE_2                 "UL2"    /* ң���� 2 */
#define MSTCMD_IRREMOTE_3                 "UL3"    /* ң���� 3 */
#define MSTCMD_IRREMOTE_4                 "UL4"    /* ң���� 4 */
#define MSTCMD_IRREMOTE_5                 "UL5"    /* ң���� 5 */
#define MSTCMD_IRREMOTE_6                 "UL6"    /* ң���� 6 */
#define MSTCMD_IRREMOTE_7                 "UL7"    /* ң���� 7 */
#define MSTCMD_IRREMOTE_8                 "UL8"    /* ң���� 8 */
#define MSTCMD_IRREMOTE_9                 "UL9"    /* ң���� 9 */

/* Color Manager Mode  */
#define MSTCMD_CM_FS                      "UI0"    /* Flight Simulation ��ɫģʽ */
#define MSTCMD_CM_REC709                  "UI1"    /* REC709 ��ɫģʽ */
#define MSTCMD_CM_DCIP3                   "UI2"    /* DCI-P3 ��ɫģʽ */
#define MSTCMD_CM_BT2020                  "UI3"    /* BT2020 ��ɫģʽ */ 

/* Low drag intensity */
#define MSTCMD_LOW_DRG_INT_0              "UJ0"    /* ����Ӱǿ��1  */ 
#define MSTCMD_LOW_DRG_INT_1              "UJ1"    /* ����Ӱǿ��2 */
#define MSTCMD_LOW_DRG_INT_2              "UJ2"    /* ����Ӱǿ��3 */

/* Input source */
#define MSTCMD_SRC_VGA                    "UK0"    /* VGA? */    
#define MSTCMD_SRC_HDMI                   "UK1"    /* HDMI? */
#define MSTCMD_SRC_HDMI2                  "UK2"    /* HDMI2? */
#define MSTCMD_SRC_DVI                    "UK3"    /* DVI? */
#define MSTCMD_SRC_DP                     "UK4"    /* DP? */ 

#define MSTCMD_RESOLUTION_2K              "UO0"    
#define MSTCMD_RESOLUTION_4K              "UO1"

#define MSTCMD_LOW_LAT_OFF                "UQ0"
#define MSTCMD_LOW_LAT_ON                 "UQ1"

#define MSTCMD_LOW_DRAG_OFF               "UR0"
#define MSTCMD_LOW_DRAG_ON                "UR1"

#define MSTCMD_PROGRAM_MODE               "UP1"    



#define MSTCMD_HEADER  ( 6 << 5)

/* ���ܺ����� */
typedef enum
{
    FUNC_CODE_NONE = 0,
    FUNC_CODE_HSG,            /* HSG���� */
    FUNC_CODE_LASER_RGB,      /*��������ɫ������*/
    FUNC_CODE_MAX,    
}FUNC_CODE_;

/* ��ɫ����ö�� */
typedef enum
{
    COLOR_ALL = 0,    /* ������ɫ */
    COLOR_R,      
    COLOR_G,
    COLOR_B,
    COLOR_C,
    COLOR_M,
    COLOR_Y,
    COLOR_W,
}COLOR_;

/* ������ */
typedef enum
{
    OPS_NONE = 0,
    OPS_RESET,        /* ����   */
    OPS_SET_VAL,      /* ����ֵ */
    OPS_GET_VAL,      /* ��ȡֵ */
    OPS_VAL_ADD,      /* ����ֵ */
    OPS_VAL_SUB,      /* ����ֵ */
}OPS_;

/* ��ɫ���� */
typedef enum
{
    COLOR_ATTR_NONE = 0,
    COLOR_ATTR_HUE,             /* ɫ��   */        // ����Ϊw��r����
    COLOR_ATTR_SATURATION,      /* ���Ͷ� */        // ����Ϊw��g����
    COLOR_ATTR_GAIN,            /* ����   */        // ����Ϊw��b����
}COLOR_ATTR_;

/* MSTAR ������Ϣ���ṹ */
#pragma pack(1)
typedef struct {
  u8_t header;          /* header, ��sizeof��MST_CMDMSG�� <<����  */
  u8_t func_code;       /*������*/
  u8_t ops;             /*������*/
  u8_t color;           /*��ɫ����*/
  u8_t attr;            /*����*/
  u16_t value;          /*ֵ */
}MST_CMDMSG; 
#pragma pack()

MST_CMDMSG mstCmdMsg= {
  .header = MSTCMD_HEADER,
}; // ��ʼ�����ݰ�


//����ǰ������
void it6635_dev_restart(void);

#define MAX_SUBSTRINGS 8 // ���������8�����ַ���  
#define MAX_LENGTH 16    // ����ÿ�����ַ�������󳤶�Ϊ1000  
  
// clang-format on


char substrings[MAX_SUBSTRINGS][MAX_LENGTH]; // �洢���ַ�����ȫ������  
int num_substrings = 0; // �Ѵ洢�����ַ�������  
  

void cJSON_RPC_CommandParse(char* cmdStr);

void splitString(char *str) {  
    // �Ƴ��ַ���ĩβ��\r\n��\n  ������У�
    size_t len = strlen(str);  
    if (len >= 2 && str[len - 2] == '\r' && str[len - 1] == '\n') {  
        str[len - 2] = '\0'; // �Ƴ�\r\n  
    } else if (len > 0 && str[len - 1] == '\n') {  
        str[len - 1] = '\0'; // �Ƴ�\n  
    }   
    
    num_substrings = 0; 
    char *token = strtok(str, ".");  
    while (token != NULL) {  
        // ���Ʒָ�����ַ�����ȫ��������  
        strcpy(substrings[num_substrings], token);  
        printf("Debug: '%s'\n", token); // ��ӡ������Ϣ  
          
        // �Ƶ���һ�����ַ���  
        token = strtok(NULL, ".");  
        num_substrings++;  
    }  
}  


/**
 * @brief �����ַ�����ʽ��������ָ���������
 * 
 * @param lanCmd  ����ָ�������ָ��
 * @return int  
 */
int lanCmdNewParser(char* lanCmd)
{   
  if(0 == memcmp(lanCmd, "sys.setting.reset",17))            /* ����ϵͳ��������ָ�� */
  {
    HAL_UART_Transmit(&huart1, (uint8_t *)"Uys", 3, 5000);   /* uartMessage settings reset */
  }
  else if(0 == memcmp(lanCmd, "sys.dlp.reset",13))           /* ����dlp����ָ�� */
  {
    HAL_UART_Transmit(&huart1, (uint8_t *)"Uwr", 3, 5000);   /* uartMessage dlp reset  */
  }
  else if(0 == memcmp(lanCmd, "sys.state", 13))
  {
    // do 
  }
  else if (0 == memcmp(lanCmd, "sys.iap.update", 14))
  {
      iapParam.iapUpdateFlag = 1 ; 
      iapParam.remoteIp[0] = (dst_udpIP.addr & 0xff) ;
      iapParam.remoteIp[1] = ((dst_udpIP.addr >> 8) & 0xff);
      iapParam.remoteIp[2] = ((dst_udpIP.addr >> 16) & 0xff);
      iapParam.remoteIp[3] = ((dst_udpIP.addr >> 24) & 0xff);
      iapParam.remotePort = dst_udpport;
      STMFLASH_Write_With_Erase(FLASH_IAP_PARAM_ADDR, (uint32_t*)(&iapParam), sizeof(iapParam) / 4);
      log_info("IAP update ip: %d.%d.%d.%d:%d", iapParam.remoteIp[0], iapParam.remoteIp[1], iapParam.remoteIp[2], iapParam.remoteIp[3], iapParam.remotePort);
      log_info("IAP update start!, Jump to Bootloader...");
      // HAL_NVIC_DisableIRQ(); // �ر������ж�
      HAL_NVIC_SystemReset(); // ����bootloader
  }
  else if (0 == memcmp(lanCmd, "sys.boot.update", 15))
  {
      iap_boot_update_loop();
  }
  else if(0 == memcmp(lanCmd,"sys.disp_mode.",14))         /* ��ʾģʽ�������� */
  {
    uint8_t* mode_ptr = (uint8_t*)&lanCmd[14];
    if(0 == memcmp(mode_ptr, "2d.normal",9))                              /* 2D ��ͨ      */
      HAL_UART_Transmit(&huart1, MSTCMD_DISP_MODE_2D_NORMAL, strlen(MSTCMD_DISP_MODE_2D_NORMAL), 1000);  
    else if(0 == memcmp(mode_ptr, "2d.low_lat",10))                       /* 2D ���ӳ�    */
      HAL_UART_Transmit(&huart1, MSTCMD_DISP_MODE_2D_LOW_LAT, strlen(MSTCMD_DISP_MODE_2D_LOW_LAT), 1000); 
    else if(0 == memcmp(mode_ptr, "2d.low_drag",11))                      /* 2D ����Ӱ    */
      HAL_UART_Transmit(&huart1, MSTCMD_DISP_MODE_2D_LOW_DRAG, strlen(MSTCMD_DISP_MODE_2D_LOW_DRAG), 1000);
    else if(0 == memcmp(mode_ptr, "3d.sync_int",11))                      /* 3D �ڲ�ͬ��  */ 
      HAL_UART_Transmit(&huart1, MSTCMD_DISP_MODE_3D_SYNC_INT, strlen(MSTCMD_DISP_MODE_3D_SYNC_INT), 1000);
    else if(0 == memcmp(mode_ptr, "3d.sync_ext",11))                      /* 3D �ⲿͬ��  */
      HAL_UART_Transmit(&huart1, MSTCMD_DISP_MODE_3D_SYNC_EXT, strlen(MSTCMD_DISP_MODE_3D_SYNC_EXT), 1000);

  }
  else if(0 == memcmp(lanCmd,"sys.ir_id.",10))         /* ģ��ң����ָ�� */
  {
      uint8_t* key = (uint8_t*)&lanCmd[10];
      if(0 == memcmp(key, "0",1))
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_0, strlen(MSTCMD_IR_ID_0), 1000);
      else if(0 == memcmp(key, "1",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_1, strlen(MSTCMD_IR_ID_1), 1000);
      else if(0 == memcmp(key, "2",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_2, strlen(MSTCMD_IR_ID_2), 1000);
      else if(0 == memcmp(key, "3",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_3, strlen(MSTCMD_IR_ID_3), 1000);
      else if(0 == memcmp(key, "4",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_4, strlen(MSTCMD_IR_ID_4), 1000);
      else if(0 == memcmp(key, "5",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_5, strlen(MSTCMD_IR_ID_5), 1000);
      else if(0 == memcmp(key, "6",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_6, strlen(MSTCMD_IR_ID_6), 1000);
      else if(0 == memcmp(key, "7",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_7, strlen(MSTCMD_IR_ID_7), 1000);
      else if(0 == memcmp(key, "8",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_8, strlen(MSTCMD_IR_ID_8), 1000);
      else if(0 == memcmp(key, "9",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IR_ID_9, strlen(MSTCMD_IR_ID_9), 1000);
  }
  else if(0 == memcmp(lanCmd,"sys.ir_remote.",14))         /* ģ��ң����ָ�� */
  {
      uint8_t* key = (uint8_t*)&lanCmd[14];
      if(0 == memcmp(key, "0",1))
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_0, strlen(MSTCMD_IRREMOTE_0), 1000);
      else if(0 == memcmp(key, "1",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_1, strlen(MSTCMD_IRREMOTE_1), 1000);
      else if(0 == memcmp(key, "2",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_2, strlen(MSTCMD_IRREMOTE_2), 1000);
      else if(0 == memcmp(key, "3",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_3, strlen(MSTCMD_IRREMOTE_3), 1000);
      else if(0 == memcmp(key, "4",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_4, strlen(MSTCMD_IRREMOTE_4), 1000);
      else if(0 == memcmp(key, "5",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_5, strlen(MSTCMD_IRREMOTE_5), 1000);
      else if(0 == memcmp(key, "6",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_6, strlen(MSTCMD_IRREMOTE_6), 1000);
      else if(0 == memcmp(key, "7",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_7, strlen(MSTCMD_IRREMOTE_7), 1000);
      else if(0 == memcmp(key, "8",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_8, strlen(MSTCMD_IRREMOTE_8), 1000);
      else if(0 == memcmp(key, "9",1))        
        HAL_UART_Transmit(&huart1, MSTCMD_IRREMOTE_9, strlen(MSTCMD_IRREMOTE_9), 1000);
  }
  else if(0 == memcmp(lanCmd,"sys.debug.",10))         /* debugָ�� */
  {
      uint8_t* ops = (uint8_t*)&lanCmd[10];
      if(0 == memcmp(ops, "pause",5))
        ;

  }
  else if (0 == memcmp(lanCmd, "sys.cm.", 7))       /* ɫ�ʹ���ָ�� */
  {
      uint8_t *colorMode = (uint8_t*)&lanCmd[7];
      if (0 == memcmp(colorMode, "FS", 2))
          HAL_UART_Transmit(&huart1, MSTCMD_CM_FS, strlen(MSTCMD_CM_FS), 1000);
      else if (0 == memcmp(colorMode, "Rec709", 7))
          HAL_UART_Transmit(&huart1, MSTCMD_CM_REC709, strlen(MSTCMD_CM_REC709), 1000);
  }
  else if(0 == memcmp(lanCmd, "sys.low_drag.int.", 17)) /*����Ӱǿ����������*/
  {
      uint8_t *mode = (uint8_t*)&lanCmd[17];
      if(0 == memcmp(mode, "0", 1))
          HAL_UART_Transmit(&huart1, MSTCMD_LOW_DRG_INT_0, strlen(MSTCMD_LOW_DRG_INT_0), 1000);
      else if(0 == memcmp(mode, "1", 1))
          HAL_UART_Transmit(&huart1, MSTCMD_LOW_DRG_INT_1, strlen(MSTCMD_LOW_DRG_INT_1), 1000);
      else if(0 == memcmp(mode, "2", 1 ))
          HAL_UART_Transmit(&huart1, MSTCMD_LOW_DRG_INT_2, strlen(MSTCMD_LOW_DRG_INT_2), 1000);
  }
  else if(0 == memcmp(lanCmd, "sys.low_drag.mode.", 18)) /*����Ӱǿ����������*/
  {
      uint8_t *mode = (uint8_t*)&lanCmd[18];
      if(0 == memcmp(mode, "0", 1))
          HAL_UART_Transmit(&huart1, MSTCMD_LOW_DRG_MODE_0, strlen(MSTCMD_LOW_DRG_MODE_0), 1000);
      else if(0 == memcmp(mode, "1", 1))
          HAL_UART_Transmit(&huart1, MSTCMD_LOW_DRG_MODE_1, strlen(MSTCMD_LOW_DRG_MODE_1), 1000);
      else if(0 == memcmp(mode, "2", 1 ))
          HAL_UART_Transmit(&huart1, MSTCMD_LOW_DRG_MODE_2, strlen(MSTCMD_LOW_DRG_MODE_2), 1000);
  }
  else if(0 == memcmp(lanCmd, "sys.lan.", 8)) /*lan��������*/
  {
      uint8_t *attr = (uint8_t*)&lanCmd[8];
      if(0 == memcmp(attr, "broadcast.", 10)) //�㲥���տ���
      {
          uint8_t *ops = &attr[10];
          if(0 == memcmp(ops, "on", 2))
          {
              ETH_Broadcast_Set(ON);
              log_info("Eth Broadcast reception On\n");
          } 
          else if(0 == memcmp(ops, "off", 3))
          {
              ETH_Broadcast_Set(OFF);
              log_info("Eth Broadcast reception Off\n");
          }   
      }
  }
  else if(0 == memcmp(lanCmd, "sys.src.", 8))
  {
      uint8_t *src = (uint8_t*)&lanCmd[8];
      if(0 == memcmp(src, "vga", 3))
          HAL_UART_Transmit(&huart1, MSTCMD_SRC_VGA, strlen(MSTCMD_SRC_VGA), 1000);
      else if(0 == memcmp(src, "dvi", 3))
          HAL_UART_Transmit(&huart1, MSTCMD_SRC_DVI, strlen(MSTCMD_SRC_DVI), 1000);
      else if(0 == memcmp(src, "hdmi2", 5))
          HAL_UART_Transmit(&huart1, MSTCMD_SRC_HDMI2, strlen(MSTCMD_SRC_HDMI2), 1000);
      else if(0 == memcmp(src, "hdmi", 4))
          HAL_UART_Transmit(&huart1, MSTCMD_SRC_HDMI, strlen(MSTCMD_SRC_HDMI), 1000);
      else if(0 == memcmp(src, "dp", 2))
          HAL_UART_Transmit(&huart1, MSTCMD_SRC_DP, strlen(MSTCMD_SRC_DP), 1000);
  }
  else if(0 == memcmp(lanCmd, "sys.res.", 8))
  {
      uint8_t *ops = (uint8_t*)&lanCmd[8];
      if(0 == memcmp(ops, "2k", 2))
        HAL_UART_Transmit(&huart1, MSTCMD_RESOLUTION_2K, strlen(MSTCMD_RESOLUTION_2K), 1000);
      else if(0 == memcmp(ops, "4k", 2))
        HAL_UART_Transmit(&huart1, MSTCMD_RESOLUTION_4K, strlen(MSTCMD_RESOLUTION_4K), 1000);
  }
  else if(0 == memcmp(lanCmd, "sys.low_lat.", 12))
  {
      uint8_t *ops = (uint8_t*)&lanCmd[12];
      if(0 == memcmp(ops, "on", 2))
        HAL_UART_Transmit(&huart1, MSTCMD_LOW_LAT_ON, strlen(MSTCMD_LOW_LAT_ON), 1000);
      else if(0 == memcmp(ops, "off", 3))
        HAL_UART_Transmit(&huart1, MSTCMD_LOW_LAT_OFF, strlen(MSTCMD_LOW_LAT_OFF), 1000);
  }
  else if(0 == memcmp(lanCmd, "sys.low_drag.", 13))
  {
      uint8_t *ops = (uint8_t*)&lanCmd[13];
      if(0 == memcmp(ops, "on", 2))
        HAL_UART_Transmit(&huart1, MSTCMD_LOW_DRAG_ON, strlen(MSTCMD_LOW_DRAG_ON), 1000);
      else if(0 == memcmp(ops, "off", 3))
        HAL_UART_Transmit(&huart1, MSTCMD_LOW_DRAG_OFF, strlen(MSTCMD_LOW_DRAG_OFF), 1000);
  }
  else if(0 == memcmp(lanCmd, "sys.fac.model.", 14))
  {
      const char *model_str = &lanCmd[14];
      char msg[16] = {0};
      msg[0] = 0X5D;
      strcpy((msg+1), model_str);
      printf("update model string:%s\n", msg+1);
      HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 1000);
   }
  else if(0 == memcmp(lanCmd, "sys.fac.prog", 12))
  {
      HAL_UART_Transmit(&huart1, MSTCMD_PROGRAM_MODE, strlen(MSTCMD_PROGRAM_MODE), 1000);
  }
  else if(0 == memcmp(lanCmd, "sys.fac.3d_sync_delay.", 22))
  {
      const char *val = &lanCmd[22];
      int value = atoi(val);
      if(value > 0 && value <= 255)
        set_sync_3d_delay_arg((uint8_t)value);
      else
        log_err("3d sync delay illegal value: %d\n", value);
  }
  else
  { 
    // �÷�ʽ�������stdup()�����������������жϴ���
    // char* dupStr = NULL;
    // dupStr = strdup(udp_recvbuf);

    static char tmpStr[128];   
    strcpy(tmpStr, udp_recvbuf);      //splitString���ƻ�ԭ��BUFFER�е����ݣ�copyһ��
    splitString(tmpStr);              //�ַ���split
    if(num_substrings >= 2)  //����ַ�����Ϊ0
    {
      if(0 == strcmp(substrings[0], "sys"))
      {
        if(0 == strcmp(substrings[1], "hsg"))
        {
          mstCmdMsg.func_code = FUNC_CODE_HSG;
          if((num_substrings == 3) && (0 == strcmp(substrings[2], "reset"))) 
          {
            mstCmdMsg.ops = OPS_RESET;
            goto send;
          }
          else if(num_substrings == 6)   //������Ŀ��ȷ�Ŵ���
          {
            if(0 == strcmp(substrings[2], "set"))
            {
              mstCmdMsg.ops = OPS_SET_VAL;
              if(0 == strcmp(substrings[3], "r"))
                mstCmdMsg.color = COLOR_R;
              else if(0 == strcmp(substrings[3], "g"))
                mstCmdMsg.color = COLOR_G;
              else if(0 == strcmp(substrings[3], "b"))
                mstCmdMsg.color = COLOR_B;
              else if(0 == strcmp(substrings[3], "c"))
                mstCmdMsg.color = COLOR_C;
              else if(0 == strcmp(substrings[3], "y"))
                mstCmdMsg.color = COLOR_Y;
              else if(0 == strcmp(substrings[3], "m"))
                mstCmdMsg.color = COLOR_M;
              else if(0 == strcmp(substrings[3], "w"))
                mstCmdMsg.color = COLOR_W;
              else 
                goto exit;  // ��������ȷ��ֱ���˳�

              mstCmdMsg.ops = OPS_SET_VAL;
              if((0 == strcmp(substrings[4], "r")) || (0 == strcmp(substrings[4], "hue")))
                mstCmdMsg.attr = COLOR_ATTR_HUE;
              else if((0 == strcmp(substrings[4], "g")) || (0 == strcmp(substrings[4], "satu")))
                mstCmdMsg.attr = COLOR_ATTR_SATURATION;
              else if((0 == strcmp(substrings[4], "b")) || (0 == strcmp(substrings[4], "gain")))
                mstCmdMsg.attr = COLOR_ATTR_GAIN;
              else 
                goto exit;  // ��������ȷ��ֱ���˳�

              mstCmdMsg.value = atoi(substrings[5]);
              goto send;
            }
          }
          else  //������Ŀ����ȷ��ֱ���˳�
          {
            goto exit;
          }
        }
        else if(0 == strcmp(substrings[1],"laser_rgb"))
        {
          mstCmdMsg.func_code = FUNC_CODE_LASER_RGB;
          if((num_substrings == 3) && (0 == strcmp(substrings[2], "reset"))) 
          {
            mstCmdMsg.ops = OPS_RESET;
            goto send;
          }
          else if(num_substrings == 5)   //������Ŀ��ȷ�Ŵ���
          {
            if(0 == strcmp(substrings[2], "set"))
            {
              mstCmdMsg.ops = OPS_SET_VAL;
              if(0 == strcmp(substrings[3], "r"))
                mstCmdMsg.color = COLOR_R;
              else if(0 == strcmp(substrings[3], "g"))
                mstCmdMsg.color = COLOR_G;
              else if(0 == strcmp(substrings[3], "b"))
                mstCmdMsg.color = COLOR_B;
              else 
                goto exit;  // ��������ȷ��ֱ���˳�

              mstCmdMsg.value = atoi(substrings[4]);
              goto send;
            }
          }
          else  //������Ŀ����ȷ��ֱ���˳�
          {
            goto exit;
          }
        }
      }
    }
exit:
    printf("illegale cmd, num_substrings: %d\n", num_substrings);
    return -1;
send: 
    HAL_UART_Transmit(&huart1, (uint8_t*)&mstCmdMsg, sizeof(mstCmdMsg), 1000);
    printf("\n");
    for (int i = 0; i < sizeof(mstCmdMsg); i++)
    {
      printf("mstCmdMsg[%d]:%02x\n",i, ((uint8_t*)&mstCmdMsg)[i]);
    }
    printf("\n");
    return 0;
  }
  return 0;
}



/**
 * @brief ����˿�ָ�������
 *
 */
void Netif_CMD_Process(void)
{
    if(udpRcvFlag)
    {
        printf("[UDP_CMD]:%s\r\n", udp_recvbuf);
        udpRcvFlag = 0;

        if (udp_recvbuf[0] == 80 && udp_recvbuf[1] == 87 && udp_recvbuf[2] == 79) //PWO
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Upw", 3, 5000);
        }
        else if (udp_recvbuf[0] == 80 && udp_recvbuf[1] == 87 && udp_recvbuf[2] == 82) //PWR
        {
            uint8_t data[3] = {MST_CMD_POWER, 0, PWR_ON};
            appSendUartMsg(data, 3, false, NULL, 0, 0);

            HAL_Delay(10 * 1000);
            uint8_t ackData[8] = {0};
            data[0] = MST_CMD_POWER;
            data[1] = 0x80;
            int retLen = appSendUartMsg(data, 2, true, ackData, 8, 1000);
            if (retLen >= 0)
            {
                printf("Get power state:\n");
                for (size_t i = 0; i < retLen; i++)
                {
                    printf("%d,", ackData[i]);
                }
                printf("\n");
            }

            waitMstAlive = 1;
        }
        else if (udp_recvbuf[0] == 83 && udp_recvbuf[1] == 82 && udp_recvbuf[2] == 67) //SRC
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Usr", 3, 5000);
        }
        else if (udp_recvbuf[0] == 82 && udp_recvbuf[1] == 84 && udp_recvbuf[2] == 78) //RTN
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Ubk", 3, 5000);
        }
        else if (udp_recvbuf[0] == 77 && udp_recvbuf[1] == 78 && udp_recvbuf[2] == 85) //MNU
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Umu", 3, 5000);
        }
        else if (udp_recvbuf[0] == 76 && udp_recvbuf[1] == 70 && udp_recvbuf[2] == 84) //LFT
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Ult", 3, 5000);
        }
        else if (udp_recvbuf[0] == 82 && udp_recvbuf[1] == 71 && udp_recvbuf[2] == 84) //RGT
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Urt", 3, 5000);
        }
        else if (udp_recvbuf[0] == 85 && udp_recvbuf[1] == 80 && udp_recvbuf[2] == 80) //UPP
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Uup", 3, 5000);
        }
        else if (udp_recvbuf[0] == 68 && udp_recvbuf[1] == 87 && udp_recvbuf[2] == 78) //DWN
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Udn", 3, 5000);
        }
        else if (udp_recvbuf[0] == 83 && udp_recvbuf[1] == 69 && udp_recvbuf[2] == 76) //SEL
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Uok", 3, 5000);
        }
        else if (udp_recvbuf[0] == 79 && udp_recvbuf[1] == 83 && udp_recvbuf[2] == 84) //OST
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Uts", 3, 5000);
        }
        else if (udp_recvbuf[0] == 70 && udp_recvbuf[1] == 67 && udp_recvbuf[2] == 83) //FCS
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Ufs", 3, 5000);
        }
        else if (udp_recvbuf[0] == 90 && udp_recvbuf[1] == 79 && udp_recvbuf[2] == 77) //ZOM
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Uzm", 3, 5000);
        }
        else if (udp_recvbuf[0] == 80 && udp_recvbuf[1] == 65 && udp_recvbuf[2] == 84) //PAT
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Uat", 3, 5000);
        }
        else if (udp_recvbuf[0] == 68 && udp_recvbuf[1] == 73 && udp_recvbuf[2] == 82) //DIR
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Uir", 3, 5000);
        }
        else if (udp_recvbuf[0] == 67 && udp_recvbuf[1] == 72 && udp_recvbuf[2] == 75) //CHK
        {
            if (rt_param.health_report_format)
                StateReportTask(STATE_REPORT_MODE_PASSIVE, STATE_REPORT_TYPE_XML);
            else
                StateReportTask(STATE_REPORT_MODE_PASSIVE, STATE_REPORT_TYPE_BYTE);
        }
        else if (udp_recvbuf[0] == 73 && udp_recvbuf[1] == 82 && udp_recvbuf[2] == 79) //IRO
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Uei", 3, 5000);
        }
        else if (udp_recvbuf[0] == 73 && udp_recvbuf[1] == 82 && udp_recvbuf[2] == 70) //IRF
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Ugi", 3, 5000);
        }
        else if (udp_recvbuf[0] == 76 && udp_recvbuf[1] == 68 && udp_recvbuf[2] == 79) //LDO
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Ujl", 3, 5000);
        }
        else if (udp_recvbuf[0] == 76 && udp_recvbuf[1] == 68 && udp_recvbuf[2] == 70) //LDF
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Unl", 3, 5000);
        }
        else if (0 == memcmp(udp_recvbuf, "IFO", 3)) //IFO
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Uxf", 3, 5000);
        }
        else if (udp_recvbuf[0] == 84 && udp_recvbuf[1] == 73 && udp_recvbuf[2] == 77 && udp_recvbuf[3] == 51 && udp_recvbuf[4] == 48) //TIM30
        {
            rt_param.health_report_period = 30;
            save_rt_param_to_flash();
        }
        else if (udp_recvbuf[0] == 84 && udp_recvbuf[1] == 73 && udp_recvbuf[2] == 77 && udp_recvbuf[3] == 52 && udp_recvbuf[4] == 53) //TIM45
        {
            rt_param.health_report_period = 45;
            save_rt_param_to_flash();
        }
        else if (udp_recvbuf[0] == 84 && udp_recvbuf[1] == 73 && udp_recvbuf[2] == 77 && udp_recvbuf[3] == 54 && udp_recvbuf[4] == 48) //TIM60
        {
            rt_param.health_report_period = 60;
            save_rt_param_to_flash();
        }
        else if (udp_recvbuf[0] == 84 && udp_recvbuf[1] == 73 && udp_recvbuf[2] == 77 && udp_recvbuf[3] == 57 && udp_recvbuf[4] == 48) //TIM90
        {
            rt_param.health_report_period = 90;
            save_rt_param_to_flash();
        }
        else if (0 == memcmp(udp_recvbuf, "WRP", 3)) //WARPING
        {
            if (udp_recvbuf[3] == 'C') // coursor ????
            {
                if (udp_recvbuf[4] == 'O') // on
                {
                    CursorMode = 0xf0;
                }
                else if (udp_recvbuf[4] == 'F') // off
                {
                    CursorMode = 0x0f;
                }
            }
            else if (udp_recvbuf[3] == 'P') // warping dot position select
            {
                if (udp_recvbuf[4] == 0) // TL
                    CursorSel_UD = 0X0C;
                else if (udp_recvbuf[4] == 1) // TR
                    CursorSel_LR = 0x09;
                else if (udp_recvbuf[4] == 2) // BL
                    CursorSel_UD = 0X0b;
                else if (udp_recvbuf[4] == 3) // BR
                    CursorSel_LR = 0x0A;
            }
            else if (udp_recvbuf[3] == 'M') // MOVE
            {
                if (udp_recvbuf[4] == 0) //??
                    Wraping_UD = 0X07;
                else if (udp_recvbuf[4] == 1) //??
                    Wraping_UD = 0x06;
                else if (udp_recvbuf[4] == 2) //??
                    Wraping_LR = 0X04;
                else if (udp_recvbuf[4] == 3) //??
                    Wraping_LR = 0x05;
            }
            else if (udp_recvbuf[3] == 'R') // RESET
            {
                Reset_Wrap = 1;
            }
            else if (udp_recvbuf[3] == 'S') // SAVE
            {
                Save_Wrap = 1;
            }
        }
        else if (0 == memcmp(udp_recvbuf, "debug_area.reset", 16))
        {
            memset(rt_param.debug_area, 0, 36);
            save_rt_param_to_flash();
        }
        else if (0 == memcmp(udp_recvbuf, "debug_area.read", 15))
        {
            udp_send_remote(rt_param.debug_area, 36); //????????????????
        }
        else if (0 == memcmp(udp_recvbuf, "fake.mst.ready", 14))
        {
            Work_Stat = 1;
        }
        else if (0 == memcmp(udp_recvbuf, "RST", 3)) //reset to Bootloader
        {
            printf("Reset to bootloader...\r\n");
            HAL_NVIC_SystemReset();
        }
        else if (0 == memcmp(udp_recvbuf, "FAN", 3)) //Fan debug
        {
            uint8_t fan = udp_recvbuf[3]; //???????? 0 - 9
            uint8_t pwm = udp_recvbuf[4]; //????PWM  0 - 100
            if (fan == 0x10)              //?????????
                FAN_PWM_Set(999 * pwm / 100);
            else if (fan == 0x11) //????debug??
                fan_enter_dbg_mode();
            else if (fan == 0x12) //???debug??
                fan_exit_dbg_mode();

            else
                fan_pwm_set_single(999 * pwm / 100, fan);
        }
        else if (0 == memcmp(udp_recvbuf, "C381WR", 6)) // c381 reg write
        {
            unsigned int addr = *(unsigned int *)(udp_recvbuf + 6);
            unsigned int val = *(unsigned int *)(udp_recvbuf + 10);
            printf("wr c381 %x %x\r\n", addr, val);
            write_C381(addr, val);
        }
        else if (0 == memcmp(udp_recvbuf, "C381RD", 6)) // c381 reg read
        {
            unsigned int addr = *(unsigned int *)(udp_recvbuf + 6);
            unsigned int val = read_C381(addr);
            printf("rd c381 %x %x\r\n", addr, val);
        }
        else if (0 == memcmp(udp_recvbuf, "TEMP", 4)) // temp debug
        {
            uint8_t temp_idx = udp_recvbuf[4];
            uint8_t temp_val = udp_recvbuf[5];
            if (temp_idx > 7)
            {
                temp_ctl.dbg_switch = OFF;
                printf("Debug temp off\r\n");
            }
            else
            {
                temp_ctl.dbg_switch = ON;
                temp_ctl.dbg_temp_idx = temp_idx;
                temp_ctl.dbg_temp_val = temp_val;
                printf("Debug temp %d %d \r\n", temp_idx, temp_val);
            }
        }
        else if (0 == memcmp(udp_recvbuf, "381RST", 6))
        {
            rt_param.c381_mode = udp_recvbuf[6];
            C381_Init();
        }
        else if (0 == memcmp(udp_recvbuf, "FAKESTART", 6))
        {
            Work_Stat = 1;
        }
        else if (0 == memcmp(udp_recvbuf, "66352RST", 8))
        {
            it6635_dev_restart();
        }
        else if (0 == memcmp(udp_recvbuf, "sys.", 4))
        {
            lanCmdNewParser(udp_recvbuf);
        }
        else if (udp_recvbuf[0] == '{')
        {
            cJSON_RPC_CommandParse(udp_recvbuf);
        }
    }
}



void cJSON_RPC_CommandParse(char* jsonStr)
{
    netRpcParse(jsonStr);
}
