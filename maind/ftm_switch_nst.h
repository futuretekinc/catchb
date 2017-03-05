#ifndef	FTM_SWITCH_NST_H_
#define	FTM_SWITCH_NST_H_

#include "ftm_types.h"

FTM_RET	FTM_SWITCH_NST_process
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_INT			nIndex,
	FTM_SWITCH_AC_POLICY	xPolicy
); //0 : switch ip deny, 1 : switch ip allow

#endif

