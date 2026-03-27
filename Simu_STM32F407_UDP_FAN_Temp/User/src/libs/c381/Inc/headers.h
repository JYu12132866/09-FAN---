// Copyright (C) 2016 i-Chips Technology Inc. All Rights Reserved.
#ifndef INCLUDED_HEADERS_H
#define INCLUDED_HEADERS_H


#include <stdint.h>
#include "stm32f407xx.h"
enum E_Input {
	E_InNoErr,
	E_InErrNoIn,// Error of No-Input
//	E_InErrDiffIn,// Error of Different input between channels
	E_InErrWp,// Error of Warping
	E_InErr// Error of other reasons
};

enum E_WpErr {
	E_WpNoErr,
	E_WpErrAngH,// Error of H-slope
	E_WpErrAngV,// Error of V-slope
	E_WpErrVShL,// Error of local V-shrink rate
	E_WpErrVShA,// Error of average V-shrink rate
	E_WpErrHSh,// Error of H-shrink rate
	E_WpErrOut,// Error of outside of ACT
	E_WpErrItv,// Error of grid interval
	E_WpErr// Error of other reasons
};

enum E_UcErr {
	E_UcNoErr,
	E_UcErrGinitMin,// Error of ginit value (Less than minimum)
	E_UcErrGinitMax,// Error of ginit value (More than maximum)
	E_UcErrDGDHMin,// Error of dg/dh value (Less than minimum)
	E_UcErrDGDHMax,// Error of dg/dh value (More than maximum)
	E_UcErrDGDVMin,// Error of dg/dv value (Less than minimum)
	E_UcErrDGDVMax,// Error of dg/dv value (More than maximum)
	E_UcErrDGDHDVMin,// Error of (dg/dh)/dv value (Less than minimum)
	E_UcErrDGDHDVMax,// Error of (dg/dh)/dv value (More than maximum)
	E_UcErr// Error of other reasons
};

enum E_Splitter {
	E_SplitOff,
	E_SplitOn,
	E_SplitErr
};

enum E_SplitType {
	E_SplitTypeH,
	E_SplitTypeV,
	E_SplitTypeQ,
	E_SplitTypeNoSplit,
	E_SplitTypeErr
};

enum E_SplitImage {
	E_SplitImgL,
	E_SplitImgR,
	E_SplitImgT,
	E_SplitImgB,
	E_SplitImgTL,
	E_SplitImgTR,
	E_SplitImgBL,
	E_SplitImgBR,
	E_SplitImgNoSplit,
	E_SplitImgErr
};

enum E_KeyInWarp {
	E_KeyNoKey,
	E_KeyQuit,
	E_KeyCancel,
	E_KeyEnter,
	E_KeyLeft,//4////
	E_KeyRight,///5
	E_KeyDown,////6
	E_KeyUp,///7
	E_KeyMove,
	E_KeyLeftwardGrid,//9
	E_KeyRightwardGrid,//10
	E_KeyDownwardGrid,//11
	E_KeyUpwardGrid,//12
	E_KeyInternalGrid,
	E_KeyCursorDisp,
	E_KeyReset,
	E_KeyTpOFill,
	E_KeyTpCrossHatch,
	E_KeyPrev,
	E_KeyNext,
	E_KeySelBoard0,
	E_KeySelBoard1,
	E_KeySelBoard2,
	E_KeySelBoard3
};

#endif
