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



 float GV_PJC_A[3];
 float GV_PJC_C[3];
 float GV_PJC_B[3];



//coord_f_t PM_GRID[9][9];

void CalcPjConv(void)
{
	float px[4], py[4];
	float d23, d24, d34;
	int16_t i, j;
	float d;

//	px[0] = 0;
//	py[0] = 0;
	px[1] = (PM_GRID[8][0].x - PM_GRID[0][0].x) / (float)PS_WP_HW;
	py[1] = (PM_GRID[8][0].y - PM_GRID[0][0].y) / (float)PS_WP_VW;
	px[2] = (PM_GRID[0][8].x - PM_GRID[0][0].x) / (float)PS_WP_HW;
	py[2] = (PM_GRID[0][8].y - PM_GRID[0][0].y) / (float)PS_WP_VW;
	px[3] = (PM_GRID[8][8].x - PM_GRID[0][0].x) / (float)PS_WP_HW;
	py[3] = (PM_GRID[8][8].y - PM_GRID[0][0].y) / (float)PS_WP_VW;

	d23 = px[1] * py[2] - px[2] * py[1];
	d24 = px[1] * py[3] - px[3] * py[1];
	d34 = px[2] * py[3] - px[3] * py[2];

	GV_PJC_A[0] = d24 - d23;
	GV_PJC_B[0] = -d34 - d23;
	GV_PJC_C[0] = d34 + d23 - d24;

	GV_PJC_A[1] = px[1] * d34;
	GV_PJC_B[1] = px[2] * (-d24);
//	GV_PJC_C[1] = 0;

	GV_PJC_A[2] = py[1] * d34;
	GV_PJC_B[2] = py[2] * (-d24);
//	GV_PJC_C[2] = 0;

	// h
	j = 0;
	for( i = 1; i < 8; i++ ) {
		if( ((GV_HGRID_CLC_EN[i]>>PM_WP_MODE)&0x01) == 1 ) {
			j = j + 1;
			d = (float)j / (float)GV_NUM_HGRID_m1;

			// TL ~ TR
			PM_GRID[i][0].x = (GV_PJC_A[1]*d) / (GV_PJC_A[0]*d + GV_PJC_C[0]) * (float)PS_WP_HW + PM_GRID[0][0].x;
			PM_GRID[i][0].y = (GV_PJC_A[2]*d) / (GV_PJC_A[0]*d + GV_PJC_C[0]) * (float)PS_WP_VW + PM_GRID[0][0].y;

			// BhL ~ BhR
			PM_GRID[i][8].x = (GV_PJC_A[1]*d + GV_PJC_B[1]) / (GV_PJC_A[0]*d + GV_PJC_B[0] + GV_PJC_C[0]) * (float)PS_WP_HW + PM_GRID[0][0].x;
			PM_GRID[i][8].y = (GV_PJC_A[2]*d + GV_PJC_B[2]) / (GV_PJC_A[0]*d + GV_PJC_B[0] + GV_PJC_C[0]) * (float)PS_WP_VW + PM_GRID[0][0].y;
		}
	}

	// v
	j = 0;
	for( i = 1; i < 8; i++ ) {
		if( ((GV_VGRID_CLC_EN[i]>>PM_WP_MODE)&0x01) == 1 ) {
			j = j + 1;
			d = (float)j / (float)GV_NUM_VGRID_m1;

			// TL ~ BL
			PM_GRID[0][i].x = (GV_PJC_B[1]*d) / (GV_PJC_B[0]*d + GV_PJC_C[0]) * (float)PS_WP_HW + PM_GRID[0][0].x;
			PM_GRID[0][i].y = (GV_PJC_B[2]*d) / (GV_PJC_B[0]*d + GV_PJC_C[0]) * (float)PS_WP_VW + PM_GRID[0][0].y;

			// TR ~ BR
			PM_GRID[8][i].x = (GV_PJC_A[1] + GV_PJC_B[1]*d) / (GV_PJC_A[0] + GV_PJC_B[0]*d + GV_PJC_C[0]) * (float)PS_WP_HW + PM_GRID[0][0].x;
			PM_GRID[8][i].y = (GV_PJC_A[2] + GV_PJC_B[2]*d) / (GV_PJC_A[0] + GV_PJC_B[0]*d + GV_PJC_C[0]) * (float)PS_WP_VW + PM_GRID[0][0].y;
		}
	}
}
