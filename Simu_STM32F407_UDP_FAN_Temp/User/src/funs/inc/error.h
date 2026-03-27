#ifndef __ERROR_H
#define __ERROR_H

#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "i2c.h"
#include "math.h"

#define Temp_TEST_count   3
#define Fan_TEST_count   3


#define TMP_R_LD1_set      55.0f //TEMP degreeC
#define TMP_R_LD2_set      55.0f //TEMP degreeC

#define	TMP_G_LD1_set      70.0f //TEMP degreeC
#define	TMP_G_LD2_set      70.0f //TEMP degreeC

#define	TMP_B_LD_set        75.0f //TEMP degreeC
#define	TMP_B_LD2_set       75.0f //TEMP degreeC
  
#define TMP_TEC_high_set    58.0f //TEMP degreeC
#define TMP_TEC_Low_set     10.0f //TEMP degreeC
#define TMP_DMD_set         70.0f// 65.0f    //TEMP degreeC
#define	TEC_CFB_set         50.0f //TEMP degreeC Environment


#define FAN_1_Error_Code             0x11    
#define FAN_2_Error_Code             0x12    
#define FAN_3_Error_Code             0x13    
#define FAN_4_Error_Code             0x14    
#define FAN_5_Error_Code             0x15  
#define FAN_6_Error_Code             0x16   
#define FAN_7_Error_Code             0x17     
#define FAN_8_Error_Code             0x18    
#define FAN_9_Error_Code             0x19    
#define FAN_10_Error_Code            0x1A    
#define FAN_11_Error_Code            0x13    
#define FAN_12_Error_Code            0x14    
#define FAN_13_Error_Code            0x15  
#define FAN_14_Error_Code            0x16   
#define PUMP_EN0_Error_Code          0x17     
#define PUMP_EN1_Error_Code          0x18 

#define FAN_1_Warn_Code             0x01    
#define FAN_2_Warn_Code             0x02    
#define FAN_3_Warn_Code             0x10    
#define FAN_4_Warn_Code             0x08    
#define FAN_5_Warn_Code             0x04  
#define FAN_6_Warn_Code             0x20   
#define FAN_7_Warn_Code             0x40     
#define FAN_8_Warn_Code             0x80    
#define FAN_9_Warn_Code             0x01    
#define FAN_10_Warn_Code            0x02    
#define FAN_11_Warn_Code            0x04    
#define FAN_12_Warn_Code            0x08    
#define FAN_13_Warn_Code            0x10 
#define FAN_14_Warn_Code            0x20   
#define PUMP_EN0_Warn_Code          0x40    
#define PUMP_EN1_Warn_Code          0x80 
//20230923
#define TMP_DMD_Error_Code        0x01
#define TMP_R_LD1_Error_Code      0x02		 
#define TMP_R_LD2_Error_Code      0x04		
#define TMP_G_LD1_Error_Code      0x08		 
#define TMP_G_LD2_Error_Code      0x10     
#define TMP_B_LD_Error_Code       0x20		 
#define TMP_B_LD2_Error_Code      0x40	
#define TMP_TEC_Error_Code        0x80
#define TMP_TEC_low_Error_Code    0x29

extern uint8_t FAN_Temp_Error[7];
extern uint8_t FAN_Error1_Buffer[];
extern uint8_t FAN_Error2_Buffer[];
extern uint8_t FAN_Warning[];
extern uint8_t Tempture_Error_Buffer[];
extern uint8_t Fan_Error;
extern uint8_t Tmp_Error;
extern uint8_t FAN_1_FLAG_COUNT;
extern uint8_t FAN_2_FLAG_COUNT;
extern uint8_t FAN_3_FLAG_COUNT;
extern uint8_t FAN_4_FLAG_COUNT;
extern uint8_t FAN_5_FLAG_COUNT;
extern uint8_t FAN_6_FLAG_COUNT;
extern uint8_t FAN_7_FLAG_COUNT;
extern uint8_t FAN_8_FLAG_COUNT;
extern uint8_t FAN_9_FLAG_COUNT;
extern uint8_t FAN_10_FLAG_COUNT;
extern uint8_t FAN_11_FLAG_COUNT;
extern uint8_t FAN_12_FLAG_COUNT;
extern uint8_t FAN_13_FLAG_COUNT;
extern uint8_t FAN_14_FLAG_COUNT;
extern uint8_t PUMP_EN0_FLAG_COUNT;
extern uint8_t PUMP_EN1_FLAG_COUNT;

extern __IO _Bool FAN_1_FLAG;
extern __IO _Bool FAN_2_FLAG;
extern __IO _Bool FAN_3_FLAG;
extern __IO _Bool FAN_4_FLAG;
extern __IO _Bool FAN_5_FLAG;
extern __IO _Bool FAN_6_FLAG;
extern __IO _Bool FAN_7_FLAG;
extern __IO _Bool FAN_8_FLAG;
extern __IO _Bool FAN_9_FLAG;
extern __IO _Bool FAN_10_FLAG;
extern __IO _Bool FAN_11_FLAG;
extern __IO _Bool FAN_12_FLAG;
extern __IO _Bool FAN_13_FLAG;
extern __IO _Bool FAN_14_FLAG;
extern __IO _Bool PUMP_EN0_FLAG;
extern __IO _Bool PUMP_EN1_FLAG;



