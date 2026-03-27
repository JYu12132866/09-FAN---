#include "healthMgr.h"
#include "error.h"
#include "main.h"
#include "stdint.h"


#define Temp_R_Threshold 55
#define Temp_G_Threshold 70
#define Temp_B_Threshold 75
#define Temp_DMD_Threshold 70
#define Temp_ENV_Threshold 45

uint8_t Warning_Update = 0;

uint32_t fanErrCode = 0;     //每一个BIT位，代表一个风扇点异常，最大支持32个风扇点异常
uint32_t tempErrCode = 0;    //每一个BIT位，代表一个温度点异常，最大支持32个温度点异常

#pragma pack(1)
typedef struct
{
    uint8_t head;
    uint8_t temp_r1;
    uint8_t temp_r2;
    uint8_t temp_g1;
    uint8_t temp_g2;
    uint8_t temp_b1;
    uint8_t temp_b2;
    uint8_t temp_env;
    uint8_t fan_err_1;
    uint8_t fan_err_2;
    uint8_t software_version_major;
    uint8_t software_version_minor;
    uint8_t software_version_patch;
} Report_Message;
#pragma pack()

int generateErrCode(void)
{
    fanErrCode = 0;
    tempErrCode = 0;
    if (ADS_Temp_Data[0] > Temp_R_Threshold)
    {
        tempErrCode |= 1 << 0;
    }
    if (ADS_Temp_Data[1] > Temp_R_Threshold)
    {
        tempErrCode |= 1 << 1;
    }
    if (ADS_Temp_Data[2] > Temp_G_Threshold)
    {
        tempErrCode |= 1 << 2;
    }
    if (ADS_Temp_Data[3] > Temp_G_Threshold)
    {
        tempErrCode |= 1 << 3;
    }
    if (ADS_Temp_Data[4] > Temp_B_Threshold)
    {
        tempErrCode |= 1 << 4;
    }
    if (ADS_Temp_Data[6] > Temp_B_Threshold)
    {
        tempErrCode |= 1 << 5;
    }
    if (ADS_Temp_Data[7] > Temp_ENV_Threshold)
    {
        tempErrCode |= 1 << 6;
    }

    fanErrCode = FAN_Warning[0] | (FAN_Warning[1] << 8);

    return 0;
}

uint32_t getFanErrCode(void)
{
    return fanErrCode;
}

uint32_t getTempErrCode(void)
{
    return tempErrCode;
}

void Report_Warning(void)
{
    static Report_Message report_message;
    static uint16_t cnt = 0; 
    
    generateErrCode();

    if (Warning_Update)
    {
        printf("Msg to -> Mstar [%d]\n", cnt++);
        Warning_Update = 0;
        report_message.head = 0xE7; //(�����ַ�����һ���ֽڹ̶�Ϊ0xE7��
        if ((int8_t)ADS_Temp_Data[0] < 0)  //��ɫ������1 �¶�
            report_message.temp_r1 = (int8_t)ADS_Temp_Data[0] - 1; 
        else
            report_message.temp_r1 = (int8_t)ADS_Temp_Data[0];
        if ((int8_t)ADS_Temp_Data[1] < 0) //��ɫ������2 �¶�
            report_message.temp_r2 = (int8_t)ADS_Temp_Data[1] - 1; 
        else
            report_message.temp_r2 = (int8_t)ADS_Temp_Data[1];
        if ((int8_t)ADS_Temp_Data[2] < 0)  //��ɫ������1 �¶�
            report_message.temp_g1 = (int8_t)ADS_Temp_Data[2] - 1; // TMP1_G_LD1
        else
            report_message.temp_g1 = (int8_t)ADS_Temp_Data[2];
        if ((int8_t)ADS_Temp_Data[3] < 0)  //��ɫ������2 �¶�
            report_message.temp_g2 = (int8_t)ADS_Temp_Data[3] - 1; // TMP1_G_LD2
        else
            report_message.temp_g2 = (int8_t)ADS_Temp_Data[3];
        if ((int8_t)ADS_Temp_Data[4] < 0)  //��ɫ������1 �¶�
            report_message.temp_b1 = (int8_t)ADS_Temp_Data[4] - 1; // TMP2_B_LD1
        else
            report_message.temp_b1 = (int8_t)ADS_Temp_Data[4];
#if Temp_B2
        if ((int8_t)ADS_Temp_Data[6] < 0) //��ɫ������2 �¶�
            report_message.temp_b2 = (int8_t)ADS_Temp_Data[6] - 1; // TMP2_B_LD2
        else
            report_message.temp_b2 = (int8_t)ADS_Temp_Data[6];
#endif
        if ((int8_t)ADS_Temp_Data[7] < 0)  //�����¶�
            report_message.temp_env = (int8_t)ADS_Temp_Data[7] - 1; // TMP2_Environment
        else
            report_message.temp_env = (int8_t)ADS_Temp_Data[7];

        report_message.fan_err_1 = FAN_Warning[0]; // �����쳣��Ϣ1  
        report_message.fan_err_2 = FAN_Warning[1]; // �����쳣��Ϣ2
        report_message.software_version_major = ((uint8_t)VERSION_MAJOR);    //�����汾�� major
        report_message.software_version_minor = ((uint8_t)VERSION_MINOR);    //�����汾�� minor
        report_message.software_version_patch = ((uint8_t)VERSION_PATCH);    //�����汾�� patch
        HAL_UART_Transmit(&huart1, (uint8_t *)&report_message, sizeof(report_message), 5000);   //���ͱ�����Ϣ
    }
}


