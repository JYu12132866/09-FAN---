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

void SetEgbBiasCursorEn( int16_t en )
{
#if 0//HPC add
	if ( en == 0 ) {
	    C381_Write_Bank_Reg(16,B16_CURCT,0x16);
	    C381_Write_Bank_Reg(16,B16_CURCT2,0x00);		
	} else {
	    C381_Write_Bank_Reg(16,B16_CURCT,0x17);
	    C381_Write_Bank_Reg(16,B16_CURCT2,0x00);	
	}
#endif//HPC add
}

void SetEgbBiasCurPos( int16_t selx, int16_t sely, int16_t dx, int16_t dy )
{
	PM_EBIAS_CUR[selx][sely].x = PM_EBIAS_CUR[selx][sely].x + dx;
	PM_EBIAS_CUR[selx][sely].x = ( PM_EBIAS_CUR[selx][sely].x < 0 ) ? 0 : (( PM_EBIAS_CUR[selx][sely].x >= PS_WP_HW ) ? PS_WP_HW - 1 : PM_EBIAS_CUR[selx][sely].x);

	PM_EBIAS_CUR[selx][sely].y = PM_EBIAS_CUR[selx][sely].y + dy;
	PM_EBIAS_CUR[selx][sely].y = ( PM_EBIAS_CUR[selx][sely].y < 0 ) ? 0 :( ( PM_EBIAS_CUR[selx][sely].y >= PS_WP_VW ) ? PS_WP_VW - 1 : PM_EBIAS_CUR[selx][sely].y);
#if 0//HPC add
	if ( selx == 0 ) {
		if ( sely == 0 ) {
	    C381_Write_Bank_Reg(16,B16_CURTLX,(uint8_t)(PM_EBIAS_CUR[selx][sely].x));
	    C381_Write_Bank_Reg(16,B16_CURTLX2,(uint8_t)((PM_EBIAS_CUR[selx][sely].x)>>8));	
	    C381_Write_Bank_Reg(16,B16_CURTLY,(uint8_t)(PM_EBIAS_CUR[selx][sely].y));
	    C381_Write_Bank_Reg(16,B16_CURTLY2,(uint8_t)((PM_EBIAS_CUR[selx][sely].y)>>8));			
		}
		if ( sely == 1 ) {
	    C381_Write_Bank_Reg(16,B16_CURBLX,(uint8_t)(PM_EBIAS_CUR[selx][sely].x));
	    C381_Write_Bank_Reg(16,B16_CURBLX2,(uint8_t)((PM_EBIAS_CUR[selx][sely].x)>>8));	
	    C381_Write_Bank_Reg(16,B16_CURBLY, (uint8_t)(PM_EBIAS_CUR[selx][sely].y - (Def_4CURSOR_Size - 1)));
	    C381_Write_Bank_Reg(16,B16_CURBLY2,(uint8_t)(( PM_EBIAS_CUR[selx][sely].y - (Def_4CURSOR_Size - 1))>>8));

		}
	}

	if ( selx == 1 ) {
		if ( sely == 0 ) {
	    C381_Write_Bank_Reg(16,B16_CURTRX, (uint8_t)(PM_EBIAS_CUR[selx][sely].x - (Def_4CURSOR_Size - 1)));
	    C381_Write_Bank_Reg(16,B16_CURTRX2,(uint8_t)(( PM_EBIAS_CUR[selx][sely].x - (Def_4CURSOR_Size - 1))>>8));	
	    C381_Write_Bank_Reg(16,B16_CURTRY, (uint8_t)(PM_EBIAS_CUR[selx][sely].y));
	    C381_Write_Bank_Reg(16,B16_CURTRY2,(uint8_t)((PM_EBIAS_CUR[selx][sely].y)>>8));

		}
		if ( sely == 1 ) {
	    C381_Write_Bank_Reg(16,B16_CURBRX,(uint8_t) (PM_EBIAS_CUR[selx][sely].x - (Def_4CURSOR_Size - 1)));
	    C381_Write_Bank_Reg(16,B16_CURBRX2,(uint8_t)(( PM_EBIAS_CUR[selx][sely].x - (Def_4CURSOR_Size - 1))>>8));	
	    C381_Write_Bank_Reg(16,B16_CURBRY,(uint8_t)(PM_EBIAS_CUR[selx][sely].y - (Def_4CURSOR_Size - 1)));
	    C381_Write_Bank_Reg(16,B16_CURBRY2,(uint8_t)((PM_EBIAS_CUR[selx][sely].y - (Def_4CURSOR_Size - 1))>>8));
		}
	}
#endif//HPC add
}

void EgbBiasEn(void)
{
#if 0//HPC add
	if ( PM_EBIAS_EN == 0 ) {
	    C381_Write_Bank_Reg(16,B16_EGBIASCT,0x50);
	    C381_Write_Bank_Reg(16,B16_EGBIASCT2,0x00);
	}
	else {
	    C381_Write_Bank_Reg(16,B16_EGBIASCT,0x51);
	    C381_Write_Bank_Reg(16,B16_EGBIASCT2,0x00);	
	}
#endif//HPC add
}

void SetEgbBiasPlt( int16_t plt, int16_t r, int16_t g, int16_t b )
{
	r = ( r < 0 ) ? 0 : (( r > 0xFF ) ? 0xFF : r);
	g = ( g < 0 ) ? 0 : (( g > 0xFF ) ? 0xFF : g);
	b = ( b < 0 ) ? 0 : (( b > 0xFF ) ? 0xFF : b);

	PM_EBIAS[plt][0] = r;
	PM_EBIAS[plt][1] = g;
	PM_EBIAS[plt][2] = b;
}
