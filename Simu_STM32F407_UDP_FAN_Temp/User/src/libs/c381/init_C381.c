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
#include "main.h"
#include "set.h"

int16_t GV_CUR_INTERNAL_ON_WP;
int16_t GV_EGB_OHW_OLD;
int16_t GV_EGB_OVW_OLD;

int16_t GV_UC_CUR_EN[4];
int16_t GV_WP_MODE_WP;


int16_t PB_000;
int16_t PB_100_MV;
int16_t PB_200;
int16_t PB_210;
int16_t PB_220;
int16_t PB_240;
int16_t PB_340;
int16_t PB_390;
int16_t PB_400;
float PB_420;
float PB_440;
float PB_450;
int16_t PB_500;
int16_t PB_800;
int16_t g_current_bank;
int16_t st_num;
int16_t FLG_AUTO_RETURN;
int32_t st_num_old;


int16_t PM_BOARD;
int16_t PM_BOARD_OLD;

int16_t PM_CUR_ON;
int16_t PM_EBIAS_COLOR;
int16_t PM_EBIAS_CUR_MV;
int16_t PM_EBIAS_EN;
int16_t PM_EBIAS_GAMMA_COLOR;
int16_t PM_EBIAS_PLT;
int16_t PM_EBIAS_SEL_CUR_X;
int16_t PM_EBIAS_SEL_CUR_Y;
int16_t PM_EGBMK_EN;
int16_t PM_EGB_EN;
int16_t PM_EGB_GAMMA_COLOR;

int16_t PM_EGB_OHW;
int16_t PM_EGB_OVW;
int16_t PM_FILE_NUM;
float PM_GAMMA[Def_OGM_NUM][3];
int16_t PM_GAMMA_COLOR;
int16_t PM_GAMMA_EN;
int16_t PM_GAMMA_PIX_SEL;
int16_t PM_GAMMA_PIX[Def_OGM_NUM];
coord_f_t PM_GRID_OLD[9][9];

int16_t PM_PATGEN;

int16_t PM_UC_COLOR;
int16_t PM_UC_LEVEL;
int16_t PM_UC_SEL_GRID_X;
int16_t PM_UC_SEL_GRID_Y;
int16_t PM_WP_CCVG_COLOR;
int16_t PS_CUR_DEF_OX[9];
int16_t PS_CUR_DEF_OY[9];
int16_t PS_UC_HW;
int16_t PS_UC_HW_EDGRD;
int16_t PS_UC_HW_GRD_NUM;
int16_t PS_UC_HW_GRD_MAX;
int16_t PS_UC_VW;
int16_t PS_UC_VW_GRD_NUM;
int16_t PS_UC_VW_EDGRD;
int16_t PS_UC_VW_GRD_MAX;
float PS_WP_LIMH_ITV;
float PS_WP_LIMV_ITV;
int16_t PS_WP_HW;
int16_t PS_WP_VW;
int16_t PM_EBIAS_AREA_TEST;
int16_t PM_EGB_GAMMA_EN;
float PM_EGB_GAMMA[3];
float PM_EBIAS_GAMMA[3];
float PM_GRID_MV;

int16_t PS_WP_HW_GRD;
int16_t PS_WP_VW_GRD;
int    PHWNZ;
int    PVWNZ;
float    PHWHF;
float    PVWHF;
// extern str_wppos4c PM_WPPOS4C;
extern int    PM_WPGDSELX;
extern int    PM_WPGDSELY;


void ScladpNormal(int ch);

void InitResolution(void)
{
#if 0//HPC add
  int16_t osyct1,osyct2;
  int16_t aoist,aoist1,aoiend,aoiend1;
  int16_t temp;

  GV_RTCT = 0x0000;
  C381_WriteReg(B0_RTCT0,(uint8_t)GV_RTCT);
  //    HAL_Delay(100);
  C381_WriteReg(B0_RTCT6,(uint8_t)(GV_RTCT>>8));

  C381_Write_Bank_Reg(8, B16_PIH0CH1, 0x00 );// PIACTSTMD enable
  C381_Write_Bank_Reg(8, B32_PIH0CH2, 0x40 );// PIACTSTMD enable

  PI_ACT_HST = C381_Read_Bank_Reg( 8,B32_IACTHSTCH2);
  PI_ACT_HST = (PI_ACT_HST<<8)| C381_Read_Bank_Reg( 8,B16_IACTHSTCH1);

  PI_ACT_HW = C381_Read_Bank_Reg( 8,B32_IACTHWCH2);
  PI_ACT_HW = (PI_ACT_HW<<8)| C381_Read_Bank_Reg( 8,B16_IACTHWCH1);

  PI_ACT_VST = C381_Read_Bank_Reg( 8,B32_IACTVSTCH2);
  PI_ACT_VST = (PI_ACT_VST<<8)| C381_Read_Bank_Reg( 8,B16_IACTVSTCH1);

  PI_ACT_VW = C381_Read_Bank_Reg( 8,B32_IACTVWCH2);
  PI_ACT_VW = (PI_ACT_VW<<8)| C381_Read_Bank_Reg( 8,B16_IACTVWCH1);

  C381_Write_Bank_Reg( 8,B16_PIH0CH1, 0x00 );// PIACTSTMD enable
  C381_Write_Bank_Reg( 8,B32_PIH0CH2, 0x00 );// PIACTSTMD enable

  osyct2 = C381_Read_Bank_Reg(2, B149_OSYCTCH2 ) & 0xff;
  osyct2=osyct2<<8;

  osyct1 = C381_Read_Bank_Reg(2, B144_OSYCTCH1 ) & 0x8f;
  osyct1=osyct2|osyct1;
  C381_Write_Bank_Reg(2, B144_OSYCTCH1, (uint8_t)(osyct1 | 0x0010) );// POACT = OAOI0
  C381_Write_Bank_Reg(2, B149_OSYCTCH2, (uint8_t)((osyct1 | 0x0010)>>8) );// POACT = OAOI0

  aoist1  =C381_Read_Bank_Reg(2,B149_OAOI0HSTCH2)& 0x1f;
  aoist=C381_Read_Bank_Reg(2,B144_OAOI0HSTCH1)& 0xff;
  aoist=aoist|(aoist1<<8);

  aoiend1  =C381_Read_Bank_Reg(2,B149_OAOI0HENDCH2)& 0x1f;;
  aoiend=C381_Read_Bank_Reg(2,B144_OAOI0HENDCH1)& 0xff;
  aoiend=aoiend|(aoiend1<<8);
  temp = aoiend - aoist;
  PS_WP_HW = ( temp < 0 ) ? 0 :( ( temp > Def_HW_Max ) ? Def_HW_Max : temp);

  aoist1  =C381_Read_Bank_Reg(2,B149_OAOI0VSTCH2)& 0x1f;
  aoist=C381_Read_Bank_Reg(2,B144_OAOI0VSTCH1)& 0xff;
  aoist=aoist|(aoist1<<8);

  aoiend1=C381_Read_Bank_Reg(2,B149_OAOI0VENDCH2)& 0x1f;
  aoiend  =C381_Read_Bank_Reg(2,B144_OAOI0VENDCH1)& 0xff;
  aoiend=aoiend|(aoiend1<<8);

  temp = aoiend - aoist;
  PS_WP_VW = ( temp < 0 ) ? 0 : (( temp > Def_VW_Max ) ? Def_VW_Max : temp);

  PS_UC_HW_GRD_MAX = ( PS_WP_HW < 1 ) ? 1 : ( PS_WP_HW + (Def_UC_HW_GRD - 1) ) / Def_UC_HW_GRD;
  PS_UC_HW_GRD_NUM = PS_UC_HW_GRD_MAX + 1;// = roundup( PS_WP_HW / Def_UC_HW_GRD ) + 1
  PS_UC_HW = PS_WP_HW / PS_UC_HW_GRD_MAX;
  PS_UC_HW_EDGRD = PS_WP_HW - PS_UC_HW * (PS_UC_HW_GRD_MAX - 1);

  PS_UC_VW_GRD_MAX = ( PS_WP_VW < 1 ) ? 1 : ( PS_WP_VW + (Def_UC_VW_GRD - 1) ) / Def_UC_VW_GRD;
  PS_UC_VW_GRD_NUM = PS_UC_VW_GRD_MAX + 1;// = roundup( PS_WP_VW / Def_UC_VW_GRD ) + 1
  PS_UC_VW = PS_WP_VW / PS_UC_VW_GRD_MAX;
  PS_UC_VW_EDGRD = PS_WP_VW - PS_UC_VW * (PS_UC_VW_GRD_MAX - 1);

  PS_WP_HW_GRD = ( PS_WP_HW + Def_Wp_Space - 1 ) / Def_Wp_Space;
  PS_WP_VW_GRD = ( PS_WP_VW + Def_Wp_Space - 1 ) / Def_Wp_Space;

  PS_WP_LIMH_ITV = Def_WPLIMITV_H;
  PS_WP_LIMV_ITV = Def_WPLIMITV_V;
#endif//HPC add
}

