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

void SetUniformityCursorEn( int16_t en )
{
#if 0//HPC add
	if ( en == 0 ) {
		C381_Write_Bank_Reg(16, B16_CURCT, 0x10);
		C381_Write_Bank_Reg(16, B16_CURCT2, 0x00);
		GV_UC_CUR_EN[0] = 0;
	} else {
		C381_Write_Bank_Reg(16, B16_CURCT, 0x11);
		C381_Write_Bank_Reg(16, B16_CURCT2, 0x00);		
		GV_UC_CUR_EN[0] = 1;
	}
#endif//HPC add
}

void SetUniformityCursorPos( int16_t grid_num_x, int16_t grid_num_y )
{
#if 0//HPC add
	int16_t x;
	int16_t y;

	x = (grid_num_x == PS_UC_HW_GRD_MAX) ? (grid_num_x - 1) * PS_UC_HW + PS_UC_HW_EDGRD : grid_num_x * PS_UC_HW;
	if ( x < 0 ) {
		x = 0;
	} else if ( x > PS_WP_HW - Def_GRD_Size ) {
		x = PS_WP_HW - Def_GRD_Size;
	}
	C381_Write_Bank_Reg( 16,B16_CURTLX, (uint8_t)x);
	C381_Write_Bank_Reg( 16,B16_CURTLX2,(uint8_t)( x>>8));

	y = (grid_num_y == PS_UC_VW_GRD_MAX) ? (grid_num_y - 1) * PS_UC_VW + PS_UC_VW_EDGRD : grid_num_y * PS_UC_VW;
	if ( y < 0 ) {
		y = 0;
	} else if ( y > (PS_WP_VW - Def_GRD_Size) ) {
		y = PS_WP_VW - Def_GRD_Size;
	}
	C381_Write_Bank_Reg(16, B16_CURTLY, (uint8_t)y);
	C381_Write_Bank_Reg(16, B16_CURTLY2,(uint8_t)( y>>8));
#endif//HPC add
}

void InitUniformity( int16_t en  )
{
	SetupUniformityPos( en );
	ResetUniformity( en );
}

void ResetUniformity( int16_t en )
{
	int16_t xgrid, ygrid, level, color;
	int16_t err;

	for ( ygrid = 0; ygrid < PS_UC_VW_GRD_NUM; ygrid++ ) {
		for ( xgrid = 0; xgrid < PS_UC_HW_GRD_NUM; xgrid++ ) {
			for ( color = 0; color < 3; color++ ) {
				for ( level = 0; level < 3; level++ ) {
					PM_UC_GAIN[level][color][xgrid][ygrid] = 1;
				}
			}
		}
	}

	SetUniformityGain( en, 1, 0, 0, 0, &err );
}

void SetupUniformityPos( int16_t en )
{
#if 0//HPC add
	int16_t i, j;
	int16_t dt;

	SetUniformityEn( 0, 1 );

	C381_Write_Bank_Reg(14, B14_UCAD, 0x00);

	for ( j = 0; j < 11; j++ ) {
		dt = ( j >= PS_UC_HW_GRD_NUM ) ? 0xfff
		 : ( j == PS_UC_HW_GRD_MAX ) ? (j - 1) * PS_UC_HW + PS_UC_HW_EDGRD
		 : j * PS_UC_HW;
		for ( i = 0; i < 5; i++ ) {
			C381_Write_Bank_Reg(14, B14_UCDT, (dt & 0xff));
			dt = ( dt >> 8 );
		}
	}
	// V pos
	for ( j = 0; j < 9; j++ ) {
		dt = ( j >= PS_UC_VW_GRD_NUM ) ? 0xfff
		 : ( j == PS_UC_VW_GRD_MAX ) ? (j - 1) * PS_UC_VW + PS_UC_VW_EDGRD
		 : j * PS_UC_VW;
		for ( i = 0; i < 5; i++ ) {
			C381_Write_Bank_Reg(14, B14_UCDT, (dt & 0xff));
			dt = ( dt >> 8 );
		}
	}
#endif//HPC add
	SetUniformityEn( en, 0 );
}

