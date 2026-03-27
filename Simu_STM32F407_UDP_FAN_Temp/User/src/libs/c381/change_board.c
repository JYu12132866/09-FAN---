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



int16_t IGV_PREV_SEL_GRID_X[4];
int16_t IGV_PREV_SEL_GRID_Y[4];
int16_t IPI_ACT_HST[4];
int16_t IPI_ACT_HW[4];
int16_t IPI_ACT_VST[4];
int16_t IPI_ACT_VW[4];
int16_t IPM_IACT_HW[4];
int16_t IPM_IACT_VW[4];
int16_t IPS_CUR_DEF_IX[4][9];
int16_t IPS_CUR_DEF_IY[4][9];

int16_t IPS_WP_BLACTHST[4];
int16_t IPS_WP_BLACTVST[4];

int16_t IGV_RTCT[4];
int16_t IGV_EGBCT[4];
int16_t GV_EGBCT;
int16_t IPM_SEL_GRID_X[4];
int16_t PM_SEL_GRID_X;
int16_t IPM_SEL_GRID_Y[4];
int16_t GV_PREV_SEL_GRID_X;
int16_t GV_PREV_SEL_GRID_Y;
coord_f_t IPM_GRID[4][9][9];
int16_t IPM_WP_MODE[4];
int16_t PM_WP_MODE;
int16_t IPM_CUR_INTERNAL_ON[4];
int16_t PM_CUR_INTERNAL_ON;
int16_t IGV_WP_TABLE_COLOR[4];
int16_t GV_WP_TABLE_COLOR;
int16_t IPM_OFILL_EN[4];
int16_t PM_OFILL_EN;
int16_t PS_WP_BLACTHST;
int16_t PS_WP_BLACTVST;
coord_t IPM_WP_CCVG[4][3];
coord_t PM_WP_CCVG[3];
int16_t IPS_SPLIT_IMAGE[4];
int16_t PS_SPLIT_IMAGE;
int16_t GV_RTCT;
int16_t PI_ACT_HST;
int16_t PI_ACT_HW;
int16_t PI_ACT_VST;
int16_t PI_ACT_VW;
int16_t PM_IACT_HW;
int16_t PM_IACT_VW;
int16_t PM_SEL_GRID_Y;
int16_t PS_CUR_DEF_IX[9];
int16_t PS_CUR_DEF_IY[9];
coord_f_t PM_GRID[9][9];
int16_t IPM_EGB_LEN[4];
int16_t IPM_EGB_REN[4];
int16_t IPM_EGB_TEN[4];
int16_t IPM_EGB_BEN[4];
int16_t PM_EGB_LEN;
int16_t PM_EGB_REN;
int16_t PM_EGB_TEN;
int16_t PM_EGB_BEN;
coord_t IPM_EBIAS_CUR[4][2][2];
coord_t PM_EBIAS_CUR[2][2];
int16_t IPM_EBIAS[4][16][3];
int16_t PM_EBIAS[16][3];
int16_t IPM_UC_EN[4];
int16_t PM_UC_EN;
float IPM_UC_GAIN[4][3][3][11][9];
float PM_UC_GAIN[3][3][11][9];

void ChangeBoard(void)
{
  ChangeBoardWarp();
  ChangeBoardEgbGain();
  ChangeBoardEgbBias();
  ChangeBoardUniformity();


  // PM -> IPM
  IGV_RTCT[PM_BOARD_OLD] = GV_RTCT;
  IGV_EGBCT[PM_BOARD_OLD] = GV_EGBCT;
  // IPM -> PM
  GV_RTCT = IGV_RTCT[PM_BOARD];
  GV_EGBCT = IGV_EGBCT[PM_BOARD];

  // handle set
  PM_BOARD_OLD = PM_BOARD;
}

