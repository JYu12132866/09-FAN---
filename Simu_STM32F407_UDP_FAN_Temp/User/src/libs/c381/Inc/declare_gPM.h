// Copyright (C) 2016 i-Chips Technology Inc. All Rights Reserved.

#include <string.h>
#include <stdio.h>
//#include <conio.h>			
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include "arm_math.h"
//#include "stm32f4xx_conf.h"

//#include "declare_gDef.h"

//#include "declare_gPS.h"
//#include "declare_gPI.h"
//#include "declare_gVar.h"
//#include "declare_gFunc.h"
#include "individual_param.h"
#include "c381.h"


#ifndef INCLUDED_DECLARE_GPM_H
#define INCLUDED_DECLARE_GPM_H

// parameter definition
extern int16_t PM_BOARD;
extern int16_t PM_BOARD_OLD;

extern int16_t PM_IACT_HW;
extern int16_t PM_IACT_VW;

//===== Warping ====//
extern int16_t PM_SEL_GRID_X;
extern int16_t PM_SEL_GRID_Y;
extern coord_f_t PM_GRID[9][9];
extern coord_f_t PM_GRID_OLD[9][9];
extern int16_t PM_WP_MODE;
extern int16_t PM_CUR_ON;
extern int16_t PM_CUR_INTERNAL_ON;
extern float PM_GRID_MV;

//===== Edgeblend Gain ====//
extern int16_t PM_EGB_OHW;
extern int16_t PM_EGB_OVW;
extern int16_t PM_EGB_EN;
extern int16_t PM_EGB_LEN;
extern int16_t PM_EGB_REN;
extern int16_t PM_EGB_TEN;
extern int16_t PM_EGB_BEN;

//===== Edgeblend Bias ====//
extern int16_t PM_EBIAS_EN;
extern int16_t PM_EBIAS_SEL_CUR_X;
extern int16_t PM_EBIAS_SEL_CUR_Y;
extern coord_t PM_EBIAS_CUR[2][2];
extern int16_t PM_EBIAS_PLT;
extern int16_t PM_EBIAS[16][3];
extern int16_t PM_EBIAS_AREA_TEST;
extern int16_t PM_EBIAS_CUR_MV;
extern int16_t PM_EBIAS_COLOR;

//===== Gamma ====//
extern int16_t PM_EGB_GAMMA_EN;
extern float PM_EGB_GAMMA[3];
extern int16_t PM_EGB_GAMMA_COLOR;

extern int16_t PM_GAMMA_EN;
extern float PM_GAMMA[Def_OGM_NUM][3];
extern int16_t PM_GAMMA_PIX[Def_OGM_NUM];
extern int16_t PM_GAMMA_PIX_SEL;
extern int16_t PM_GAMMA_COLOR;

extern float PM_EBIAS_GAMMA[3];
extern int16_t PM_EBIAS_GAMMA_COLOR;
extern int16_t PM_EBIAS_GMDT[16][3][16];

//===== Test Pattern ====//
extern int16_t PM_PATGEN;
extern int16_t PM_EGBMK_EN;
extern int16_t PM_OFILL_EN;

//===== Uniformity ====//
extern int16_t PM_UC_EN;
extern int16_t PM_UC_SEL_GRID_X;
extern int16_t PM_UC_SEL_GRID_Y;
extern float PM_UC_GAIN[3][3][11][9];// level, color, xgrid, ygrid
extern int16_t PM_UC_COLOR;
extern int16_t PM_UC_LEVEL;

//===== Color Convergence ====//
extern coord_t PM_WP_CCVG[3];
extern int16_t PM_WP_CCVG_COLOR;

//===== File ====//
extern int16_t PM_FILE_NUM;


//===== Branch Parameter =====//
extern int16_t PB_000;
extern int16_t PB_100_MV;
extern int16_t PB_200;
extern int16_t PB_210;
extern int16_t PB_220;
extern int16_t PB_240;
extern int16_t PB_340;
extern int16_t PB_390;
extern int16_t PB_400;
extern float PB_420;
extern float PB_440;
extern float PB_450;
extern int16_t PB_500;
extern int16_t PB_800;

#endif
