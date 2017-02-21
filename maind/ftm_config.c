#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_config.h"
#include "ftm_trace.h"
#include "cjson/cJSON.h"
#include "ftm_mem.h"

//////////////////////////////////////////////////////////////
//	FTM_SWITCH_CONFIG functions
//////////////////////////////////////////////////////////////
FTM_RET	FTM_SWITCH_CONFIG_create
(
	FTM_SWITCH_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_SWITCH_CONFIG_PTR	pConfig;

	pConfig = (FTM_SWITCH_CONFIG_PTR)FTM_MEM_malloc(sizeof(FTM_SWITCH_CONFIG));
	if (pConfig == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create switch config!\n");
		return	xRet;
	}

	memset(pConfig, 0, sizeof(FTM_SWITCH_CONFIG));
	*ppConfig = pConfig;

	return	xRet;
}

FTM_RET	FTM_SWITCH_CONFIG_destroy
(
	FTM_SWITCH_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	ASSERT(*ppConfig != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	FTM_MEM_free(*ppConfig);

	*ppConfig = NULL;

	return	xRet;
}

//////////////////////////////////////////////////////////////
//	FTM_CONFIG functions
//////////////////////////////////////////////////////////////
FTM_RET	FTM_CONFIG_create
(
	FTM_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	FTM_RET			xRet;
	FTM_CONFIG_PTR	pConfig = NULL;

	pConfig = (FTM_CONFIG_PTR)FTM_MEM_malloc(sizeof(FTM_CONFIG));
	if (pConfig == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create config!\n");
		return	xRet;
	
	}
	memset(pConfig, 0, sizeof(FTM_CONFIG));
	
	xRet = FTM_LIST_create(&pConfig->pSwitchList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create switch list!\n");
		goto error;
	}

	xRet = FTM_LIST_create(&pConfig->pProcessList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create switch list!\n");
		goto error;
	}

	*ppConfig = pConfig;

	return	FTM_RET_OK;

error:
	if (pConfig != NULL)
	{
		if(pConfig->pProcessList != NULL)
		{
			FTM_LIST_destroy(&pConfig->pProcessList);
			pConfig->pProcessList = NULL;
		}

		if(pConfig->pSwitchList != NULL)
		{
			FTM_LIST_destroy(&pConfig->pSwitchList);
			pConfig->pSwitchList = NULL;
		}

		FTM_MEM_free(pConfig);
		pConfig = NULL;
	}
	return	xRet;
}

FTM_RET	FTM_CONFIG_destroy
(
	FTM_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	ASSERT(*ppConfig != NULL);

	FTM_RET		xRet;
	FTM_UINT32	i;
	FTM_UINT32	count = 0;

	FTM_LIST_count((*ppConfig)->pProcessList, &count);
	for(i = 0 ; i < count ; i++)
	{
		FTM_CHAR_PTR	pProcessName;

		xRet = FTM_LIST_getAt((*ppConfig)->pProcessList, i, (FTM_VOID_PTR _PTR_)&pProcessName);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pProcessName);
		}
	}

	xRet = FTM_LIST_destroy(&(*ppConfig)->pProcessList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to destroy process list!\n");	
	}

	FTM_LIST_count((*ppConfig)->pSwitchList, &count);
	for(i = 0 ; i < count ; i++)
	{
		FTM_SWITCH_CONFIG_PTR	pSwitchConfig;

		xRet = FTM_LIST_getAt((*ppConfig)->pSwitchList, i, (FTM_VOID_PTR _PTR_)&pSwitchConfig);
		if (xRet == FTM_RET_OK)
		{
			FTM_SWITCH_CONFIG_destroy(&pSwitchConfig);
		}
	}

	xRet = FTM_LIST_destroy(&(*ppConfig)->pSwitchList);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to destroy switch list!\n");	
	}

	FTM_MEM_free(*ppConfig);

	*ppConfig = NULL;

	return	xRet;
}

