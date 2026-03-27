#include "main.h"
#include "c381.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "set.h"
#include "rt_param.h"
//================================
// c381 spi declaraion, for migration 
#include "spi.h"
#define SpiHandle hspi2
//================================


GPIO_InitTypeDef GPIO_InitStruct;


extern int16_t g_current_bank;
int PM_WPGDSELX;
int PM_WPGDSELY;

uint8_t g_ieb_board_num = 1;
uint8_t PS_SPLIT_TYPE;
int16_t WP_Hkeystone = 0; // LEO 20180524 C381
int16_t WP_Vkeystone = 0;
int16_t WP_HkeystoneOld = 50; // LEO 20180524 C381
int16_t WP_VkeystoneOld = 50;
int16_t WP_HkeystoneStatus = 0; // LEO 20180524 C381
int16_t WP_VkeystoneStatus = 0;
int16_t CursorMode;   // LEO
int16_t Movement;     // LEO
int16_t CursorSel_LR; // LEO
int16_t CursorSel_UD; // LEO
int16_t Wraping_LR;   // LEO
int16_t Wraping_UD;   // LEO
int16_t Reset_Wrap;   // LEO
int16_t Save_Wrap;    // LEO
int16_t Load_Wrap;    // LEO

_Bool C381ModeChange = TRUE;

str_wppos4c PM_WPPOS4C = {
    .tl_x = 0,
    .tl_y = 0,
    .tr_x = PS_PANEL_ACT_HW,
    .tr_y = 0,
    .bl_x = 0,
    .bl_y = PS_PANEL_ACT_VW,
    .br_x = PS_PANEL_ACT_HW,
    .br_y = PS_PANEL_ACT_VW,
};
str_wppos4c WPPOS4C_OLD;

uint8_t HMIN;
uint8_t VMIN;
uint8_t PM_WPDTMD;

void C381_GPIO_Init(void)
{
  # if(Projector_Model == LP92SLB || Projector_Model == NP72BN)
  # else
  GPIO_InitTypeDef GPIO_InitStructure;
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStructure.Pin = C381_INIT_GPIO_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(C381_INIT_GPIO_PORT, &GPIO_InitStructure);
  # endif
}

