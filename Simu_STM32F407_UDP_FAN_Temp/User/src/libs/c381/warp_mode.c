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


int16_t GV_NUM_HGRID_m2;
int16_t GV_NUM_VGRID_m2;

int16_t GV_NUM_HGRID;
int16_t GV_NUM_HGRID_m1;
int16_t GV_NUM_VGRID;
int16_t GV_NUM_VGRID_m1;
int16_t GV_NUM_HGRID_p1;
int16_t GV_NUM_VGRID_p1;
int16_t GV_GRID_EN[9][9];
int16_t GV_HGRID_CLC_EN[9];
int16_t GV_VGRID_CLC_EN[9];

void WpmodeChange(int16_t wpmd)
{
	int16_t i, j;
	int16_t icuren;

	PM_WP_MODE = wpmd;
	GV_NUM_HGRID = (PM_WP_MODE==3) ? 9 :((PM_WP_MODE==2) ? 5 : 3);
	GV_NUM_VGRID = (PM_WP_MODE==3) ? 9 :((PM_WP_MODE==2) ? 5 : 3);
	GV_NUM_HGRID_m1 = GV_NUM_HGRID - 1;
	GV_NUM_VGRID_m1 = GV_NUM_VGRID - 1;
	GV_NUM_HGRID_m2 = GV_NUM_HGRID - 2;
	GV_NUM_VGRID_m2 = GV_NUM_VGRID - 2;
	GV_NUM_HGRID_p1 = GV_NUM_HGRID + 1;
	GV_NUM_VGRID_p1 = GV_NUM_VGRID + 1;

	for(j = 0; j < 9; j++) {
		for(i = 0; i < 9; i++) {
			icuren = ((PM_CUR_INTERNAL_ON==0) && (i>0) && (i<8) && (j>0) && (j<8)) ? 0 : 1;

			if (PM_WP_MODE == 0) {
				GV_GRID_EN[i][j] = ((i == 0) || (i == 8)) ? 1 : 0;
				GV_GRID_EN[i][j] = ((j == 0) || (j == 8)) ? GV_GRID_EN[i][j] & 0x1 : 0;

				if (j == 0) {
					GV_HGRID_CLC_EN[i] = ((i == 0) || (i == 4) || (i == 8)) ? 1 : 0;
				}
				if (i == 0) {
					GV_VGRID_CLC_EN[j] = ((j == 0) || (j == 4) || (j == 8)) ? 1 : 0;
				}
			}
			else if (PM_WP_MODE == 1) {
				if (icuren == 0) {
					GV_GRID_EN[i][j] = 0;
				}
				else {
					GV_GRID_EN[i][j] = ((i == 0) || (i == 4) || (i == 8)) ? 1 : 0;
					GV_GRID_EN[i][j] = ((j == 0) || (j == 4) || (j == 8)) ? GV_GRID_EN[i][j] & 0x1 : 0;
				}

				if (j == 0) {
					GV_HGRID_CLC_EN[i] = ((i == 0) || (i == 4) || (i == 8)) ? 1 : 0;
				}
				if (i == 0) {
					GV_VGRID_CLC_EN[j] = ((j == 0) || (j == 4) || (j == 8)) ? 1 : 0;
				}
			}
			else if (PM_WP_MODE == 2) {
				if (icuren == 0) {
					GV_GRID_EN[i][j] = 0;
				}
				else {
					GV_GRID_EN[i][j] = ((i == 0) || (i == 2) || (i == 4) || (i == 6) || (i == 8)) ? 1 : 0;
					GV_GRID_EN[i][j] = ((j == 0) || (j == 2) || (j == 4) || (j == 6) || (j == 8)) ? GV_GRID_EN[i][j] & 0x1 : 0;
				}

				if (j == 0) {
					GV_HGRID_CLC_EN[i] = ((i == 0) || (i == 2) || (i == 4) || (i == 6) || (i == 8)) ? 1 : 0;
				}
				if (i == 0) {
					GV_VGRID_CLC_EN[j] = ((j == 0) || (j == 2) || (j == 4) || (j == 6) || (j == 8)) ? 1 : 0;
				}
			}
			else {
				if (icuren == 0) {
					GV_GRID_EN[i][j] = 0;
				}
				else {
					GV_GRID_EN[i][j] = 1;
				}

				if (j == 0) {
					GV_HGRID_CLC_EN[i] = 1;
				}
				if (i == 0) {
					GV_VGRID_CLC_EN[j] = 1;
				}
			}
		}
	}

	WpmdGridChange(0, 0);

	return;
}

void WpmdGridChange(int16_t xdir, int16_t ydir)
{
	int16_t i, j;
	int16_t wpmd_change = ((xdir==0) && (ydir==0)) ? 1 : 0;
  int16_t cnt=8000;
	if(wpmd_change != 0) {
		if(GV_GRID_EN[PM_SEL_GRID_X][PM_SEL_GRID_Y] == 0) {
			PM_SEL_GRID_X = 0;
			PM_SEL_GRID_Y = 0;
		}
	}
	else {
		do {
			cnt=cnt-1;
			i = (xdir==0) ? PM_SEL_GRID_X
				: ((xdir>0) ? ((PM_SEL_GRID_X == 8) ? 8 : PM_SEL_GRID_X + 1)
				: ((PM_SEL_GRID_X == 0) ? 0 : PM_SEL_GRID_X - 1));
			j = (ydir==0) ? PM_SEL_GRID_Y
				: ((ydir>0) ? ((PM_SEL_GRID_Y == 8) ? 8 : PM_SEL_GRID_Y + 1)
				: ((PM_SEL_GRID_Y == 0) ? 0 : PM_SEL_GRID_Y - 1));

			if((i==PM_SEL_GRID_X) && (j==PM_SEL_GRID_Y)) {
				break;
			}
			PM_SEL_GRID_X = i;
			PM_SEL_GRID_Y = j;
		} while(GV_GRID_EN[PM_SEL_GRID_X][PM_SEL_GRID_Y] == 0&&cnt>0);
	}

	return;
}