#if 0//HPC add
void InitSys(void)
{
  g_current_bank = -1;

  st_num = 000;
  FLG_AUTO_RETURN = 0;
  st_num_old = 65535;

  PB_000 = Def_PB_000_INIT;
  PB_100_MV = Def_PB_100_MV_INIT;
  PB_200 = Def_PB_200_INIT;
  PB_210 = Def_PB_210_INIT;
  PB_220 = Def_PB_220_INIT;
  PB_240 = Def_PB_240_INIT;
  PB_340 = Def_PB_340_INIT;
  PB_390 = Def_PB_390_INIT;
  PB_420 = Def_PB_420_INIT;
  PB_440 = Def_PB_440_INIT;
  PB_450 = Def_PB_450_INIT;
  PB_500 = Def_PB_500_INIT;
  PB_800 = Def_PB_800_INIT;
  PM_BOARD = 0;
  PM_BOARD_OLD = PM_BOARD;

  ChangeBoard();
}
#endif

/**
 * @brief 初始化c381光标显示
 *
 */
void load_setupWP(void)
{
  int acthst, actvst, acthw;
  int16_t x, y;
  // OSD setting
  write_C381(B4_OSDCT, 0x00);

  acthst = (read_C381(B16_IACTHSTCH1) & 0x3fff) + 6;
  actvst = (read_C381(B16_IACTVSTCH1) & 0x3fff);
  acthw = (PS_PROCMODE == 1) ? (PS_PANEL_ACT_HW / 4) : (PS_PROCMODE == 2) ? PS_PANEL_ACT_HW
                                                                          : (PS_PANEL_ACT_HW / 2);
  write_C381(B9_OSDMODE1CH1, 0x0B);
  write_C381(B9_OSDACTHST1CH1, (acthst & 0x3fff));
  write_C381(B9_OSDACTHW1CH1, (acthw & 0x1fff));
  write_C381(B9_OSDACTVST1CH1, (actvst & 0x3fff));
  write_C381(B9_OSDACTVW1CH1, (PS_PANEL_ACT_VW & 0x1fff));
  write_C381(B9_OSDSAD1CH1, (PS_OSDSAD0 & 0x7fffffff));
  write_C381(B9_OSDMWI1CH1, (PS_OSDMWI & 0xff));
  write_C381(B9_BOTRANS01CH1, 0x00); // pallet 0 OSD 0%(trans)

  acthst = (read_C381(B32_IACTHSTCH2) & 0x3fff) + 6;
  actvst = (read_C381(B32_IACTVSTCH2) & 0x3fff);
  write_C381(B25_OSDMODE1CH2, 0x0B);
  write_C381(B25_OSDACTHST1CH2, (acthst & 0x3fff));
  write_C381(B25_OSDACTHW1CH2, (acthw & 0x1fff));
  write_C381(B25_OSDACTVST1CH2, (actvst & 0x3fff));
  write_C381(B25_OSDACTVW1CH2, (PS_PANEL_ACT_VW & 0x1fff));
  write_C381(B25_OSDSAD1CH2, ((PS_OSDSAD0 + acthw) & 0x7fffffff));
  write_C381(B25_OSDMWI1CH2, (PS_OSDMWI & 0xff));
  write_C381(B25_BOTRANS01CH2, 0x00); // pallet 0 OSD 0%(trans)

  acthst = (read_C381(B48_IACTHSTCH3) & 0x3fff) + 6;
  actvst = (read_C381(B48_IACTVSTCH3) & 0x3fff);
  write_C381(B41_OSDMODE1CH3, 0x0B);
  write_C381(B41_OSDACTHST1CH3, (acthst & 0x3fff));
  write_C381(B41_OSDACTHW1CH3, (acthw & 0x1fff));
  write_C381(B41_OSDACTVST1CH3, (actvst & 0x3fff));
  write_C381(B41_OSDACTVW1CH3, (PS_PANEL_ACT_VW & 0x1fff));
  write_C381(B41_OSDSAD1CH3, ((PS_OSDSAD0 + acthw * 2) & 0x7fffffff));
  write_C381(B41_OSDMWI1CH3, (PS_OSDMWI & 0xff));
  write_C381(B41_BOTRANS01CH3, 0x00); // pallet 0 OSD 0%(trans)

  acthst = (read_C381(B64_IACTHSTCH4) & 0x3fff) + 6;
  actvst = (read_C381(B64_IACTVSTCH4) & 0x3fff);
  write_C381(B57_OSDMODE1CH4, 0x0B);
  write_C381(B57_OSDACTHST1CH4, (acthst & 0x3fff));
  write_C381(B57_OSDACTHW1CH4, (acthw & 0x1fff));
  write_C381(B57_OSDACTVST1CH4, (actvst & 0x3fff));
  write_C381(B57_OSDACTVW1CH4, (PS_PANEL_ACT_VW & 0x1fff));
  write_C381(B57_OSDSAD1CH4, ((PS_OSDSAD0 + acthw * 3) & 0x7fffffff));
  write_C381(B57_OSDMWI1CH4, (PS_OSDMWI & 0xff));
  write_C381(B57_BOTRANS01CH4, 0x00); // pallet 0 OSD 0%(trans)

  // Cursor setting
  // cursor CHI
  write_C381(B147_CURCTCH1, 0x0040);
  write_C381(B147_CURCPUADCH1, 0x00);
  write_C381(B147_CURCPUADCH1, 0x00);
  for (y = 0; y < 64; y++)
  {
    for (x = 0; x < 32; x++)
    {
      if (y < 32)
      {
        if (x < 16)
        {
          write_C381(B147_CURCPUDTCH1, 0x00);
        }
        else
        {
          write_C381(B147_CURCPUDTCH1, 0x11);
        }
      }
      else
      {
        if (x < 16)
        {
          write_C381(B147_CURCPUDTCH1, 0x22);
        }
        else
        {
          write_C381(B147_CURCPUDTCH1, 0x33);
        }
      }
    }
  }
  write_C381(B147_CURPLTDTR0CH1, 0xff);
  write_C381(B147_CURPLTDTR1CH1, 0xff);
  write_C381(B147_CURPLTDTR2CH1, 0xff);
  write_C381(B147_CURPLTDTR3CH1, 0xff);
  write_C381(B147_CURPLTDTR4CH1, 0xff);
  write_C381(B147_CURPLTDTR5CH1, 0xff);
  write_C381(B147_CURPLTDTR6CH1, 0xff);
  write_C381(B147_CURPLTDTR7CH1, 0xff);
  write_C381(B147_CURPLTDTR8CH1, 0xff);
  write_C381(B147_CURPLTDTR9CH1, 0xff);
  write_C381(B147_CURPLTDTR10CH1, 0xff);
  write_C381(B147_CURPLTDTR11CH1, 0xff);
  write_C381(B147_CURPLTDTR12CH1, 0xff);
  write_C381(B147_CURPLTDTR13CH1, 0xff);
  write_C381(B147_CURPLTDTR14CH1, 0xff);
  write_C381(B147_CURPLTDTR15CH1, 0xff);

  write_C381(B147_CURPLTDTG0CH1, 0xff);
  write_C381(B147_CURPLTDTG1CH1, 0xff);
  write_C381(B147_CURPLTDTG2CH1, 0xff);
  write_C381(B147_CURPLTDTG3CH1, 0xff);
  write_C381(B147_CURPLTDTG4CH1, 0xff);
  write_C381(B147_CURPLTDTG5CH1, 0xff);
  write_C381(B147_CURPLTDTG6CH1, 0xff);
  write_C381(B147_CURPLTDTG7CH1, 0xff);
  write_C381(B147_CURPLTDTG8CH1, 0xff);
  write_C381(B147_CURPLTDTG9CH1, 0xff);
  write_C381(B147_CURPLTDTG10CH1, 0xff);
  write_C381(B147_CURPLTDTG11CH1, 0xff);
  write_C381(B147_CURPLTDTG12CH1, 0xff);
  write_C381(B147_CURPLTDTG13CH1, 0xff);
  write_C381(B147_CURPLTDTG14CH1, 0xff);
  write_C381(B147_CURPLTDTG15CH1, 0xff);

  write_C381(B147_CURPLTDTB0CH1, 0xff);
  write_C381(B147_CURPLTDTB1CH1, 0xff);
  write_C381(B147_CURPLTDTB2CH1, 0xff);
  write_C381(B147_CURPLTDTB3CH1, 0xff);
  write_C381(B147_CURPLTDTB4CH1, 0xff);
  write_C381(B147_CURPLTDTB5CH1, 0xff);
  write_C381(B147_CURPLTDTB6CH1, 0xff);
  write_C381(B147_CURPLTDTB7CH1, 0xff);
  write_C381(B147_CURPLTDTB8CH1, 0xff);
  write_C381(B147_CURPLTDTB9CH1, 0xff);
  write_C381(B147_CURPLTDTB10CH1, 0xff);
  write_C381(B147_CURPLTDTB11CH1, 0xff);
  write_C381(B147_CURPLTDTB12CH1, 0xff);
  write_C381(B147_CURPLTDTB13CH1, 0xff);
  write_C381(B147_CURPLTDTB14CH1, 0xff);
  write_C381(B147_CURPLTDTB15CH1, 0xff);

  write_C381(B147_CURHWCH1, Def_4CURSOR_Size);
  write_C381(B147_CURVWCH1, Def_4CURSOR_Size);
  write_C381(B147_CURTLXCH1, 0x0fff);
  write_C381(B147_CURTLYCH1, 0x0fff);
  write_C381(B147_CURTRXCH1, 0x0fff);
  write_C381(B147_CURTRYCH1, 0x0fff);
  write_C381(B147_CURBLXCH1, 0x0fff);
  write_C381(B147_CURBLYCH1, 0x0fff);
  write_C381(B147_CURBRXCH1, 0x0fff);
  write_C381(B147_CURBRYCH1, 0x0fff);
  write_C381(B147_CURCTCH1, 0x0010);

  // cursor CH2
  y = 0;
  x = 0;
  write_C381(B152_CURCTCH2, 0x0040);
  write_C381(B152_CURCPUADCH2, 0x00);
  write_C381(B152_CURCPUADCH2, 0x00);
  for (y = 0; y < 64; y++)
  {
    for (x = 0; x < 32; x++)
    {
      if (y < 32)
      {
        if (x < 16)
        {
          write_C381(B152_CURCPUDTCH2, 0x00);
        }
        else
        {
          write_C381(B152_CURCPUDTCH2, 0x11);
        }
      }
      else
      {
        if (x < 16)
        {
          write_C381(B152_CURCPUDTCH2, 0x22);
        }
        else
        {
          write_C381(B152_CURCPUDTCH2, 0x33);
        }
      }
    }
  }
  write_C381(B152_CURPLTDTR0CH2, 0xff);
  write_C381(B152_CURPLTDTR1CH2, 0xff);
  write_C381(B152_CURPLTDTR2CH2, 0xff);
  write_C381(B152_CURPLTDTR3CH2, 0xff);
  write_C381(B152_CURPLTDTR4CH2, 0xff);
  write_C381(B152_CURPLTDTR5CH2, 0xff);
  write_C381(B152_CURPLTDTR6CH2, 0xff);
  write_C381(B152_CURPLTDTR7CH2, 0xff);
  write_C381(B152_CURPLTDTR8CH2, 0xff);
  write_C381(B152_CURPLTDTR9CH2, 0xff);
  write_C381(B152_CURPLTDTR10CH2, 0xff);
  write_C381(B152_CURPLTDTR11CH2, 0xff);
  write_C381(B152_CURPLTDTR12CH2, 0xff);
  write_C381(B152_CURPLTDTR13CH2, 0xff);
  write_C381(B152_CURPLTDTR14CH2, 0xff);
  write_C381(B152_CURPLTDTR15CH2, 0xff);

  write_C381(B152_CURPLTDTG0CH2, 0xff);
  write_C381(B152_CURPLTDTG1CH2, 0xff);
  write_C381(B152_CURPLTDTG2CH2, 0xff);
  write_C381(B152_CURPLTDTG3CH2, 0xff);
  write_C381(B152_CURPLTDTG4CH2, 0xff);
  write_C381(B152_CURPLTDTG5CH2, 0xff);
  write_C381(B152_CURPLTDTG6CH2, 0xff);
  write_C381(B152_CURPLTDTG7CH2, 0xff);
  write_C381(B152_CURPLTDTG8CH2, 0xff);
  write_C381(B152_CURPLTDTG9CH2, 0xff);
  write_C381(B152_CURPLTDTG10CH2, 0xff);
  write_C381(B152_CURPLTDTG11CH2, 0xff);
  write_C381(B152_CURPLTDTG12CH2, 0xff);
  write_C381(B152_CURPLTDTG13CH2, 0xff);
  write_C381(B152_CURPLTDTG14CH2, 0xff);
  write_C381(B152_CURPLTDTG15CH2, 0xff);

  write_C381(B152_CURPLTDTB0CH2, 0xff);
  write_C381(B152_CURPLTDTB1CH2, 0xff);
  write_C381(B152_CURPLTDTB2CH2, 0xff);
  write_C381(B152_CURPLTDTB3CH2, 0xff);
  write_C381(B152_CURPLTDTB4CH2, 0xff);
  write_C381(B152_CURPLTDTB5CH2, 0xff);
  write_C381(B152_CURPLTDTB6CH2, 0xff);
  write_C381(B152_CURPLTDTB7CH2, 0xff);
  write_C381(B152_CURPLTDTB8CH2, 0xff);
  write_C381(B152_CURPLTDTB9CH2, 0xff);
  write_C381(B152_CURPLTDTB10CH2, 0xff);
  write_C381(B152_CURPLTDTB11CH2, 0xff);
  write_C381(B152_CURPLTDTB12CH2, 0xff);
  write_C381(B152_CURPLTDTB13CH2, 0xff);
  write_C381(B152_CURPLTDTB14CH2, 0xff);
  write_C381(B152_CURPLTDTB15CH2, 0xff);

  write_C381(B152_CURHWCH2, Def_4CURSOR_Size);
  write_C381(B152_CURVWCH2, Def_4CURSOR_Size);
  write_C381(B152_CURTLXCH2, 0x0fff);
  write_C381(B152_CURTLYCH2, 0x0fff);
  write_C381(B152_CURTRXCH2, 0x0fff);
  write_C381(B152_CURTRYCH2, 0x0fff);
  write_C381(B152_CURBLXCH2, 0x0fff);
  write_C381(B152_CURBLYCH2, 0x0fff);
  write_C381(B152_CURBRXCH2, 0x0fff);
  write_C381(B152_CURBRYCH2, 0x0fff);
  write_C381(B152_CURCTCH2, 0x0010);

  // Pixel interpolation LUT mode
  //	ScladpNormal(0);
  //	ScladpNormal(1);
  //	ScladpNormal(2);
  //	ScladpNormal(3);

  return;
}

