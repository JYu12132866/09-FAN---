/**
  */ 

/* Includes ------------------------------------------------------------------*/

#include "set.h"
#include "c381.h"
int    GPTx[3];
int    GPTy[3];




//限制范围
int CheckWpLimit(str_wppos4c *p4c)
{
  int    x,y,tx,ty,ic,ix,iy;
  float  FHW,FVW;
  float  hg_sx,hg_sy,hg_dx1,hg_dx2,hg_dy1,hg_dy2,hg_z,hg_g,hg_h;
  float  hg_sys0,hg_sys1,hg_sys2,hg_sys3,hg_sys4,hg_sys5,hg_sys6,hg_sys7;
  float  hg_inv0,hg_inv1,hg_inv2,hg_inv3,hg_inv4,hg_inv5;
  float  WX,WY,tmp;
  int    sftn;
  float  Px[4][2][2],Py[4][2][2];
  float  diffx_x,diffy_x,diffx_y,diffy_y;
  float  tan_h,tan_v;

  FHW  = (float)PS_PANEL_ACT_HW;
  FVW   = (float)PS_PANEL_ACT_VW;
  sftn = (PM_WPDTMD==2) ? 4
     : (PM_WPDTMD==1) ? 6
                      : 5;

//===== calculate homography coefficient =====//
  hg_sx  = (float)((p4c->tl_x - p4c->tr_x) + (p4c->br_x - p4c->bl_x));
  hg_sy  = (float)((p4c->tl_y - p4c->tr_y) + (p4c->br_y - p4c->bl_y));
  hg_dx1  = (float)(p4c->tr_x - p4c->br_x);
  hg_dx2  = (float)(p4c->bl_x - p4c->br_x);
  hg_dy1  = (float)(p4c->tr_y - p4c->br_y);
  hg_dy2  = (float)(p4c->bl_y - p4c->br_y);
  hg_z  = (hg_dx1*hg_dy2) - (hg_dy1*hg_dx2);
  hg_g  = ((hg_sx*hg_dy2) - (hg_sy*hg_dx2)) / hg_z;
  hg_h  = ((hg_sy*hg_dx1) - (hg_sx*hg_dy1)) / hg_z;
 
  hg_sys0  = p4c->tr_x*(1 + hg_g) - p4c->tl_x;
  hg_sys1  = p4c->bl_x*(1 + hg_h) - p4c->tl_x;
  hg_sys2  = p4c->tl_x;
  hg_sys3  = p4c->tr_y*(1 + hg_g) - p4c->tl_y;
  hg_sys4  = p4c->bl_y*(1 + hg_h) - p4c->tl_y;
  hg_sys5  = p4c->tl_y;
  hg_sys6 = hg_g;
  hg_sys7  = hg_h;

//===== check homography error =====//
  for(ic=3; ic>=0; ic--){
    if(ic==0){    // Top Left
      x = 0;
      y = 0;
    }
    else if(ic==1){  // Top Right
      x = PHWNZ;
      y = 0;
    }
    else if(ic==2){  // Bottom Left
      x = 0;
      y = PVWNZ;
    }
    else{      // Bottom Right
      x = PHWNZ;
      y = PVWNZ;
    }

    hg_inv0  = hg_sys7*(x<<sftn) - hg_sys1;
    hg_inv1  = hg_sys0 - hg_sys6*(x<<sftn);
    hg_inv1 = (hg_inv1<0.000001f) ? 0.000001f : hg_inv1;
    hg_inv2  = (x<<sftn) - hg_sys2;
    hg_inv3 = hg_sys6*(y<<sftn) - hg_sys3;
    hg_inv4 = hg_sys4 - hg_sys7*(y<<sftn);
    hg_inv4 = (hg_inv4<0.000001f) ? 0.000001f : hg_inv4;
    hg_inv5  = (y<<sftn) - hg_sys5;
    tmp = 1 - (hg_inv3*hg_inv0)/(hg_inv4*hg_inv1);

    if(tmp<0.000001f) return DEF_WPERR_HOMOGRAPHY;
  }

//===== check warping error =====//
  for(ic=3; ic>=0; ic--){
    if(ic==0){    // Top Left
      tx = ((p4c->tl_x)>>sftn);
      ty = ((p4c->tl_y)>>sftn);
    }
    else if(ic==1){  // Top Right
      tx = ((p4c->tr_x)>>sftn);
      ty = ((p4c->tr_y)>>sftn);
    }
    else if(ic==2){  // Bottom Left
      tx = ((p4c->bl_x)>>sftn);
      ty = ((p4c->bl_y)>>sftn);
    }
    else{      // Bottom Right
      tx = ((p4c->br_x)>>sftn);
      ty = ((p4c->br_y)>>sftn);
    }
    
    for(iy=1; iy>=0; iy--){
      for(ix=1; ix>=0; ix--){
        x = tx + ix;
        y = ty + iy;
        hg_inv0  = hg_sys7*(x<<sftn) - hg_sys1;
        hg_inv1  = hg_sys0 - hg_sys6*(x<<sftn);
        hg_inv1 = (hg_inv1<0.000001f) ? 0.000001f : hg_inv1;
        hg_inv2  = (x<<sftn) - hg_sys2;
        hg_inv3 = hg_sys6*(y<<sftn) - hg_sys3;
        hg_inv4 = hg_sys4 - hg_sys7*(y<<sftn);
        hg_inv4 = (hg_inv4<0.000001f) ? 0.000001f : hg_inv4;
        hg_inv5  = (y<<sftn) - hg_sys5;
        tmp = 1 - (hg_inv3*hg_inv0)/(hg_inv4*hg_inv1);
        tmp = (tmp<0.000001f) ? 0.000001f : tmp;

        WY  = FVW * (hg_inv3*hg_inv2 + hg_inv1*hg_inv5) / (hg_inv4*hg_inv1*tmp);
        WX  = (FHW/hg_inv1) * ((hg_inv0*WY/FVW) + hg_inv2);
        Px[ic][ix][iy] = (WX>32767) ? 32767 : (WX<-32768) ? -32768 : WX;
        Py[ic][ix][iy] = (WY>32767) ? 32767 : (WY<-32768) ? -32768 : WY;
      } // end for ix
    } // end for iy

    diffx_x = Px[ic][1][0] - Px[ic][0][0];
    diffy_x = Py[ic][1][0] - Py[ic][0][0];
    // Horizontal slope
    tan_h = diffy_x/diffx_x;
    if((tan_h < (-DEF_WPLIMANG_H))||(tan_h > DEF_WPLIMANG_H)){
      //HAL_UART_Transmit(&UartHandle, (uint8_t *)"CheckWpLimit1 ", 14, 5000);
//      printf("(%d):(%6.2f,%6.2f), (%6.2f,%6.2f)\n",ic,Px[ic][0][0],Py[ic][0][0],Px[ic][1][0],Py[ic][1][0]);
//      printf("    (%6.2f,%6.2f), (%6.2f,%6.2f)\n",Px[ic][0][1],Py[ic][0][1],Px[ic][1][1],Py[ic][1][1]);
//      printf("diffx_x=%f, diffy_x=%f, tan_h=%f\n\n",diffx_x,diffy_x,tan_h);
      return DEF_WPERRANG_H;
    }
    // Horizontal shrink rate
    if( diffx_x > DEF_WPLIMHSH ){
      //HAL_UART_Transmit(&UartHandle, (uint8_t *)"CheckWpLimit2 ", 14, 5000);
//      printf("(%d):(%6.2f,%6.2f), (%6.2f,%6.2f)\n",ic,Px[ic][0][0],Py[ic][0][0],Px[ic][1][0],Py[ic][1][0]);
//      printf("    (%6.2f,%6.2f), (%6.2f,%6.2f)\n",Px[ic][0][1],Py[ic][0][1],Px[ic][1][1],Py[ic][1][1]);
//      printf("diffx_x=%f\n\n",diffx_x);
      return DEF_WPERRHSH;
    }
      
    diffx_y = Px[ic][0][1] - Px[ic][0][0];
    diffy_y = Py[ic][0][1] - Py[ic][0][0];
    // Vertical local shrink rate
    if( diffy_y > DEF_WPLIMVSH_L ){
      //HAL_UART_Transmit(&UartHandle, (uint8_t *)"CheckWpLimit3 ", 14, 5000);
//      printf("(%d):(%6.2f,%6.2f), (%6.2f,%6.2f)\n",ic,Px[ic][0][0],Py[ic][0][0],Px[ic][1][0],Py[ic][1][0]);
//      printf("    (%6.2f,%6.2f), (%6.2f,%6.2f)\n",Px[ic][0][1],Py[ic][0][1],Px[ic][1][1],Py[ic][1][1]);
//      printf("diffy_y=%f\n\n",diffy_y);
      return DEF_WPERRVSH_L;
    }
    // Vertical slope
    tan_v = diffx_y/diffy_y;
    if((tan_v < (-DEF_WPLIMANG_V))||(tan_v > DEF_WPLIMANG_V)){
      //HAL_UART_Transmit(&UartHandle, (uint8_t *)"CheckWpLimit4 ", 14, 5000);
//      printf("(%d):(%6.2f,%6.2f), (%6.2f,%6.2f)\n",ic,Px[ic][0][0],Py[ic][0][0],Px[ic][1][0],Py[ic][1][0]);
//      printf("    (%6.2f,%6.2f), (%6.2f,%6.2f)\n",Px[ic][0][1],Py[ic][0][1],Px[ic][1][1],Py[ic][1][1]);
//      printf("diffy_y=%f, diffx_y=%f, tan_v=%f\n\n",diffy_y,diffx_y,tan_v);
      return DEF_WPERRANG_V;
    }
  } // end for ic

  return 0;
}

