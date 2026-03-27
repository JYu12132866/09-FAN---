// Copyright (C) 2016 i-Chips Technology Inc. All Rights Reserved.
#include <time.h>// for TimeOut
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

int16_t WaitPovs( void)
{
//  int16_t ct;
  int16_t err = 0;
#if 0//HPC add

  ct = 0x04;// POVS

  C381_Write_Bank_Reg(1,B1_INTEN ,ct);

  do{
      C381_Write_Bank_Reg(1,B1_INTRST ,ct);
  err = C381_Read_Bank_Reg(1,B1_INTSTAT)&ct;
  } while(err!=0x04); 

  C381_Write_Bank_Reg(1,B1_INTEN ,0x00);
#endif//HPC add
  return err;

}
