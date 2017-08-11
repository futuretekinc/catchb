#include <string.h>
#include "cJSON/cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_json_utils.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

FTM_RET	FTM_CGI_GET_LOG_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulCount = 0;
	FTM_UINT32	ulFirstTime = 0;
	FTM_UINT32	ulLastTime = 0;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_CLIENT_LOG_getInfo(pClient, pSSID, &ulCount, &ulFirstTime, &ulLastTime);
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

FTM_RET	FTM_CGI_POST_LOG_getInfo
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
	FTM_UINT32	ulCount = 0;
	FTM_UINT32	ulFirstTime = 0;
	FTM_UINT32	ulLastTime = 0;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_CLIENT_LOG_getInfo(pClient, pSSID, &ulCount, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pLog = cJSON_CreateObject();
	cJSON_AddNumberToObject(pLog, "count", ulCount);
	cJSON_AddStringToObject(pLog, "first", FTM_TIME_printf2(ulFirstTime, NULL));
	cJSON_AddStringToObject(pLog, "last", FTM_TIME_printf2(ulLastTime, NULL));

	cJSON_AddItemToObject(pRespRoot, "log_info", pLog);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_LOG_del
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
	FTM_UINT32	ulFirstTime = 0;
	FTM_UINT32	ulLastTime = 0;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	xRet |= FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	xRet |= FTM_CGI_getCount(pReq, &ulCount, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_LOG_del(pClient, pSSID, ulIndex, ulCount, &ulRemainCount, &ulFirstTime, &ulLastTime);
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

FTM_RET	FTM_CGI_POST_LOG_del
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
	FTM_UINT32	ulFirstTime = 0;
	FTM_UINT32	ulLastTime = 0;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	xRet |= FTM_JSON_getIndex(pReqRoot, FTM_TRUE, &ulIndex);
	xRet |= FTM_JSON_getCount(pReqRoot, FTM_FALSE, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_LOG_del(pClient, pSSID, ulIndex, ulCount, &ulRemainCount, &ulFirstTime, &ulLastTime);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pLog = cJSON_CreateObject();
	cJSON_AddNumberToObject(pLog, "count", ulRemainCount);
	cJSON_AddNumberToObject(pLog, "first", ulFirstTime);
	cJSON_AddNumberToObject(pLog, "last", ulLastTime);

	cJSON_AddItemToObject(pRespRoot, "log_info", pLog);
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_LOG_getList
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
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_LOG_TYPE	xLogType = FTM_LOG_TYPE_UNKNOWN;
	FTM_CCTV_STAT	xStat = FTM_CCTV_STAT_UNREGISTERED;
	FTM_UINT32	ulBeginTime = 0;
	FTM_UINT32	ulEndTime = 0;
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_LOG_PTR	pLogList = NULL;

	cJSON _PTR_	pRoot;

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));
	memset(pIP, 0, sizeof(pIP));

	pRoot = cJSON_CreateObject();

	xRet = FTM_CGI_getID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get SSID");
	}
	xRet = FTM_CGI_getLogType(pReq, &xLogType, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get Log type");
	}
	xRet = FTM_CGI_getID(pReq, pID, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get ID");
	}
	xRet = FTM_CGI_getIPString(pReq, pIP, FTM_IP_LEN, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get IP");
	}
	xRet = FTM_CGI_getBeginTime(pReq, &ulBeginTime, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get Begin Time");
	}
	xRet = FTM_CGI_getEndTime(pReq, &ulEndTime, FTM_TRUE);
	INFO("xRet = %d", xRet);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get End Time");
	}
	xRet = FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	INFO("xRet = %d", xRet);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get Index");
	}
	xRet = FTM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid arguments!");
		goto finished;
	}

	pLogList = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG) * ulCount);
	if (pLogList == NULL)
	{
		ERROR(xRet, "Failed to get log because not enough memory!");
		goto finished;	
	}

	xRet = FTM_CLIENT_LOG_getList(pClient, pSSID, xLogType, pID, pIP, xStat, ulBeginTime, ulEndTime, ulIndex, ulCount, pLogList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log!");
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


FTM_RET	FTM_CGI_POST_LOG_getList
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
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_LOG_TYPE	xLogType = FTM_LOG_TYPE_UNKNOWN;
	FTM_CCTV_STAT	xStat = FTM_CCTV_STAT_UNREGISTERED;
	FTM_UINT32	ulBeginTime = 0;
	FTM_UINT32	ulEndTime = 0;
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_LOG_PTR	pLogList = NULL;

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));
	memset(pIP, 0, sizeof(pIP));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get SSID");
	}
	xRet = FTM_JSON_getLogType(pReqRoot, FTM_TRUE, &xLogType);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get Log type");
	}
	xRet = FTM_JSON_getID(pReqRoot, FTM_TRUE, pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get ID");
	}
	xRet = FTM_JSON_getIP(pReqRoot, FTM_TRUE, pIP);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get IP");
	}
	xRet = FTM_JSON_getBeginTime(pReqRoot, FTM_TRUE, &ulBeginTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get Begin Time");
	}
	xRet = FTM_JSON_getEndTime(pReqRoot, FTM_TRUE, &ulEndTime);
	INFO("xRet = %d", xRet);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get End Time");
	}
	xRet = FTM_JSON_getIndex(pReqRoot, FTM_TRUE, &ulIndex);
	INFO("xRet = %d", xRet);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get Index");
	}
	xRet = FTM_JSON_getCount(pReqRoot, FTM_TRUE, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid arguments!");
		goto finished;
	}

	pLogList = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG) * ulCount);
	if (pLogList == NULL)
	{
		ERROR(xRet, "Failed to get log because not enough memory!");
		goto finished;	
	}

	xRet = FTM_CLIENT_LOG_getList(pClient, pSSID, xLogType, pID, pIP, xStat, ulBeginTime, ulEndTime, ulIndex, ulCount, pLogList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log!");
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

	cJSON_AddItemToObject(pRespRoot, "logs", pLogArray);

finished:

	return	xRet;
}


FTM_RET	FTM_CGI_POST_LOG_getCount2
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
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_LOG_TYPE	xLogType = FTM_LOG_TYPE_UNKNOWN;
	FTM_CCTV_STAT	xStat = FTM_CCTV_STAT_UNREGISTERED;
	FTM_UINT32	ulBeginTime = 0;
	FTM_UINT32	ulEndTime = 0;
	FTM_UINT32	ulCount = 0;

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));
	memset(pIP, 0, sizeof(pIP));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get SSID");
	}
	xRet = FTM_JSON_getLogType(pReqRoot, FTM_TRUE, &xLogType);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get Log type");
	}
	xRet = FTM_JSON_getID(pReqRoot, FTM_TRUE, pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get ID");
	}
	xRet = FTM_JSON_getIP(pReqRoot, FTM_TRUE, pIP);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get IP");
	}
	xRet = FTM_JSON_getBeginTime(pReqRoot, FTM_TRUE, &ulBeginTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get Begin Time");
	}
	xRet = FTM_JSON_getEndTime(pReqRoot, FTM_TRUE, &ulEndTime);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get End Time");
	}

	xRet = FTM_CLIENT_LOG_getCount2(pClient, pSSID, xLogType, pID, pIP, xStat, ulBeginTime, ulEndTime, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log!");
		goto finished;
	}

	cJSON_AddNumberToObject(pRespRoot, "count", ulCount);

finished:

	return	xRet;
}


