#include <string.h>
#include "ftm_json_utils.h"
#include "cjson/cJSON.h"

FTM_RET	FTM_JSON_getSSID
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pSSID
)
{
	return	FTM_JSON_getString(pRoot, "ssid", FTM_SSID_LEN, bEmptyAllow, pSSID);
}

FTM_RET	FTM_JSON_getName
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pName
)
{
	return	FTM_JSON_getString(pRoot, "name", FTM_NAME_LEN, bEmptyAllow, pName);
}

FTM_RET	FTM_JSON_getModel
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "model", FTM_MODEL_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getID
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "id", FTM_ID_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getUserID
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "userid", FTM_ID_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getPasswd
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "passwd", FTM_PASSWD_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getNewPasswd
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "new_passwd", FTM_PASSWD_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getIP
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "ip", FTM_IP_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getComment
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "comment", FTM_COMMENT_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getHash
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "hash", FTM_HASH_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getLogType
(
	cJSON _PTR_	pRoot,
	FTM_BOOL	bAllowEmpty,
	FTM_LOG_TYPE_PTR	pType
)
{
	ASSERT(pRoot != NULL);
	ASSERT(pType!= NULL);

	FTM_RET			xRet;
	FTM_CHAR		pBuffer[256];
	
	xRet = FTM_JSON_getString(pRoot, "type", sizeof(pBuffer), FTM_FALSE, pBuffer);
	if(xRet == FTM_RET_OBJECT_NOT_FOUND)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strcasecmp(pBuffer , "normal") == 0) || (strcasecmp(pBuffer, "nomal") == 0))
	{
		*pType = FTM_LOG_TYPE_NORMAL;	
	}
	else if(strcasecmp(pBuffer, "error") == 0)
	{
		*pType = FTM_LOG_TYPE_ERROR;	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_JSON_getPolicy
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_SWITCH_AC_POLICY_PTR	pPolicy
)
{
	FTM_RET		xRet;
	FTM_CHAR	pBuffer[256];

	xRet = FTM_JSON_getString(pRoot, "policy", sizeof(pBuffer), FTM_FALSE, pBuffer);
	if (xRet == FTM_RET_OK)
	{
		if (strcmp(pBuffer, "allow") == 0)
		{
			*pPolicy = FTM_SWITCH_AC_POLICY_ALLOW;
		}
		else if (strcmp(pBuffer, "deny") == 0)
		{
			*pPolicy = FTM_SWITCH_AC_POLICY_DENY;
		}

		return	xRet;
	}

	if (bEmptyAllow && (xRet == FTM_RET_OBJECT_NOT_FOUND))
	{
		return	FTM_RET_OK;	
	}

	return	xRet;
}

FTM_RET	FTM_JSON_getIndex
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_UINT32_PTR	pValue
)
{
	return	FTM_JSON_getUINT32(pRoot, "index", bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getCount
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_UINT32_PTR	pValue
)
{
	return	FTM_JSON_getUINT32(pRoot, "count", bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getBeginTime
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_UINT32_PTR	pValue
)
{
	return	FTM_JSON_getUINT32(pRoot, "begin", bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getEndTime
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_UINT32_PTR	pValue
)
{
	return	FTM_JSON_getUINT32(pRoot, "end", bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getSecure
(
	cJSON _PTR_ 	pRoot,
	FTM_BOOL		bEmptyAllow,
	FTM_BOOL_PTR	pValue
)
{
	return	FTM_JSON_getBool(pRoot, "secure", bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getSwitchID
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	return	FTM_JSON_getString(pRoot, "switchid", FTM_ID_LEN, bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getInterval
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_UINT32_PTR	pValue
)
{
	return	FTM_JSON_getUINT32(pRoot, "interval", bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getDate
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_UINT32_PTR	pValue
)
{
	return	FTM_JSON_getUINT32(pRoot, "time", bEmptyAllow, pValue);
}

FTM_RET	FTM_JSON_getString
(
	cJSON _PTR_ 	pRoot,
	FTM_CHAR_PTR	pTitle,
	FTM_UINT32		ulMaxLen,
	FTM_BOOL		bEmptyAllow,
	FTM_CHAR_PTR	pValue
)
{
	cJSON _PTR_ pItem = cJSON_GetObjectItem(pRoot, pTitle);
	if (pItem == NULL)
	{
		if (bEmptyAllow)
		{
			return	FTM_RET_OK;	
		}
		else
		{
			return	FTM_RET_OBJECT_NOT_FOUND;
		}
	}

	if ((pItem->type != cJSON_String) || (strlen(pItem->valuestring) > ulMaxLen))
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	strcpy(pValue, pItem->valuestring);

	return	FTM_RET_OK;
}

FTM_RET	FTM_JSON_getBool
(
	cJSON _PTR_ 	pRoot,
	FTM_CHAR_PTR	pTitle,
	FTM_BOOL		bEmptyAllow,
	FTM_BOOL_PTR	pValue
)
{
	cJSON _PTR_ pItem = cJSON_GetObjectItem(pRoot, pTitle);
	if (pItem == NULL)
	{
		if (bEmptyAllow)
		{
			return	FTM_RET_OK;	
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	if (pItem->type == cJSON_Number)
	{
		*pValue = pItem->valueint;
	}
	else if (pItem->type == cJSON_String)
	{
		if ((strcmp(pItem->valuestring, "on") == 0) ||(strcmp(pItem->valuestring, "true") == 0) ||(strcmp(pItem->valuestring, "yes") == 0) ||(strcmp(pItem->valuestring, "1") == 0))
		{
			*pValue = FTM_TRUE;
		}
		else if ((strcmp(pItem->valuestring, "off") == 0) ||(strcmp(pItem->valuestring, "false") == 0) ||(strcmp(pItem->valuestring, "no") == 0) || (strcmp(pItem->valuestring, "0") == 0))
		{
			*pValue = FTM_FALSE;
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}


	return	FTM_RET_OK;
}

FTM_RET	FTM_JSON_getUINT16
(
	cJSON _PTR_ 	pRoot,
	FTM_CHAR_PTR	pTitle,
	FTM_BOOL		bEmptyAllow,
	FTM_UINT16_PTR	pValue
)
{
	cJSON _PTR_ pItem = cJSON_GetObjectItem(pRoot, pTitle);
	if (pItem == NULL)
	{
		if (bEmptyAllow)
		{
			return	FTM_RET_OK;	
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	if (pItem->type != cJSON_Number)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	*pValue = pItem->valueint;

	return	FTM_RET_OK;
}

FTM_RET	FTM_JSON_getUINT32
(
	cJSON _PTR_ 	pRoot,
	FTM_CHAR_PTR	pTitle,
	FTM_BOOL		bEmptyAllow,
	FTM_UINT32_PTR	pValue
)
{
	cJSON _PTR_ pItem = cJSON_GetObjectItem(pRoot, pTitle);
	if (pItem == NULL)
	{
		if (bEmptyAllow)
		{
			return	FTM_RET_OK;	
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	if (pItem->type != cJSON_Number)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	*pValue = pItem->valueint;

	return	FTM_RET_OK;
}
