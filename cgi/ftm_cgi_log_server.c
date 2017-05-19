#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

typedef	struct
{
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_CHAR	pComment[FTM_COMMENT_LEN+1];
}	FTM_LOG_SERVER_CONFIG, _PTR_ FTM_LOG_SERVER_CONFIG_PTR;

extern	FTM_CHAR_PTR	FTM_trim(FTM_CHAR_PTR	pString);

FTM_RET	FTM_CGI_addLogServer
(
	FTM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_LOG_SERVER_CONFIG	xConfig;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_CGI_getID(pReq, xConfig.pID, FTM_FALSE);
	xRet |= FTM_CGI_getIPString(pReq, xConfig.pIP, FTM_IP_LEN, FTM_TRUE);
	xRet |= FTM_CGI_getComment(pReq, xConfig.pComment, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	FTM_CHAR	pCommand[256];
	FTM_CHAR	pResult[256];

	memset(pResult, 0, sizeof(pResult));

	sprintf(pCommand, "/usr/bin/server.sh add %s %s \"%s\"", xConfig.pID, xConfig.pIP, xConfig.pComment);

	FILE*	pFP;

	pFP = popen(pCommand, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;	
	}

	fscanf(pFP, "%s", pResult);
	pclose(pFP);

	if (strcasecmp(pResult, "ok") != 0)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;
	}

	cJSON _PTR_ pLogServer = cJSON_CreateObject();

	cJSON_AddStringToObject(pLogServer, "id", xConfig.pID);
	cJSON_AddStringToObject(pLogServer, "ip", 		xConfig.pIP);
	cJSON_AddStringToObject(pLogServer, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "log server", pLogServer);
finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_delLogServer
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

	FTM_CHAR	pCommand[256];
	FTM_CHAR	pResult[256];

	memset(pResult, 0, sizeof(pResult));

	sprintf(pCommand, "/usr/bin/server.sh del %s", pID);

	FILE*	pFP;

	pFP = popen(pCommand, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;	
	}

	fscanf(pFP, "%s", pResult);
	pclose(pFP);


	if (strcasecmp(pResult, "ok") != 0)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;
	}

	cJSON _PTR_ pLogServer = cJSON_CreateObject();

	cJSON_AddStringToObject(pLogServer, "id", pID);

	cJSON_AddItemToObject(pRoot, "log server", pLogServer);