void C381_Reset(void)
{
  HAL_GPIO_WritePin(C381_RST_GPIO_PORT, C381_RST_GPIO_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(C381_RST_GPIO_PORT, C381_RST_GPIO_PIN, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(C381_RST_GPIO_PORT, C381_RST_GPIO_PIN, GPIO_PIN_SET);
  HAL_Delay(100);
}


uint8_t C381_ReadReg(uint8_t addr)
{
  uint8_t pData[1];
  uint8_t Rdata;
  uint8_t SPIReadReceive[1];
  pData[0] = addr & C381_READ_MASK; // MSB should be 0 when read

  C381_CS_LOW();
  //  HAL_SPI_Receive_IT(&SpiHandle, (uint8_t *)SPIReadReceive, 1);

  HAL_SPI_Transmit(&SpiHandle, (uint8_t *)pData, 1, 5000);
  //  HAL_Delay(1);
  HAL_SPI_Receive(&SpiHandle, (uint8_t *)SPIReadReceive, 1, 5000);

  Rdata = SPIReadReceive[0]; // leo 20180313
  C381_CS_HIGH();
  return (Rdata); // leo 20180313
}


unsigned int read_C381(unsigned int ad)
{
  int bank, addr, nbyte;
  unsigned int dt;
  unsigned int rdt;

  dt = 0;
  bank = (ad >> 16) & 0xff;
  addr = (ad >> 8) & 0xff;
  nbyte = ad & 0xff;

  if ((bank != g_current_bank) && (addr >= 0x08))
  {
    g_current_bank = bank;
    C381_WriteReg(0x80, (unsigned char)g_current_bank);
  }

  while (nbyte > 0)
  {
    nbyte--;
    dt = (dt << 8);
    rdt = (unsigned int)C381_ReadReg((unsigned char)((addr + nbyte) & 0x7f));
    dt = dt + rdt;
  }

  return dt;
}

// wirte byte to c381
void C381_WriteReg(uint8_t addr, uint8_t byte)
{
  uint8_t tmp[1];
  uint8_t pData[1];
  tmp[0] = addr | C381_WRITE_MASK; // MSB should be 1 when write
  pData[0] = byte;
  C381_CS_LOW();
  int ret = 0;
  ret = HAL_SPI_Transmit(&SpiHandle, (uint8_t *)tmp, 1, 5000);
  HAL_SPI_Transmit(&SpiHandle, (uint8_t *)pData, 1, 5000);
  // HAL_Delay(1);   
  if (ret != HAL_OK)
  {
    printf("C381_WriteReg error\n");
  }
  C381_CS_HIGH();
}

void write_C381(unsigned int ad, unsigned int dt)
{
  int bank, addr, nbyte;
  //-----------------------------------------------------------------------------
  bank = (ad >> 16) & 0xff;
  addr = (ad >> 8) & 0xff;
  nbyte = ad & 0xff;
  //-----------------------------------------------------------------------------
  if (addr == 0x00)
  {
    g_current_bank = dt;
  }
  else if ((bank != g_current_bank) && (addr >= 0x08))
  { // Bank Update
    g_current_bank = bank;
    // write_SPI(0x80, (unsigned char)g_current_bank, 0x00);
    C381_WriteReg(0x80, (uint8_t)g_current_bank);
  }
  //-----------------------------------------------------------------------------
  while (nbyte > 0)
  {
    // write_SPI((unsigned char)(addr | 0x80), (unsigned char)(dt & 0xff), 0x00);
    C381_WriteReg((unsigned char)(addr | 0x80), (unsigned char)(dt & 0xff));
    addr++;
    dt = (dt >> 8);
    nbyte--;
  }

  return;
}

void C381_Write_Bank_Reg(uint8_t bank, uint8_t addr, uint8_t data)
{
  uint8_t bank_tmp;

  bank_tmp = bank & 0x1f;
  C381_WriteReg(0x00, bank_tmp); // sel which bank to write

  C381_WriteReg(addr, data);
}

void C381_init_2712x1528(void)
{
  // MCLK_FREQ   27000000
  // PO1CLK_FREQ	27000000
  // PO2CLK_FREQ	27000000
  HAL_Delay(12);

  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  write_C381(BN_RSTCT, 0x000001);
  HAL_Delay(100);

  write_C381(B5_GIOPDCT, 0x3fff);

  write_C381(BN_DDR3CT, 0x00);
  write_C381(BN_MEMCT, 0x00);
  write_C381(B1_DDR3DLLCT, 0x00);
  HAL_Delay(1);

  write_C381(B1_DDR3CT2, 0x30);
  write_C381(B1_DDR3GDS, 0x33333333);
  write_C381(B1_DDR3MSDLY, 0x66664455);
  write_C381(B1_DDR3PHYODTMD, 0x22222222);
  write_C381(B1_DDR3RONMD_0, 0x77);
  write_C381(B1_DDR3RONMD_1, 0x77);
  write_C381(B1_DDR3RONMD_2, 0x77);
  write_C381(B1_DDR3RONMD_3, 0x77);
  write_C381(B1_DDR3RONMD_4, 0x07);
  write_C381(B1_DDR3OCDDIP, 0x3f);
  write_C381(B1_DDR3OCDDIN, 0x3f);
  write_C381(B1_DDR3LVLCT, 0x00);
  write_C381(B1_DDR3WRLVLPHCT1, 0x26);
  write_C381(B1_DDR3WRLVLPHCT2, 0x26);
  write_C381(B1_DDR3WRLVLPHCT3, 0x24);
  write_C381(B1_DDR3WRLVLPHCT4, 0x24);
  write_C381(B1_DDR3WRLVLPHCT5, 0x45);
  write_C381(B1_DDR3WRLVLPHCT6, 0x45);
  write_C381(B1_DDR3WRLVLPHCT7, 0x62);
  write_C381(B1_DDR3WRLVLPHCT8, 0x62);
  write_C381(B1_DDR3VREFCT, 0x09);
  write_C381(B1_DDR3DLLCT, 0x0f);
  write_C381(B1_DDR3ODTCT, 0x11);
  write_C381(B1_DDR3ZQCT, 0x01);
  write_C381(B1_DDR3WRTMGCT, 0x88888888);
  write_C381(B1_DDR3RDTMGCT1, 0x77);
  write_C381(B1_DDR3RDTMGCT2, 0x77);
  write_C381(B1_DDR3RDTMGCT3, 0x77);
  write_C381(B1_DDR3RDTMGCT4, 0x77);
  write_C381(B1_DDR3RDTMGCT5, 0x77);
  write_C381(B1_DDR3RDTMGCT6, 0x77);
  write_C381(B1_DDR3RDTMGCT7, 0x77);
  write_C381(B1_DDR3RDTMGCT8, 0x77);
  write_C381(B1_DDR3CMDTMGCT, 0x07);
  write_C381(B1_DDR3CKTMGCT1, 0x67);
  write_C381(B1_DDR3CKTMGCT2, 0x65);
  write_C381(B1_DDR3CKTMGCT3, 0x62);
  write_C381(B1_DDR3CKTMGCT4, 0x60);
  write_C381(B1_DDR3CKTMGCT5, 0x6a);
  write_C381(B1_DDR3CKTMGCT6, 0x67);
  write_C381(B1_DDR3CKTMGCT7, 0x64);
  write_C381(B1_DDR3CKTMGCT8, 0x61);
  write_C381(B1_DDR3CKTMGCTA, 0x6c);
  write_C381(B1_DDR3CKDUTYSEL, 0x08);
  write_C381(B1_DDR3DQRDDLSEL1, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL2, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL3, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL4, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL5, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL6, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL7, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL8, 0x924924);
  write_C381(B1_DDR3DQWRDLSEL1, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL2, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL3, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL4, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL5, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL6, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL7, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL8, 0x6db6db);
  write_C381(B1_DDR3DQMDLSEL, 0x33333333);
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0x88); // 0x88
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0xc8); // 0xc8
  HAL_Delay(1);
  write_C381(BN_MEMCT, 0x61);

  HAL_Delay(1);
  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  // VbyOne-RXCH1setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0e);         //[0]ResetEnableV-by-OneRX1leV-by-OneRX1
  write_C381(B2_VBOCT, 0x0f);         //[0]ResetDisableV-by-OneRX1bleV-by-OneRX1
  write_C381(B2_VBORX1_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX1_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX1_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps
  HAL_Delay(100);

  // VbyOne-RXCH2setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0d);         //[1]ResetEnableV-by-OneRX2leV-by-OneRX2
  write_C381(B2_VBOCT, 0x0f);         //[1]ResetDisableV-by-OneRX2bleV-by-OneRX2
  write_C381(B2_VBORX2_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX2_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX2_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps

  HAL_Delay(100);

  write_C381(B0_MCT1, 0x10);
  write_C381(B0_MCT2, 0x10);
  write_C381(B0_ICHSEL1, 0x08); // leo0ch//
  write_C381(B0_ICHSEL2, 0x0a); // leo0eh//
  write_C381(B0_PI1SWP, 0x04);
  write_C381(B0_PI2SWP, 0x04);
  write_C381(B0_PI3SWP, 0x04);
  write_C381(B0_PI4SWP, 0x04);
  write_C381(B0_VI1CT, 0x0404);
  write_C381(B0_VI2CT, 0x0404);
  write_C381(B0_VIDSKW, 0x0202);

  write_C381(B3_IDUALCTCH12, 0x02);    // 2-phase input, four pixels at a time
  write_C381(B3_IDACTHSTCH12, 0x0062); // ch1&2 start: (back-porch + pulse-width)/2 = (164+32)/2=62h modified1226
  write_C381(B3_IDACTHWCH12, 0x0A98);  // ch1&2 2712  modified1226
  write_C381(B3_OIDUALCTCH12, 0x005);  // ch1&2 2-phase output enable?? area select AOI0
  write_C381(B3_ODACTHWCH12, 0x054c);  // ch1&2 2-phase output ACT horizontal width 1356
  write_C381(B3_OSYCTTX1, 0x19);       // CMOS ACT select OAOI0 ;V-by-One? TX sync signal polarity high; V-by-One?TX ACT select OAOI0;
  write_C381(B3_OHSCTTX1, 0x16);       // width of horizontal sync signal output HS (clock / 2 - 1 )  16
  write_C381(B3_OVSCTTX1, 0x16);       // width of the vertical sync signal output POVS at image output port 22
  write_C381(B3_VBOCHSEL, 0x51405140);
  write_C381(B3_LANESELTX1, 0x73625140);
  write_C381(B6_VISYPOLCTRL, 0x00003333);

  // CH1 ARGS
  write_C381(B16_ISYCTCH1, 0x01);     // Image input port field signal control  Cycle between 0 and 3.
  write_C381(B16_DIFCTCH1, 0x81);     // Mask image input other than PIACT to 0. ;Input port horizontal sync signal measurement CLK select TICLK. (TICLK=PO1PLLCT created with CLK).
  write_C381(B16_IACTHSTCH1, 0x005b); // Input ACT Horizontal Start: B3_IDACTHSTCH12(0x0062)-offset(7)=0x5b modified1226
  write_C381(B16_IACTHWCH1, 0x054c);  // Input ACT Horizontal Width 1356 0x054c modified1226
  write_C381(B16_IACTVSTCH1, 0x23);   // Input ACT Vertical Start 35
  write_C381(B16_IACTVWCH1, 0x05F8);  // Input ACT Vertical Width 1528
  write_C381(B16_PIH0CH1, 0x0000);    // auto IACTHST/IACTHW/IACTVST/IACTVW OFF
  // CH2 ARGS
  write_C381(B32_ISYCTCH2, 0x08);
  write_C381(B32_DIFCTCH2, 0x81);
  write_C381(B32_IACTHSTCH2, 0x005b); // Input ACT Horizontal Start B3_IDACTHSTCH12(0x0062)-offset(7)=0x5b modified1226
  write_C381(B32_IACTHWCH2, 0x054c);  // Input ACT Horizontal Width 1356 0x054c modified1226
  write_C381(B32_IACTVSTCH2, 0x23);   // Input ACT Vertical Start 35
  write_C381(B32_IACTVWCH2, 0x05F8);  // Input ACT Vertical Width 1528
  write_C381(B32_PIH0CH2, 0x0000);    // auto IACTHST/IACTHW/IACTVST/IACTVW OFF

  // CH1 ARGS
  write_C381(B144_OFLDOUTCTCH1, 0x02);   // Output field signal from Warping block output side
  write_C381(B144_MCT2CH1, 0x11);        // Output port field  input/output enable
  write_C381(B144_OSYCTCH1, 0x0003);     // Image output port sync signal generation enable ; Use the input VS signal to reset output VS signal.
  write_C381(B144_OHCYCLCH1, 0x063E);    // HS output interval;   POCLK - 2   Htotal/2 -2 = 3200/2-2 = 1598
  write_C381(B144_OVCYCLCH1, 0x06b8);    // VS output interval;   lines - 2  1563*1.1=1720
  write_C381(B144_SYRDLYCH1, 0x0008);    // sync reset delay, modified1226
  write_C381(B144_OVCYCLMINCH1, 0x05b7); // Output Vertical Cycle Minimum Limit   1463
  write_C381(B144_FLDDLYCH1, 0x0009);    // Field Delay, modified1226
  write_C381(B144_OAOI0HSTCH1, 0x0066);  // B3_IDACTHSTCH12(0x0062)-offset(-4)=0x66 modified1226
  write_C381(B144_OAOI0HENDCH1, 0x05b2); // Output AOI0 Horizontal End. modified1226
  write_C381(B144_OAOI0VSTCH1, 0x015);   // Output AOI0 Vertical direction  Start
  write_C381(B144_OAOI0VENDCH1, 0x060D); // Output AOI0 Vertical direction  End
  write_C381(B144_OACTHSTCH1, 0x0066);   // B3_IDACTHSTCH12(0x0062)-offset(-4)=0x66 modified1226
  write_C381(B144_OACTHWCH1, 0x054c);    // Output ACT horizontal direction width 1356
  write_C381(B144_OACTVSTCH1, 0x0015);   // Output ACT vertical direction start 	21
  write_C381(B144_OACTVWCH1, 0x05f8);    // Output ACT vertical direction width  	1528
  write_C381(B145_DTCTCH1, 0x2008);      // Distortion correction table , create through table mode ; Overflow correction enable
  write_C381(B145_IACTHW3CH1, 0x054c);   // Input port ACT horizontal direction width3 1356
  write_C381(B145_IACTVW3CH1, 0x05f8);   // Input port ACT vertical direction width3 1528

  write_C381(B146_HLUT23CH1, 0x40);
  write_C381(B146_VLUT23CH1, 0x40);
  write_C381(B148_OSFLD0CH1, 0x0000);     // modified1226
  write_C381(B148_OSFLD1CH1, 0x0800);     // modified1226
  write_C381(B148_OSFLD2CH1, 0x1000);     // modified1226
  write_C381(B148_OSFLD3CH1, 0x1800);     // modified1226
  write_C381(B148_ISFLD0CH1, 0x00000000); // Input field 0-3 memory write start address
  write_C381(B148_ISFLD1CH1, 0x04000000);
  write_C381(B148_ISFLD2CH1, 0x08000000);
  write_C381(B148_ISFLD3CH1, 0x0c000000);
  write_C381(B148_OMWICH1, 0x00); // Memory read linefeed width 4096 pix
  write_C381(B148_IMWICH1, 0x00); // Memory write linefeed width 4096 pix

  // CH2 ARGS
  write_C381(B149_OFLDOUTCTCH2, 0x02);
  write_C381(B149_MCT2CH2, 0x11);
  write_C381(B149_OSYCTCH2, 0x1013);     // Image output port field signal select O1FLD
  write_C381(B149_OHCYCLCH2, 0x063E);    // HS output interval; 3200/2-2 = 1598
  write_C381(B149_OVCYCLCH2, 0x06b8);    // VS output interval; 1563*1.1 = 1720
  write_C381(B149_SYRDLYCH2, 0x0008);    // sync reset delay, modified1226
  write_C381(B149_OVCYCLMINCH2, 0x05b7); // Output Vertical Cycle Minimum Limit    1463
  write_C381(B149_FLDDLYCH2, 0x0009);    // Field Delay,modified1226
  write_C381(B149_OAOI0HSTCH2, 0x0066);  // 0x0062-offset(-4)=0x66 modified1226
  write_C381(B149_OAOI0HENDCH2, 0x05b2); // Output AOI0 Horizontal End. modified1226
  write_C381(B149_OAOI0VSTCH2, 0x0015);  // Output AOI0 Vertical direction  Start
  write_C381(B149_OAOI0VENDCH2, 0x060D); // Output AOI0 Vertical direction  End
  write_C381(B149_OACTHSTCH2, 0x0066);   // B3_IDACTHSTCH12(0x0062)-offset(-4)=0x66 modified1226
  write_C381(B149_OACTHWCH2, 0x054c);    // Output ACT horizontal direction width 1356
  write_C381(B149_OACTVSTCH2, 0x0015);   // Output ACT vertical direction start  21
  write_C381(B149_OACTVWCH2, 0x05f8);    // Output ACT vertical direction width 1528
  write_C381(B150_DTCTCH2, 0x2008);      // Distortion correction table , create through table mode ; Overflow correction enable
  write_C381(B150_IACTHW3CH2, 0x054c);   // Input port ACT horizontal direction width3 1356
  write_C381(B150_IACTVW3CH2, 0x05f8);   // Input port ACT vertical direction width3  1528

  write_C381(B151_HLUT23CH2, 0x40);
  write_C381(B151_VLUT23CH2, 0x40);
  write_C381(B153_OSFLD0CH2, 0x0000);     // modified1226
  write_C381(B153_OSFLD1CH2, 0x0800);     // modified1226
  write_C381(B153_OSFLD2CH2, 0x1000);     // modified1226
  write_C381(B153_OSFLD3CH2, 0x1800);     // modified1226
  write_C381(B153_ISFLD0CH2, 0x00005500); // Input field 0-3 memory write start address
  write_C381(B153_ISFLD1CH2, 0x04005500);
  write_C381(B153_ISFLD2CH2, 0x08005500);
  write_C381(B153_ISFLD3CH2, 0x0c005500);
  write_C381(B153_OMWICH2, 0x00); // Memory read linefeed width 4096 pix
  write_C381(B153_IMWICH2, 0x00); // Memory write linefeed width 4096 pix

  write_C381(B2_VBOCT, 0x0b);
  write_C381(B2_VBOCT, 0x0f);           // Vbyone read select RX11 , TX1/TX2/RX1/RX2 reset
  write_C381(B2_VBOTX1_PWR, 0x00);      // Vbyone TX1 power down
  write_C381(B2_VBOTX1MD, 0x0011);      // Vbyone TX1 4byte mode , RGB/YUV_444 30bit
  write_C381(B2_VBOTX1_LANECTRL, 0xff); // LANE 0-7 enable setting
  write_C381(B2_VBOTX1_FMT, 0x11);      // TX1 RGB444/YCbCr444 30bit
  write_C381(B2_VBOTX1_CTR1, 0x4c);     // Post DIV control  /1 (40 MHz < FREF < 133.33 MHz);  PMA PLL normal ;
  write_C381(B2_VBOTX1_PWR, 0x01);      // Vbyone TX1 power on
  write_C381(B2_VBOTX1CTL, 0x09);       // 4 pixels parallel;output en
  write_C381(B2_VBOTX1MD, 0x8011);      // TX1 Transfer start

  write_C381(B0_MCT1, 0x11);     // Controls image input and output command en
  write_C381(B0_MCT2, 0x11);     // Controls image input and output command en
  write_C381(B0_STBYCT, 0xfcfc); // Input CH1/2 clock enable; I/O CH1/2 clock enable
}

void C381_init_3840x2160(void)
{
  // MCLK_FREQ   27000000
  // PO1CLK_FREQ	27000000
  // PO2CLK_FREQ	27000000
  HAL_Delay(12);

  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  write_C381(BN_RSTCT, 0x000001);
  //HAL_Delay(100);
  HAL_Delay(2000);
  write_C381(B5_GIOPDCT, 0x3fff);

  write_C381(BN_DDR3CT, 0x00);
  write_C381(BN_MEMCT, 0x00);
  write_C381(B1_DDR3DLLCT, 0x00);
  HAL_Delay(1);

  write_C381(B1_DDR3CT2, 0x30);
  write_C381(B1_DDR3GDS, 0x33333333);
  write_C381(B1_DDR3MSDLY, 0x66664455);
  write_C381(B1_DDR3PHYODTMD, 0x22222222);
  write_C381(B1_DDR3RONMD_0, 0x77);
  write_C381(B1_DDR3RONMD_1, 0x77);
  write_C381(B1_DDR3RONMD_2, 0x77);
  write_C381(B1_DDR3RONMD_3, 0x77);
  write_C381(B1_DDR3RONMD_4, 0x07);
  write_C381(B1_DDR3OCDDIP, 0x3f);
  write_C381(B1_DDR3OCDDIN, 0x3f);
  write_C381(B1_DDR3LVLCT, 0x00);
  write_C381(B1_DDR3WRLVLPHCT1, 0x26);
  write_C381(B1_DDR3WRLVLPHCT2, 0x26);
  write_C381(B1_DDR3WRLVLPHCT3, 0x24);
  write_C381(B1_DDR3WRLVLPHCT4, 0x24);
  write_C381(B1_DDR3WRLVLPHCT5, 0x45);
  write_C381(B1_DDR3WRLVLPHCT6, 0x45);
  write_C381(B1_DDR3WRLVLPHCT7, 0x62);
  write_C381(B1_DDR3WRLVLPHCT8, 0x62);
  write_C381(B1_DDR3VREFCT, 0x09);
  write_C381(B1_DDR3DLLCT, 0x0f);
  write_C381(B1_DDR3ODTCT, 0x11);
  write_C381(B1_DDR3ZQCT, 0x01);
  write_C381(B1_DDR3WRTMGCT, 0x88888888);
  write_C381(B1_DDR3RDTMGCT1, 0x77);
  write_C381(B1_DDR3RDTMGCT2, 0x77);
  write_C381(B1_DDR3RDTMGCT3, 0x77);
  write_C381(B1_DDR3RDTMGCT4, 0x77);
  write_C381(B1_DDR3RDTMGCT5, 0x77);
  write_C381(B1_DDR3RDTMGCT6, 0x77);
  write_C381(B1_DDR3RDTMGCT7, 0x77);
  write_C381(B1_DDR3RDTMGCT8, 0x77);
  write_C381(B1_DDR3CMDTMGCT, 0x07);
  write_C381(B1_DDR3CKTMGCT1, 0x67);
  write_C381(B1_DDR3CKTMGCT2, 0x65);
  write_C381(B1_DDR3CKTMGCT3, 0x62);
  write_C381(B1_DDR3CKTMGCT4, 0x60);
  write_C381(B1_DDR3CKTMGCT5, 0x6a);
  write_C381(B1_DDR3CKTMGCT6, 0x67);
  write_C381(B1_DDR3CKTMGCT7, 0x64);
  write_C381(B1_DDR3CKTMGCT8, 0x61);
  write_C381(B1_DDR3CKTMGCTA, 0x6c);
  write_C381(B1_DDR3CKDUTYSEL, 0x08);
  write_C381(B1_DDR3DQRDDLSEL1, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL2, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL3, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL4, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL5, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL6, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL7, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL8, 0x924924);
  write_C381(B1_DDR3DQWRDLSEL1, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL2, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL3, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL4, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL5, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL6, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL7, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL8, 0x6db6db);
  write_C381(B1_DDR3DQMDLSEL, 0x33333333);
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0x88);
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0xc8);
  HAL_Delay(1);
  write_C381(BN_MEMCT, 0x61);

  HAL_Delay(1);
  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);  // PLL 250-333 MHz
  write_C381(B0_PO1REFSEL, 0x43); // RX1LANE0CLK   Clock selected by PO1REFSEL2
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  // VbyOne-RXCH1setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0e);         //[0]ResetEnableV-by-OneRX1leV-by-OneRX1
  write_C381(B2_VBOCT, 0x0f);         //[0]ResetDisableV-by-OneRX1bleV-by-OneRX1
  write_C381(B2_VBORX1_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX1_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX1_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps
  HAL_Delay(100);

  // VbyOne-RXCH2setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0d);         //[1]ResetEnableV-by-OneRX2leV-by-OneRX2
  write_C381(B2_VBOCT, 0x0f);         //[1]ResetDisableV-by-OneRX2bleV-by-OneRX2
  write_C381(B2_VBORX2_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX2_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX2_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps

  HAL_Delay(100);

  write_C381(B0_MCT1, 0x10);
  write_C381(B0_MCT2, 0x10);
  write_C381(B0_ICHSEL1, 0x08); // leo0ch//
  write_C381(B0_ICHSEL2, 0x0a); // leo0eh//
  write_C381(B0_PI1SWP, 0x04);
  write_C381(B0_PI2SWP, 0x04);
  write_C381(B0_PI3SWP, 0x04);
  write_C381(B0_PI4SWP, 0x04);
  write_C381(B0_VI1CT, 0x0404);
  write_C381(B0_VI2CT, 0x0404);
  write_C381(B0_VIDSKW, 0x0202);

  write_C381(B3_IDUALCTCH12, 0x02);    // 2-phase input, four pixels at a time
  write_C381(B3_IDACTHSTCH12, 0x0120); // ch1&2  input image ACT area start  288
  write_C381(B3_IDACTHWCH12, 0x0f10);  // ch1&2 input image ACT area width 3856
  write_C381(B3_OIDUALCTCH12, 0x0005); // ch1&2 2-phase output enable?? area select AOI0
  write_C381(B3_ODACTHWCH12, 0x0780);  // ch1&2 2-phase output ACT horizontal width 1920
  write_C381(B3_OSYCTTX1, 0x19);       // CMOS ACT select OAOI0 ;V-by-One? TX sync signal polarity high; V-by-One?TX ACT select OAOI0;
  write_C381(B3_OHSCTTX1, 0x15);       // width of horizontal sync signal output HS  21
  write_C381(B3_OVSCTTX1, 0x09);       // width of the vertical sync signal output POVS at image output port 9
  write_C381(B3_VBOCHSEL, 0x51405140);
  write_C381(B3_LANESELTX1, 0x73625140);
  write_C381(B6_VISYPOLCTRL, 0x00003333);

  // CH1 ARGS
  write_C381(B16_ISYCTCH1, 0x01);     // Image input port field signal control  Cycle between 0 and 3.
  write_C381(B16_DIFCTCH1, 0x81);     // Mask image input other than PIACT to 0. ;Input port horizontal sync signal measurement CLK select TICLK. (TICLK=PO1PLLCT created with CLK).
  write_C381(B16_IACTHSTCH1, 0x0111); // Input ACT Horizontal Start 273
  write_C381(B16_IACTHWCH1, 0x0790);  // Input ACT Horizontal Width 1936
  write_C381(B16_IACTVSTCH1, 0x0021); // Input ACT Vertical Start 33
  write_C381(B16_IACTVWCH1, 0x0870);  // Input ACT Vertical Width 2160
  write_C381(B16_PIH0CH1, 0x0000);    // IACT setting based on DE signal automatic measurement CLOSE; IACTHW/VW setting for automatic measurement CLOSE;
  // CH2 ARGS
  write_C381(B32_ISYCTCH2, 0x08);
  write_C381(B32_DIFCTCH2, 0x81);
  write_C381(B32_IACTHSTCH2, 0x0109); // Input ACT Horizontal Start 265
  write_C381(B32_IACTHWCH2, 0x0780);  // Input ACT Horizontal Width 1920
  write_C381(B32_IACTVSTCH2, 0x0021); // Input ACT Vertical Start 33
  write_C381(B32_IACTVWCH2, 0x0870);  // Input ACT Vertical Width 2160
  write_C381(B32_PIH0CH2, 0x0000);    // IACT setting based on DE signal automatic measurement CLOSE; IACTHW/VW setting for automatic measurement CLOSE;

  // CH1 ARGS
  write_C381(B144_OFLDOUTCTCH1, 0x02);   // Output field signal from Warping block output side
  write_C381(B144_MCT2CH1, 0x11);        // Output port field  input/output enable
  write_C381(B144_OSYCTCH1, 0x0003);     // Image output port sync signal generation enable ; Use the input VS signal to reset output VS signal.
  write_C381(B144_OHCYCLCH1, 0x08e6);    // HS output interval;  POCLK - 2 = 2278
  write_C381(B144_OVCYCLCH1, 0x096c);    // VS output interval;   lines - 2 = 2412
  write_C381(B144_SYRDLYCH1, 0x0009);    // Forced sync reset delay
  write_C381(B144_OVCYCLMINCH1, 0x082a); // Output Vertical Cycle Minimum Limit   2090
  write_C381(B144_FLDDLYCH1, 0x0008);    // Field propagation delay
  write_C381(B144_OAOI0HSTCH1, 0x011c);  // Output AOI0 horizontal direction start 284
  write_C381(B144_OAOI0HENDCH1, 0x089c); // Output AOI0 Horizontal direction End   2204
  write_C381(B144_OAOI0VSTCH1, 0x0011);  // Output AOI0 Vertical direction  Start 17
  write_C381(B144_OAOI0VENDCH1, 0x0881); // Output AOI0 Vertical direction  End   2177
  write_C381(B144_OACTHSTCH1, 0x011c);   // Output ACT horizontal direction start  284
  write_C381(B144_OACTHWCH1, 0x0780);    // Output ACT horizontal direction width 1920
  write_C381(B144_OACTVSTCH1, 0x0011);   // Output ACT vertical direction start   17
  write_C381(B144_OACTVWCH1, 0x0870);    // Output ACT vertical direction width 2160
  write_C381(B145_DTCTCH1, 0x2008);      // Distortion correction table , create through table mode ; Overflow correction enable
  write_C381(B145_IACTHW3CH1, 0x0780);   // Input port ACT horizontal direction width3 1920
  write_C381(B145_IACTVW3CH1, 0x0870);   // Input port ACT vertical direction width 3   2160

  write_C381(B146_HLUT23CH1, 0x40);
  write_C381(B146_VLUT23CH1, 0x40);
  write_C381(B148_OSFLD0CH1, 0x1800); // Output field 0-3 memory read start address
  write_C381(B148_OSFLD1CH1, 0x0000);
  write_C381(B148_OSFLD2CH1, 0x0800);
  write_C381(B148_OSFLD3CH1, 0x1000);
  write_C381(B148_ISFLD0CH1, 0x00000000); // Input field 0-3 memory write start address
  write_C381(B148_ISFLD1CH1, 0x04000000);
  write_C381(B148_ISFLD2CH1, 0x08000000);
  write_C381(B148_ISFLD3CH1, 0x0c000000);
  write_C381(B148_OMWICH1, 0x00); // Memory read linefeed width 4096 pix
  write_C381(B148_IMWICH1, 0x00); // Memory write linefeed width 4096 pix

  // CH2 ARGS
  write_C381(B149_OFLDOUTCTCH2, 0x02);
  write_C381(B149_MCT2CH2, 0x11);
  write_C381(B149_OSYCTCH2, 0x1013);     // Image output port field signal select O1FLD
  write_C381(B149_OHCYCLCH2, 0x08e6);    // HS output interval;  POCLK - 2 = 2278
  write_C381(B149_OVCYCLCH2, 0x096c);    // VS output interval;   lines - 2 = 2412
  write_C381(B149_SYRDLYCH2, 0x0009);    // Forced sync reset delay
  write_C381(B149_OVCYCLMINCH2, 0x082a); // Output Vertical Cycle Minimum Limit   2090
  write_C381(B149_FLDDLYCH2, 0x0008);    // Field propagation delay
  write_C381(B149_OAOI0HSTCH2, 0x011c);  // Output AOI0 horizontal direction start 284
  write_C381(B149_OAOI0HENDCH2, 0x089c); // Output AOI0 Horizontal direction End   2204
  write_C381(B149_OAOI0VSTCH2, 0x0011);  // Output AOI0 Vertical direction  Start 17
  write_C381(B149_OAOI0VENDCH2, 0x0881); // Output AOI0 Vertical direction  End   2177
  write_C381(B149_OACTHSTCH2, 0x011c);   // Output ACT horizontal direction start  284
  write_C381(B149_OACTHWCH2, 0x0780);    // Output ACT horizontal direction width 1920
  write_C381(B149_OACTVSTCH2, 0x0011);   // Output ACT vertical direction start   17
  write_C381(B149_OACTVWCH2, 0x0870);    // Output ACT vertical direction width 2160
  write_C381(B150_DTCTCH2, 0x2008);      // Distortion correction table , create through table mode ; Overflow correction enable
  write_C381(B150_IACTHW3CH2, 0x0780);   // Input port ACT horizontal direction width3 1920
  write_C381(B150_IACTVW3CH2, 0x0870);   // Input port ACT vertical direction width 3   2160

  write_C381(B151_HLUT23CH2, 0x40);
  write_C381(B151_VLUT23CH2, 0x40);
  write_C381(B153_OSFLD0CH2, 0x1800); // Output field 0-3 memory read start address
  write_C381(B153_OSFLD1CH2, 0x0000);
  write_C381(B153_OSFLD2CH2, 0x0800);
  write_C381(B153_OSFLD3CH2, 0x1000);
  write_C381(B153_ISFLD0CH2, 0x00007800); // Input field 0-3 memory write start address
  write_C381(B153_ISFLD1CH2, 0x04007800);
  write_C381(B153_ISFLD2CH2, 0x08007800);
  write_C381(B153_ISFLD3CH2, 0x0c007800);
  write_C381(B153_OMWICH2, 0x00); // Memory read linefeed width 4096 pix
  write_C381(B153_IMWICH2, 0x00); // Memory write linefeed width 4096 pix

  write_C381(B2_VBOCT, 0x0b);
  write_C381(B2_VBOCT, 0x0f);           // Vbyone read select RX11 , TX1/TX2/RX1/RX2 reset
  write_C381(B2_VBOTX1_PWR, 0x00);      // Vbyone TX1 power down
  write_C381(B2_VBOTX1MD, 0x0011);      // Vbyone TX1 4byte mode , RGB/YUV_444 30bit
  write_C381(B2_VBOTX1_LANECTRL, 0xff); // LANE 0-7 enable setting
  write_C381(B2_VBOTX1_FMT, 0x11);      // TX1 RGB444/YCbCr444 30bit
  write_C381(B2_VBOTX1_CTR1, 0x4c);     // Post DIV control  /1 (40 MHz < FREF < 133.33 MHz);  PMA PLL normal ;
  write_C381(B2_VBOTX1_PWR, 0x01);      // Vbyone TX1 power on
  write_C381(B2_VBOTX1CTL, 0x09);       // 4 pixels parallel;output en
  write_C381(B2_VBOTX1MD, 0x8011);      // TX1 Transfer start

  write_C381(B0_MCT1, 0x11);     // Controls image input and output command en
  write_C381(B0_MCT2, 0x11);     // Controls image input and output command en
  write_C381(B0_STBYCT, 0xfcfc); // Input CH1/2 clock enable; I/O CH1/2 clock enable
}

void C381_init_3840x2160_bak(void)
{

  // REGISTERDUMPFILEtime:22:29:28date:04/24/17
  // MCLK_FREQ   32000000
  // PO1CLK_FREQ	27000000
  // PO2CLK_FREQ	27000000
  //  HAL_Delay(12);

  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  write_C381(BN_RSTCT, 0x000001);
  HAL_Delay(100);

  write_C381(B5_GIOPDCT, 0x3fff);

  write_C381(BN_DDR3CT, 0x00);
  write_C381(BN_MEMCT, 0x00);
  write_C381(B1_DDR3DLLCT, 0x00);
  HAL_Delay(1);
  // DDR setting
  write_C381(B1_DDR3CT2, 0x30); 
  write_C381(B1_DDR3GDS, 0x33333333);
  write_C381(B1_DDR3MSDLY, 0x66664455);
  write_C381(B1_DDR3PHYODTMD, 0x22222222);
  write_C381(B1_DDR3RONMD_0, 0x77);
  write_C381(B1_DDR3RONMD_1, 0x77);
  write_C381(B1_DDR3RONMD_2, 0x77);
  write_C381(B1_DDR3RONMD_3, 0x77);
  write_C381(B1_DDR3RONMD_4, 0x07);
  write_C381(B1_DDR3OCDDIP, 0x3f);
  write_C381(B1_DDR3OCDDIN, 0x3f);
  write_C381(B1_DDR3LVLCT, 0x00);
  write_C381(B1_DDR3WRLVLPHCT1, 0x26);
  write_C381(B1_DDR3WRLVLPHCT2, 0x26);
  write_C381(B1_DDR3WRLVLPHCT3, 0x24);
  write_C381(B1_DDR3WRLVLPHCT4, 0x24);
  write_C381(B1_DDR3WRLVLPHCT5, 0x45);
  write_C381(B1_DDR3WRLVLPHCT6, 0x45);
  write_C381(B1_DDR3WRLVLPHCT7, 0x62);
  write_C381(B1_DDR3WRLVLPHCT8, 0x62);
  write_C381(B1_DDR3VREFCT, 0x09);
  write_C381(B1_DDR3DLLCT, 0x0f);
  write_C381(B1_DDR3ODTCT, 0x11);
  write_C381(B1_DDR3ZQCT, 0x01);
  write_C381(B1_DDR3WRTMGCT, 0x88888888);
  write_C381(B1_DDR3RDTMGCT1, 0x77);
  write_C381(B1_DDR3RDTMGCT2, 0x77);
  write_C381(B1_DDR3RDTMGCT3, 0x77);
  write_C381(B1_DDR3RDTMGCT4, 0x77);
  write_C381(B1_DDR3RDTMGCT5, 0x77);
  write_C381(B1_DDR3RDTMGCT6, 0x77);
  write_C381(B1_DDR3RDTMGCT7, 0x77);
  write_C381(B1_DDR3RDTMGCT8, 0x77);
  write_C381(B1_DDR3CMDTMGCT, 0x07);
  write_C381(B1_DDR3CKTMGCT1, 0x67);
  write_C381(B1_DDR3CKTMGCT2, 0x65);
  write_C381(B1_DDR3CKTMGCT3, 0x62);
  write_C381(B1_DDR3CKTMGCT4, 0x60);
  write_C381(B1_DDR3CKTMGCT5, 0x6a);
  write_C381(B1_DDR3CKTMGCT6, 0x67);
  write_C381(B1_DDR3CKTMGCT7, 0x64);
  write_C381(B1_DDR3CKTMGCT8, 0x61);
  write_C381(B1_DDR3CKTMGCTA, 0x6c);
  write_C381(B1_DDR3CKDUTYSEL, 0x08);
  write_C381(B1_DDR3DQRDDLSEL1, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL2, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL3, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL4, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL5, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL6, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL7, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL8, 0x924924);
  write_C381(B1_DDR3DQWRDLSEL1, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL2, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL3, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL4, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL5, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL6, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL7, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL8, 0x6db6db);
  write_C381(B1_DDR3DQMDLSEL, 0x33333333);
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0x88);
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0xc8);
  HAL_Delay(2);
  write_C381(BN_MEMCT, 0x61);

  // V-by-One Receiver setup
  HAL_Delay(1);
  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  // VbyOne-RXCH1setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0e);         //[0]ResetEnableV-by-OneRX1leV-by-OneRX1
  write_C381(B2_VBOCT, 0x0f);         //[0]ResetDisableV-by-OneRX1bleV-by-OneRX1
  write_C381(B2_VBORX1_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX1_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX1_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps
  HAL_Delay(100);

  // VbyOne-RXCH2setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0d);         //[1]ResetEnableV-by-OneRX2leV-by-OneRX2
  write_C381(B2_VBOCT, 0x0f);         //[1]ResetDisableV-by-OneRX2bleV-by-OneRX2
  write_C381(B2_VBORX2_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX2_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX2_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps

  HAL_Delay(100);

  write_C381(B0_MCT1, 0x10);
  write_C381(B0_MCT2, 0x10);
  // 8 Lane Parallel V-by-One? Input per 1 pixel
  write_C381(B0_ICHSEL1, 0x08); // leo0ch//
  write_C381(B0_ICHSEL2, 0x0a); // leo0eh//
  write_C381(B0_PI1SWP, 0x04);
  write_C381(B0_PI2SWP, 0x04);
  write_C381(B0_PI3SWP, 0x04);
  write_C381(B0_PI4SWP, 0x04);
  write_C381(B0_VI1CT, 0x0404);
  write_C381(B0_VI2CT, 0x0404);
  write_C381(B0_VIDSKW, 0x0202);

  write_C381(B3_IDUALCTCH12, 0x02);
  write_C381(B3_IDACTHSTCH12, 0x0120); // leo00c0h//
  write_C381(B3_IDACTHWCH12, 0x0f10);  // leo
  write_C381(B3_OIDUALCTCH12, 0x0005);
  write_C381(B3_ODACTHWCH12, 0x0780); // 770
  write_C381(B3_OSYCTTX1, 0x19);      // DE=AOI0
  write_C381(B3_OHSCTTX1, 0x15);
  write_C381(B3_OVSCTTX1, 0x09);
  write_C381(B3_VBOCHSEL, 0x51405140);
  write_C381(B3_LANESELTX1, 0x73625140);

  write_C381(B6_VISYPOLCTRL, 0x00003333);

  write_C381(B16_ISYCTCH1, 0x01);
  write_C381(B16_DIFCTCH1, 0x81);
  write_C381(B16_IACTHSTCH1, 0x0111);
  write_C381(B16_IACTHWCH1, 0x0790); // LEO 780h
  write_C381(B16_IACTVSTCH1, 0x0021);
  write_C381(B16_IACTVWCH1, 0x0870);
  write_C381(B16_PIH0CH1, 0x0000);

  write_C381(B32_ISYCTCH2, 0x08);
  write_C381(B32_DIFCTCH2, 0x81);
  write_C381(B32_IACTHSTCH2, 0x0109);
  write_C381(B32_IACTHWCH2, 0x0780);
  write_C381(B32_IACTVSTCH2, 0x0021);
  write_C381(B32_IACTVWCH2, 0x0870);
  write_C381(B32_PIH0CH2, 0x0000);

  write_C381(B144_OFLDOUTCTCH1, 0x02);
  write_C381(B144_MCT2CH1, 0x11);
  write_C381(B144_OSYCTCH1, 0x0003);
  write_C381(B144_OHCYCLCH1, 0x08e6);
  write_C381(B144_OVCYCLCH1, 0x096c);
  write_C381(B144_SYRDLYCH1, 0x0009);
  write_C381(B144_OVCYCLMINCH1, 0x082a);
  write_C381(B144_FLDDLYCH1, 0x0008);
  write_C381(B144_OAOI0HSTCH1, 0x011c);
  write_C381(B144_OAOI0HENDCH1, 0x089c);
  write_C381(B144_OAOI0VSTCH1, 0x0011);
  write_C381(B144_OAOI0VENDCH1, 0x0881);
  write_C381(B144_OACTHSTCH1, 0x011c);
  write_C381(B144_OACTHWCH1, 0x0780);
  write_C381(B144_OACTVSTCH1, 0x0011);
  write_C381(B144_OACTVWCH1, 0x0870);

  write_C381(B145_DTCTCH1, 0x2008);
  write_C381(B145_IACTHW3CH1, 0x0780);
  write_C381(B145_IACTVW3CH1, 0x0870);

  write_C381(B146_HLUT23CH1, 0x40);
  write_C381(B146_VLUT23CH1, 0x40);

  write_C381(B148_OSFLD0CH1, 0x1800);
  write_C381(B148_OSFLD1CH1, 0x0000);
  write_C381(B148_OSFLD2CH1, 0x0800);
  write_C381(B148_OSFLD3CH1, 0x1000);
  write_C381(B148_ISFLD0CH1, 0x00000000);
  write_C381(B148_ISFLD1CH1, 0x04000000);
  write_C381(B148_ISFLD2CH1, 0x08000000);
  write_C381(B148_ISFLD3CH1, 0x0c000000);
  write_C381(B148_OMWICH1, 0x00);
  write_C381(B148_IMWICH1, 0x00);

  write_C381(B149_OFLDOUTCTCH2, 0x02);
  write_C381(B149_MCT2CH2, 0x11);
  write_C381(B149_OSYCTCH2, 0x1013);
  write_C381(B149_OHCYCLCH2, 0x08e6);
  write_C381(B149_OVCYCLCH2, 0x096c);
  write_C381(B149_SYRDLYCH2, 0x0009);
  write_C381(B149_OVCYCLMINCH2, 0x082a);
  write_C381(B149_FLDDLYCH2, 0x0008);
  write_C381(B149_OAOI0HSTCH2, 0x011c);
  write_C381(B149_OAOI0HENDCH2, 0x089c);
  write_C381(B149_OAOI0VSTCH2, 0x0011);
  write_C381(B149_OAOI0VENDCH2, 0x0881);
  write_C381(B149_OACTHSTCH2, 0x011c);
  write_C381(B149_OACTHWCH2, 0x0780);
  write_C381(B149_OACTVSTCH2, 0x0011);
  write_C381(B149_OACTVWCH2, 0x0870);

  write_C381(B150_DTCTCH2, 0x2008);
  write_C381(B150_IACTHW3CH2, 0x0780);
  write_C381(B150_IACTVW3CH2, 0x0870);

  write_C381(B151_HLUT23CH2, 0x40);
  write_C381(B151_VLUT23CH2, 0x40);

  write_C381(B153_OSFLD0CH2, 0x1800);
  write_C381(B153_OSFLD1CH2, 0x0000);
  write_C381(B153_OSFLD2CH2, 0x0800);
  write_C381(B153_OSFLD3CH2, 0x1000);
  write_C381(B153_ISFLD0CH2, 0x00007800);
  write_C381(B153_ISFLD1CH2, 0x04007800);
  write_C381(B153_ISFLD2CH2, 0x08007800);
  write_C381(B153_ISFLD3CH2, 0x0c007800);
  write_C381(B153_OMWICH2, 0x00);
  write_C381(B153_IMWICH2, 0x00);

  write_C381(B2_VBOCT, 0x0b);
  write_C381(B2_VBOCT, 0x0f);
  write_C381(B2_VBOTX1_PWR, 0x00);
  write_C381(B2_VBOTX1MD, 0x0011);

  write_C381(B2_VBOTX1_LANECTRL, 0xff);
  write_C381(B2_VBOTX1_FMT, 0x11);
  write_C381(B2_VBOTX1_CTR1, 0x4c);

  write_C381(B2_VBOTX1_PWR, 0x01);
  write_C381(B2_VBOTX1CTL, 0x09);
  write_C381(B2_VBOTX1MD, 0x8011);

  write_C381(B0_MCT1, 0x11);
  write_C381(B0_MCT2, 0x11);

  write_C381(B0_STBYCT, 0xfcfc);

}

/* Reference Video Timings for fixed 600Mhz Pixel Clock, 8-channel Vx1 (recommended )
Resolution  PixelClock  H-Sync     V-Sync      H-Total     Display     Back-porch  Pulse-width Front-porch V-Total     Display     Back-porch  Pulse-width Front-porch
            (unit:MHz)  (unit:KHz)  (unit:Hz)   unit:pixel  unit:pixel  unit:pixel  unit:pixel  unit:pixel  unit:Line   unit:line   unit:line   unit:line   unit:line
3840x2160   600         131.58      60          4560        3840        256         24          440         2193        2160        11          5           17
2712x1528   600         194.81      120         3200        2712        164         32          172         1563        1528        15          5           42
2560x1600   600         150         60          4000        2560        472         280         688         2500        1600        32          6           862
2560x1600   600         199.47      120         3088        2560        100         100         248         1620        1600        15          4           16
*/

void C381_init_2560x1600_120hz(void)
{
  HAL_Delay(12);

#if 1 // 2560x1600 120Hz modified0119
  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  write_C381(BN_RSTCT, 0x000001);
  HAL_Delay(100);

  write_C381(B5_GIOPDCT, 0x3fff);

  write_C381(BN_DDR3CT, 0x00);
  write_C381(BN_MEMCT, 0x00);
  write_C381(B1_DDR3DLLCT, 0x00);
  HAL_Delay(1);

  write_C381(B1_DDR3CT2, 0x30);
  write_C381(B1_DDR3GDS, 0x33333333);
  write_C381(B1_DDR3MSDLY, 0x66664455);
  write_C381(B1_DDR3PHYODTMD, 0x22222222);
  write_C381(B1_DDR3RONMD_0, 0x77);
  write_C381(B1_DDR3RONMD_1, 0x77);
  write_C381(B1_DDR3RONMD_2, 0x77);
  write_C381(B1_DDR3RONMD_3, 0x77);
  write_C381(B1_DDR3RONMD_4, 0x07);
  write_C381(B1_DDR3OCDDIP, 0x3f);
  write_C381(B1_DDR3OCDDIN, 0x3f);
  write_C381(B1_DDR3LVLCT, 0x00);
  write_C381(B1_DDR3WRLVLPHCT1, 0x26);
  write_C381(B1_DDR3WRLVLPHCT2, 0x26);
  write_C381(B1_DDR3WRLVLPHCT3, 0x24);
  write_C381(B1_DDR3WRLVLPHCT4, 0x24);
  write_C381(B1_DDR3WRLVLPHCT5, 0x45);
  write_C381(B1_DDR3WRLVLPHCT6, 0x45);
  write_C381(B1_DDR3WRLVLPHCT7, 0x62);
  write_C381(B1_DDR3WRLVLPHCT8, 0x62);
  write_C381(B1_DDR3VREFCT, 0x09);
  write_C381(B1_DDR3DLLCT, 0x0f);
  write_C381(B1_DDR3ODTCT, 0x11);
  write_C381(B1_DDR3ZQCT, 0x01);
  write_C381(B1_DDR3WRTMGCT, 0x88888888);
  write_C381(B1_DDR3RDTMGCT1, 0x77);
  write_C381(B1_DDR3RDTMGCT2, 0x77);
  write_C381(B1_DDR3RDTMGCT3, 0x77);
  write_C381(B1_DDR3RDTMGCT4, 0x77);
  write_C381(B1_DDR3RDTMGCT5, 0x77);
  write_C381(B1_DDR3RDTMGCT6, 0x77);
  write_C381(B1_DDR3RDTMGCT7, 0x77);
  write_C381(B1_DDR3RDTMGCT8, 0x77);
  write_C381(B1_DDR3CMDTMGCT, 0x07);
  write_C381(B1_DDR3CKTMGCT1, 0x67);
  write_C381(B1_DDR3CKTMGCT2, 0x65);
  write_C381(B1_DDR3CKTMGCT3, 0x62);
  write_C381(B1_DDR3CKTMGCT4, 0x60);
  write_C381(B1_DDR3CKTMGCT5, 0x6a);
  write_C381(B1_DDR3CKTMGCT6, 0x67);
  write_C381(B1_DDR3CKTMGCT7, 0x64);
  write_C381(B1_DDR3CKTMGCT8, 0x61);
  write_C381(B1_DDR3CKTMGCTA, 0x6c);
  write_C381(B1_DDR3CKDUTYSEL, 0x08);
  write_C381(B1_DDR3DQRDDLSEL1, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL2, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL3, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL4, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL5, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL6, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL7, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL8, 0x924924);
  write_C381(B1_DDR3DQWRDLSEL1, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL2, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL3, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL4, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL5, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL6, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL7, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL8, 0x6db6db);
  write_C381(B1_DDR3DQMDLSEL, 0x33333333);
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0x88); // 0x88
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0xc8); // 0xc8
  HAL_Delay(1);
  write_C381(BN_MEMCT, 0x61);

  HAL_Delay(1);
  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  // VbyOne-RXCH1setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0e);         //[0]ResetEnableV-by-OneRX1leV-by-OneRX1
  write_C381(B2_VBOCT, 0x0f);         //[0]ResetDisableV-by-OneRX1bleV-by-OneRX1
  write_C381(B2_VBORX1_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX1_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX1_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps
  HAL_Delay(100);

  // VbyOne-RXCH2setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0d);         //[1]ResetEnableV-by-OneRX2leV-by-OneRX2
  write_C381(B2_VBOCT, 0x0f);         //[1]ResetDisableV-by-OneRX2bleV-by-OneRX2
  write_C381(B2_VBORX2_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX2_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX2_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps

  HAL_Delay(100);

  write_C381(B0_MCT1, 0x10);
  write_C381(B0_MCT2, 0x10);
  write_C381(B0_ICHSEL1, 0x08); 
  write_C381(B0_ICHSEL2, 0x0a); 
  write_C381(B0_PI1SWP, 0x04);
  write_C381(B0_PI2SWP, 0x04);
  write_C381(B0_PI3SWP, 0x04);
  write_C381(B0_PI4SWP, 0x04);
  write_C381(B0_VI1CT, 0x0404);
  write_C381(B0_VI2CT, 0x0404);
  write_C381(B0_VIDSKW, 0x0202);

  write_C381(B3_IDUALCTCH12, 0x02);     // 2-phase input, four pixels at a time
  write_C381(B3_IDACTHSTCH12,0x38); 	// modified0119 ch1&2 ACT H start: (H_back-porch + H_pulse-width)/2 = (100+100)/2 = 100
  write_C381(B3_IDACTHWCH12, 0x0A00);   // ch1&2 ACT H width: 2560 
  write_C381(B3_OIDUALCTCH12, 0x005);   // ch1&2 2-phase output enable area select AOI0
  write_C381(B3_ODACTHWCH12, 0x0500);   // ch1&2 2-phase output ACT horizontal width 1280
  write_C381(B3_OSYCTTX1, 0x19);        // CMOS ACT select OAOI0 ;V-by-One TX sync signal polarity high; V-by-One TX ACT select OAOI0;
  write_C381(B3_OHSCTTX1, 0x31);        // width of horizontal sync signal output HS (clock / 2 - 1 )  H-SyncPulse/2 -1 = 100/2-1= 0x31 
  write_C381(B3_OVSCTTX1, 0x3);         // width of the vertical sync signal output POVS   V-SyncPulse -1 = 4 -1 = 0x3
  write_C381(B3_VBOCHSEL, 0x51405140);	
  write_C381(B3_LANESELTX1, 0x73625140);
  write_C381(B6_VISYPOLCTRL, 0x00003333);   //VbyOne 1 H/V sync pulse polarity high, ACT area polarity low 

  // CH1 ARGS
  write_C381(B16_ISYCTCH1, 0x01);     // Image input port field signal control.  Cycle between 0 and 3.
  write_C381(B16_DIFCTCH1, 0x80);     // modified0119 Mask image input other than PIACT to 0. ;Input port horizontal sync signal measurement CLK select TICLK. (TICLK=PO1PLLCT created with CLK).
  write_C381(B16_IACTHSTCH1, 0x31);   // modified0119 Input ACT Horizontal Start: B3_IDACTHSTCH12(0x64)-offset(7)=0x5D
  write_C381(B16_IACTHWCH1, 0x0500);  // Input ACT Horizontal Width: Display_width/2 = 1280  
  write_C381(B16_IACTVSTCH1, 0x13);   // Input ACT Vertical Start (V-back-porch + V-pulse-width) = 15+4 = 0X13
  write_C381(B16_IACTVWCH1, 0x0640);  // Input ACT Vertical Width 1600
  write_C381(B16_PIH0CH1, 0x0000);    // auto IACTHST/IACTHW/IACTVST/IACTVW OFF
  // CH2 ARGS
  write_C381(B32_ISYCTCH2, 0x08);	// Image input port field signal control, I1FLD (Keep 0 for CH1)
  write_C381(B32_DIFCTCH2, 0x80);     // modified0119 Mask image input other than PIACT to 0. ;Input port horizontal sync signal measurement CLK select TICLK. (TICLK=PO1PLLCT created with CLK).
  write_C381(B32_IACTHSTCH2, 0x31);   // modified0119 Input ACT Horizontal Start B3_IDACTHSTCH12(0x64)-offset(7)=0x5D 
  write_C381(B32_IACTHWCH2, 0x0500);  // Input ACT Horizontal Width: Display_width/2 = 1280
  write_C381(B32_IACTVSTCH2, 0x13);   // Input ACT Vertical Start (back-porch + pulse-width) = 15+4 = 0X13
  write_C381(B32_IACTVWCH2, 0x0640);  // Input ACT Vertical Width 1600
  write_C381(B32_PIH0CH2, 0x0000);    // auto IACTHST/IACTHW/IACTVST/IACTVW OFF

  // CH1 ARGS
  write_C381(B144_OFLDOUTCTCH1, 0x02);   // Output field signal from Warping block output side
  write_C381(B144_MCT2CH1, 0x11);        // Output port field  input/output enable
  write_C381(B144_OSYCTCH1, 0x0003);     // Image output port sync signal generation enable ; Use the input VS signal to reset output VS signal.
  write_C381(B144_OHCYCLCH1, 0x606);     // HS output interval;  Htotal/2 -2 = 3088/2-2 = 1542
  write_C381(B144_OVCYCLCH1, 0x652);     // VS output interval;   VTotal -2  = 1620 -2 = 1618
  write_C381(B144_SYRDLYCH1, 0x0008);    // sync reset delay, force frame sync recommend 8
  write_C381(B144_OVCYCLMINCH1, 0x05b7); // Output Vertical Cycle Minimum Limit 	???how to calc min limit???
  write_C381(B144_FLDDLYCH1, 0x0009);    // Field Delay; recommend SYRDLY + 1 ;  
  write_C381(B144_OAOI0HSTCH1, 0x3c);    // modified0119 B3_IDACTHSTCH12(0x64)-offset(-4)= 0x68  	???how to set offset value???
  write_C381(B144_OAOI0HENDCH1, 0x53c); //  modified0119 Output AOI0 Horizontal End. B144_OAOI0HSTCH1 + display_width /2  = 0x68 +  1280 = 0x568
  write_C381(B144_OAOI0VSTCH1, 0x13);   // Output AOI0 Vertical direction  Start ; (back-porch + pulse-width) = 15+4 = 0X13
  write_C381(B144_OAOI0VENDCH1, 0x653); // Output AOI0 Vertical direction  End   ; B144_OAOI0VSTCH1 + display height = 0x13 + 1600 = 0x653
  write_C381(B144_OACTHSTCH1, 0x3c);     // modified0119 B3_IDACTHSTCH12(0x64)-offset(-4)=0x68       ???how to set offset value???
  write_C381(B144_OACTHWCH1, 0x0500);    // Output ACT horizontal direction width 1280
  write_C381(B144_OACTVSTCH1, 0x13);     // Output ACT vertical direction start (back-porch + pulse-width) = 15+4 = 19
  write_C381(B144_OACTVWCH1, 0x0640);    // Output ACT vertical direction width  1600
  write_C381(B145_DTCTCH1, 0x2008);      // Distortion correction table , create through table mode ; Overflow correction enable
  write_C381(B145_IACTHW3CH1, 0x0500);   // Input port ACT horizontal direction width3 1280
  write_C381(B145_IACTVW3CH1, 0x0640);   // Input port ACT vertical direction width3 1600

  write_C381(B146_HLUT23CH1, 0x40);
  write_C381(B146_VLUT23CH1, 0x40);
  write_C381(B148_OSFLD0CH1, 0x0000);     // Output field 0-3 memory read start address
  write_C381(B148_OSFLD1CH1, 0x0800);     
  write_C381(B148_OSFLD2CH1, 0x1000);     
  write_C381(B148_OSFLD3CH1, 0x1800);     
  write_C381(B148_ISFLD0CH1, 0x00000000); // Input field 0-3 memory write start address
  write_C381(B148_ISFLD1CH1, 0x04000000);
  write_C381(B148_ISFLD2CH1, 0x08000000);
  write_C381(B148_ISFLD3CH1, 0x0c000000);
  write_C381(B148_OMWICH1, 0x00); 	// Memory read linefeed width 4096 pix
  write_C381(B148_IMWICH1, 0x00); 	// Memory write linefeed width 4096 pix

  // CH2 ARGS
  write_C381(B149_OFLDOUTCTCH2, 0x02);   // Output field signal from Warping block output side
  write_C381(B149_MCT2CH2, 0x11);        // Output port field  input/output enable
  write_C381(B149_OSYCTCH2, 0x1013);     // Image output port field signal select O1FLD, ch2 sync common with ch1
  write_C381(B149_OHCYCLCH2, 0x606);     // HS output interval;  Htotal/2 -2 = 3088/2-2 = 1542
  write_C381(B149_OVCYCLCH2, 0x652);     // VS output interval;   VTotal -2  = 1620 -2 = 1618
  write_C381(B149_SYRDLYCH2, 0x0008);    // sync reset delay, force frame sync recommend 8
  write_C381(B149_OVCYCLMINCH2, 0x05b7); // Output Vertical Cycle Minimum Limit ; 1463 ???how to calc min limit???
  write_C381(B149_FLDDLYCH2, 0x0009);    // Field Delay; recommend SYRDLY + 1 ;  
  write_C381(B149_OAOI0HSTCH2,  0x3c);   // modified0119 B3_IDACTHSTCH12(0x64)-offset(-4)= 0x68 ???how to set offset value???
  write_C381(B149_OAOI0HENDCH2, 0x53c);  // modified0119 Output AOI0 Horizontal End. B144_OAOI0HSTCH1 + display_width /2  = 0x68 +  1280 = 0x568
  write_C381(B149_OAOI0VSTCH2, 0x13);    // Output AOI0 Vertical direction  Start ; (back-porch + pulse-width) = 15+4 = 0X13
  write_C381(B149_OAOI0VENDCH2, 0x653);  // Output AOI0 Vertical direction  End   ; B144_OAOI0VSTCH1 + display height = 0x13 + 1600 = 0x653
  write_C381(B149_OACTHSTCH2, 0x3c);     // modified0119 B3_IDACTHSTCH12(0x64)-offset(-4)=0x68 ???how to set offset value???
  write_C381(B149_OACTHWCH2, 0x0500);    // Output ACT horizontal direction width 1280
  write_C381(B149_OACTVSTCH2, 0x13);     // Output ACT vertical direction start (back-porch + pulse-width) = 15+4 = 19
  write_C381(B149_OACTVWCH2, 0x0640);    // Output ACT vertical direction width 1600
  write_C381(B150_DTCTCH2, 0x2008);      // Distortion correction table , create through table mode ; Overflow correction enable
  write_C381(B150_IACTHW3CH2, 0x0500);   // Input port ACT horizontal direction width3 1280
  write_C381(B150_IACTVW3CH2, 0x0640);   // Input port ACT vertical direction width3  1600

  write_C381(B151_HLUT23CH2, 0x40);
  write_C381(B151_VLUT23CH2, 0x40);
  write_C381(B153_OSFLD0CH2, 0x0000);     // Output field 0-3 memory read start address  ???how to calc? We noticed you're not using the result of??C381_Memory_address_setting_v10.xml?��???
  write_C381(B153_OSFLD1CH2, 0x0800);     
  write_C381(B153_OSFLD2CH2, 0x1000);     
  write_C381(B153_OSFLD3CH2, 0x1800);     
  write_C381(B153_ISFLD0CH2, 0x00005000); // Input field 0-3 memory write start address  ???how to calc? We noticed you're not using the result of??C381_Memory_address_setting_v10.xml?��???
  write_C381(B153_ISFLD1CH2, 0x04005000);
  write_C381(B153_ISFLD2CH2, 0x08005000);
  write_C381(B153_ISFLD3CH2, 0x0c005000);
  write_C381(B153_OMWICH2, 0x00); 	// Memory read linefeed width 4096 pix
  write_C381(B153_IMWICH2, 0x00); 	// Memory write linefeed width 4096 pix

  write_C381(B2_VBOCT, 0x0b);
  write_C381(B2_VBOCT, 0x0f);           // Vbyone read select RX11 , TX1/TX2/RX1/RX2 reset
  write_C381(B2_VBOTX1_PWR, 0x00);      // Vbyone TX1 power down
  write_C381(B2_VBOTX1MD, 0x0011);      // Vbyone TX1 4byte mode , RGB/YUV_444 30bit
  write_C381(B2_VBOTX1_LANECTRL, 0xff); // LANE 0-7 enable setting
  write_C381(B2_VBOTX1_FMT, 0x11);      // TX1 RGB444/YCbCr444 30bit
  write_C381(B2_VBOTX1_CTR1, 0x4c);     // Post DIV control  /1 (40 MHz < FREF < 133.33 MHz);  PMA PLL normal ;
  write_C381(B2_VBOTX1_PWR, 0x01);      // Vbyone TX1 power on
  write_C381(B2_VBOTX1CTL, 0x09);       // 4 pixels parallel;output en
  write_C381(B2_VBOTX1MD, 0x8011);      // TX1 Transfer start

  write_C381(B0_MCT1, 0x11);     	// Controls image input and output command en
  write_C381(B0_MCT2, 0x11);     	// Controls image input and output command en
  write_C381(B0_STBYCT, 0xfcfc); 	// Input CH1/2 clock enable; I/O CH1/2 clock enable
#endif

#if 0  // freerun 
  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  write_C381(BN_RSTCT, 0x000001);
  HAL_Delay(100);

  write_C381(B5_GIOPDCT, 0x3fff);

  write_C381(BN_DDR3CT, 0x00);
  write_C381(BN_MEMCT, 0x00);
  write_C381(B1_DDR3DLLCT, 0x00);
  HAL_Delay(1);

  write_C381(B1_DDR3CT2, 0x30);
  write_C381(B1_DDR3GDS, 0x33333333);
  write_C381(B1_DDR3MSDLY, 0x66664455);
  write_C381(B1_DDR3PHYODTMD, 0x22222222);
  write_C381(B1_DDR3RONMD_0, 0x77);
  write_C381(B1_DDR3RONMD_1, 0x77);
  write_C381(B1_DDR3RONMD_2, 0x77);
  write_C381(B1_DDR3RONMD_3, 0x77);
  write_C381(B1_DDR3RONMD_4, 0x07);
  write_C381(B1_DDR3OCDDIP, 0x3f);
  write_C381(B1_DDR3OCDDIN, 0x3f);
  write_C381(B1_DDR3LVLCT, 0x00);
  write_C381(B1_DDR3WRLVLPHCT1, 0x26);
  write_C381(B1_DDR3WRLVLPHCT2, 0x26);
  write_C381(B1_DDR3WRLVLPHCT3, 0x24);
  write_C381(B1_DDR3WRLVLPHCT4, 0x24);
  write_C381(B1_DDR3WRLVLPHCT5, 0x45);
  write_C381(B1_DDR3WRLVLPHCT6, 0x45);
  write_C381(B1_DDR3WRLVLPHCT7, 0x62);
  write_C381(B1_DDR3WRLVLPHCT8, 0x62);
  write_C381(B1_DDR3VREFCT, 0x09);
  write_C381(B1_DDR3DLLCT, 0x0f);
  write_C381(B1_DDR3ODTCT, 0x11);
  write_C381(B1_DDR3ZQCT, 0x01);
  write_C381(B1_DDR3WRTMGCT, 0x88888888);
  write_C381(B1_DDR3RDTMGCT1, 0x77);
  write_C381(B1_DDR3RDTMGCT2, 0x77);
  write_C381(B1_DDR3RDTMGCT3, 0x77);
  write_C381(B1_DDR3RDTMGCT4, 0x77);
  write_C381(B1_DDR3RDTMGCT5, 0x77);
  write_C381(B1_DDR3RDTMGCT6, 0x77);
  write_C381(B1_DDR3RDTMGCT7, 0x77);
  write_C381(B1_DDR3RDTMGCT8, 0x77);
  write_C381(B1_DDR3CMDTMGCT, 0x07);
  write_C381(B1_DDR3CKTMGCT1, 0x67);
  write_C381(B1_DDR3CKTMGCT2, 0x65);
  write_C381(B1_DDR3CKTMGCT3, 0x62);
  write_C381(B1_DDR3CKTMGCT4, 0x60);
  write_C381(B1_DDR3CKTMGCT5, 0x6a);
  write_C381(B1_DDR3CKTMGCT6, 0x67);
  write_C381(B1_DDR3CKTMGCT7, 0x64);
  write_C381(B1_DDR3CKTMGCT8, 0x61);
  write_C381(B1_DDR3CKTMGCTA, 0x6c);
  write_C381(B1_DDR3CKDUTYSEL, 0x08);
  write_C381(B1_DDR3DQRDDLSEL1, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL2, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL3, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL4, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL5, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL6, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL7, 0x924924);
  write_C381(B1_DDR3DQRDDLSEL8, 0x924924);
  write_C381(B1_DDR3DQWRDLSEL1, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL2, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL3, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL4, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL5, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL6, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL7, 0x6db6db);
  write_C381(B1_DDR3DQWRDLSEL8, 0x6db6db);
  write_C381(B1_DDR3DQMDLSEL, 0x33333333);
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0x88); // 0x88
  HAL_Delay(1);
  write_C381(BN_DDR3CT, 0xc8); // 0xc8
  HAL_Delay(1);
  write_C381(BN_MEMCT, 0x61);

  HAL_Delay(1);
  write_C381(B0_STBYCT, 0x0000);
  write_C381(B0_VBOBYPSCT, 0x00);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(1);

  // VbyOne-RXCH1setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0e);         //[0]ResetEnableV-by-OneRX1leV-by-OneRX1
  write_C381(B2_VBOCT, 0x0f);         //[0]ResetDisableV-by-OneRX1bleV-by-OneRX1
  write_C381(B2_VBORX1_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX1_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX1_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps
  HAL_Delay(100);

  // VbyOne-RXCH2setupSeq.(4byteMode/RGB30)

  write_C381(B2_VBOCT, 0x0d);         //[1]ResetEnableV-by-OneRX2leV-by-OneRX2
  write_C381(B2_VBOCT, 0x0f);         //[1]ResetDisableV-by-OneRX2bleV-by-OneRX2
  write_C381(B2_VBORX2_FMT, 0x05);    //[1:0]Bytemode=4-byte
                                      //[5:2]ColorFormat=RGB/YUV44430bit
  write_C381(B2_VBORX2_LANECT, 0xff); //[7:0]AllLaneEnable
  write_C381(B2_VBORX2_OPT1, 0xa3);   //[3]DataRate=2.0-4.0Gbps

  HAL_Delay(100);

  write_C381(B0_MCT1, 0x10);
  write_C381(B0_MCT2, 0x10);
  write_C381(B0_ICHSEL1, 0x08); 
  write_C381(B0_ICHSEL2, 0x0a); 
  write_C381(B0_PI1SWP, 0x04);
  write_C381(B0_PI2SWP, 0x04);
  write_C381(B0_PI3SWP, 0x04);
  write_C381(B0_PI4SWP, 0x04);
  write_C381(B0_VI1CT, 0x0404);
  write_C381(B0_VI2CT, 0x0404);
  write_C381(B0_VIDSKW, 0x0202);

  write_C381(B3_IDUALCTCH12, 0x02);     // 2-phase input, four pixels at a time
  write_C381(B3_IDACTHSTCH12,0x64); 	// ch1&2 ACT H start: (H_back-porch + H_pulse-width)/2 = (100+100)/2 = 100
  write_C381(B3_IDACTHWCH12, 0x0A00);   // ch1&2 ACT H width: 2560 
  write_C381(B3_OIDUALCTCH12, 0x005);   // ch1&2 2-phase output enable area select AOI0
  write_C381(B3_ODACTHWCH12, 0x0500);   // ch1&2 2-phase output ACT horizontal width 1280
  write_C381(B3_OSYCTTX1, 0x19);        // CMOS ACT select OAOI0 ;V-by-One TX sync signal polarity high; V-by-One TX ACT select OAOI0;
  write_C381(B3_OHSCTTX1, 0x31);        // width of horizontal sync signal output HS (clock / 2 - 1 )  H-SyncPulse/2 -1 = 100/2-1= 0x31 
  write_C381(B3_OVSCTTX1, 0x3);         // width of the vertical sync signal output POVS   V-SyncPulse -1 = 4 -1 = 0x3
  write_C381(B3_VBOCHSEL, 0x51405140);	
  write_C381(B3_LANESELTX1, 0x73625140);
  write_C381(B6_VISYPOLCTRL, 0x00003333);   //VbyOne 1 H/V sync pulse polarity high, ACT area polarity low 

  // CH1 ARGS
  write_C381(B16_ISYCTCH1, 0x01);     // Image input port field signal control.  Cycle between 0 and 3.
  write_C381(B16_DIFCTCH1, 0x81);     // Mask image input other than PIACT to 0. ;Input port horizontal sync signal measurement CLK select TICLK. (TICLK=PO1PLLCT created with CLK).
  write_C381(B16_IACTHSTCH1, 0x5D);   // Input ACT Horizontal Start: B3_IDACTHSTCH12(0x64)-offset(7)=0x5D
  write_C381(B16_IACTHWCH1, 0x0500);  // Input ACT Horizontal Width: Display_width/2 = 1280  
  write_C381(B16_IACTVSTCH1, 0x13);   // Input ACT Vertical Start (V-back-porch + V-pulse-width) = 15+4 = 0X13
  write_C381(B16_IACTVWCH1, 0x0640);  // Input ACT Vertical Width 1600
  write_C381(B16_PIH0CH1, 0x0000);    // auto IACTHST/IACTHW/IACTVST/IACTVW OFF
  // CH2 ARGS
  write_C381(B32_ISYCTCH2, 0x08);	// Image input port field signal control, I1FLD (Keep 0 for CH1)
  write_C381(B32_DIFCTCH2, 0x81);	// Mask image input other than PIACT to 0. ;Input port horizontal sync signal measurement CLK select TICLK. (TICLK=PO1PLLCT created with CLK).
  write_C381(B32_IACTHSTCH2, 0x5D);   // Input ACT Horizontal Start B3_IDACTHSTCH12(0x64)-offset(7)=0x5D 
  write_C381(B32_IACTHWCH2, 0x0500);  // Input ACT Horizontal Width: Display_width/2 = 1280
  write_C381(B32_IACTVSTCH2, 0x13);   // Input ACT Vertical Start (back-porch + pulse-width) = 15+4 = 0X13
  write_C381(B32_IACTVWCH2, 0x0640);  // Input ACT Vertical Width 1600
  write_C381(B32_PIH0CH2, 0x0000);    // auto IACTHST/IACTHW/IACTVST/IACTVW OFF

  // CH1 ARGS
  write_C381(B144_OFLDOUTCTCH1, 0x02);   // Output field signal from Warping block output side
  write_C381(B144_MCT2CH1, 0x11);        // Output port field  input/output enable
  write_C381(B144_OSYCTCH1, 0x0001);     // modified for freerun Image output port sync signal generation enable ; Use the input VS signal to reset output VS signal.
  write_C381(B144_OHCYCLCH1, 0x606);     // HS output interval;  Htotal/2 -2 = 3088/2-2 = 1542
  write_C381(B144_OVCYCLCH1, 0x652);     // modified for freerun VS output interval;   VTotal -2  = 1620 -2 = 1618
  write_C381(B144_SYRDLYCH1, 0x0008);    // sync reset delay, force frame sync recommend 8
  write_C381(B144_OVCYCLMINCH1, 0x05b7); // Output Vertical Cycle Minimum Limit 	???how to calc min limit???
  write_C381(B144_FLDDLYCH1, 0x0009);    // Field Delay; recommend SYRDLY + 1 ;  
  write_C381(B144_OAOI0HSTCH1, 0x68);    // B3_IDACTHSTCH12(0x64)-offset(-4)= 0x68  	???how to set offset value???
  write_C381(B144_OAOI0HENDCH1, 0x568); // Output AOI0 Horizontal End. B144_OAOI0HSTCH1 + display_width /2  = 0x68 +  1280 = 0x568
  write_C381(B144_OAOI0VSTCH1, 0x13);   // Output AOI0 Vertical direction  Start ; (back-porch + pulse-width) = 15+4 = 0X13
  write_C381(B144_OAOI0VENDCH1, 0x653); // Output AOI0 Vertical direction  End   ; B144_OAOI0VSTCH1 + display height = 0x13 + 1600 = 0x653
  write_C381(B144_OACTHSTCH1, 0x68);     // B3_IDACTHSTCH12(0x64)-offset(-4)=0x68       ???how to set offset value???
  write_C381(B144_OACTHWCH1, 0x0500);    // Output ACT horizontal direction width 1280
  write_C381(B144_OACTVSTCH1, 0x13);     // Output ACT vertical direction start (back-porch + pulse-width) = 15+4 = 19
  write_C381(B144_OACTVWCH1, 0x0640);    // Output ACT vertical direction width  1600
  write_C381(B145_DTCTCH1, 0x2008);      // Distortion correction table , create through table mode ; Overflow correction enable
  write_C381(B145_IACTHW3CH1, 0x0500);   // Input port ACT horizontal direction width3 1280
  write_C381(B145_IACTVW3CH1, 0x0640);   // Input port ACT vertical direction width3 1600

  write_C381(B146_HLUT23CH1, 0x40);
  write_C381(B146_VLUT23CH1, 0x40);
  write_C381(B148_OSFLD0CH1, 0x0000);     // Output field 0-3 memory read start address
  write_C381(B148_OSFLD1CH1, 0x0800);     
  write_C381(B148_OSFLD2CH1, 0x1000);     
  write_C381(B148_OSFLD3CH1, 0x1800);     
  write_C381(B148_ISFLD0CH1, 0x00000000); // Input field 0-3 memory write start address
  write_C381(B148_ISFLD1CH1, 0x04000000);
  write_C381(B148_ISFLD2CH1, 0x08000000);
  write_C381(B148_ISFLD3CH1, 0x0c000000);
  write_C381(B148_OMWICH1, 0x00); 	// Memory read linefeed width 4096 pix
  write_C381(B148_IMWICH1, 0x00); 	// Memory write linefeed width 4096 pix

  // CH2 ARGS
  write_C381(B149_OFLDOUTCTCH2, 0x02);   // Output field signal from Warping block output side
  write_C381(B149_MCT2CH2, 0x11);        // Output port field  input/output enable
  write_C381(B149_OSYCTCH2, 0x1011);     // modified for freerun Image output port field signal select O1FLD, ch2 sync common with ch1
  write_C381(B149_OHCYCLCH2, 0x606);     // HS output interval;  Htotal/2 -2 = 3088/2-2 = 1542
  write_C381(B149_OVCYCLCH2, 0x652);     // modified for freerun VS output interval;   VTotal -2  = 1620 -2 = 1618
  write_C381(B149_SYRDLYCH2, 0x0008);    // sync reset delay, force frame sync recommend 8
  write_C381(B149_OVCYCLMINCH2, 0x05b7); // Output Vertical Cycle Minimum Limit ; 1463 ???how to calc min limit???
  write_C381(B149_FLDDLYCH2, 0x0009);    // Field Delay; recommend SYRDLY + 1 ;  
  write_C381(B149_OAOI0HSTCH2,  0x68);   // B3_IDACTHSTCH12(0x64)-offset(-4)= 0x68 ???how to set offset value???
  write_C381(B149_OAOI0HENDCH2, 0x568);  // Output AOI0 Horizontal End. B144_OAOI0HSTCH1 + display_width /2  = 0x68 +  1280 = 0x568
  write_C381(B149_OAOI0VSTCH2, 0x13);    // Output AOI0 Vertical direction  Start ; (back-porch + pulse-width) = 15+4 = 0X13
  write_C381(B149_OAOI0VENDCH2, 0x653);  // Output AOI0 Vertical direction  End   ; B144_OAOI0VSTCH1 + display height = 0x13 + 1600 = 0x653
  write_C381(B149_OACTHSTCH2, 0x68);     // B3_IDACTHSTCH12(0x64)-offset(-4)=0x68 ???how to set offset value???
  write_C381(B149_OACTHWCH2, 0x0500);    // Output ACT horizontal direction width 1280
  write_C381(B149_OACTVSTCH2, 0x13);     // Output ACT vertical direction start (back-porch + pulse-width) = 15+4 = 19
  write_C381(B149_OACTVWCH2, 0x0640);    // Output ACT vertical direction width 1600
  write_C381(B150_DTCTCH2, 0x2008);      // Distortion correction table , create through table mode ; Overflow correction enable
  write_C381(B150_IACTHW3CH2, 0x0500);   // Input port ACT horizontal direction width3 1280
  write_C381(B150_IACTVW3CH2, 0x0640);   // Input port ACT vertical direction width3  1600

  write_C381(B151_HLUT23CH2, 0x40);
  write_C381(B151_VLUT23CH2, 0x40);
  write_C381(B153_OSFLD0CH2, 0x0000);     // Output field 0-3 memory read start address  ???how to calc? We noticed you're not using the result of??C381_Memory_address_setting_v10.xml?��???
  write_C381(B153_OSFLD1CH2, 0x0800);     
  write_C381(B153_OSFLD2CH2, 0x1000);     
  write_C381(B153_OSFLD3CH2, 0x1800);     
  write_C381(B153_ISFLD0CH2, 0x00005000); // Input field 0-3 memory write start address  ???how to calc? We noticed you're not using the result of??C381_Memory_address_setting_v10.xml?��???
  write_C381(B153_ISFLD1CH2, 0x04005000);
  write_C381(B153_ISFLD2CH2, 0x08005000);
  write_C381(B153_ISFLD3CH2, 0x0c005000);
  write_C381(B153_OMWICH2, 0x00); 	// Memory read linefeed width 4096 pix
  write_C381(B153_IMWICH2, 0x00); 	// Memory write linefeed width 4096 pix

  write_C381(B2_VBOCT, 0x0b);
  write_C381(B2_VBOCT, 0x0f);           // Vbyone read select RX11 , TX1/TX2/RX1/RX2 reset
  write_C381(B2_VBOTX1_PWR, 0x00);      // Vbyone TX1 power down
  write_C381(B2_VBOTX1MD, 0x0011);      // Vbyone TX1 4byte mode , RGB/YUV_444 30bit
  write_C381(B2_VBOTX1_LANECTRL, 0xff); // LANE 0-7 enable setting
  write_C381(B2_VBOTX1_FMT, 0x11);      // TX1 RGB444/YCbCr444 30bit
  write_C381(B2_VBOTX1_CTR1, 0x4c);     // Post DIV control  /1 (40 MHz < FREF < 133.33 MHz);  PMA PLL normal ;
  write_C381(B2_VBOTX1_PWR, 0x01);      // Vbyone TX1 power on
  write_C381(B2_VBOTX1CTL, 0x09);       // 4 pixels parallel;output en
  write_C381(B2_VBOTX1MD, 0x8011);      // TX1 Transfer start

  write_C381(B0_MCT1, 0x11);     	// Controls image input and output command en
  write_C381(B0_MCT2, 0x11);     	// Controls image input and output command en
  write_C381(B0_STBYCT, 0xfcfc); 	// Input CH1/2 clock enable; I/O CH1/2 clock enable
#endif
}

void C381_Bypass_Mode(void)
{
  HAL_Delay(12);
  write_C381(BN_RSTCT, 0x000001);
  //HAL_Delay(100);
  HAL_Delay(2000);
	// for safety
  write_C381(B5_GIOPDCT, 0x3fff);
  write_C381(B0_CLKSEL, 0x01);
  write_C381(B0_PO1CLKCT, 0x00);
  write_C381(B0_PO1PLLCT, 0x89);
  write_C381(B0_PO1REFSEL, 0x43);
  write_C381(B0_PO1REFDIV, 0x04);
  write_C381(B0_PO1FBDIV, 0x10);
  HAL_Delay(10);
  write_C381(B0_VBOBYPSCT, 0x22);  // Vbyone Bypass mode

  // VbyOne-RX CH1 setup Seq.(4byteMode/RGB30)
  write_C381(B2_VBOCT, 0x0e);      // [  0] Reset Enable  V-by-One RX 1
  write_C381(B2_VBOCT, 0x0f);      // [  0] Reset Disable V-by-One RX 1
  write_C381(B2_VBORX1_FMT, 0x05); // [1:0] Byte mode    = 4-byte
  //                        [5:2] Color Format = RGB/YUV444 30bit
  write_C381(B2_VBORX1_LANECT, 0xff); // [7:0] All Lane Enable
  write_C381(B2_VBORX1_OPT1, 0xa3);   // [  3] Data Rate = 2.0-4.0 Gbps

  // VbyOne-RX CH2 setup Seq.(4byteMode/RGB30)
  write_C381(B2_VBOCT, 0x0d);      // [  1] Reset Enable  V-by-One RX 2
  write_C381(B2_VBOCT, 0x0f);      // [  1] Reset Disable V-by-One RX 2
  write_C381(B2_VBORX2_FMT, 0x05); // [1:0] Byte mode    = 4-byte
  //                        [5:2] Color Format = RGB/YUV444 30bit
  write_C381(B2_VBORX2_LANECT, 0xff); // [7:0] All Lane Enable
  write_C381(B2_VBORX2_OPT1, 0xa3);   // [  3] Data Rate = 2.0-4.0 Gbps

  // VbyOne-TX CH1 setup Seq.(4byteMode/RGB30)
  write_C381(B2_VBOCT, 0x0b);           // [2]TX1RSTB=0(Rst ENB.)
  write_C381(B2_VBOCT, 0x0f);           // [2]TX1RSTB=1(Rst DIS.)
  write_C381(B2_VBOTX1_PWR, 0x00);      // [0]TX1PWR=0(PWR OFF)
  write_C381(B2_VBOTX1MD, 0x0011);      // [15]STRM_DATALOCK=0(Not Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)
  write_C381(B2_VBOTX1_LANECTRL, 0xff); // [7:0]LANE7-0ENB(1=ENB)
  write_C381(B2_VBOTX1_FMT, 0x11);      // [5:4]BMOD=01(4Byte) / [3:0]CFMT=0001(RGB/444 30bit)
  write_C381(B2_VBOTX1_CTR1, 0x4c);     // [7:6]RS=01 / [3:2]FRG=11(40MHz <= FREF <= 133.33M)z)
  write_C381(B2_VBOTX1_PWR, 0x01);      // [0]TX1PWR=1(PWR ON)
  write_C381(B2_VBOTX1CTL, 0x09);       // [3:2]PARAMD=10(4para) / [0]VBOEN=1(ENB)
  write_C381(B2_VBOTX1MD, 0x8011);      // [15]STRM_DATALOCK=1(Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)

  // VbyOne-TX CH2 setup Seq.(4byteMode/RGB30)
  write_C381(B2_VBOCT, 0x07);           // [3]TX2RSTB=0(Rst ENB.)
  write_C381(B2_VBOCT, 0x0f);           // [3]TX2RSTB=1(Rst DIS.)
  write_C381(B2_VBOTX2_PWR, 0x00);      // [0]TX2PWR=0(PWR OFF)
  write_C381(B2_VBOTX2MD, 0x0011);      // [15]STRM_DATALOCK=0(Not Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)
  write_C381(B2_VBOTX2_LANECTRL, 0xff); // [7:0]LANE7-0ENB(1=ENB)
  write_C381(B2_VBOTX2_FMT, 0x11);      // [5:4]BMOD=01(4Byte) / [3:0]CFMT=0001(RGB/444 30bit)
  write_C381(B2_VBOTX2_CTR1, 0x4c);     // [7:6]RS=01 / [3:2]FRG=11(40MHz <= FREF <= 133.33MHz)
  write_C381(B2_VBOTX2_PWR, 0x01);      // [0]TX2PWR=1(PWR ON)
  write_C381(B2_VBOTX2CTL, 0x09);       // [3:2]PARAMD=10(4para) / [0]VBOEN=1(ENB)
  write_C381(B2_VBOTX2MD, 0x8011);      // [15]STRM_DATALOCK=1(Locked) / [7:4]Color=0001(RGB/444 30bit) / [1:0]ByteMode=01(4Byte)
}

void C381_warp_init(void)
{
  load_setupWP();
  // HAL_Delay(100);
  InitMenu();
  // HAL_Delay(100);
  FlashRegLoad();
  HAL_Delay(3000);   //解决花屏问题，必须延迟配置4角矫正

  SetWpKey4c(&PM_WPPOS4C);

  printf("C381 warp config Load\r\n");
}

/**
 * @brief 4kmode init
 *
 */
void C381_4K_Mode(void)
{
#if (Projector_Model == LP92SLB )
  C381_init_3840x2160();
  C381_warp_init();
#elif (Projector_Model == NP72BN)
  C381_init_2560x1600_120hz();
  C381_warp_init();
#else
#endif
}

/**
 * @brief 3d mode init
 *
 */
void C381_3D_Mode(void)
{
#if (Projector_Model == LP92SLB)
  // C381_init_2712x1528();
  C381_Bypass_Mode();
#elif (Projector_Model == NP72BN)
  C381_init_2560x1600_120hz();
  // C381_warp_init();
#else
#endif
}

void C381_Init(void)
{
  printf("Init C381, Mode: 0x%x\n", rt_param.c381_mode);
  C381_Reset();
  switch (rt_param.c381_mode)
  {
  case C381_MODE_3D:
    C381_3D_Mode();
    // C381_Bypass_Mode();
    break;
  case C381_MODE_4K:
    C381_4K_Mode();
    // C381_Bypass_Mode();
    break;
  case C381_MODE_BYPASS:
  default:
    C381_Bypass_Mode();
    break;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
