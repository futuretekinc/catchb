#define _XOPEN_SOURCE
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_time.h"
#include "ftm_trace.h"

FTM_RET	FTM_TIME_getCurrent
(
	FTM_TIME_PTR 	pTime
)
{
	ASSERT(pTime != NULL);
	
	gettimeofday(&pTime->xTimeval, NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_getCurrentSecs
(
	FTM_UINT32_PTR	pSecs
)
{
	ASSERT(pSecs != NULL);

	FTM_TIME	xTime;

	FTM_TIME_getCurrent(&xTime);

	*pSecs = xTime.xTimeval.tv_sec;

	return	FTM_RET_OK;
}

FTM_BOOL	FTM_TIME_isZero
(
	FTM_TIME_PTR 	pTime
)
{
	return	((pTime == NULL) || ((pTime->xTimeval.tv_sec == 0) && (pTime->xTimeval.tv_usec == 0)));
}

FTM_RET	FTM_TIME_set
(
	FTM_TIME_PTR 	pTime, 
	FTM_UINT32 		ulTimeMS
)
{
	ASSERT(pTime != NULL);
	
	pTime->xTimeval.tv_sec = ulTimeMS / 1000;
	pTime->xTimeval.tv_usec = ulTimeMS % 1000 * 1000;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_setString
(
	FTM_TIME_PTR	pTime,
	FTM_CHAR_PTR	pString
)
{
	ASSERT(pTime != NULL);
	ASSERT(pString != NULL);
	FTM_INT		i;
	struct tm	xTM;
	FTM_CHAR	pFormattedString[64];
	FTM_UINT32	ulLen = 0;

	if (strlen(pString) == 10)
	{
		if ((pString[4] != '/') || (pString[7] != '/'))
		{
			return	FTM_RET_INVALID_FORMAT;
		}

		memset(&xTM, 0, sizeof(xTM));
		strptime(pString, "%Y/%m/%d", &xTM);
	}
	if (strlen(pString) == 16)
	{
		if ((pString[4] != '/') || (pString[7] != '/') || (pString[10] != '/') || (pString[13] != '/'))
		{
			return	FTM_RET_INVALID_FORMAT;
		}

		memset(&xTM, 0, sizeof(xTM));
		strptime(pString, "%Y/%m/%d/%H/%M", &xTM);
	}
	else if (strlen(pString) == 19)
	{
		if ((pString[4] != '-') || (pString[7] != '-') || (pString[10] != ' ') || (pString[13] != ':')  || (pString[16] != ':'))
		{
			return	FTM_RET_INVALID_FORMAT;
		}

		memset(&xTM, 0, sizeof(xTM));
		strptime(pString, "%Y-%m-%d %H:%M:%S", &xTM);
	}
	else if (strlen(pString) == 12)
	{
		for(i = 0 ; i < 12 ; i++)
		{
			if (isdigit(pString[i]) == 0)
			{
				return	FTM_RET_INVALID_FORMAT;	
			}
		}

		strcpy(pFormattedString, "20");
		ulLen += 2;
		memcpy(&pFormattedString[ulLen], &pString[0], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = '-';
		memcpy(&pFormattedString[ulLen], &pString[2], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = '-';
		memcpy(&pFormattedString[ulLen], &pString[4], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = ' ';
		memcpy(&pFormattedString[ulLen], &pString[6], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = ':';
		memcpy(&pFormattedString[ulLen], &pString[8], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = ':';
		memcpy(&pFormattedString[ulLen], &pString[10], 2);
		ulLen += 2;
		pFormattedString[ulLen] = '\0';

		strptime(pFormattedString, "%Y-%m-%d %H:%M:%S", &xTM);
	}
	else if (strlen(pString) == 14)
	{
		for(i = 0 ; i < 14 ; i++)
		{
			if (isdigit(pString[i]) == 0)
			{
				return	FTM_RET_INVALID_FORMAT;	
			}
		}

		memcpy(&pFormattedString[ulLen], &pString[0], 4);
		ulLen += 4;
		pFormattedString[ulLen++] = '-';
		memcpy(&pFormattedString[ulLen], &pString[4], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = '-';
		memcpy(&pFormattedString[ulLen], &pString[6], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = ' ';
		memcpy(&pFormattedString[ulLen], &pString[8], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = ':';
		memcpy(&pFormattedString[ulLen], &pString[10], 2);
		ulLen += 2;
		pFormattedString[ulLen++] = ':';
		memcpy(&pFormattedString[ulLen], &pString[12], 2);
		ulLen += 2;
		pFormattedString[ulLen] = '\0';

		strptime(pFormattedString, "%Y-%m-%d %H:%M:%S", &xTM);
	}
	else
	{
		time_t	xTime;

		for(FTM_UINT32 i = 0 ; i < strlen(pString) ; i++)
		{
			if (!isdigit(pString[i]))
			{
				return	FTM_RET_INVALID_FORMAT;	
			}
		}
	
		xTime = (time_t)strtoul(pString, 0, 10);
		localtime_r(&xTime, &xTM);
	}

	pTime->xTimeval.tv_sec = mktime(&xTM);
	pTime->xTimeval.tv_usec =0;

	return	FTM_RET_OK;
}


FTM_RET	FTM_TIME_setSeconds
(
	FTM_TIME_PTR 	pTime, 
	FTM_UINT32 		ulTimeS
)
{
	ASSERT(pTime != NULL);
	
	pTime->xTimeval.tv_sec = ulTimeS;
	pTime->xTimeval.tv_usec = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_add
(
	FTM_TIME_PTR 	pTime1, 
	FTM_TIME_PTR 	pTime2, 
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTime2 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;

	xTimeR.xTimeval.tv_usec = pTime1->xTimeval.tv_usec + pTime2->xTimeval.tv_usec;
	xTimeR.xTimeval.tv_sec = pTime1->xTimeval.tv_sec + pTime2->xTimeval.tv_sec + xTimeR.xTimeval.tv_usec / 1000000;
	xTimeR.xTimeval.tv_usec = xTimeR.xTimeval.tv_usec % 1000000;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_addMS
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeMS,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;

	xTimeR.xTimeval.tv_usec = pTime1->xTimeval.tv_usec + ulTimeMS * 1000;
	xTimeR.xTimeval.tv_sec = pTime1->xTimeval.tv_sec + xTimeR.xTimeval.tv_usec / 1000000;
	xTimeR.xTimeval.tv_usec = xTimeR.xTimeval.tv_usec % 1000000;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_addSecs
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeS,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;

	xTimeR.xTimeval.tv_sec = pTime1->xTimeval.tv_sec + ulTimeS;
	xTimeR.xTimeval.tv_usec = pTime1->xTimeval.tv_usec;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_addMins
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeM,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;

	xTimeR.xTimeval.tv_sec = pTime1->xTimeval.tv_sec + ulTimeM * 60;
	xTimeR.xTimeval.tv_usec = pTime1->xTimeval.tv_usec;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_addHours
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeH,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;

	xTimeR.xTimeval.tv_sec = pTime1->xTimeval.tv_sec + ulTimeH * 60 * 60;
	xTimeR.xTimeval.tv_usec = pTime1->xTimeval.tv_usec;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_addDays
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeD,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;

	xTimeR.xTimeval.tv_sec = pTime1->xTimeval.tv_sec + ulTimeD * 60 * 60 * 24;
	xTimeR.xTimeval.tv_usec = pTime1->xTimeval.tv_usec;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_sub
(
	FTM_TIME_PTR 	pTime1, 
	FTM_TIME_PTR 	pTime2, 
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTime2 != NULL);
	ASSERT(pTimeR != NULL);

	FTM_INT64 	nValue1, nValue2, nValueR;

	nValue1 = (FTM_INT64)pTime1->xTimeval.tv_sec * 1000000 + pTime1->xTimeval.tv_usec;
	nValue2 = (FTM_INT64)pTime2->xTimeval.tv_sec * 1000000 + pTime2->xTimeval.tv_usec;

	nValueR = nValue1 - nValue2;
	if (nValueR > 0)
	{
		pTimeR->xTimeval.tv_sec = nValueR / 1000000;
		pTimeR->xTimeval.tv_usec = nValueR % 1000000;
	}
	else
	{
		pTimeR->xTimeval.tv_sec = 0;
		pTimeR->xTimeval.tv_usec = 0;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_subMS
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeMS,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;
	FTM_UINT64	ullTimeUS;

	ullTimeUS = pTime1->xTimeval.tv_sec * (FTM_UINT64)1000000 + pTime1->xTimeval.tv_usec;
	if (ullTimeUS > ulTimeMS * (FTM_UINT64)1000)
	{
		ullTimeUS = ullTimeUS -	(ulTimeMS * (FTM_UINT64)1000);
	}
	else
	{
		ullTimeUS = 0;
	}

	xTimeR.xTimeval.tv_usec = ullTimeUS % 1000000;
	xTimeR.xTimeval.tv_sec = ullTimeUS / 1000000;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_subSecs
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeS,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;
	FTM_UINT64	ullTimeUS;

	ullTimeUS = pTime1->xTimeval.tv_sec * (FTM_UINT64)1000000 + pTime1->xTimeval.tv_usec;
	if (ullTimeUS > ulTimeS * (FTM_UINT64)1000000)
	{
		ullTimeUS = ullTimeUS -	(ulTimeS * (FTM_UINT64)1000000);
	}
	else
	{
		ullTimeUS = 0;
	}

	xTimeR.xTimeval.tv_usec = ullTimeUS % 1000000;
	xTimeR.xTimeval.tv_sec = ullTimeUS / 1000000;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_subMins
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeM,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;
	FTM_UINT64	ullTimeUS;

	ullTimeUS = pTime1->xTimeval.tv_sec * (FTM_UINT64)1000000 + pTime1->xTimeval.tv_usec;
	if (ullTimeUS > ulTimeM * 60 * (FTM_UINT64)1000000)
	{
		ullTimeUS = ullTimeUS -	(ulTimeM * 60 * (FTM_UINT64)1000000);
	}
	else
	{
		ullTimeUS = 0;
	}

	xTimeR.xTimeval.tv_usec = ullTimeUS % 1000000;
	xTimeR.xTimeval.tv_sec = ullTimeUS / 1000000;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_subHours
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeH,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;
	FTM_UINT64	ullTimeUS;

	ullTimeUS = pTime1->xTimeval.tv_sec * (FTM_UINT64)1000000 + pTime1->xTimeval.tv_usec;
	if (ullTimeUS > ulTimeH * 60 * 60 * (FTM_UINT64)1000000)
	{
		ullTimeUS = ullTimeUS -	(ulTimeH * 60 * 60 * (FTM_UINT64)1000);
	}
	else
	{
		ullTimeUS = 0;
	}

	xTimeR.xTimeval.tv_usec = ullTimeUS % 1000000;
	xTimeR.xTimeval.tv_sec = ullTimeUS / 1000000;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_subDays
(
	FTM_TIME_PTR 	pTime1, 
	FTM_UINT32		ulTimeD,
	FTM_TIME_PTR 	pTimeR
)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;
	FTM_UINT64	ullTimeUS;

	ullTimeUS = pTime1->xTimeval.tv_sec * (FTM_UINT64)1000000 + pTime1->xTimeval.tv_usec;
	if (ullTimeUS > ulTimeD * 60 * 60 * 24 * (FTM_UINT64)1000)
	{
		ullTimeUS = ullTimeUS -	(ulTimeD * (FTM_UINT64)1000);
	}
	else
	{
		ullTimeUS = 0;
	}

	xTimeR.xTimeval.tv_usec = ullTimeUS % 1000000;
	xTimeR.xTimeval.tv_sec = ullTimeUS / 1000000;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}
FTM_RET	FTM_TIME_isValid
(
	FTM_TIME_PTR	pTime,
	FTM_TIME_PTR	pBegin,
	FTM_TIME_PTR	pEnd
)
{
	ASSERT(pTime != NULL);
	ASSERT(pBegin != NULL);
	ASSERT(pEnd != NULL);

	if ((pBegin->xTimeval.tv_sec <= pTime->xTimeval.tv_sec) && (pTime->xTimeval.tv_sec < pEnd->xTimeval.tv_sec))
	{
		return	FTM_RET_OK;
	}


	return	FTM_RET_ERROR;
}

FTM_RET	FTM_TIME_align
(
	FTM_TIME_PTR	pTimeS,
	FTM_TIME_ALIGN	xAlign,
	FTM_TIME_PTR	pTimeR
)
{
	ASSERT(pTimeS != NULL);
	ASSERT(pTimeR != NULL);

	switch(xAlign)
	{
	case	FTM_TIME_ALIGN_1S:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_10S:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / 10 * 10;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_1M:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / 60 * 60;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_10M:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / 600 * 600;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_1H:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / 3600 * 3600;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_1D:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / (24 * 3600) * (24 * 3600);
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	default:
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTM_TIME_printf
(
	FTM_TIME_PTR	pTime,
	FTM_CHAR_PTR	pFormat
)
{
	ASSERT(pTime != NULL);

	static	FTM_CHAR	pString[64];

	if (pFormat != NULL)
	{
		strftime(pString, sizeof(pString) - 1, pFormat, localtime(&pTime->xTimeval.tv_sec));
	}
	else
	{
		strftime(pString, sizeof(pString) - 1, "%Y-%m-%d %H:%M:%S", localtime(&pTime->xTimeval.tv_sec));
	}

	return	pString;
}

FTM_CHAR_PTR	FTM_TIME_printf2
(
	FTM_UINT32		ulTime,
	FTM_CHAR_PTR	pFormat
)
{
	static	FTM_CHAR	pString[64];
	time_t	xTime	= (time_t)ulTime;

	if (pFormat != NULL)
	{
		strftime(pString, sizeof(pString) - 1, pFormat, localtime(&xTime));
	}
	else
	{
		strftime(pString, sizeof(pString) - 1, "%Y-%m-%d %H:%M:%S", localtime(&xTime));
	}

	return	pString;
}

FTM_CHAR_PTR	FTM_TIME_printfCurrent
(
	FTM_CHAR_PTR	pFormat
)
{
	struct timeval	xTimeval;

	static	FTM_CHAR	pString[64];

	gettimeofday(&xTimeval, NULL);

	time_t	xTime	= (time_t)xTimeval.tv_sec;

	if (pFormat != NULL)
	{
		strftime(pString, sizeof(pString) - 1, pFormat, localtime(&xTime));
	}
	else
	{
		strftime(pString, sizeof(pString) - 1, "%Y-%m-%d %H:%M:%S", localtime(&xTime));
	}

	return	pString;
}

FTM_RET	FTM_TIME_toMS
(
	FTM_TIME_PTR	pTime,
	FTM_UINT64_PTR	pullMS
)
{
	ASSERT(pTime != NULL);
	ASSERT(pullMS != NULL);

	*pullMS = pTime->xTimeval.tv_sec * (FTM_UINT64)1000 + pTime->xTimeval.tv_usec / 1000;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_toSecs
(
	FTM_TIME_PTR	pTime,
	FTM_UINT32_PTR	pulSecs
)
{
	ASSERT(pTime != NULL);
	ASSERT(pulSecs != NULL);

	*pulSecs = pTime->xTimeval.tv_sec;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_toDate
(
	FTM_TIME_PTR	pTime,
	FTM_DATE_PTR	pDate
)
{
	ASSERT(pTime != NULL);
	ASSERT(pDate != NULL);

	time_t	xTime;
	struct tm xTM;
	
	xTime = pTime->xTimeval.tv_sec;
	localtime_r(&xTime, &xTM);

	pDate->ulYear 	= xTM.tm_year;
	pDate->ulMon 	= xTM.tm_mon;
	pDate->ulDay	= xTM.tm_mday;
	pDate->ulHour	= xTM.tm_hour;
	pDate->ulMin	= xTM.tm_min;
	pDate->ulSec	= xTM.tm_sec;

	return	FTM_RET_OK;
}

FTM_RET	FTM_DATE_getCurrent
(
	FTM_DATE_PTR	pDate
)
{
	ASSERT(pDate != NULL);

	time_t	xTime;
	struct tm xTM;
	
	xTime = time(NULL);
	localtime_r(&xTime, &xTM);

	pDate->ulYear 	= xTM.tm_year;
	pDate->ulMon 	= xTM.tm_mon;
	pDate->ulDay	= xTM.tm_mday;
	pDate->ulHour	= xTM.tm_hour;
	pDate->ulMin	= xTM.tm_min;
	pDate->ulSec	= xTM.tm_sec;

	return	FTM_RET_OK;
}

FTM_RET	FTM_DATE_toTime
(
	FTM_DATE_PTR	pDate,
	FTM_TIME_PTR	pTime
)
{
	ASSERT(pDate != NULL);
	ASSERT(pTime != NULL);

	struct tm xTM;

	memset(&xTM, 0, sizeof(struct tm));

	xTM.tm_year = pDate->ulYear;
	xTM.tm_mon 	= pDate->ulMon;
	xTM.tm_mday	= pDate->ulDay;
	xTM.tm_hour	= pDate->ulHour;
	xTM.tm_min	= pDate->ulMin;
	xTM.tm_sec	= pDate->ulSec;

	pTime->xTimeval.tv_sec = mktime(&xTM);
	pTime->xTimeval.tv_usec = 0;

	return	FTM_RET_OK;
}