void SetUniformityGain( int16_t en, int16_t all, int16_t color, int16_t xgrid, int16_t ygrid, int16_t *ept )
{
#if 0//HPC add
	int16_t ad[4];
	ucdt_t dt[360];// ucad = 20 ~ 379
	int16_t i;

	if ( all != 0 ) {
		i = 0;
		// ginit
		for ( color = 0; color < 3; color++ ) {
			dt[i] = CalcUniformityDataGINIT( color, ept );
			if ( *ept != E_UcNoErr ) { return; }
			i++;
		}

		// dg/dv
		for ( color = 0; color < 3; color++ ) {
			for ( ygrid = 0; ygrid < 9; ygrid++ ) {
				if ( ygrid >= PS_UC_VW_GRD_NUM ) {
					dt[i].byte0 = 0;
					dt[i].byte1 = 0;
					dt[i].byte2 = 0;
					dt[i].byte3 = 0;
					dt[i].byte4 = 0;
				}
				else {
					dt[i] = CalcUniformityDataDGDV( color, ygrid, ept );
					if ( *ept != E_UcNoErr ) { return; }
				}
				i++;
			}
		}

		// dg/dh
		for ( color = 0; color < 3; color++ ) {
			for ( xgrid = 0; xgrid < 11; xgrid++ ) {
				if ( xgrid >= PS_UC_HW_GRD_NUM ) {
					dt[i].byte0 = 0;
					dt[i].byte1 = 0;
					dt[i].byte2 = 0;
					dt[i].byte3 = 0;
					dt[i].byte4 = 0;
				}
				else {
					dt[i] = CalcUniformityDataDGDH( color, xgrid, ept );
					if ( *ept != E_UcNoErr ) { return; }
				}
				i++;
			}
		}

		// (dg/dh)/dv
		for ( ygrid = 0; ygrid < 9; ygrid++ ) {
			for ( color = 0; color < 3; color++ ) {
				for ( xgrid = 0; xgrid < 11; xgrid++ ) {
					if ( (ygrid >= PS_UC_VW_GRD_NUM) || (xgrid >= PS_UC_HW_GRD_NUM) ) {
						dt[i].byte0 = 0;
						dt[i].byte1 = 0;
						dt[i].byte2 = 0;
						dt[i].byte3 = 0;
						dt[i].byte4 = 0;
					}
					else {
						dt[i] = CalcUniformityDataDGDHDV( color, xgrid, ygrid, ept );
						if ( *ept != E_UcNoErr ) { return; }
					}
					i++;
				}
			}
		}


		SetUniformityEn( 0, 1 );
		ad[0] = 20;
		C381_Write_Bank_Reg(14, B14_UCAD, (ad[0] & 0xff));
		C381_Write_Bank_Reg(14, B14_UCAD, ((ad[0] >> 8) & 0x03));
		
		for ( i = 0; i < 360; i++ ) {// ucad = 20 ~ 379
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte0 & 0xff));
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte1 & 0xff));
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte2 & 0xff));
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte3 & 0xff));
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte4 & 0xff));

		}
		SetUniformityEn( en, 0 );
	}
	else {
		if ( (xgrid == 0) && (ygrid == 0) ) {
			// ginit
			ad[0] = 20 + color;
			dt[0] = CalcUniformityDataGINIT( color, ept );
			if ( *ept != E_UcNoErr ) { return; }
			// dg/dv
			ad[1] = 23 + color * 9;
			dt[1] = CalcUniformityDataDGDV( color, ygrid, ept );
			if ( *ept != E_UcNoErr ) { return; }
			// dg/dh
			ad[2] = 50 + color * 11;
			dt[2] = CalcUniformityDataDGDH( color, xgrid, ept );
			if ( *ept != E_UcNoErr ) { return; }
			// (dg/dh)/dv
			ad[3] = 83 + color * 11;
			dt[3] = CalcUniformityDataDGDHDV( color, xgrid, ygrid, ept );
			if ( *ept != E_UcNoErr ) { return; }

			SetUniformityEn( 0, 1 );
			for ( i = 0; i < 4; i++ ) {
			C381_Write_Bank_Reg(14, B14_UCAD, ( ad[i] & 0xff));
			C381_Write_Bank_Reg(14, B14_UCAD, (ad[i] >> 8) & 0x03);
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte0 & 0xff));
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte1 & 0xff));
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte2 & 0xff));
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte3 & 0xff));
			C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte4 & 0xff));

			}
			SetUniformityEn( en, 0 );
		}
		else if ( xgrid == 0 ) {
			// dg/dv
			// upper
			ad[0] = 23 + (ygrid - 1) + color * 9;
			dt[0] = CalcUniformityDataDGDV( color, (ygrid - 1), ept );
			if ( *ept != E_UcNoErr ) { return; }
			// current
			ad[1] = 23 + ygrid + color * 9;
			dt[1] = CalcUniformityDataDGDV( color, ygrid, ept );
			if ( *ept != E_UcNoErr ) { return; }
			// (dg/dh)/dv
			// upper
			ad[2] = 83 + (ygrid - 1) * 33 + xgrid + color * 11;
			dt[2] = CalcUniformityDataDGDHDV( color, xgrid, (ygrid - 1), ept );
			if ( *ept != E_UcNoErr ) { return; }
			// current
			ad[3] = 83 + ygrid * 33 + xgrid + color * 11;
			dt[3] = CalcUniformityDataDGDHDV( color, xgrid, ygrid, ept );
			if ( *ept != E_UcNoErr ) { return; }

			SetUniformityEn( 0, 1 );
			
			for ( i = 0; i < 4; i++ ) {
				C381_Write_Bank_Reg(14, B14_UCAD, ( ad[i] & 0xff));
				C381_Write_Bank_Reg(14, B14_UCAD, (ad[i] >> 8) & 0x03);
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte0 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte1 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte2 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte3 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte4 & 0xff));

			}
			SetUniformityEn( en, 0 );
		}
		else if ( ygrid == 0 ) {
			// dg/dh
			// left
			ad[0] = 50 + (xgrid - 1) + color * 11;
			dt[0] = CalcUniformityDataDGDH( color, (xgrid - 1), ept );
			if ( *ept != E_UcNoErr ) { return; }
			// current
			ad[1] = 50 + xgrid + color * 11;
			dt[1] = CalcUniformityDataDGDH( color, xgrid, ept );
			if ( *ept != E_UcNoErr ) { return; }
			// (dg/dh)/dv
			// left
			ad[2] = 83 + ygrid * 33 + (xgrid - 1) + color * 11;
			dt[2] = CalcUniformityDataDGDHDV( color, (xgrid - 1), ygrid, ept );
			if ( *ept != E_UcNoErr ) { return; }
			// current
			ad[3] = 83 + ygrid * 33 + xgrid + color * 11;
			dt[3] = CalcUniformityDataDGDHDV( color, xgrid, ygrid, ept );
			if ( *ept != E_UcNoErr ) { return; }

			SetUniformityEn( 0, 1 );
			for ( i = 0; i < 4; i++ ) {
				C381_Write_Bank_Reg(14, B14_UCAD, ( ad[i] & 0xff));
				C381_Write_Bank_Reg(14, B14_UCAD, (ad[i] >> 8) & 0x03);
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte0 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte1 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte2 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte3 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte4 & 0xff));

			}
			SetUniformityEn( en, 0 );
		}
		else {
			// (dg/dh)/dv
			// upper left
			ad[0] = 83 + (ygrid - 1) * 33 + (xgrid - 1) + color * 11;
			dt[0] = CalcUniformityDataDGDHDV( color, (xgrid - 1), (ygrid - 1), ept );
			if ( *ept != E_UcNoErr ) { return; }
			// upper
			ad[1] = 83 + (ygrid - 1) * 33 + xgrid + color * 11;
			dt[1] = CalcUniformityDataDGDHDV( color, xgrid, (ygrid - 1), ept );
			if ( *ept != E_UcNoErr ) { return; }
			// left
			ad[2] = 83 + ygrid * 33 + (xgrid - 1) + color * 11;
			dt[2] = CalcUniformityDataDGDHDV( color, (xgrid - 1), ygrid, ept );
			if ( *ept != E_UcNoErr ) { return; }
			// current
			ad[3] = 83 + ygrid * 33 + xgrid + color * 11;
			dt[3] = CalcUniformityDataDGDHDV( color, xgrid, ygrid, ept );
			if ( *ept != E_UcNoErr ) { return; }

			SetUniformityEn( 0, 1 );
			for ( i = 0; i < 4; i++ ) {
				C381_Write_Bank_Reg(14, B14_UCAD, ( ad[i] & 0xff));
				C381_Write_Bank_Reg(14, B14_UCAD, (ad[i] >> 8) & 0x03);
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte0 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte1 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte2 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte3 & 0xff));
				C381_Write_Bank_Reg(14, B14_UCDT, (dt[i].byte4 & 0xff));

			}
			SetUniformityEn( en, 0 );
		}
	}