extern uint8_t TMP_R_LD1_Flag_COUNT;
extern uint8_t TMP_R_LD2_Flag_COUNT;
extern uint8_t TMP_G_LD1_Flag_COUNT;
extern uint8_t TMP_G_LD2_Flag_COUNT;
extern uint8_t TMP_B_LD_Flag_COUNT;
extern uint8_t TMP_TEC_Flag_COUNT;
extern uint8_t TMP_TEC_Minus_Flag_COUNT;
extern uint8_t TMP_DMD_Flag_COUNT;
extern uint8_t TES_CFB_Flag_COUNT;

extern __IO _Bool TMP_R_LD1_Flag;
extern __IO _Bool TMP_R_LD2_Flag;
extern __IO _Bool TMP_G_LD1_Flag;
extern __IO _Bool TMP_G_LD2_Flag;
extern __IO _Bool TMP_B_LD_Flag;
extern __IO _Bool TMP_TEC_Flag;
extern __IO _Bool TMP_TEC_minus_Flag;
extern __IO _Bool TMP_DMD_Flag;
extern __IO _Bool TES_CFB_Flag;

extern __IO float ADS_Temp_Data[8];
/************************************************************
* TI ADS1115 REGISTER SET ADDRESSES
************************************************************/
	//leo 20190925

#define ADS1115_I2C_Address                           (0x90)                   // Device Address
#define ADS1015_I2C_Address                           (0x92)                   // Device Address

#define ADS1015_I2C_Add1                              (0x94)                   // Device Address
#define ADS1015_I2C_Add2                              (0x96)                   // Device Address

#define TI_ADS1115_CONVERSION_REG                     (0x00)                   
#define TI_ADS1115_CONFIG_REG                         (0x01)
#define TI_ADS1115_LO_THRESH_REG                      (0x02)
#define TI_ADS1115_HI_THRESH_REG                      (0x03)

#define TI_ADS1115_PWRDN_DEFAULT                      (0x83)
#define TI_ADS1115_PWRDN_DEFAULT_2                    (0x05)

#define TI_ADS1115_CH0_DEFAULT                        (0x83)
#define TI_ADS1115_CH0_DEFAULT_2                      (0x75)

#define TI_ADS1115_LO_THLD                            (0xa3)
#define TI_ADS1115_LO_THLD1_2                         (0x75)

// Device Conversion State
#define TI_ADS1115_READY                              (0x8000)


#define TI_ADS1115_START_CONVERSION                   (0x8000)



// Channel Selection: Skin (ch0), ambient (ch1), battery (ch2)
#define TI_ADS1115_CH0                                (0x4000)


#define TI_ADS1115_CH1                                (0x5000)

#define TI_ADS1115_CH2                                (0x6000)

#define TI_ADS1115_CH3                                (0x7000)
// Full Scale:4.096V, 2.048V or 1.024V

#define TI_ADS1115_FS6V                               (0x0000)

#define TI_ADS1115_FS4V                               (0x0200)

#define TI_ADS1115_FS2V                               (0x0400)

#define TI_ADS1115_FS1V                               (0x0600)


// Device Operating Mode 
#define TI_ADS1115_PWRDN_SINGLE_SHOT                  (0x0100)

#define TI_ADS1115_CONTINUOUS                         (0x0101)

// Data Rate
#define TI_ADS1115_64SPS                              (0x0060)

#define TI_ADS1115_128SPS                             (0x0080)

// Default COMP settings
#define TI_ADS1115_COMP_DEFAULT                       (0x0003)


#define S1 1
#define S2 2
#define S3 3
#define S4 4
#define S5 5
#define S6 6
#define S7 7
#define S8 8

typedef enum{
	STATE_REPORT_MODE_ACTIVE = 0,		//主动上报类型枚举
	STATE_REPORT_MODE_PASSIVE,			//被动上报类型枚举
}STATE_REPORT_MODE_;

typedef enum{
	STATE_REPORT_TYPE_NONE = 0 ,		//关闭
	STATE_REPORT_TYPE_XML,				//上报报文类型枚举:XML
	STATE_REPORT_TYPE_BYTE,				//上报报文类型枚举:BYTE
	STATE_REPORT_TYPE_JSON,				//上报报文类型枚举:JSON
	STATE_REPORT_TYPE_MAX,				
}STATE_REPORT_TYPE_;


void Fan_LOCKED_check(uint8_t sw);
void ads1115_CH_check(void);
int Tempture_Erorr_Test(void);
extern void Error_IRQ_Reset(void);
void Test_ads1115(void);
float ADS1115_convert(uint16_t ads_ch,uint8_t ads1115_address);
int Fan_Error_warn(void);
void byteMsgConstruct(uint8_t **pBuf, int* pDataLen);

void StateReportTask(STATE_REPORT_MODE_ mode, STATE_REPORT_TYPE_ type);

int check_fan_err(void);
int check_temp_err(void);

#endif
