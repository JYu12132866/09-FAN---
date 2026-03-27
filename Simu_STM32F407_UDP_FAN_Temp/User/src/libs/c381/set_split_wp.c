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

int16_t GV_EGB_IHW;
int16_t GV_EGB_IVW;


void SetSplitWpParam(void)
{
	int16_t iacthw_temp;
	int16_t iactvw_temp;
	float temp_f;
	int16_t temp_i;
	int16_t i, j;

	PS_SPLIT_TYPE=E_SplitTypeNoSplit;

	if (0)// PS_SPLIT_TYPE != E_SplitTypeNoSplit ) 
	{
		temp_f = ( (float)PS_WP_HW / ( (float)PS_WP_HW - (float)PM_EGB_OHW / 2 ) - 1 ) * (float)PI_ACT_HW;
		temp_i = (int16_t)temp_f;
		temp_i = temp_i / 2;
		temp_i = temp_i * 2;// Even number
		GV_EGB_IHW = ( (temp_f - (float)temp_i) > 1 ) ? temp_i + 2 : temp_i;

		temp_f = ( (float)PS_WP_VW / ( (float)PS_WP_VW - (float)PM_EGB_OVW / 2 ) - 1 ) * (float)PI_ACT_VW;
		temp_i = (int16_t)temp_f;
		temp_i = temp_i / 2;
		temp_i = temp_i * 2;// Even number
		GV_EGB_IVW = ( (temp_f - (float)temp_i) > 1 ) ? temp_i + 2 : temp_i;

		iacthw_temp = ( (PS_SPLIT_TYPE == E_SplitTypeH) || (PS_SPLIT_TYPE == E_SplitTypeQ) ) ? (PI_ACT_HW + GV_EGB_IHW) / 2 : PI_ACT_HW;
		iactvw_temp = ( (PS_SPLIT_TYPE == E_SplitTypeV) || (PS_SPLIT_TYPE == E_SplitTypeQ) ) ? (PI_ACT_VW + GV_EGB_IVW) / 2 : PI_ACT_VW;

		if ( (iacthw_temp != PM_IACT_HW) || (iactvw_temp != PM_IACT_VW) ) {
			PM_IACT_HW = iacthw_temp;
			PM_IACT_VW = iactvw_temp;

			for( j = 0; j < 9; j++ ) {
				PS_CUR_DEF_IY[j] = PM_IACT_VW * j / 8;

				if( j == 0 ) {
					for( i = 0; i < 9; i++ ) {
						PS_CUR_DEF_IX[i] = PM_IACT_HW * i / 8;
					}
				}
			}
		}
	}
	else {
		GV_EGB_IHW = PM_EGB_OHW;
		GV_EGB_IVW = PM_EGB_OVW;

		if ( (PM_IACT_HW != PI_ACT_HW) || (PM_IACT_VW != PI_ACT_VW) ) {
			PM_IACT_HW = PI_ACT_HW;
			PM_IACT_VW = PI_ACT_VW;

			for( j = 0; j < 9; j++ ) {
				PS_CUR_DEF_IY[j] = PM_IACT_VW * j / 8;

				if( j == 0 ) {
					for( i = 0; i < 9; i++ ) {
						PS_CUR_DEF_IX[i] = PM_IACT_HW * i / 8;
					}
				}
			}
		}
	}
}

void SetSplitWpReg( int16_t split_image, int16_t cur_disp )
{
	int16_t iacthst;
	int16_t iactvst;
	// int16_t temp_pm_wp_mode;
	// int16_t temp_pm_cur_internal_on;

	split_image=E_SplitImgNoSplit;

	// ACT
	if ( (split_image == E_SplitImgR) || (split_image == E_SplitImgTR) || (split_image == E_SplitImgBR) ) {
		iacthst = PI_ACT_HST + PM_IACT_HW - GV_EGB_IHW;
	}
	else {
		iacthst = PI_ACT_HST;
	}

	if ( (split_image == E_SplitImgB) || (split_image == E_SplitImgBL) || (split_image == E_SplitImgBR) ) {
		iactvst = PI_ACT_VST + PM_IACT_VW - GV_EGB_IVW;
	}
	else {
		iactvst = PI_ACT_VST;
	}

	GV_RTCT = GV_RTCT | 0x0700;
#if 0//HPC add
	C381_WriteReg( BN_RTCT,(uint8_t ) GV_RTCT);
	C381_WriteReg( BN_RTCT2,(uint8_t )( GV_RTCT>>8));

	
 	C381_Write_Bank_Reg(8,B8_IACTHSTCH1,(uint8_t)iacthst);
 	C381_Write_Bank_Reg(8,B8_IACTHSTCH2,(uint8_t)(iacthst>>8));
	
 	C381_Write_Bank_Reg(8,B8_IACTHWCH1,(uint8_t)PM_IACT_HW);
 	C381_Write_Bank_Reg(8,B8_IACTHWCH2,(uint8_t)(PM_IACT_HW>>8));	

 	C381_Write_Bank_Reg(8,B8_IACTVSTCH1,(uint8_t)iactvst);
 	C381_Write_Bank_Reg(8,B8_IACTVSTCH2,(uint8_t)(iactvst>>8));

 	C381_Write_Bank_Reg(8,B8_IACTVWCH1,(uint8_t)PM_IACT_VW);
 	C381_Write_Bank_Reg(8,B8_IACTVWCH2,(uint8_t)(PM_IACT_VW>>8));	

	PS_WP_BLACTHST = iacthst + 6;
	PS_WP_BLACTVST = iactvst;
	
 	C381_Write_Bank_Reg(3,B3_OSDACTHSTCH1,(uint8_t)PS_WP_BLACTHST);
 	C381_Write_Bank_Reg(3,B3_OSDACTHSTCH2,(uint8_t)(PS_WP_BLACTHST>>8));		

	C381_Write_Bank_Reg(3,B3_OSDACTHWCH1,(uint8_t)PM_IACT_HW);
 	C381_Write_Bank_Reg(3,B3_OSDACTHWCH2,(uint8_t)(PM_IACT_HW>>8));	

 	C381_Write_Bank_Reg(3,B3_OSDACTVSTCH1,(uint8_t)PS_WP_BLACTVST);
 	C381_Write_Bank_Reg(3,B3_OSDACTVSTCH2,(uint8_t)(PS_WP_BLACTVST>>8));		

	C381_Write_Bank_Reg(3,B3_OSDACTVWCH1,(uint8_t)PM_IACT_VW);
 	C381_Write_Bank_Reg(3,B3_OSDACTVWCH2,(uint8_t)(PM_IACT_VW>>8));	


	temp_pm_wp_mode = PM_WP_MODE;
	temp_pm_cur_internal_on = PM_CUR_INTERNAL_ON;
	PM_WP_MODE = 3;
	PM_CUR_INTERNAL_ON = 0;
	WpmodeChange( PM_WP_MODE );
	if ( MoveCur( 0, 0 ) != E_WpNoErr ) 
		{
		PS_INPUT_ERR[PM_BOARD] = E_InErrWp;
	}

	PM_WP_MODE = temp_pm_wp_mode;
	PM_CUR_INTERNAL_ON = temp_pm_cur_internal_on;
	WpmodeChange( PM_WP_MODE );
	InitGrid( cur_disp );

GV_RTCT = GV_RTCT & 0xf0ff;
GV_RTCT = GV_RTCT | 0x0100;

C381_WriteReg( BN_RTCT,(uint8_t ) GV_RTCT);
C381_WriteReg( BN_RTCT2,(uint8_t )( GV_RTCT>>8));
#endif//HPC add

	
}
