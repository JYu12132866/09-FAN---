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

void TestCrossHatch(void)
{
#if 0//HPC add
	if ( PM_PATGEN == 0 ) {
		C381_Write_Bank_Reg(2, B2_PGCTRLCH1, 0x80);
	}
	else {
		if ( GV_EGB_IHW < 64 ) {
			C381_Write_Bank_Reg(2, B2_HLIMITMAXCH1, 0x40);
			C381_Write_Bank_Reg(2, B2_HLIMITMAXCH2, 0x00);
		}
		else if ( GV_EGB_IHW > 255 ) {
			C381_Write_Bank_Reg(2, B2_HLIMITMAXCH1, 0xff);
			C381_Write_Bank_Reg(2, B2_HLIMITMAXCH2, 0x00);
		}
		else {
			C381_Write_Bank_Reg(2, B2_HLIMITMAXCH1, (uint8_t)(GV_EGB_IHW - 1));
			C381_Write_Bank_Reg(2, B2_HLIMITMAXCH2, (uint8_t)((GV_EGB_IHW - 1)>>8));
		}

		if ( GV_EGB_IVW < 64 ) {
			C381_Write_Bank_Reg(2, B2_VLIMITMAXCH1, 0x40);
			C381_Write_Bank_Reg(2, B2_VLIMITMAXCH2, 0x00);
		}
		else if ( GV_EGB_IVW > 255 ) {
			C381_Write_Bank_Reg(2, B2_VLIMITMAXCH1, 0xff);
			C381_Write_Bank_Reg(2, B2_VLIMITMAXCH2, 0x00);
		}
		else {
			C381_Write_Bank_Reg(2, B2_VLIMITMAXCH1,(uint8_t) (GV_EGB_IVW - 1));
			C381_Write_Bank_Reg(2, B2_VLIMITMAXCH2, (uint8_t)((GV_EGB_IVW - 1)>>8));
		}

		if ( PM_PATGEN == 2 ) {
			C381_Write_Bank_Reg(2,B2_PGPTCTRLCH1 , 0x38);
		}
		else {
			C381_Write_Bank_Reg(2, B2_PGPTCTRLCH1, 0x30);
		}
		C381_Write_Bank_Reg(2, B2_PGCTRLCH1, 0x88);
	}
#endif//HPC add
}

void TestEGBMarker(void)
{
#if 0//HPC add
	int16_t lct;
	int16_t rct;
	int16_t tct;
	int16_t bct;

	if ( PM_EGBMK_EN == 0 ) {
		C381_Write_Bank_Reg(16, B16_LEGBMKCT, (Def_EGBMK_L & 0xfe));
		C381_Write_Bank_Reg(16, B16_REGBMKCT, (Def_EGBMK_R & 0xfe));
		C381_Write_Bank_Reg(16, B16_TEGBMKCT, (Def_EGBMK_T & 0xfe));
		C381_Write_Bank_Reg(16, B16_BEGBMKCT, (Def_EGBMK_B & 0xfe));

	}
	else {
		lct = ( PM_EGB_LEN != 0 ) ? (Def_EGBMK_L | 0x01) : (Def_EGBMK_L & 0xfe);
		rct = ( PM_EGB_REN != 0 ) ? (Def_EGBMK_R | 0x01) : (Def_EGBMK_R & 0xfe);
		tct = ( PM_EGB_TEN != 0 ) ? (Def_EGBMK_T | 0x01) : (Def_EGBMK_T & 0xfe);
		bct = ( PM_EGB_BEN != 0 ) ? (Def_EGBMK_B | 0x01) : (Def_EGBMK_B & 0xfe);
		C381_Write_Bank_Reg(16, B16_LEGBMKCT, lct);
		C381_Write_Bank_Reg(16, B16_REGBMKCT, rct);
		C381_Write_Bank_Reg(16, B16_TEGBMKCT, tct);
		C381_Write_Bank_Reg(16, B16_BEGBMKCT, bct);

	}
#endif//HPC add
}

void TestOFILL(void)
{
	if ( PM_OFILL_EN == 2 ) {
		SetOFILL( PM_OFILL_EN, 0x3f3f3f );
	}
	else if ( PM_OFILL_EN == 3 ) {
		SetOFILL( PM_OFILL_EN, 0x7f7f7f );
	}
	else if ( PM_OFILL_EN == 4 ) {
		SetOFILL( PM_OFILL_EN, 0xffffff );
	}
	else {
		SetOFILL( PM_OFILL_EN, 0x000000 );
	}
}
