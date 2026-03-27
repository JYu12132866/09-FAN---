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



typedef struct {
	float a;
	float b;
} line_coef_ab_t;

typedef struct {
	int16_t st;
	int16_t end;
} pos_t;

line_coef_ab_t CalcCoef( const coord_t *p0, const coord_t *p1 );
pos_t CalcLineX( const int16_t *y_crt, const coord_t *p_b, const coord_t *p_t, const line_coef_ab_t *coef );
pos_t XMinMax( const pos_t x[], int16_t num );
pos_t YMinMax( const coord_t p[], int16_t num );

void WPLT( int16_t plt_sel, pos_t x, int16_t y );// bias area file

void AREA_WR( int16_t plt_sel, coord_t P1, coord_t P2, coord_t P3, coord_t P4 ) {
	coord_t P[4] = { P1, P2, P3, P4 };
	line_coef_ab_t coef[6];
	pos_t line[6];
	pos_t px, py;
	int16_t y;
	///// bias area file /////
	FILE *fpr, *fpw;
	int16_t fpr_open, fpw_open;
	int16_t ferr;
	char *fn_r;
	char fn_tmp[] = "temp.bak";
	char buf[Def_HW_Max+2];
	int16_t len;
	int16_t x;
	int16_t plt_om, plt_hend;
	int16_t x_plt_om;
	//////////////////////////

	///// bias area file /////
	fpr_open = 0;
	fpw_open = 0;
	ferr = 0;

	if ( (fpr = fopen( fn_tmp, "r" )) != NULL ) {
		fclose( fpr );
		remove( fn_tmp );
	}

//	fn_r = FileNameEBAreaBak( PM_BOARD );
	if ( rename( fn_r, fn_tmp ) != 0 ) {
		printf( "\n" );
		printf( "File error : %s, %s\n", fn_r, fn_tmp );
		ferr = 1;
	}

	if ( ferr == 0 ) {
		if ( (fpr = fopen( fn_tmp, "r" )) == NULL ) {
			printf( "\n" );
			printf( "File open error : %s\n", fn_tmp );
			fpr_open = 0;
		}
		else {
			fpr_open = 1;
		}

		if ( (fpw = fopen( fn_r, "w" )) == NULL ) {
			printf( "\n" );
			printf( "File open error : %s\n", fn_r );
			fpw_open = 0;
		}
		else {
			fpw_open = 1;
		}
	}
	//////////////////////////

	coef[0] = CalcCoef( &P[0], &P[1] );
	coef[1] = CalcCoef( &P[0], &P[2] );
	coef[2] = CalcCoef( &P[0], &P[3] );
	coef[3] = CalcCoef( &P[1], &P[2] );
	coef[4] = CalcCoef( &P[1], &P[3] );
	coef[5] = CalcCoef( &P[2], &P[3] );

	py = YMinMax( P, 4 );
	///// bias area file /////
	for ( y = 0; y < py.st; y++ ) {
		if ( fpr_open == 1 ) {
			if ( fgets( buf, sizeof( buf ), fpr ) == NULL ) {
//				printf( "\n" );
//				printf( "Close %s\n", fn_tmp );
				fclose( fpr );
				remove( fn_tmp );
				fpr_open = 0;
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
			}
		}
		else {
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
		}
		len = strlen( buf ) - 1;
		if ( len < 1 ) {
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
			len = strlen( buf ) - 1;
		}

		if ( fpw_open == 1 ) {
			fputs( buf, fpw );
		}
	}
	//////////////////////////

	for ( y = py.st; y <= py.end; y++ ) {
		line[0] = CalcLineX( &y, &P[0], &P[1], &coef[0] );
		line[1] = CalcLineX( &y, &P[0], &P[2], &coef[1] );
		line[2] = CalcLineX( &y, &P[0], &P[3], &coef[2] );
		line[3] = CalcLineX( &y, &P[1], &P[2], &coef[3] );
		line[4] = CalcLineX( &y, &P[1], &P[3], &coef[4] );
		line[5] = CalcLineX( &y, &P[2], &P[3], &coef[5] );
		px = XMinMax( line, 6 );
		if ( (px.st < PS_WP_HW) && (px.end >= 0) && (px.end >= px.st) ) {
			px.st = ( px.st < 0 ) ? 0 : px.st;
			px.end = ( px.end >= PS_WP_HW ) ? PS_WP_HW - 1 : px.end;
			WPLT( plt_sel, px, y );// bias area file

			///// bias area file /////
			if ( fpr_open == 1 ) {
				if ( fgets( buf, sizeof( buf ), fpr ) == NULL ) {
//					printf( "\n" );
//					printf( "Close %s\n", fn_tmp );
					fclose( fpr );
					remove( fn_tmp );
					fpr_open = 0;
					buf[0] = '0';
					buf[1] = '\n';
					buf[2] = '\0';
				}
			}
			else {
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
			}
			len = strlen( buf ) - 1;
			if ( len < 1 ) {
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
				len = strlen( buf ) - 1;
			}

			plt_om = 0;
			x_plt_om = 0;
			plt_hend = buf[len-1];
			for ( x = 0; x < PS_WP_HW; x++ ) {
				if ( (x >= px.st) && (x <= px.end) ) {
					buf[x] = HexToChar( plt_sel & 0x0f );
				}
				else {
					if ( x >= len ) { buf[x] = plt_hend; }
				}

				if ( CharToHex(buf[x]) != plt_om ) {
					plt_om = CharToHex( buf[x] );
					x_plt_om = x;
				}
			}
			buf[x_plt_om+1] = '\n';
			buf[x_plt_om+2] = '\0';

			if ( fpw_open == 1 ) {
				fputs( buf, fpw );
			}
			//////////////////////////
		}
	}
#if 0//HPC add
	C381_Write_Bank_Reg(0, B0_CPUWAD, 0 );// dummy
	C381_Write_Bank_Reg(0, B0_CPUWAD2, 0 );// dummy
	C381_Write_Bank_Reg(0, B0_CPUWAD3, 0 );// dummy
	C381_Write_Bank_Reg(0, B0_CPUWAD4, 0 );// dummy
#endif//HPC add
	///// bias area file /////
	for ( y = (py.end + 1); y < PS_WP_VW; y++ ) {
		if ( fpr_open == 1 ) {
			if ( fgets( buf, sizeof( buf ), fpr ) == NULL ) {
//				printf( "\n" );
//				printf( "Close %s\n", fn_tmp );
				fclose( fpr );
				remove( fn_tmp );
				fpr_open = 0;
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
			}
		}
		else {
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
		}
		len = strlen( buf ) - 1;
		if ( len < 1 ) {
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
			len = strlen( buf ) - 1;
		}

		if ( fpw_open == 1 ) {
			fputs( buf, fpw );
		}
	}

	if ( fpw_open == 1 ) {
		fclose( fpw );
	}

	if ( fpr_open == 1 ) {
		fclose( fpr );
		remove( fn_tmp );
	}
	//////////////////////////
}

