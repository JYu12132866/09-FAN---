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

#include "individual_param.h"
#include "c381.h"

#ifndef INCLUDED_DECLARE_GFUNC_H
#define INCLUDED_DECLARE_GFUNC_H

extern uint8_t autoWarpDemo;

// uniformity
typedef struct {
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
} ucdt_t;

// function definition
//===== key input =====//
int16_t KeyinSplit(void);
int16_t KeyinSplitType(void);
int16_t KeyinSplitImage( int16_t board, int16_t type );
int16_t KeyinMain(void);
void PrintWpErrLog( int16_t err );
void PrintUcErrLog( int16_t err, int16_t color );

//===== Convert =====//
char HexToChar( int16_t val );
int16_t CharToHex( char ch );

//===== initial setting =====//
void Initwp(void);
void InitSys(void);
void InitResolution(void);
void InitParam(void);
void InitWpTable(void);

//===== device select =====//
void DeviceSel(void);
void ChangeBoard(void);
void ChangeBoardWarp(void);
void ChangeBoardEgbGain(void);
void ChangeBoardEgbBias(void);
void ChangeBoardUniformity(void);

//===== Warp Lim =====//
int16_t CheckWpLimitA( int16_t *x, int16_t *y, float *diffy_y_sum, float *divn );
int16_t CheckWpLimitB( float *diffy_y_sum, float *divn );

//===== Splitter =====//
void SetSplitWpParam(void);
void SetSplitWpReg( int16_t split_image, int16_t cur_disp );

//===== Warp Mode =====//
int16_t StateMachine(void);
void WpmodeChange( int16_t wpmd );
void WpmdGridChange( int16_t xdir, int16_t ydir );

void UpdateInput( int16_t cur_disp );

//===== warp func =====//
int16_t MoveCur( float x, float y );
int16_t Warp(void);

void IntWpTable(void);

void WriteWpTable(void);
void SetLut(void);
void InitGrid( int16_t cur_disp );
void ChangeGrid( int16_t cur_disp );

//===== warp calc func =====//
void CalcLineCoef(void);
void CalcMVXCoef( float nx[]);
void CalcMVYCoef( float ny[], int16_t xgrid );
void CalcPjConv(void);
void CalcHLineCoef( int16_t hline );
void CalcVLineCoef( int16_t vline );

//===== edgeblend gain func =====//
void EgbGain(void);

//===== edgeblend bias func =====//
void EgbBiasEn(void);
void SetEgbBiasCursorEn( int16_t en );
void SetEgbBiasCurPos( int16_t selx, int16_t sely, int16_t dx, int16_t dy );
void SetEgbBiasPlt( int16_t plt, int16_t r, int16_t g, int16_t b );
void AREA_WR( int16_t plt_sel, coord_t P1, coord_t P2, coord_t P3, coord_t P4 );
//char *FileNameEBAreaBak( int16_t num );// file_io
int16_t LoadEgbBiasArea( char fn[] );// file_io

//===== gamma func =====//
void EgbGainGammaEn(void);
void CalcEgbGainGammaTable( int16_t color );
void EgbGainGammaTable( int16_t color );
void OutputGammaEn(void);
void CalcOutputGammaTable( int16_t color );
void OutputGammaTable( int16_t color );
void CalcEgbBiasGammaTable( int16_t color, int16_t plt, int16_t allplt );
void EgbBiasGammaTable( int16_t color, int16_t plt, int16_t allplt );

//===== test pattern func =====//
void TestCrossHatch(void);
void TestEGBMarker(void);
void TestOFILL(void);

//===== color convergence func =====//
void SetColorConvergence( int16_t color );
int16_t CheckColorConvergenceLim( int16_t color );

//===== uniformity func =====//
void InitUniformity( int16_t en );
void ResetUniformity( int16_t en );
ucdt_t CalcUniformityDataGINIT( int16_t level, int16_t *ept );
ucdt_t CalcUniformityDataDGDV( int16_t level, int16_t ygrid, int16_t *ept );
ucdt_t CalcUniformityDataDGDH( int16_t level, int16_t xgrid, int16_t *ept );
ucdt_t CalcUniformityDataDGDHDV( int16_t level, int16_t xgrid, int16_t ygrid, int16_t *ept );
void SetupUniformityPos( int16_t en );
void SetUniformityGain( int16_t en, int16_t all, int16_t color, int16_t xgrid, int16_t ygrid, int16_t *ept );
void SetUniformityCursorEn( int16_t en );
void SetUniformityCursorPos( int16_t grid_num_x, int16_t grid_num_y );
void SetUniformityEn( int16_t en, int16_t acen );

//===== file func =====//
void FileSave( int16_t all, int16_t ebias_area );
void FileLoad( int16_t all );

//===== load_setting_list =====//
void load_setting_list( char *fname );
void init_reg_list(void);
void add_reg_list( char *name, int16_t address );
void free_reg_list(void);
void read_declare_reg_h(void);
void set_register( char fname[] );

//===== state machine =====//
///////////////
void stwp(void);
void keystone(void);
void MoveCursor(int16_t xdir, int16_t ydir);
void FlashDMASave(void) ;
void FlashDMALoad(void) ;
void FLASHSectorErase(void) ;
void FlashRegSave(void);
void FlashRegLoad(void);

void MenuStm(void);
void MenuProc(void);
void MenuPrintProc(void);
///////////////
void func000(void);
void func100(void);
void func110(void);
void func130(void);
void func140(void);
void func150(void);
void func160(void);
void func170(void);
void func211(void);
void func300(void);
void func310(void);
void func320(void);
void func330(void);
void func341(void);
void func360(void);
void func370(void);
void func380(void);
void func391(void);
void func392(void);
void func393(void);
void func394(void);
void func397(void);
void func398(void);
void func410(void);
void func421(void);
void func430(void);
void func441(void);
void func451(void);
void func510(void);
void func520(void);
void func530(void);
void func600(void);
void func610(void);
void func620(void);
void func621(void);
void func630(void);
void func640(void);
void func710(void);
void func811(void);
void func821(void);
void func831(void);
void func841(void);
void func851(void);
void func861(void);
///////////////
void print_func000(void);
void print_func100(void);
void print_func200(void);
void print_func210(void);
void print_func220(void);
void print_func230(void);
void print_func240(void);
void print_func241(void);
void print_func242(void);
void print_func243(void);
void print_func244(void);
void print_func300(void);
void print_func340(void);
void print_func390(void);
void print_func395(void);
void print_func396(void);
void print_func400(void);
void print_func410(void);
void print_func420(void);
void print_func430(void);
void print_func440(void);
void print_func450(void);
void print_func500(void);
void print_func510(void);
void print_func520(void);
void print_func530(void);
void print_func600(void);
void print_func620(void);
void print_func700(void);
void print_func800(void);
void print_func810(void);
void print_func820(void);

//===== wait =====//
int16_t WaitPivs( void);
int16_t WaitPovs( void);

//===== other functions =====//
void SetOFILL( int16_t en, int32_t color );

//===== register access =====//
//void write_C381( int16_t ad,  int16_t dt, int16_t bk_offset );
void writeb_C381( int16_t ad,  int16_t dt, int16_t bk_offset );
void writef_C381( int16_t ad,  int16_t dt, int16_t bk_offset );
//int16_t  read_C381( int16_t ad, int16_t bk_offset );

#endif
