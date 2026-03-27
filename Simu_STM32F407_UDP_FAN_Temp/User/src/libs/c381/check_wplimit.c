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



int16_t CheckWpLimitA( int16_t *x, int16_t *y, float *diffy_y_sum, float *divn )
{
	float diffx_x, diffy_x, diffx_y, diffy_y, idiffy_y;
	float tan_h = 0;
	float tan_v = 0;

	if (   ( (GV_WARP_TABLE_X[*x][*y] > (PM_IACT_HW<<4)) && (GV_WARP_TABLE_X[*x+1][*y] > (PM_IACT_HW<<4)) )
		|| ( (GV_WARP_TABLE_X[*x][*y] > (PM_IACT_HW<<4)) && (GV_WARP_TABLE_X[*x][*y+1] > (PM_IACT_HW<<4)) )
		|| ( (GV_WARP_TABLE_X[*x][*y] < 0) && (GV_WARP_TABLE_X[*x+1][*y] < 0) )
		|| ( (GV_WARP_TABLE_X[*x][*y] < 0) && (GV_WARP_TABLE_X[*x][*y+1] < 0) )
		|| ( (GV_WARP_TABLE_Y[*x][*y] > (PM_IACT_VW<<4)) && (GV_WARP_TABLE_Y[*x+1][*y] > (PM_IACT_VW<<4)) )
		|| ( (GV_WARP_TABLE_Y[*x][*y] > (PM_IACT_VW<<4)) && (GV_WARP_TABLE_Y[*x][*y+1] > (PM_IACT_VW<<4)) )
		|| ( (GV_WARP_TABLE_Y[*x][*y] < 0) && (GV_WARP_TABLE_Y[*x+1][*y] < 0) )
		|| ( (GV_WARP_TABLE_Y[*x][*y] < 0) && (GV_WARP_TABLE_Y[*x][*y+1] < 0) )
	) {
		return E_WpNoErr;// outside of ACT
	}

	if( (*x < PS_WP_HW_GRD) || (*y < PS_WP_VW_GRD) ) {
		if( *x < PS_WP_HW_GRD ) {
			diffx_x = (float)(GV_WARP_TABLE_X[*x+1][*y] - GV_WARP_TABLE_X[*x][*y]);
			diffy_x = (float)(GV_WARP_TABLE_Y[*x+1][*y] - GV_WARP_TABLE_Y[*x][*y]);

			if( !( ((GV_WARP_TABLE_X[*x+1][*y]<0) && (GV_WARP_TABLE_X[*x][*y]<0))
			|| ((GV_WARP_TABLE_Y[*x+1][*y]<0) && (GV_WARP_TABLE_Y[*x][*y]<0)) ) ) {
//				diffx_x = (float)(GV_WARP_TABLE_X[*x+1][*y] - GV_WARP_TABLE_X[*x][*y]);
//				diffy_x = (float)(GV_WARP_TABLE_Y[*x+1][*y] - GV_WARP_TABLE_Y[*x][*y]);
			
				// Horizontal slope
				tan_h = diffy_x/diffx_x;
				if( (tan_h < ((-Def_WPLIMANG_H)*1.0f)) || (tan_h > Def_WPLIMANG_H) ) {
					
					return E_WpErrAngH;
				}
				// Horizontal shrink rate
				if( diffx_x > Def_WPLIMHSH ) {
					
					return E_WpErrHSh;
				}
			}
		} // end if(x<PS_WP_HW_GRD)
		else {
			diffx_x = 1;
			diffy_x = 0;// -> diffy_x / diffx_x = 0
		}

		if( *y < PS_WP_VW_GRD ) {
			if( !( ((GV_WARP_TABLE_X[*x][*y+1]<0) && (GV_WARP_TABLE_X[*x][*y]<0))
			|| ((GV_WARP_TABLE_Y[*x][*y+1]<0)&&(GV_WARP_TABLE_Y[*x][*y]<0)) ) ) {
				diffx_y = (float)( GV_WARP_TABLE_X[*x][*y+1] - GV_WARP_TABLE_X[*x][*y] );
				diffy_y = (float)( GV_WARP_TABLE_Y[*x][*y+1] - GV_WARP_TABLE_Y[*x][*y] );

//				idiffy_y = diffy_y + diffx_y * (-tan_h);
				idiffy_y = diffy_y + diffx_y * (( -diffy_x/diffx_x )*1.0f);// theta = arctan(diffy_x/diffx_x), tan(theta) = tan( arctan(diffy_x/diffx_x) ) = diffy_x/diffx_x

				// Vertical local shrink rate
				if( idiffy_y > Def_WPLIMVSH_L ) {
					
					return E_WpErrVShL;
				}
				// Vertical slope
				tan_v = diffx_y / diffy_y;
			if( (tan_v < ((-Def_WPLIMANG_V)*1.0f)) || (tan_v > Def_WPLIMANG_V) ) 
			  {
				//	return E_WpErrAngV;
				}

		  	
				*diffy_y_sum = (*diffy_y_sum) + idiffy_y;
				*divn = (*divn) + 1;
			}
		} // end if(y<PS_WP_VW_GRD)
	} // end if((x<PS_WP_HW_GRD)||(y<PS_WP_VW_GRD))

	return E_WpNoErr;// no error
}

int16_t CheckWpLimitB( float *diffy_y_sum, float *divn )
{
	if( (*diffy_y_sum) > (Def_WPLIMVSH_A * (*divn)) ){
		return E_WpErrVShA;
	}

	return E_WpNoErr;// no error
}
