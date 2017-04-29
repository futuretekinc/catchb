#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"

FTM_RET	FTM_CGI_getStatInfo
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

	xRet = FTM_CLIENT_getStatInfo(pClient, &ulCount, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pStat = cJSON_CreateObject();
	cJSON_AddNumberToObject(pStat, "count", ulCount);
	cJSON_AddStringToObject(pStat, "first", FTM_TIME_printf2(ulFirstTime, NULL));
	cJSON_AddStringToObject(pStat, "last", FTM_TIME_printf2(ulLastTime, NULL));

	cJSON_AddItemToObject(pRoot, "log_info", pStat);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_delStat
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

	xRet = FTM_CLIENT_delStat(pClient, ulIndex, ulCount, &ulRemainCount, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pStat = cJSON_CreateObject();
	cJSON_AddNumberToObject(pStat, "count", ulRemainCount);
	cJSON_AddNumberToObject(pStat, "first", ulFirstTime);
	cJSON_AddNumberToObject(pStat, "last", ulLastTime);

	cJSON_AddItemToObject(pRoot, "log_info", pStat);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_getStatList
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
	FTM_STATISTICS_PTR	pStatList = NULL;

	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

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

	xRet = FTM_CLIENT_getStatList(pClient, ulIndex, ulCount, pStatList, &ulCount);
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

	cJSON_AddItemToObject(pRoot, "statistics", pStatArray);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}


