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


coord_f_t GV_CP[9][9];
int16_t GV_WARP_TABLE_X[130][76];
int16_t GV_WARP_TABLE_Y[130][76];

void IntWpTable(void)
{
  int16_t x,y;

  for(y=20; y>=0; y--)
  {
    for(x=20; x>=0; x--)
    {
      GV_WARP_TABLE_X[x][y] =(x<<4);
      GV_WARP_TABLE_Y[x][y] =(y<<4);
    }
  }
  return;
}

int16_t Warp(void)
{
  char err = 0;

  float x, y;
  int16_t i, j, k;
  int16_t h, v;
  int16_t w;
  float ver_line_x[9];
  float hor_line_y[130][9];
  float x0, y0;
  int16_t area;
  float dx, dy;
  float bi, cu;
  float d1;
#if Def_WpLimitMode
  float diffy_y_sum, divn;
#endif
  float cp_hscl[2][7];
  float cp_vscl[2][7];


  if(GV_WP_MODE_WP == 0)// 2x2 grid mode
  {
    CalcPjConv();
  }

  if(GV_NUM_HGRID != 9)
  {
    v = 0;
    for(j = 0; j < 9; j++)
    {
      if(GV_VGRID_CLC_EN[j] == 1)
      {
        h = 0;
        for(i = 0; i < 9; i++)
        {
          if(GV_HGRID_CLC_EN[i] == 1)
          {
            GV_CP[h][v] = PM_GRID[i][j];
            h = h + 1;
          }
        }
        CalcHLineCoef(v);
        v = v + 1;
      }
    }

    v = 0;
    for(j = 0; j < 9; j++)
    {
      if(GV_VGRID_CLC_EN[j] == 1)
      {
        area = 0;
        h = 0;
        for(i = 1; i < 9; i++)
        {
          w = 1;
          while(GV_HGRID_CLC_EN[i] == 0)
          {
            w = w + 1;
            i = i + 1;
          }

          for(k = 1; k < w; k++)
          {
            PM_GRID[area+k][j].x = (PM_GRID[i][j].x * k + PM_GRID[area][j].x * (w - k)) / w;
            dx = PM_GRID[area+k][j].x - PM_GRID[area][j].x;
            bi = dx * dx;
            cu = bi * dx;
            PM_GRID[area+k][j].y = GV_WP_HOR_LINE[h][v].a + GV_WP_HOR_LINE[h][v].b * dx + GV_WP_HOR_LINE[h][v].c * bi + GV_WP_HOR_LINE[h][v].d * cu;
          }
          area = i;
          h = h + 1;
        }
        v = v + 1;
      }
    }
  }

  if(GV_NUM_VGRID != 9)
  {
    h = 0;
    for(j = 0; j < 9; j++)
    {
      if(GV_HGRID_CLC_EN[j] == 1)
      {
        v = 0;
        for(i=0; i<9; i++)
        {
          if(GV_VGRID_CLC_EN[i] == 1)
          {
            GV_CP[h][v] = PM_GRID[j][i];
            v = v + 1;
          }
        }
        CalcVLineCoef(h);
        h = h + 1;
      }
    }

    h = 0;
    for(j = 0; j < 9; j++)
    {
      if(GV_HGRID_CLC_EN[j] == 1)
      {
        area = 0;
        v = 0;
        for(i = 1; i < 9; i++)
        {
          w = 1;
          while(GV_VGRID_CLC_EN[i] == 0)
          {
            w = w + 1;
            i = i + 1;
          }

          for(k = 1; k < w; k++)
          {
            PM_GRID[j][area+k].y = (PM_GRID[j][i].y * k + PM_GRID[j][area].y * (w - k)) / w;
            dy = PM_GRID[j][area+k].y - PM_GRID[j][area].y;
            bi = dy * dy;
            cu = bi * dy;
            PM_GRID[j][area+k].x = GV_WP_VER_LINE[h][v].a + GV_WP_VER_LINE[h][v].b * dy + GV_WP_VER_LINE[h][v].c * bi + GV_WP_VER_LINE[h][v].d * cu;
          }
          area = i;
          v = v + 1;
        }
        h = h + 1;
      }
    }
  }

  if((GV_WP_MODE_WP!=0) && (GV_CUR_INTERNAL_ON_WP==1) && ((GV_NUM_HGRID!=9) || (GV_NUM_VGRID!=9)))
  {
    for(j = 0; j < 9; j++)
    {
      area = 0;
      if(GV_VGRID_CLC_EN[j] == 0)
      {
        for(i = 1; i < 9; i++)
        {
          w = 1;
          while(GV_HGRID_CLC_EN[i] == 0)
          {
            w = w + 1;
            i = i + 1;
          }

          for(k = 1; k < w; k++)
          {
            PM_GRID[area+k][j].x = (PM_GRID[i][j].x * k + PM_GRID[area][j].x * (w - k)) / w;
          }
          area = i;
        }
      }
    }

    for(j = 0; j < 9; j++)
    {
      area = 0;
      if(GV_HGRID_CLC_EN[j] == 0)
      {
        for(i=1; i<9; i++){
          w = 1;
          while(GV_VGRID_CLC_EN[i] == 0)
          {
            w = w + 1;
            i = i + 1;
          }

          for(k = 1; k < w; k++)
          {
            PM_GRID[j][area+k].y = (PM_GRID[j][i].y * k + PM_GRID[j][area].y * (w - k)) / w;
          }
          area = i;
        }
      }
    }
  }
  else if((GV_WP_MODE_WP==0) || (GV_CUR_INTERNAL_ON_WP==0))
  {
    dx = PM_GRID[8][0].x - PM_GRID[0][0].x;
    for(i = 1; i < 8; i++)
    {
      cp_hscl[0][i-1] = (PM_GRID[i][0].x - PM_GRID[0][0].x) / dx;
    }

    dx = PM_GRID[8][8].x - PM_GRID[0][8].x;
    for(i = 1; i < 8; i++)
    {
      cp_hscl[1][i-1] = (PM_GRID[i][8].x - PM_GRID[0][8].x) / dx;
    }

    dy = PM_GRID[0][8].y - PM_GRID[0][0].y;
    for(i = 1; i < 8; i++)
    {
      cp_vscl[0][i-1] = (PM_GRID[0][i].y - PM_GRID[0][0].y) / dy;
    }

    dy = PM_GRID[8][8].y - PM_GRID[8][0].y;
    for(i = 1; i < 8; i++)
    {
      cp_vscl[1][i-1] = (PM_GRID[8][i].y - PM_GRID[8][0].y) / dy;
    }

    // internal cursor
    // x
    for(j = 1; j < 8; j++)
    {
      for(i = 1; i < 8; i++)
      {
        dx = PM_GRID[8][j].x - PM_GRID[0][j].x;
        dy = PM_GRID[i][8].y - PM_GRID[i][0].y;

        PM_GRID[i][j].x = PM_GRID[0][j].x + dx * ((float)(8-j) * cp_hscl[0][i-1] + j * cp_hscl[1][i-1]) / (float)8;
        PM_GRID[i][j].y = PM_GRID[i][0].y + dy * ((float)(8-i) * cp_vscl[0][j-1] + i * cp_vscl[1][j-1]) / (float)8;
      }
    }
  }

  // spline
  CalcLineCoef();

  // table
  for(i = 0; i <= PS_WP_HW_GRD; i++)
  {
    x0 = (float)(i << Def_Wp_Space_Bit);

    for(j = 0; j < 9; j++) 
    {
      for(area = 0; area < 8; area++) 
      {

        if((x0 <PM_GRID[area+1][j].x) || (area == 7)) 
        {
          dx = x0 - PM_GRID[area][j].x;
          bi = dx * dx;
          cu = bi * dx;
          hor_line_y[i][j] = GV_WP_HOR_LINE[area][j].a + GV_WP_HOR_LINE[area][j].b * dx+ GV_WP_HOR_LINE[area][j].c * bi + GV_WP_HOR_LINE[area][j].d * cu;
          break;
        }
      }

#if Def_WpLimitMode
      if(j > 0)
      {
        dy = hor_line_y[i][j] - PS_WP_LIMV_ITV;
        if(hor_line_y[i][j-1] > dy)
        {
          return E_WpErrItv;
        }
      }
#endif
    }
    CalcMVYCoef(hor_line_y[i], i);
  }

//  PS_WP_HW_GRD=(PS_WP_HW_GRD>133)?133:(PS_WP_HW_GRD);
//  PS_WP_VW_GRD=(PS_WP_VW_GRD>76)?76:(PS_WP_VW_GRD);

  for(j = PS_WP_VW_GRD; j >= 0; j--)
  {
    y0 = (float)(j << Def_Wp_Space_Bit);

    for(i = 0; i < 9; i++)
    {
      for(area = 0; area < 8; area++)
      {
        if((y0 < PM_GRID[i][area+1].y) || (area == 7))
        {
          dy = y0 - PM_GRID[i][area].y;
          bi = dy * dy;
          cu = bi * dy;
          ver_line_x[i] = GV_WP_VER_LINE[i][area].a + GV_WP_VER_LINE[i][area].b * dy + GV_WP_VER_LINE[i][area].c * bi + GV_WP_VER_LINE[i][area].d * cu;
          break;
        }
      }

#if Def_WpLimitMode
      if(i>0)
      {
        dx = ver_line_x[i] - PS_WP_LIMH_ITV;
        if(ver_line_x[i-1] > dx)
        {
          return E_WpErrItv;
        }
      }
#endif
    }

    CalcMVXCoef(ver_line_x);

#if Def_WpLimitMode
    diffy_y_sum = 0;
    divn = 0;
#endif

    for(i = PS_WP_HW_GRD; i >= 0; i--)
    {
      x0 = (float)(i << Def_Wp_Space_Bit);
      // x
      dx = ver_line_x[1] - ver_line_x[0];
      d1 = ver_line_x[8] - ver_line_x[7];
      if((x0 - ver_line_x[0]) < (-dx))
      {
        x = -1024;
      }
      else if((x0 - ver_line_x[8]) > d1)
      {
        x = 3068;
      }
      else
      {
        for(k = 1; k < 9; k++)
        {
          if((x0 < ver_line_x[k]) || (k == 8))
          {
            area = k - 1;
            break;
          }
        }

        dx = x0 - ver_line_x[area];

        bi = dx * dx;
        cu = bi * dx;
        x = GV_WP_HOR_LINE_MV[area].a + GV_WP_HOR_LINE_MV[area].b * dx + GV_WP_HOR_LINE_MV[area].c * bi + GV_WP_HOR_LINE_MV[area].d * cu;
        x = x + (float)(i << Def_Wp_Space_Bit);
      }

       x = (x > (PM_IACT_HW+(Def_Wp_Space<<3))) ? (PM_IACT_HW+(Def_Wp_Space<<3)) : ((x < -(Def_Wp_Space<<3)) ? -(Def_Wp_Space<<3) : x);
       GV_WARP_TABLE_X[i][j] = (int16_t)(x * 16 + 0.5f);

#if 1
      // y  
      dy = hor_line_y[i][1] - hor_line_y[i][0];
      d1 = hor_line_y[i][8] - hor_line_y[i][7];
      if((y0 - hor_line_y[i][0]) < (-dy))
      {
        y = -1024;
      }
      else if((y0 - hor_line_y[i][8]) > d1)
      {
        y = 3071;
      }
      else
      {
        for(k = 1; k < 9; k++)
        {
          if((y0 < hor_line_y[i][k]) || (k == 8))
          {
            area = k - 1;
            break;
          }
        }

        dy = y0 - hor_line_y[i][area];

        bi = dy * dy;
        cu = bi * dy;
        y = GV_WP_VER_LINE_MV[i][area].a + GV_WP_VER_LINE_MV[i][area].b * dy + GV_WP_VER_LINE_MV[i][area].c * bi + GV_WP_VER_LINE_MV[i][area].d * cu;
        y = y + (float)(j << Def_Wp_Space_Bit);
      }
      y = (y > (PM_IACT_VW+(Def_Wp_Space<<3))) ? (PM_IACT_VW+(Def_Wp_Space<<3)) : ((y < -(Def_Wp_Space<<3)) ? -(Def_Wp_Space<<3) : y);
      GV_WARP_TABLE_Y[i][j] = (int16_t)(y * 16 + 0.5f); 
#endif    
#if Def_WpLimitMode
      // Check Limit
      err = CheckWpLimitA(&i, &j, &diffy_y_sum, &divn);
      if(err != E_WpNoErr)
      {
        return err;
      }
#endif
    }

#if Def_WpLimitMode
    // Check Limit
    err = CheckWpLimitB(&diffy_y_sum, &divn);
    if(err != E_WpNoErr)
    {
        return err;
    }
#endif
  }

  WriteWpTable();

  return err;
}
