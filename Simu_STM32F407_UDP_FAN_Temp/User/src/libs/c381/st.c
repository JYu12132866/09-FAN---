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
#include "stm32f4xx_hal.h"
//#include "uart.h"
#include "main.h"
#include "set.h"

#define UP              0X01
#define DOWN            0X02
#define LEFT            0X03
#define RIGHT           0X04

#define TOP_LEFT        0X01
#define TOP_RIGHT       0X02 
#define BOTTOM_LEFT     0X03
#define BOTTOM_RIGHT    0X04

int16_t Cursor_Sel_OLD=0;//LEO
// int16_t Wraping_LR_OLD=0;//LEO
// int16_t Wraping_UD_OLD=0;//LEO
// int16_t Reset_Wrap_OLD=0;//LEO
// extern str_wppos4c PM_WPPOS4C;
// extern str_wppos4c WPPOS4C_OLD;

uint8_t autoWarpDemo = 0;


void FlashRegSave(void)
{
#if 1//HPC add
	uint16_t cnt;
	uint8_t j;
  uint8_t WEMPTY;
  uint8_t WBUBY;
  uint8_t WCompleted;
  uint8_t WFIFOFULL;
  uint8_t FlashRegWrite[16];
	
	 FlashRegWrite[0]=(uint8_t)PM_WPPOS4C.tl_x;
   FlashRegWrite[1]=(uint8_t)(PM_WPPOS4C.tl_x>>8);
 	 FlashRegWrite[2]=(uint8_t)PM_WPPOS4C.tl_y;
   FlashRegWrite[3]=(uint8_t)(PM_WPPOS4C.tl_y>>8);
	 FlashRegWrite[4]=(uint8_t)PM_WPPOS4C.tr_x;
   FlashRegWrite[5]=(uint8_t)(PM_WPPOS4C.tr_x>>8);
	 FlashRegWrite[6]=(uint8_t)PM_WPPOS4C.tr_y;
   FlashRegWrite[7]=(uint8_t)(PM_WPPOS4C.tr_y>>8);
	 FlashRegWrite[8]=(uint8_t)PM_WPPOS4C.bl_x;
   FlashRegWrite[9]=(uint8_t)(PM_WPPOS4C.bl_x>>8);
	 FlashRegWrite[10]=(uint8_t)PM_WPPOS4C.bl_y;
   FlashRegWrite[11]=(uint8_t)(PM_WPPOS4C.bl_y>>8);
	 FlashRegWrite[12]=(uint8_t)PM_WPPOS4C.br_x;
   FlashRegWrite[13]=(uint8_t)(PM_WPPOS4C.br_x>>8);
	 FlashRegWrite[14]=(uint8_t)PM_WPPOS4C.br_y;
   FlashRegWrite[15]=(uint8_t)(PM_WPPOS4C.br_y>>8);	 
//	 for(j=0;j<16;j++)
//		 printf("FlashRegread=%d\r\n",FlashRegWrite[j]);
//	 //16 20211206
//FlashRegWrite[0]=0x4f;
//FlashRegWrite[1]=0x5f;
  ///////////writing data from CPU to FLASH
  write_C381(B5_GIOS, 0x000055);// Use Serial Flash mode
	write_C381(B5_GIOPDCT, 0x03FF);
	write_C381(B5_GIOC, 0x000300);//Drive   GIO[9:8](WP#,HLD#)
	write_C381(B5_GIOO, 0x000300);//High to GIO[9:8](WP#,HLD#)
	 HAL_Delay(200);
  write_C381(B4_SFLCT,0x00);
  write_C381(B4_SFLMODE,0x80);
  HAL_Delay(100);
  write_C381(B4_SFLMODE,0x31);
  write_C381(B4_SFLCMDWR,0x12);
  write_C381(B4_SFLCMDWREN,0x06);
  write_C381(B4_SFLCMDRDSR,0x05);
 
  write_C381(B4_SFLAD,0x00000000);
	 write_C381(B4_SFLSZSEL,0x01);
//	write_C381(B4_SFLBUSCT,0x20);
 


  for(j = 0; j <16; j++)//16 20211206
  {
    write_C381(B4_SFLDT,FlashRegWrite[j]);
    write_C381(B4_SFLAD,j+1);

    if(j==1)
    {
				cnt=30000;
      do
      {
				cnt--;
        WFIFOFULL = read_C381(B4_SFLDTCTL) & 0x10;
      }
      while(WFIFOFULL != 0&&cnt>0);
    }
		//  write_C381(B4_SFLAD,0x00000100);
	}
	  cnt=30000;
    do
    {
			cnt--;
      WEMPTY = read_C381(B4_SFLDTCTL)&0x20;
    }
    while(WEMPTY==0&&cnt>0);
    cnt=30000;
    do
    {
			cnt--;
      WBUBY = read_C381(B4_SFLSTAT)&0x02;
    }
    while(WBUBY!=0&&cnt>0);
		cnt=30000;
    do
    {
			cnt--;
      WCompleted = read_C381(B4_SFLRDSR)&0x01;
    }
    while(WCompleted!=0&&cnt>0);
write_C381(B4_SFLAD,0x00000100);



  ///////////writing data from CPU to FLASH
#endif//HPC add
}


