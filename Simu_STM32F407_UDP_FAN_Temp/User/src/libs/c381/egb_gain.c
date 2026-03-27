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

void EgbGain(void)
{
#if 0//HPC add
	float lcoef_f;
	float rcoef_f;
	float tcoef_f;
	float bcoef_f;
	int16_t lcoef_i;
	int16_t rcoef_i;
	int16_t tcoef_i;
	int16_t bcoef_i;

	if ( PM_EGB_EN == 0 ) {
		 	C381_Write_Bank_Reg(16,B16_LEGBHST,0x00);
		 	C381_Write_Bank_Reg(16,B16_LEGBHST2,0x00);
		if ( PM_EGB_LEN == 0 ) {
			C381_Write_Bank_Reg(16,B16_LEGBHW,0x00);
		 	C381_Write_Bank_Reg(16,B16_LEGBHW2,0x00);
		}
		else {
			C381_Write_Bank_Reg(16,B16_LEGBHW,(uint8_t)GV_EGB_IHW);
		 	C381_Write_Bank_Reg(16,B16_LEGBHW2,(uint8_t)(GV_EGB_IHW>>8));

		}
		C381_Write_Bank_Reg(16,B16_REGBHST,(uint8_t)(PM_IACT_HW - GV_EGB_IHW));
		C381_Write_Bank_Reg(16,B16_REGBHST2,(uint8_t)((PM_IACT_HW - GV_EGB_IHW)>>8));

		if ( PM_EGB_REN == 0 ) {
			C381_Write_Bank_Reg(16,B16_REGBHW,0x00);
		 	C381_Write_Bank_Reg(16,B16_REGBHW2,0x00);			
		}
		else {
			C381_Write_Bank_Reg(16,B16_REGBHW,(uint8_t)GV_EGB_IHW);
		 	C381_Write_Bank_Reg(16,B16_REGBHW2,(uint8_t)(GV_EGB_IHW>>8));
		}
		C381_Write_Bank_Reg(16,B16_TEGBVST,0x00);
		C381_Write_Bank_Reg(16,B16_TEGBVST2,0x00);

		if ( PM_EGB_TEN == 0 ) {
		C381_Write_Bank_Reg(16,B16_TEGBVW,0x00);
		C381_Write_Bank_Reg(16,B16_TEGBVW2,0x00);
		}
		else {
		C381_Write_Bank_Reg(16,B16_TEGBVW,(uint8_t)GV_EGB_IVW);
		C381_Write_Bank_Reg(16,B16_TEGBVW2,(uint8_t)(GV_EGB_IVW>>8));			
		}
		C381_Write_Bank_Reg(16,B16_BEGBVST,(uint8_t)(PM_IACT_VW - GV_EGB_IVW));
		C381_Write_Bank_Reg(16,B16_BEGBVST2,(uint8_t)((PM_IACT_VW - GV_EGB_IVW)>>8));

		if ( PM_EGB_BEN == 0 ) {
		C381_Write_Bank_Reg(16,B16_BEGBVW,0x00);
		C381_Write_Bank_Reg(16,B16_BEGBVW2,0x00);
		}
		else {
		C381_Write_Bank_Reg(16,B16_BEGBVW,(uint8_t)GV_EGB_IVW);
		C381_Write_Bank_Reg(16,B16_BEGBVW2,(uint8_t)(GV_EGB_IVW>>8));	
		}

		GV_EGBCT = 0x0000;
		C381_Write_Bank_Reg(16,B16_EGBCT,(uint8_t)GV_EGBCT);
		C381_Write_Bank_Reg(16,B16_EGBCT2,(uint8_t)(GV_EGBCT>>8));	
	}
	else {
		lcoef_f = ( (PM_EGB_LEN == 0) || (GV_EGB_IHW < 4) ) ? 0 : 1 / ((float)GV_EGB_IHW - 1) * 65536;
		rcoef_f = ( (PM_EGB_REN == 0) || (GV_EGB_IHW < 4) ) ? 0 : ( 1 - 1 / ((float)GV_EGB_IHW - 1) ) * 65536;
		tcoef_f = ( (PM_EGB_TEN == 0) || (GV_EGB_IVW < 4) ) ? 0 : 1 / ((float)GV_EGB_IVW - 1) * 65536;
		bcoef_f = ( (PM_EGB_BEN == 0) || (GV_EGB_IHW < 4) ) ? 0 : ( 1 - 1 / ((float)GV_EGB_IVW - 1) ) * 65536;

		lcoef_i = (int16_t)lcoef_f;
		rcoef_i = (int16_t)rcoef_f;
		tcoef_i = (int16_t)tcoef_f;
		bcoef_i = (int16_t)bcoef_f;

		if ( lcoef_f > (float)lcoef_i ) { lcoef_i += 1; }
		if ( tcoef_f > (float)tcoef_i ) { tcoef_i += 1; }

		if ( lcoef_i > 0x7fff ) { lcoef_i = 0x7fff; }
		if ( rcoef_i < 0x8000 ) { rcoef_i = 0x8000; }
		if ( tcoef_i > 0x7fff ) { tcoef_i = 0x7fff; }
		if ( bcoef_i < 0x8000 ) { bcoef_i = 0x8000; }

		GV_EGBCT = ( PM_EGB_GAMMA_EN == 0 ) ? 0x0000 : 0x0010;

		C381_Write_Bank_Reg(16,B16_LEGBHST,0x00);
		C381_Write_Bank_Reg(16,B16_LEGBHST2,0x00);	

		if ( (PM_EGB_LEN == 0) || (lcoef_i == 0) ) {
		    C381_Write_Bank_Reg(16,B16_HEGBINIT,0x80);	
			
		    C381_Write_Bank_Reg(16,B16_LEGBHW,0x00);	
		    C381_Write_Bank_Reg(16,B16_LEGBHW2,0x00);	
		    C381_Write_Bank_Reg(16,B16_LEGBCOEF,0x00);	
		    C381_Write_Bank_Reg(16,B16_LEGBCOEF2,0x00);

		}
		else {
			GV_EGBCT = GV_EGBCT | 0x0001;
		    C381_Write_Bank_Reg(16,B16_HEGBINIT,0x00);	
		    C381_Write_Bank_Reg(16,B16_LEGBHW,(uint8_t)GV_EGB_IHW);	
		    C381_Write_Bank_Reg(16,B16_LEGBHW2,(uint8_t)(GV_EGB_IHW>>8));	
		    C381_Write_Bank_Reg(16,B16_LEGBCOEF,(uint8_t)lcoef_i);	
		    C381_Write_Bank_Reg(16,B16_LEGBCOEF2,(uint8_t)(lcoef_i>>8));
		}
		C381_Write_Bank_Reg(16,B16_REGBHST,(uint8_t)(PM_IACT_HW - GV_EGB_IHW));
		C381_Write_Bank_Reg(16,B16_REGBHST2,(uint8_t)((PM_IACT_HW - GV_EGB_IHW)>>8));

		if ( PM_EGB_REN == 0 ) {
			C381_Write_Bank_Reg(16,B16_REGBHW,0x00);	
		    C381_Write_Bank_Reg(16,B16_REGBHW2,0x00);	
		    C381_Write_Bank_Reg(16,B16_REGBCOEF,0x00);	
		    C381_Write_Bank_Reg(16,B16_REGBCOEF2,0x00);

		}
		else {
			GV_EGBCT = GV_EGBCT | 0x0002;
			C381_Write_Bank_Reg(16,B16_REGBHW,(uint8_t)GV_EGB_IHW);	
		    C381_Write_Bank_Reg(16,B16_REGBHW2,(uint8_t)(GV_EGB_IHW>>8));	
		    C381_Write_Bank_Reg(16,B16_REGBCOEF,(uint8_t)rcoef_i);	
		    C381_Write_Bank_Reg(16,B16_REGBCOEF2,(uint8_t)(rcoef_i>>8));

		}
		C381_Write_Bank_Reg(16,B16_TEGBVST,0x00);
		C381_Write_Bank_Reg(16,B16_TEGBVST2,0x00);

		if ( (PM_EGB_TEN == 0) || (tcoef_i == 0) ) {
			C381_Write_Bank_Reg(16,B16_VEGBINIT,0x80);
		    C381_Write_Bank_Reg(16,B16_TEGBVW,0x00); 
		    C381_Write_Bank_Reg(16,B16_TEGBVW2,0x00); 
		    C381_Write_Bank_Reg(16,B16_TEGBCOEF,0x00); 
		    C381_Write_Bank_Reg(16,B16_TEGBCOEF2,0x00); 

		}
		else {
			GV_EGBCT = GV_EGBCT | 0x0004;

			C381_Write_Bank_Reg(16,B16_VEGBINIT,0x00);
		    C381_Write_Bank_Reg(16,B16_TEGBVW,(uint8_t)GV_EGB_IVW); 
		    C381_Write_Bank_Reg(16,B16_TEGBVW2,(uint8_t)(GV_EGB_IVW>>8)); 
		    C381_Write_Bank_Reg(16,B16_TEGBCOEF,(uint8_t)tcoef_i); 
		    C381_Write_Bank_Reg(16,B16_TEGBCOEF2,(uint8_t)(tcoef_i>>8)); 

		}
		C381_Write_Bank_Reg(16,B16_BEGBVST,(uint8_t)(PM_IACT_VW - GV_EGB_IVW));
		C381_Write_Bank_Reg(16,B16_BEGBVST2,(uint8_t)((PM_IACT_VW - GV_EGB_IVW)>>8));

		if ( PM_EGB_BEN == 0 ) {
		    C381_Write_Bank_Reg(16,B16_BEGBVW,0x00); 
		    C381_Write_Bank_Reg(16,B16_BEGBVW2,0x00); 
		    C381_Write_Bank_Reg(16,B16_BEGBCOEF,0x00); 
		    C381_Write_Bank_Reg(16,B16_BEGBCOEF2,0x00); 

		}
		else {
			GV_EGBCT = GV_EGBCT | 0x0008;
		    C381_Write_Bank_Reg(16,B16_BEGBVW,(uint8_t)GV_EGB_IVW); 
		    C381_Write_Bank_Reg(16,B16_BEGBVW2,(uint8_t)(GV_EGB_IVW>>8)); 
		    C381_Write_Bank_Reg(16,B16_BEGBCOEF,(uint8_t)bcoef_i); 
		    C381_Write_Bank_Reg(16,B16_BEGBCOEF2,(uint8_t)(bcoef_i>>8)); 

		}
		C381_Write_Bank_Reg(16,B16_EGBCT,(uint8_t)GV_EGBCT);
		C381_Write_Bank_Reg(16,B16_EGBCT2,(uint8_t)(GV_EGBCT>>8));

	}
#endif//HPC add
}
