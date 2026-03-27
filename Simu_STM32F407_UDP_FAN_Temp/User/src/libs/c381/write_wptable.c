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

void WriteWpTable(void)
{
#if 0//HPC add
	int16_t x,y;
	int16_t adsft;
	int16_t x2k_ad_jmp;
	int16_t x2k_grd_jmp;
	int16_t x2k_ad_ysft;
	int16_t temp_i;
	int16_t xad;

	if( GV_WP_TABLE_COLOR == 0 ) {
		if( Def_Wp_Space == 32 ) {
			
			C381_Write_Bank_Reg(14,B14_DTCT,0x02);	
			C381_Write_Bank_Reg(14,B14_DTCT2,0x02);	
			adsft = 6;
			x2k_ad_jmp = 2560;
			x2k_grd_jmp = 64;
			x2k_ad_ysft = 1;
		}
		else {
			C381_Write_Bank_Reg(14,B14_DTCT,0x00);	
			C381_Write_Bank_Reg(14,B14_DTCT2,0x02);			
			adsft = 7;
			x2k_ad_jmp = 9984;
			x2k_grd_jmp = 128;
			x2k_ad_ysft = 2;// v053
		}
	}
	else {
		if( Def_Wp_Space == 32 ) {
			C381_Write_Bank_Reg(14,B14_DTCT,0x12);	
			C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
			adsft = 6;
			x2k_ad_jmp = 2560;
			x2k_grd_jmp = 64;
			x2k_ad_ysft = 1;
		}
		else {
			C381_Write_Bank_Reg(14,B14_DTCT,0x10);	
			C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
			adsft = 7;
			x2k_ad_jmp = 9984;
			x2k_grd_jmp = 128;
			x2k_ad_ysft = 2;// v053
		}
	}
	#if 1
	GV_RTCT = GV_RTCT & 0xfff8;

			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
	//	HAL_Delay(100);
		C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

	for( y = 0; y <= PS_WP_VW_GRD; y++ ) {
		C381_Write_Bank_Reg(14,B14_DTAD,(uint8_t)((y<<adsft)&0xff));	
		C381_Write_Bank_Reg(14,B14_DTAD,(uint8_t)((y>>(8-adsft))&0xff));
	
    	xad = 0;
		// Dummy area (HST)
		for( x = 0; x < Def_HW_DUMMY_GRD_HST; x++ ) {
			temp_i = GV_WARP_TABLE_X[0][y] - (((Def_HW_DUMMY_GRD_HST - x)<<Def_Wp_Space_Bit)<<4);
			// write register
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)(temp_i&0xff));	
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)((temp_i>>8)&0xff));
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)(GV_WARP_TABLE_Y[0][y]&0xff));	
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)((GV_WARP_TABLE_Y[0][y]>>8)&0xff));

			xad++;
		}

		//
		for( x = 0; x <= PS_WP_HW_GRD; x++ ) {
			if ( xad == x2k_grd_jmp ) {// x >= 2k
				temp_i = x2k_ad_jmp + (y<<x2k_ad_ysft);

			C381_Write_Bank_Reg(14,B14_DTAD,(uint8_t)(temp_i&0xff));	
			C381_Write_Bank_Reg(14,B14_DTAD,(uint8_t)((temp_i>>8)&0xff));				
			}
			// write register
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)(GV_WARP_TABLE_X[x][y]&0xff));	
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)((GV_WARP_TABLE_X[x][y]>>8)&0xff));
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)(GV_WARP_TABLE_Y[x][y]&0xff));	
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)((GV_WARP_TABLE_Y[x][y]>>8)&0xff));
		
			xad++;
		} // end for x

		// Dummy area (HEND)
		for( x = 0; x < Def_HW_DUMMY_GRD_HEND; x++ ) {
			if ( xad == x2k_grd_jmp ) {// x >= 2k
				temp_i = x2k_ad_jmp + (y<<x2k_ad_ysft);
				C381_Write_Bank_Reg(14,B14_DTAD,(uint8_t)(temp_i&0xff));	
				C381_Write_Bank_Reg(14,B14_DTAD,(uint8_t)((temp_i>>8)&0xff));	
			}

			temp_i = GV_WARP_TABLE_X[PS_WP_HW_GRD][y] + (((x+1)<<Def_Wp_Space_Bit)<<4);
			// write register
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)(temp_i&0xff));	
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)((temp_i>>8)&0xff));
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)(GV_WARP_TABLE_Y[PS_WP_HW_GRD][y]&0xff));	
			C381_Write_Bank_Reg(14,B14_DTDT,(uint8_t)((GV_WARP_TABLE_Y[PS_WP_HW_GRD][y]>>8)&0xff));

			xad++;
		}
	} // end for y
#endif 	
	GV_RTCT = GV_RTCT | 0x0007;

	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

	if( GV_WP_TABLE_COLOR == 0 ) {
		if( Def_Wp_Space == 32 ) {
			C381_Write_Bank_Reg(14,B14_DTCT,0x12);	
			C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
		}
		else {
			C381_Write_Bank_Reg(14,B14_DTCT,0x10);	
			C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
		
		}
		GV_WP_TABLE_COLOR = 1;
	}
	else {
		if( Def_Wp_Space == 32 ) {
			C381_Write_Bank_Reg(14,B14_DTCT,0x02);	
			C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
		
		}
		else {
			C381_Write_Bank_Reg(14,B14_DTCT,0x00);	
			C381_Write_Bank_Reg(14,B14_DTCT2,0x00);

		}
		GV_WP_TABLE_COLOR = 0;
	}

//	HAL_Delay(100);

	SetLut();

  GV_RTCT = GV_RTCT & 0xfff8;
  GV_RTCT = GV_RTCT | 0x0005;
  C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
  C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

#endif//HPC add
}