void FlashRegLoad(void)
{
#if 1//HPC add
  uint8_t WEMPTY;
  uint16_t k;
	uint16_t cnt;
//  uint8_t x,y;
  uint8_t FlashRegread[16];
  ///////////READING data from FLASH to CPU
	
	
  write_C381(B5_GIOS, 0x000055);// Use Serial Flash mode
	write_C381(B5_GIOPDCT, 0x03FF);
	write_C381(B5_GIOC, 0x000300);//Drive   GIO[9:8](WP#,HLD#)
	write_C381(B5_GIOO, 0x000300);//High to GIO[9:8](WP#,HLD#)
	 HAL_Delay(200);

  write_C381(B4_SFLCT,0x00);
  write_C381(B4_SFLMODE,0x80);
  HAL_Delay(10);
  write_C381(B4_SFLMODE,0x31);
  write_C381(B4_SFLCMDRD,0x0C);
  write_C381(B4_SFLSZSEL,0x01);
  write_C381(B4_SFLAD,0x00000000);
  // write_C381(B4_SFLBUSCT,0x20);
  write_C381(B4_SFLDTCTL,0x01);
  
  for(k=0; k<16; k++)
  {
		cnt=30000;
    do
    {
			cnt--;
      WEMPTY = read_C381(B4_SFLDTCTL)&0x80;
    }
    while(WEMPTY==0x80&&cnt>0);
    FlashRegread[k] = read_C381(B4_SFLDT);
		// printf("FlashRegread=%d\r\n",FlashRegread[k]);
  }
// HAL_UART_Transmit(&UartHandle, (uint8_t *)FlashRegread, 16,5000);
  write_C381(B4_SFLAD,0x00000000);
  // if( FlashRegread[0]==0x4f)//&&FlashRegread[1]==0x5f) ////16 20211206
	//   printf("Read_OK\n");
  // else 
	//   printf("Read_NG\n");
  k=0;
	PM_WPPOS4C.tl_x= (uint16_t)FlashRegread[0]|((uint16_t)FlashRegread[1])<<8;
 	PM_WPPOS4C.tl_y= (uint16_t)FlashRegread[2]|((uint16_t)FlashRegread[3])<<8;
  PM_WPPOS4C.tr_x= (uint16_t)FlashRegread[4]|((uint16_t)FlashRegread[5])<<8;
	PM_WPPOS4C.tr_y= (uint16_t)FlashRegread[6]|((uint16_t)FlashRegread[7])<<8;
	PM_WPPOS4C.bl_x= (uint16_t)FlashRegread[8]|((uint16_t)FlashRegread[9])<<8;
	PM_WPPOS4C.bl_y= (uint16_t)FlashRegread[10]|((uint16_t)FlashRegread[11])<<8;
	PM_WPPOS4C.br_x= (uint16_t)FlashRegread[12]|((uint16_t)FlashRegread[13])<<8;
	PM_WPPOS4C.br_y= (uint16_t)FlashRegread[14]|((uint16_t)FlashRegread[15])<<8;
	
  // printf("tl_x=%d,tl_y=%d,\r\n", PM_WPPOS4C.tl_x,PM_WPPOS4C.tl_y);
  // printf("tr_x=%d,tr_y=%d,\r\n", PM_WPPOS4C.tr_x,PM_WPPOS4C.tr_y);
  // printf("bl_x=%d,bl_y=%d,\r\n", PM_WPPOS4C.bl_x,PM_WPPOS4C.bl_y);
  // printf("br_x=%d,br_y=%d,\r\n", PM_WPPOS4C.br_x,PM_WPPOS4C.br_y);

	
	if( PM_WPPOS4C.tr_x<=0xFF||PM_WPPOS4C.br_x<=0xFF)
	{
		printf("Warp_Read_NG\r\n");;
		PM_WPPOS4C.tl_x = 0;
		PM_WPPOS4C.tl_y = 0;
		PM_WPPOS4C.tr_x = PS_PANEL_ACT_HW;
		PM_WPPOS4C.tr_y = 0;
		PM_WPPOS4C.bl_x = 0;
		PM_WPPOS4C.bl_y = PS_PANEL_ACT_VW;
		PM_WPPOS4C.br_x = PS_PANEL_ACT_HW;
		PM_WPPOS4C.br_y = PS_PANEL_ACT_VW;
		
		
	}
//HAL_UART_Transmit(&UartHandle, (uint8_t *)"Read_OK\n",7, 5000);
#endif//HPC add
  ///////////READING data from FLASH to CPU
}

void FLASHSectorErase(void)
{
  uint16_t cnt;
  uint8_t rdat;
#if 1
  // Set Serial Flash
  write_C381(B5_GIOS, 0x000055);// Use Serial Flash mode
	write_C381(B5_GIOPDCT, 0x03FF);
	write_C381(B5_GIOC, 0x000300);//Drive   GIO[9:8](WP#,HLD#)
	write_C381(B5_GIOO, 0x000300);//High to GIO[9:8](WP#,HLD#)
	 HAL_Delay(200);
	
  write_C381(B4_SFLCT, 0x00);// Command Idle
  write_C381(B4_SFLMODE, 0x80);// Serial Flash I/F is initialized
  write_C381(B4_SFLMODE, 0x31);// Set Fast_Read, Page size and Serial Flash clock
  write_C381(B4_SFLCMDER, 0xDC);// Write command set of Serial Flash
  write_C381(B4_SFLCMDWREN, 0x06);// WREN Command set to Serial Flash
  write_C381(B4_SFLCMDRDSR, 0x05);// RDSR Command set to Serial Flash
  write_C381(B4_SFLSZSEL, 0x01);// SFLsize is more than 256Mbit
  write_C381(B4_SFLCNT, 0x00002200);// Number of bytes to transfer to Serial Flash
//  write_C381(B4_SFLREGSEL, 0x04);// Set DMA transfer target register

//  write_C381(B4_SFLAD,0x00000000);

  write_C381(B4_SFLCT, 0x04);// Set DMA transfer target register
	cnt=30000;
  do
  {
		cnt--;
    rdat = read_C381(B4_SFLSTAT) & 0x02;
  }
  while (rdat != 0&&cnt>0); // WBUSY and DMABUSY
	cnt=30000;
  do
  {
		cnt--;
    rdat = read_C381(B4_SFLRDSR) & 0x01;
  }
  while (rdat != 0&&cnt>0); // SFL Read Status Register

  write_C381(B4_SFLCT, 0x00);// Command Idle
#endif

}