void InitWpGrid(int mode)
{
  int x, y, cpuwad = 0;
  int inc;
  int16_t cnt;
  inc = (mode == 0) ? 2 : 1;

  // OSD-plane initialize
  write_C381(B4_OSDCT, 0x00);
  write_C381(B4_BBACTHW, 0x0fff); // BitBLT ACT HW(4096)
  write_C381(B4_BBACTVW, 0x095f); // BitBLT ACT VW(2400)
  write_C381(B4_CPUWAD, (PS_OSDSAD0 & 0x7fffffff));
  write_C381(B4_BBWMWI, (PS_OSDMWI & 0xff)); // BitBLT WMWI
  write_C381(B4_BBRMWI, (PS_OSDMWI & 0xff)); // BitBLT RMWI
  write_C381(B4_OSDFILL, 0x00);              // OSDFILL(trans)
  write_C381(B4_OSDCT, 0x01);                // OSDFILL exe
  cnt = 8000;
  do
  {
    cnt = cnt - 1;
  } while ((read_C381(B4_BOSTAT) & 0x01) == 0x01 && (cnt > 0)); // 20210623
  write_C381(B4_OSDCT, 0x00);
  // write grid
  // top
  for (x = 0; x < 3; x = x + inc)
  {
    if (x == 0)
      cpuwad = PS_OSDSAD0;
    else if (x == 1)
      cpuwad = PS_OSDSAD0 + (GPTx[x] - (DEF_WPGDSIZE >> 1));
    else
      cpuwad = PS_OSDSAD0 + (PS_PANEL_ACT_HW - DEF_WPGDSIZE);
    write_C381(B4_OSDFILL, 0xFF);
    write_C381(B4_BBACTHW, (DEF_WPGDSIZE - 1) & 0xffff);
    write_C381(B4_BBACTVW, (DEF_WPGDSIZE - 1) & 0x3fff);
    write_C381(B4_CPUWAD, (cpuwad & 0x7fffffff));
    write_C381(B4_OSDCT, 0x01);
    cnt = 8000;
    do
    {
      cnt = cnt - 1;
    } while ((read_C381(B4_BOSTAT) & 0x01) == 0x01 && (cnt > 0)); // 20210623
    write_C381(B4_OSDCT, 0x00);
  }
  // bottom
  cpuwad = PS_OSDSAD0 + (PS_OSDMWI << 7) * (PS_PANEL_ACT_VW - DEF_WPGDSIZE);

  write_C381(B4_OSDFILL, 0xFF);
  write_C381(B4_BBACTHW, (PS_PANEL_ACT_HW - 1) & 0xffff);
  write_C381(B4_BBACTVW, (DEF_WPGDSIZE - 1) & 0x3fff);
  write_C381(B4_CPURAD, (PS_OSDSAD0 & 0x7fffffff));
  write_C381(B4_CPUWAD, (cpuwad & 0x7fffffff));
  write_C381(B4_OSDCT, 0x02);
  cnt = 8000;
  do
  {
    cnt = cnt - 1;
  } while ((read_C381(B4_BOSTAT) & 0x01) == 0x01 && (cnt > 0)); // 20210623
  write_C381(B4_OSDCT, 0x00);

  // left
  for (y = 0; y < 3; y = y + inc)
  {
    if (y == 0)
      cpuwad = PS_OSDSAD0;
    else if (y == 1)
      cpuwad = PS_OSDSAD0 + (PS_OSDMWI << 7) * (GPTy[y] - (DEF_WPGDSIZE >> 1));
    else
      cpuwad = PS_OSDSAD0 + (PS_OSDMWI << 7) * (PS_PANEL_ACT_VW - DEF_WPGDSIZE);

    write_C381(B4_OSDFILL, 0xFF);
    write_C381(B4_BBACTHW, (DEF_WPGDSIZE - 1) & 0xffff);
    write_C381(B4_BBACTVW, (DEF_WPGDSIZE - 1) & 0x3fff);
    write_C381(B4_CPUWAD, (cpuwad & 0x7fffffff));
    write_C381(B4_OSDCT, 0x01);
    cnt = 8000;
    do
    {
      cnt = cnt - 1;
    } while ((read_C381(B4_BOSTAT) & 0x01) == 0x01 && (cnt > 0)); // 20210623
    write_C381(B4_OSDCT, 0x00);
  }

  // right
  cpuwad = PS_OSDSAD0 + (PS_PANEL_ACT_HW - DEF_WPGDSIZE);

  write_C381(B4_OSDFILL, 0x11);
  write_C381(B4_BBACTHW, (DEF_WPGDSIZE - 1) & 0xffff);
  write_C381(B4_BBACTVW, (PS_PANEL_ACT_VW - 1) & 0x3fff);
  write_C381(B4_CPURAD, (PS_OSDSAD0 & 0x7fffffff));
  write_C381(B4_CPUWAD, (cpuwad & 0x7fffffff));
  write_C381(B4_OSDCT, 0x02);
  cnt = 8000;
  do
  {
    cnt = cnt - 1;
  } while ((read_C381(B4_BOSTAT) & 0x01) == 0x01 && (cnt > 0)); // 20210623
  write_C381(B4_OSDCT, 0x00);
  return;
}

