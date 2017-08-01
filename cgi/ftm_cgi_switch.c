#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_json_utils.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

FTM_RET	FTM_CGI_GET_SWITCH_add
(
	FTM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_SWITCH_CONFIG	xConfig;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_CGI_getID(pReq, xConfig.pID, FTM_FALSE);
	xRet |= FTM_CGI_getSwitchModel(pReq, xConfig.pModel, FTM_FALSE);
	xRet |= FTM_CGI_getIPString(pReq, xConfig.pIP, FTM_IP_LEN, FTM_TRUE);
	xRet |= FTM_CGI_getUserID(pReq, xConfig.pUserID, FTM_TRUE);
	xRet |= FTM_CGI_getPasswd(pReq, xConfig.pPasswd, sizeof(xConfig.pPasswd), FTM_TRUE);
	xRet |= FTM_CGI_getSecure(pReq, &xConfig.bSecure, FTM_TRUE);
	xRet |= FTM_CGI_getComment(pReq, xConfig.pComment, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_add(pClient, pSSID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_getProperties(pClient, pSSID, xConfig.pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", xConfig.pModel);
	cJSON_AddStringToObject(pSwitch, "ip", 		xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", 	xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "secure", 	(xConfig.bSecure?"on":"off"));
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "switch", pSwitch);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SWITCH_add
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
	FTM_SWITCH_CONFIG	xConfig;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE ,pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_JSON_getID(pReqRoot, FTM_FALSE, xConfig.pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add switch with invalid id!");
		goto finished;
	}

	xRet = FTM_JSON_getModel(pReqRoot, FTM_FALSE, xConfig.pModel);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add switch with invalid model!");
		goto finished;
	}

	xRet = FTM_JSON_getIP(pReqRoot, 	FTM_TRUE, xConfig.pIP);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add switch with invalid ip!");
		goto finished;
	}

	xRet = FTM_JSON_getUserID(pReqRoot, FTM_TRUE, xConfig.pUserID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add switch with invalid user id!");
		goto finished;
	}

	xRet = FTM_JSON_getPasswd(pReqRoot, FTM_TRUE, xConfig.pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add switch with invalid passwd!");
		goto finished;
	}

	xRet = FTM_JSON_getComment(pReqRoot, FTM_TRUE, xConfig.pComment);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add switch with invalid comment!");
		goto finished;
	}

	xRet = FTM_JSON_getSecure(pReqRoot, FTM_TRUE, &xConfig.bSecure);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add switch with invalid secure!");
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_add(pClient, pSSID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add switch!");
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_getProperties(pClient, pSSID, xConfig.pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get properties from server!");
		goto finished;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", 		xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", 	xConfig.pModel);
	cJSON_AddStringToObject(pSwitch, "ip", 		xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", 	xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "secure", 	(xConfig.bSecure?"on":"off"));
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRespRoot, "switch", pSwitch);
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_SWITCH_del
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

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	memset(pID, 0, sizeof(pID));
	xRet = FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_del(pClient, pSSID, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", pID);

	cJSON_AddItemToObject(pRoot, "switch", pSwitch);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SWITCH_del
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

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log with invalid ssid.");
		goto finished;
	}

	xRet = FTM_JSON_getID(pReqRoot, FTM_FALSE, pID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log with invalid id.");
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_del(pClient, pSSID, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", pID);

	cJSON_AddItemToObject(pRespRoot, "switch", pSwitch);
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_SWITCH_get
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
	FTM_SWITCH_CONFIG	xConfig;

	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	memset(pID, 0, sizeof(pID));
	xRet = FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_getProperties(pClient, pSSID, pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", 		xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", 	xConfig.pModel);
	cJSON_AddStringToObject(pSwitch, "ip", 		xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "secure", (xConfig.bSecure?"on":"off"));
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "switch", pSwitch);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SWITCH_get
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
	FTM_SWITCH_CONFIG	xConfig;

	memset(pSSID, 0, sizeof(pSSID));
	memset(pID, 0, sizeof(pID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	xRet = FTM_JSON_getID(pReqRoot, FTM_FALSE, pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_getProperties(pClient, pSSID, pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", 		xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", 	xConfig.pModel);
	cJSON_AddStringToObject(pSwitch, "ip", 		xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", 	xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "secure", (xConfig.bSecure?"on":"off"));
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRespRoot, "switch", pSwitch);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_SWITCH_set
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
	FTM_SWITCH_CONFIG	xConfig;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	memset(&xConfig, 0, sizeof(xConfig));
	xRet = FTM_CGI_getID(pReq, xConfig.pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
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

	xRet = FTM_CGI_getPasswd(pReq, xConfig.pPasswd, sizeof(xConfig.pPasswd), FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_PASSWD;
	}

	xRet = FTM_CGI_getSecure(pReq, &xConfig.bSecure, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_SECURE;
	}

	xRet = FTM_CGI_getComment(pReq, xConfig.pComment, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_COMMENT;
	}


	xRet = FTM_CLIENT_SWITCH_setProperties(pClient, pSSID, xConfig.pID, &xConfig, ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_getProperties(pClient, pSSID, xConfig.pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", 		xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", 	xConfig.pModel);
	cJSON_AddStringToObject(pSwitch, "ip", 		xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", 	xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "secure", (xConfig.bSecure?"on":"off"));
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "switch", pSwitch);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SWITCH_set
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
	FTM_SWITCH_CONFIG	xConfig;

	memset(pSSID, 0, sizeof(pSSID));
	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log with invalid ssid.");
		goto finished;
	}

	xRet = FTM_JSON_getID(pReqRoot, FTM_FALSE, xConfig.pID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_JSON_getIP(pReqRoot, FTM_FALSE, xConfig.pIP);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_IP;
	}

	xRet = FTM_JSON_getUserID(pReqRoot, FTM_FALSE, xConfig.pUserID);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_USER_ID;
	}

	xRet = FTM_JSON_getPasswd(pReqRoot, FTM_FALSE, xConfig.pPasswd);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_PASSWD;
	}

	xRet = FTM_JSON_getSecure(pReqRoot, FTM_FALSE, &xConfig.bSecure);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_SECURE;
	}

	xRet = FTM_JSON_getComment(pReqRoot, FTM_FALSE, xConfig.pComment);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_COMMENT;
	}


	xRet = FTM_CLIENT_SWITCH_setProperties(pClient, pSSID, xConfig.pID, &xConfig, ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_getProperties(pClient, pSSID, xConfig.pID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pSwitch = cJSON_CreateObject();

	cJSON_AddStringToObject(pSwitch, "id", 		xConfig.pID);
	cJSON_AddStringToObject(pSwitch, "model", 	xConfig.pModel);
	cJSON_AddStringToObject(pSwitch, "ip", 		xConfig.pIP);
	cJSON_AddStringToObject(pSwitch, "userid", 	xConfig.pUserID);
	cJSON_AddStringToObject(pSwitch, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pSwitch, "secure", (xConfig.bSecure?"on":"off"));
	cJSON_AddStringToObject(pSwitch, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRespRoot, "switch", pSwitch);
finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_SWITCH_getIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_UINT32	i = 0;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_ID_PTR	pIDList = NULL;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	pIDList = (FTM_ID_PTR)FTM_MEM_malloc(sizeof(FTM_ID) * ulCount);
	if (pIDList == NULL)
	{
		goto finished;			
	}

	xRet = FTM_CGI_getIndex(pReq, &ulIndex, FTM_TRUE);
	xRet |= FTM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_getIDList(pClient, pSSID, ulCount, pIDList, &ulCount);
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

	cJSON_AddItemToObject(pRoot, "idlist", pIDArray);

finished:

	if (pIDList != NULL)
	{
		FTM_MEM_free(pIDList);
	}

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_SWITCH_getIDList
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
	FTM_UINT32	i = 0;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_ID_PTR	pIDList = NULL;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get log because invalid ssid.");
		goto finished;
	}

	pIDList = (FTM_ID_PTR)FTM_MEM_malloc(sizeof(FTM_ID) * ulCount);
	if (pIDList == NULL)
	{
		goto finished;			
	}

	xRet = FTM_JSON_getIndex(pReqRoot, FTM_TRUE, &ulIndex);
	xRet |= FTM_JSON_getCount(pReqRoot, FTM_TRUE, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SWITCH_getIDList(pClient, pSSID, ulCount, pIDList, &ulCount);
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

	cJSON_AddItemToObject(pRespRoot, "idlist", pIDArray);

finished:

	if (pIDList != NULL)
	{
		FTM_MEM_free(pIDList);
	}

	return	xRet;
}