void MoveCursor(int16_t xdir, int16_t ydir)
{
  write_C381(B147_CURBLXCH1, 0xfff);
  write_C381(B147_CURBLYCH1, 0xfff);
  write_C381(B152_CURBRXCH2, 0xfff);
  write_C381(B152_CURBRYCH2, 0xfff);
  write_C381(B152_CURTRXCH2, 0xfff);
  write_C381(B152_CURTRYCH2, 0xfff);
  write_C381(B147_CURTLXCH1, 0xfff);
  write_C381(B147_CURTLYCH1, 0xfff);
  write_C381(B147_CURCTCH1, 0x0010);
  write_C381(B152_CURCTCH2, 0x0010);
  if (xdir == 0x0C) // TL
  {
    write_C381(B147_CURTLXCH1, PM_WPPOS4C.tl_x);
    write_C381(B147_CURTLYCH1, PM_WPPOS4C.tl_y);
    write_C381(B147_CURCTCH1, 0x0011);
    Cursor_Sel_OLD = 11;
  }
  else if (xdir == 0x0B) // BL
  {
    write_C381(B147_CURBLXCH1, PM_WPPOS4C.bl_x);
    write_C381(B147_CURBLYCH1, (PM_WPPOS4C.bl_y - 24));
    write_C381(B147_CURCTCH1, 0x0011);
    Cursor_Sel_OLD = 13;
  }
  else if (xdir == 0x09) // TR
  {
    write_C381(B152_CURTRXCH2, (PM_WPPOS4C.tr_x - 1936));
    write_C381(B152_CURTRYCH2, (PM_WPPOS4C.tr_y));
    write_C381(B152_CURCTCH2, 0x0011);
    Cursor_Sel_OLD = 12;
  }
  else if (xdir == 0x0A) // BR
  {
    write_C381(B152_CURBRXCH2, (PM_WPPOS4C.br_x - 1936));
    write_C381(B152_CURBRYCH2, (PM_WPPOS4C.br_y - 24));
    write_C381(B152_CURCTCH2, 0x0011);
    Cursor_Sel_OLD = 14;
  }

#if 0
	  if(Cursor_Sel_OLD==11)
		{
			
		 if(xdir!=0)
		 {
//				HAL_UART_Transmit(&UartHandle, (uint8_t *)"11xdir\n",7, 5000);
				write_C381( B147_CURCTCH1, 0x0010);
				write_C381( B152_CURTRXCH2, (PM_WPPOS4C.tr_x-1936));
				write_C381( B152_CURTRYCH2, (PM_WPPOS4C.tr_y));
				write_C381( B152_CURCTCH2, 0x0011);
				Cursor_Sel_OLD=12;
	   }
	  else if(ydir!=0)
		{
//				HAL_UART_Transmit(&UartHandle, (uint8_t *)"11ydir\n",7, 5000);

				write_C381( B147_CURBLXCH1, PM_WPPOS4C.bl_x);
				write_C381( B147_CURBLYCH1, (PM_WPPOS4C.bl_y-24));
				Cursor_Sel_OLD=13;
			
		}
	}
		else if(Cursor_Sel_OLD==12)
			{
			
		 if(xdir!=0)
		 {
//				HAL_UART_Transmit(&UartHandle, (uint8_t *)"12xdir\n",7, 5000);
				write_C381( B152_CURCTCH2, 0x0010);
				write_C381( B147_CURTLXCH1, PM_WPPOS4C.tl_x);
				write_C381( B147_CURTLYCH1, PM_WPPOS4C.tl_y);
				write_C381( B147_CURCTCH1, 0x0011);
				Cursor_Sel_OLD=11;
	   }
	  else if(ydir!=0)
		{
//			HAL_UART_Transmit(&UartHandle, (uint8_t *)"12ydir\n",7, 5000);

			write_C381( B152_CURBRXCH2, (PM_WPPOS4C.br_x-1936));
			write_C381( B152_CURBRYCH2, (PM_WPPOS4C.br_y-24));
			Cursor_Sel_OLD=14;
			
		}
	}		
		else if(Cursor_Sel_OLD==14)
			{
			
		 if(xdir!=0)
		 {
//			 HAL_UART_Transmit(&UartHandle, (uint8_t *)"14xdir\n",7, 5000);
			write_C381( B152_CURCTCH2, 0x0010);
			write_C381( B147_CURBLXCH1, PM_WPPOS4C.bl_x);
			write_C381( B147_CURBLYCH1, (PM_WPPOS4C.bl_y-24));
			write_C381( B147_CURCTCH1, 0x0011);

			 Cursor_Sel_OLD=13;
	   }
	  else if(ydir!=0)
		{
//			HAL_UART_Transmit(&UartHandle, (uint8_t *)"14ydir\n",7, 5000);

			write_C381( B152_CURTRXCH2, (PM_WPPOS4C.tr_x-1936));
			write_C381( B152_CURTRYCH2, (PM_WPPOS4C.tr_y));
			Cursor_Sel_OLD=12;
			
		}
	}		
			else if(Cursor_Sel_OLD==13)
			{
				
		 if(xdir!=0)
		 {
//			 HAL_UART_Transmit(&UartHandle, (uint8_t *)"13xdir\n",7, 5000);
			write_C381( B147_CURCTCH1, 0x0010);
			write_C381( B152_CURBRXCH2, (PM_WPPOS4C.br_x-1936));
			write_C381( B152_CURBRYCH2, (PM_WPPOS4C.br_y-24));
			write_C381( B152_CURCTCH2, 0x0011);

			Cursor_Sel_OLD=14;
	   }
	  else if(ydir!=0)
		{
//			HAL_UART_Transmit(&UartHandle, (uint8_t *)"13ydir\n",7, 5000);

			write_C381( B147_CURTLXCH1, PM_WPPOS4C.tl_x);
			write_C381( B147_CURTLYCH1, PM_WPPOS4C.tl_y);
			Cursor_Sel_OLD=11;
			
		}
	}		
			#endif
}

