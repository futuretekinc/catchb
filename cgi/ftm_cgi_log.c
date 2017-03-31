#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"

FTM_RET	FTM_CGI_getLogInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_UINT32	ulCount = 0;
	FTM_UINT32	ulFirstTime = 0;
	FTM_UINT32	ulLastTime = 0;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTM_CLIENT_getLogInfo(pClient, &ulCount, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pLog = cJSON_CreateObject();
	cJSON_AddNumberToObject(pLog, "count", ulCount);
	cJSON_AddStringToObject(pLog, "first", FTM_TIME_printf2(ulFirstTime, NULL));
	cJSON_AddStringToObject(pLog, "last", FTM_TIME_printf2(ulLastTime, NULL));

	cJSON_AddItemToObject(pRoot, "log_info", pLog);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_delLog
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 0;
	FTM_UINT32	ulRemainCount = 0;
	FTM_UINT32	ulFirstTime = 0;
	FTM_UINT32	ulLastTime = 0;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	xRet |= FTM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_delLog(pClient, ulIndex, ulCount, &ulRemainCount, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pLog = cJSON_CreateObject();
	cJSON_AddNumberToObject(pLog, "count", ulRemainCount);
	cJSON_AddNumberToObject(pLog, "first", ulFirstTime);
	cJSON_AddNumberToObject(pLog, "last", ulLastTime);

	cJSON_AddItemToObject(pRoot, "log_info", pLog);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_getLogList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT		i;
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_LOG_PTR	pLogList = NULL;

	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	xRet |= FTM_CGI_getCount(pReq, &ulCount, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	pLogList = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG) * ulCount);
	if (pLogList == NULL)
	{
		goto finished;	
	}

	xRet = FTM_CLIENT_getLogList(pClient, ulIndex, ulCount, pLogList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pLogArray = cJSON_CreateArray();

	for(i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pLog = cJSON_CreateObject();

		if (pLogList[i].xType == FTM_LOG_TYPE_NORMAL)
		{
			cJSON_AddStringToObject(pLog, "type", "normal");
		}
		else
		{
			cJSON_AddStringToObject(pLog, "type", "error");
		}

		cJSON_AddStringToObject(pLog, "time", FTM_TIME_printf2(pLogList[i].ulTime, NULL));
		cJSON_AddStringToObject(pLog, "id", pLogList[i].pID);
		cJSON_AddStringToObject(pLog, "ip", pLogList[i].pIP);
		cJSON_AddStringToObject(pLog, "hash", pLogList[i].pHash);
		cJSON_AddStringToObject(pLog, "stat", FTM_CCTV_STAT_print(pLogList[i].xStat));

		cJSON_AddItemToArray(pLogArray, pLog);
	}

	cJSON_AddItemToObject(pRoot, "logs", pLogArray);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}


