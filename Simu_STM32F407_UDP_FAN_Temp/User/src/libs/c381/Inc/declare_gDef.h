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


//#include "declare_gPM.h"
//#include "declare_gPS.h"
//#include "declare_gPI.h"
//#include "declare_gVar.h"
//#include "declare_gFunc.h"
//#include "individual_param.h"
//#include "c381.h"

#ifndef INCLUDED_DECLARE_GDEF_H
#define INCLUDED_DECLARE_GDEF_H

// divide by 0
#define Def_DIV0 0xFFFFFFFF

// Buffer Length
#define Def_BUF_MAX	256

// power
#define Def_POW_2_10	1024
#define Def_POW_2_11	2048
#define Def_POW_2_12	4096
#define Def_POW_2_20	1048576
#define Def_POW_2_30	1073741824

//===== Warp Param =====//
#define Def_WpLimitMode		1// 0 : WpLimit OFF, 1 : WpLimit ON
#define Def_Wp_Space		16// 16, 32
#define Def_Wp_Space_Bit	4//((Def_Wp_Space==32) ? 5 : 4)

#define Def_HW_Max			2048
#define Def_VW_Max			1200
#define Def_HW_Max_GRD		130//133// = (Def_HW_Max + Def_Wp_Dummy * 2 + Def_Wp_Space - 1) / Def_Wp_Space + 1	[rounddown]
#define Def_VW_Max_GRD		76//76// = (Def_VW_Max + Def_Wp_Space - 1) / Def_Wp_Space + 1	[rounddown]

#define Def_HW_DUMMY_GRD_HST	2// = DEF_HW_DUMMY_F / DEF_WP_SPACE
#define Def_HW_DUMMY_GRD_HEND	2// = DEF_HW_DUMMY_B / DEF_WP_SPACE
#define Def_HW_DUMMY_HST		2//(Def_HW_DUMMY_GRD_HST<<Def_Wp_Space_Bit)// Dummy area(Correction to the outside)
#define Def_HW_DUMMY_HEND		2//(Def_HW_DUMMY_GRD_HEND<<Def_Wp_Space_Bit)// Dummy area(Correction to the outside)

//===== Initial value =====//
#define Def_PB_000_INIT	0
#define Def_PB_100_MV_INIT	4// = PM_GRID_MV
#define Def_PB_200_INIT	0
#define Def_PB_210_INIT	200// = PM_EGB_HW
#define Def_PB_220_INIT	200// = PM_EGB_VW
#define Def_PB_240_INIT	0
#define Def_PB_340_INIT	0
#define Def_PB_390_INIT	0
#define Def_PB_420_INIT	2.2// = PM_EGB_GAMMA
#define Def_PB_440_INIT	1// = PM_GAMMA
#define Def_PB_450_INIT	2.2// = PM_EBIAS_GAMMA
#define Def_PB_500_INIT	0
#define Def_PB_800_INIT	0


//===== Grid parameter =====//
#define Def_GRD_Size		8
#define Def_GRD_Size_o		8

//===== 4-Cursor =====//
#define Def_4CURSOR_Size	16

//===== Address =====//
#define Def_OSDSAD 0x0fd00000
#define Def_OSDMWI 0x10
#define Def_EBIASSAD 0x0f800000
#define Def_EBIASMWI 0x04

//===== Warp Lim =====//
#define Def_WPLIMANG_H	1.00f	// Limitation of H-slope : tan(45deg)
#define Def_WPLIMANG_V	5.67f	// Limitation of V-slope : tan(80deg)
#define Def_WPLIMVSH_L	(Def_Wp_Space/(1.0f/3.0f) * 16.0f)	// Limitation of local V-shrink rate
#define Def_WPLIMVSH_A	(Def_Wp_Space/(1.0f/2.0f) * 16.0f)	// Limitation of averate V-shrink rate
#define Def_WPLIMHSH	(Def_Wp_Space/(1.0f/12.0f) * 16.0f) 	// Limitation of H-shrink rate
#define Def_WPLIMITV_H	2	// Limitation of horizontal interval of grid : DEF_WPLIMITV_H > 0
#define Def_WPLIMITV_V	2	// Limitation of vertical interval of grid : DEF_WPLIMITV_V > 0

//===== Edgeblend area marker =====//
#define Def_EGBMK_L	0x24
#define Def_EGBMK_R	0x42
#define Def_EGBMK_T	0x24
#define Def_EGBMK_B	0x42

//===== Edgeblend Bias =====//
#define Def_EBIAS_STEP	1

//===== Edgeblend Gamma =====//
#define Def_EGBGM_STEP	0.005f

//===== Output Gamma =====//
#define Def_OGM_NUM	33
#define Def_OGM_COEF_STEP	0.005f

//===== Edgeblend Bias Gamma =====//
#define Def_EBSGM_STEP	0.005f

//===== Uniformity Param =====//
#define Def_UC_HW_GRD	256
#define Def_UC_VW_GRD	256
#define Def_UC_HW_Max_GRD	9// = roundup( Def_HW_Max / Def_UC_HW_GRD ) + 1
#define Def_UC_VW_Max_GRD	6// = roundup( Def_VW_Max / Def_UC_VW_GRD ) + 1


#endif