void MovePixel(int16_t xdir, int16_t ydir)
{
  if (Cursor_Sel_OLD == 11)
  {
    if (xdir == 1 && ydir == 0) // left
    {
      //	 HAL_UART_Transmit(&UartHandle, (uint8_t *)"LFT_11\n",7, 5000);
      if (PM_WPPOS4C.tl_x >= 4)
      {
        PM_WPPOS4C.tl_x = PM_WPPOS4C.tl_x - 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B147_CURTLXCH1, PM_WPPOS4C.tl_x);
        write_C381(B147_CURTLYCH1, PM_WPPOS4C.tl_y);
      }
    }
    else if (xdir == 255 && ydir == 0) // right
    {
      //		HAL_UART_Transmit(&UartHandle, (uint8_t *)"RGT_11\n",7, 5000);
      if (PM_WPPOS4C.tl_x <= 1888)
      {
        PM_WPPOS4C.tl_x = PM_WPPOS4C.tl_x + 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B147_CURTLXCH1, PM_WPPOS4C.tl_x);
        write_C381(B147_CURTLYCH1, PM_WPPOS4C.tl_y);
      }
    }
    else if (ydir == 1 && xdir == 0) // down
    {
      //		HAL_UART_Transmit(&UartHandle, (uint8_t *)"DWN_11\n",7, 5000);
      if (PM_WPPOS4C.tl_y <= 1052)
      {
        PM_WPPOS4C.tl_y = PM_WPPOS4C.tl_y + 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B147_CURTLXCH1, PM_WPPOS4C.tl_x);
        write_C381(B147_CURTLYCH1, PM_WPPOS4C.tl_y);
      }
    }
    else if (ydir == 255 && xdir == 0) // up
    {
      //		HAL_UART_Transmit(&UartHandle, (uint8_t *)"UPP_11\n",7, 5000);
      if (PM_WPPOS4C.tl_y >= 4)
      {
        PM_WPPOS4C.tl_y = PM_WPPOS4C.tl_y - 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B147_CURTLXCH1, PM_WPPOS4C.tl_x);
        write_C381(B147_CURTLYCH1, PM_WPPOS4C.tl_y);
      }
    }
  }
  else if (Cursor_Sel_OLD == 12)
  {
    if (xdir == 1 && ydir == 0) // left
    {
      //			 HAL_UART_Transmit(&UartHandle, (uint8_t *)"LFT_12\n",7, 5000);
      if ((PM_WPPOS4C.tr_x - 1936) >= 4)
      {
        PM_WPPOS4C.tr_x = PM_WPPOS4C.tr_x - 4;

        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B152_CURTRXCH2, (PM_WPPOS4C.tr_x - 1936));
        write_C381(B152_CURTRYCH2, (PM_WPPOS4C.tr_y));
      }
    }
    else if (xdir == 255 && ydir == 0) // right
    {
      //			 HAL_UART_Transmit(&UartHandle, (uint8_t *)"RGT_12\n",7, 5000);
      if ((PM_WPPOS4C.tr_x - 1936) <= 1900) // 1888 leo 20200819
      {
        PM_WPPOS4C.tr_x = PM_WPPOS4C.tr_x + 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B152_CURTRXCH2, (PM_WPPOS4C.tr_x - 1936));
        write_C381(B152_CURTRYCH2, (PM_WPPOS4C.tr_y));
      }
    }
    else if (ydir == 1 && xdir == 0) // down
    {
      //			 HAL_UART_Transmit(&UartHandle, (uint8_t *)"DWN_12\n",7, 5000);
      if (PM_WPPOS4C.tr_y <= 1052)
      {
        PM_WPPOS4C.tr_y = PM_WPPOS4C.tr_y + 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B152_CURTRXCH2, (PM_WPPOS4C.tr_x - 1936));
        write_C381(B152_CURTRYCH2, (PM_WPPOS4C.tr_y));
      }
    }
    else if (ydir == 255 && xdir == 0) // up
    {
      //			 HAL_UART_Transmit(&UartHandle, (uint8_t *)"UPP_12\n",7, 5000);
      if (PM_WPPOS4C.tr_y >= 4)
      {
        PM_WPPOS4C.tr_y = PM_WPPOS4C.tr_y - 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B152_CURTRXCH2, (PM_WPPOS4C.tr_x - 1936));
        write_C381(B152_CURTRYCH2, (PM_WPPOS4C.tr_y));
      }
    }
  }
  else if (Cursor_Sel_OLD == 14)
  {
    if (xdir == 1 && ydir == 0) // left
    {
      //			 HAL_UART_Transmit(&UartHandle, (uint8_t *)"LFT_14\n",7, 5000);
      if ((PM_WPPOS4C.br_x - 1936) >= 4)
      {
        PM_WPPOS4C.br_x = PM_WPPOS4C.br_x - 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B152_CURBRXCH2, (PM_WPPOS4C.br_x - 1936));
        write_C381(B152_CURBRYCH2, (PM_WPPOS4C.br_y - 24));
      }
    }
    else if (xdir == 255 && ydir == 0) // right
    {
      //			HAL_UART_Transmit(&UartHandle, (uint8_t *)"RHT_14\n",7, 5000);
      if ((PM_WPPOS4C.br_x - 1936) <= 1900) // 1888 leo 20200819
      {
        PM_WPPOS4C.br_x = PM_WPPOS4C.br_x + 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B152_CURBRXCH2, (PM_WPPOS4C.br_x - 1936));
        write_C381(B152_CURBRYCH2, (PM_WPPOS4C.br_y - 24));
      }
    }
    else if (ydir == 1 && xdir == 0) // down
    {
      //			HAL_UART_Transmit(&UartHandle, (uint8_t *)"DWN_14\n",7, 5000);
      if (((PM_WPPOS4C.br_y - 24) / 2) <= 1066) // 1052 leo 20200819
      {
        PM_WPPOS4C.br_y = PM_WPPOS4C.br_y + 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B152_CURBRXCH2, (PM_WPPOS4C.br_x - 1936));
        write_C381(B152_CURBRYCH2, (PM_WPPOS4C.br_y - 24));
      }
    }
    else if (ydir == 255 && xdir == 0) // up
    {
      //			HAL_UART_Transmit(&UartHandle, (uint8_t *)"UPP_14\n",7, 5000);
      if (((PM_WPPOS4C.br_y - 24) / 2) >= 4)
      {
        PM_WPPOS4C.br_y = PM_WPPOS4C.br_y - 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B152_CURBRXCH2, (PM_WPPOS4C.br_x - 1936));
        write_C381(B152_CURBRYCH2, (PM_WPPOS4C.br_y - 24));
      }
    }
  }
  else if (Cursor_Sel_OLD == 13)
  {

    if (xdir == 1 && ydir == 0) // left
    {
      //			 HAL_UART_Transmit(&UartHandle, (uint8_t *)"LFT_13\n",7, 5000);
      if (PM_WPPOS4C.bl_x >= 4)
      {
        PM_WPPOS4C.bl_x = PM_WPPOS4C.bl_x - 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B147_CURBLXCH1, PM_WPPOS4C.bl_x);
        write_C381(B147_CURBLYCH1, (PM_WPPOS4C.bl_y - 24));
      }
    }
    else if (xdir == 255 && ydir == 0) // right
    {
      //			HAL_UART_Transmit(&UartHandle, (uint8_t *)"RGT_13\n",7, 5000);
      if (PM_WPPOS4C.bl_x <= 1892) // leo 20200819 1888
      {
        PM_WPPOS4C.bl_x = PM_WPPOS4C.bl_x + 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B147_CURBLXCH1, PM_WPPOS4C.bl_x);
        write_C381(B147_CURBLYCH1, (PM_WPPOS4C.bl_y - 24));
      }
    }
    else if (ydir == 1 && xdir == 0) // down
    {
      //			HAL_UART_Transmit(&UartHandle, (uint8_t *)"DWN_13\n",7, 5000);
      if (((PM_WPPOS4C.bl_y - 24) / 2) <= 1066) // 1054 //leo 20200819
      {
        PM_WPPOS4C.bl_y = PM_WPPOS4C.bl_y + 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B147_CURBLXCH1, PM_WPPOS4C.bl_x);
        write_C381(B147_CURBLYCH1, (PM_WPPOS4C.bl_y - 24));
      }
    }
    else if (ydir == 255 && xdir == 0) // up
    {
      //			HAL_UART_Transmit(&UartHandle, (uint8_t *)"UPP_13\n",7, 5000);
      if (((PM_WPPOS4C.bl_y - 24) / 2) >= 4)
      {
        PM_WPPOS4C.bl_y = PM_WPPOS4C.bl_y - 4;
        SetWpKey4c(&PM_WPPOS4C);
        write_C381(B147_CURBLXCH1, PM_WPPOS4C.bl_x);
        write_C381(B147_CURBLYCH1, (PM_WPPOS4C.bl_y - 24));
      }
    }
  }
}