void SetWpGrid(int gx, int gy, int chg)
{
  int  cpuwad=0,offset;
  int16_t cnt;
  // 1st(make back ground)
  offset = (PS_OSDMWI<<7)*DEF_WPGDSIZE + DEF_WPGDSIZE;
  if((gy==0)&&(gx==0))      cpuwad = PS_OSDSAD0 - offset;
  else if((gy==0)&&(gx==1)) cpuwad = PS_OSDSAD0 + (GPTx[gx]-(DEF_WPGDSIZE>>1)) - offset;
  else if((gy==0)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;
  else if((gy==1)&&(gx==0)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(GPTy[gy]-(DEF_WPGDSIZE>>1)) - offset;
  else if((gy==1)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(GPTy[gy]-(DEF_WPGDSIZE>>1)) + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;
  else if((gy==2)&&(gx==0)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) - offset;
  else if((gy==2)&&(gx==1)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) + (GPTx[gx]-(DEF_WPGDSIZE>>1)) - offset;
  else if((gy==2)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;

  if(chg) write_C381(B4_OSDFILL  ,0xFF);
  else    write_C381(B4_OSDFILL  ,0x00);
  write_C381(B4_BBACTHW    ,(DEF_WPGDSIZE*3-1)&0xffff);
  write_C381(B4_BBACTVW    ,(DEF_WPGDSIZE*3-1)&0x3fff);
  write_C381(B4_CPUWAD     ,(cpuwad&0x7fffffff));
  write_C381(B4_OSDCT      ,0x01);
	cnt=8000;
  do {cnt=cnt-1;   } while ((read_C381(B4_BOSTAT)&0x01)==0x01&&(cnt>0));//leo 20210623
  write_C381(B4_OSDCT      ,0x00);

  // 2nd(make frame)
  if(chg)
  {
    offset = (PS_OSDMWI<<7)*((DEF_WPGDSIZE*3)>>2) + ((DEF_WPGDSIZE*3)>>2);
    if((gy==0)&&(gx==0))      cpuwad = PS_OSDSAD0 - offset;
    else if((gy==0)&&(gx==1)) cpuwad = PS_OSDSAD0 + (GPTx[gx]-(DEF_WPGDSIZE>>1)) - offset;
    else if((gy==0)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;
    else if((gy==1)&&(gx==0)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(GPTy[gy]-(DEF_WPGDSIZE>>1)) - offset;
    else if((gy==1)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(GPTy[gy]-(DEF_WPGDSIZE>>1)) + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;
    else if((gy==2)&&(gx==0)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) - offset;
    else if((gy==2)&&(gx==1)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) + (GPTx[gx]-(DEF_WPGDSIZE>>1)) - offset;
    else if((gy==2)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;

    write_C381(B4_OSDFILL    ,0x00);
    write_C381(B4_BBACTHW    ,(((DEF_WPGDSIZE*10)>>2)-1)&0xffff);
    write_C381(B4_BBACTVW    ,(((DEF_WPGDSIZE*10)>>2)-1)&0x3fff);
    write_C381(B4_CPUWAD     ,(cpuwad&0x7fffffff));
    write_C381(B4_OSDCT      ,0x01);
			cnt=8000;
    do {cnt=cnt-1;  } while ((read_C381(B4_BOSTAT)&0x01)==0x01&&(cnt>0));//leo 20210623
    write_C381(B4_OSDCT      ,0x00);
  }

  // 3rd(make grid)
  offset = 0;
  if((gy==0)&&(gx==0))      cpuwad = PS_OSDSAD0 - offset;
  else if((gy==0)&&(gx==1)) cpuwad = PS_OSDSAD0 + (GPTx[gx]-(DEF_WPGDSIZE>>1)) - offset;
  else if((gy==0)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;
  else if((gy==1)&&(gx==0)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(GPTy[gy]-(DEF_WPGDSIZE>>1)) - offset;
  else if((gy==1)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(GPTy[gy]-(DEF_WPGDSIZE>>1)) + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;
  else if((gy==2)&&(gx==0)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) - offset;
  else if((gy==2)&&(gx==1)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) + (GPTx[gx]-(DEF_WPGDSIZE>>1)) - offset;
  else if((gy==2)&&(gx==2)) cpuwad = PS_OSDSAD0 + (PS_OSDMWI<<7)*(PS_PANEL_ACT_VW-DEF_WPGDSIZE) + (PS_PANEL_ACT_HW-DEF_WPGDSIZE) - offset;

  write_C381(B4_OSDFILL    ,0xFF);
  write_C381(B4_BBACTHW    ,(DEF_WPGDSIZE-1)&0xffff);
  write_C381(B4_BBACTVW    ,(DEF_WPGDSIZE-1)&0x3fff);
  write_C381(B4_CPUWAD     ,(cpuwad&0x7fffffff));
  write_C381(B4_OSDCT      ,0x01);
	cnt=8000;
  do {cnt=cnt-1;  } while ((read_C381(B4_BOSTAT)&0x01)==0x01&&(cnt>0));//leo 20210623
  write_C381(B4_OSDCT      ,0x00);

  return;
}



void lutset_6s_c03w00(int LUT[24])
{
  LUT[0]  = 0x02;
  LUT[1]  = 0x03;
  LUT[2]  = 0x04;
  LUT[3]  = 0x04;
  LUT[4]  = 0x05;
  LUT[5]  = 0x06;
  LUT[6]  = 0x07;
  LUT[7]  = 0x08;
  LUT[8]  = 0x09;
  LUT[9]  = 0x09;
  LUT[10] = 0x0a;
  LUT[11] = 0x0b;
  LUT[12] = 0x0c;
  LUT[13] = 0x0c;
  LUT[14] = 0x0d;
  LUT[15] = 0x0e;
  LUT[16] = 0x0e;
  LUT[17] = 0x0f;
  LUT[18] = 0x0f;
  LUT[19] = 0x11;
  LUT[20] = 0x12;
  LUT[21] = 0x13;
  LUT[22] = 0x13;
  LUT[23] = 0x14;

  return;
}

void lutset_6s_c04w00(int LUT[24])
{
  LUT[0]  = 0xfe;
  LUT[1]  = 0xff;
  LUT[2]  = 0xff;
  LUT[3]  = 0x00;
  LUT[4]  = 0x01;
  LUT[5]  = 0x02;
  LUT[6]  = 0x03;
  LUT[7]  = 0x05;
  LUT[8]  = 0x06;
  LUT[9]  = 0x08;
  LUT[10] = 0x09;
  LUT[11] = 0x0b;
  LUT[12] = 0x0c;
  LUT[13] = 0x0e;
  LUT[14] = 0x0f;
  LUT[15] = 0x10;
  LUT[16] = 0x12;
  LUT[17] = 0x13;
  LUT[18] = 0x14;
  LUT[19] = 0x15;
  LUT[20] = 0x17;
  LUT[21] = 0x16;
  LUT[22] = 0x18;
  LUT[23] = 0x16;

  return;
}

void lutset_6s_c05w00(int LUT[24])
{
  LUT[0]  = 0xfa;
  LUT[1]  = 0xfa;
  LUT[2]  = 0xfa;
  LUT[3]  = 0xfb;
  LUT[4]  = 0xfc;
  LUT[5]  = 0xfd;
  LUT[6]  = 0xff;
  LUT[7]  = 0x00;
  LUT[8]  = 0x02;
  LUT[9]  = 0x04;
  LUT[10] = 0x06;
  LUT[11] = 0x09;
  LUT[12] = 0x0c;
  LUT[13] = 0x0e;
  LUT[14] = 0x11;
  LUT[15] = 0x11;
  LUT[16] = 0x16;
  LUT[17] = 0x18;
  LUT[18] = 0x1a;
  LUT[19] = 0x1c;
  LUT[20] = 0x1e;
  LUT[21] = 0x1f;
  LUT[22] = 0x1e;
  LUT[23] = 0x1e;

  return;
}

void lutset_6s_c06w00(int LUT[24])
{
  LUT[0]  = 0xfb;
  LUT[1]  = 0xf9;
  LUT[2]  = 0xf8;
  LUT[3]  = 0xf8;
  LUT[4]  = 0xf7;
  LUT[5]  = 0xf8;
  LUT[6]  = 0xf9;
  LUT[7]  = 0xfb;
  LUT[8]  = 0xfc;
  LUT[9]  = 0xff;
  LUT[10] = 0x01;
  LUT[11] = 0x04;
  LUT[12] = 0x08;
  LUT[13] = 0x0c;
  LUT[14] = 0x10;
  LUT[15] = 0x13;
  LUT[16] = 0x18;
  LUT[17] = 0x1d;
  LUT[18] = 0x21;
  LUT[19] = 0x24;
  LUT[20] = 0x27;
  LUT[21] = 0x27;
  LUT[22] = 0x28;
  LUT[23] = 0x24;

  return;
}

void lutset_6s_c07w00(int LUT[24])
{
  LUT[0]  = 0x00;
  LUT[1]  = 0xff;
  LUT[2]  = 0xfc;
  LUT[3]  = 0xfa;
  LUT[4]  = 0xf8;
  LUT[5]  = 0xf7;
  LUT[6]  = 0xf6;
  LUT[7]  = 0xf6;
  LUT[8]  = 0xf7;
  LUT[9]  = 0xf8;
  LUT[10] = 0xfb;
  LUT[11] = 0xfe;
  LUT[12] = 0x01;
  LUT[13] = 0x06;
  LUT[14] = 0x0c;
  LUT[15] = 0x12;
  LUT[16] = 0x17;
  LUT[17] = 0x1d;
  LUT[18] = 0x23;
  LUT[19] = 0x28;
  LUT[20] = 0x2d;
  LUT[21] = 0x2f;
  LUT[22] = 0x30;
  LUT[23] = 0x30;

  return;
}

void lutset_6s_c08w00(int LUT[24])
{
  LUT[0]  = 0x05;
  LUT[1]  = 0x04;
  LUT[2]  = 0x02;
  LUT[3]  = 0x00;
  LUT[4]  = 0xfe;
  LUT[5]  = 0xfb;
  LUT[6]  = 0xf8;
  LUT[7]  = 0xf5;
  LUT[8]  = 0xf5;
  LUT[9]  = 0xf5;
  LUT[10] = 0xf6;
  LUT[11] = 0xf8;
  LUT[12] = 0xfc;
  LUT[13] = 0x00;
  LUT[14] = 0x05;
  LUT[15] = 0x0d;
  LUT[16] = 0x13;
  LUT[17] = 0x1a;
  LUT[18] = 0x21;
  LUT[19] = 0x28;
  LUT[20] = 0x2d;
  LUT[21] = 0x32;
  LUT[22] = 0x36;
  LUT[23] = 0x3c;

  return;
}

void lutset_6s_c09w00(int LUT[24])
{
  LUT[0]  = 0x06;
  LUT[1]  = 0x07;
  LUT[2]  = 0x06;
  LUT[3]  = 0x05;
  LUT[4]  = 0x03;
  LUT[5]  = 0x00;
  LUT[6]  = 0xfe;
  LUT[7]  = 0xfa;
  LUT[8]  = 0xf8;
  LUT[9]  = 0xf6;
  LUT[10] = 0xf5;
  LUT[11] = 0xf5;
  LUT[12] = 0xf7;
  LUT[13] = 0xfb;
  LUT[14] = 0x00;
  LUT[15] = 0x06;
  LUT[16] = 0x0e;
  LUT[17] = 0x16;
  LUT[18] = 0x1e;
  LUT[19] = 0x26;
  LUT[20] = 0x2d;
  LUT[21] = 0x32;
  LUT[22] = 0x36;
  LUT[23] = 0x40;

  return;
}

void lutset_6s_c10w00(int LUT[24])
{
  LUT[0]  = 0x02;
  LUT[1]  = 0x04;
  LUT[2]  = 0x06;
  LUT[3]  = 0x07;
  LUT[4]  = 0x07;
  LUT[5]  = 0x05;
  LUT[6]  = 0x03;
  LUT[7]  = 0x00;
  LUT[8]  = 0xfd;
  LUT[9]  = 0xf9;
  LUT[10] = 0xf6;
  LUT[11] = 0xf4;
  LUT[12] = 0xf4;
  LUT[13] = 0xf6;
  LUT[14] = 0xfa;
  LUT[15] = 0x00;
  LUT[16] = 0x08;
  LUT[17] = 0x11;
  LUT[18] = 0x1b;
  LUT[19] = 0x25;
  LUT[20] = 0x2e;
  LUT[21] = 0x37;
  LUT[22] = 0x3c;
  LUT[23] = 0x40;

  return;
}

void lutset_6s_c10w10(int LUT[24])
{
    LUT[0]   = 0x00;
    LUT[1]   = 0x00;
    LUT[2]  = 0x00;
    LUT[3]  = 0x01;
    LUT[4]  = 0x01;
    LUT[5]  = 0x01;
    LUT[6]  = 0x01;
    LUT[7]  = 0x00;
    LUT[8]  = 0xff;
    LUT[9]  = 0xfd;
    LUT[10]  = 0xfb;
    LUT[11]  = 0xf9;
    LUT[12]  = 0xf8;
    LUT[13]  = 0xf9;
    LUT[14]  = 0xfc;
    LUT[15]  = 0x00;
    LUT[16]  = 0x06;
    LUT[17]  = 0x0f;
    LUT[18]  = 0x1a;
    LUT[19]  = 0x26;
    LUT[20]  = 0x32;
    LUT[21]  = 0x3a;
    LUT[22]  = 0x3e;
    LUT[23]  = 0x40;

  return;
}

void lutset_6s_c10w15(int LUT[24])
{
    LUT[0]   = 0x00;
    LUT[1]  = 0x00;
    LUT[2]  = 0x00;
    LUT[3]  = 0x00;
    LUT[4]  = 0x00;
    LUT[5]  = 0x00;
    LUT[6]  = 0x00;
    LUT[7]  = 0x00;
    LUT[8]  = 0x00;
    LUT[9]  = 0xff;
    LUT[10]  = 0xfe;
    LUT[11]  = 0xfd;
    LUT[12]  = 0xfc;
    LUT[13]  = 0xfc;
    LUT[14]  = 0xfd;
    LUT[15]  = 0x00;
    LUT[16]  = 0x05;
    LUT[17]  = 0x0d;
    LUT[18]  = 0x18;
    LUT[19]  = 0x23;
    LUT[20]  = 0x2e;
    LUT[21]  = 0x38;
    LUT[22]  = 0x3e;
    LUT[23] = 0x40;

  return;
}


void calclut(float fratio, int LUT[24])
{
  if(fratio<0.4f)      lutset_6s_c03w00(LUT);
  else if(fratio<0.5f) lutset_6s_c04w00(LUT);
  else if(fratio<0.6f) lutset_6s_c05w00(LUT);
  else if(fratio<0.7f) lutset_6s_c06w00(LUT);
  else if(fratio<0.8f) lutset_6s_c07w00(LUT);
  else if(fratio<0.9f) lutset_6s_c08w00(LUT);
  else if(fratio<1.0f) lutset_6s_c09w00(LUT);
  else if(fratio<1.1f) lutset_6s_c10w00(LUT);
  else if(fratio<1.5f) lutset_6s_c10w10(LUT);
  else                lutset_6s_c10w15(LUT);

  return;
}


void SetLut_wplg(void)
{
  int    HLUT[24],VLUT[24];
  int   i;
  float  hscl,vscl,hwin,vwin;

  // +++++ HLUT +++++ //
  hscl = (double)HMIN/PS_PANEL_ACT_HW - 0.1f;
  hscl = (hscl>DEF_LUTCUT_MAX) ? DEF_LUTCUT_MAX : (hscl<DEF_LUTCUT_MIN) ? DEF_LUTCUT_MIN : hscl; // limit
  hwin = (hscl<1.0f) ? 0 : (hscl+1)*0.5f;
  hwin = (hwin>DEF_LUTWIN_MAX) ? DEF_LUTWIN_MAX : (hwin<DEF_LUTWIN_MIN) ? DEF_LUTWIN_MIN : hwin; // limit

  calclut(0.85f,HLUT);

  // +++++ VLUT +++++ //
  vscl = (double)VMIN/PS_PANEL_ACT_VW - 0.1f;
  vscl = (vscl>DEF_LUTCUT_MAX) ? DEF_LUTCUT_MAX : (vscl<DEF_LUTCUT_MIN) ? DEF_LUTCUT_MIN : vscl; // limit
  vwin = (vscl<1.0f) ? 0 : (vscl+1)*0.5f;
  vwin = (vwin>DEF_LUTWIN_MAX) ? DEF_LUTWIN_MAX : (vwin<DEF_LUTWIN_MIN) ? DEF_LUTWIN_MIN : vwin; // limit

  calclut(0.85f,VLUT);

  // +++++ set LUT registers +++++ //
  for(i=23; i>=0; i--)
  {
    write_C381(B146_HLUT0CH1+(i<<8),HLUT[i]&0xff);  // ch1 HLUT0-23<<8
  }
  for(i=23; i>=0; i--)
  {
    write_C381(B146_VLUT0CH1+(i<<8),VLUT[i]&0xff);  // ch1 VLUT0-23
  }
  if((PS_PROCMODE==0)||(PS_PROCMODE==1))
  {
    for(i=23; i>=0; i--) write_C381(B151_HLUT0CH2+(i<<8),HLUT[i]&0xff);  // ch2 HLUT0-23
    for(i=23; i>=0; i--) write_C381(B151_VLUT0CH2+(i<<8),VLUT[i]&0xff);  // ch2 VLUT0-23
  }
  if(PS_PROCMODE==1)
  {
    for(i=23; i>=0; i--) write_C381(B156_HLUT0CH3+(i<<8),HLUT[i]&0xff);  // ch3 HLUT0-23
    for(i=23; i>=0; i--) write_C381(B156_VLUT0CH3+(i<<8),VLUT[i]&0xff);  // ch3 VLUT0-23
    for(i=23; i>=0; i--) write_C381(B161_HLUT0CH4+(i<<8),HLUT[i]&0xff);  // ch4 HLUT0-23
    for(i=23; i>=0; i--) write_C381(B161_VLUT0CH4+(i<<8),VLUT[i]&0xff);  // ch4 VLUT0-23
  }

  return;
}


void wait1_povs()
{
  int    w1osts;
	uint16_t  cnt;
	
  cnt=30000;
  write_C381(B5_INTEN_10 ,0x01);  // PO1VS
  do{
		cnt--;
    w1osts = (read_C381(B5_INTSTAT_10))&0x01;
  } while(w1osts!=0x01&&cnt>0);

  write_C381(B5_INTEN_10 ,0x00);

  return ;
}


void CalcHomography(str_wppos4c *p4c)
{
  float  FHW,FVW;
  float  hg_sx,hg_sy,hg_dx1,hg_dx2,hg_dy1,hg_dy2,hg_z,hg_g,hg_h;
  float  hg_sys0,hg_sys1,hg_sys2,hg_sys3,hg_sys4,hg_sys5,hg_sys6,hg_sys7;
  unsigned int  wplgct;
  unsigned int  dtct1,dtct2,dtct3,dtct4;
  int    dtsel;
  int    intarval;

  double P[3][3],Q[3][3];
  double det;

  FHW  = (float)PS_PANEL_ACT_HW;
  FVW   = (float)PS_PANEL_ACT_VW;
  intarval = (PM_WPDTMD==2) ? 16 : ((PM_WPDTMD==1) ? 64 : 32);

  HMIN = min((int)(p4c->tr_x - p4c->tl_x) , (int)(p4c->br_x - p4c->bl_x));
  VMIN = min((int)(p4c->bl_y - p4c->tl_y) , (int)(p4c->br_y - p4c->tr_y));

  // write_C381(B148_WTGMODECH1,0xa0);
  write_C381(B148_WTGMODECH1,0x20);

  //===== calculate homography coefficient =====//
  hg_sx  = (float)((p4c->tl_x - p4c->tr_x) + (p4c->br_x - p4c->bl_x));
  hg_sy  = (float)((p4c->tl_y - p4c->tr_y) + (p4c->br_y - p4c->bl_y));
  hg_dx1  = (float)(p4c->tr_x - p4c->br_x);
  hg_dx2  = (float)(p4c->bl_x - p4c->br_x);
  hg_dy1  = (float)(p4c->tr_y - p4c->br_y);
  hg_dy2  = (float)(p4c->bl_y - p4c->br_y);
  hg_z  = (hg_dx1*hg_dy2) - (hg_dy1*hg_dx2);
  hg_g  = ((hg_sx*hg_dy2) - (hg_sy*hg_dx2)) / hg_z;
  hg_h  = ((hg_sy*hg_dx1) - (hg_sx*hg_dy1)) / hg_z;

  hg_sys0  = p4c->tr_x*(1 + hg_g) - p4c->tl_x;
  hg_sys1  = p4c->bl_x*(1 + hg_h) - p4c->tl_x;
  hg_sys2  = p4c->tl_x;
  hg_sys3  = p4c->tr_y*(1 + hg_g) - p4c->tl_y;
  hg_sys4  = p4c->bl_y*(1 + hg_h) - p4c->tl_y;
  hg_sys5  = p4c->tl_y;
  hg_sys6 = hg_g;
  hg_sys7  = hg_h;

  P[0][0] = hg_sys0;
  P[0][1] = hg_sys1;
  P[0][2] = hg_sys2;
  P[1][0] = hg_sys3;
  P[1][1] = hg_sys4;
  P[1][2] = hg_sys5;
  P[2][0] = hg_sys6;
  P[2][1] = hg_sys7;
  P[2][2] = 1;

  det=P[0][0]*P[1][1]*P[2][2]
      +P[1][0]*P[2][1]*P[0][2]
      +P[2][0]*P[0][1]*P[1][2]
      -P[0][0]*P[2][1]*P[1][2]
      -P[2][0]*P[1][1]*P[0][2]
      -P[1][0]*P[0][1]*P[2][2];

  Q[0][0]=(P[1][1]*P[2][2]-P[1][2]*P[2][1])/det * FHW;
  Q[1][0]=(P[1][2]*P[2][0]-P[1][0]*P[2][2])/det * FVW;
  Q[2][0]=(P[1][0]*P[2][1]-P[1][1]*P[2][0])/det;

  Q[0][1]=(P[0][2]*P[2][1]-P[0][1]*P[2][2])/det * FHW;
  Q[1][1]=(P[0][0]*P[2][2]-P[0][2]*P[2][0])/det * FVW;
  Q[2][1]=(P[0][1]*P[2][0]-P[0][0]*P[2][1])/det;

  Q[0][2]=(P[0][1]*P[1][2]-P[0][2]*P[1][1])/det * FHW;
  Q[1][2]=(P[0][2]*P[1][0]-P[0][0]*P[1][2])/det * FVW;
  Q[2][2]=(P[0][0]*P[1][1]-P[0][1]*P[1][0])/det;

  if((FHW>=4128)|( (int)(FHW/intarval)*intarval == (int)(FHW) )){
    write_C381(B148_WTGACTHWCH1,(int)(FHW/intarval));
  }else{
    write_C381(B148_WTGACTHWCH1,(int)(FHW/intarval)+1);
  }
  if((FVW>=4384)|( (int)(FVW/intarval)*intarval == (int)(FVW) )){
    write_C381(B148_WTGACTVWCH1,(int)(FVW/intarval));
  }else{
    write_C381(B148_WTGACTVWCH1,(int)(FVW/intarval)+1);
  }

  write_C381(B148_WTGACH1,(int)(Q[0][0]*pow(2.0f,19)+0.5f)&0xffffffff);//
  write_C381(B148_WTGBCH1,(int)(Q[0][1]*pow(2.0f,19)+0.5f)&0xffffffff);

  write_C381(B148_WTGDCH1,(int)(Q[1][0]*pow(2.0f,19)+0.5f)&0xffffffff);
  write_C381(B148_WTGECH1,(int)(Q[1][1]*pow(2.0f,19)+0.5f)&0xffffffff);//

  write_C381(B148_WTGGCH1,(int)(Q[2][0]*pow(2.0f,26)+0.5f)&0xffffffff);
  write_C381(B148_WTGHCH1,(int)(Q[2][1]*pow(2.0f,26)+0.5f)&0xffffffff);//

  if(PM_WPDTMD==0)
  {
    write_C381(B148_WTGCCH1,(int)(Q[0][2]*pow(2.0f, 9)+0.5f)&0xffffffff);
    write_C381(B148_WTGFCH1,(int)(Q[1][2]*pow(2.0f, 9)+0.5f)&0xffffffff);
    write_C381(B148_WTGICH1,(int)(Q[2][2]*pow(2.0f,19)+0.5f)&0xffffffff);
  }
  else if(PM_WPDTMD==1)
  {
    write_C381(B148_WTGCCH1,(int)(Q[0][2]*pow(2.0f, 8)+0.5f)&0xffffffff);
    write_C381(B148_WTGFCH1,(int)(Q[1][2]*pow(2.0f, 8)+0.5f)&0xffffffff);
    write_C381(B148_WTGICH1,(int)(Q[2][2]*pow(2.0f,18)+0.5f)&0xffffffff);
  }else{
    write_C381(B148_WTGCCH1,(int)(Q[0][2]*pow(2.0f,10)+0.5f)&0xffffffff);
    write_C381(B148_WTGFCH1,(int)(Q[1][2]*pow(2.0f,10)+0.5f)&0xffffffff);
    write_C381(B148_WTGICH1,(int)(Q[2][2]*pow(2.0f,20)+0.5f)&0xffffffff);
  }
/////////////////////////////////////////////////////////////////////////////////////
  dtct1 = ((read_C381(B145_DTCTCH1))&0xffff);
  dtsel = (dtct1>>4)&0x03;

  if(dtsel==0)
  {
    wplgct = (0x0021) | (PM_WPDTMD<<8);
  }else{
    wplgct = (0x0011) | (PM_WPDTMD<<8);
  }

  wplgct = (wplgct | ((PS_PROCMODE&0x03)<<10));
  write_C381(B148_WTGCTCH1,wplgct&0xff00);
 write_C381(B148_WTGCTCH1,wplgct);

  do{
    wplgct = read_C381(B148_WTGCTCH1)&0xffff;
  }while((wplgct&0x0002) == 0x0002 );

  write_C381(B148_WTGCTCH1,wplgct&0xff00);

  if(dtsel==0)
  {
    dtct1 = (dtct1&0xd800) | (0x2010) | (PM_WPDTMD);
  }else{
    dtct1 = (dtct1&0xd800) | (0x2000) | (PM_WPDTMD);
  }

  dtct1 = (dtct1 | ((PS_PROCMODE&0x03)<<6));
  dtct2 = (PS_PROCMODE==0)||(PS_PROCMODE==1) ? dtct1 : 0x0000;
  dtct3 = (PS_PROCMODE==1) ? dtct1 : 0x0000;
  dtct4 = dtct3;

  write_C381(B0_RTCT6,0xff);
  write_C381(B145_DTCTCH1,dtct1);//
  write_C381(B150_DTCTCH2,dtct2);//
  write_C381(B155_DTCTCH3,dtct3);
  write_C381(B160_DTCTCH4,dtct4);
  SetLut_wplg();

  write_C381(B0_RTCT6,0xdd);//有问题 0xdd -> 0xde
  wait1_povs(); // wait 1-POVS
  write_C381(B0_RTCT6,0x00);//有问题 0x00 -> 0x33/0x34

  return;
}

int SetWpKey4c( str_wppos4c *p4c)
{
  CalcHomography(p4c);
  return 0;
}

int check_curpos( str_wppos4c *p4c)
{
  if((p4c->tl_x >  (PS_PANEL_ACT_HW>>1) - DEF_CURSIZE)||(p4c->tl_x <  0)||
     (p4c->tl_y >= p4c->bl_y)||(p4c->tl_y <  0)||
     (p4c->tr_x >  PS_PANEL_ACT_HW)||(p4c->tr_x < ((PS_PANEL_ACT_HW>>1) + DEF_CURSIZE))||
     (p4c->tr_y >= p4c->br_y)||(p4c->tr_y <  0)||
     (p4c->bl_x >  (PS_PANEL_ACT_HW>>1) - DEF_CURSIZE)||(p4c->bl_x <  0)||
     (p4c->bl_y >  PS_PANEL_ACT_VW)||(p4c->bl_y <= p4c->tl_y)||
     (p4c->br_x >  PS_PANEL_ACT_HW)||(p4c->br_x < ((PS_PANEL_ACT_HW>>1) + DEF_CURSIZE))||
     (p4c->br_y >  PS_PANEL_ACT_VW)||(p4c->br_y <= p4c->tr_y)) return DEF_WPERR_CURPOS;

  return 0;
}

void move_cursor( str_wppos4c *p4c)
{
  return;
}

int SetCurPos( str_wppos4c *p4c)
{
  int poserr=0;

  poserr = check_curpos(p4c);

  if(!poserr) move_cursor(p4c);

  return (poserr);
}



/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