void WPLT( int16_t plt_sel, pos_t x, int16_t y ) {// bias area file
#if 0//HPC add
	int32_t ad;
	int16_t bbacthw;
	int16_t fill;
//	int16_t rdt;
	///// bias area file /////
//	int16_t biasdt = (plt_sel<<4) + plt_sel;
	//////////////////////////

	if ( (x.st >= PS_WP_HW) || (x.end < 0) || (x.st > x.end) ) { return; }

	x.st = ( x.st < 0 ) ? 0 : x.st;
	x.end = ( x.end >= PS_WP_HW ) ? PS_WP_HW - 1 : x.end;
	if ( (x.st & 0x01) == 1 ) {
		ad = Def_EBIASSAD + ( x.st >> 1 ) + y * Def_EBIASMWI * 256;
		C381_Write_Bank_Reg(0, B0_CPURAD, (uint8_t)ad);
		C381_Write_Bank_Reg(0, B0_CPURAD2,(uint8_t)( ad>>8));
		C381_Write_Bank_Reg(0, B0_CPURAD3, (uint8_t)(ad>>16));
		C381_Write_Bank_Reg(0, B0_CPURAD4, (uint8_t)(ad>>24));
		C381_Write_Bank_Reg(0, B0_CPUDTCTL, 0x01);

		fill = C381_Read_Bank_Reg(0, B0_CPUDT) & 0x0f;
		fill = fill + ( plt_sel << 4 );
		C381_Write_Bank_Reg(0, B0_CPUWAD, (uint8_t)ad );
		C381_Write_Bank_Reg(0, B0_CPUWAD2,(uint8_t)( ad>>8) );
		C381_Write_Bank_Reg(0, B0_CPUWAD3, (uint8_t)(ad>>16) );
		C381_Write_Bank_Reg(0, B0_CPUWAD4,(uint8_t)( ad>>24 ));
		C381_Write_Bank_Reg(0, B0_CPUDT, fill );
		x.st = x.st + 1;
	}

	if ( (x.st <= x.end) && ((x.end & 0x01) == 0) ) {
		ad = Def_EBIASSAD + ( x.end >> 1 ) + y * Def_EBIASMWI * 256;
		C381_Write_Bank_Reg(0, B0_CPURAD,(uint8_t) ad);
		C381_Write_Bank_Reg(0, B0_CPURAD2,(uint8_t) (ad>>8));
		C381_Write_Bank_Reg(0, B0_CPURAD3,(uint8_t)( ad>>16));
		C381_Write_Bank_Reg(0, B0_CPURAD4,(uint8_t) (ad>>24));
		C381_Write_Bank_Reg(0, B0_CPUDTCTL, 0x01);
		fill = C381_Read_Bank_Reg(0, B0_CPUDT ) & 0xf0;
		fill = fill + plt_sel;
		C381_Write_Bank_Reg(0, B0_CPUWAD,(uint8_t) ad);
		C381_Write_Bank_Reg(0, B0_CPUWAD2,(uint8_t) (ad>>8));
		C381_Write_Bank_Reg(0, B0_CPUWAD3,(uint8_t) (ad>>16));
		C381_Write_Bank_Reg(0, B0_CPUWAD4,(uint8_t) (ad>>24));
		C381_Write_Bank_Reg(0, B0_CPUDT, fill);

		x.end = x.end - 1;
	}

	if ( x.st <= x.end ) {
		ad = Def_EBIASSAD + ( x.st >> 1 ) + y * Def_EBIASMWI * 256;
		bbacthw = (x.end >> 1) - (x.st >> 1);
		fill = (plt_sel<<4) + plt_sel;
		C381_Write_Bank_Reg(0, B0_CPUWAD, (uint8_t)ad);
		C381_Write_Bank_Reg(0, B0_CPUWAD2,(uint8_t) (ad>>8));
		C381_Write_Bank_Reg(0, B0_CPUWAD3,(uint8_t)( ad>>16));
		C381_Write_Bank_Reg(0, B0_CPUWAD4,(uint8_t) (ad>>24));
		if ( bbacthw == 0 ) {
			C381_Write_Bank_Reg(0, B0_CPUDT, fill);
		}
		else {
			C381_Write_Bank_Reg(0, B0_OSDFILL, (uint8_t)fill);
			C381_Write_Bank_Reg(0, B0_OSDFILL2,(uint8_t)( fill>>8));
			C381_Write_Bank_Reg(0, B0_BBACTHW, (uint8_t)bbacthw);
            C381_Write_Bank_Reg(0, B0_BBACTHW2, (uint8_t)(bbacthw>>8));			
			C381_Write_Bank_Reg(0, B0_BBACTVW, 0x00);
			C381_Write_Bank_Reg(0, B0_BBACTVW2, 0x00);
			C381_Write_Bank_Reg(0, B0_OSDCT, 0x01);
			C381_Write_Bank_Reg(0, B0_OSDCT, 0x00);
//			do { rdt = read_C381( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );

		}
	}
#endif//HPC add
//	write_C381( B0_CPUWAD, 0, 0 );//dummy
}


