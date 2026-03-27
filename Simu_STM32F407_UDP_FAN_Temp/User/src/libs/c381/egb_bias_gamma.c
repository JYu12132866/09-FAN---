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

int16_t PM_EBIAS_GMDT[16][3][16];

static float ebgmLevel[16] = { 0, 4, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 1024 };
static int16_t ebTestPlt[16][3] = {
	{ 0x00, 0x00, 0x00 },// plt0
	{ 0xFF, 0x00, 0x00 },// plt1
	{ 0x00, 0xFF, 0x00 },// plt2
	{ 0x00, 0x00, 0xFF },// plt3
	{ 0xFF, 0xFF, 0x00 },// plt4
	{ 0xFF, 0x00, 0xFF },// plt5
	{ 0x00, 0xFF, 0xFF },// plt6
	{ 0xFF, 0x40, 0x80 },// plt7
	{ 0x80, 0xFF, 0x40 },// plt8
	{ 0x40, 0x80, 0xFF },// plt9
	{ 0xFF, 0x80, 0x40 },// plt10
	{ 0x40, 0xFF, 0x80 },// plt11
	{ 0x80, 0x40, 0xFF },// plt12
	{ 0x80, 0x80, 0x40 },// plt13
	{ 0x80, 0x40, 0x80 },// plt14
	{ 0x40, 0x80, 0x80 }// plt15
};

void CalcEgbBiasGammaTable( int16_t color, int16_t plt, int16_t allplt )
{
	int16_t lv;
	int16_t col;
	float coef;
	float bias;
	float gm;

	if ( allplt == 0 ) {
		for ( col = 0; col < 3; col++ ) {
			if ( (col == color) || (color == 3) ) {
				coef = ( PM_EBIAS_AREA_TEST == 0 ) ? PM_EBIAS_GAMMA[col] : 1;
				bias = ( PM_EBIAS_AREA_TEST == 0 ) ? (float)PM_EBIAS[plt][col] : (float)ebTestPlt[plt][col];
				for ( lv = 0; lv < 16; lv++ ) {
					gm = pow( (pow( ebgmLevel[lv], coef ) + pow( bias, coef )), 1/coef ) - ebgmLevel[lv];
					PM_EBIAS_GMDT[plt][col][lv] = ( gm < 0 ) ? 0 : (( gm > 255 ) ? 255 : (int16_t)gm); 
				}
			}
		}
	}
	else {
		for ( plt = 0; plt < 16; plt++ ) {
			for ( col = 0; col < 3; col++ ) {
				coef = ( PM_EBIAS_AREA_TEST == 0 ) ? PM_EBIAS_GAMMA[col] : 1;
				bias = ( PM_EBIAS_AREA_TEST == 0 ) ? (float)PM_EBIAS[plt][col] : (float)ebTestPlt[plt][col];
				for ( lv = 0; lv < 16; lv++ ) {
					gm = pow( (pow( ebgmLevel[lv], coef ) + pow( bias, coef )), 1/coef ) - ebgmLevel[lv];
					PM_EBIAS_GMDT[plt][col][lv] = ( gm < 0 ) ? 0 :( ( gm > 255 ) ? 255 : (int16_t)gm); 
				}
			}
		}
	}
}

void EgbBiasGammaTable( int16_t color, int16_t plt, int16_t allplt )
{
#if 0//HPC add
	int16_t lv;
	int16_t ad;
	int16_t acct;
	int16_t rtct_org = GV_RTCT;
	GV_RTCT = 0x0000;

	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));


	acct = ( color == 0 ) ? 0x1 : (( color == 1 ) ? 0x3 :( ( color == 2 ) ? 0x5 : 0x7));
	C381_Write_Bank_Reg(16, B16_EGBIASCT, (uint8_t)(0x0050 + (acct << 1)));
	C381_Write_Bank_Reg(16, B16_EGBIASCT2,(uint8_t)(( 0x0050 + (acct << 1))>>8));


	if ( allplt == 0 ) {
		ad = ((plt < 16) && (plt >= 0)) ? (plt << 4) : 0;
		C381_Write_Bank_Reg(16, B16_EGBIASAD, ad & 0xff);

		if ( (color >= 0) && (color < 3) ) {
			for ( lv = 0; lv < 16; lv++ ) {
				C381_Write_Bank_Reg(16, B16_EGBIASDT, PM_EBIAS_GMDT[plt][color][lv] & 0xff);
				
			}
		}
		else {
			for ( lv = 0; lv < 16; lv++ ) {
	            
				C381_Write_Bank_Reg(16, B16_EGBIASDT, PM_EBIAS_GMDT[plt][0][lv] & 0xff);
			}
		}
	}
	else {
		C381_Write_Bank_Reg(16, B16_EGBIASAD, 0x00);

		for ( plt = 0; plt < 16; plt++ ) {
			if ( (color >= 0) && (color < 3) ) {
				for ( lv = 0; lv < 16; lv++ ) {
					C381_Write_Bank_Reg(16, B16_EGBIASDT, PM_EBIAS_GMDT[plt][color][lv] & 0xff);
				}
			}
			else {
				for ( lv = 0; lv < 16; lv++ ) {
					C381_Write_Bank_Reg(16, B16_EGBIASDT, PM_EBIAS_GMDT[plt][0][lv] & 0xff);
				}
			}
		}
	}

	EgbBiasEn();

	GV_RTCT = rtct_org;

	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//		HAL_Delay(100);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));


#endif//HPC add
}
