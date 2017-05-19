#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_config.h"
#include "ftm_catchb.h"
#include "ftm_trace.h"
#include "cjson/cJSON.h"
#include "ftm_mem.h"
#include "ftm_switch.h"
#include "ftm_logger.h"

#undef	__MODULE__
#define	__MODULE__ "config"

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

	FTM_CONFIG_setDefault(pConfig);

	*ppConfig = pConfig;

	return	FTM_RET_OK;

error:
	if (pConfig != NULL)
	{
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

FTM_RET	FTM_CONFIG_setDefault
(	
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	FTM_SYSTEM_CONFIG_setDefault(&pConfig->xSystem);
	FTM_DB_CONFIG_setDefault(&pConfig->xDB);
	FTM_ANALYZER_CONFIG_setDefault(&pConfig->xAnalyzer);
	FTM_NOTIFIER_CONFIG_setDefault(&pConfig->xNotifier);
	FTM_LOGGER_CONFIG_setDefault(&pConfig->xLogger);
	FTM_TRACE_CONFIG_setDefault(&pConfig->xTrace);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONFIG_load
(
	FTM_CONFIG_PTR 	pConfig, 
	char* 			pFileName
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

	strncpy(pConfig->pFileName, pFileName, sizeof(pConfig->pFileName) - 1);

	pFile = fopen(pConfig->pFileName, "rt");
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

	pRoot = cJSON_Parse(pData);
	if (pRoot == NULL)
	{    
		xRet = FTM_RET_INVALID_JSON_FORMAT;
		ERROR(xRet, "Invalid json format!\n");
		goto finished;
	}    

	pSection = cJSON_GetObjectItem(pRoot, "system");
	if (pSection != NULL)
	{
		FTM_SYSTEM_CONFIG_load(&pConfig->xSystem, pSection);
	}

	pSection = cJSON_GetObjectItem(pRoot, "database");
	if (pSection != NULL)
	{
		FTM_DB_CONFIG_load(&pConfig->xDB, pSection);
	}

	pSection = cJSON_GetObjectItem(pRoot, "analyzer");
	if (pSection != NULL)
	{
		FTM_ANALYZER_CONFIG_load(&pConfig->xAnalyzer, pSection);
	}

	pSection = cJSON_GetObjectItem(pRoot, "notifier");
	if (pSection != NULL)
	{
		FTM_NOTIFIER_CONFIG_load(&pConfig->xNotifier, pSection);
	}

	pSection = cJSON_GetObjectItem(pRoot, "logger");
	if (pSection != NULL)
	{
		FTM_LOGGER_CONFIG_load(&pConfig->xLogger, pSection);
	}

	pSection = cJSON_GetObjectItem(pRoot, "trace");
	if (pSection != NULL)
	{
		FTM_TRACE_CONFIG_load(&pConfig->xTrace, pSection);
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

FTM_RET	FTM_CONFIG_save
(
	FTM_CONFIG_PTR 	pConfig, 
	char* 			pFileName
)
{
	ASSERT(pConfig != NULL);

	FILE *pFile; 
	FTM_RET		xRet = FTM_RET_OK;
	cJSON _PTR_		pRoot = NULL;
	cJSON _PTR_		pSection;
	
	pRoot = cJSON_CreateObject();
	if (pRoot == NULL)
	{    
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to save configuration due to not allocated memory!\n");
		goto finished;
	}    

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		xRet = FTM_SYSTEM_CONFIG_save(&pConfig->xSystem, pSection);
		if (xRet == FTM_RET_OK)
		{
			cJSON_AddItemToObject(pRoot, "system", pSection);	
		}
		else
		{
			cJSON_Delete(pSection);
		}
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		xRet = FTM_DB_CONFIG_save(&pConfig->xDB, pSection);
		if (xRet == FTM_RET_OK)
		{
			cJSON_AddItemToObject(pRoot, "database", pSection);	
		}
		else
		{
			cJSON_Delete(pSection);
		}
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		xRet = FTM_ANALYZER_CONFIG_save(&pConfig->xAnalyzer, pSection);
		if (xRet == FTM_RET_OK)
		{
			cJSON_AddItemToObject(pRoot, "analyzer", pSection);	
		}
		else
		{
			cJSON_Delete(pSection);
		}
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		xRet = FTM_NOTIFIER_CONFIG_save(&pConfig->xNotifier, pSection);
		if (xRet == FTM_RET_OK)
		{
			cJSON_AddItemToObject(pRoot, "notifier", pSection);	
		}
		else
		{
			cJSON_Delete(pSection);
		}
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		xRet = FTM_LOGGER_CONFIG_save(&pConfig->xLogger, pSection);
		if (xRet == FTM_RET_OK)
		{
			cJSON_AddItemToObject(pRoot, "logger", pSection);	
		}
		else
		{
			cJSON_Delete(pSection);
		}
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		xRet = FTM_TRACE_CONFIG_save(&pConfig->xTrace, pSection);
		if (xRet == FTM_RET_OK)
		{
			cJSON_AddItemToObject(pRoot, "trace", pSection);	
		}
		else
		{
			cJSON_Delete(pSection);
		}
	}

	
	if (pFileName != NULL)
	{
		INFO("Configuraion save to %s", pFileName);
		pFile = fopen(pFileName, "wt"); 
	}
	else
	{
		INFO("Configuraion save to %s", pConfig->pFileName);
		pFile = fopen(pConfig->pFileName, "wt");
	}

	if (pFile != NULL)
	{
		fprintf(pFile, "%s", cJSON_Print(pRoot));

		fclose(pFile);
	}
	else
	{
		ERROR(FTM_RET_ERROR, "Failed to save configuraion");
	}

finished:
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}

	return	xRet;	
}


FTM_RET	FTM_CONFIG_show
(
	FTM_CONFIG_PTR 	pConfig,
	FTM_TRACE_LEVEL	xLevel
)
{
	ASSERT(pConfig != NULL);

	FTM_SYSTEM_CONFIG_show(&pConfig->xSystem, xLevel);

	FTM_DB_CONFIG_show(&pConfig->xDB, xLevel);

	FTM_ANALYZER_CONFIG_show(&pConfig->xAnalyzer, xLevel);

	FTM_NOTIFIER_CONFIG_show(&pConfig->xNotifier, xLevel);

	FTM_LOGGER_CONFIG_show(&pConfig->xLogger, xLevel);


	return	FTM_RET_OK;
}

