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



line_coef_t GV_WP_HOR_LINE[8][9];


// line_coef_t GV_WP_HOR_LINE[8][9];
 line_coef_t GV_WP_HOR_LINE_MV[8]; 
 line_coef_t GV_WP_VER_LINE[9][8];
 line_coef_t GV_WP_VER_LINE_MV[130][8];
// coord_f_t PM_GRID[9][9];

void CalcLineCoef(void)
{
	int16_t i, j;
	float xp[9], yp[9];
	float h[11], yd[11];
	float a[11], b[11], c[11], d[11], xk[9];

	// horizontal line
	for( j = 0; j < 9; j++ ) {
		for( i = 0; i < 9; i++ ) {
			xp[i] = PM_GRID[i][j].x;	yp[i] = PM_GRID[i][j].y;
		}

		for( i = 2; i < 10; i++ ) {
			h[i]  = xp[i-1] - xp[i-2];
			yd[i] = ( yp[i-1] - yp[i-2] ) / h[i];
		}
		// dummy
		h[0] =h[2];  h[1] =h[2];  h[10] =h[9];
		yd[0]=yd[2]; yd[1]=yd[2]; yd[10]=yd[9];

		for( i = 1; i < 10; i++ ) {
			a[i] = 2 * ( h[i] + h[i+1] );
			b[i] = h[i+1];
			c[i] = h[i];
			d[i] = 3 * ( yd[i+1] - yd[i] );
		}
		// dummy
		a[0] = a[1]; a[10] = a[9];
		b[0] = b[1]; b[10] = b[9];
		c[0] = c[1]; c[10] = c[9];
		d[0] = d[1]; d[10] = d[9];

		for( i = 0; i < 9; i++ ) {
			xk[i] = ( a[i]*a[i+2]*d[i+1] - a[i+2]*c[i+1]*d[i] - a[i]*b[i+1]*d[i+2] ) / ( a[i]*a[i+1]*a[i+2] - a[i+2]*b[i]*c[i+1] - a[i]*b[i+1]*c[i+2] );
		}

		for( i = 0; i < 8; i++ ) {
			GV_WP_HOR_LINE[i][j].c = xk[i];
			GV_WP_HOR_LINE[i][j].d = ( xk[i+1] - xk[i] ) / (3 * h[i+2]);
			GV_WP_HOR_LINE[i][j].b = yd[i+2] - h[i+2] * ( GV_WP_HOR_LINE[i][j].c + GV_WP_HOR_LINE[i][j].d * h[i+2] );
			GV_WP_HOR_LINE[i][j].a = yp[i];
		}
	}

	// vertical line
	for( j = 0; j < 9; j++ ) {
		for( i = 0; i < 9; i++ ) {
			xp[i] = PM_GRID[j][i].y;		yp[i] = PM_GRID[j][i].x;
		}

		for( i = 2; i < 10; i++ ) {
			h[i]  = xp[i-1] - xp[i-2];
			yd[i] = ( yp[i-1] - yp[i-2] ) / h[i];
		}
		// dummy
		h[0] =h[2];  h[1] =h[2];  h[10] =h[9];
		yd[0]=yd[2]; yd[1]=yd[2]; yd[10]=yd[9];

		for( i = 1; i < 10; i++ ) {
			a[i] = 2 * ( h[i] + h[i+1] );
			b[i] = h[i+1];
			c[i] = h[i];
			d[i] = 3 * ( yd[i+1] - yd[i] );
		}
		// dummy
		a[0] = a[1]; a[10] = a[9];
		b[0] = b[1]; b[10] = b[9];
		c[0] = c[1]; c[10] = c[9];
		d[0] = d[1]; d[10] = d[9];

		for( i = 0; i < 9; i++ ) {
			xk[i] = ( a[i]*a[i+2]*d[i+1] - a[i+2]*c[i+1]*d[i] - a[i]*b[i+1]*d[i+2] ) / ( a[i]*a[i+1]*a[i+2] - a[i+2]*b[i]*c[i+1] - a[i]*b[i+1]*c[i+2] );
		}

		for( i = 0; i < 8; i++ ) {
			GV_WP_VER_LINE[j][i].c = xk[i];
			GV_WP_VER_LINE[j][i].d = ( xk[i+1] - xk[i] ) / ( 3 * h[i+2] );
			GV_WP_VER_LINE[j][i].b = yd[i+2] - h[i+2] * ( GV_WP_VER_LINE[j][i].c + GV_WP_VER_LINE[j][i].d * h[i+2] );
			GV_WP_VER_LINE[j][i].a = yp[i];
		}
	}
}

