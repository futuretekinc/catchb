#include "ftm_logger.h"
#include "ftm_trace.h"

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
	FTM_LOGGER_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	LOG("");
	LOG("[ Logger Configuration ]");
	LOG("%16s : %u", "Retention Period", pConfig->ulRetentionPeriod);

	return	FTM_RET_OK;
}

/////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////

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

