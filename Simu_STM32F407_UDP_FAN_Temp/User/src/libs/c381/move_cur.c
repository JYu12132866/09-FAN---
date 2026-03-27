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

int16_t MoveCur(float dx, float dy)
{
	int16_t err = E_WpNoErr;
	int16_t i, j;

	if ( (dx != 0) || (dy != 0) ) {
		PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].x = PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].x + dx;
		PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].y = PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].y + dy;

		if ( PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].x < 0 ) { PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].x = 0; }
		else if ( PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].x > PS_WP_HW ) { PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].x = (float)PS_WP_HW; }

		if ( PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].y < 0 ) { PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].y = 0; }
		else if ( PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].y > PS_WP_VW ) { PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].y = (float)PS_WP_VW; }

		if( (PM_GRID_OLD[PM_SEL_GRID_X][PM_SEL_GRID_Y].x==PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].x) && (PM_GRID_OLD[PM_SEL_GRID_X][PM_SEL_GRID_Y].y==PM_GRID[PM_SEL_GRID_X][PM_SEL_GRID_Y].y) ) {
			err = E_WpErrOut;
		}
	}

	if ( err == E_WpNoErr ) {
		
		GV_WP_MODE_WP = PM_WP_MODE;
		GV_CUR_INTERNAL_ON_WP = PM_CUR_INTERNAL_ON;
	err = Warp();
#if Def_WpLimitMode
		if( err != E_WpNoErr ) {
			
			for ( j = 0; j < 9; j++ ) {
				for ( i = 0; i < 9; i++ ) {
					PM_GRID[i][j] = PM_GRID_OLD[i][j];
				}
			}
		}
		else {
			for ( j = 0; j < 9; j++ ) {
				for ( i = 0; i < 9; i++ ) {
					PM_GRID_OLD[i][j] = PM_GRID[i][j];
				}
			}
		}
#endif
	}
	if ( (err == E_WpNoErr) && (PS_INPUT_ERR[PM_BOARD] == E_InErrWp) ) {
		PS_INPUT_ERR[PM_BOARD] = E_InNoErr;
		TestOFILL();
	}

	return err;
}
