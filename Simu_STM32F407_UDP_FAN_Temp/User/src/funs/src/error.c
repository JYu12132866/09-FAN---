#include "error.h"
#include "adc.h"
#include "main.h"
#include "tim.h"
#include "ezxml.h"
#include "udp_demo.h"
#include "fanctl.h"
#include "logger.h"
/*********************************************************************************/
// IIC ADS1115 Sensor data. //ADS1115
// leo 20190925
// static int16_t ADS1115_TempSensor_Address = ADS1115_I2C_Address;

uint16_t ADS1115_TempSensor_Enable;
uint8_t ADS1115_TempSensor_status = PASS;
uint32_t ADS1115_TempSensorSem_id;
uint32_t ADS1115_TempSensor_WriteBytes;
uint8_t ADS1115_TempSensor_WriteData1[1] = {0x01};
uint32_t ADS1115_TempSensor_ReadBytes = 2;
uint8_t ADS1115_TempSensor_ReadData[2] = {0x00, 0x00};
uint32_t ADS1115_TempSensor_TimeOut;
uint32_t ADS1115_TempSensor_DataSent;
uint32_t ADS1115_TempSensor_DataReceived;
uint32_t ADS1115_TempSensor_WriteDelay = 0;
uint32_t ADS1115_TempSensor_WriteTimeOut = 200;
uint8_t ADS1115_TempSensor_Mode;
uint8_t ADS1115_Sensor_Init[2] = {0x09, 0xC4};
int16_t ADS1115_TempSensor_Value = 0;
int16_t ADS1115_TempSensor_Current = 0x00;
_Bool ADS1115_TempSensor_Status = FALSE;
_Bool ADS1115_TempSensor_Power = FALSE;
/*********************************************************************************/
#define pwm_print 0
uint8_t Fan_Error = 0;
uint8_t Tmp_Error = 0;
uint8_t FAN_Error1_Buffer[8] = {0};
uint8_t FAN_Error2_Buffer[8] = {0};

uint16_t workhour;
uint8_t FAN_Warning[2] = {0};
__IO float ADS_Temp_Data[8];
uint8_t Tempture_Error_Buffer[8];
uint8_t FAN_Temp_Error[7] = {0};
extern uint8_t Warn_Type;
// extern int XML;
extern char Warn_state[4];
extern char Fan1_check[80];
extern char Fan2_check[80];
extern char Tem_check[80];
extern char DLP_check[80];
extern char Mstar_check[80];
extern char Workhour_check[80];
extern char Fan1_Status[5];
extern char Fan2_Status[5];
extern char Temp_B_Status[8];
extern char Temp_B2_Status[8];
extern char Temp_G1_Status[8];
extern char Temp_G2_Status[8];
extern char Temp_R1_Status[8];
extern char Temp_R2_Status[8];
extern char Temp_E_Status[8];
extern char Temp_TEC_Status[8];
extern char Temp_DMD_Status[8];
extern uint8_t DMD_Temp;
char Report[40] = {0x76};
char UDP_Report[200];
uint8_t Fan_Err = 0;
uint8_t FrameNo;
uint8_t Fan_warning;
uint8_t Tmp_warning;
uint8_t FAN_1_FLAG_COUNT;
uint8_t FAN_2_FLAG_COUNT;
uint8_t FAN_3_FLAG_COUNT;
uint8_t FAN_4_FLAG_COUNT;
uint8_t FAN_5_FLAG_COUNT;
uint8_t FAN_6_FLAG_COUNT;
uint8_t FAN_7_FLAG_COUNT;
uint8_t FAN_8_FLAG_COUNT;
uint8_t FAN_9_FLAG_COUNT;
uint8_t FAN_10_FLAG_COUNT;
uint8_t FAN_11_FLAG_COUNT;
uint8_t FAN_12_FLAG_COUNT;
uint8_t FAN_13_FLAG_COUNT;
uint8_t FAN_14_FLAG_COUNT;
uint8_t PUMP_EN0_FLAG_COUNT;
uint8_t PUMP_EN1_FLAG_COUNT;

__IO _Bool FAN_1_FLAG;
__IO _Bool FAN_2_FLAG;
__IO _Bool FAN_3_FLAG;
__IO _Bool FAN_4_FLAG;
__IO _Bool FAN_5_FLAG;
__IO _Bool FAN_6_FLAG;
__IO _Bool FAN_7_FLAG;
__IO _Bool FAN_8_FLAG;
__IO _Bool FAN_9_FLAG;
__IO _Bool FAN_10_FLAG;
__IO _Bool FAN_11_FLAG;
__IO _Bool FAN_12_FLAG;
__IO _Bool FAN_13_FLAG;
__IO _Bool FAN_14_FLAG;
__IO _Bool PUMP_EN0_FLAG;
__IO _Bool PUMP_EN1_FLAG;

uint8_t TMP_R_LD1_Flag_COUNT;
uint8_t TMP_R_LD2_Flag_COUNT;
uint8_t TMP_G_LD1_Flag_COUNT;
uint8_t TMP_G_LD2_Flag_COUNT;
uint8_t TMP_B_LD_Flag_COUNT;
uint8_t TMP_B_LD2_Flag_COUNT;
uint8_t TMP_TEC_Flag_COUNT;
uint8_t TMP_TEC_Minus_Flag_COUNT;
uint8_t TMP_DMD_Flag_COUNT;
uint8_t TES_CFB_Flag_COUNT;

__IO _Bool TMP_R_LD1_Flag;
__IO _Bool TMP_R_LD2_Flag;
__IO _Bool TMP_G_LD1_Flag;
__IO _Bool TMP_G_LD2_Flag;
__IO _Bool TMP_B_LD_Flag;
__IO _Bool TMP_B_LD2_Flag;
__IO _Bool TMP_TEC_Flag;
__IO _Bool TMP_TEC_minus_Flag;
__IO _Bool TMP_DMD_Flag;
__IO _Bool TES_CFB_Flag;

extern __IO _Bool DMD_Overheated_Flag;
extern __IO _Bool DMD_UnderTemp_Flag; // leo 20220723