FTM_RET	FTM_CONFIG_load
(
	FTM_CONFIG_PTR 	pConfig, 
	char* 				pFileName
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	FILE *pFile; 
	FTM_RET		xRet = FTM_RET_OK;
	FTM_CHAR_PTR	pData = NULL;
	FTM_UINT32	ulFileLen;
	FTM_UINT32	ulReadSize;
	cJSON _PTR_		pRoot = NULL;
	cJSON _PTR_		pSection;

	pFile = fopen(pFileName, "rt");
	if (pFile == NULL)
	{         
		xRet = FTM_RET_CONFIG_LOAD_FAILED; 
		ERROR(xRet, "Can't open file[%s]\n", pFileName);
		return  xRet; 
	}    

	fseek(pFile, 0L, SEEK_END);
	ulFileLen = ftell(pFile);
	fseek(pFile, 0L, SEEK_SET);

	if (ulFileLen > 0)
	{
		pData = (FTM_CHAR_PTR)FTM_MEM_malloc(ulFileLen);
		if (pData != NULL)
		{
			memset(pData, 0, ulFileLen);
			ulReadSize = fread(pData, 1, ulFileLen, pFile); 
			if (ulReadSize != ulFileLen)
			{    
				xRet = FTM_RET_FAILED_TO_READ_FILE;
				ERROR(xRet, "Failed to read configuration file[%u:%u]\n", ulFileLen, ulReadSize);
				goto finished;
			}    
		}
		else
		{    
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;  
			ERROR(xRet, "Failed to alloc buffer[size = %u]\n", ulFileLen);
			goto finished;
		}    

	}
	fclose(pFile);
	pFile = NULL;

	TRACE_ENTRY();
	pRoot = cJSON_Parse(pData);
	if (pRoot == NULL)
	{    
		xRet = FTM_RET_INVALID_JSON_FORMAT;
		ERROR(xRet, "Invalid json format!\n");
		goto finished;
	}    

	TRACE_ENTRY();
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
	if (pSection != NULL)
	{
		FTM_UINT32	i, ulCount = 0;

	TRACE_ENTRY();
		if(pSection->type != cJSON_Array)
		{
			xRet = FTM_RET_CONFIG_INVALID_OBJECT;	
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

				FTM_CHAR_PTR	pProcessName = (FTM_CHAR_PTR)FTM_MEM_malloc(strlen(pItem->valuestring) + 1);
				if (pProcessName != NULL)
				{
					strcpy(pProcessName, pItem->valuestring);
					xRet = FTM_LIST_append(pConfig->pSwitchList, pProcessName);
					if (xRet != FTM_RET_OK)
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
		FTM_UINT32	i, ulCount = 0;

		if(pSection->type != cJSON_Array)
		{
			xRet = FTM_RET_CONFIG_INVALID_OBJECT;	
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
				FTM_SWITCH_CONFIG_PTR	pSwitchConfig;
				FTM_SWITCH_CONFIG	xSwitchConfig;
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
			
				xRet = FTM_SWITCH_CONFIG_create(&pSwitchConfig);
				if (xRet != FTM_RET_OK)
				{
					ERROR(xRet, "Failed to create switch object!\n");				
					goto finished;
				}
			
				memcpy(pSwitchConfig, &xSwitchConfig, sizeof(xSwitchConfig));

				xRet = FTM_LIST_append(pConfig->pSwitchList, pSwitchConfig);
				if (xRet != FTM_RET_OK)
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
		FTM_MEM_free(pData);
		pData = NULL;
	}

	if (pFile != NULL)
	{
		fclose(pFile);	
		pFile = NULL;
	}

	return	FTM_RET_OK;	
}


FTM_RET	FTM_CONFIG_show
(
	FTM_CONFIG_PTR 	pConfig
)
{
	ASSERT(pConfig != NULL);
	FTM_RET		xRet;
	FTM_UINT32	i, ulCount = 0;

	printf("%16s : %u\n", "Retention Period", pConfig->xLog.ulRetentionPeriod);

	printf("%4s   %s\n", "", "Process");
	FTM_LIST_count(pConfig->pProcessList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR_PTR	pProcessName;

		xRet = FTM_LIST_getAt(pConfig->pProcessList, i, (FTM_VOID_PTR _PTR_)&pProcessName);
		if (xRet != FTM_RET_OK)
		{
			printf("Failed to get process config in list!\n");
		}
		else
		{
			printf("%4d : %s\n", i+1, pProcessName);
		}
	
	}

	printf("%4s   %24s %24s\n", "", "ID", "IP Address");
	FTM_LIST_count(pConfig->pSwitchList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_SWITCH_CONFIG_PTR	pSwitchConfig;

		xRet = FTM_LIST_getAt(pConfig->pSwitchList, i, (FTM_VOID_PTR _PTR_)&pSwitchConfig);
		if (xRet != FTM_RET_OK)
		{
			printf("Failed to get switch config in list!\n");
		}
		else
		{
			printf("%4d : %24s %24s\n", i+1, pSwitchConfig->pID, pSwitchConfig->pIP);
		}
	
	}

	return	FTM_RET_OK;
}
