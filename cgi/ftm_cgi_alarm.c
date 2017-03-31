#include <string.h>
#include "cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"

FTM_RET	FTM_CGI_addAlarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	cJSON _PTR_	pRoot;
	cJSON _PTR_ pReqRoot = NULL;
	FTM_ALARM	xAlarm;
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_CHAR	pEmail[FTM_EMAIL_LEN+1];
	FTM_CHAR	pMessage[FTM_ALARM_MESSAGE_LEN+1];

	FTM_CHAR_PTR	pBody = qcgireq_getquery(Q_CGI_POST);

	if (pBody == NULL)
	{
		goto finished;
	}

	pReqRoot = cJSON_Parse(pBody);
	if (pReqRoot == NULL)
	{
		goto finished;	
	}

	memset(pName, 0, sizeof(pName));
	memset(pEmail, 0, sizeof(pEmail));
	memset(pMessage, 0, sizeof(pMessage));

	cJSON _PTR_ pItem = cJSON_GetObjectItem(pReqRoot, "name");
	if ((pItem == NULL) || (pItem->type != cJSON_String) || (strlen(pItem->valuestring) > FTM_NAME_LEN))
	{
		goto finished;	
	}
	strcpy(pName, pItem->valuestring);

	pItem = cJSON_GetObjectItem(pReqRoot, "email");
	if (pItem != NULL)
	{
		if ((pItem->type != cJSON_String) || (strlen(pItem->valuestring) > FTM_EMAIL_LEN))
		{
			goto finished;	
		}
		strcpy(pEmail, pItem->valuestring);
	}

	pItem = cJSON_GetObjectItem(pReqRoot, "message");
	if (pItem != NULL)
	{
		if ((pItem->type != cJSON_String) || (strlen(pItem->valuestring) > FTM_ALARM_MESSAGE_LEN))
		{
			goto finished;	
		}
		strcpy(pMessage, pItem->valuestring);
	}

	xRet = FTM_CLIENT_addAlarm(pClient, pName, pEmail, pMessage);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_getAlarm(pClient, pName, &xAlarm);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	pRoot = cJSON_CreateObject();
	cJSON _PTR_ pAlarm = cJSON_CreateObject();
	cJSON_AddStringToObject(pAlarm, "name", xAlarm.pName);
	cJSON_AddStringToObject(pAlarm, "email", xAlarm.pEmail);
	cJSON_AddStringToObject(pAlarm, "message", xAlarm.pMessage);

	cJSON_AddItemToObject(pRoot, "alarm", pAlarm);

finished:

	if (pReqRoot != NULL)
	{
		cJSON_Delete(pReqRoot);	
	}

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_delAlarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	xRet = FTM_CGI_getSTRING(pReq, "name", pName, FTM_NAME_LEN, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_delAlarm(pClient, pName);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pAlarm = cJSON_CreateObject();
	cJSON_AddStringToObject(pAlarm, "name", pName);

	cJSON_AddItemToObject(pRoot, "alarm", pAlarm);
finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_getAlarmList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT		i;
	FTM_UINT32	ulCount = 20;
	FTM_NAME_PTR	pAlarmNameList = NULL;
	FTM_ALARM_PTR	pAlarmList = NULL;

	cJSON _PTR_	pRoot;

	pAlarmNameList = (FTM_NAME_PTR)FTM_MEM_malloc(sizeof(FTM_NAME) * ulCount);
	if (pAlarmNameList == NULL)
	{
		goto finished;	
	}
	
	xRet = FTM_CLIENT_getAlarmNameList(pClient, ulCount, pAlarmNameList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finished;	
	}

	pAlarmList = (FTM_ALARM_PTR)FTM_MEM_malloc(sizeof(FTM_ALARM)*ulCount);
	if (pAlarmList == NULL)
	{
		goto finished;	
	}
	
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_CLIENT_getAlarm(pClient, pAlarmNameList[i], &pAlarmList[i]);
		if (xRet != FTM_RET_OK)
		{
			goto finished;	
		}
	}

	cJSON _PTR_ pAlarmArray = cJSON_CreateArray();

	for(i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pAlarm = cJSON_CreateObject();

		cJSON_AddStringToObject(pAlarm, "name", pAlarmList[i].pName);
		cJSON_AddStringToObject(pAlarm, "email", pAlarmList[i].pEmail);
		cJSON_AddStringToObject(pAlarm, "message", pAlarmList[i].pMessage);

		cJSON_AddItemToArray(pAlarmArray, pAlarm);
	}

	pRoot = cJSON_CreateObject();

	cJSON_AddNumberToObject(pRoot, "count", ulCount);
	cJSON_AddItemToObject(pRoot, "alarm_list", pAlarmArray);

finished:

	if (pAlarmList != NULL)
	{
		FTM_MEM_free(pAlarmList);	
	}

	if (pAlarmNameList != NULL)
	{
		FTM_MEM_free(pAlarmNameList);	
	}

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}