#endif//HPC add
}

ucdt_t CalcUniformityDataGINIT( int16_t color, int16_t *ept )
{
	int16_t level;
	float temp_f;
	int16_t temp_i;
	int16_t reg_gain_dt[3] = { 0, 0, 0 };
	ucdt_t dt = { 0, 0, 0, 0, 0 };

	*ept = E_UcNoErr;

	for ( level = 0; level < 3; level++ ) {
		temp_f = PM_UC_GAIN[level][color][0][0] * (float)Def_POW_2_10 + 0.5f;// round
		temp_i = (int16_t)temp_f;
		if ( temp_i < 0 ) {
			reg_gain_dt[level] = 0x000;
			*ept = E_UcErrGinitMin;
		}
		else if ( temp_i > 0x7ff ) {
			reg_gain_dt[level] = 0x7ff;
			*ept = E_UcErrGinitMax;
		}
		else {
			reg_gain_dt[level] = temp_i;
		}
	}

	temp_i = reg_gain_dt[0] & 0x0ff;
	dt.byte0 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[0] & 0xf00) >> 8 ) + ( (reg_gain_dt[1] & 0x00f) << 4 );
	dt.byte1 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[1] & 0xff0) >> 4 );
	dt.byte2 = (unsigned char)temp_i;
	temp_i = reg_gain_dt[2] & 0x0ff;
	dt.byte3 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[2] & 0xf00) >> 8 );
	dt.byte4 = (unsigned char)temp_i;

	return dt;
}

