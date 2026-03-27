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



char HexToChar( int16_t val )
{
  	char ch;

	switch ( val ) {
		case 0  : ch = '0'; break;
		case 1  : ch = '1'; break;
		case 2  : ch = '2'; break;
		case 3  : ch = '3'; break;
		case 4  : ch = '4'; break;
		case 5  : ch = '5'; break;
		case 6  : ch = '6'; break;
		case 7  : ch = '7'; break;
		case 8  : ch = '8'; break;
		case 9  : ch = '9'; break;
		case 10 : ch = 'A'; break;
		case 11 : ch = 'B'; break;
		case 12 : ch = 'C'; break;
		case 13 : ch = 'D'; break;
		case 14 : ch = 'E'; break;
		case 15 : ch = 'F'; break;
	//	default : ch = -1;  break;
		default : ch = 0; break;
	}

	return ch;
}

int16_t CharToHex( char ch )
{
	char val;

	switch( ch ) {
		case '0': val = 0; break;
		case '1': val = 1; break;
		case '2': val = 2; break;
		case '3': val = 3; break;
		case '4': val = 4; break;
		case '5': val = 5; break;
		case '6': val = 6; break;
		case '7': val = 7; break;
		case '8': val = 8; break;
		case '9': val = 9; break;
		case 'a':
		case 'A': val = 10; break;
		case 'b':
		case 'B': val = 11; break;
		case 'c':
		case 'C': val = 12; break;
		case 'd':
		case 'D': val = 13; break;
		case 'e':
		case 'E': val = 14; break;
		case 'f':
		case 'F': val = 15; break;
//		default : val = -1; break;
		default : val = 0; break;
	}

	return val;
}