void ScladpNormal(int ch)
{
	int	bank_offset;
	
	bank_offset = ch*5;

	// ;; Horizontal
	write_C381(B146_SCLADPCTCH1+(bank_offset<<16),0x04);
	write_C381(B146_SCLADPADCH1+(bank_offset<<16),0x00);


	// ;; Vertical
	write_C381(B146_SCLADPCTCH1+(bank_offset<<16),0x08);
	write_C381(B146_SCLADPADCH1+(bank_offset<<16),0x00);

	
	write_C381(B146_SCLADPCTCH1+(bank_offset<<16),0x00);

	return;
}



void InitMenu(void)
{
  int wpdot;

  // for 4-corner Keystone coordinate
  PM_WPPOS4C.tl_x = 0;
  PM_WPPOS4C.tl_y = 0;
  PM_WPPOS4C.tr_x = PS_PANEL_ACT_HW;
  PM_WPPOS4C.tr_y = 0;
  PM_WPPOS4C.bl_x = 0;
  PM_WPPOS4C.bl_y = PS_PANEL_ACT_VW;
  PM_WPPOS4C.br_x = PS_PANEL_ACT_HW;
  PM_WPPOS4C.br_y = PS_PANEL_ACT_VW;

  //WPPOS4C_OLD     = PM_WPPOS4C;

  HMIN = min((int)(PM_WPPOS4C.tr_x - PM_WPPOS4C.tl_x), (int)(PM_WPPOS4C.br_x - PM_WPPOS4C.bl_x) );
  VMIN = min((int)(PM_WPPOS4C.bl_y - PM_WPPOS4C.tl_y), (int)(PM_WPPOS4C.br_y - PM_WPPOS4C.tr_y) );

  wpdot = (PM_WPDTMD==1) ? 64: (PM_WPDTMD==2) ? 16 : 32;

  // others
  PHWNZ           = (int)(ceilf(PS_PANEL_ACT_HW/(float)wpdot));
  PVWNZ           = (int)(ceilf(PS_PANEL_ACT_VW/(float)wpdot));
  PHWHF           = PS_PANEL_ACT_HW/2.0f;
  PVWHF           = PS_PANEL_ACT_VW/2.0f;

  InitWpGrid(0);
  PM_WPGDSELX = 0;
  PM_WPGDSELY = 0;
  SetWpGrid(0,0,1);   // select Top Left
	
//SetWpKey4c(&PM_WPPOS4C);

  return;
}


