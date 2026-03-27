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

void InitWpTable(void)
{
	int16_t x,y;

	for( y = 0; y < 9; y++ ) {
		for( x = 0; x < 9; x++ ) {
			PM_GRID[x][y].x = (float)PS_CUR_DEF_OX[x];
			PM_GRID[x][y].y = (float)PS_CUR_DEF_OY[y];
		}
	}
	MoveCur( 0, 0 );

	return;
}