//calculation
line_coef_ab_t CalcCoef( const coord_t *p0, const coord_t *p1 )
{
	line_coef_ab_t ans;

	ans.a = ( (*p0).x == (*p1).x ) ? (float)( (*p0).y - (*p1).y ) * Def_DIV0 : (float)( (*p0).y - (*p1).y ) / (float)( (*p0).x - (*p1).x );
	ans.b = (float)(*p0).y - ans.a * (float)(*p0).x;
	return ans;
}

pos_t CalcLineX( const int16_t *y_crt, const coord_t *p_b, const coord_t *p_t, const line_coef_ab_t *coef )
{
	pos_t ans;
	coord_t y_min, y_max;
	int16_t x_crt, x_nxt;
	int16_t x_center, y_center;
	float d_temp;

	if( (*p_b).y > (*p_t).y ) {
		y_min = (*p_t);
		y_max = (*p_b);
	}
	else {
		y_min = (*p_b);
		y_max = (*p_t);
	}

	if( (*y_crt < y_min.y) || (*y_crt > y_max.y) ) {
		ans.st = -1;
		ans.end = -1;
	}
	else if( (*p_b).y == (*p_t).y ) {
		ans.st  = ( (*p_b).x > (*p_t).x ) ? (*p_t).x : (*p_b).x;
//		ans.end = ( (*p_b).x > (*p_t).x ) ? (*p_b).x + 1 : (*p_t).x + 1;
		ans.end = ( (*p_b).x > (*p_t).x ) ? (*p_b).x : (*p_t).x;
	}
	else if( (*p_b).x == (*p_t).x ) {
		ans.st = (*p_b).x;
//		ans.end = (*p_b).x + 1;
		ans.end = (*p_b).x;
	}
	else {
		d_temp = ( (*coef).a == 0 ) ? Def_DIV0 :( ( (*coef).a < 0 ) ? -1 / (*coef).a : 1 / (*coef).a);
		x_center = (int16_t)( d_temp / 2 );
		y_center = ( (*coef).a < 0 ) ? (int16_t)( -(*coef).a / 2 ) : (int16_t)( (*coef).a / 2 );

		//current
		d_temp = ( (*coef).a == 0 ) ? ( (*y_crt - y_center) - (*coef).b ) * Def_DIV0 : ( (*y_crt - y_center) - (*coef).b ) / (*coef).a;
		x_crt = (int16_t)( d_temp + 0.5f );
		//next
		d_temp = ( (*coef).a == 0 ) ? ( ((*y_crt + 1) - y_center) - (*coef).b ) * Def_DIV0 : ( ((*y_crt + 1) - y_center) - (*coef).b ) / (*coef).a;
		x_nxt = (int16_t)( d_temp + 0.5f );

		if( x_crt < x_nxt ) {
			ans.st = ( *y_crt == y_min.y ) ? y_min.x : x_crt - x_center;
//			ans.end = ( *y_crt == y_max.y ) ? y_max.x + 1 : x_nxt - x_center;
			ans.end = ( *y_crt == y_max.y ) ? y_max.x : x_nxt - x_center;
		}
		else if( x_crt > x_nxt ) {
			ans.st = ( *y_crt == y_max.y ) ? y_max.x : (x_nxt + 1) + x_center;
//			ans.end = ( *y_crt == y_min.y ) ? y_min.x + 1 : (x_crt + 1) + x_center;
			ans.end = ( *y_crt == y_min.y ) ? y_min.x : (x_crt + 1) + x_center;
		}
		else {
			ans.st = x_crt;
//			ans.end = x_crt + 1;
			ans.end = x_crt;
		}
	}

	return ans;
}