void InitParam(void)
{
  int16_t i, j, k, l;
  int16_t bd;

// individual parameters
  // register
  IGV_RTCT[0] = 0x0105;
  IGV_RTCT[1] = 0x0105;
  IGV_RTCT[2] = 0x0105;
  IGV_RTCT[3] = 0x0105;

  // edgeblend gain
  // TL
  IGV_EGBCT[0] = 0;
  IPM_EGB_LEN[0] = ( (IPS_SPLIT_IMAGE[0] == E_SplitImgR) || (IPS_SPLIT_IMAGE[0] == E_SplitImgTR) || (IPS_SPLIT_IMAGE[0] == E_SplitImgBR) ) ? 1 : 0;
  IPM_EGB_REN[0] = ( (IPS_SPLIT_IMAGE[0] == E_SplitImgL) || (IPS_SPLIT_IMAGE[0] == E_SplitImgTL) || (IPS_SPLIT_IMAGE[0] == E_SplitImgBL) || (IPS_SPLIT_IMAGE[0] == E_SplitImgNoSplit) ) ? 1 : 0;
  IPM_EGB_TEN[0] = ( (IPS_SPLIT_IMAGE[0] == E_SplitImgB) || (IPS_SPLIT_IMAGE[0] == E_SplitImgBL) || (IPS_SPLIT_IMAGE[0] == E_SplitImgBR) ) ? 1 : 0;
  IPM_EGB_BEN[0] = ( (IPS_SPLIT_IMAGE[0] == E_SplitImgT) || (IPS_SPLIT_IMAGE[0] == E_SplitImgTL) || (IPS_SPLIT_IMAGE[0] == E_SplitImgTR) || (IPS_SPLIT_IMAGE[0] == E_SplitImgNoSplit) ) ? 1 : 0;
  // TR
  IGV_EGBCT[1] = 0;
  IPM_EGB_LEN[1] = ( (IPS_SPLIT_IMAGE[1] == E_SplitImgR) || (IPS_SPLIT_IMAGE[1] == E_SplitImgTR) || (IPS_SPLIT_IMAGE[1] == E_SplitImgBR) || (IPS_SPLIT_IMAGE[1] == E_SplitImgNoSplit) ) ? 1 : 0;
  IPM_EGB_REN[1] = ( (IPS_SPLIT_IMAGE[1] == E_SplitImgL) || (IPS_SPLIT_IMAGE[1] == E_SplitImgTL) || (IPS_SPLIT_IMAGE[1] == E_SplitImgBL) ) ? 1 : 0;
  IPM_EGB_TEN[1] = ( (IPS_SPLIT_IMAGE[1] == E_SplitImgB) || (IPS_SPLIT_IMAGE[1] == E_SplitImgBL) || (IPS_SPLIT_IMAGE[1] == E_SplitImgBR) ) ? 1 : 0;
  IPM_EGB_BEN[1] = ( (IPS_SPLIT_IMAGE[1] == E_SplitImgT) || (IPS_SPLIT_IMAGE[1] == E_SplitImgTL) || (IPS_SPLIT_IMAGE[1] == E_SplitImgTR) || (IPS_SPLIT_IMAGE[1] == E_SplitImgNoSplit) ) ? 1 : 0;
  // BL
  IGV_EGBCT[2] = 0;
  IPM_EGB_LEN[2] = ( (IPS_SPLIT_IMAGE[2] == E_SplitImgR) || (IPS_SPLIT_IMAGE[2] == E_SplitImgTR) || (IPS_SPLIT_IMAGE[2] == E_SplitImgBR) ) ? 1 : 0;
  IPM_EGB_REN[2] = ( (IPS_SPLIT_IMAGE[2] == E_SplitImgL) || (IPS_SPLIT_IMAGE[2] == E_SplitImgTL) || (IPS_SPLIT_IMAGE[2] == E_SplitImgBL) || (IPS_SPLIT_IMAGE[2] == E_SplitImgNoSplit) ) ? 1 : 0;
  IPM_EGB_TEN[2] = ( (IPS_SPLIT_IMAGE[2] == E_SplitImgB) || (IPS_SPLIT_IMAGE[2] == E_SplitImgBL) || (IPS_SPLIT_IMAGE[2] == E_SplitImgBR) || (IPS_SPLIT_IMAGE[2] == E_SplitImgNoSplit) ) ? 1 : 0;
  IPM_EGB_BEN[2] = ( (IPS_SPLIT_IMAGE[2] == E_SplitImgT) || (IPS_SPLIT_IMAGE[2] == E_SplitImgTL) || (IPS_SPLIT_IMAGE[2] == E_SplitImgTR) ) ? 1 : 0;
  // BR
  IGV_EGBCT[3] = 0;
  IPM_EGB_LEN[3] = ( (IPS_SPLIT_IMAGE[3] == E_SplitImgR) || (IPS_SPLIT_IMAGE[3] == E_SplitImgTR) || (IPS_SPLIT_IMAGE[3] == E_SplitImgBR) || (IPS_SPLIT_IMAGE[3] == E_SplitImgNoSplit) ) ? 1 : 0;
  IPM_EGB_REN[3] = ( (IPS_SPLIT_IMAGE[3] == E_SplitImgL) || (IPS_SPLIT_IMAGE[3] == E_SplitImgTL) || (IPS_SPLIT_IMAGE[3] == E_SplitImgBL) ) ? 1 : 0;
  IPM_EGB_TEN[3] = ( (IPS_SPLIT_IMAGE[3] == E_SplitImgB) || (IPS_SPLIT_IMAGE[3] == E_SplitImgBL) || (IPS_SPLIT_IMAGE[3] == E_SplitImgBR) || (IPS_SPLIT_IMAGE[3] == E_SplitImgNoSplit) ) ? 1 : 0;
  IPM_EGB_BEN[3] = ( (IPS_SPLIT_IMAGE[3] == E_SplitImgT) || (IPS_SPLIT_IMAGE[3] == E_SplitImgTL) || (IPS_SPLIT_IMAGE[3] == E_SplitImgTR) ) ? 1 : 0;

  // edgeblend bias
  for ( bd = 0; bd < 4; bd++ )
  {

    IPM_EBIAS_CUR[bd][0][0].x = 0;
    IPM_EBIAS_CUR[bd][0][0].y = 0;
    IPM_EBIAS_CUR[bd][1][0].x = PS_WP_HW - 1;
    IPM_EBIAS_CUR[bd][1][0].y = 0;
    IPM_EBIAS_CUR[bd][0][1].x = 0;
    IPM_EBIAS_CUR[bd][0][1].y = PS_WP_VW - 1;
    IPM_EBIAS_CUR[bd][1][1].x = PS_WP_HW - 1;
    IPM_EBIAS_CUR[bd][1][1].y = PS_WP_VW - 1;
    for ( i = 0; i < 16; i++ )
    {
      IPM_EBIAS[bd][i][0] = 0;
      IPM_EBIAS[bd][i][1] = 0;
      IPM_EBIAS[bd][i][2] = 0;
    }
  }

  // test pattern
  IPM_OFILL_EN[0] = 0;
  IPM_OFILL_EN[1] = 0;
  IPM_OFILL_EN[2] = 0;
  IPM_OFILL_EN[3] = 0;

  // uniformity
  IPM_UC_EN[0] = 0;
  IPM_UC_EN[1] = 0;
  IPM_UC_EN[2] = 0;
  IPM_UC_EN[3] = 0;
  for ( l = 0; l < 3; l++ )
  {
    for ( k = 0; k < 3; k++ )
    {
      for ( j = 0; j < 9; j++ )
      {
        for ( i = 0; i < 11; i++ )
        {
          IPM_UC_GAIN[0][l][k][i][j] = 1;
          IPM_UC_GAIN[1][l][k][i][j] = 1;
          IPM_UC_GAIN[2][l][k][i][j] = 1;
          IPM_UC_GAIN[3][l][k][i][j] = 1;

        }
      }
    }
  }

  // color convergence
  for ( bd = 0; bd < g_ieb_board_num; bd++ )
  {
    IPM_WP_CCVG[bd][0].x = 0;
    IPM_WP_CCVG[bd][0].y = 0;
    IPM_WP_CCVG[bd][1].x = 0;
    IPM_WP_CCVG[bd][1].y = 0;
    IPM_WP_CCVG[bd][2].x = 0;
    IPM_WP_CCVG[bd][2].y = 0;
  }

  // warping
  for ( bd = 0; bd < g_ieb_board_num; bd++ )
  {
    IPM_SEL_GRID_X[bd] = 0;
    IPM_SEL_GRID_Y[bd] = 0;
    for ( j = 0; j < 9; j++ )
    {
      PS_CUR_DEF_OY[j] = PS_WP_VW * j / 8;

      for ( i = 0; i < 9; i++ )
      {
        if( j == 0 )
        {
          PS_CUR_DEF_OX[i] = PS_WP_HW * i / 8;
        }

        IPM_GRID[bd][i][j].x = (float)PS_CUR_DEF_OX[i];
        IPM_GRID[bd][i][j].y = (float)PS_CUR_DEF_OY[j];
      }
    }
    IPM_WP_MODE[bd] = 1;// 5*5//leo 20180621 0:2x2;1:3x3;2:5x5;3:9x9
    IPM_CUR_INTERNAL_ON[bd] = 0;
    IGV_WP_TABLE_COLOR[bd] = 0;

  }

// global parameters
  // edgeblend gain
  PS_SPLIT_IMAGE = IPS_SPLIT_IMAGE[PM_BOARD];
  PM_EGB_OHW = PB_210;
  PM_EGB_OVW = PB_220;
  PM_EGB_EN = 0;
  PM_EGB_LEN = IPM_EGB_LEN[PM_BOARD];
  PM_EGB_REN = IPM_EGB_REN[PM_BOARD];
  PM_EGB_TEN = IPM_EGB_TEN[PM_BOARD];
  PM_EGB_BEN = IPM_EGB_BEN[PM_BOARD];
  GV_EGBCT = ( PM_EGB_EN == 0 ) ? 0 : PM_EGB_LEN + (PM_EGB_REN<<1) + (PM_EGB_TEN<<2) + (PM_EGB_BEN<<3);
  GV_RTCT = IGV_RTCT[PM_BOARD];

  // edgeblend bias
  PM_EBIAS_EN = 0;
  PM_EBIAS_SEL_CUR_X = 0;
  PM_EBIAS_SEL_CUR_Y = 0;
  PM_EBIAS_CUR[0][0] = IPM_EBIAS_CUR[PM_BOARD][0][0];
  PM_EBIAS_CUR[1][0] = IPM_EBIAS_CUR[PM_BOARD][1][0];
  PM_EBIAS_CUR[0][1] = IPM_EBIAS_CUR[PM_BOARD][0][1];
  PM_EBIAS_CUR[1][1] = IPM_EBIAS_CUR[PM_BOARD][1][1];
  PM_EBIAS_PLT = 0;
  for ( i = 0; i < 16; i++ )
  {
    PM_EBIAS[i][0] = IPM_EBIAS[PM_BOARD][i][0];
    PM_EBIAS[i][1] = IPM_EBIAS[PM_BOARD][i][1];
    PM_EBIAS[i][2] = IPM_EBIAS[PM_BOARD][i][2];
  }
  PM_EBIAS_AREA_TEST = 0;
  PM_EBIAS_CUR_MV = 32;
  PM_EBIAS_COLOR = 3;

  // gamma
  PM_EGB_GAMMA_EN = 0;
  PM_EGB_GAMMA[0] = 2.2f;
  PM_EGB_GAMMA[1] = 2.2f;
  PM_EGB_GAMMA[2] = 2.2f;
  PM_EGB_GAMMA_COLOR = 3;
  PM_GAMMA_EN = 0;
  for ( i = 0; i < Def_OGM_NUM; i++ )
  {
    PM_GAMMA[i][0] = 1.0f;
    PM_GAMMA[i][1] = 1.0f;
    PM_GAMMA[i][2] = 1.0f;
    PM_GAMMA_PIX[i] = ( Def_OGM_NUM < 2 ) ? 0 : ( 1024 * i ) / ( Def_OGM_NUM - 1 );
  }
  PM_GAMMA_COLOR = 3;
  PM_GAMMA_PIX_SEL = Def_OGM_NUM;
  PM_EBIAS_GAMMA[0] = 2.2f;
  PM_EBIAS_GAMMA[1] = 2.2f;
  PM_EBIAS_GAMMA[2] = 2.2f;
  PM_EBIAS_GAMMA_COLOR = 3;

  // test pattern
  PM_PATGEN = 0;
  PM_EGBMK_EN = 0;
  PM_OFILL_EN = IPM_OFILL_EN[PM_BOARD];

  // uniformity
  GV_UC_CUR_EN[0]= 0;
  GV_UC_CUR_EN[1] = 0;
  GV_UC_CUR_EN[2] = 0;
  GV_UC_CUR_EN[3] = 0;

  PM_UC_EN = IPM_UC_EN[PM_BOARD];
  PM_UC_SEL_GRID_X = 0;
  PM_UC_SEL_GRID_Y = 0;
  PM_UC_LEVEL = 3;
  PM_UC_COLOR = 3;
  for ( l = 0; l < 3; l++ )
  {
    for ( k = 0; k < 3; k++ )
    {
      for ( j = 0; j < 9; j++ )
      {
        for ( i = 0; i < 11; i++ )
        {
          PM_UC_GAIN[l][k][i][j] = IPM_UC_GAIN[PM_BOARD][l][k][i][j];
        }
      }
    }
  }

  // file
  PM_FILE_NUM = 0;

  // color convergence
  PM_WP_CCVG[0] = IPM_WP_CCVG[PM_BOARD][0];
  PM_WP_CCVG[1] = IPM_WP_CCVG[PM_BOARD][1];
  PM_WP_CCVG[2] = IPM_WP_CCVG[PM_BOARD][2];
  PM_WP_CCVG_COLOR = 0;

  // warping
  GV_WP_TABLE_COLOR = IGV_WP_TABLE_COLOR[PM_BOARD];
  PM_WP_MODE = IPM_WP_MODE[PM_BOARD];//2x2//leo 20180621 0:2x2;1:3x3;2:5x5;3:9x9
  GV_WP_MODE_WP = PM_WP_MODE;
  PM_CUR_ON = 0;
  PM_CUR_INTERNAL_ON = IPM_CUR_INTERNAL_ON[PM_BOARD];
  GV_CUR_INTERNAL_ON_WP = PM_CUR_INTERNAL_ON;

  PM_SEL_GRID_X = IPM_SEL_GRID_X[PM_BOARD];
  PM_SEL_GRID_Y = IPM_SEL_GRID_Y[PM_BOARD];
  GV_PREV_SEL_GRID_X = PM_SEL_GRID_X;
  GV_PREV_SEL_GRID_Y = PM_SEL_GRID_Y;

  PM_GRID_MV = 16;//pixel

  for( j = 0; j < 9; j++ )
  {
    for( i = 0; i < 9; i++ )
    {
      PM_GRID[i][j] = IPM_GRID[PM_BOARD][i][j];
      PM_GRID_OLD[i][j] = PM_GRID[i][j];
    }
  }

  // register
  GV_RTCT = 0x0105;

  // input image splitter

  GV_EGB_OHW_OLD = PM_EGB_OHW;
  GV_EGB_OVW_OLD = PM_EGB_OVW;
}

