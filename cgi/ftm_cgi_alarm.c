#include <string.h>
#include "cJSON/cJSON.h"
#include "ftm_mem.h"
#include "ftm_cgi.h"
#include "ftm_json_utils.h"
#include "ftm_cgi_command.h"

#undef	__MODULE__
#define	__MODULE__	"cgi"

FTM_RET	FTM_CGI_GET_ALARM_add
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	cJSON _PTR_	pRoot;
	FTM_ALARM	xAlarm;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_CHAR	pEmail[FTM_EMAIL_LEN+1];
	FTM_CHAR	pMessage[FTM_ALARM_MESSAGE_LEN+1];

	cJSON _PTR_ pReqRoot = NULL;

	memset(pSSID, 0, sizeof(pSSID));
	memset(pName, 0, sizeof(pName));
	memset(pEmail, 0, sizeof(pEmail));
	memset(pMessage, 0, sizeof(pMessage));

	cJSON _PTR_ pItem = cJSON_GetObjectItem(pReqRoot, "ssid");
	if ((pItem == NULL) || (pItem->type != cJSON_String) || (strlen(pItem->valuestring) > FTM_SSID_LEN))
	{
		goto finished;	
	}
	strcpy(pSSID, pItem->valuestring);

	pItem = cJSON_GetObjectItem(pReqRoot, "name");
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

	xRet = FTM_CLIENT_ALARM_add(pClient, pSSID, pName, pEmail, pMessage);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pName, &xAlarm);
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

FTM_RET	FTM_CGI_POST_ALARM_add
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
	FTM_ALARM	xAlarm;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_CHAR	pEmail[FTM_EMAIL_LEN+1];
	FTM_CHAR	pMessage[FTM_ALARM_MESSAGE_LEN+1];

	memset(pSSID, 0, sizeof(pSSID));
	memset(pName, 0, sizeof(pName));
	memset(pEmail, 0, sizeof(pEmail));
	memset(pMessage, 0, sizeof(pMessage));

	xRet = FTM_JSON_getSSID(pReqRoot, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;	
	}

	xRet = FTM_JSON_getName(pReqRoot, FTM_FALSE, pName);
	if (xRet != FTM_RET_OK)
	{
		goto finished;	
	}

	xRet = FTM_JSON_getString(pReqRoot, "email", FTM_EMAIL_LEN, FTM_TRUE, pEmail);
	if (xRet != FTM_RET_OK)
	{
		goto finished;	
	}

	xRet = FTM_JSON_getString(pReqRoot, "message", FTM_ALARM_MESSAGE_LEN, FTM_TRUE, pMessage);
	if (xRet != FTM_RET_OK)
	{
		goto finished;	
	}

	xRet = FTM_CLIENT_ALARM_add(pClient, pSSID, pName, pEmail, pMessage);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pName, &xAlarm);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pAlarm = cJSON_CreateObject();
	cJSON_AddStringToObject(pAlarm, "name", xAlarm.pName);
	cJSON_AddStringToObject(pAlarm, "email", xAlarm.pEmail);
	cJSON_AddStringToObject(pAlarm, "message", xAlarm.pMessage);

	cJSON_AddItemToObject(pRespRoot, "alarm", pAlarm);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_ALARM_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSTRING(pReq, "ssid", pSSID, FTM_SSID_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CGI_getSTRING(pReq, "name", pName, FTM_NAME_LEN, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_del(pClient, pSSID, pName);
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

FTM_RET	FTM_CGI_POST_ALARM_del
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
	FTM_CHAR	pName[FTM_NAME_LEN+1];

	memset(pSSID, 0, sizeof(pSSID));
	memset(pName, 0, sizeof(pName));

	xRet = FTM_JSON_getString(pReqRoot, "ssid", FTM_SSID_LEN, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_JSON_getString(pReqRoot, "name", FTM_NAME_LEN, FTM_TRUE, pName);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_del(pClient, pSSID, pName);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pAlarm = cJSON_CreateObject();
	cJSON_AddStringToObject(pAlarm, "name", pName);

	cJSON_AddItemToObject(pRespRoot, "alarm", pAlarm);
finished:
	
	return	xRet;
}

FTM_RET	FTM_CGI_GET_ALARM_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_ALARM	xAlarm;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSTRING(pReq, "ssid", pSSID, FTM_SSID_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CGI_getSTRING(pReq, "name", pName, FTM_NAME_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pName, &xAlarm);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm");
		goto finished;	
	}

	cJSON _PTR_ pAlarm = cJSON_CreateObject();

	cJSON_AddStringToObject(pAlarm, "name", xAlarm.pName);
	cJSON_AddStringToObject(pAlarm, "email", xAlarm.pEmail);
	cJSON_AddStringToObject(pAlarm, "message", xAlarm.pMessage);


	cJSON_AddItemToObject(pRoot, "alarm", pAlarm);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}


FTM_RET	FTM_CGI_POST_ALARM_get
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
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_ALARM	xAlarm;

	memset(pSSID, 0, sizeof(pSSID));
	memset(pName, 0, sizeof(pName));

	xRet = FTM_JSON_getString(pReqRoot, "ssid", FTM_SSID_LEN, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_JSON_getString(pReqRoot, "name", FTM_NAME_LEN, FTM_FALSE, pName);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pName, &xAlarm);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm");
		goto finished;	
	}

	cJSON _PTR_ pAlarm = cJSON_CreateObject();

	cJSON_AddStringToObject(pAlarm, "name", xAlarm.pName);
	cJSON_AddStringToObject(pAlarm, "email", xAlarm.pEmail);
	cJSON_AddStringToObject(pAlarm, "message", xAlarm.pMessage);


	cJSON_AddItemToObject(pRespRoot, "alarm", pAlarm);