ucdt_t CalcUniformityDataDGDV( int16_t color, int16_t ygrid, int16_t *ept )
{
	int16_t level;
	float temp_f;
	int16_t temp_i;
	int16_t reg_gain_dt[3] = { 0, 0, 0 };
	ucdt_t dt = { 0, 0, 0, 0, 0 };
	float ucvw;

	if ( (ygrid < 0) || (ygrid >= 9) ) {
		*ept = E_UcErr;
		return dt;
	}

	*ept = E_UcNoErr;

	ucvw = ( (ygrid+1) == PS_UC_VW_GRD_MAX ) ? (float)PS_UC_VW_EDGRD : (float)PS_UC_VW;
	for ( level = 0; level < 3; level++ ) {
		temp_f = ( PM_UC_GAIN[level][color][0][ygrid+1] - PM_UC_GAIN[level][color][0][ygrid] ) / ucvw * (float)Def_POW_2_20;
		if ( temp_f < 0 ) {
			temp_i = Def_POW_2_12 - (int16_t)( -temp_f + 0.5f );
			if ( temp_i < Def_POW_2_11 ) {
				temp_i = Def_POW_2_11;
				*ept = E_UcErrDGDVMin;
			}
		}
		else {
			temp_i = (int16_t)( temp_f + 0.5f );
			if ( temp_i >= Def_POW_2_11 ) {
				temp_i = Def_POW_2_11 - 1;
				*ept = E_UcErrDGDVMax;
			}
		}
		reg_gain_dt[level] = temp_i;
	}

	temp_i = reg_gain_dt[0] & 0x0ff;
	dt.byte0 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[0] & 0xf00) >> 8 ) + ( (reg_gain_dt[1] & 0x00f) << 4 );
	dt.byte1 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[1] & 0xff0) >> 4 );
	dt.byte2 = (unsigned char)temp_i;
	temp_i = reg_gain_dt[2] & 0x0ff;
	dt.byte3 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[2] & 0xf00) >> 8 );
	dt.byte4 = (unsigned char)temp_i;

	return dt;
}

ucdt_t CalcUniformityDataDGDH( int16_t color, int16_t xgrid, int16_t *ept )
{
	int16_t level;
	float temp_f;
	int16_t temp_i;
	int16_t reg_gain_dt[3] = { 0, 0, 0 };
	ucdt_t dt = { 0, 0, 0, 0, 0 };
	float uchw;

	if ( (xgrid < 0) || (xgrid >= 11) ) {
		*ept = E_UcErr;
		return dt;
	}

	*ept = E_UcNoErr;

	uchw = ( (xgrid+1) == PS_UC_HW_GRD_MAX ) ? (float)PS_UC_HW_EDGRD : (float)PS_UC_HW;
	for ( level = 0; level < 3; level++ ) {
		temp_f = ( PM_UC_GAIN[level][color][xgrid+1][0] - PM_UC_GAIN[level][color][xgrid][0] ) / uchw * (float)Def_POW_2_20;
		if ( temp_f < 0 ) {
			temp_i = Def_POW_2_12 - (int16_t)( -temp_f + 0.5f );
			if ( temp_i < Def_POW_2_11 ) {
				temp_i = Def_POW_2_11;
				*ept = E_UcErrDGDHMin;
			}
		}
		else {
			temp_i = (int16_t)( temp_f + 0.5f );
			if ( temp_i >= Def_POW_2_11 ) {
				temp_i = Def_POW_2_11 - 1;
				*ept = E_UcErrDGDHMax;
			}
		}

		reg_gain_dt[level] = temp_i;
	}

	temp_i = reg_gain_dt[0] & 0x0ff;
	dt.byte0 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[0] & 0xf00) >> 8 ) + ( (reg_gain_dt[1] & 0x00f) << 4 );
	dt.byte1 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[1] & 0xff0) >> 4 );
	dt.byte2 = (unsigned char)temp_i;
	temp_i = reg_gain_dt[2] & 0x0ff;
	dt.byte3 = (unsigned char)temp_i;
	temp_i = ( (reg_gain_dt[2] & 0xf00) >> 8 );
	dt.byte4 = (unsigned char)temp_i;

	return dt;
}

