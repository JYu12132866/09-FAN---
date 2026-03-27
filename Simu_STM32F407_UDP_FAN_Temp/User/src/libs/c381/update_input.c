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

void UpdateInput( int16_t cur_disp )
{
#if 0//HPC add
  int16_t PI_ACT_HST_tmp;
  int16_t PI_ACT_HW_tmp;
  int16_t PI_ACT_VST_tmp;
  int16_t PI_ACT_VW_tmp;
  //  int16_t input_err_org = PS_INPUT_ERR[PM_BOARD] ;// v0.51
  PS_INPUT_ERR[PM_BOARD]  = E_InNoErr;

  GV_RTCT = 0x0000;
  C381_WriteReg( BN_RTCT,(uint8_t ) GV_RTCT);
  C381_WriteReg( BN_RTCT2,(uint8_t )( GV_RTCT>>8));

  C381_Write_Bank_Reg( 8,B8_PIH0CH1, 0x00 );// PIACTSTMD disable  PI_ACT_VW = 0x4c0;
  C381_Write_Bank_Reg( 8,B8_PIH0CH2, 0x40 );// PIACTSTMD disable  PI_ACT_VW = 0x4c0;

  PI_ACT_HST = C381_Read_Bank_Reg( 8,B8_IACTHSTCH1 );
  PI_ACT_HST_tmp=C381_Read_Bank_Reg( 8,B8_IACTHSTCH2 );
  PI_ACT_HST=(PI_ACT_HST_tmp<<8)|PI_ACT_HST;

  PI_ACT_HW = C381_Read_Bank_Reg( 8,B8_IACTHWCH1);
  PI_ACT_HW_tmp=C381_Read_Bank_Reg( 8,B8_IACTHWCH2 );
  PI_ACT_HW=(PI_ACT_HW_tmp<<8)|PI_ACT_HW;

  PI_ACT_VST = C381_Read_Bank_Reg(8, B8_IACTVSTCH1);
  PI_ACT_VST_tmp=C381_Read_Bank_Reg(8, B8_IACTVSTCH2);
  PI_ACT_VST=(PI_ACT_VST_tmp<<8)|PI_ACT_VST;

  PI_ACT_VW = C381_Read_Bank_Reg( 8,B8_IACTVWCH1 );
  PI_ACT_VW_tmp = C381_Read_Bank_Reg( 8,B8_IACTVWCH2 );
  PI_ACT_VW=(PI_ACT_VW_tmp<<8)|PI_ACT_VW;


  if ( (PI_ACT_HST == 0) || (PI_ACT_HW == 0) || (PI_ACT_VST == 0) || (PI_ACT_VW == 0) ) {// v054
  PS_INPUT_ERR[PM_BOARD] = E_InErrNoIn;// v054
  }// v054

  C381_Write_Bank_Reg( 8,B8_PIH0CH1, 0x00 );// PIACTSTMD disable  PI_ACT_VW = 0x4c0;
  C381_Write_Bank_Reg( 8,B8_PIH0CH2, 0x00 );// PIACTSTMD disable  PI_ACT_VW = 0x4c0;

  //HAL_Delay(150);
  //if ( PS_INPUT_ERR[PM_BOARD]  != E_InErrNoIn ) { WaitPivs( ); }// v054

  SetSplitWpParam();
  PS_SPLIT_IMAGE=E_SplitImgNoSplit;

  SetSplitWpReg( PS_SPLIT_IMAGE, cur_disp );

  // edgeblend gain
  EgbGain();
  GV_RTCT = 0x0105;

  C381_WriteReg( BN_RTCT,(uint8_t ) GV_RTCT);
  C381_WriteReg( BN_RTCT2,(uint8_t )( GV_RTCT>>8));
#endif//HPC add

}
