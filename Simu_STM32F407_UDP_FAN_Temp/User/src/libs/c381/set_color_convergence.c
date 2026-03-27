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

void SetColorConvergence( int16_t color )
{
#if 0//HPC add
	int16_t val_x;
	int16_t val_y;

	if ( color > 2 ) {
		val_x = ( PM_WP_CCVG[0].x < 0 ) ? 256 + PM_WP_CCVG[0].x : PM_WP_CCVG[0].x;
		val_y = ( PM_WP_CCVG[0].y < 0 ) ? 256 + PM_WP_CCVG[0].y : PM_WP_CCVG[0].y;

		C381_Write_Bank_Reg(14, B14_DTSFTXR, (val_x & 0xff));
		C381_Write_Bank_Reg(14, B14_DTSFTYR, (val_y & 0xff));

		val_x = ( PM_WP_CCVG[1].x < 0 ) ? 256 + PM_WP_CCVG[1].x : PM_WP_CCVG[1].x;
		val_y = ( PM_WP_CCVG[1].y < 0 ) ? 256 + PM_WP_CCVG[1].y : PM_WP_CCVG[1].y;
		C381_Write_Bank_Reg(14, B14_DTSFTXG, (val_x & 0xff));
		C381_Write_Bank_Reg(14, B14_DTSFTYG, (val_y & 0xff));

		val_x = ( PM_WP_CCVG[2].x < 0 ) ? 256 + PM_WP_CCVG[2].x : PM_WP_CCVG[2].x;
		val_y = ( PM_WP_CCVG[2].y < 0 ) ? 256 + PM_WP_CCVG[2].y : PM_WP_CCVG[2].y;
		C381_Write_Bank_Reg(14, B14_DTSFTXB, (val_x & 0xff));
		C381_Write_Bank_Reg(14, B14_DTSFTYB, (val_y & 0xff));

	}
	else if ( color >= 0 ) {
		val_x = ( PM_WP_CCVG[color].x < 0 ) ? 256 + PM_WP_CCVG[color].x : PM_WP_CCVG[color].x;
		val_y = ( PM_WP_CCVG[color].y < 0 ) ? 256 + PM_WP_CCVG[color].y : PM_WP_CCVG[color].y;

		if ( color == 0 ) {
		C381_Write_Bank_Reg(14, B14_DTSFTXR, (val_x & 0xff));
		C381_Write_Bank_Reg(14, B14_DTSFTYR, (val_y & 0xff));
		}
		else if ( color == 1 ) {
		C381_Write_Bank_Reg(14, B14_DTSFTXG, (val_x & 0xff));
		C381_Write_Bank_Reg(14, B14_DTSFTYG, (val_y & 0xff));

		}
		else {
		C381_Write_Bank_Reg(14, B14_DTSFTXB, (val_x & 0xff));
		C381_Write_Bank_Reg(14, B14_DTSFTYB, (val_y & 0xff));

		}
	}
#endif//HPC add
}

