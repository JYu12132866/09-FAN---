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

//#include "declare_gPI.h"
#include "declare_gVar.h"
#include "declare_gFunc.h"
#include "individual_param.h"
#include "c381.h"


#ifndef INCLUDED_DECLARE_GPS_H
#define INCLUDED_DECLARE_GPS_H

// parameter definition
//===== edgeblend mode =====//
extern int16_t PS_SPLIT_TYPE;
extern int16_t PS_SPLIT_IMAGE;

//===== parameters 1 =====//
extern int16_t PS_WP_HW;
extern int16_t PS_WP_VW;
extern int16_t PS_WP_HW_GRD;
extern int16_t PS_WP_VW_GRD;
extern int16_t PS_WP_BLACTHST;
extern int16_t PS_WP_BLACTVST;
extern float PS_WP_LIMH_ITV;
extern float PS_WP_LIMV_ITV;

extern int16_t PS_UC_HW;
extern int16_t PS_UC_VW;
extern int16_t PS_UC_HW_GRD_NUM;
extern int16_t PS_UC_VW_GRD_NUM;
extern int16_t PS_UC_HW_EDGRD;
extern int16_t PS_UC_VW_EDGRD;
extern int16_t PS_UC_HW_GRD_MAX;
extern int16_t PS_UC_VW_GRD_MAX;

//===== parameters 2 =====//
extern int16_t PS_CUR_DEF_IX[9];
extern int16_t PS_CUR_DEF_IY[9];
extern int16_t PS_CUR_DEF_OX[9];
extern int16_t PS_CUR_DEF_OY[9];

//===== parameters 3 =====//
extern int16_t PS_INPUT_ERR[4];

#endif