void CalcMVXCoef( float nx[] )
{
	int16_t i;
	float xp[9], yp[9];
	float h[11], yd[11];
	float a[11], b[11], c[11], d[11], xk[9];

	// horizontal line
	for( i = 0; i < 9; i++ ) {
		xp[i] = nx[i];	yp[i] = (float)PS_CUR_DEF_IX[i] - nx[i];
	}

	for( i= 2 ; i < 10; i++ ) {
		h[i]  = xp[i-1] - xp[i-2];
		yd[i] = ( yp[i-1] - yp[i-2] ) / h[i];
	}
	// dummy
	h[0] =h[2];  h[1] =h[2];  h[10] =h[9];
	yd[0]=yd[2]; yd[1]=yd[2]; yd[10]=yd[9];

	for( i = 1; i < 10; i++ ) {
		a[i] = 2 * ( h[i] + h[i+1] );
		b[i] = h[i+1];
		c[i] = h[i];
		d[i] = 3 * ( yd[i+1] - yd[i] );
	}
	// dummy
	a[0] = a[1]; a[10] = a[9];
	b[0] = b[1]; b[10] = b[9];
	c[0] = c[1]; c[10] = c[9];
	d[0] = d[1]; d[10] = d[9];

	for( i = 0; i < 9; i++ ) {
		xk[i] = ( a[i]*a[i+2]*d[i+1] - a[i+2]*c[i+1]*d[i] - a[i]*b[i+1]*d[i+2] ) / ( a[i]*a[i+1]*a[i+2] - a[i+2]*b[i]*c[i+1] - a[i]*b[i+1]*c[i+2] );
	}

	for( i = 0; i < 8; i++ ) {
		GV_WP_HOR_LINE_MV[i].c = xk[i];
		GV_WP_HOR_LINE_MV[i].d = ( xk[i+1] - xk[i] ) / ( 3 * h[i+2] );
		GV_WP_HOR_LINE_MV[i].b = yd[i+2] - h[i+2] * ( GV_WP_HOR_LINE_MV[i].c + GV_WP_HOR_LINE_MV[i].d * h[i+2] );
		GV_WP_HOR_LINE_MV[i].a = yp[i];
	}
}


void CalcMVYCoef( float ny[], int16_t xgrid )
{
	int16_t i;
	float xp[9], yp[9];
	float h[11], yd[11];
	float a[11], b[11], c[11], d[11], xk[9];

	// horizontal line
	for( i = 0; i < 9; i++ ) {
		xp[i] = ny[i];	yp[i] = (float)PS_CUR_DEF_IY[i] - ny[i];
	}

	for( i = 2; i < 10; i++ ) {
		h[i]  = xp[i-1] - xp[i-2];
		yd[i] = ( yp[i-1] - yp[i-2] ) / h[i];
	}
	// dummy
	h[0] =h[2];  h[1] =h[2];  h[10] =h[9];
	yd[0]=yd[2]; yd[1]=yd[2]; yd[10]=yd[9];

	for( i = 1; i < 10; i++ ) {
		a[i] = 2 * ( h[i] + h[i+1] );
		b[i] = h[i+1];
		c[i] = h[i];
		d[i] = 3 * ( yd[i+1] - yd[i] );
	}
	// dummy
	a[0] = a[1]; a[10] = a[9];
	b[0] = b[1]; b[10] = b[9];
	c[0] = c[1]; c[10] = c[9];
	d[0] = d[1]; d[10] = d[9];

	for( i = 0; i < 9; i++ ) {
		xk[i] = ( a[i]*a[i+2]*d[i+1] - a[i+2]*c[i+1]*d[i] - a[i]*b[i+1]*d[i+2] ) / ( a[i]*a[i+1]*a[i+2] - a[i+2]*b[i]*c[i+1] - a[i]*b[i+1]*c[i+2] );
	}

	for( i = 0; i < 8; i++ ) {
		GV_WP_VER_LINE_MV[xgrid][i].c = xk[i];
		GV_WP_VER_LINE_MV[xgrid][i].d = ( xk[i+1] - xk[i] ) / ( 3* h[i+2] );
		GV_WP_VER_LINE_MV[xgrid][i].b = yd[i+2] - h[i+2] * ( GV_WP_VER_LINE_MV[xgrid][i].c + GV_WP_VER_LINE_MV[xgrid][i].d * h[i+2] );
		GV_WP_VER_LINE_MV[xgrid][i].a = yp[i];
	}
}