finished:

	return	xRet;
}


FTM_RET	FTM_CGI_GET_ALARM_getList
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
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_NAME_PTR	pAlarmNameList = NULL;
	FTM_ALARM_PTR	pAlarmList = NULL;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSTRING(pReq, "ssid", pSSID, FTM_SSID_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CGI_getCount(pReq, &ulIndex, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "The index parameter is messing!");
		goto finished;
	}

	xRet = FTM_CGI_getCount(pReq, &ulCount, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "The count parameter is messing!");
		goto finished;
	}

	pAlarmNameList = (FTM_NAME_PTR)FTM_MEM_malloc(sizeof(FTM_NAME) * ulCount);
	if (pAlarmNameList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory[size = %d]!", sizeof(FTM_NAME) * ulCount);
		goto finished;	
	}
	
	xRet = FTM_CLIENT_ALARM_getNameList(pClient, pSSID, ulIndex, ulCount, pAlarmNameList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm name list!");
		goto finished;	
	}

	INFO("Name list count : %d", ulCount);
	pAlarmList = (FTM_ALARM_PTR)FTM_MEM_malloc(sizeof(FTM_ALARM) * ulCount);
	if (pAlarmList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory[size = %d]!", sizeof(FTM_ALARM) * ulCount);
		goto finished;	
	}
	
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pAlarmNameList[i], &pAlarmList[i]);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get alarm[%s]!", pAlarmNameList[i]);
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


FTM_RET	FTM_CGI_POST_ALARM_getList
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
	FTM_UINT32	ulIndex = 0;
	FTM_UINT32	ulCount = 20;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_NAME_PTR	pAlarmNameList = NULL;
	FTM_ALARM_PTR	pAlarmList = NULL;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getString(pReqRoot, "ssid", FTM_SSID_LEN, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_JSON_getUINT32(pReqRoot, "index", FTM_TRUE, &ulIndex);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "The index parameter is messing!");
		goto finished;
	}

	xRet = FTM_JSON_getUINT32(pReqRoot, "count", FTM_TRUE, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "The count parameter is messing!");
		goto finished;
	}

	pAlarmNameList = (FTM_NAME_PTR)FTM_MEM_malloc(sizeof(FTM_NAME) * ulCount);
	if (pAlarmNameList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory[size = %d]!", sizeof(FTM_NAME) * ulCount);
		goto finished;	
	}
	
	xRet = FTM_CLIENT_ALARM_getNameList(pClient, pSSID, ulIndex, ulCount, pAlarmNameList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm name list!");
		goto finished;	
	}

	INFO("Name list count : %d", ulCount);
	pAlarmList = (FTM_ALARM_PTR)FTM_MEM_malloc(sizeof(FTM_ALARM) * ulCount);
	if (pAlarmList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory[size = %d]!", sizeof(FTM_ALARM) * ulCount);
		goto finished;	
	}
	
	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pAlarmNameList[i], &pAlarmList[i]);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get alarm[%s]!", pAlarmNameList[i]);
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

	cJSON_AddNumberToObject(pRespRoot, "count", ulCount);
	cJSON_AddItemToObject(pRespRoot, "alarm", pAlarmArray);

