///*****************************************
//  Copyright (C) 2009-2023
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <config.h>
//   @author Kenneth.Hung@ite.com.tw
//   @date   2023/06/28
//   @fileversion: IT66353_SAMPLE_1.12
//******************************************/
#ifndef _6635_CONFIG_H_
#define _6635_CONFIG_H_

#ifndef _SHOW_PRAGMA_MSG
#define message(ignore)
#endif

//
// Switch register i2c address: 0x94(PCADR=0) or 0x96(PCADR=1)
//
#define SWAddr          0xAC
// #define SWAddr          0x94



//
// RX register i2c address ( programmable )
//
#define RXAddr			0xB2


//
// CEC register i2c address ( programmable )
//
#define CECAddr         0xC0


//
// EDID RAM i2c address ( programmable )
//
#define RXEDIDAddr      0xa8




//**********************
// Internal compile options:

#define DEBUG_FSM_CHANGE 1
#define USING_WDOG 0

// 66353 and 66354 don't need this:
// set to 0
#define CHECK_INT_BEFORE_TXOE 0

//
// EN_AUTO_RS: ( compile option )
// 1: Enable Auto EQ code
// 0: Disable Auto EQ code
#define EN_H14_SKEW 0

//
// EN_AUTO_RS: ( compile option )
// 1: Enable Auto EQ code
// 0: Disable Auto EQ code
#define EN_AUTO_RS 1

//
// EN_CEC:
// 1: Enable CEC function
// 0: Disable CEC function
#define EN_CEC 0

//
// FIX_EDID_FOR_ATC_4BLOCK_CTS:
// 1: For ATC 4 blocks EDID test
//
#define FIX_EDID_FOR_ATC_4BLOCK_CTS 1

//
// EDID_PA_MANUAL_ASSIGN:
// 1: assign physical address manually
// 0: physical address auto increment according to sinks'
#define EDID_PA_MANUAL_ASSIGN 0
#if EDID_PA_MANUAL_ASSIGN
// define Physical Address of Port 0,1,2
#define PA_PORT0 0x1000
#define PA_PORT1 0x2000
#define PA_PORT2 0x3000
#endif

#define PORT_CHANGE_WITH_FSM 1
#endif  // _CONFIG_H_