_Bool ADS1115_TempSensorWrite(uint8_t byte_count, uint8_t *data_buffer_ptr, uint8_t I2CAddress)
{
	uint8_t I2C_RTN;
	_Bool ret_val = PASS;
	I2C_RTN = HAL_I2C_Master_Transmit(&hi2c2, I2CAddress, data_buffer_ptr, byte_count, 5000);
	if (I2C_RTN != HAL_OK)
		ret_val = FAILED;
	return ret_val;
}
_Bool ADS1115_TempSensorRead(uint8_t device_subaddress, uint8_t byte_count, uint8_t *data_buffer_ptr, uint8_t I2CAddress)
{
	uint8_t ADS1115_W[1]; //,ADS1115_R[1]
	uint8_t rtn;
	_Bool ret_val = PASS;
	ADS1115_W[0] = device_subaddress;
	if (HAL_OK == HAL_I2C_Master_Transmit(&hi2c2, I2CAddress, (uint8_t *)ADS1115_W, 1, 5000))
	{
		rtn = HAL_I2C_Master_Receive(&hi2c2, I2CAddress, data_buffer_ptr, 1, 5000);
	}
	if (rtn != HAL_OK)
		ret_val = FAILED;
	return ret_val;
}

void Test_ads1115(void)
{
	uint8_t read_val[2]; // buffer to store register values
	uint8_t Write_Reg[3];

	Write_Reg[0] = TI_ADS1115_CONFIG_REG;
	Write_Reg[1] = TI_ADS1115_CH0_DEFAULT_2;
	Write_Reg[2] = TI_ADS1115_CH0_DEFAULT;
	ADS1115_TempSensorWrite(3, Write_Reg, ADS1115_I2C_Address);

	ADS1115_TempSensorRead(TI_ADS1115_CONFIG_REG, 2, read_val, ADS1115_I2C_Address);
	printf("environment: ADS1115_TempSensorRead TI_ADS1115_CONFIG_REG1=0x%x,TI_ADS1115_CONFIG_REG2=0x%x\r\n", read_val[0], read_val[1]);
}

float ADS1115_convert(uint16_t ads_ch, uint8_t ads1115_address)
{
	uint16_t status = 0;
	uint8_t count = 0;
	uint8_t read_val[2] = {0}; // buffer to store register values
	uint8_t Write_Reg[3] = {0};
	float TempValue = 0.000f;
	uint16_t adc_config = 0;
	float Rt = 0;
	float Temp = 0;
	float checktmp = 0;

	adc_config = TI_ADS1115_START_CONVERSION | ads_ch | TI_ADS1115_FS4V |
				 TI_ADS1115_PWRDN_SINGLE_SHOT |
				 TI_ADS1115_128SPS |
				 TI_ADS1115_COMP_DEFAULT;

	Write_Reg[0] = TI_ADS1115_CONFIG_REG;
	Write_Reg[1] = (uint8_t)(adc_config >> 8);
	Write_Reg[2] = (uint8_t)(adc_config);
	ADS1115_TempSensorWrite(3, Write_Reg, ads1115_address);
	while (count < 5) // status != TI_ADS1115_READY&&
	{
		//				ADS1115_TempSensorRead(TI_ADS1115_CONFIG_REG,2,read_val,ads1115_address);
		//				status=(uint16_t)(read_val[0]<<8)|(uint16_t)read_val[1];
		//				status = status & TI_ADS1115_READY;
		HAL_Delay(10);
		count++;
	}
	ADS1115_TempSensorRead(TI_ADS1115_CONFIG_REG, 2, read_val, ads1115_address);
	status = (uint16_t)(read_val[0] << 8) | (uint16_t)read_val[1];
	status = status & TI_ADS1115_READY;

	if (status != TI_ADS1115_READY)
		printf(" ADS1115(0x%x)_CHN(%d) is not ready\r\n", ads1115_address, ((ads_ch >> 12) - 3));

	read_val[1] = 0;
	read_val[0] = 0;
	ADS1115_TempSensorRead(TI_ADS1115_CONVERSION_REG, 2, read_val, ads1115_address);
	TempValue = (float)(read_val[0] << 8 | read_val[1]);
	TempValue = 4.096f * TempValue / 32752;
	//
	if (TempValue > 3.1572f)
		TempValue = 3.1592f;
	else if (TempValue < 0.2271f)
		TempValue = 0.2271f;
	checktmp = 3.3f - TempValue;
	Rt = TempValue * 4700 / checktmp;
	//			printf(" ADS1115_Rt=%.3f Ohm\r\n",Rt);//%.2f
	Temp = 1 / (1 / T2 + log(Rt / Rp) / Bx) - Ka + 0.1f;
	//			printf(" ADS1115_Temp=%.2f degC\r\n",Temp);
	return Temp;
}

void ads1115_CH_check(void)
{
	ADS_Temp_Data[0] = ADS1115_convert(TI_ADS1115_CH0, ADS1115_I2C_Address); //R1
	ADS_Temp_Data[1] = ADS1115_convert(TI_ADS1115_CH1, ADS1115_I2C_Address); //R2
	ADS_Temp_Data[2] = ADS1115_convert(TI_ADS1115_CH2, ADS1115_I2C_Address); //G1
	ADS_Temp_Data[3] = ADS1115_convert(TI_ADS1115_CH3, ADS1115_I2C_Address); //G2

	ADS_Temp_Data[4] = ADS1115_convert(TI_ADS1115_CH0, ADS1015_I2C_Address); //B1
	ADS_Temp_Data[5] = ADS1115_convert(TI_ADS1115_CH1, ADS1015_I2C_Address); //
	ADS_Temp_Data[6] = ADS1115_convert(TI_ADS1115_CH2, ADS1015_I2C_Address); //B2
	ADS_Temp_Data[7] = ADS1115_convert(TI_ADS1115_CH3, ADS1015_I2C_Address); //ENV

	printf(" \r\n");
	printf(" TMP1_R_LD1: %.2f degC\r\n", ADS_Temp_Data[0]);
	printf(" TMP1_R_LD2: %.2f degC\r\n", ADS_Temp_Data[1]);
	printf(" TMP1_G_LD1: %.2f degC\r\n", ADS_Temp_Data[2]);
	printf(" TMP1_G_LD2: %.2f degC\r\n", ADS_Temp_Data[3]);
	printf(" TMP2_B_LD1: %.2f degC\r\n", ADS_Temp_Data[4]);
	// printf(" TMP2_TEC:   %.2f degC\r\n", ADS_Temp_Data[5]);
	printf(" TEC2_B_LD2: %.2f degC\r\n", ADS_Temp_Data[6]);
	printf(" TMP2_Environment : %.2f degC\r\n", ADS_Temp_Data[7]);
}


