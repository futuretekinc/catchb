#include "ftm_system.h"
#include "ftm_default.h"

FTM_RET	FTM_SYSTEM_CONFIG_setDefault
(
	FTM_SYSTEM_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	pConfig->xCCTV.ulUpdateInterval = 10000;
	pConfig->xStatistics.ulCount	= FTM_CATCHB_STATISTICS_COUNT;
	pConfig->xStatistics.ulInterval = FTM_CATCHB_STATISTICS_UPDATE_INTERVAL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SYSTEM_CONFIG_load
(
	FTM_SYSTEM_CONFIG_PTR	pConfig,
	cJSON _PTR_ 		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pSection;
	cJSON _PTR_ pItem;

	pSection = cJSON_GetObjectItem(pRoot, "statistics");
	if (pSection != NULL)
	{
		pItem = cJSON_GetObjectItem(pSection, "count");
		if ((pItem != NULL) && (pItem->type == cJSON_Number))
		{
			if ((pItem->valueint >= FTM_CATCHB_STATISTICS_COUNT) && (pItem->valueint <= FTM_CATCHB_STATISTICS_COUNT))
			{
				pConfig->xStatistics.ulCount = pItem->valueint;
			}
		}

		pItem = cJSON_GetObjectItem(pSection, "interval");
		if ((pItem != NULL) && (pItem->type == cJSON_Number))
		{
			if ((pItem->valueint >= FTM_CATCHB_STATISTICS_UPDATE_INTERVAL_MIN) && (pItem->valueint <= FTM_CATCHB_STATISTICS_UPDATE_INTERVAL_MAX))
			{
				pConfig->xStatistics.ulInterval = pItem->valueint;
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SYSTEM_CONFIG_save
(
	FTM_SYSTEM_CONFIG_PTR	pConfig,
	cJSON _PTR_ 		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pSection;

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		cJSON_AddNumberToObject(pSection, "count", pConfig->xStatistics.ulCount);
		cJSON_AddNumberToObject(pSection, "interval", pConfig->xStatistics.ulInterval);

		cJSON_AddItemToObject(pRoot, "statistics", pSection);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SYSTEM_CONFIG_show
(
	FTM_SYSTEM_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL		xLevel
)
{
	ASSERT(pConfig != NULL);

	printf("\n[ System Configuration ]\n");
	printf("%16s : %u\n", "Max Count", pConfig->xStatistics.ulCount);
	printf("%16s : %u s\n", "Update Interval", pConfig->xStatistics.ulInterval / 1000);

	return	FTM_RET_OK;
}

