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

char GV_GRID_DISP[9][9];

void EraseGrid( int16_t x, int16_t y )
{
#if 0//HPC add
	int32_t cpuwad;
	int16_t xst = (x==0) ? 0 : ((x==8) ? PS_CUR_DEF_IX[x] - (Def_GRD_Size + Def_GRD_Size_o) : PS_CUR_DEF_IX[x] - (Def_GRD_Size / 2 + Def_GRD_Size_o));
	int16_t xw  = ((x==0)||(x==8)) ? Def_GRD_Size + Def_GRD_Size_o : Def_GRD_Size + Def_GRD_Size_o * 2;
	int16_t yst = (y==0) ? 0 : ((y==8) ? PS_CUR_DEF_IY[y] - (Def_GRD_Size + Def_GRD_Size_o) : PS_CUR_DEF_IY[y] - (Def_GRD_Size / 2 + Def_GRD_Size_o));
	int16_t yw  = ((y==0)||(y==8)) ? Def_GRD_Size + Def_GRD_Size_o : Def_GRD_Size + Def_GRD_Size_o * 2;

	// erase grid
	cpuwad = Def_OSDSAD + xst + Def_OSDMWI * 128 * yst;

	GV_RTCT = GV_RTCT | 0x0007;
	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//		HAL_Delay(100);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

	
	C381_Write_Bank_Reg(0,B0_OSDFILL,0x00);
	C381_Write_Bank_Reg(0,B0_OSDFILL2,0x00);
	
	C381_Write_Bank_Reg(0,B0_BBACTHW,(uint8_t)(xw-1));
	C381_Write_Bank_Reg(0,B0_BBACTHW2,(uint8_t)(xw-1)>>8);

	C381_Write_Bank_Reg(0,B0_BBACTVW,(uint8_t)(yw-1));
	C381_Write_Bank_Reg(0,B0_BBACTVW2,(uint8_t)(yw-1)>>8);
	
	C381_Write_Bank_Reg(0,B0_CPUWAD,(uint8_t)cpuwad);
	C381_Write_Bank_Reg(0,B0_CPUWAD2,(uint8_t)(cpuwad>>8));
	C381_Write_Bank_Reg(0,B0_CPUWAD3,(uint8_t)(cpuwad>>16));
	C381_Write_Bank_Reg(0,B0_CPUWAD4,(uint8_t)(cpuwad>>24));

	C381_Write_Bank_Reg(0,B0_OSDCT,0x01);
	GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;

			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
///		HAL_Delay(100);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));


	do { } while ( (C381_Read_Bank_Reg(0, B0_BOSTAT) & 0x01) == 0x01 );
#endif//HPC add
	return;
}

