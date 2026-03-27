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
//#include "declare_gPM.h"
//#include "declare_gPS.h"
#include "declare_gPI.h"
//#include "declare_gVar.h"
//#include "declare_gFunc.h"

#include "c381.h"


#ifndef INCLUDED_INDIVIDUAL_PARAM_H
#define INCLUDED_INDIVIDUAL_PARAM_H

typedef struct {//leo 20180629

	int16_t x;
	int16_t y;
} coord_t;

typedef struct {
	float x;
	float y;
} coord_f_t;

//===== Input ====//
extern int16_t IPI_ACT_HST[4];
extern int16_t IPI_ACT_HW[4];
extern int16_t IPI_ACT_VST[4];
extern int16_t IPI_ACT_VW[4];

extern int16_t IPM_IACT_HW[4];
extern int16_t IPM_IACT_VW[4];

extern int16_t IPS_CUR_DEF_IX[4][9];
extern int16_t IPS_CUR_DEF_IY[4][9];

//===== Warping ====//
extern int16_t IPM_SEL_GRID_X[4];
extern int16_t IPM_SEL_GRID_Y[4];
extern coord_f_t IPM_GRID[4][9][9];
extern int16_t IPM_WP_MODE[4];
extern int16_t IPM_CUR_INTERNAL_ON[4];

extern int16_t IGV_PREV_SEL_GRID_X[4];
extern int16_t IGV_PREV_SEL_GRID_Y[4];

extern int16_t IGV_WP_TABLE_COLOR[4];

//===== Edgeblend Gain ====//
extern int16_t IPS_SPLIT_IMAGE[4];// splitter

extern int16_t IPM_EGB_LEN[4];
extern int16_t IPM_EGB_REN[4];
extern int16_t IPM_EGB_TEN[4];
extern int16_t IPM_EGB_BEN[4];

//===== Edgeblend Bias ====//
extern coord_t IPM_EBIAS_CUR[4][2][2];
extern int16_t IPM_EBIAS[4][16][3];

//===== Test Pattern ====//
extern int16_t IPM_OFILL_EN[4];

//===== Uniformity ====//
extern int16_t IPM_UC_EN[4];
extern float IPM_UC_GAIN[4][3][3][11][9];

//===== Color Convergence ====//
extern coord_t IPM_WP_CCVG[4][3];

//===== OSD ====//
extern int16_t IPS_WP_BLACTHST[4];
extern int16_t IPS_WP_BLACTVST[4];

//===== Register ====//
extern int16_t IGV_RTCT[4];
extern int16_t IGV_EGBCT[4];

//===== Write register control parameters for evbBoard =====//
extern int16_t IGV_MCR64BANK[4];	// for write(read)_C381()

#endif
