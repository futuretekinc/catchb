#include <string.h>
#include <pthread.h>
#include "ftm_catchb.h"
#include "ftm_logger.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_message.h"
#include "ftm_time.h"
#include "ftm_timer.h"

#undef	__MODULE__
#define	__MODULE__	"logger"

FTM_VOID_PTR	FTM_LOGGER_threadMain
(
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_LOGGER_CONFIG_setDefault
(
	FTM_LOGGER_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	pConfig->ulRetentionPeriod = FTM_CATCHB_LOGGER_DEFAULT_RETENTION;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOGGER_CONFIG_load
(
	FTM_LOGGER_CONFIG_PTR	pConfig,
	cJSON _PTR_			pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	cJSON _PTR_ pItem;

	pItem = cJSON_GetObjectItem(pRoot, "retention");
	if (pItem != NULL)
	{ 
		if (pItem->type == cJSON_Number)
		{
			pConfig->ulRetentionPeriod = pItem->valueint;
		}
		else
		{
			xRet = FTM_RET_INVALID_JSON_FORMAT;
		}
	}

	return	xRet;
}

FTM_RET	FTM_LOGGER_CONFIG_save
(
	FTM_LOGGER_CONFIG_PTR	pConfig,
	cJSON _PTR_			pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_LOGGER_CONFIG_show
(
	FTM_LOGGER_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
)
{
	ASSERT(pConfig != NULL);

	OUTPUT(xLevel, "");
	OUTPUT(xLevel, "[ Logger Configuration ]");
	OUTPUT(xLevel, "%16s : %u", "Retention Period", pConfig->ulRetentionPeriod);

	return	FTM_RET_OK;
}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

FTM_RET	FTM_LOGGER_create
(
	struct FTM_CATCHB_STRUCT _PTR_ pCatchB,
	FTM_LOGGER_PTR _PTR_ ppLogger
)
{
	ASSERT(pCatchB != NULL);
	ASSERT(ppLogger != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_LOGGER_PTR	pLogger = NULL;

	pLogger = (FTM_LOGGER_PTR)FTM_MEM_malloc(sizeof(FTM_LOGGER));
	if (pLogger == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create logger!");
		goto finished;
	}

	strncpy(pLogger->pName, __MODULE__, sizeof(pLogger->pName)-1);

	xRet = FTM_MSGQ_create(&pLogger->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create logger!");
		goto finished;
	}

	pLogger->bStop = FTM_TRUE;
	pLogger->pCatchB = pCatchB;

	*ppLogger = pLogger;

finished:
	if (xRet != FTM_RET_OK)
	{
		if (pLogger != NULL)
		{
			FTM_MEM_free(pLogger);
		}
	}

	return	xRet;
}

FTM_RET	FTM_LOGGER_destroy
(
	FTM_LOGGER_PTR _PTR_ ppLogger
)
{
	ASSERT(ppLogger != NULL);

	if ((*ppLogger) != NULL)
	{
		FTM_LOGGER_stop((*ppLogger));

		if ((*ppLogger)->pMsgQ != NULL)
		{
			FTM_MSGQ_destroy(&(*ppLogger)->pMsgQ);
		}

		FTM_MEM_free(*ppLogger);

		*ppLogger = NULL;
	}


	return	FTM_RET_OK;
}

FTM_RET	FTM_LOGGER_setConfig
(
	FTM_LOGGER_PTR	pLogger,
	FTM_LOGGER_CONFIG_PTR	pConfig
)
{
	ASSERT(pLogger != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	if (!pLogger->bStop)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
		ERROR(xRet, "Failed to set logger configuration!");
	}
	else
	{
		memcpy(&pLogger->xConfig, pConfig, sizeof(FTM_LOGGER_CONFIG));
	}

	return	xRet;
}

FTM_RET	FTM_LOGGER_start
(
	FTM_LOGGER_PTR	pLogger
)
{
	ASSERT(pLogger != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	if (pLogger->xThread != 0)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
		INFO("The %s is already running!", pLogger->pName);	
		return	xRet;
	}

	if (pthread_create(&pLogger->xThread, NULL, FTM_LOGGER_threadMain, (FTM_VOID_PTR)pLogger) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		INFO("Failed to start %s!", pLogger->pName);
	}

    return xRet;
}

FTM_RET	FTM_LOGGER_stop
(
	FTM_LOGGER_PTR	pLogger
)
{
	ASSERT(pLogger != NULL);
	
	FTM_RET	xRet = FTM_RET_OK;

	if (!pLogger->bStop)
	{
		FTM_MSGQ_pushQuit(pLogger->pMsgQ);

		pLogger->bStop = FTM_TRUE;
		pthread_join(pLogger->xThread, NULL);
		pLogger->xThread = 0;
	}

	return	xRet;
}

FTM_VOID_PTR	FTM_LOGGER_threadMain
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_LOGGER_PTR	pLogger = (FTM_LOGGER_PTR)pData;
	FTM_DATE	xDate;
	FTM_TIME	xCurrentTime;
	FTM_TIME	xTime;
	FTM_TIME	xDiffTime;
	FTM_TIMER	xLogExpireTimer;

	INFO("%s started", pLogger->pName);

	FTM_CATCHB_removeExpiredLog(pLogger->pCatchB, pLogger->xConfig.ulRetentionPeriod);

	FTM_TIME_getCurrent(&xCurrentTime);
	FTM_DATE_getCurrent(&xDate);

	xDate.ulHour = 0;
	xDate.ulMin = 0;
	xDate.ulSec = 0;
	FTM_DATE_toTime(&xDate, &xTime);
	FTM_TIME_addSecs(&xTime, FTM_TIME_SECONDS_OF_DAY, &xTime);
	FTM_TIME_sub(&xTime, &xCurrentTime, &xDiffTime);

	FTM_TIMER_initTime(&xLogExpireTimer, &xDiffTime);

	pLogger->bStop = FTM_FALSE;

	while(!pLogger->bStop)
	{
		FTM_MSG_PTR	pRcvdMsg;

		xRet = FTM_MSGQ_timedPop(pLogger->pMsgQ, 10, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pRcvdMsg->xType)
			{
			case	FTM_MSG_TYPE_QUIT:
				{
					pLogger->bStop = FTM_TRUE;
				}
				break;

			default:
				{
					INFO("Unknown message[%s]", FTM_MESSAGE_getString(pRcvdMsg->xType));	
				}
			}

			FTM_MEM_free(pRcvdMsg);
		}

		if (FTM_TIMER_isExpired(&xLogExpireTimer) == FTM_TRUE)
		{
			FTM_CATCHB_removeExpiredLog(pLogger->pCatchB, pLogger->xConfig.ulRetentionPeriod);
			FTM_TIMER_addS(&xLogExpireTimer, FTM_TIME_SECONDS_OF_DAY);
		}
	}

	INFO("%s stopped", pLogger->pName);

	return	0;
}