void autoWarpDemoSvc()
{
  autoWarpDemo = 0 ;
  // NOTE: 不再需要开发自动融合矫正接口s
  return ;
}



void stwp()
{
#if 1
  if (C381ModeChange == TRUE)
  {
    C381_Init();
    C381ModeChange = FALSE;
  }
  if (CursorSel_LR != 0 && PM_CUR_ON == 1)
  {
    switch (CursorSel_LR)
    {
    case 0x09: // E_KeyLeftwardGrid TR
      MoveCursor(0x09, 0);
      CursorSel_LR = 0;
      break;
    case 0x0A: // BR
      MoveCursor(0x0A, 0);
      CursorSel_LR = 0;
      break;
    default:
      break;
    }
  }
  else if (CursorSel_UD != 0 && PM_CUR_ON == 1)
  {
    switch (CursorSel_UD)
    {
    case 0x0B: // E_KeyDownwardGrid  BL
      MoveCursor(0x0B, 0);
      CursorSel_UD = 0;
      break;
    case 0x0C:             // TL
      MoveCursor(0x0C, 0); // E_KeyUpwardGrid
      CursorSel_UD = 0;
      break;
    default:
      break;
    }
  }
  else if (Wraping_LR != 0 && PM_CUR_ON == 1)
  {
    //   int16_t wperr = E_WpNoErr;
    switch (Wraping_LR)
    {
    case 0x04: // left
      MovePixel(1, 0);
      // Wraping_LR_OLD=Wraping_LR;
      Wraping_LR = 0;
      break;
    case 0x05: // Right
      MovePixel(255, 0);
      //      Wraping_LR_OLD=Wraping_LR;
      Wraping_LR = 0;
      break;
    default:
      break;
    }
  }
  else if (Wraping_UD != 0 && PM_CUR_ON == 1)
  {
    // int16_t wperr = E_WpNoErr;
    switch (Wraping_UD)
    {
    case 0x06: // donw
      MovePixel(0, 1);
      Wraping_UD = 0;
      break;
    case 0x07: // up
      MovePixel(0, 255);
      Wraping_UD = 0;
      break;
    default:
      break;
    }
  }
  //  else if(Movement!=0)
  //  {
  //    PM_GRID_MV=Movement;
  //    Movement=0;
  //  }
  else if (CursorMode != 0)
  {
    if (CursorMode == 0x0f)
    {

      write_C381(B147_CURBLXCH1, 0xfff);
      write_C381(B147_CURBLYCH1, 0xfff);
      write_C381(B152_CURBRXCH2, 0xfff);
      write_C381(B152_CURBRYCH2, 0xfff);
      write_C381(B152_CURTRXCH2, 0xfff);
      write_C381(B152_CURTRYCH2, 0xfff);
      write_C381(B147_CURTLXCH1, 0xfff);
      write_C381(B147_CURTLYCH1, 0xfff);
      PM_CUR_ON = 0;

      write_C381(B147_CURTLXCH1, PM_WPPOS4C.tl_x);
      write_C381(B147_CURTLYCH1, PM_WPPOS4C.tl_y);
      write_C381(B147_CURCTCH1, 0x0010);
      write_C381(B152_CURCTCH2, 0x0010);
      Cursor_Sel_OLD = 11;
      printf("Wrap cursor off\r\n");
    }
    else
    {
      PM_CUR_ON = 1;
      write_C381(B147_CURTLXCH1, PM_WPPOS4C.tl_x);
      write_C381(B147_CURTLYCH1, PM_WPPOS4C.tl_y);
      write_C381(B147_CURCTCH1, 0x0011);
      Cursor_Sel_OLD = 11; // TL
      printf("x,y:%x,%x\n", PM_WPPOS4C.tl_x, PM_WPPOS4C.tl_y);
      printf("Wrap cursor on\r\n");
    }
    CursorMode = 0;
  }
  else if (Reset_Wrap != 0)
  {
    PM_CUR_ON = 0;
    write_C381(B147_CURBLXCH1, 0xfff);
    write_C381(B147_CURBLYCH1, 0xfff);
    write_C381(B152_CURBRXCH2, 0xfff);
    write_C381(B152_CURBRYCH2, 0xfff);
    write_C381(B152_CURTRXCH2, 0xfff);
    write_C381(B152_CURTRYCH2, 0xfff);
    write_C381(B147_CURTLXCH1, 0xfff);
    write_C381(B147_CURTLYCH1, 0xfff);
    Cursor_Sel_OLD = 11;
    //    ChangeGrid(0);
    //    InitWpTable();
    //    PM_WP_MODE=1;
    //    WpmodeChange(PM_WP_MODE);
    WP_Hkeystone = 0; // LEO 20180524 C381
    WP_Vkeystone = 0;
    WP_HkeystoneOld = 50; // LEO 20180524 C381
    WP_VkeystoneOld = 50;
    PM_WPPOS4C.tl_x = 0;
    PM_WPPOS4C.tl_y = 0;
    PM_WPPOS4C.tr_x = PS_PANEL_ACT_HW;
    PM_WPPOS4C.tr_y = 0;
    PM_WPPOS4C.bl_x = 0;
    PM_WPPOS4C.bl_y = PS_PANEL_ACT_VW;
    PM_WPPOS4C.br_x = PS_PANEL_ACT_HW;
    PM_WPPOS4C.br_y = PS_PANEL_ACT_VW;
    SetWpKey4c(&PM_WPPOS4C);
    Reset_Wrap = 0;
  }
  else if (Save_Wrap != 0)
  {
    FLASHSectorErase();
    FlashRegSave();
    Save_Wrap = 0;
    printf("Warp_Save\r\n");
  }
  // else if(Load_Wrap!=0)
  // {
  //   // FlashRegLoad();
  //   // SetWpKey4c(&PM_WPPOS4C);
  //   // Load_Wrap=0;
  // 	// printf("Warp_Load\r\n");
  // }
  else if(autoWarpDemo!=0)
  {
    autoWarpDemoSvc();
  }


  CursorSel_LR = 0;
  CursorSel_UD = 0; // LEO
  Wraping_LR = 0;   // LEO
  Wraping_UD = 0;   // LEO
#endif
}