int16_t CheckColorConvergenceLim( int16_t color )
{
	int16_t pm_min;
	int16_t pm_max;

	char cmp_rg;
	char cmp_rb;
	char cmp_gb;

	int16_t min_rgb;
	int16_t max_rgb;

	if ( (color < 0) || (color > 3) ) {
		return -1;
	}

	// x
	PM_WP_CCVG[0].x = ( PM_WP_CCVG[0].x > 31 ) ? 31 : (( PM_WP_CCVG[0].x < -32 ) ? -32 : PM_WP_CCVG[0].x);
	PM_WP_CCVG[1].x = ( PM_WP_CCVG[1].x > 31 ) ? 31 : (( PM_WP_CCVG[1].x < -32 ) ? -32 : PM_WP_CCVG[1].x);
	PM_WP_CCVG[2].x = ( PM_WP_CCVG[2].x > 31 ) ? 31 : (( PM_WP_CCVG[2].x < -32 ) ? -32 : PM_WP_CCVG[2].x);

	do {
		cmp_rg = ( PM_WP_CCVG[0].x > PM_WP_CCVG[1].x ) ? 1 : 0;
		cmp_rb = ( PM_WP_CCVG[0].x > PM_WP_CCVG[2].x ) ? 1 : 0;
		cmp_gb = ( PM_WP_CCVG[1].x > PM_WP_CCVG[2].x ) ? 1 : 0;

		if ( (cmp_rg == 0) && (cmp_rb == 0) ) {// (r < g) && (r < b)
			pm_min = PM_WP_CCVG[0].x;
			min_rgb = 0;
		}
		else if ( (cmp_rg != 0) && (cmp_gb == 0) ) {// (r > g) && (g < b)
			pm_min = PM_WP_CCVG[1].x;
			min_rgb = 1;
		}
		else {
			pm_min = PM_WP_CCVG[2].x;
			min_rgb = 2;
		}

		if ( (cmp_rg != 0) && (cmp_rb != 0) ) {// (r > g) && (r > b)
			pm_max = PM_WP_CCVG[0].x;
			max_rgb = 0;
		}
		else if ( (cmp_rg == 0) && (cmp_gb != 0) ) {// (r < g) && (g > b)
			pm_max = PM_WP_CCVG[1].x;
			max_rgb = 1;
		}
		else {
			pm_max = PM_WP_CCVG[2].x;
			max_rgb = 2;
		}

		if ( (pm_max - pm_min) <= 24 ) {
			break;
		}
		else {
			if ( color == 3 ) {
				if ( pm_min < 0 ) {
					if ( pm_max < (-pm_min) ) { PM_WP_CCVG[min_rgb].x = PM_WP_CCVG[min_rgb].x + 1; }
					else { PM_WP_CCVG[max_rgb].x = PM_WP_CCVG[max_rgb].x - 1; }
				}
				else {
					if ( pm_max < pm_min ) { PM_WP_CCVG[min_rgb].x = PM_WP_CCVG[min_rgb].x + 1; }
					else { PM_WP_CCVG[max_rgb].x = PM_WP_CCVG[max_rgb].x - 1; }
				}
			}
			else {
				if ( min_rgb == color ) {
					PM_WP_CCVG[min_rgb].x = PM_WP_CCVG[min_rgb].x + 1;
				}
				else if ( max_rgb == color ) {
					PM_WP_CCVG[max_rgb].x = PM_WP_CCVG[max_rgb].x - 1;
				}
				else {
					if ( pm_min < 0 ) {
						if ( pm_max < (-pm_min) ) { PM_WP_CCVG[min_rgb].x = PM_WP_CCVG[min_rgb].x + 1; }
						else { PM_WP_CCVG[max_rgb].x = PM_WP_CCVG[max_rgb].x - 1; }
					}
					else {
						if ( pm_max < pm_min ) { PM_WP_CCVG[min_rgb].x = PM_WP_CCVG[min_rgb].x + 1; }
						else { PM_WP_CCVG[max_rgb].x = PM_WP_CCVG[max_rgb].x - 1; }
					}
				}
			}
		}
	} while ( 1 );

	// y
	PM_WP_CCVG[0].y = ( PM_WP_CCVG[0].y > 31 ) ? 31 :( ( PM_WP_CCVG[0].y < -32 ) ? -32 : PM_WP_CCVG[0].y);
	PM_WP_CCVG[1].y = ( PM_WP_CCVG[1].y > 31 ) ? 31 : (( PM_WP_CCVG[1].y < -32 ) ? -32 : PM_WP_CCVG[1].y);
	PM_WP_CCVG[2].y = ( PM_WP_CCVG[2].y > 31 ) ? 31 :( ( PM_WP_CCVG[2].y < -32 ) ? -32 : PM_WP_CCVG[2].y);

	do {
		cmp_rg = ( PM_WP_CCVG[0].y > PM_WP_CCVG[1].y ) ? 1 : 0;
		cmp_rb = ( PM_WP_CCVG[0].y > PM_WP_CCVG[2].y ) ? 1 : 0;
		cmp_gb = ( PM_WP_CCVG[1].y > PM_WP_CCVG[2].y ) ? 1 : 0;

		if ( (cmp_rg == 0) && (cmp_rb == 0) ) {// (r < g) && (r < b)
			pm_min = PM_WP_CCVG[0].y;
			min_rgb = 0;
		}
		else if ( (cmp_rg != 0) && (cmp_gb == 0) ) {// (r > g) && (g < b)
			pm_min = PM_WP_CCVG[1].y;
			min_rgb = 1;
		}
		else {
			pm_min = PM_WP_CCVG[2].y;
			min_rgb = 2;
		}

		if ( (cmp_rg != 0) && (cmp_rb != 0) ) {// (r > g) && (r > b)
			pm_max = PM_WP_CCVG[0].y;
			max_rgb = 0;
		}
		else if ( (cmp_rg == 0) && (cmp_gb != 0) ) {// (r < g) && (g > b)
			pm_max = PM_WP_CCVG[1].y;
			max_rgb = 1;
		}
		else {
			pm_max = PM_WP_CCVG[2].y;
			max_rgb = 2;
		}

		if ( (pm_max - pm_min) <= 24 ) {
			break;
		}
		else {
			if ( color == 3 ) {
				if ( pm_min < 0 ) {
					if ( pm_max < (-pm_min) ) { PM_WP_CCVG[min_rgb].y = PM_WP_CCVG[min_rgb].y + 1; }
					else { PM_WP_CCVG[max_rgb].y = PM_WP_CCVG[max_rgb].y - 1; }
				}
				else {
					if ( pm_max < pm_min ) { PM_WP_CCVG[min_rgb].y = PM_WP_CCVG[min_rgb].y + 1; }
					else { PM_WP_CCVG[max_rgb].y = PM_WP_CCVG[max_rgb].y - 1; }
				}
			}
			else {
				if ( min_rgb == color ) {
					PM_WP_CCVG[min_rgb].y = PM_WP_CCVG[min_rgb].y + 1;
				}
				else if ( max_rgb == color ) {
					PM_WP_CCVG[max_rgb].y = PM_WP_CCVG[max_rgb].y - 1;
				}
				else {
					if ( pm_min < 0 ) {
						if ( pm_max < (-pm_min) ) { PM_WP_CCVG[min_rgb].y = PM_WP_CCVG[min_rgb].y + 1; }
						else { PM_WP_CCVG[max_rgb].y = PM_WP_CCVG[max_rgb].y - 1; }
					}
					else {
						if ( pm_max < pm_min ) { PM_WP_CCVG[min_rgb].y = PM_WP_CCVG[min_rgb].y + 1; }
						else { PM_WP_CCVG[max_rgb].y = PM_WP_CCVG[max_rgb].y - 1; }
					}
				}
			}
		}
	} while ( 1 );

	return 0;
}