void Initwp(void)
{
#if 0//HPC add
  int16_t oaoihst,oaoihst1;
  int16_t oaoivst,oaoivst1;
  int16_t bd;
  int16_t bd_old;
  int16_t x, y;
  ///// edgeblending bias area file /////
//  char *fn;
//  char buf[Def_HW_Max+2];
  ///////////////////////////////////////
  bd_old = PM_BOARD;

  InitParam();
  for ( bd = 0; bd < g_ieb_board_num; bd++ )
  {
    PM_BOARD = bd;
    ChangeBoard();
    GV_RTCT = 0x0000;
    C381_WriteReg( B0_RTCT0,(uint8_t ) GV_RTCT);
    C381_WriteReg( B0_RTCT6,(uint8_t )( GV_RTCT>>8));
    // ACT
    C381_Write_Bank_Reg(16, B16_PTHCH1, 0x80);
    C381_Write_Bank_Reg(16, B16_PMODECH1, 0xa1 );

    oaoihst = C381_Read_Bank_Reg(144, B144_OAOI0HSTCH1 ) & 0xff;
    oaoihst1=C381_Read_Bank_Reg(149, B149_OAOI0HSTCH2 ) & 0x0f;
    oaoihst=oaoihst|(oaoihst1<<8);
    oaoivst = C381_Read_Bank_Reg(144, B144_OAOI0VSTCH1 ) & 0xff;
    oaoivst1=C381_Read_Bank_Reg(149, B149_OAOI0VSTCH2 )& 0x0f;
    oaoivst=oaoivst|(oaoivst1<<8);


    C381_Write_Bank_Reg(144, B144_OACTVWCH1,(uint8_t ) PS_WP_VW);
    C381_Write_Bank_Reg(149, B149_OACTVWCH2, (uint8_t )(PS_WP_VW>>8));

    C381_Write_Bank_Reg(144, B144_OAOI0HENDCH1, (uint8_t )(oaoihst + PS_WP_HW) );
    C381_Write_Bank_Reg(149, B149_OAOI0HENDCH2, (uint8_t )((oaoihst + PS_WP_HW)>>8 ));

    C381_Write_Bank_Reg(144, B144_OAOI0VENDCH1, (uint8_t )(oaoivst + PS_WP_VW));
    C381_Write_Bank_Reg(149, B149_OAOI0VENDCH2, (uint8_t )((oaoivst + PS_WP_VW)>>8));

    // warping
    WpmodeChange( PM_WP_MODE );

    // cursor
    C381_Write_Bank_Reg(3,B9_PLTAD2CH1,0x00);
    C381_Write_Bank_Reg(3,B9_PLTDT2CH1,0x00);

    C381_Write_Bank_Reg(3,B3_PLTDT0CH1,0x00);
    C381_Write_Bank_Reg(3,B3_PLTDT0CH1,0x00);

    C381_Write_Bank_Reg(3,B3_PLTDT0CH1,0xff);
    C381_Write_Bank_Reg(3,B3_PLTDT0CH1,0xff);
    C381_Write_Bank_Reg(3,B3_PLTDT0CH1,0xff);

    C381_Write_Bank_Reg(3,B3_OSDSADCH1,0x00);
    C381_Write_Bank_Reg(3,B3_OSDSADCH2,0x00);
    C381_Write_Bank_Reg(3,B3_OSDSADCH3,0xd0);
    C381_Write_Bank_Reg(3,B3_OSDSADCH4,0x07);

    C381_Write_Bank_Reg(3,B3_OSDMWICH1,0x10);
    C381_Write_Bank_Reg(0,B0_BBWMWI,0x10);
    C381_Write_Bank_Reg(3,B3_OSDCTCH1,0x40);


//    C381_Write_Bank_Reg(8,B8_DIFCTCH1,0x03);//C381B8  DIFCT   83h//leo 20180418

    // edgeblend gain
    UpdateInput( 0 );// RTCT = 0x0105

    GV_RTCT = 0x0000;
    C381_WriteReg( BN_RTCT,(uint8_t ) GV_RTCT);
    C381_WriteReg( BN_RTCT2,(uint8_t )( GV_RTCT>>8));

    // cursor
    C381_Write_Bank_Reg(16,B16_CURCT,0x40);
    C381_Write_Bank_Reg(16,B16_CURCT2,0x00);
    C381_Write_Bank_Reg(16,B16_CURCPUAD,0x00);
    C381_Write_Bank_Reg(16,B16_CURCPUAD,0x00);

    for ( y = 0; y < 64; y++ )
    {
      for ( x = 0; x < 32; x++ )
      {
        if ( y < 32 )
        {
          if ( x < 16 )
          {
            C381_Write_Bank_Reg(16, B16_CURCPUDT, 0x00);
          }
          else
          {
            C381_Write_Bank_Reg(16, B16_CURCPUDT, 0x11);
          }
        }
        else
        {
          if ( x < 16 )
          {
            C381_Write_Bank_Reg(16, B16_CURCPUDT, 0x22);
          }
          else
          {
            C381_Write_Bank_Reg(16, B16_CURCPUDT, 0x33 );
          }
        }
      }
    }
    C381_Write_Bank_Reg(16, B16_CURPLTDTR0, 0xff);
    C381_Write_Bank_Reg(16, B16_CURPLTDTR1, 0xff);
    C381_Write_Bank_Reg(16, B16_CURPLTDTR2, 0xff);
    C381_Write_Bank_Reg(16, B16_CURPLTDTR3, 0xff);
    C381_Write_Bank_Reg(16, B16_CURPLTDTG0, 0x00);
    C381_Write_Bank_Reg(16, B16_CURPLTDTG1, 0x00);
    C381_Write_Bank_Reg(16, B16_CURPLTDTG2, 0x00);
    C381_Write_Bank_Reg(16, B16_CURPLTDTG3, 0x00);
    C381_Write_Bank_Reg(16, B16_CURPLTDTB0, 0x00);
    C381_Write_Bank_Reg(16, B16_CURPLTDTB1, 0x00);
    C381_Write_Bank_Reg(16, B16_CURPLTDTB2, 0x00);
    C381_Write_Bank_Reg(16, B16_CURPLTDTB3, 0x00);

    C381_Write_Bank_Reg(16, B16_CURHW, Def_4CURSOR_Size);
    C381_Write_Bank_Reg(16, B16_CURVW, Def_4CURSOR_Size);
    C381_Write_Bank_Reg(16, B16_CURTLX, 0x00);
    C381_Write_Bank_Reg(16, B16_CURTLX2, 0x00);
    C381_Write_Bank_Reg(16, B16_CURTLY, 0x00);
    C381_Write_Bank_Reg(16, B16_CURTLY2, 0x00);
    C381_Write_Bank_Reg(16, B16_CURTRX, 0xff);
    C381_Write_Bank_Reg(16, B16_CURTRX2, 0x0f);
    C381_Write_Bank_Reg(16, B16_CURTRY, 0xff);
    C381_Write_Bank_Reg(16, B16_CURTRY2, 0x0f);
    C381_Write_Bank_Reg(16, B16_CURBLX, 0xff);
    C381_Write_Bank_Reg(16, B16_CURBLX2, 0x0f);
    C381_Write_Bank_Reg(16, B16_CURBLY, 0xff);
    C381_Write_Bank_Reg(16, B16_CURBLY2, 0x0f);
    C381_Write_Bank_Reg(16, B16_CURBRX, 0xff);
    C381_Write_Bank_Reg(16, B16_CURBRX2, 0x0f);
    C381_Write_Bank_Reg(16, B16_CURBRY, 0xff);
    C381_Write_Bank_Reg(16, B16_CURBRY2, 0x0f);
    C381_Write_Bank_Reg(16, B16_CURCT, 0x10);
    C381_Write_Bank_Reg(16, B16_CURCT2, 0x00);

    // edgeblend bias
    C381_Write_Bank_Reg(17, B17_EBIASSAD,(uint8_t )(Def_EBIASSAD&0x07ffffff) );
    C381_Write_Bank_Reg(17, B17_EBIASSAD2,(uint8_t )( (Def_EBIASSAD&0x07ffffff)>>8));
    C381_Write_Bank_Reg(17, B17_EBIASSAD3,(uint8_t )( (Def_EBIASSAD&0x07ffffff)>>16 ));
    C381_Write_Bank_Reg(17, B17_EBIASSAD4,(uint8_t )( (Def_EBIASSAD&0x07ffffff)>>24) );

    C381_Write_Bank_Reg(17, B17_EBIASMWI, Def_EBIASMWI&0xff);

    // gamma
    CalcEgbGainGammaTable( 0 );
    EgbGainGammaTable( 3 );
    CalcOutputGammaTable( 0 );
    OutputGammaTable( 3 );
    CalcEgbBiasGammaTable( 3, 0, 1 );
    EgbBiasGammaTable( 0, 0, 1 );
    EgbBiasGammaTable( 1, 0, 1 );
    EgbBiasGammaTable( 2, 0, 1 );

    // test pattern
    C381_Write_Bank_Reg(2, B2_PGCOLRCH1, 0xff );
    C381_Write_Bank_Reg(2, B2_PGCOLGCH1, 0xff );
    C381_Write_Bank_Reg(2, B2_PGCOLBCH1, 0xff);
    TestCrossHatch();
    TestEGBMarker();

    // uniformity
    InitUniformity( PM_UC_EN );
    SetUniformityEn( 1, 0 );

    // color convergence
    SetColorConvergence( 3 );

    SetUniformityEn( PM_UC_EN, 0 );
    TestOFILL();

    GV_RTCT = 0x0105;
    C381_WriteReg( BN_RTCT,(uint8_t ) GV_RTCT);
    C381_WriteReg( BN_RTCT2,(uint8_t )( GV_RTCT>>8));

    ///// edgeblending bias area file /////
  }

  PM_BOARD = bd_old;
  ChangeBoard();
#endif//HPC add
}

