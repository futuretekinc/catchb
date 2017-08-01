#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_json_utils.h"

#undef	__MODULE__
#define	__MODULE__ "cgi"

FTM_RET	FTM_CGI_GET_CCTV_add
(
	FTM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_CHAR	pSwitchID[FTM_ID_LEN+1];
	FTM_CHAR	pComment[FTM_COMMENT_LEN+1];
	FTM_CCTV_CONFIG	xConfig;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pID, 0, sizeof(pID));
	memset(pIP, 0, sizeof(pIP));
	memset(pSwitchID, 0, sizeof(pSwitchID));
	memset(pComment, 0, sizeof(pComment));

	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	xRet |= FTM_CGI_getID(pReq, pID, FTM_FALSE);
	xRet |= FTM_CGI_getIPString(pReq, pIP, FTM_IP_LEN, FTM_TRUE);
	xRet |= FTM_CGI_getSwitchID(pReq, pSwitchID, FTM_TRUE);
	xRet |= FTM_CGI_getComment(pReq, pComment, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create CCTV.!");
		goto finish;
	}

	xRet = FTM_CLIENT_CCTV_add(pClient, pSSID, pID, pIP, pSwitchID, pComment);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add CCTV.!");
		cJSON_AddStringToObject(pRoot, "error", "Failed to add CCTV.!");
		goto finish;
	}

	xRet = FTM_CLIENT_CCTV_getProperties(pClient, pSSID, pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV properties.!");
		cJSON_AddStringToObject(pRoot, "error", "Failed to get CCTV properties.!");
		goto finish;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", xConfig.pID);
	cJSON_AddStringToObject(pCCTV, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pCCTV, "switchid", xConfig.pSwitchID);
	cJSON_AddStringToObject(pCCTV, "comment", xConfig.pComment);
//	cJSON_AddStringToObject(pCCTV, "hash", xConfig.pHash);
//	cJSON_AddStringToObject(pCCTV, "time", FTM_TIME_printf2(xConfig.ulTime, NULL));
//	cJSON_AddStringToObject(pCCTV, "stat", FTM_CCTV_STAT_print(xConfig.xStat));

	cJSON_AddItemToObject(pRoot, "cctv", pCCTV);

finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_CCTV_add
(
	FTM_CLIENT_PTR pClient,
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_INVALID_ARGUMENTS;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_CHAR	pSwitchID[FTM_ID_LEN+1];
	FTM_CHAR	pComment[FTM_COMMENT_LEN+1];
	FTM_CCTV_CONFIG	xConfig;

	memset(pID, 0, sizeof(pID));
	memset(pIP, 0, sizeof(pIP));
	memset(pSwitchID, 0, sizeof(pSwitchID));
	memset(pComment, 0, sizeof(pComment));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	xRet |= FTM_JSON_getID(pReqRoot, FTM_FALSE, pID);
	xRet |= FTM_JSON_getIP(pReqRoot, FTM_TRUE, pIP);
	xRet |= FTM_JSON_getSwitchID(pReqRoot, FTM_TRUE, pSwitchID);
	xRet |= FTM_JSON_getComment(pReqRoot, FTM_TRUE, pComment);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		ERROR(xRet, "Failed to create CCTV.!");
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_add(pClient, pSSID, pID, pIP, pSwitchID, pComment);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add CCTV.!");
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_getProperties(pClient, pSSID, pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get CCTV properties.!");
		goto finished;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", xConfig.pID);
	cJSON_AddStringToObject(pCCTV, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pCCTV, "switchid", xConfig.pSwitchID);
	cJSON_AddStringToObject(pCCTV, "comment", xConfig.pComment);
//	cJSON_AddStringToObject(pCCTV, "hash", xConfig.pHash);
//	cJSON_AddStringToObject(pCCTV, "time", FTM_TIME_printf2(xConfig.ulTime, NULL));
//	cJSON_AddStringToObject(pCCTV, "stat", FTM_CCTV_STAT_print(xConfig.xStat));

	cJSON_AddItemToObject(pRespRoot, "cctv", pCCTV);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_CCTV_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pID, 0, sizeof(pID));

	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	xRet |= FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_del(pClient, pSSID, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", pID);

	cJSON_AddItemToObject(pRoot, "cctv", pCCTV);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_CCTV_del
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
	FTM_CHAR	pID[FTM_ID_LEN+1];

	memset(pID, 0, sizeof(pID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	xRet |= FTM_JSON_getID(pReqRoot, FTM_FALSE, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_del(pClient, pSSID, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", pID);

	cJSON_AddItemToObject(pRespRoot, "cctv", pCCTV);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_CCTV_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CCTV_CONFIG	xConfig;
	cJSON _PTR_	pRoot;

	memset(pID, 0, sizeof(pID));

	pRoot = cJSON_CreateObject();

	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	xRet |= FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "CCTV ID not found!");
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_getProperties(pClient, pSSID, pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "CCTV[%s] not found!", pID);
		goto finished;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", xConfig.pID);
	cJSON_AddStringToObject(pCCTV, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pCCTV, "switchid", xConfig.pSwitchID);
	cJSON_AddStringToObject(pCCTV, "comment", xConfig.pComment);
	cJSON_AddStringToObject(pCCTV, "hash", xConfig.pHash);
	cJSON_AddStringToObject(pCCTV, "time", FTM_TIME_printf2(xConfig.ulTime, NULL));
	cJSON_AddStringToObject(pCCTV, "stat", FTM_CCTV_STAT_print(xConfig.xStat));

	cJSON_AddItemToObject(pRoot, "cctv", pCCTV);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_CCTV_get
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
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CCTV_CONFIG	xConfig;

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	xRet |= FTM_JSON_getID(pReqRoot, FTM_FALSE, pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "CCTV ID not found!");
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_getProperties(pClient, pSSID, pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "CCTV[%s] not found!", pID);
		goto finished;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", xConfig.pID);
	cJSON_AddStringToObject(pCCTV, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pCCTV, "switchid", xConfig.pSwitchID);
	cJSON_AddStringToObject(pCCTV, "comment", xConfig.pComment);
	cJSON_AddStringToObject(pCCTV, "hash", xConfig.pHash);
	cJSON_AddStringToObject(pCCTV, "time", FTM_TIME_printf2(xConfig.ulTime, NULL));
	cJSON_AddStringToObject(pCCTV, "stat", FTM_CCTV_STAT_print(xConfig.xStat));

	cJSON_AddItemToObject(pRespRoot, "cctv", pCCTV);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_CCTV_set
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulFieldFlags = 0;
	FTM_CCTV_CONFIG	xConfig;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	xRet |= FTM_CGI_getID(pReq, xConfig.pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CGI_getIPString(pReq, xConfig.pIP, FTM_IP_LEN, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_CCTV_FIELD_IP;
	}

	xRet = FTM_CGI_getSwitchID(pReq, xConfig.pSwitchID, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_CCTV_FIELD_SWITCH_ID;
	}

	xRet = FTM_CGI_getComment(pReq, xConfig.pComment, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_CCTV_FIELD_COMMENT;
	}


	xRet = FTM_CLIENT_CCTV_setProperties(pClient, pSSID, xConfig.pID, &xConfig, ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_CCTV_getProperties(pClient, pSSID, xConfig.pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", xConfig.pID);
	cJSON_AddStringToObject(pCCTV, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pCCTV, "switchid", xConfig.pSwitchID);
	cJSON_AddStringToObject(pCCTV, "comment", xConfig.pComment);
	cJSON_AddStringToObject(pCCTV, "hash", xConfig.pHash);
	cJSON_AddStringToObject(pCCTV, "time", FTM_TIME_printf2(xConfig.ulTime, NULL));
	cJSON_AddStringToObject(pCCTV, "stat", FTM_CCTV_STAT_print(xConfig.xStat));

	cJSON_AddItemToObject(pRoot, "cctv", pCCTV);
finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_CCTV_set
(
	FTM_CLIENT_PTR	pClient,
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulFieldFlags = 0;
	FTM_CCTV_CONFIG	xConfig;

	memset(pSSID, 0, sizeof(pSSID));
	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	xRet |= FTM_JSON_getID(pReqRoot, FTM_FALSE, xConfig.pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_JSON_getIP(pReqRoot, FTM_FALSE, xConfig.pIP);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_CCTV_FIELD_IP;
	}

	xRet = FTM_JSON_getSwitchID(pReqRoot, FTM_FALSE, xConfig.pSwitchID);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_CCTV_FIELD_SWITCH_ID;
	}

	xRet = FTM_JSON_getComment(pReqRoot, FTM_FALSE, xConfig.pComment);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_CCTV_FIELD_COMMENT;
	}


	xRet = FTM_CLIENT_CCTV_setProperties(pClient, pSSID, xConfig.pID, &xConfig, ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_getProperties(pClient, pSSID, xConfig.pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", xConfig.pID);
	cJSON_AddStringToObject(pCCTV, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pCCTV, "switchid", xConfig.pSwitchID);
	cJSON_AddStringToObject(pCCTV, "comment", xConfig.pComment);
	cJSON_AddStringToObject(pCCTV, "hash", xConfig.pHash);
	cJSON_AddStringToObject(pCCTV, "time", FTM_TIME_printf2(xConfig.ulTime, NULL));
	cJSON_AddStringToObject(pCCTV, "stat", FTM_CCTV_STAT_print(xConfig.xStat));

	cJSON_AddItemToObject(pRespRoot, "cctv", pCCTV);
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_CCTV_getIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	i = 0;
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_ID_PTR	pIDList = NULL;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	pIDList = (FTM_ID_PTR)FTM_MEM_malloc(sizeof(FTM_ID) * ulCount);
	if (pIDList == NULL)
	{
		goto finish;			
	}

	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	xRet |= FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	xRet |= FTM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_CCTV_getIDList(pClient, pSSID, ulCount, pIDList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pIDArray;

	pIDArray = cJSON_CreateArray();

	for(i = 0 ; i < ulCount ; i++)
	{
		cJSON_AddItemToArray(pIDArray, cJSON_CreateString(pIDList[i]));
	}

	cJSON_AddItemToObject(pRoot, "id_list", pIDArray);

finish:

	if (pIDList != NULL)
	{
		FTM_MEM_free(pIDList);
	}

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_CCTV_getIDList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReqRoot != NULL);
	ASSERT(pRespRoot != NULL);

	FTM_RET		xRet = FTM_RET_INVALID_ARGUMENTS;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	i = 0;
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_ID_PTR	pIDList = NULL;

	memset(pSSID, 0, sizeof(pSSID));

	pIDList = (FTM_ID_PTR)FTM_MEM_malloc(sizeof(FTM_ID) * ulCount);
	if (pIDList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to get id list with not enough memory!");
		goto finished;
	}

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	xRet |= FTM_JSON_getIndex(pReqRoot, FTM_TRUE, &ulIndex);
	xRet |= FTM_JSON_getCount(pReqRoot, FTM_TRUE, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get id list with invalid argument!");
		xRet = FTM_RET_INVALID_ARGUMENTS;
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_getIDList(pClient, pSSID, ulCount, pIDList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pIDArray;

	pIDArray = cJSON_CreateArray();

	for(i = 0 ; i < ulCount ; i++)
	{
		cJSON_AddItemToArray(pIDArray, cJSON_CreateString(pIDList[i]));
	}

	cJSON_AddItemToObject(pRespRoot, "id_list", pIDArray);

finished:

	if (pIDList != NULL)
	{
		FTM_MEM_free(pIDList);
	}

	return	xRet;
}

FTM_RET	FTM_CGI_GET_CCTV_setPolicy
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_SWITCH_AC_POLICY	xPolicy;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pID, 0, sizeof(pID));

	xRet = FTM_CGI_getSSID(pReq, pID, FTM_FALSE);
	xRet |= FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CGI_getPolicy(pReq, &xPolicy, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_CCTV_setPolicy(pClient, pSSID, pID, xPolicy);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", pID);

	cJSON_AddItemToObject(pRoot, "ccrv", pCCTV);

finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_CCTV_setPolicy
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
	FTM_SWITCH_AC_POLICY	xPolicy;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	xRet |= FTM_JSON_getID(pReqRoot, FTM_FALSE, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_JSON_getPolicy(pReqRoot, FTM_FALSE,  &xPolicy);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_setPolicy(pClient, pSSID, pID, xPolicy);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", pID);

	cJSON_AddItemToObject(pRespRoot, "ccrv", pCCTV);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_CCTV_reset
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pHash[FTM_HASH_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reset cctv because ssid invalid.");
		goto finish;
	}

	memset(pID, 0, sizeof(pID));
	xRet = FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reset cctv because invalid arguments");
		goto finish;
	}

	xRet = FTM_CGI_getHash(pReq, pHash, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reset cctv because invalid arguments");
		goto finish;
	}

	xRet = FTM_CLIENT_CCTV_reset(pClient, pSSID, pID, pHash);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reset cctv[%s]", pID);
		goto finish;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", pID);

	cJSON_AddItemToObject(pRoot, "ccrv", pCCTV);
finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}


FTM_RET	FTM_CGI_POST_CCTV_reset
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
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pHash[FTM_HASH_LEN+1];

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reset cctv because ssid invalid.");
		goto finished;
	}

	xRet = FTM_JSON_getID(pReqRoot, FTM_FALSE, pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reset cctv because invalid arguments");
		goto finished;
	}

	xRet = FTM_JSON_getHash(pReqRoot, FTM_FALSE, pHash);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reset cctv because invalid arguments");
		goto finished;
	}

	xRet = FTM_CLIENT_CCTV_reset(pClient, pSSID, pID, pHash);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to reset cctv[%s]", pID);
		goto finished;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", pID);

	cJSON_AddItemToObject(pRespRoot, "ccrv", pCCTV);
finished:

	return	xRet;
}