pos_t XMinMax( const pos_t x[], int16_t num )
{
	pos_t ans = { PS_WP_HW, -1 };

	while ( num ) {
		num--;
		ans.st = ( (x[num].st != -1) && ( (ans.st == PS_WP_HW) || (ans.st > x[num].st) ) ) ? x[num].st : ans.st;
		ans.end = ( ans.end < x[num].end ) ? x[num].end : ans.end;
	}

	return ans;
}

pos_t YMinMax( const coord_t p[], int16_t num )
{
	pos_t ans = { PS_WP_VW, -1 };

	while ( num ) {
		num--;
		ans.st = ( ans.st > p[num].y ) ? p[num].y : ans.st;
		ans.end = ( ans.end < p[num].y ) ? p[num].y : ans.end;
	}
//	ans.end += 1;

	return ans;
}


// file_io
//char *FileNameEBAreaBak( int16_t num ) {
//	char fn[Def_BUF_MAX];

//	sprintf( fn, "ebarea%d.bak", num );

//	return fn;
//}

int16_t LoadEgbBiasArea( char fn[] ) {
#if 0//HPC add
	FILE *fpr;
	int16_t fpr_open;
	char buf[Def_HW_Max+2];

	int32_t ad;
	int16_t len;
	int16_t x, y, wbyte_rest;
	int16_t dt_old, dt;
//	int16_t rdt;

	if ( (fpr = fopen( fn, "r" )) == NULL ) {
//		printf( "\n" );
//		printf( "File open error : %s\n", fn );
		fpr_open = 0;
	}
	else {
		fpr_open = 1;
	}
	C381_Write_Bank_Reg(0, B0_BBACTVW, 0x00);
	C381_Write_Bank_Reg(0, B0_BBACTVW2, 0x00);


	for ( y = 0; y < PS_WP_VW; y++ ) {
		if ( fpr_open == 1 ) {
			if ( fgets( buf, sizeof( buf ), fpr ) == NULL ) {
//				printf( "\n" );
//				printf( "Close %s\n", fn );
				fclose( fpr );
				fpr_open = 0;
				buf[0] = '0';
				buf[1] = '\n';
				buf[2] = '\0';
			}
		}
		else {
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
		}
		len = strlen( buf ) - 1;
		if ( len < 1 ) {
			buf[0] = '0';
			buf[1] = '\n';
			buf[2] = '\0';
			len = strlen( buf ) - 1;
		}

		dt_old = ( len == 1 ) ? CharToHex( buf[0] ) + ( CharToHex( buf[0] ) << 4 ) : CharToHex( buf[0] ) + ( CharToHex( buf[1] ) << 4 );
		ad = Def_EBIASSAD + y * Def_EBIASMWI * 256;
		wbyte_rest = 1;
		for ( x = 2; x < PS_WP_HW; x = x + 2 ) {
			if ( x < len ) {
				dt = ( len == (x + 1) ) ? CharToHex( buf[x] ) + ( CharToHex( buf[x] ) << 4 ) : CharToHex( buf[x] ) + ( CharToHex( buf[x+1] ) << 4 );
			}
			else {
				dt = CharToHex( buf[len-1] ) + ( CharToHex( buf[len-1] ) << 4 );
			}

			if ( dt != dt_old ) {
				
				C381_Write_Bank_Reg(0, B0_CPUWAD, (uint8_t)ad);
				C381_Write_Bank_Reg(0, B0_CPUWAD2, (uint8_t)(ad>>8));
				C381_Write_Bank_Reg(0, B0_CPUWAD3, (uint8_t)(ad>>16));
				C381_Write_Bank_Reg(0, B0_CPUWAD4, (uint8_t)(ad>>24));
				if ( wbyte_rest == 1 ) {
					C381_Write_Bank_Reg(0, B0_CPUDT, dt_old);
				}
				else {
					C381_Write_Bank_Reg(0, B0_OSDFILL,(uint8_t) dt_old);
					C381_Write_Bank_Reg(0, B0_OSDFILL2, (uint8_t)(dt_old>>8));
					C381_Write_Bank_Reg(0, B0_BBACTHW, (uint8_t)(wbyte_rest - 1));
					C381_Write_Bank_Reg(0, B0_BBACTHW2,(uint8_t)( (wbyte_rest - 1)>>8));
//					write_C381( B0_BBACTVW, 0x00, 0 );
					C381_Write_Bank_Reg(0, B0_OSDCT, 0x01);
//					do { rdt = read_C381( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );
					C381_Write_Bank_Reg(0, B0_OSDCT, 0x00);
				}
				dt_old = dt;
				ad = Def_EBIASSAD + (x >> 1) + y * Def_EBIASMWI * 256;
				wbyte_rest = 1;
			}
			else {
				wbyte_rest = wbyte_rest + 1;
			}
		}
		C381_Write_Bank_Reg(0, B0_CPUWAD,(uint8_t) ad);
		C381_Write_Bank_Reg(0, B0_CPUWAD2,(uint8_t)( ad>>8));
		C381_Write_Bank_Reg(0, B0_CPUWAD3,(uint8_t)( ad>>16));
		C381_Write_Bank_Reg(0, B0_CPUWAD4,(uint8_t)( ad>>24));

		if ( wbyte_rest == 1 ) {
			C381_Write_Bank_Reg(0, B0_CPUDT, dt_old);
		}
		else {
			C381_Write_Bank_Reg(0, B0_OSDFILL, dt_old);
			C381_Write_Bank_Reg(0, B0_BBACTHW,(uint8_t) (wbyte_rest - 1));
			C381_Write_Bank_Reg(0, B0_BBACTHW2, (uint8_t)((wbyte_rest - 1)>>8));
//			write_C381( B0_BBACTVW, 0x00, 0 );
			C381_Write_Bank_Reg(0, B0_OSDCT, 0x01);
//			do { rdt = read_C381( B0_BOSTAT, 0 ) & 0x01; } while ( rdt != 0x00 );
			C381_Write_Bank_Reg(0, B0_OSDCT, 0x00);
		}

//		write_C381( B0_CPUWAD, 0, 0 );//dummy
	}

	if ( fpr_open == 1 ) {
		fclose( fpr );
	}
	return len;
#endif//HPC add
	return 0;
}
