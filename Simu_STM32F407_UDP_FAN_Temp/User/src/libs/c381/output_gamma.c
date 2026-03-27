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

void OutputGammaEn(void)
{
#if 0//HPC add
	if ( PM_GAMMA_EN == 0 ) {
		C381_Write_Bank_Reg(14, B14_GMCT3, 0x00);
	}
	else {
		C381_Write_Bank_Reg(14, B14_GMCT3, 0x05);
	}
#endif//HPC add
}

void CalcOutputGammaTable( int16_t color )
{
	int16_t i;
	float gm;
	int16_t val[2];
	float coef[2];
	int16_t area;
	float w, a, b;

	area = 0;
	coef[0] = ( color == 1 ) ? PM_GAMMA[area][1] : (( color == 2 ) ? PM_GAMMA[area][2] : PM_GAMMA[area][0]);
	coef[1] = ( color == 1 ) ? PM_GAMMA[area+1][1] :( ( color == 2 ) ? PM_GAMMA[area+1][2] : PM_GAMMA[area+1][0]);
	val[0] = 0;
	val[1] = ( (area + 1) >= (Def_OGM_NUM - 1) ) ? 1024 : PM_GAMMA_PIX[area+1];
	w = (float)( val[1] - val[0] );

	for ( i = 0; i < 1024; i++ ) {
		while ( i > val[1] ) {
			area++;
			coef[0] = ( color == 1 ) ? PM_GAMMA[area][1] : (( color == 2 ) ? PM_GAMMA[area][2] : PM_GAMMA[area][0]);
			coef[1] = ( color == 1 ) ? PM_GAMMA[area+1][1] :( ( color == 2 ) ? PM_GAMMA[area+1][2] : PM_GAMMA[area+1][0]);
			val[0] = PM_GAMMA_PIX[area];
			val[1] = ( (area + 1) >= (Def_OGM_NUM - 1) ) ? 1024 : PM_GAMMA_PIX[area+1];
			w = (float)( val[1] - val[0] );
		}

		b = (float)( i - val[0] ) / w;
		a = 1 - b; 
		gm = ( pow( ((float)i / 1023), 1 / coef[0] ) * a + pow( ((float)i / 1023), 1 / coef[1] ) * b ) * 65536 + 0.5f;
		GV_TEMP_GAMMA_TABLE[i] = ( gm < 0 ) ? 0 : (( gm > 65535 ) ? 65535 : (int16_t)gm);
	}
}

void OutputGammaTable( int16_t color )
{
#if 0//HPC add
	int16_t i;
	int16_t rtct_org = GV_RTCT;
  GV_RTCT = 0x0000;
	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//		HAL_Delay(100);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));


	if ( color == 0 ) {
		C381_Write_Bank_Reg(14, B14_GMCT3, 0x84);
	}
	else if ( color == 1 ) {
		C381_Write_Bank_Reg(14, B14_GMCT3, 0x94);
	}
	else if ( color == 2 ) {
		C381_Write_Bank_Reg(14, B14_GMCT3, 0xa4);
	}
	else {
		C381_Write_Bank_Reg(14, B14_GMCT3, 0xb4);
	}
	C381_Write_Bank_Reg(14, B14_GMAD3, 0x00);
	C381_Write_Bank_Reg(14, B14_GMAD3, 0x00);

	for ( i = 0; i < 1024; i++ ) {
		C381_Write_Bank_Reg(14, B14_GMDT3, GV_TEMP_GAMMA_TABLE[i] & 0xff);
		C381_Write_Bank_Reg(14, B14_GMDT3, (GV_TEMP_GAMMA_TABLE[i]>>8) & 0xff);
	}

	OutputGammaEn();
GV_RTCT = rtct_org;
			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
		//		HAL_Delay(100);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));
#endif//HPC add
}