void ChangeBoardWarp(void)
{
  int16_t i, j;

  // PM -> IPM
  // warping
  IPM_SEL_GRID_X[PM_BOARD_OLD] = PM_SEL_GRID_X;
  IPM_SEL_GRID_Y[PM_BOARD_OLD] = PM_SEL_GRID_Y;
  IGV_PREV_SEL_GRID_X[PM_BOARD_OLD] = GV_PREV_SEL_GRID_X;
  IGV_PREV_SEL_GRID_Y[PM_BOARD_OLD] = GV_PREV_SEL_GRID_Y;
  for ( j = 0; j < 9; j++ ) {
    IPS_CUR_DEF_IX[PM_BOARD_OLD][j] = PS_CUR_DEF_IX[j];
    IPS_CUR_DEF_IY[PM_BOARD_OLD][j] = PS_CUR_DEF_IY[j];
    for ( i = 0; i < 9; i++ ) {
      IPM_GRID[PM_BOARD_OLD][i][j] = PM_GRID[i][j];
    }
  }
  IPM_WP_MODE[PM_BOARD_OLD] = PM_WP_MODE;
  IPM_CUR_INTERNAL_ON[PM_BOARD_OLD] = PM_CUR_INTERNAL_ON;

  IGV_WP_TABLE_COLOR[PM_BOARD_OLD] = GV_WP_TABLE_COLOR;

  // test pattern
  IPM_OFILL_EN[PM_BOARD_OLD] = PM_OFILL_EN;

  // OSD
  IPS_WP_BLACTHST[PM_BOARD_OLD] = PS_WP_BLACTHST;
  IPS_WP_BLACTVST[PM_BOARD_OLD] = PS_WP_BLACTVST;

  // color convergence
  IPM_WP_CCVG[PM_BOARD_OLD][0] = PM_WP_CCVG[0];
  IPM_WP_CCVG[PM_BOARD_OLD][1] = PM_WP_CCVG[1];
  IPM_WP_CCVG[PM_BOARD_OLD][2] = PM_WP_CCVG[2];

  // split image
  IPS_SPLIT_IMAGE[PM_BOARD_OLD] = PS_SPLIT_IMAGE;
  IPI_ACT_HST[PM_BOARD_OLD] = PI_ACT_HST;
  IPI_ACT_HW[PM_BOARD_OLD] = PI_ACT_HW;
  IPI_ACT_VST[PM_BOARD_OLD] = PI_ACT_VST;
  IPI_ACT_VW[PM_BOARD_OLD] = PI_ACT_VW;
  IPM_IACT_HW[PM_BOARD_OLD] = PM_IACT_HW;
  IPM_IACT_VW[PM_BOARD_OLD] = PM_IACT_VW;


  // IPM -> PM
  // warping
  PM_SEL_GRID_X= IPM_SEL_GRID_X[PM_BOARD];
  PM_SEL_GRID_Y= IPM_SEL_GRID_Y[PM_BOARD];
  GV_PREV_SEL_GRID_X= IGV_PREV_SEL_GRID_X[PM_BOARD_OLD];
  GV_PREV_SEL_GRID_Y = IGV_PREV_SEL_GRID_Y[PM_BOARD_OLD];
  for ( j = 0; j < 9; j++ ) {
    PS_CUR_DEF_IX[j] = IPS_CUR_DEF_IX[PM_BOARD][j];
    PS_CUR_DEF_IY[j] = IPS_CUR_DEF_IY[PM_BOARD][j];
    for ( i = 0; i < 9; i++ ) {
      PM_GRID[i][j] = IPM_GRID[PM_BOARD][i][j];
    }
  }
  PM_WP_MODE = IPM_WP_MODE[PM_BOARD];
  PM_CUR_INTERNAL_ON = IPM_CUR_INTERNAL_ON[PM_BOARD];

  GV_WP_TABLE_COLOR = IGV_WP_TABLE_COLOR[PM_BOARD];

  // test pattern
  PM_OFILL_EN = IPM_OFILL_EN[PM_BOARD];

  // OSD
  PS_WP_BLACTHST = IPS_WP_BLACTHST[PM_BOARD];
  PS_WP_BLACTVST = IPS_WP_BLACTVST[PM_BOARD];

  // color convergence
  PM_WP_CCVG[0] = IPM_WP_CCVG[PM_BOARD][0];
  PM_WP_CCVG[1] = IPM_WP_CCVG[PM_BOARD][1];
  PM_WP_CCVG[2] = IPM_WP_CCVG[PM_BOARD][2];

  // split image
  PS_SPLIT_IMAGE = IPS_SPLIT_IMAGE[PM_BOARD];
  PI_ACT_HST = IPI_ACT_HST[PM_BOARD];
  PI_ACT_HW = IPI_ACT_HW[PM_BOARD];
  PI_ACT_VST = IPI_ACT_VST[PM_BOARD];
  PI_ACT_VW = IPI_ACT_VW[PM_BOARD];
  PM_IACT_HW = IPM_IACT_HW[PM_BOARD];
  PM_IACT_VW = IPM_IACT_VW[PM_BOARD];
}