void WriteGrid( int16_t x, int16_t y )
{
#if 0//HPC add
	int32_t cpuwad;
	int16_t xst;
	int16_t xw;
	int16_t yst;
	int16_t yw;

	if( (x == PM_SEL_GRID_X) && (y == PM_SEL_GRID_Y) ) {
		xst = (x==0) ? 0 : ((x==8) ? PS_CUR_DEF_IX[x] - (Def_GRD_Size + Def_GRD_Size_o) : PS_CUR_DEF_IX[x] - (Def_GRD_Size / 2 + Def_GRD_Size_o));
		xw  = ((x==0)||(x==8)) ? Def_GRD_Size + Def_GRD_Size_o : Def_GRD_Size + Def_GRD_Size_o * 2;
		yst = (y==0) ? 0 :( (y==8) ? PS_CUR_DEF_IY[y] - (Def_GRD_Size + Def_GRD_Size_o) : PS_CUR_DEF_IY[y] - (Def_GRD_Size / 2 + Def_GRD_Size_o));
		yw  = ((y==0)||(y==8)) ? Def_GRD_Size + Def_GRD_Size_o : Def_GRD_Size + Def_GRD_Size_o * 2;
		cpuwad = Def_OSDSAD + xst + Def_OSDMWI * 128 * yst;

	GV_RTCT = GV_RTCT | 0x0007;	
			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
		//		HAL_Delay(100);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));


		C381_Write_Bank_Reg(0,B0_OSDFILL,0x01);
		C381_Write_Bank_Reg(0,B0_OSDFILL2,0x00);

		C381_Write_Bank_Reg(0,B0_BBACTHW,(uint8_t)(xw-1));
		C381_Write_Bank_Reg(0,B0_BBACTHW2,(uint8_t)(xw-1)>>8);

		C381_Write_Bank_Reg(0,B0_BBACTVW,(uint8_t)(yw-1));
		C381_Write_Bank_Reg(0,B0_BBACTVW2,(uint8_t)(yw-1)>>8);

		C381_Write_Bank_Reg(0,B0_CPUWAD,(uint8_t)cpuwad);
		C381_Write_Bank_Reg(0,B0_CPUWAD2,(uint8_t)(cpuwad>>8));
		C381_Write_Bank_Reg(0,B0_CPUWAD3,(uint8_t)(cpuwad>>16));
		C381_Write_Bank_Reg(0,B0_CPUWAD4,(uint8_t)(cpuwad>>24));

		C381_Write_Bank_Reg(0,B0_OSDCT,0x01);
		GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;

				C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
			//		HAL_Delay(100);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));



	do { } while ( (C381_Read_Bank_Reg(0, B0_BOSTAT) & 0x01) == 0x01 );

		xst = (x==0) ? xst : xst + 2;
		xw  = ((x==0)||(x==8)) ? xw - 2 : xw - 4;
		yst = (y==0) ? yst : yst + 2;
		yw  = ((y==0)||(y==8)) ? yw - 2 : yw - 4;
		cpuwad = Def_OSDSAD + xst + Def_OSDMWI * 128 * yst;
		GV_RTCT = GV_RTCT | 0x0007;

				C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
			//		HAL_Delay(100);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));



		C381_Write_Bank_Reg(0,B0_OSDFILL,0x00);
		C381_Write_Bank_Reg(0,B0_OSDFILL2,0x00);

		C381_Write_Bank_Reg(0,B0_BBACTHW,(uint8_t)(xw-1));
		C381_Write_Bank_Reg(0,B0_BBACTHW2,(uint8_t)(xw-1)>>8);

		C381_Write_Bank_Reg(0,B0_BBACTVW,(uint8_t)(yw-1));
		C381_Write_Bank_Reg(0,B0_BBACTVW2,(uint8_t)(yw-1)>>8);

		C381_Write_Bank_Reg(0,B0_CPUWAD,(uint8_t)cpuwad);
		C381_Write_Bank_Reg(0,B0_CPUWAD2,(uint8_t)(cpuwad>>8));
		C381_Write_Bank_Reg(0,B0_CPUWAD3,(uint8_t)(cpuwad>>16));
		C381_Write_Bank_Reg(0,B0_CPUWAD4,(uint8_t)(cpuwad>>24));

		C381_Write_Bank_Reg(0,B0_OSDCT,0x01);

			GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;
			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
	//	//		HAL_Delay(100);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));



		do { } while ( (C381_Read_Bank_Reg(0, B0_BOSTAT) & 0x01) == 0x01 );
	}

	xst = (x==0) ? 0 :( (x==8) ? PS_CUR_DEF_IX[x] - Def_GRD_Size : PS_CUR_DEF_IX[x] - (Def_GRD_Size / 2));
	xw  = Def_GRD_Size;
	yst = (y==0) ? 0 : ((y==8) ? PS_CUR_DEF_IY[y] - Def_GRD_Size : PS_CUR_DEF_IY[y] - (Def_GRD_Size / 2));
	yw  = Def_GRD_Size;
	cpuwad = Def_OSDSAD + xst + Def_OSDMWI * 128 * yst;

		GV_RTCT = GV_RTCT | 0x0007;


		C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
	//		HAL_Delay(100);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

	C381_Write_Bank_Reg(0,B0_OSDFILL,0x01);
	C381_Write_Bank_Reg(0,B0_OSDFILL2,0x00);

	C381_Write_Bank_Reg(0,B0_BBACTHW,(uint8_t)(xw-1));
	C381_Write_Bank_Reg(0,B0_BBACTHW2,(uint8_t)(xw-1)>>8);
	
	C381_Write_Bank_Reg(0,B0_BBACTVW,(uint8_t)(yw-1));
	C381_Write_Bank_Reg(0,B0_BBACTVW2,(uint8_t)(yw-1)>>8);

	C381_Write_Bank_Reg(0,B0_CPUWAD,(uint8_t)cpuwad);
	C381_Write_Bank_Reg(0,B0_CPUWAD2,(uint8_t)(cpuwad>>8));
	C381_Write_Bank_Reg(0,B0_CPUWAD3,(uint8_t)(cpuwad>>16));
	C381_Write_Bank_Reg(0,B0_CPUWAD4,(uint8_t)(cpuwad>>24));

	C381_Write_Bank_Reg(0,B0_OSDCT,0x01);
	GV_RTCT = (GV_RTCT & 0xfff8) | 0x05;
		C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
	//		HAL_Delay(100);
		C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));



	do { } while ( (C381_Read_Bank_Reg(0, B0_BOSTAT) & 0x01) == 0x01 );