ucdt_t CalcUniformityDataDGDHDV( int16_t color, int16_t xgrid, int16_t ygrid, int16_t *ept )
{
	int16_t level;
	float temp_f;
	int16_t temp_i;
	int16_t reg_gain_dt[3] = { 0, 0, 0 };
	ucdt_t dt = { 0, 0, 0, 0, 0 };
	float uchw;
	float ucvw;

	if ( (xgrid < 0) || (ygrid < 0) || (xgrid >= 11) || (ygrid >= 9) ) {
		*ept = E_UcErr;
		return dt;
	}

	*ept = E_UcNoErr;

	// dg/dh/dv
	uchw = ( (xgrid+1) == PS_UC_HW_GRD_MAX ) ? (float)PS_UC_HW_EDGRD : (float)PS_UC_HW;
	ucvw = ( (ygrid+1) == PS_UC_VW_GRD_MAX ) ? (float)PS_UC_VW_EDGRD : (float)PS_UC_VW;
	if ( (ygrid < (Def_UC_VW_Max_GRD - 1)) && (xgrid < (Def_UC_HW_Max_GRD - 1)) ) {
		for ( level = 0; level < 3; level++ ) {
			temp_f = ( (PM_UC_GAIN[level][color][xgrid+1][ygrid+1] - PM_UC_GAIN[level][color][xgrid][ygrid+1]) - (PM_UC_GAIN[level][color][xgrid+1][ygrid] - PM_UC_GAIN[level][color][xgrid][ygrid]) ) / (uchw * ucvw) * (float)Def_POW_2_30;
			if ( temp_f < 0 ) {
				temp_i = Def_POW_2_12 - (int16_t)( -temp_f + 0.5f );
				if ( temp_i < Def_POW_2_11 ) {
					temp_i = Def_POW_2_11;
					*ept = E_UcErrDGDHDVMin;
				}
			}
			else {
				temp_i = (int16_t)( temp_f + 0.5f );
				if ( temp_i >= Def_POW_2_11 ) {
					temp_i = Def_POW_2_11 - 1;
					*ept = E_UcErrDGDHDVMax;
				}
			}
			reg_gain_dt[level] = temp_i;
		}

		temp_i = reg_gain_dt[0] & 0x0ff;
		dt.byte0 = (unsigned char)temp_i;
		temp_i = ( (reg_gain_dt[0] & 0xf00) >> 8 ) + ( (reg_gain_dt[1] & 0x00f) << 4 );
		dt.byte1 = (unsigned char)temp_i;
		temp_i = ( (reg_gain_dt[1] & 0xff0) >> 4 );
		dt.byte2 = (unsigned char)temp_i;
		temp_i = reg_gain_dt[2] & 0x0ff;
		dt.byte3 = (unsigned char)temp_i;
		temp_i = ( (reg_gain_dt[2] & 0xf00) >> 8 );
		dt.byte4 = (unsigned char)temp_i;
	}

	return dt;
}

void SetUniformityEn( int16_t en, int16_t acen )
{
#if 0//HPC add
	if ( en == 0 ) {
		if ( acen == 0 ) {
			GV_RTCT = (GV_RTCT & 0xf8) | 0x05;
			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));
			C381_Write_Bank_Reg(14, B14_UCCT, 0x04);// v0.56
		} else {
			GV_RTCT = GV_RTCT & 0xf8;
			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));
			C381_Write_Bank_Reg(14, B14_UCCT, 0x06);// v0.56

		}
	} else {
		if ( acen == 0 ) {
			GV_RTCT = (GV_RTCT & 0xf8) | 0x05;
			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));
			C381_Write_Bank_Reg(14, B14_UCCT, 0x05);// v0.56
		} else {
			GV_RTCT = GV_RTCT & 0xf8;
			C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
			C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));
			C381_Write_Bank_Reg(14, B14_UCCT, 0x07);// v0.56
		}
	}
#endif//HPC add
}