void CalcHLineCoef( int16_t hline )
{
	int16_t i;
	float xp[9], yp[9];
	float h[11], yd[11];
	float a[11], b[11], c[11], d[11], xk[9];

	// horizontal line
	for( i = 0; i < GV_NUM_HGRID; i++ ) {
		xp[i] = GV_CP[i][hline].x;	yp[i] = GV_CP[i][hline].y;
	}

	for( i = 2; i < GV_NUM_HGRID_p1; i++ ) {
		h[i]  = xp[i-1] - xp[i-2];
		yd[i] = ( yp[i-1] - yp[i-2] ) / h[i];
	}
	// dummy
	h[0] =h[2];  h[1] =h[2];  h[GV_NUM_HGRID_p1] =h[GV_NUM_HGRID];
	yd[0]=yd[2]; yd[1]=yd[2]; yd[GV_NUM_HGRID_p1]=yd[GV_NUM_HGRID];

	for( i = 1; i < GV_NUM_HGRID_p1; i++ ) {
		a[i] = 2 * ( h[i] + h[i+1] );
		b[i] = h[i+1];
		c[i] = h[i];
		d[i] = 3 * ( yd[i+1] - yd[i] );
	}
	// dummy
	a[0] = a[1]; a[GV_NUM_HGRID_p1] = a[GV_NUM_HGRID];
	b[0] = b[1]; b[GV_NUM_HGRID_p1] = b[GV_NUM_HGRID];
	c[0] = c[1]; c[GV_NUM_HGRID_p1] = c[GV_NUM_HGRID];
	d[0] = d[1]; d[GV_NUM_HGRID_p1] = d[GV_NUM_HGRID];

	for( i = 0; i < GV_NUM_HGRID; i++ ) {
		xk[i] = ( a[i]*a[i+2]*d[i+1] - a[i+2]*c[i+1]*d[i] - a[i]*b[i+1]*d[i+2] ) / ( a[i]*a[i+1]*a[i+2] - a[i+2]*b[i]*c[i+1] - a[i]*b[i+1]*c[i+2] );
	}

	for( i = 0; i < GV_NUM_HGRID_m1; i++ ) {
		GV_WP_HOR_LINE[i][hline].c = xk[i];
		GV_WP_HOR_LINE[i][hline].d = ( xk[i+1] - xk[i] ) / ( 3 * h[i+2] );
		GV_WP_HOR_LINE[i][hline].b = yd[i+2] - h[i+2] * ( GV_WP_HOR_LINE[i][hline].c + GV_WP_HOR_LINE[i][hline].d * h[i+2] );
		GV_WP_HOR_LINE[i][hline].a = yp[i];
	}
}

void CalcVLineCoef( int16_t vline )
{
	int16_t i;
	float xp[9], yp[9];
	float h[11], yd[11];
	float a[11], b[11], c[11], d[11], xk[9];

	// vertical line
	for( i = 0; i < GV_NUM_VGRID; i++ ) {
		xp[i] = GV_CP[vline][i].y;		yp[i] = GV_CP[vline][i].x;
	}

	for( i = 2; i < GV_NUM_VGRID_p1; i++ ) {
		h[i]  = xp[i-1] - xp[i-2];
		yd[i] = (yp[i-1] - yp[i-2])/h[i];
	}
	// dummy
	h[0] =h[2];  h[1] =h[2];  h[GV_NUM_VGRID_p1] =h[GV_NUM_VGRID];
	yd[0]=yd[2]; yd[1]=yd[2]; yd[GV_NUM_VGRID_p1]=yd[GV_NUM_VGRID];

	for( i = 1; i < GV_NUM_VGRID_p1; i++ ) {
		a[i] = 2 * ( h[i] + h[i+1] );
		b[i] = h[i+1];
		c[i] = h[i];
		d[i] = 3 * ( yd[i+1] - yd[i] );
	}
	// dummy
	a[0] = a[1]; a[GV_NUM_VGRID_p1] = a[GV_NUM_VGRID];
	b[0] = b[1]; b[GV_NUM_VGRID_p1] = b[GV_NUM_VGRID];
	c[0] = c[1]; c[GV_NUM_VGRID_p1] = c[GV_NUM_VGRID];
	d[0] = d[1]; d[GV_NUM_VGRID_p1] = d[GV_NUM_VGRID];

	for( i = 0; i < GV_NUM_VGRID; i++ ) {
		xk[i] = ( a[i]*a[i+2]*d[i+1] - a[i+2]*c[i+1]*d[i] - a[i]*b[i+1]*d[i+2] ) / ( a[i]*a[i+1]*a[i+2] - a[i+2]*b[i]*c[i+1] - a[i]*b[i+1]*c[i+2] );
	}

	for( i = 0; i < GV_NUM_VGRID_m1; i++ ) {
		GV_WP_VER_LINE[vline][i].c = xk[i];
		GV_WP_VER_LINE[vline][i].d = ( xk[i+1] - xk[i] ) / ( 3 * h[i+2] );
		GV_WP_VER_LINE[vline][i].b = yd[i+2] - h[i+2] * ( GV_WP_VER_LINE[vline][i].c + GV_WP_VER_LINE[vline][i].d * h[i+2] );
		GV_WP_VER_LINE[vline][i].a = yp[i];
	}
}
