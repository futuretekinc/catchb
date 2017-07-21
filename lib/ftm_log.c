#include "ftm_log.h"
#include "ftm_time.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"

FTM_CHAR_PTR	FTM_LOG_TYPE_print
(
	FTM_LOG_TYPE	xType
)
{
	switch(xType)
	{
	case	FTM_LOG_TYPE_NORMAL:	return	"Normal";
	case	FTM_LOG_TYPE_ERROR:	return	"Error";
	default:
		return	"Unknown";
	}

}

FTM_RET	FTM_LOG_add
(
	FTM_LOG_TYPE	xType,
	FTM_UINT32		ulTime,
	FTM_CHAR_PTR	pCCTVID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_CHAR_PTR	pLog
)
{
	FTM_RET	xRet;

	if (pCatchB == NULL)
	{
		xRet = FTM_RET_NOT_INITIALIZED;
		ERROR(xRet, "CatchB not initialized!");
		return	xRet;
	}

	if (ulTime == 0)
	{
		FTM_TIME_getCurrentSecs(&ulTime);	
	}

	INFO("Add Log : %s, %s, %s, %d, %s", FTM_TIME_printf2(ulTime, NULL), pCCTVID, pIP, xStat, pLog);

	return	FTM_DB_addLog(pCatchB->pDB, xType, ulTime, pCCTVID, pIP, xStat, pLog);
}