void xmlParamUpdate(void)
{
	int Temp_R_LD1_Stat, Temp_R_LD2_Stat, Temp_G_LD1_Stat, Temp_G_LD2_Stat, Temp_B_LD1_Stat, Temp_Envi_Stat, Temp_DMD_Stat;
#if Temp_B2
	int Temp_B_LD2_Stat; //,Temp_TEC_Stat
#endif
	int Fan1_Stat = 0;
	int Fan2_Stat = 0;
	uint8_t War_State = 0;
	// XML = 2;

	if (Work_Stat)
	{
		Temp_R_LD1_Stat = (int8_t)ADS_Temp_Data[0]; // TMP1_R_LD1
		Temp_R_LD2_Stat = (int8_t)ADS_Temp_Data[1]; // TMP1_R_LD2
		Temp_G_LD1_Stat = (int8_t)ADS_Temp_Data[2]; // TMP1_G_LD1
		Temp_G_LD2_Stat = (int8_t)ADS_Temp_Data[3]; // TMP1_G_LD2
		Temp_B_LD1_Stat = (int8_t)ADS_Temp_Data[4]; // TMP2_B_LD1
#if Temp_B2
		Temp_B_LD2_Stat = (int8_t)ADS_Temp_Data[6]; // TMP2_B_LD2
#endif
		//	Temp_TEC_Stat=(uint8_t)ADS_Temp_Data[5];//TMP2_TEC
		Temp_Envi_Stat = (int8_t)ADS_Temp_Data[7]; // TMP2_Environment
		Temp_DMD_Stat = DMD_Temp;

		Fan1_Stat = FAN_Warning[0]; // FAN_1_Speed
		Fan2_Stat = FAN_Warning[1]; // FAN_2_Speed
	}
	else
	{
		Temp_R_LD1_Stat = 0; // TMP1_R_LD1
		Temp_R_LD2_Stat = 0; // TMP1_R_LD2
		Temp_G_LD1_Stat = 0; // TMP1_G_LD1
		Temp_G_LD2_Stat = 0; // TMP1_G_LD2
		Temp_B_LD1_Stat = 0; // TMP2_B_LD1
#if Temp_B2
		Temp_B_LD2_Stat = 0; // TMP2_B_LD2
#endif
		Temp_Envi_Stat = 0; // TMP2_Environment
		Temp_DMD_Stat = 0;

		Fan1_Stat = 0; // FAN_1_Speed
		Fan2_Stat = 0; // FAN_2_Speed
	}

	sprintf(Fan1_Status, "%d", Fan1_Stat);
	sprintf(Fan2_Status, "%d", Fan2_Stat);
	sprintf(Warn_state, "%d", War_State);
	sprintf(Temp_R1_Status, "%d", Temp_R_LD1_Stat);
	sprintf(Temp_R2_Status, "%d", Temp_R_LD2_Stat);
	sprintf(Temp_G1_Status, "%d", Temp_G_LD1_Stat);
	sprintf(Temp_G2_Status, "%d", Temp_G_LD2_Stat);
	sprintf(Temp_B_Status, "%d", Temp_B_LD1_Stat);
#if Temp_B2
	// 5��6���ͳһ
	if(Temp_B_LD2_Stat == -19)     //������������-19�棬��ʾû�����������
		sprintf(Temp_B2_Status, "N/A");
	else
		sprintf(Temp_B2_Status, "%d", Temp_B_LD2_Stat);

#endif
	sprintf(Temp_E_Status, "%d", Temp_Envi_Stat);
	//  printf(" Temp_E_Status:0x%x\r\n",(uint8_t) Temp_Envi_Stat);
	// sprintf(Temp_TEC_Status, "%d", Temp_TEC_Stat);
	sprintf(Temp_DMD_Status, "%d", Temp_DMD_Stat);
}


void byteMsgConstruct(uint8_t **pBuf, int* pDataLen)
{
	{
		FrameNo++;
		Report[1] = FrameNo;
	}
	Report[2] = Work_Stat;
	if (Work_Stat == 1)
	{
		Report[3] = (Fan_warning << 1) | Tmp_warning;
		Report[4] = Warn_Type;
		Report[6] = Fan_Err;
		Report[8] = DMD_Temp;				   // DMD TMP
		Report[9] = (int8_t)ADS_Temp_Data[0];  // R1 TEMP
		Report[10] = (int8_t)ADS_Temp_Data[1]; // R2 TEMP
		Report[11] = (int8_t)ADS_Temp_Data[2]; // G1 TEMP
		Report[12] = (int8_t)ADS_Temp_Data[3]; // G2 TEMP
		Report[13] = (int8_t)ADS_Temp_Data[4]; // B1 TEMP
#if Temp_B2
		// 5 �� 6 ���ͳһ
		if((int8_t)ADS_Temp_Data[6] != -19)
			Report[14] = (int8_t)ADS_Temp_Data[6]; // B2 TEMP
		else
			Report[14] = 0; // B2 TEMP
#endif
		Report[15] = (int8_t)ADS_Temp_Data[7]; // ENVIRONMENT TEMP
	}
	else
	{
		Report[3] = 0;
		Report[4] = 0;
		Report[6] = 0;
		Report[8] = 0;	// DMD TMP
		Report[9] = 0;	// R1 TEMP
		Report[10] = 0; // R2 TEMP
		Report[11] = 0; // G1 TEMP
		Report[12] = 0; // G2 TEMP
		Report[13] = 0; // B1 TEMP
#if Temp_B2
		Report[14] = 0; // B2 TEMP
#endif
		Report[15] = 0; // ENVIRONMENT TEMP
	}

	*pBuf = (uint8_t*)Report;
    *pDataLen = 40;

	//	  sprintf(UDP_Report, "[%x][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d] [%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d] [%d][%d][%d][%d][%d][%d][%d][%d][%d]",
	//		       Report[0],Report[1],Report[2],Report[3],Report[4],Report[5],Report[6],Report[7],Report[8],Report[9],Report[10],Report[11],Report[12],Report[13],Report[14],Report[15],
	//		       Report[16],Report[17],Report[18],Report[19],Report[20],Report[21],Report[22],Report[23],Report[24],Report[25],Report[26],Report[27],Report[28],Report[29],Report[30],
	//		        Report[31], Report[32], Report[33], Report[34], Report[35], Report[36], Report[37], Report[38], Report[39]);

	// udp_demo_senddata(udppcb);
	// udp_send_data(udppcb, )
}


