#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"

FTM_RET	FTM_CGI_addSwitch
(
	FTM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_SWITCH_CONFIG	xConfig;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_CGI_getID(pReq, xConfig.pID, FTM_FALSE);
	xRet |= FTM_CGI_getSwitchModel(pReq, &xConfig.xModel, FTM_FALSE);
	xRet |= FTM_CGI_getIPString(pReq, xConfig.pIP, FTM_IP_LEN, FTM_TRUE);
	xRet |= FTM_CGI_getUserID(pReq, xConfig.pUserID, FTM_TRUE);
	xRet |= FTM_CGI_getPasswd(pReq, xConfig.pPasswd, FTM_TRUE);
	xRet |= FTM_CGI_getComment(pReq, xConfig.pComment, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_addSwitch(pClient, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_getSwitchProperties(pClient, xConfig.pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", FTM_getSwitchModelName(xConfig.xModel));
	cJSON_AddStringToObject(pSwitch, "ip", 		xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", 	xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "switch", pSwitch);
finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_delSwitch
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

	xRet = FTM_CLIENT_delSwitch(pClient, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", pID);

	cJSON_AddItemToObject(pRoot, "switch", pSwitch);
finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_getSwitch
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_SWITCH_CONFIG	xConfig;

	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pID, 0, sizeof(pID));

	xRet = FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_getSwitchProperties(pClient, pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", FTM_getSwitchModelName(xConfig.xModel));
	cJSON_AddStringToObject(pSwitch, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "switch", pSwitch);

finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_setSwitch
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_UINT32	ulFieldFlags = 0;
	FTM_SWITCH_CONFIG	xConfig;
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
		ulFieldFlags |= FTM_SWITCH_FIELD_IP;
	}

	xRet = FTM_CGI_getUserID(pReq, xConfig.pUserID, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_USER_ID;
	}

	xRet = FTM_CGI_getPasswd(pReq, xConfig.pPasswd, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_PASSWD;
	}

	xRet = FTM_CGI_getComment(pReq, xConfig.pComment, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_COMMENT;
	}


	xRet = FTM_CLIENT_setSwitchProperties(pClient, xConfig.pID, &xConfig, ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	xRet = FTM_CLIENT_getSwitchProperties(pClient, xConfig.pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", FTM_getSwitchModelName(xConfig.xModel));
	cJSON_AddStringToObject(pSwitch, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "switch", pSwitch);
finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_getSwitchIDList
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

	xRet = FTM_CLIENT_getSwitchIDList(pClient, ulCount, pIDList, &ulCount);
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

	cJSON_AddItemToObject(pRoot, "idlist", pIDArray);

finish:

	if (pIDList != NULL)
	{
		FTM_MEM_free(pIDList);
	}

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