finished:

	if (pAlarmList != NULL)
	{
		FTM_MEM_free(pAlarmList);	
	}

	if (pAlarmNameList != NULL)
	{
		FTM_MEM_free(pAlarmNameList);	
	}

	return	xRet;
}


FTM_RET	FTM_CGI_GET_ALARM_set
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet;
	cJSON _PTR_	pRoot;
	cJSON _PTR_	pItem;
	cJSON _PTR_ pReqRoot = NULL;
	FTM_UINT32	ulFieldFlags = 0;
	FTM_ALARM	xAlarm;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	
	pRoot = cJSON_CreateObject();

	FTM_CHAR_PTR	pBody = qcgireq_getquery(Q_CGI_POST);

	if (pBody == NULL)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		ERROR(xRet, "Post body is empry!");
		goto finished;
	}

	pReqRoot = cJSON_Parse(pBody);
	if (pReqRoot == NULL)
	{
		xRet = FTM_RET_INVALID_JSON_FORMAT;
		ERROR(xRet, "Invalid JSON format!");
		goto finished;	
	}

	memset(&xAlarm, 0, sizeof(xAlarm));

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSTRING(pReq, "ssid", pSSID, FTM_SSID_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pAlarmItem = cJSON_GetObjectItem(pReqRoot, "alarm");
	if (pAlarmItem == NULL)
	{
		xRet = FTM_RET_OBJECT_NOT_FOUND;
		ERROR(xRet, "Failed to get alarm element!");
		goto finished;	
	}
	
	memset(pName, 0, sizeof(pName));
	xRet = FTM_CGI_getSTRING(pReq, "name", pName, FTM_NAME_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm name!");
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pName, &xAlarm);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm");
		goto finished;	
	}

	pItem = cJSON_GetObjectItem(pAlarmItem, "email");
	if (pItem != NULL)
	{
		if ((pItem->type != cJSON_String) || (strlen(pItem->valuestring) > FTM_EMAIL_LEN))
		{
			ERROR(xRet, "Invalid email!");
			goto finished;	
		}
		strcpy(xAlarm.pEmail, pItem->valuestring);
		ulFieldFlags |= FTM_ALARM_FIELD_EMAIL;
	}

	pItem = cJSON_GetObjectItem(pAlarmItem, "message");
	if (pItem != NULL)
	{
		if ((pItem->type != cJSON_String) || (strlen(pItem->valuestring) > FTM_ALARM_MESSAGE_LEN))
		{
			ERROR(xRet, "Invalid message!");
			goto finished;	
		}
		strcpy(xAlarm.pMessage, pItem->valuestring);
		ulFieldFlags |= FTM_ALARM_FIELD_MESSAGE;
	}

	xRet = FTM_CLIENT_ALARM_set(pClient, pSSID, pName, &xAlarm, ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pName, &xAlarm);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

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

