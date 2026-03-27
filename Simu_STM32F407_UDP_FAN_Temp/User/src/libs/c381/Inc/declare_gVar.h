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

#include "declare_gDef.h"
//#include "declare_gPM.h"
//#include "declare_gPS.h"
#include "declare_gDef.h"

#include "declare_gFunc.h"
#include "individual_param.h"
#include "c381.h"

#ifndef INCLUDED_DECLARE_GVAR_H
#define INCLUDED_DECLARE_GVAR_H

typedef struct {
	float a;
	float b;
	float c;
	float d;
} line_coef_t;

typedef struct declare_reg_h_tag {
	char *name;
	int16_t address;
	struct declare_reg_h_tag *next;
} declare_reg_h_t;
// parameter definition

//===== Main control parameters =====//
extern int16_t button_num;
extern int16_t st_num;
extern int16_t FLG_AUTO_RETURN;
extern int32_t st_num_old;

//===== Warp Table parameters ====//
extern int16_t GV_WARP_TABLE_X[130][76];
extern int16_t GV_WARP_TABLE_Y[130][76];

//===== OSD Cursor parameters ====//
extern int16_t GV_NUM_HGRID;
extern int16_t GV_NUM_VGRID;
extern int16_t GV_NUM_HGRID_m1;
extern int16_t GV_NUM_VGRID_m1;
extern int16_t GV_NUM_HGRID_m2;
extern int16_t GV_NUM_VGRID_m2;
extern int16_t GV_NUM_HGRID_p1;
extern int16_t GV_NUM_VGRID_p1;
extern int16_t CursorMode;//LEO 
extern int16_t Movement;//LEO 
extern  int16_t CursorSel_LR;//LEO 
extern  int16_t CursorSel_UD;//LEO 
extern  int16_t Wraping_LR;//LEO 
extern  int16_t Wraping_UD;//LEO 
extern  int16_t Reset_Wrap;//LEO 
extern  int16_t Save_Wrap;//LEO 
extern  int16_t Load_Wrap;//LEO 

// extern  int16_t Fan_Error;//LEO 
extern  int16_t WP_VkeystoneStatus;//LEO 
extern  int16_t WP_HkeystoneStatus;//LEO 
extern  int16_t WP_Vkeystone;//LEO 
extern  int16_t WP_Hkeystone;//LEO 
extern uint8_t Fread;//LEO

extern  int16_t WP_HkeystoneOld;//LEO 
extern  int16_t WP_VkeystoneOld;//LEO 

extern int16_t GV_PREV_SEL_GRID_X;
extern int16_t GV_PREV_SEL_GRID_Y;
extern int16_t GV_WP_TABLE_COLOR;

extern char GV_GRID_DISP[9][9];
extern int16_t GV_GRID_EN[9][9];

extern int16_t GV_WP_MODE_WP;
extern int16_t GV_CUR_INTERNAL_ON_WP;

//===== warp parameters ====//
extern line_coef_t GV_WP_VER_LINE[9][8];
extern line_coef_t GV_WP_HOR_LINE[8][9];
extern line_coef_t GV_WP_HOR_LINE_MV[8];
extern line_coef_t GV_WP_VER_LINE_MV[130][8];
extern coord_f_t GV_CP[9][9];
extern int16_t GV_HGRID_CLC_EN[9];
extern int16_t GV_VGRID_CLC_EN[9];

//===== warp_2x2 parameters ====//
extern float GV_PJC_A[3];
extern float GV_PJC_B[3];
extern float GV_PJC_C[3];

//===== Edgeblend Gain ====//
extern int16_t GV_EGB_IHW;
extern int16_t GV_EGB_IVW;

//===== gamma parameters ====//
extern int16_t GV_TEMP_GAMMA_TABLE[1024];

//===== Uniformity ====//
extern int16_t GV_UC_CUR_EN[4];
extern float GV_UC_GAIN_ORG[3][3];

//===== Register parameters =====//
extern int16_t GV_RTCT;
extern int16_t GV_EGBCT;

//===== old parameters for input image splitter ====//
extern int16_t GV_EGB_OHW_OLD;
extern int16_t GV_EGB_OVW_OLD;

//===== load_register_list =====//
extern declare_reg_h_t *g_reglist_head;

//===== Write register control parameters for evbBoard =====//
extern int16_t g_current_bank;

#endif