void ChangeBoardEgbGain(void)
{
  // PM -> IPM
  IPM_EGB_LEN[PM_BOARD_OLD] = PM_EGB_LEN;
  IPM_EGB_REN[PM_BOARD_OLD] = PM_EGB_REN;
  IPM_EGB_TEN[PM_BOARD_OLD] = PM_EGB_TEN;
  IPM_EGB_BEN[PM_BOARD_OLD] = PM_EGB_BEN;

  // IPM -> PM
  PM_EGB_LEN = IPM_EGB_LEN[PM_BOARD];
  PM_EGB_REN = IPM_EGB_REN[PM_BOARD];
  PM_EGB_TEN = IPM_EGB_TEN[PM_BOARD];
  PM_EGB_BEN = IPM_EGB_BEN[PM_BOARD];
}

void ChangeBoardEgbBias(void)
{
  int16_t i, j;
  // PM -> IPM
  for ( j = 0; j < 2; j++ ) {
    for ( i = 0; i < 2; i++ ) {
      IPM_EBIAS_CUR[PM_BOARD_OLD][i][j] = PM_EBIAS_CUR[i][j];
    }
  }

  for ( j = 0; j < 3; j++ ) {
    for ( i = 0; i < 16; i++ ) {
      IPM_EBIAS[PM_BOARD_OLD][i][j] = PM_EBIAS[i][j];
    }
  }

  // IPM -> PM
  for ( j = 0; j < 2; j++ ) {
    for ( i = 0; i < 2; i++ ) {
      PM_EBIAS_CUR[i][j] = IPM_EBIAS_CUR[PM_BOARD][i][j];
    }
  }

  for ( j = 0; j < 3; j++ ) {
    for ( i = 0; i < 16; i++ ) {
      PM_EBIAS[i][j] = IPM_EBIAS[PM_BOARD][i][j];
    }
  }
}

void ChangeBoardUniformity(void)
{
  int16_t i, j, k, l;

  // PM -> IPM
  IPM_UC_EN[PM_BOARD_OLD] = PM_UC_EN;
  for ( l = 0; l < 3; l++ ) {
    for ( k = 0; k < 3; k++ ) {
      for ( j = 0; j < 9; j++ ) {
        for ( i = 0; i < 11; i++ ) {
          IPM_UC_GAIN[PM_BOARD_OLD][l][k][i][j] = PM_UC_GAIN[l][k][i][j];
        }
      }
    }
  }

  // IPM -> PM
  PM_UC_EN = IPM_UC_EN[PM_BOARD];
  for ( l = 0; l < 3; l++ ) {
    for ( k = 0; k < 3; k++ ) {
      for ( j = 0; j < 9; j++ ) {
        for ( i = 0; i < 11; i++ ) {
          PM_UC_GAIN[l][k][i][j] = IPM_UC_GAIN[PM_BOARD][l][k][i][j];
        }
      }
    }
  }
}