void keystone()
{
  if(WP_HkeystoneStatus==0x0f)
  {
    if(WP_Hkeystone<50)
    {
      if(WP_Hkeystone<WP_HkeystoneOld)
      {
        for(uint8_t i=(WP_HkeystoneOld-WP_Hkeystone); i>0; i--)
          MoveCur(16, 0);//right move
        WpmdGridChange(1, 0);//E_KeyRightwardGrid
        ChangeGrid(0);
        for(uint8_t i=(WP_HkeystoneOld-WP_Hkeystone); i>0; i--)
          MoveCur(-16, 0);//left move
        WpmdGridChange(-1, 0);//E_KeyLeftwardGrid
        ChangeGrid(0);
      }
      else if(WP_Hkeystone>WP_HkeystoneOld)
      {
        MoveCur(-16, 0);//left move
        WpmdGridChange(1, 0);//E_KeyRightwardGrid
        ChangeGrid(0);
        MoveCur(16, 0);//right move
        WpmdGridChange(-1, 0);//E_KeyLeftwardGrid
        ChangeGrid(0);
      }
      WP_HkeystoneOld= WP_Hkeystone;
    }
    else if(WP_Hkeystone>50)
    {
      if(WP_Hkeystone>WP_HkeystoneOld)
      {
        WpmdGridChange(0, 1);//E_KeyDownwardGrid
        ChangeGrid(0);
        for(uint8_t i=(WP_Hkeystone-WP_HkeystoneOld); i>0; i--)
          MoveCur(16, 0);//right move
        WpmdGridChange(1, 0);//E_KeyRightwardGrid
        ChangeGrid(0);
        for(uint8_t i=(WP_Hkeystone-WP_HkeystoneOld); i>0; i--)
          MoveCur(-16, 0);//left move
        WpmdGridChange(-1, 0);//E_KeyLeftwardGrid
        ChangeGrid(0);
        WpmdGridChange(0, -1);//E_KeyUpwardGrid
        ChangeGrid(0);
      }
      else if(WP_Hkeystone<WP_HkeystoneOld)
      {
        WpmdGridChange(0, 1);//E_KeyDownwardGrid
        ChangeGrid(0);
        MoveCur(-16, 0);//left move
        WpmdGridChange(1, 0);//E_KeyRightwardGrid
        ChangeGrid(0);
        MoveCur(16, 0);//right move
        WpmdGridChange(-1, 0);//E_KeyLeftwardGrid
        ChangeGrid(0);
        WpmdGridChange(0, -1);//E_KeyUpwardGrid
        ChangeGrid(0);
      }
      WP_HkeystoneOld= WP_Hkeystone;
    }
    else if(WP_Hkeystone==50)
    {
      InitWpTable();
      WP_HkeystoneOld= WP_Hkeystone;
    }
    WP_HkeystoneStatus=0;
  }
  else if(WP_VkeystoneStatus==0x0f)
  {
    if(WP_Vkeystone<50)
    {
      if(WP_Vkeystone<WP_VkeystoneOld)
      {
        for(uint8_t i=(WP_VkeystoneOld-WP_Vkeystone); i>0; i--)
          MoveCur(0, 16);// move donw
        WpmdGridChange(0, 1);//E_KeyDownwardGrid
        ChangeGrid(0);
        for(uint8_t i=(WP_VkeystoneOld-WP_Vkeystone); i>0; i--)
          MoveCur(0, -16);//move up
        WpmdGridChange(0, -1);//E_KeyUpwardGrid
        ChangeGrid(0);
      }
      else if(WP_Vkeystone>WP_VkeystoneOld)
      {
        MoveCur(0, -16);//move up
        WpmdGridChange(0, 1);//E_KeyDownwardGrid
        ChangeGrid(0);
        MoveCur(0, 16);// move donw
        WpmdGridChange(0, -1);//E_KeyUpwardGrid
        ChangeGrid(0);
      }
      WP_VkeystoneOld= WP_Vkeystone;
    }
    else if(WP_Vkeystone>50)
    {
      if(WP_Vkeystone<WP_VkeystoneOld)
      {
        WpmdGridChange(1, 0);//E_KeyRightwardGrid
        ChangeGrid(0);
        MoveCur(0, -16);//move up
        WpmdGridChange(0, 1);//E_KeyDownwardGrid
        ChangeGrid(0);
        MoveCur(0, 16);// move donw
        WpmdGridChange(0, -1);//E_KeyUpwardGrid
        ChangeGrid(0);
        WpmdGridChange(-1, 0);//E_KeyLeftwardGrid
        ChangeGrid(0);
      }
      else if(WP_Vkeystone>WP_VkeystoneOld)
      {
        WpmdGridChange(1, 0);//E_KeyRightwardGrid
        ChangeGrid(0);
        for(uint8_t i=(WP_Vkeystone-WP_VkeystoneOld); i>0; i--)
          MoveCur(0, 16);// move donw
        WpmdGridChange(0, 1);//E_KeyDownwardGrid
        ChangeGrid(0);
        for(uint8_t i=(WP_Vkeystone-WP_VkeystoneOld); i>0; i--)
          MoveCur(0, -16);//move up
        WpmdGridChange(0, -1);//E_KeyUpwardGrid
        ChangeGrid(0);
        WpmdGridChange(-1, 0);//E_KeyLeftwardGrid
        ChangeGrid(0);
      }
      WP_VkeystoneOld= WP_Vkeystone;
    }
    else if(WP_Vkeystone==50)
    {
      InitWpTable();
      WP_VkeystoneOld= WP_Vkeystone;
    }
    WP_VkeystoneStatus=0;
  }
}


