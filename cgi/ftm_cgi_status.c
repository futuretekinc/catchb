#include <string.h>
#include "cJSON/cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_json_utils.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

FTM_RET	FTM_CGI_GET_STATUS_setInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_SYSTEM_INFO	xInfo;
	FTM_UINT32	ulField = 0;
	cJSON _PTR_	pRoot;

	memset(&xInfo, 0, sizeof(xInfo));

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_CGI_getInterval(pReq, &xInfo.xStatistics.ulInterval, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulField |= FTM_SYSTEM_FIELD_STATISTICS_INTERVAL;
	}

	xRet = FTM_CGI_getCount(pReq, &xInfo.xStatistics.ulMaxCount, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulField |= FTM_SYSTEM_FIELD_STATISTICS_COUNT;
	}

	xRet = FTM_CLIENT_STAT_setInfo(pClient, pSSID, &xInfo, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set stat info!");
		goto finished;
	}

	cJSON _PTR_ pStat = cJSON_CreateObject();
	cJSON_AddNumberToObject(pStat, "interval", xInfo.xStatistics.ulInterval);
	cJSON_AddNumberToObject(pStat, "max count", xInfo.xStatistics.ulMaxCount);
	cJSON_AddNumberToObject(pStat, "count", xInfo.xStatistics.ulCount);
	cJSON_AddStringToObject(pStat, "first", FTM_TIME_printf2(xInfo.xStatistics.ulFirstTime, NULL));
	cJSON_AddStringToObject(pStat, "last", FTM_TIME_printf2(xInfo.xStatistics.ulLastTime, NULL));

	cJSON_AddItemToObject(pRoot, "status_info", pStat);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_STATUS_setInfo
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_SYSTEM_INFO	xInfo;
	FTM_UINT32	ulField = 0;

	memset(pSSID, 0, sizeof(pSSID));
	memset(&xInfo, 0, sizeof(xInfo));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_JSON_getInterval(pReqRoot, FTM_FALSE, &xInfo.xStatistics.ulInterval);
	if (xRet == FTM_RET_OK)
	{
		ulField |= FTM_SYSTEM_FIELD_STATISTICS_INTERVAL;
	}

	xRet = FTM_JSON_getCount(pReqRoot, FTM_FALSE, &xInfo.xStatistics.ulMaxCount);
	if (xRet == FTM_RET_OK)
	{
		ulField |= FTM_SYSTEM_FIELD_STATISTICS_COUNT;
	}

	xRet = FTM_CLIENT_STAT_setInfo(pClient, pSSID, &xInfo, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set stat info!");
		goto finished;
	}

	cJSON _PTR_ pStat = cJSON_CreateObject();
	cJSON_AddNumberToObject(pStat, "interval", xInfo.xStatistics.ulInterval);
	cJSON_AddNumberToObject(pStat, "max count", xInfo.xStatistics.ulMaxCount);
	cJSON_AddNumberToObject(pStat, "count", xInfo.xStatistics.ulCount);
	cJSON_AddStringToObject(pStat, "first", FTM_TIME_printf2(xInfo.xStatistics.ulFirstTime, NULL));
	cJSON_AddStringToObject(pStat, "last", FTM_TIME_printf2(xInfo.xStatistics.ulLastTime, NULL));

	cJSON_AddItemToObject(pRespRoot, "status_info", pStat);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_STATUS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_SYSTEM_INFO	xInfo;
	cJSON _PTR_	pRoot;

	memset(&xInfo, 0, sizeof(FTM_SYSTEM_INFO));

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_CLIENT_STAT_getInfo(pClient, pSSID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pStat = cJSON_CreateObject();
	cJSON_AddNumberToObject(pStat, "interval", xInfo.xStatistics.ulInterval);
	cJSON_AddNumberToObject(pStat, "max count", xInfo.xStatistics.ulMaxCount);
	cJSON_AddNumberToObject(pStat, "count", xInfo.xStatistics.ulCount);
	cJSON_AddStringToObject(pStat, "first", FTM_TIME_printf2(xInfo.xStatistics.ulFirstTime, NULL));
	cJSON_AddStringToObject(pStat, "last", FTM_TIME_printf2(xInfo.xStatistics.ulLastTime, NULL));

	cJSON_AddItemToObject(pRoot, "status_info", pStat);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_STATUS_getInfo
(
	FTM_CLIENT_PTR 	pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_SYSTEM_INFO	xInfo;

	memset(pSSID, 0, sizeof(pSSID));
	memset(&xInfo, 0, sizeof(FTM_SYSTEM_INFO));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_CLIENT_STAT_getInfo(pClient, pSSID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pStat = cJSON_CreateObject();
	cJSON_AddNumberToObject(pStat, "interval", xInfo.xStatistics.ulInterval);
	cJSON_AddNumberToObject(pStat, "max count", xInfo.xStatistics.ulMaxCount);
	cJSON_AddNumberToObject(pStat, "count", xInfo.xStatistics.ulCount);
	cJSON_AddStringToObject(pStat, "first", FTM_TIME_printf2(xInfo.xStatistics.ulFirstTime, NULL));
	cJSON_AddStringToObject(pStat, "last", FTM_TIME_printf2(xInfo.xStatistics.ulLastTime, NULL));

	cJSON_AddItemToObject(pRespRoot, "status_info", pStat);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_STATUS_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 0;
	FTM_UINT32	ulRemainCount = 0;
	FTM_UINT32	ulBeginTime = 0;
	FTM_UINT32	ulEndTime = 0;
	FTM_UINT32	ulFirstTime = 0;
	FTM_UINT32	ulLastTime = 0;
	FTM_BOOL	bPeriod = FTM_FALSE;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_CGI_getBeginTime(pReq, &ulBeginTime, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		bPeriod = FTM_TRUE;
	}

	xRet = FTM_CGI_getEndTime(pReq, &ulEndTime, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		bPeriod = FTM_TRUE;
	}

	if (bPeriod == FTM_FALSE)
	{
		xRet = FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
		xRet |= FTM_CGI_getCount(pReq, &ulCount, FTM_FALSE);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}
	}

	if (bPeriod == FTM_TRUE)
	{
		INFO("Begin : %d(%s)", ulBeginTime, FTM_TIME_printf2(ulBeginTime, NULL));
		INFO("End : %d(%s)", ulEndTime, FTM_TIME_printf2(ulEndTime, NULL));
		xRet = FTM_CLIENT_STAT_del2(pClient, pSSID, ulBeginTime, ulEndTime, &ulRemainCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}
	}
	else
	{
		xRet = FTM_CLIENT_STAT_del(pClient, pSSID, ulIndex, ulCount, &ulRemainCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}
	}

	cJSON _PTR_ pStat = cJSON_CreateObject();
	cJSON_AddNumberToObject(pStat, "count", ulRemainCount);
	cJSON_AddStringToObject(pStat, "first", FTM_TIME_printf2(ulFirstTime, NULL));
	cJSON_AddStringToObject(pStat, "last", FTM_TIME_printf2(ulLastTime, NULL));

	cJSON_AddItemToObject(pRoot, "status_info", pStat);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_STATUS_del
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 0;
	FTM_UINT32	ulRemainCount = 0;
	FTM_UINT32	ulBeginTime = 0;
	FTM_UINT32	ulEndTime = 0;
	FTM_UINT32	ulFirstTime = 0;
	FTM_UINT32	ulLastTime = 0;
	FTM_BOOL	bPeriod = FTM_FALSE;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_JSON_getBeginTime(pReqRoot, FTM_FALSE, &ulBeginTime);
	if (xRet == FTM_RET_OK)
	{
		bPeriod = FTM_TRUE;
	}

	xRet = FTM_JSON_getEndTime(pReqRoot, FTM_FALSE, &ulEndTime);
	if (xRet == FTM_RET_OK)
	{
		bPeriod = FTM_TRUE;
	}

	if (bPeriod == FTM_FALSE)
	{
		xRet = FTM_JSON_getIndex(pReqRoot, FTM_TRUE, &ulIndex);
		xRet |= FTM_JSON_getCount(pReqRoot, FTM_FALSE, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}
	}

	if (bPeriod == FTM_TRUE)
	{
		INFO("Begin : %d(%s)", ulBeginTime, FTM_TIME_printf2(ulBeginTime, NULL));
		INFO("End : %d(%s)", ulEndTime, FTM_TIME_printf2(ulEndTime, NULL));
		xRet = FTM_CLIENT_STAT_del2(pClient, pSSID, ulBeginTime, ulEndTime, &ulRemainCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}
	}
	else
	{
		xRet = FTM_CLIENT_STAT_del(pClient, pSSID, ulIndex, ulCount, &ulRemainCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}
	}

	cJSON _PTR_ pStat = cJSON_CreateObject();
	cJSON_AddNumberToObject(pStat, "count", ulRemainCount);
	cJSON_AddStringToObject(pStat, "first", FTM_TIME_printf2(ulFirstTime, NULL));
	cJSON_AddStringToObject(pStat, "last", FTM_TIME_printf2(ulLastTime, NULL));

	cJSON_AddItemToObject(pRespRoot, "status_info", pStat);
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_STATUS_getList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT		i;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_STATISTICS_PTR	pStatList = NULL;

	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	xRet |= FTM_CGI_getCount(pReq, &ulCount, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	pStatList = (FTM_STATISTICS_PTR)FTM_MEM_malloc(sizeof(FTM_STATISTICS) * ulCount);
	if (pStatList == NULL)
	{
		goto finished;	
	}

	xRet = FTM_CLIENT_STAT_getList(pClient, pSSID, ulIndex, ulCount, pStatList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pStatArray = cJSON_CreateArray();

	for(i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pStat = cJSON_CreateObject();

		cJSON_AddStringToObject(pStat, "time", FTM_TIME_printf(&pStatList[i].xTime, "%Y/%m/%d %H:%M:%S"));
		cJSON_AddNumberToObject(pStat, "cpu", pStatList[i].fCPU);
		cJSON_AddNumberToObject(pStat, "total memory", pStatList[i].xMemory.ulTotal);
		cJSON_AddNumberToObject(pStat, "free memory", pStatList[i].xMemory.ulFree);
		cJSON_AddNumberToObject(pStat, "Rx Bytes", pStatList[i].xNet.ulRxBytes);
		cJSON_AddNumberToObject(pStat, "Tx Bytes", pStatList[i].xNet.ulTxBytes);

		cJSON_AddItemToArray(pStatArray, pStat);
	}

	cJSON_AddItemToObject(pRoot, "status", pStatArray);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}


FTM_RET	FTM_CGI_POST_STATUS_getList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT		i;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_STATISTICS_PTR	pStatList = NULL;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_JSON_getIndex(pReqRoot, FTM_TRUE, &ulIndex);
	xRet |= FTM_JSON_getCount(pReqRoot, FTM_FALSE, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	pStatList = (FTM_STATISTICS_PTR)FTM_MEM_malloc(sizeof(FTM_STATISTICS) * ulCount);
	if (pStatList == NULL)
	{
		goto finished;	
	}

	xRet = FTM_CLIENT_STAT_getList(pClient, pSSID, ulIndex, ulCount, pStatList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pStatArray = cJSON_CreateArray();

	for(i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pStat = cJSON_CreateObject();

		cJSON_AddStringToObject(pStat, "time", FTM_TIME_printf(&pStatList[i].xTime, "%Y/%m/%d %H:%M:%S"));
		cJSON_AddNumberToObject(pStat, "cpu", pStatList[i].fCPU);
		cJSON_AddNumberToObject(pStat, "total memory", pStatList[i].xMemory.ulTotal);
		cJSON_AddNumberToObject(pStat, "free memory", pStatList[i].xMemory.ulFree);
		cJSON_AddNumberToObject(pStat, "Rx Bytes", pStatList[i].xNet.ulRxBytes);
		cJSON_AddNumberToObject(pStat, "Tx Bytes", pStatList[i].xNet.ulTxBytes);

		cJSON_AddItemToArray(pStatArray, pStat);
	}

	cJSON_AddItemToObject(pRespRoot, "status", pStatArray);

finished:

	return	xRet;
}


