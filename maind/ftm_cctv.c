#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_cctv.h"

FTM_RET	FTM_CCTV_create
(
	FTM_CCTV_CONFIG_PTR	pConfig,
	FTM_CCTV_PTR _PTR_ ppCCTV
)
{
	ASSERT(ppCCTV != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_CCTV_PTR	pCCTV;

	pCCTV = (FTM_CCTV_PTR)FTM_MEM_malloc(sizeof(FTM_CCTV));
	if (pCCTV == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	if (pConfig != NULL)
	{
		memcpy(&pCCTV->xConfig, pConfig, sizeof(FTM_CCTV_CONFIG));	
	}
	else
	{
		pCCTV->xConfig.xStat = FTM_CCTV_STAT_UNREGISTERED;
	}

	xRet = FTM_LOCK_init(&pCCTV->xLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to init lock!");
		goto finished;
	}

	xRet = FTM_TIMER_initS(&pCCTV->xExpiredTimer, 0);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to init timer!");
		goto finished;
	}

	*ppCCTV = pCCTV;

finished:
	if (xRet != FTM_RET_OK)
	{
		if (pCCTV != NULL)
		{
			FTM_MEM_free(pCCTV);	
		}
	}

	return	xRet;
}

FTM_RET	FTM_CCTV_destroy
(
	FTM_CCTV_PTR _PTR_ ppCCTV
)
{
	ASSERT(ppCCTV != NULL);
	ASSERT(*ppCCTV != NULL);

	if (*ppCCTV != NULL)
	{
		FTM_LOCK_final(&(*ppCCTV)->xLock);

		FTM_MEM_free(*ppCCTV);

		*ppCCTV = NULL;
	}


	return	FTM_RET_OK;
}

FTM_RET	FTM_CCTV_lock
(
	FTM_CCTV_PTR pCCTV
)
{
	ASSERT(pCCTV != NULL);

	FTM_LOCK_set(&pCCTV->xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CCTV_unlock
(
	FTM_CCTV_PTR pCCTV
)
{
	ASSERT(pCCTV != NULL);

	FTM_LOCK_reset(&pCCTV->xLock);

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTM_CCTV_STAT_print
(
	FTM_CCTV_STAT	xStat
)
{
	switch(xStat)
	{
	case	FTM_CCTV_STAT_UNREGISTERED:	return	"UNREGISTERED";
	case	FTM_CCTV_STAT_UNUSED:		return	"UNUSED";
	case	FTM_CCTV_STAT_NORMAL:		return	"NORMAL";
	case	FTM_CCTV_STAT_ABNORMAL:		return	"ABNORMAL";
	default:	return	"UNKNOWN";	
	}

}