void FlashDMASave(void)
{
 // uint16_t rdat;

#if 0
// GV_RTCT=GV_RTCT & 0xf8f8;
// C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
// C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

  // Set Serial Flash
  write_C381(1,B1_GIOS, 0x0f);// Use Serial Flash mode
  write_C381(1,B1_GIOS2, 0x00);// Use Serial Flash mode
  write_C381(0,B0_SFLCT, 0x00);// Command Idle
  write_C381(0,B0_SFLMODE, 0x80);// Serial Flash I/F is initialized
  write_C381(0,B0_SFLMODE, 0x15);// Set Fast_Read, Page size and Serial Flash clock
  write_C381(0,B0_SFLCMDWR, 0x12);// Write command set of Serial Flash
  write_C381(0,B0_SFLCMDWREN, 0x06);// WREN Command set to Serial Flash
  write_C381(0,B0_SFLCMDRDSR, 0x05);// RDSR Command set to Serial Flash
  write_C381(0,B0_SFLSZSEL, 0x01);// SFLsize is more than 256Mbit
  write_C381(0,B0_SFLCNT0, 0xC0);// Number of bytes to transfer to Serial Flash
  write_C381(0,B0_SFLCNT1, 0xA0);// Number of bytes to transfer to Serial Flash
  write_C381(0,B0_SFLCNT2, 0x00);// Number of bytes to transfer to Serial Flash
  write_C381(0,B0_SFLCNT3, 0x00);// Number of bytes to transfer to Serial Flash
  write_C381(0,B0_SFLREGSEL, 0x0f);// Set DMA transfer target register

  if (GV_WP_TABLE_COLOR == 0)
  {
    // Access TableA
    if (Def_Wp_Space == 32)
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x02);
      //C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
      C381_Write_Bank_Reg(14,B14_DTCT,0x12);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
    }
    else
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x00);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
      C381_Write_Bank_Reg(14,B14_DTCT,0x10);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
    }
  }
  else // Access TableB
  {
    if (Def_Wp_Space == 32)
    {
      //C381_Write_Bank_Reg(14,B14_DTCT,0x12);
      //C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
      C381_Write_Bank_Reg(14,B14_DTCT,0x22);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x02);
    }
    else
    {
      //C381_Write_Bank_Reg(14,B14_DTCT,0x10);
      //C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
      C381_Write_Bank_Reg(14,B14_DTCT,0x20);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x02);
    }
  }

  C381_Write_Bank_Reg(14,B14_DTAD, 0x00);
  C381_Write_Bank_Reg(14,B14_DTAD, 0x00);

  C381_Write_Bank_Reg(0,B0_SFLAD1,0x00);
  C381_Write_Bank_Reg(0,B0_SFLAD2,0x00);
  C381_Write_Bank_Reg(0,B0_SFLAD3,0x00);
  C381_Write_Bank_Reg(0,B0_SFLAD4,0x00);

  C381_Write_Bank_Reg(0,B0_SFLCT, 0x01);// Set DMA transfer

  do
  {
    rdat = C381_Read_Bank_Reg(0,B0_SFLSTAT) & 0x03;
  }
  while (rdat != 0); // WBUSY and DMABUSY
  do
  {
    rdat = C381_Read_Bank_Reg(0,B0_SFLRDSR) & 0x03;
  }
  while (rdat != 0); // SFL Read Status Register

  C381_Write_Bank_Reg(0,B0_SFLCT, 0x00);// Command Idle
  C381_Write_Bank_Reg(14,B14_DTAD, 0x20);