void StateReportTask(STATE_REPORT_MODE_ mode, STATE_REPORT_TYPE_ type)
{
	struct ip_addr* ip;
	u16_t port;
	u8_t *pBuf;
	int dataLen; 

	switch (type)
	{
	case STATE_REPORT_TYPE_XML:
	default:
		xmlParamUpdate();
		xmlMsgConstruct(&pBuf, &dataLen);
		break;
	case STATE_REPORT_TYPE_BYTE:
		byteMsgConstruct(&pBuf, &dataLen);
		break;
	}

	switch (mode)
	{
	case STATE_REPORT_MODE_ACTIVE:
	default:
		ip = &rmtipaddr;
		port = rmtport;
		break;
	case STATE_REPORT_MODE_PASSIVE:
		ip = &dst_udpIP;
		port = dst_udpport;
		break;
	}
	udp_send_data(udppcb, ip, port, pBuf, dataLen);
	log_info("Lan report state, dst:%d.%d.%d.%d:%d\n", 
		(u8_t)(ip->addr), (u8_t)(ip->addr >> 8), (u8_t)(ip->addr >> 16), (u8_t)(ip->addr >> 24), port);
}


/****************************************************************************/
/* FAN LOCKED check                               */

void Fan_Switch(uint8_t gate)
{

	uint8_t switchgate;
	switchgate = gate;
	switch (switchgate)
	{
	case S1:
	{
		// 0 0 0 
		HAL_GPIO_WritePin(GPIOC, FAN3_EN_Pin | FAN1_EN_Pin | FAN2_EN_Pin, GPIO_PIN_RESET);
	}
	break;
	case S2:
	{
		//001
		HAL_GPIO_WritePin(GPIOC, FAN1_EN_Pin | FAN2_EN_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, FAN3_EN_Pin, GPIO_PIN_SET);
	}
	break;
	case S3:
	{
		//010
		HAL_GPIO_WritePin(GPIOC, FAN1_EN_Pin | FAN3_EN_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, FAN2_EN_Pin, GPIO_PIN_SET);
	}
	break;
	case S4:
	{
		//011
		HAL_GPIO_WritePin(GPIOC, FAN1_EN_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, FAN3_EN_Pin | FAN2_EN_Pin, GPIO_PIN_SET);
	}
	break;
	case S5:
	{
		//100
		HAL_GPIO_WritePin(GPIOC, FAN3_EN_Pin | FAN2_EN_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, FAN1_EN_Pin, GPIO_PIN_SET);
	}
	break;
	case S6:
	{
		//101
		HAL_GPIO_WritePin(GPIOC, FAN2_EN_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, FAN1_EN_Pin | FAN3_EN_Pin, GPIO_PIN_SET);
	}
	break;
	case S7:
	{
		//110
		HAL_GPIO_WritePin(GPIOC, FAN3_EN_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOC, FAN1_EN_Pin | FAN2_EN_Pin, GPIO_PIN_SET);
	}
	break;
	case S8:
	{
		//111
		HAL_GPIO_WritePin(GPIOC, FAN1_EN_Pin | FAN2_EN_Pin | FAN3_EN_Pin, GPIO_PIN_SET);
	}
	break;
	default:
		break;
	}
	// if(gate > 8 )
	// return;
	// static GPIO_PinState bit0_pin_lvl = GPIO_PIN_RESET ;
	// static GPIO_PinState bit1_pin_lvl = GPIO_PIN_RESET ;
	// static GPIO_PinState bit2_pin_lvl = GPIO_PIN_RESET ;

	// bit0_pin_lvl = (gate & 0x01) != 0 ? GPIO_PIN_SET : GPIO_PIN_RESET;
	// bit1_pin_lvl = (gate & 0x02) != 0 ? GPIO_PIN_SET : GPIO_PIN_RESET;
	// bit2_pin_lvl = (gate & 0x04) != 0 ? GPIO_PIN_SET : GPIO_PIN_RESET;
	// HAL_GPIO_WritePin(GPIOC , FAN3_EN_Pin , bit0_pin_lvl);
	// HAL_GPIO_WritePin(GPIOC , FAN2_EN_Pin , bit1_pin_lvl);
	// HAL_GPIO_WritePin(GPIOC , FAN1_EN_Pin , bit2_pin_lvl);

}

