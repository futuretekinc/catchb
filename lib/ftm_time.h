#ifndef	_FTM_TIME_H_
#define	_FTM_TIME_H_

#include <sys/time.h>
#include "ftm_types.h"

#define	FTM_TIME_SECONDS_OF_DAY	(24*60*60)

typedef	enum
{
	FTM_TIME_ALIGN_1S,
	FTM_TIME_ALIGN_10S,
	FTM_TIME_ALIGN_1M,
	FTM_TIME_ALIGN_10M,
	FTM_TIME_ALIGN_1H,
	FTM_TIME_ALIGN_1D
}	FTM_TIME_ALIGN, _PTR_ FTM_TIME_ALIGN_PTR;

typedef	struct
{
	struct timeval	xTimeval;
}	FTM_TIME, _PTR_ FTM_TIME_PTR;

typedef	struct
{
	FTM_UINT32	ulYear;
	FTM_UINT32	ulMon;
	FTM_UINT32	ulDay;
	FTM_UINT32	ulHour;
	FTM_UINT32	ulMin;
	FTM_UINT32	ulSec;
}	FTM_DATE, _PTR_ FTM_DATE_PTR;

FTM_RET	FTM_TIME_getCurrent
(
	FTM_TIME_PTR pTime
);

FTM_RET	FTM_TIME_getCurrentSecs
(
	FTM_UINT32_PTR	pSecs
);

FTM_RET	FTM_TIME_set
(
	FTM_TIME_PTR 	pTime, 
	FTM_UINT32 		ulMilliSeconds
);

FTM_RET	FTM_TIME_setString
(
	FTM_TIME_PTR	pTime,
	FTM_CHAR_PTR	pString
);

FTM_RET	FTM_TIME_setSeconds
(
	FTM_TIME_PTR 	pTime, 
	FTM_UINT32 		ulSeconds
);

FTM_RET	FTM_TIME_add
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_TIME_PTR 	pTimeB, 
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addMS
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeMS,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addSecs
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeS,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addMins
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeM,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addHours
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeH,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_addDays
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeD,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_sub
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_TIME_PTR 	pTimeB, 
	FTM_TIME_PTR 	pTimeR
);

FTM_RET	FTM_TIME_subMS
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeMS,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_subSecs
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeS,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_subMins
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeM,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_subHours
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeH,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_subDays
(
	FTM_TIME_PTR 	pTimeA, 
	FTM_UINT32		ulTimeD,
	FTM_TIME_PTR 	pTimeS
);

FTM_RET	FTM_TIME_isValid
(
	FTM_TIME_PTR	pTime,
	FTM_TIME_PTR	pBegin,
	FTM_TIME_PTR	pEnd
);

FTM_RET	FTM_TIME_align
(
	FTM_TIME_PTR	pTimeS,
	FTM_TIME_ALIGN	xAlign,
	FTM_TIME_PTR	pTimeR
);

FTM_RET	FTM_TIME_toMS
(
	FTM_TIME_PTR	pTime,
	FTM_UINT64_PTR	pullMS
);

FTM_RET	FTM_TIME_toSecs
(
	FTM_TIME_PTR	pTime,
	FTM_UINT32_PTR	pulSecs
);

FTM_BOOL		FTM_TIME_isZero
(
	FTM_TIME_PTR 	pTime
);

FTM_CHAR_PTR	FTM_TIME_printf
(
	FTM_TIME_PTR	pTime,
	FTM_CHAR_PTR	pFormat
);

FTM_CHAR_PTR	FTM_TIME_printf2
(
	FTM_UINT32		ulTime,
	FTM_CHAR_PTR	pFormat
);

FTM_CHAR_PTR	FTM_TIME_printfCurrent
(
	FTM_CHAR_PTR	pFormat
);

FTM_RET	FTM_TIME_toDate
(
	FTM_TIME_PTR	pTime,
	FTM_DATE_PTR	pDate
);

FTM_RET	FTM_DATE_getCurrent
(
	FTM_DATE_PTR	pDate
);

FTM_RET	FTM_DATE_toTime
(
	FTM_DATE_PTR	pDate,
	FTM_TIME_PTR	pTime
);
#endif