finish:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_getLogServer
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_LOG_SERVER_CONFIG	xConfig;

	cJSON _PTR_	pRoot;
	FILE*	pFP = NULL;

	pRoot = cJSON_CreateObject();

	memset(pID, 0, sizeof(pID));
	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_CGI_getID(pReq, pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	FTM_CHAR	pCommand[256];
	FTM_CHAR	pBuffer[256];

	sprintf(pCommand, "/usr/bin/server.sh get");


	pFP = popen(pCommand, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;	
	}

	while(fgets(pBuffer, sizeof(pBuffer), pFP) != 0)
	{
		FTM_CHAR	pName[128];

		INFO("LINE : %s", pBuffer);
		if (sscanf(pBuffer, "%s", pName) != 0)
		{
			FTM_CHAR_PTR pValue = strstr(pBuffer, pName);
			if (pValue != NULL)
			{
				pValue += strlen(pName) + 1;
				INFO("pValue : %s", pValue);
				pValue  = FTM_trim(pValue);	

				if (strcasecmp(pName, "id") == 0)
				{
					strncpy(xConfig.pID, pValue, sizeof(xConfig.pID) - 1);	
				}
				if (strcasecmp(pName, "ip") == 0)
				{
					strncpy(xConfig.pIP, pValue, sizeof(xConfig.pIP) - 1);	
				}
				if (strcasecmp(pName, "comment") == 0)
				{
					strncpy(xConfig.pComment, pValue, sizeof(xConfig.pComment) - 1);	
				}
			}
		}
	}


	pclose(pFP);
	pFP = NULL;

	cJSON _PTR_ pLogServer = cJSON_CreateObject();

	cJSON_AddStringToObject(pLogServer, "id", xConfig.pID);
	cJSON_AddStringToObject(pLogServer, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pLogServer, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "log server", pLogServer);

finish:

	if (pFP != NULL)
	{
		pclose(pFP);
	}

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_setLogServer
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_UINT32	ulFieldFlags = 0;
	FTM_LOG_SERVER_CONFIG	xConfig;
	cJSON _PTR_	pRoot;
	FILE*	pFP = NULL;

	pRoot = cJSON_CreateObject();

	memset(&xConfig, 0, sizeof(xConfig));

	xRet = FTM_CGI_getID(pReq, xConfig.pID, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	FTM_CHAR	pCommand[256];
	FTM_CHAR	pBuffer[256];

	sprintf(pCommand, "/usr/bin/server.sh get");


	pFP = popen(pCommand, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;	
	}

	while(fgets(pBuffer, sizeof(pBuffer), pFP) != 0)
	{
		FTM_CHAR	pName[128];

		if (sscanf(pBuffer, "%s", pName) != 0)
		{
			FTM_CHAR_PTR pValue = strstr(pBuffer, pName);
			if (pValue != NULL)
			{
				pValue += strlen(pName) + 1;
				pValue  = FTM_trim(pValue);	

				if (strcasecmp(pName, "id") == 0)
				{
					strncpy(xConfig.pID, pValue, sizeof(xConfig.pID) - 1);	
				}
				if (strcasecmp(pName, "ip") == 0)
				{
					strncpy(xConfig.pIP, pValue, sizeof(xConfig.pIP) - 1);	
				}
				if (strcasecmp(pName, "comment") == 0)
				{
					strncpy(xConfig.pComment, pValue, sizeof(xConfig.pComment) - 1);	
				}
			}
		}
	}


	pclose(pFP);
	pFP = NULL;

	xRet = FTM_CGI_getIPString(pReq, xConfig.pIP, FTM_IP_LEN, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_IP;
	}

	xRet = FTM_CGI_getComment(pReq, xConfig.pComment, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		ulFieldFlags |= FTM_SWITCH_FIELD_COMMENT;
	}

	xRet = FTM_RET_OK;
	sprintf(pCommand, "/usr/bin/server.sh add %s %s %s", xConfig.pID, xConfig.pIP, xConfig.pComment);


	pFP = popen(pCommand, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;	
	}

	FTM_CHAR	pResult[64];

	memset(pResult, 0, sizeof(pResult));
	fscanf(pFP, "%s", pResult);
	pclose(pFP);
	pFP = NULL;

	if (strcasecmp(pResult, "ok") != 0)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;
	}

	cJSON _PTR_ pLogServer = cJSON_CreateObject();

	cJSON_AddStringToObject(pLogServer, "id", xConfig.pID);
	cJSON_AddStringToObject(pLogServer, "ip", xConfig.pIP);
	cJSON_AddStringToObject(pLogServer, "comment", xConfig.pComment);

	cJSON_AddItemToObject(pRoot, "log server", pLogServer);
finish:

	if (pFP != NULL)
	{
		pclose(pFP);
	}

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_getLogServerIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_ID_PTR	pIDList = NULL;
	cJSON _PTR_	pRoot;
	FTM_LOG_SERVER_CONFIG	xConfig;
	FILE *pFP = NULL;

	memset(&xConfig, 0, sizeof(xConfig));

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

	FTM_CHAR	pCommand[256];
	FTM_CHAR	pBuffer[256];

	sprintf(pCommand, "/usr/bin/server.sh get");


	pFP = popen(pCommand, "r");
	if (pFP == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to exec server.sh");
		goto finish;	
	}

	while(fgets(pBuffer, sizeof(pBuffer), pFP) != 0)
	{
		FTM_CHAR	pName[128];

		if (sscanf(pBuffer, "%s", pName) != 0)
		{
			FTM_CHAR_PTR pValue = strstr(pBuffer, pName);
			if (pValue != NULL)
			{
				pValue += strlen(pName) + 1;
				pValue  = FTM_trim(pValue);	

				if (strcasecmp(pName, "id") == 0)
				{
					strncpy(xConfig.pID, pValue, sizeof(xConfig.pID) - 1);	
				}
			}
		}
	}


	pclose(pFP);
	pFP = NULL;


	cJSON _PTR_ pIDArray;

	pIDArray = cJSON_CreateArray();

	if (strlen(xConfig.pID) != 0)
	{
		cJSON_AddItemToArray(pIDArray, cJSON_CreateString(xConfig.pID));
	}

	cJSON_AddItemToObject(pRoot, "id list", pIDArray);

finish:

	if (pIDList != NULL)
	{
		FTM_MEM_free(pIDList);
	}

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}