void Fan_LOCKED_check(uint8_t sw)
{
#if (Projector_Model == LP92SLB || Projector_Model == NP72BN)
	/**
	 * IDX        ADG PIN NAME      WIRE NAME     HW INTERFACE     FAN NAME
	 * 1            S1_ADG        	FAN5_LOCKD1		FAN5			��������1
	 * 2            S2_ADG         	FAN1_LOCKD5		FAN1			����������1
	 * 3            S3_ADG         	FAN14_LOCKD		FAN_L4			��������2
	 * 4            S4_ADG         	FAN13_LOCKD		FAN_L3			DMD����
	 * 5            S5_ADG         	FAN12_LOCKD		FAN_W1			����������3
	 * 6            S6_ADG         	FAN11_LOCKD		FAN_W2			����������
	 * 7            S7_ADG         	FAN10_LOCKD		FAN_L1			-
	 * 8			S8_ADG			FAN9_LOCKD		FAN_L2			-
	 */
	//NOTE�� ����������2δ����lock���

	// printf(" FAN_Gate_START=%d \r\n", sw);
	uint8_t i;
	Fan_Switch(sw);
	HAL_Delay(1);
	if (sw == S1)
	{
		for (i = 0; i < 10; i++)
		{
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
			// printf("%d,",ret);
		}
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == GPIO_PIN_SET)
		{
			FAN_1_FLAG_COUNT++;
			if (FAN_1_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_1_FLAG_COUNT = 0;
				FAN_1_FLAG = 1;  //�ϱ���MSTAR�ķ���״̬
				FAN_Error1_Buffer[0] = FAN_1_Warn_Code;  //�ϱ���MSTAR�ķ���״̬
				Fan_Err = Fan_Err | 0x01;	//�ϱ���XML�ķ���״̬���
				printf("FAN_1 Error\r\n");
			}
		}
		else
		{
			FAN_1_FLAG = 0;
			FAN_1_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & (~FAN_1_Warn_Code);
			Fan_Err = Fan_Err & (~(0X01));
			FAN_Error1_Buffer[0] = 0;
			printf("Fan 1 ok\n");
		}
	}
	else if (sw == S2)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			FAN_2_FLAG_COUNT++;
			if (FAN_2_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_2_FLAG_COUNT = 0;
				FAN_2_FLAG = 1;
				FAN_Error1_Buffer[1] = FAN_2_Warn_Code;
				Fan_Err = Fan_Err | 0x02;
				printf("FAN_2 Error\r\n");
			}
		}
		else
		{
			FAN_2_FLAG = 0;
			FAN_2_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & (~FAN_2_Warn_Code);
			Fan_Err = Fan_Err & (~(0X02));
			FAN_Error1_Buffer[1] = 0;
			printf("Fan 2 ok\n");
		}
	}
	else if (sw == S3)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			FAN_3_FLAG_COUNT++;
			if (FAN_3_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_3_FLAG_COUNT = 0;
				FAN_3_FLAG = 1;
				FAN_Error1_Buffer[2] = FAN_3_Warn_Code;
				Fan_Err = Fan_Err | 0x03;
				printf("FAN_3 Error\r\n");
			}
		}
		else
		{
			FAN_3_FLAG = 0;
			FAN_3_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & (~FAN_3_Warn_Code);
			Fan_Err = Fan_Err & (~(0X03));
			FAN_Error1_Buffer[2] = 0;
			printf("Fan 3 ok\n");
		}
	}
	else if (sw == S4)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			FAN_4_FLAG_COUNT++;
			if (FAN_4_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_4_FLAG_COUNT = 0;
				FAN_4_FLAG = 1;
				FAN_Error1_Buffer[3] = FAN_4_Warn_Code;
				Fan_Err = Fan_Err | 0x04;
				printf("FAN_4 Error\r\n");
			}
		}
		else
		{
			FAN_4_FLAG = 0;
			FAN_4_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & (~FAN_4_Warn_Code);
			Fan_Err = Fan_Err & (~(0X04));
			FAN_Error1_Buffer[3] = 0;
			printf("Fan 4 ok\n");
		}
	}
	else if (sw == S5)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			FAN_5_FLAG_COUNT++;
			if (FAN_5_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_5_FLAG_COUNT = 0;
				FAN_5_FLAG = 1;
				FAN_Error1_Buffer[4] = FAN_5_Warn_Code;
				Fan_Err = Fan_Err | 0x05;
				printf("FAN_5 Error\r\n");
			}
		}
		else
		{
			FAN_5_FLAG = 0;
			FAN_5_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & (~FAN_5_Warn_Code);
			Fan_Err = Fan_Err & (~(0X05));
			FAN_Error1_Buffer[4] = 0;
			printf("Fan 5 ok\n");
		}
	}
	else if (sw == S6)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			FAN_6_FLAG_COUNT++;
			if (FAN_6_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_6_FLAG_COUNT = 0;
				FAN_6_FLAG = 1;
				FAN_Error1_Buffer[5] = FAN_6_Warn_Code;
				Fan_Err = Fan_Err | 0x06;
				printf("FAN_6 Error\r\n");
			}
		}
		else
		{
			FAN_6_FLAG = 0;
			FAN_6_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & (~FAN_6_Warn_Code);
			Fan_Err = Fan_Err & (~(0X06));
			FAN_Error1_Buffer[5] = 0;
			printf("Fan 6 ok\n");
		}
	}
	else if (sw == S7)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			FAN_7_FLAG_COUNT++;
			if (FAN_7_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_7_FLAG_COUNT = 0;
				FAN_7_FLAG = 1;
				FAN_Error1_Buffer[6] = FAN_7_Warn_Code;
				Fan_Err = Fan_Err | 0x07;
				printf("FAN_7 Error\r\n");
			}
		}
		else
		{
			FAN_7_FLAG = 0;
			FAN_7_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & (~FAN_7_Warn_Code);
			Fan_Err = Fan_Err & (~(0X07));
			FAN_Error1_Buffer[6] = 0;
			printf("Fan 7 ok\n");
		}
	}
#if 0
	if (sw == S8)
	{
		if (FAN_1_FLAG || FAN_2_FLAG || FAN_3_FLAG || FAN_4_FLAG || FAN_5_FLAG || FAN_6_FLAG || FAN_7_FLAG ||
			FAN_8_FLAG || PUMP_EN1_FLAG || FAN_10_FLAG || FAN_12_FLAG || FAN_9_FLAG || FAN_11_FLAG ||
			FAN_13_FLAG || FAN_14_FLAG)
		{
			//  HAL_GPIO_WritePin(MCU0_IRQ_GPIO_Port,MCU0_IRQ_Pin,GPIO_PIN_RESET);
			Fan_warning = 1;
			Fan_Error = 1;
			printf("fan error_check=0x%x\r\n", Fan_Error);
		}
		else
		{
			Fan_warning = 0;
			Fan_Error = 0;
			//				 Warn_Type=0;
			printf("Fan_NO_Warning\r\n");
			for (i = 0; i < 8; i++)
			{
				FAN_Error1_Buffer[i] = 0;
				FAN_Error2_Buffer[i] = 0;
			}
		}
	}
#endif

