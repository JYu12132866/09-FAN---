#ifndef HEALTH_MGR_
#define HEALTH_MGR_

#include "stdint.h"

extern uint8_t Warning_Update;

void Report_Warning(void);

uint32_t getFanErrCode(void);
uint32_t getTempErrCode(void);

#endif // HEALTH_MGR_
