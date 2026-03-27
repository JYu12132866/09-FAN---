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

int16_t PS_INPUT_ERR[4];

void SetOFILL( int16_t en, int32_t color )
{
#if 0//HPC add
	C381_Write_Bank_Reg(2,B2_OFILLCH1,(uint8_t)color);
	C381_Write_Bank_Reg(2,B2_OFILLCH2,(uint8_t)(color>>8));
	C381_Write_Bank_Reg(2,B2_OFILLCH3,(uint8_t)(color>>16));

	if ( en != 0 ) {
		
		C381_Write_Bank_Reg(2,B2_OIMGCTCH1,0x01);
	}
	else {
		if ( PS_INPUT_ERR[PM_BOARD] == E_InNoErr ) {
			
		C381_Write_Bank_Reg(2,B2_OIMGCTCH1,0x00);
		}
	}
#endif//HPC add
}