#else
	/**
	 * IDX        ADG PIN NAME      WIRE NAME     HW INTERFACE     FAN NAME     NOTE
	 * 1         	U116_S1		     PUMP1_EN		FAN_B1			-           正常检测
	 * 2            -               -				-				-			不检测	
	 * 3            U116_S3     	FAN14_LOCKD		FAN_L4			-			正常检测
	 * 4            U116_S4     	FAN13_LOCKD		FAN_L3			-			正常检测
	 * 5            U116_S5     	FAN12_LOCKD		FAN_W1			-			正常检测
	 * 6            -	         	-				-				-			不检测
	 * 7            U116_S7     	FAN2_LOCKD		FAN_5			-			正常检测
	 * 8			U116_S8			FAN1_LOCKD		FAN_1			-			正常检测
	 */
	printf(" FAN_Gate_START=%d \r\n", sw);
	uint8_t i;
	// 切换芯片到对应的检测点
	Fan_Switch(sw);
	//开始读取引脚电平判断lock状态
	if (sw == S1)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			PUMP_EN1_FLAG_COUNT++;
			if (PUMP_EN1_FLAG_COUNT >= Fan_TEST_count)
			{
				PUMP_EN1_FLAG_COUNT = 0;
				PUMP_EN1_FLAG = 1;
				FAN_Error2_Buffer[7] = PUMP_EN1_Warn_Code;
				Fan_Err = Fan_Err | 0x01;
				printf("FAN_B1(0x80) Error\r\n");
			}
		}
		else
		{
			Fan_Err = Fan_Err & 0xFE;
			PUMP_EN1_FLAG = 0;
			PUMP_EN1_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & 0x7F;
			FAN_Error2_Buffer[7] = 0;
		}
	}
	else if(sw==S2)
	{
		//pass
	}
	else if (sw == S3)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			FAN_14_FLAG_COUNT++;
			if (FAN_14_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_14_FLAG_COUNT = 0;
				FAN_14_FLAG = 1;
				FAN_Error2_Buffer[5] = FAN_14_Warn_Code;
				Fan_Err = Fan_Err | 0x02;
				printf("FAN_L4(0x20) Error\r\n");
			}
		}
		else
		{
			Fan_Err = Fan_Err & 0xFD;
			FAN_14_FLAG = 0;
			FAN_14_FLAG_COUNT = 0;
			FAN_Warning[1] = FAN_Warning[1] & 0xDF;
			FAN_Error2_Buffer[5] = 0;
		}
	}
	else if (sw == S4)
	{
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
		{
			FAN_13_FLAG_COUNT++;
			if (FAN_13_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_13_FLAG_COUNT = 0;
				FAN_13_FLAG = 1;
				FAN_Error2_Buffer[4] = FAN_13_Warn_Code;
				Fan_Err = Fan_Err | 0x04;
				printf("FAN_L3(0x10) Error\r\n");
			}
		}
		else
		{
			Fan_Err = Fan_Err & 0xFB;
			FAN_Warning[1] = FAN_Warning[1] & 0xEF;
			FAN_13_FLAG = 0;
			FAN_13_FLAG_COUNT = 0;
			FAN_Error2_Buffer[4] = 0;
		}
	}
    else if (sw == S5)
    {
        for (i = 0; i < 10; i++)
            HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin);
        if (HAL_GPIO_ReadPin(FAN_LOCKD2_GPIO_Port, FAN_LOCKD2_Pin) == 1)
        {
            FAN_12_FLAG_COUNT++;
            if (FAN_12_FLAG_COUNT >= Fan_TEST_count)
            {
                FAN_12_FLAG_COUNT = 0;
                FAN_12_FLAG = 1;
                FAN_Error2_Buffer[3] = FAN_12_Warn_Code;
				Fan_Err = Fan_Err | 0x08;
                printf("FAN_W1(0x08) Error\r\n");
            }
        }
        else
        {
            FAN_Warning[1] = FAN_Warning[1] & 0xF7;
            FAN_12_FLAG = 0;
            FAN_12_FLAG_COUNT = 0;
            FAN_Error2_Buffer[3] = 0;
        }
    }
	else if(sw==S6)
	{
		//pass
	}
	else if (sw == S7)
	{ // 20230404
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD1_GPIO_Port, FAN_LOCKD1_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD1_GPIO_Port, FAN_LOCKD1_Pin) == 1)
		{
			FAN_5_FLAG_COUNT++;
			if (FAN_5_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_5_FLAG_COUNT = 0;
				FAN_5_FLAG = 1;
				FAN_Error1_Buffer[1] = FAN_5_Warn_Code;
				Fan_Err = Fan_Err | 0x10;
				printf("Fan_5(0x04) Error\r\n");
			}
		}
		else
		{
			Fan_Err = Fan_Err & 0xEF;
			FAN_5_FLAG = 0;
			FAN_5_FLAG_COUNT = 0;
			FAN_Warning[0] = FAN_Warning[0] & 0xFB;
			FAN_Error1_Buffer[1] = 0;
		}
	}
    else if (sw == S8)
    {
		for (i = 0; i < 10; i++)
			HAL_GPIO_ReadPin(FAN_LOCKD1_GPIO_Port, FAN_LOCKD1_Pin);
		if (HAL_GPIO_ReadPin(FAN_LOCKD1_GPIO_Port, FAN_LOCKD1_Pin) == 1)
		{
			FAN_1_FLAG_COUNT++;
			if (FAN_1_FLAG_COUNT >= Fan_TEST_count)
			{
				FAN_1_FLAG_COUNT = 0;
				FAN_1_FLAG = 1;
                FAN_Error1_Buffer[0] = FAN_1_Warn_Code;
				Fan_Err = Fan_Err | 0x20;
				printf("Fan_1(0x01) Error\r\n");
			}
		}
		else
		{
			Fan_Err = Fan_Err & 0xDF;
			FAN_1_FLAG = 0;
			FAN_1_FLAG_COUNT = 0;
			FAN_Warning[0] = FAN_Warning[0] & 0xFE;
			FAN_Error1_Buffer[0] = 0;
		}
    }

