// Copyright (C) 2016 i-Chips Technology Inc. All Rights Reserved.
#include "headers.h"

#include <string.h>
#include <stdio.h>
//#include <conio.h>			
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include "arm_math.h"
//#include "stm32f4xx_conf.h"


#include "declare_gDef.h"
#include "declare_gPM.h"
#include "declare_gPS.h"
#include "declare_gPI.h"
#include "declare_gVar.h"
#include "declare_gFunc.h"
#include "individual_param.h"
#include "c381.h"

int16_t GV_TEMP_GAMMA_TABLE[1024];
void EgbGainGammaEn(void)
{
	if ( PM_EGB_GAMMA_EN == 0 ) {
		GV_EGBCT = GV_EGBCT & 0x000f;
	}
	else {
		GV_EGBCT = GV_EGBCT | 0x0010;
	}
#if 0//HPC add
	C381_Write_Bank_Reg(16, B16_EGBCT, (uint8_t)GV_EGBCT);
	C381_Write_Bank_Reg(16, B16_EGBCT2,(uint8_t)( GV_EGBCT>>8));
#endif//HPC add
}

void CalcEgbGainGammaTable( int16_t color )
{
	int16_t i;
	float coef;
	float gm;

	coef = ( color == 1 ) ? PM_EGB_GAMMA[1] :( ( color == 2 ) ? PM_EGB_GAMMA[2] : PM_EGB_GAMMA[0]);
	for ( i = 0; i < 1024; i++ ) {
		gm  = pow( ((float)i / 1024), 1 / coef ) * 65536 + 0.5f;
		GV_TEMP_GAMMA_TABLE[i] = ( gm < 0 ) ? 0 : ( gm > 65535 ) ? 65535 : (int16_t)gm;
	}
}

void EgbGainGammaTable( int16_t color )
{
#if 0//HPC add
	int16_t i;
	int16_t rtct_org = GV_RTCT;
	GV_RTCT = 0x0000;
	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));


	if ( color == 0 ) {
		
		C381_Write_Bank_Reg(16, B16_EGBCT, 0x20);
		C381_Write_Bank_Reg(16, B16_EGBCT2, 0x00);
	}
	else if ( color == 1 ) {
		C381_Write_Bank_Reg(16, B16_EGBCT, 0x60);
		C381_Write_Bank_Reg(16, B16_EGBCT2, 0x00);

	}
	else if ( color == 2 ) {
		C381_Write_Bank_Reg(16, B16_EGBCT, 0xA0);
		C381_Write_Bank_Reg(16, B16_EGBCT2, 0x00);

	}
	else {
		C381_Write_Bank_Reg(16, B16_EGBCT, 0xE0);
		C381_Write_Bank_Reg(16, B16_EGBCT2, 0x00);

	}
	C381_Write_Bank_Reg(16, B16_EGBGMAD, 0x00);
	C381_Write_Bank_Reg(16, B16_EGBGMAD, 0x00);

	for ( i = 0; i < 1024; i++ ) {
		C381_Write_Bank_Reg(16, B16_EGBGMDT,( GV_TEMP_GAMMA_TABLE[i] & 0xff));
		C381_Write_Bank_Reg(16, B16_EGBGMDT,(GV_TEMP_GAMMA_TABLE[i]>>8) & 0xff);
	}

	EgbGainGammaEn();
	GV_RTCT = rtct_org;

			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
		//		HAL_Delay(100);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

#endif//HPC add

}
