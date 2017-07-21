#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_client.h"
#include "ftm_profile.h"

#undef	__MODULE__
#define	__MODULE__ "cgi"

FTM_RET	FTM_CGI_SSID_create
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
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	cJSON _PTR_	pRoot;
	FTM_PROFILE	xProfile;

	pRoot = cJSON_CreateObject();

	memset(pID, 0, sizeof(pID));
	memset(pPasswd, 0, sizeof(pPasswd));

	xRet = FTM_CGI_getUserID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create SSID.!");
		goto finished;
	}

	xRet = FTM_CGI_getPasswd(pReq, pPasswd, FTM_PASSWD_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create SSID.!");
		goto finished;
	}

	xRet = FTM_PROFILE_get(&xProfile);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		goto finished;
	}

	if ((strcmp(xProfile.pUserID, pID) != 0) || (strcmp(xProfile.pPasswd, pPasswd) != 0))
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get profile!");
		cJSON_AddStringToObject(pRoot, "message", "Passwd do not match!");
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_create(pClient, pID, strlen(pID), pPasswd, strlen(pPasswd), pSSID, FTM_SSID_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create SSID.!");
		cJSON_AddStringToObject(pRoot, "error", "Failed to create SSID.!");
		goto finished;
	}

	cJSON_AddStringToObject(pRoot, "ssid", pSSID);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_SSID_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_del(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_SSID_verify
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTM_CGI_getSSID(pReq, pSSID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SSID_verify(pClient, pSSID, strlen(pSSID));
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