FTM_RET	FTM_CGI_POST_ALARM_set
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
	cJSON _PTR_	pItem;
	FTM_UINT32	ulFieldFlags = 0;
	FTM_ALARM	xAlarm;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	
	memset(&xAlarm, 0, sizeof(xAlarm));
	memset(pSSID, 0, sizeof(pSSID));
	memset(pName, 0, sizeof(pName));

	xRet = FTM_JSON_getString(pReqRoot, "ssid", FTM_SSID_LEN, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_JSON_getString(pReqRoot, "name", FTM_NAME_LEN, FTM_FALSE, pName);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm name!");
		goto finished;
	}

	cJSON _PTR_ pAlarmItem = cJSON_GetObjectItem(pReqRoot, "alarm");
	if (pAlarmItem == NULL)
	{
		xRet = FTM_RET_OBJECT_NOT_FOUND;
		ERROR(xRet, "Failed to get alarm element!");
		goto finished;	
	}
	
	xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pName, &xAlarm);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm");
		goto finished;	
	}

	pItem = cJSON_GetObjectItem(pAlarmItem, "email");
	if (pItem != NULL)
	{
		if ((pItem->type != cJSON_String) || (strlen(pItem->valuestring) > FTM_EMAIL_LEN))
		{
			ERROR(xRet, "Invalid email!");
			goto finished;	
		}
		strcpy(xAlarm.pEmail, pItem->valuestring);
		ulFieldFlags |= FTM_ALARM_FIELD_EMAIL;
	}

	pItem = cJSON_GetObjectItem(pAlarmItem, "message");
	if (pItem != NULL)
	{
		if ((pItem->type != cJSON_String) || (strlen(pItem->valuestring) > FTM_ALARM_MESSAGE_LEN))
		{
			ERROR(xRet, "Invalid message!");
			goto finished;	
		}
		strcpy(xAlarm.pMessage, pItem->valuestring);
		ulFieldFlags |= FTM_ALARM_FIELD_MESSAGE;
	}

	xRet = FTM_CLIENT_ALARM_set(pClient, pSSID, pName, &xAlarm, ulFieldFlags);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_get(pClient, pSSID, pName, &xAlarm);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	cJSON _PTR_ pAlarm = cJSON_CreateObject();
	cJSON_AddStringToObject(pAlarm, "name", xAlarm.pName);
	cJSON_AddStringToObject(pAlarm, "email", xAlarm.pEmail);
	cJSON_AddStringToObject(pAlarm, "message", xAlarm.pMessage);

	cJSON_AddItemToObject(pRespRoot, "alarm", pAlarm);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_ALARM_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_UINT32	ulCount = 0;
	cJSON _PTR_	pRoot;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSTRING(pReq, "ssid", pSSID, FTM_SSID_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_getCount(pClient, pSSID, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm");
		goto finished;	
	}

	cJSON _PTR_ pAlarm = cJSON_CreateObject();

	cJSON_AddNumberToObject(pAlarm, "count", ulCount);

	cJSON_AddItemToObject(pRoot, "alarm_info", pAlarm);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_ALARM_getInfo
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
	FTM_UINT32	ulCount = 0;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getString(pReqRoot, "ssid", FTM_SSID_LEN, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_ALARM_getCount(pClient, pSSID, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm");
		goto finished;	
	}

	cJSON _PTR_ pAlarm = cJSON_CreateObject();

	cJSON_AddNumberToObject(pAlarm, "count", ulCount);

	cJSON_AddItemToObject(pRespRoot, "alarm_info", pAlarm);

finished:

	return	xRet;
}


FTM_RET	FTM_CGI_GET_ALARM_getConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	cJSON _PTR_	pRoot;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_NOTIFIER_SMTP_CONFIG	xConfig;

	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSTRING(pReq, "ssid", pSSID, FTM_SSID_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SMTP_get(pClient, pSSID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm config");
		goto finished;	
	}

	cJSON _PTR_ pConfig = cJSON_CreateObject();

	cJSON_AddStringToObject(pConfig, "enable", 	((xConfig.bEnable)?"yes":"no"));
	cJSON_AddStringToObject(pConfig, "server", 	xConfig.pServer);
	cJSON_AddNumberToObject(pConfig, "port", 	xConfig.usPort);
	cJSON_AddStringToObject(pConfig, "userid", 	xConfig.pUserID);
	cJSON_AddStringToObject(pConfig, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pConfig, "sender", 	xConfig.pSender);

	cJSON_AddItemToObject(pRoot, "alarm_config", pConfig);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_ALARM_getConfig
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
	FTM_NOTIFIER_SMTP_CONFIG	xConfig;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getString(pReqRoot, "ssid", FTM_SSID_LEN, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SMTP_get(pClient, pSSID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm config");
		goto finished;	
	}

	cJSON _PTR_ pConfig = cJSON_CreateObject();

	cJSON_AddStringToObject(pConfig, "enable", 	((xConfig.bEnable)?"yes":"no"));
	cJSON_AddStringToObject(pConfig, "server", 	xConfig.pServer);
	cJSON_AddNumberToObject(pConfig, "port", 	xConfig.usPort);
	cJSON_AddStringToObject(pConfig, "userid", 	xConfig.pUserID);
	cJSON_AddStringToObject(pConfig, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pConfig, "sender", 	xConfig.pSender);

	cJSON_AddItemToObject(pRespRoot, "alarm_config", pConfig);

finished:

	return	xRet;
}

FTM_RET	FTM_CGI_GET_ALARM_setConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	cJSON _PTR_	pRoot;
	FTM_CHAR	pSSID[FTM_SSID_LEN+1];
	FTM_NOTIFIER_SMTP_CONFIG	xConfig;
	FTM_NOTIFIER_SMTP_CONFIG	xNewConfig;


	pRoot = cJSON_CreateObject();

	memset(pSSID, 0, sizeof(pSSID));
	xRet = FTM_CGI_getSTRING(pReq, "ssid", pSSID, FTM_SSID_LEN, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SMTP_get(pClient, pSSID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm config");
		goto finished;	
	}

	memcpy(&xNewConfig, &xConfig, sizeof(xConfig));

	xRet = FTM_CGI_getEnable(pReq, &xNewConfig.bEnable, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get enable!");
		goto finished;
	}

	xRet = FTM_CGI_getServer(pReq, xNewConfig.pServer, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get server!");
		goto finished;
	}

	xRet = FTM_CGI_getPort(pReq, &xNewConfig.usPort, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get port!");
		goto finished;
	}

	xRet = FTM_CGI_getUserID(pReq, xNewConfig.pUserID, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get user id!");
		goto finished;
	}

	xRet = FTM_CGI_getPasswd(pReq, xNewConfig.pPasswd, sizeof(xNewConfig.pPasswd) - 1, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get passwd!");
		goto finished;
	}

	xRet = FTM_CGI_getSender(pReq, xNewConfig.pSender, sizeof(xNewConfig.pSender) - 1, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get sender!");
		goto finished;
	}

	xRet = FTM_CLIENT_SMTP_set(pClient, pSSID, &xNewConfig, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm config");
		goto finished;	
	}

	cJSON _PTR_ pConfig = cJSON_CreateObject();

	cJSON_AddStringToObject(pConfig, "enable", 	((xConfig.bEnable)?"yes":"no"));
	cJSON_AddStringToObject(pConfig, "server", 	xConfig.pServer);
	cJSON_AddNumberToObject(pConfig, "port", 	xConfig.usPort);
	cJSON_AddStringToObject(pConfig, "userid", xConfig.pUserID);
	cJSON_AddStringToObject(pConfig, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pConfig, "sender", 	xConfig.pSender);

	cJSON_AddItemToObject(pRoot, "alarm_config", pConfig);

finished:

	return	FTM_CGI_finish(pReq, pRoot, xRet);
}

FTM_RET	FTM_CGI_POST_ALARM_setConfig
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
	FTM_NOTIFIER_SMTP_CONFIG	xConfig;
	FTM_NOTIFIER_SMTP_CONFIG	xNewConfig;

	memset(pSSID, 0, sizeof(pSSID));

	xRet = FTM_JSON_getString(pReqRoot, "ssid", FTM_SSID_LEN, FTM_FALSE, pSSID);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_CLIENT_SMTP_get(pClient, pSSID, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm config");
		goto finished;	
	}

	memcpy(&xNewConfig, &xConfig, sizeof(xConfig));

	xRet = FTM_JSON_getBool(pReqRoot, "enable", FTM_TRUE, &xNewConfig.bEnable);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get enable!");
		goto finished;
	}

	xRet = FTM_JSON_getString(pReqRoot, "server", sizeof(xNewConfig.pServer), FTM_TRUE, xNewConfig.pServer);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get server!");
		goto finished;
	}

	xRet = FTM_JSON_getUINT16(pReqRoot, "port", FTM_TRUE, &xNewConfig.usPort);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get port!");
		goto finished;
	}

	xRet = FTM_JSON_getString(pReqRoot, "userid", sizeof(xNewConfig.pUserID), FTM_TRUE, xNewConfig.pUserID);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get user id!");
		goto finished;
	}

	xRet = FTM_JSON_getString(pReqRoot, "passwd", sizeof(xNewConfig.pPasswd), FTM_TRUE, xNewConfig.pPasswd);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get passwd!");
		goto finished;
	}

	xRet = FTM_JSON_getString(pReqRoot, "sender", sizeof(xNewConfig.pSender), FTM_TRUE, xNewConfig.pSender);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get sender!");
		goto finished;
	}

	xRet = FTM_CLIENT_SMTP_set(pClient, pSSID, &xNewConfig, &xConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get alarm config");
		goto finished;	
	}

	cJSON _PTR_ pConfig = cJSON_CreateObject();

	cJSON_AddStringToObject(pConfig, "enable", 	((xConfig.bEnable)?"yes":"no"));
	cJSON_AddStringToObject(pConfig, "server", 	xConfig.pServer);
	cJSON_AddNumberToObject(pConfig, "port", 	xConfig.usPort);
	cJSON_AddStringToObject(pConfig, "userid", xConfig.pUserID);
	cJSON_AddStringToObject(pConfig, "passwd", 	xConfig.pPasswd);
	cJSON_AddStringToObject(pConfig, "sender", 	xConfig.pSender);

	cJSON_AddItemToObject(pRespRoot, "alarm_config", pConfig);

finished:

	return	xRet;
}
