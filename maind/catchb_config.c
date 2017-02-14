#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catchb_config.h"
#include "catchb_trace.h"
#include "cjson/cJSON.h"

//////////////////////////////////////////////////////////////
//	CATCHB_SWITCH_CONFIG functions
//////////////////////////////////////////////////////////////
CATCHB_RET	CATCHB_SWITCH_CONFIG_create
(
	CATCHB_SWITCH_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	CATCHB_RET	xRet = CATCHB_RET_OK;
	CATCHB_SWITCH_CONFIG_PTR	pConfig;

	pConfig = (CATCHB_SWITCH_CONFIG_PTR)malloc(sizeof(CATCHB_SWITCH_CONFIG));
	if (pConfig == NULL)
	{
		xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create switch config!\n");
		return	xRet;
	}

	*ppConfig = pConfig;

	return	xRet;
}

CATCHB_RET	CATCHB_SWITCH_CONFIG_destroy
(
	CATCHB_SWITCH_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	ASSERT(*ppConfig != NULL);
	CATCHB_RET	xRet = CATCHB_RET_OK;

	free(*ppConfig);

	*ppConfig = NULL;

	return	xRet;
}

//////////////////////////////////////////////////////////////
//	CATCHB_CONFIG functions
//////////////////////////////////////////////////////////////
CATCHB_RET	CATCHB_CONFIG_create
(
	CATCHB_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	CATCHB_RET			xRet;
	CATCHB_CONFIG_PTR	pConfig = NULL;

	pConfig = (CATCHB_CONFIG_PTR)malloc(sizeof(CATCHB_CONFIG));
	if (pConfig == NULL)
	{
		xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create config!\n");
		return	xRet;
	
	}
	
	xRet = CATCHB_LIST_create(&pConfig->pSwitchList);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to create switch list!\n");
		goto error;
	}

	xRet = CATCHB_LIST_create(&pConfig->pProcessList);
	{
		ERROR(xRet, "Failed to create switch list!\n");
		goto error;
	}

	*ppConfig = pConfig;

	return	CATCHB_RET_OK;

error:
	if (pConfig != NULL)
	{
		if(pConfig->pProcessList != NULL)
		{
			CATCHB_LIST_destroy(&pConfig->pProcessList);
			pConfig->pProcessList = NULL;
		}

		if(pConfig->pSwitchList != NULL)
		{
			CATCHB_LIST_destroy(&pConfig->pSwitchList);
			pConfig->pSwitchList = NULL;
		}

		free(pConfig);
		pConfig = NULL;
	}
	return	xRet;
}

CATCHB_RET	CATCHB_CONFIG_destroy
(
	CATCHB_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	ASSERT(*ppConfig != NULL);

	CATCHB_RET		xRet;
	CATCHB_UINT32	i;
	CATCHB_UINT32	count = 0;

	CATCHB_LIST_count((*ppConfig)->pProcessList, &count);
	for(i = 0 ; i < count ; i++)
	{
		CATCHB_CHAR_PTR	pProcessName;

		xRet = CATCHB_LIST_getAt((*ppConfig)->pProcessList, i, (CATCHB_VOID_PTR _PTR_)&pProcessName);
		if (xRet == CATCHB_RET_OK)
		{
			free(pProcessName);
		}
	}

	xRet = CATCHB_LIST_destroy(&(*ppConfig)->pProcessList);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to destroy process list!\n");	
	}

	CATCHB_LIST_count((*ppConfig)->pSwitchList, &count);
	for(i = 0 ; i < count ; i++)
	{
		CATCHB_SWITCH_CONFIG_PTR	pSwitchConfig;

		xRet = CATCHB_LIST_getAt((*ppConfig)->pSwitchList, i, (CATCHB_VOID_PTR _PTR_)&pSwitchConfig);
		if (xRet == CATCHB_RET_OK)
		{
			CATCHB_SWITCH_CONFIG_destroy(&pSwitchConfig);
		}
	}

	xRet = CATCHB_LIST_destroy(&(*ppConfig)->pSwitchList);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to destroy switch list!\n");	
	}

	free(*ppConfig);

	*ppConfig = NULL;

	return	xRet;
}