#if 0
	//如果检测到最后一个风扇，则进行统计。
    if (sw == S8)
	{
		if (FAN_1_FLAG || FAN_2_FLAG || FAN_3_FLAG || FAN_4_FLAG || FAN_5_FLAG || FAN_6_FLAG || FAN_7_FLAG ||
			FAN_8_FLAG || PUMP_EN1_FLAG || FAN_10_FLAG || FAN_12_FLAG || FAN_9_FLAG || FAN_11_FLAG ||
			FAN_13_FLAG || FAN_14_FLAG)
		{
			//  HAL_GPIO_WritePin(MCU0_IRQ_GPIO_Port,MCU0_IRQ_Pin,GPIO_PIN_RESET);
			Fan_warning = 1;
			Fan_Error = 1;
			printf("fan error_check=0x%x\r\n", Fan_Error);
		}
		else
		{
			Fan_warning = 0;
			Fan_Error = 0;
			//				 Warn_Type=0;
			printf("Fan_NO_Warning\r\n");
			for (i = 0; i < 8; i++)
			{
				FAN_Error1_Buffer[i] = 0;
				FAN_Error2_Buffer[i] = 0;
			}
		}
	}
#endif

#endif
}

int Fan_Error_warn(void)
{
	FAN_Warning[0] = 0 ;
	FAN_Warning[1] = 0 ;
	
	// if (FAN_1_FLAG || FAN_2_FLAG || FAN_3_FLAG || FAN_4_FLAG || FAN_5_FLAG || FAN_6_FLAG || FAN_7_FLAG || FAN_8_FLAG)
	// {
		// for (size_t i = 0; i < 8; i++)
		// {
		// 	printf("%d ", FAN_Error1_Buffer[i]);
		// }
		
		FAN_Warning[0] |= FAN_Error1_Buffer[0] | FAN_Error1_Buffer[1] | FAN_Error1_Buffer[2] | FAN_Error1_Buffer[3] |
						 FAN_Error1_Buffer[4] | FAN_Error1_Buffer[5] | FAN_Error1_Buffer[6] | FAN_Error1_Buffer[7];
		printf("FAN_Warning[0]:0x%x\r\n", FAN_Warning[0]);
		if (FAN_Warning[0] != FAN_Temp_Error[0])
		{
			FAN_Temp_Error[0] = FAN_Warning[0];
		}
	// }

	if (FAN_9_FLAG || FAN_10_FLAG || FAN_11_FLAG || FAN_12_FLAG || FAN_13_FLAG || FAN_14_FLAG || PUMP_EN0_FLAG || PUMP_EN1_FLAG)
	{
		FAN_Warning[1] |= FAN_Error2_Buffer[0] | FAN_Error2_Buffer[1] | FAN_Error2_Buffer[2] | FAN_Error2_Buffer[3] |
						 FAN_Error2_Buffer[4] | FAN_Error2_Buffer[5] | FAN_Error2_Buffer[6] | FAN_Error2_Buffer[7];
		printf("FAN_Warning[1]:0x%x\r\n", FAN_Warning[1]);
		if (FAN_Warning[1] != FAN_Temp_Error[1]) // leo 20230916
		{
			FAN_Temp_Error[1] = FAN_Warning[1];
		}
	}
	
	if(FAN_Warning[0] || FAN_Warning[1]) //�ж��Ƿ����쳣�����쳣�򷵻�1
		return 1;
	else
		return 0;
}
#if 1
int Tempture_Erorr_Test(void)
{
	uint8_t j;
	if ((ADS_Temp_Data[0] > TMP_R_LD1_set))
	{
		TMP_R_LD1_Flag_COUNT++;
		if (TMP_R_LD1_Flag_COUNT >= Temp_TEST_count)
		{
			TMP_R_LD1_Flag_COUNT = 0;
			TMP_R_LD1_Flag = 1;
			Tempture_Error_Buffer[0] = TMP_R_LD1_Error_Code;
			//					Warn_Type=TMP_R_LD1_Error_Code;
			printf("LD_R1 TEMP OVER:%.1f\r\n", TMP_R_LD1_set);
		}
		//				else
		//					TMP_R_LD1_Flag=0;
	}
	else
	{
		TMP_R_LD1_Flag = 0;
		TMP_R_LD1_Flag_COUNT = 0;
		Tempture_Error_Buffer[0] = 0;
	}
	if (ADS_Temp_Data[1] > TMP_R_LD2_set)
	{
		TMP_R_LD2_Flag_COUNT++;
		if (TMP_R_LD2_Flag_COUNT >= Temp_TEST_count)
		{
			TMP_R_LD2_Flag_COUNT = 0;
			TMP_R_LD2_Flag = 1;
			Tempture_Error_Buffer[1] = TMP_R_LD2_Error_Code;
			//					Warn_Type=TMP_R_LD2_Error_Code;
			printf("LD_R2 TEMP OVER:%.1f\r\n", TMP_R_LD2_set);
		}
		//				else
		//					TMP_R_LD2_Flag=0;
	}
	else
	{
		Tempture_Error_Buffer[1] = 0;
		TMP_R_LD2_Flag = 0;
		TMP_R_LD2_Flag_COUNT = 0;
	}
	if (ADS_Temp_Data[2] > TMP_G_LD1_set)
	{
		TMP_G_LD1_Flag_COUNT++;
		if (TMP_G_LD1_Flag_COUNT >= Temp_TEST_count)
		{
			TMP_G_LD1_Flag_COUNT = 0;
			TMP_G_LD1_Flag = 1;
			Tempture_Error_Buffer[2] = TMP_G_LD1_Error_Code;
			//					  Warn_Type=TMP_G_LD1_Error_Code;
			printf("LD_G1 TEMP OVER:%.1f\r\n", TMP_G_LD1_set);
		}
		//				else
		//					TMP_G_LD1_Flag=0;
	}
	else
	{
		Tempture_Error_Buffer[2] = 0;
		TMP_G_LD1_Flag = 0;
		TMP_G_LD1_Flag_COUNT = 0;
	}
	if (ADS_Temp_Data[3] > TMP_G_LD2_set)
	{
		TMP_G_LD2_Flag_COUNT++;
		if (TMP_G_LD2_Flag_COUNT >= Temp_TEST_count)
		{
			TMP_G_LD2_Flag_COUNT = 0;
			TMP_G_LD2_Flag = 1;
			Tempture_Error_Buffer[3] = TMP_G_LD2_Error_Code;
			//					  Warn_Type=TMP_G_LD2_Error_Code;
			printf("LD_G2 TEMP OVER:%.1f\r\n", TMP_G_LD2_set);
		}
		//				else
		//					TMP_G_LD2_Flag=0;
	}
	else
	{
		Tempture_Error_Buffer[3] = 0;
		TMP_G_LD2_Flag = 0;
		TMP_G_LD2_Flag_COUNT = 0;
	}
	if (ADS_Temp_Data[4] > TMP_B_LD_set)
	{
		TMP_B_LD_Flag_COUNT++;
		if (TMP_B_LD_Flag_COUNT >= Temp_TEST_count)
		{
			TMP_B_LD_Flag_COUNT = 0;
			TMP_B_LD_Flag = 1;
			Tempture_Error_Buffer[4] = TMP_B_LD_Error_Code;
			//					  Warn_Type=TMP_B_LD_Error_Code;
			printf("LD_B TEMP OVER:%.1f\r\n", TMP_B_LD_set);
		}
		//				else
		//					TMP_B_LD_Flag=0;
	}
	else
	{
		Tempture_Error_Buffer[4] = 0;
		TMP_B_LD_Flag = 0;
		TMP_B_LD_Flag_COUNT = 0;
	}
#if Temp_B2
	if (ADS_Temp_Data[6] > TMP_B_LD2_set)
	{
		TMP_B_LD2_Flag_COUNT++;
		if (TMP_B_LD2_Flag_COUNT >= Temp_TEST_count)
		{
			TMP_B_LD2_Flag_COUNT = 0;
			TMP_B_LD2_Flag = 1;
			Tempture_Error_Buffer[5] = TMP_B_LD2_Error_Code;
			//					  Warn_Type=TMP_B_LD_Error_Code;
			printf("LD_B2 TEMP OVER:%.1f\r\n", TMP_B_LD2_set);
		}
		//				else
		//					TMP_B_LD_Flag=0;
	}
	else
	{
		Tempture_Error_Buffer[5] = 0;
		TMP_B_LD2_Flag = 0;
		TMP_B_LD2_Flag_COUNT = 0;
	}
#endif

	if (DMD_Temp > TMP_DMD_set)
	{
		TMP_DMD_Flag_COUNT++;
		if (TMP_DMD_Flag_COUNT >= Temp_TEST_count)
		{
			TMP_DMD_Flag_COUNT = 0;
			TMP_DMD_Flag = 1;
			Tempture_Error_Buffer[6] = TMP_DMD_Error_Code;
			//					  Warn_Type=TMP_DMD_Error_Code;
			printf("DMD TEMP OVER:%.1f\r\n", TMP_DMD_set);
		}
		else
			TMP_DMD_Flag = 0;
	}
	else
	{
		Tempture_Error_Buffer[6] = 0;
		TMP_DMD_Flag = 0;
		TMP_DMD_Flag_COUNT = 0;
	}

	if (ADS_Temp_Data[7] > TEC_CFB_set)
	{
		TES_CFB_Flag_COUNT++;
		if (TES_CFB_Flag_COUNT >= Temp_TEST_count)
		{
			TES_CFB_Flag_COUNT = 0;
			TES_CFB_Flag = 1;
			Tempture_Error_Buffer[7] = TMP_TEC_Error_Code;
			//					  Warn_Type=TES_CFB_Error_Code;
			printf("Environment TMP OVER:%.1f\r\n", TEC_CFB_set);
		}
		//				else
		//					TES_CFB_Flag=0;
	}
	else
	{
		Tempture_Error_Buffer[7] = 0;
		TES_CFB_Flag = 0;
		TES_CFB_Flag_COUNT = 0;
	}
	if (TMP_R_LD1_Flag || TMP_R_LD2_Flag || TMP_G_LD1_Flag || TMP_G_LD2_Flag ||
		TMP_B_LD_Flag || TES_CFB_Flag || TMP_DMD_Flag || TMP_B_LD2_Flag) // TMP_TEC_Flag          || TES_CFB_Flag     )

	{
		//      HAL_GPIO_WritePin(MCU_FAN_IRQ_GPIO_Port,MCU_FAN_IRQ_Pin,GPIO_PIN_RESET);
		Warn_Type = Warn_Type | Tempture_Error_Buffer[7] | Tempture_Error_Buffer[6] | Tempture_Error_Buffer[5] | Tempture_Error_Buffer[4] |
					Tempture_Error_Buffer[3] | Tempture_Error_Buffer[2] | Tempture_Error_Buffer[1] | Tempture_Error_Buffer[0];
		Tmp_warning = 1;
		Tmp_Error = 1;
		if (Warn_Type != FAN_Temp_Error[2]) // leo 20230916
		{
			FAN_Temp_Error[2] = Warn_Type;
		}
		printf("Temp error_check=0x%x\r\n", Tmp_Error);
		return 1;
	}
	else
	{
		Warn_Type = 0;
		Tmp_warning = 0;
		Tmp_Error = 0;
		for (j = 0; j < 8; j++)
		{
			Tempture_Error_Buffer[j] = 0;
		}
		//				Warn_Type=0;
		printf(" TEMP_NO_Warning \r\n");
		return 0;
	}
}

void Error_IRQ_Reset(void)
{
	if (Fan_Error == 1)
	{
		Fan_Error = 0;
	}
	if (Tmp_Error == 1)
	{
		Tmp_Error = 0;
	}
	// HAL_GPIO_WritePin(MCU0_IRQ_GPIO_Port,MCU0_IRQ_Pin,GPIO_PIN_SET);
}

#endif

/**
 * @brief �������Ƿ��쳣��װ, ���ڸ�PJLINK����
 * 
 * @return int 
 */
inline int check_fan_err(void)
{
	return Fan_Error_warn();  
}

/**
 * @brief ����¶��Ƿ��쳣��װ�����ڸ�pjlink����
 * 
 * @return int 
 */
inline int check_temp_err(void)
{
	return Tempture_Erorr_Test();
}
