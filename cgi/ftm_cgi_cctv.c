#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"

FTM_RET	FTM_CGI_addCCTV
(
	FTM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
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

	xRet = FTM_CGI_getID(pReq, pID, FTM_FALSE);
	xRet |= FTM_CGI_getIPString(pReq, pIP, FTM_IP_LEN, FTM_TRUE);
	xRet |= FTM_CGI_getSwitchID(pReq, pSwitchID, FTM_TRUE);
	xRet |= FTM_CGI_getComment(pReq, pComment, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create CCTV.!");
		goto finish;
	}

	xRet = FTM_CLIENT_addCCTV(pClient, pID, pIP, pSwitchID, pComment);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add CCTV.!");
		cJSON_AddStringToObject(pRoot, "error", "Failed to add CCTV.!");
		goto finish;
	}

	xRet = FTM_CLIENT_getCCTVProperties(pClient, pID, &xConfig);
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

FTM_RET	FTM_CGI_delCCTV
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pID[FTM_ID_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pID, 0, sizeof(pID));

	xRet = FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_delCCTV(pClient, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pCCTV = cJSON_CreateObject();
	cJSON_AddStringToObject(pCCTV, "id", pID);

	cJSON_AddItemToObject(pRoot, "cctv", pCCTV);

finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_getCCTV
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CCTV_CONFIG	xConfig;

	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pID, 0, sizeof(pID));

	xRet = FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_getCCTVProperties(pClient, pID, &xConfig);
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

FTM_RET	FTM_CGI_setCCTV
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_UINT32	ulFieldFlags = 0;
	FTM_CCTV_CONFIG	xConfig;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_CGI_getID(pReq, xConfig.pID, FTM_FALSE);
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


	xRet = FTM_CLIENT_setCCTVProperties(pClient, xConfig.pID, &xConfig, ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_getCCTVProperties(pClient, xConfig.pID, &xConfig);
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

FTM_RET	FTM_CGI_getCCTVIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
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

	xRet = FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	xRet |= FTM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_getCCTVIDList(pClient, ulCount, pIDList, &ulCount);
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

