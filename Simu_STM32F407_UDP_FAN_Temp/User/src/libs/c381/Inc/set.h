#ifndef __SET_H
#define __SET_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include "main.h"
#include "c381.h"
#include "arm_math.h"
//#include "stm32f4xx_conf.h"

// Error
#define DEF_WPERR_CURPOS	(30)

#define	    DEF_WPDMYHW		0
// for Cursor
#define DEF_CURSIZE		16			
// for 8-point
#define	DEF_WPGDSIZE	16	
// for warp table
#define DEF_WPTBLMAX_H	264			 
#define DEF_WPTBLMAX_V	144		


extern uint8_t    HMIN;
extern uint8_t    VMIN;

extern int		PHWNZ;
extern int		PVWNZ;
extern float		PHWHF;
extern float		PVWHF;


void InitWpGrid(int mode);
void SetWpGrid(int gx, int gy, int chg);
void InitWpTable(void);
void calclut(float fratio, int LUT[24]);
void wait1_povs(void);
int SetCurPos( str_wppos4c *p4c);
void SetLut_wplg(void);
int SetWpKey4c( str_wppos4c *p4c);
void CalcHomography(str_wppos4c *p4c);
int CheckWpLimit(str_wppos4c *p4c);
int check_curpos( str_wppos4c *p4c);

#endif