//  GV_RTCT = GV_RTCT | 0x0007;

//  C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//  C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

  if(GV_WP_TABLE_COLOR == 0)
  {
    if(Def_Wp_Space == 32)
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x12);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
      C381_Write_Bank_Reg(14,B14_DTCT,0x02);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
    }
    else
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x10);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x01);

      C381_Write_Bank_Reg(14,B14_DTCT,0x00);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
    }
  }
  else
  {
    if(Def_Wp_Space == 32)
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x02);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x00);

      C381_Write_Bank_Reg(14,B14_DTCT,0x12);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
    }
    else
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x00);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x00);

      C381_Write_Bank_Reg(14,B14_DTCT,0x10);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
    }
  }

// GV_RTCT = GV_RTCT & 0xfff8;
// GV_RTCT = GV_RTCT | 0x0005;
// C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
// C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

#endif
}


void FlashDMALoad(void)
{

//  uint8_t rdat;
#if 0
  // Set Serial Flash
  C381_Write_Bank_Reg(1,B1_GIOS, 0x0f);// Use Serial Flash mode
  C381_Write_Bank_Reg(1,B1_GIOS2, 0x00);// Use Serial Flash mode

  C381_Write_Bank_Reg(0,B0_SFLCT, 0x00);// Command Idle
  C381_Write_Bank_Reg(0,B0_SFLMODE, 0x80);// Serial Flash I/F is initialized
  C381_Write_Bank_Reg(0,B0_SFLMODE, 0x15);// Set Fast_Read, Page size and Serial Flash clock
  C381_Write_Bank_Reg(0,B0_SFLCMDRD, 0x0C);// Read Command setting from Serial Flash
  C381_Write_Bank_Reg(0,B0_SFLSZSEL, 0x01);// SFLsize is more than 256Mbit
  C381_Write_Bank_Reg(0,B0_SFLCNT0, 0xC0);// Number of bytes to transfer to Serial Flash
  C381_Write_Bank_Reg(0,B0_SFLCNT1, 0xA0);// Number of bytes to transfer to Serial Flash
  C381_Write_Bank_Reg(0,B0_SFLCNT2, 0x00);// Number of bytes to transfer to Serial Flash
  C381_Write_Bank_Reg(0,B0_SFLCNT3, 0x00);// Number of bytes to transfer to Serial Flash

  C381_Write_Bank_Reg(0,B0_SFLREGSEL, 0x0f);// Set DMA transfer target register

//    GV_RTCT=GV_RTCT & 0xf8f8;
// C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
// C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

  if (GV_WP_TABLE_COLOR == 0)
  {
    // Access TableA
    if (Def_Wp_Space == 32)
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x02);
      //C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
      C381_Write_Bank_Reg(14,B14_DTCT,0x12);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
    }
    else
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x00);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
      C381_Write_Bank_Reg(14,B14_DTCT,0x10);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
    }
  }
  else // Access TableB
  {
    if (Def_Wp_Space == 32)
    {
      //C381_Write_Bank_Reg(14,B14_DTCT,0x12);
      //C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
      C381_Write_Bank_Reg(14,B14_DTCT,0x22);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x02);
    }
    else
    {
      //C381_Write_Bank_Reg(14,B14_DTCT,0x10);
      //C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
      C381_Write_Bank_Reg(14,B14_DTCT,0x20);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x02);
    }
  }

  C381_Write_Bank_Reg(14,B14_DTAD, 0x00);
  C381_Write_Bank_Reg(14,B14_DTAD, 0x00);

  C381_Write_Bank_Reg(0,B0_SFLAD1,0x001);
  C381_Write_Bank_Reg(0,B0_SFLAD2,0x00);
  C381_Write_Bank_Reg(0,B0_SFLAD3,0x00);
  C381_Write_Bank_Reg(0,B0_SFLAD4,0x00);// Serial Flash write address

  C381_Write_Bank_Reg(0,B0_SFLCT, 0x02);// Set DMA transfer

  do
  {
    rdat = C381_Read_Bank_Reg(0,B0_SFLSTAT) & 0x01;
  }
  while (rdat != 0); // DMABUSY
  C381_Write_Bank_Reg(0,B0_SFLCT, 0x00);// Command Idle

//   GV_RTCT = GV_RTCT | 0x0007;

//   C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//   C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));

  if(GV_WP_TABLE_COLOR == 0)
  {
    if(Def_Wp_Space == 32)
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x12);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
      C381_Write_Bank_Reg(14,B14_DTCT,0x02);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
    }
    else
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x10);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
      C381_Write_Bank_Reg(14,B14_DTCT,0x00);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x00);
    }
  }
  else
  {
    if(Def_Wp_Space == 32)
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x02);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x00);

      C381_Write_Bank_Reg(14,B14_DTCT,0x12);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x01);

    }
    else
    {
      // C381_Write_Bank_Reg(14,B14_DTCT,0x00);
      // C381_Write_Bank_Reg(14,B14_DTCT2,0x00);

      C381_Write_Bank_Reg(14,B14_DTCT,0x10);
      C381_Write_Bank_Reg(14,B14_DTCT2,0x01);
    }
  }

//  GV_RTCT = GV_RTCT & 0xfff8;
//  GV_RTCT = GV_RTCT | 0x0005;
//  C381_WriteReg(BN_RTCT,(uint8_t)GV_RTCT);
//  C381_WriteReg(BN_RTCT2,(uint8_t)(GV_RTCT>>8));
#endif

}