CATCHB_RET	CATCHB_CONFIG_load
(
	CATCHB_CONFIG_PTR 	pConfig, 
	char* 				pFileName
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	FILE *pFile; 
	CATCHB_RET		xRet = CATCHB_RET_OK;
	CATCHB_CHAR_PTR	pData = NULL;
	CATCHB_UINT32	ulFileLen;
	CATCHB_UINT32	ulReadSize;
	cJSON _PTR_		pRoot = NULL;
	cJSON _PTR_		pSection;

	pFile = fopen(pFileName, "rt");
	if (pFile == NULL)
	{         
		xRet = CATCHB_RET_CONFIG_LOAD_FAILED; 
		ERROR(xRet, "Can't open file[%s]\n", pFileName);
		return  xRet; 
	}    

	fseek(pFile, 0L, SEEK_END);
	ulFileLen = ftell(pFile);
	fseek(pFile, 0L, SEEK_SET);

	if (ulFileLen > 0)
	{
		pData = (CATCHB_CHAR_PTR)malloc(ulFileLen);
		if (pData != NULL)
		{
			ulReadSize = fread(pData, 1, ulFileLen, pFile); 
			if (ulReadSize != ulFileLen)
			{    
				xRet = CATCHB_RET_FAILED_TO_READ_FILE;
				ERROR(xRet, "Failed to read configuration file[%u:%u]\n", ulFileLen, ulReadSize);
				goto finished;
			}    
		}
		else
		{    
			xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;  
			ERROR(xRet, "Failed to alloc buffer[size = %u]\n", ulFileLen);
			goto finished;
		}    

	}
	fclose(pFile);
	pFile = NULL;

	pRoot = cJSON_Parse(pData);
	if (pRoot == NULL)
	{    
		xRet = CATCHB_RET_INVALID_JSON_FORMAT;
		ERROR(xRet, "Invalid json format!\n");
		goto finished;
	}    

	pSection = cJSON_GetObjectItem(pRoot, "log");
	if (pSection != NULL)
	{
		cJSON _PTR_ pItem;

		pItem = cJSON_GetObjectItem(pSection, "retention");
		if ((pItem != NULL) && (pItem->type == cJSON_Number))
		{
			pConfig->xLog.ulRetentionPeriod = pItem->valueint;
		}
	}

	TRACE_ENTRY();
	pSection = cJSON_GetObjectItem(pRoot, "processes");
	if (pSection->type != cJSON_NULL)
	{
		CATCHB_UINT32	i, ulCount = 0;

	TRACE_ENTRY();
		if(pSection->type != cJSON_Array)
		{
			xRet = CATCHB_RET_CONFIG_INVALID_OBJECT;	
			ERROR(xRet, "Invalid json object!\n");
			goto finished;
		}

		ulCount = cJSON_GetArraySize(pSection);
		for(i = 0 ; i < ulCount ; i++)
		{
			cJSON _PTR_ pObject;

			pObject = cJSON_GetArrayItem(pSection, i);
			if (pObject != NULL)
			{
				cJSON _PTR_ pItem;

				pItem = cJSON_GetObjectItem(pObject, "name");
				if (pItem == NULL)
				{
					ERROR(xRet, "Failed to get switch ip!\n");
					goto finished;
				}

				CATCHB_CHAR_PTR	pProcessName = (CATCHB_CHAR_PTR)malloc(strlen(pItem->valuestring) + 1);
				if (pProcessName != NULL)
				{
					strcpy(pProcessName, pItem->valuestring);
					xRet = CATCHB_LIST_append(pConfig->pSwitchList, pProcessName);
					if (xRet != CATCHB_RET_OK)
					{
						ERROR(xRet, "Failed to append config to list!\n");
						goto finished;	
					}
				}
			}
		}
		
	}

	pSection = cJSON_GetObjectItem(pRoot, "switches");
	if (pSection != NULL)
	{
		CATCHB_UINT32	i, ulCount = 0;

		if(pSection->type != cJSON_Array)
		{
			xRet = CATCHB_RET_CONFIG_INVALID_OBJECT;	
			ERROR(xRet, "Invalid json object!\n");
			goto finished;
		}

		ulCount = cJSON_GetArraySize(pSection);
		for(i = 0 ; i < ulCount ; i++)
		{
			cJSON _PTR_ pObject;

			pObject = cJSON_GetArrayItem(pSection, i);
			if (pObject != NULL)
			{
				CATCHB_SWITCH_CONFIG_PTR	pSwitchConfig;
				CATCHB_SWITCH_CONFIG	xSwitchConfig;
				cJSON _PTR_ pItem;

				memset(&xSwitchConfig, 0, sizeof(xSwitchConfig));

				pItem = cJSON_GetObjectItem(pObject, "ip");
				if (pItem == NULL)
				{
					ERROR(xRet, "Failed to get switch ip!\n");
					goto finished;
				}
				strncpy(xSwitchConfig.pIP, pItem->valuestring, sizeof(xSwitchConfig.pIP) - 1);					
				

				pItem = cJSON_GetObjectItem(pObject, "id");
				if(pItem != NULL)
				{
					strncpy(xSwitchConfig.pID, pItem->valuestring, sizeof(xSwitchConfig.pID) - 1);					
				}
			
				xRet = CATCHB_SWITCH_CONFIG_create(&pSwitchConfig);
				if (xRet != CATCHB_RET_OK)
				{
					ERROR(xRet, "Failed to create switch object!\n");				
					goto finished;
				}
			
				memcpy(pSwitchConfig, &xSwitchConfig, sizeof(xSwitchConfig));

				xRet = CATCHB_LIST_append(pConfig->pSwitchList, pSwitchConfig);
				if (xRet != CATCHB_RET_OK)
				{
					ERROR(xRet, "Failed to append config to list!\n");
					goto finished;	
				}
			}
		}
		
	}

finished:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}

	if (pData != NULL)
	{
		free(pData);
		pData = NULL;
	}

	if (pFile != NULL)
	{
		fclose(pFile);	
		pFile = NULL;
	}

	return	CATCHB_RET_OK;	
}