#endif //HPC add
	return;
}

void ChangeGrid( int16_t cur_disp )
{
#if 0//HPC add
	int16_t i, j;

	for( j = 0; j < 9; j++ ) {
		for( i = 0; i < 9; i++ ) {
			if( GV_GRID_DISP[i][j] != GV_GRID_EN[i][j] ) {
				if( GV_GRID_DISP[i][j] == 1 ) {
					EraseGrid( i, j );
					GV_GRID_DISP[i][j] = 0;
				}
				else {
					WriteGrid( i, j );
					GV_GRID_DISP[i][j] = 1;
				}
			}
			else if( (PM_SEL_GRID_X != GV_PREV_SEL_GRID_X) || (PM_SEL_GRID_Y != GV_PREV_SEL_GRID_Y) ) {
				if( ((i==PM_SEL_GRID_X) && (j==PM_SEL_GRID_Y)) || ((i==GV_PREV_SEL_GRID_X) && (j==GV_PREV_SEL_GRID_Y)) ) {
					EraseGrid( i, j );
					if( GV_GRID_DISP[i][j] == 1 ) {
						WriteGrid( i, j );
					}
				}
			}
		}
	}

	GV_PREV_SEL_GRID_X = PM_SEL_GRID_X;
	GV_PREV_SEL_GRID_Y = PM_SEL_GRID_Y;

		GV_RTCT = GV_RTCT | 0x0007;

		C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
	//		HAL_Delay(100);
		C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

	
	C381_Write_Bank_Reg(0,B0_OSDCT,0x00);
	if( (cur_disp == 1)&& (PM_CUR_ON == 1) ) 
		{
		
		C381_Write_Bank_Reg(3,B3_OSDMODECH1,0x03);
	}
	else {
		
		C381_Write_Bank_Reg(3,B3_OSDMODECH1,0x00);
	}
	GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;
	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//		HAL_Delay(100);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));
#endif//HPC add



	return;
}

void InitGrid( int16_t cur_disp )
{
#if 0//HPC add
	int16_t i, j;

	GV_RTCT = GV_RTCT | 0x0007;

	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//		HAL_Delay(100);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

	C381_Write_Bank_Reg(0,B0_OSDFILL,0x00);
	C381_Write_Bank_Reg(0,B0_OSDFILL2,0x00);
	
	C381_Write_Bank_Reg(0,B0_BBACTHW,(uint8_t)(PM_IACT_HW-1));
	C381_Write_Bank_Reg(0,B0_BBACTHW2,(uint8_t)((PM_IACT_HW-1)>>8));
	C381_Write_Bank_Reg(0,B0_BBACTVW,(uint8_t)(PM_IACT_VW-1));
	C381_Write_Bank_Reg(0,B0_BBACTVW2,(uint8_t)((PM_IACT_VW-1)>>8));

	C381_Write_Bank_Reg(0,B0_CPUWAD,(uint8_t)Def_OSDSAD);
	C381_Write_Bank_Reg(0,B0_CPUWAD2,(uint8_t)(Def_OSDSAD>>8));
	C381_Write_Bank_Reg(0,B0_CPUWAD3,(uint8_t)(Def_OSDSAD>>16));
	C381_Write_Bank_Reg(0,B0_CPUWAD4,(uint8_t)(Def_OSDSAD>>24));

	C381_Write_Bank_Reg(0,B0_OSDCT,0x01);

	GV_RTCT = (GV_RTCT & 0xfff8) | 0x0005;

	C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//		HAL_Delay(100);
	C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));


	do { } while ( (C381_Read_Bank_Reg(0,B0_BOSTAT)&0x01) == 0x01 );
	C381_Write_Bank_Reg(0, B0_OSDCT, 0x00 );
	for( j = 0; j < 9; j++ ) {
		for( i = 0; i < 9; i++ ) {
			GV_GRID_DISP[i][j] = 0;
		}
	}

	ChangeGrid( cur_disp );
#endif//HPC add
	return;
}