CATCHB_RET	CATCHB_CONFIG_show
(
	CATCHB_CONFIG_PTR 	pConfig
)
{
	ASSERT(pConfig != NULL);
	CATCHB_RET		xRet;
	CATCHB_UINT32	i, ulCount = 0;

	printf("%16s : %u\n", "Retention Period", pConfig->xLog.ulRetentionPeriod);

	printf("%4s   %s\n", "", "Process");
	CATCHB_LIST_count(pConfig->pProcessList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		CATCHB_CHAR_PTR	pProcessName;

		xRet = CATCHB_LIST_getAt(pConfig->pProcessList, i, (CATCHB_VOID_PTR _PTR_)&pProcessName);
		if (xRet != CATCHB_RET_OK)
		{
			printf("Failed to get process config in list!\n");
		}
		else
		{
			printf("%4d : %s\n", i+1, pProcessName);
		}
	
	}

	printf("%4s   %24s %24s\n", "", "ID", "IP Address");
	CATCHB_LIST_count(pConfig->pSwitchList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		CATCHB_SWITCH_CONFIG_PTR	pSwitchConfig;

		xRet = CATCHB_LIST_getAt(pConfig->pSwitchList, i, (CATCHB_VOID_PTR _PTR_)&pSwitchConfig);
		if (xRet != CATCHB_RET_OK)
		{
			printf("Failed to get switch config in list!\n");
		}
		else
		{
			printf("%4d : %24s %24s\n", i+1, pSwitchConfig->pID, pSwitchConfig->pIP);
		}
	
	}

	return	CATCHB_RET_OK;
}
