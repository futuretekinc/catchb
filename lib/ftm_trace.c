#include <execinfo.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ftm_trace.h"
#include "ftm_time.h"
#include "ftm_lock.h"

extern	
FTM_CHAR_PTR	program_invocation_short_name;

static
FTM_BOOL	bInit = FTM_FALSE;
static
FTM_LOCK	xLock;

static
FTM_RET	FTM_TRACE_getFileSize
(
	FTM_CHAR_PTR	pFileName,
	FTM_UINT32_PTR	pSize
);

static
FTM_RET	FTM_TRACE_writeToFile
(
	FTM_CHAR_PTR	pFileName,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
);

static const 
FTM_TRACE_CONFIG	xDefaultConfig =
{
	.pPath = "/var/log",
	.ulFileSize = FTM_CATCHB_TRACE_LOG_SIZE,
	.xConsole =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "console"
	},
	.xLog =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "catchb.log",
	},
	.xInfo =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "catchb.log",
	},
	.xWarn =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "catchb.log",
	},
	.xError =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "catchb.log",
	}
};

FTM_TRACE_CONFIG	xTraceConfig =
{
	.pPath = "/var/log",
	.ulFileSize = FTM_CATCHB_TRACE_LOG_SIZE,
	.xConsole =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable	= FTM_FALSE,
			.bDynamic	= FTM_FALSE,
			.ulSize 	= FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "coneole",
	},
	.xLog =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "catchb.log",
	},
	.xInfo =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "catchb.log",
	},
	.xWarn =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "catchb.log",
	},
	.xError =
	{
		.bEnable = FTM_TRUE,
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.bDynamic= FTM_FALSE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "catchb.log",
	}
};

static FTM_UINT32	ulModuleCount = 0;
static FTM_CHAR	xModuleName[32][32] =
{
	"cgi"
};

FTM_RET		FTM_TRACE_CONFIG_setDefault
(
	FTM_TRACE_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &xDefaultConfig, sizeof(FTM_TRACE_CONFIG));
	sprintf(pConfig->xLog.pFileName, "%s.log", program_invocation_short_name);
	sprintf(pConfig->xInfo.pFileName, "%s.log", program_invocation_short_name);
	sprintf(pConfig->xWarn.pFileName, "%s.log", program_invocation_short_name);
	sprintf(pConfig->xError.pFileName, "%s.log", program_invocation_short_name);

	return	FTM_RET_OK;
}

FTM_RET		FTM_TRACE_FIELD_CONFIG_load
(
	FTM_TRACE_FIELD_CONFIG_PTR	pConfig,
	cJSON _PTR_				pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pItem;

	pItem = cJSON_GetObjectItem(pRoot, "enable");
	if (pItem != NULL)
	{
		if (pItem->type == cJSON_String)
		{
			if ((strcmp(pItem->valuestring, "yes") == 0) || (strcmp(pItem->valuestring, "on") == 0))
			{
				pConfig->bEnable = FTM_TRUE;
			}
			else if ((strcmp(pItem->valuestring, "no") == 0) || (strcmp(pItem->valuestring, "off") == 0))
			{
				pConfig->bEnable = FTM_FALSE;
			}
		}
	}

	pItem = cJSON_GetObjectItem(pRoot, "dynamic");
	if (pItem != NULL)
	{
		if (pItem->type == cJSON_String)
		{
			if ((strcmp(pItem->valuestring, "yes") == 0) || (strcmp(pItem->valuestring, "on") == 0))
			{
				pConfig->bDynamic = FTM_TRUE;
			}
			else if ((strcmp(pItem->valuestring, "no") == 0) || (strcmp(pItem->valuestring, "off") == 0))
			{
				pConfig->bDynamic = FTM_FALSE;
			}
		}
	}

	pItem = cJSON_GetObjectItem(pRoot, "size");
	if (pItem != NULL)
	{
		if (pItem->type == cJSON_Number)
		{
			pConfig->ulSize = pItem->valueint;	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET		FTM_TRACE_TYPE_CONFIG_load
(
	FTM_TRACE_TYPE_CONFIG_PTR	pConfig,
	cJSON _PTR_				pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pSection;
	cJSON _PTR_ pItem;

	pItem = cJSON_GetObjectItem(pRoot, "enable");
	if (pItem != NULL)
	{
		if (pItem->type == cJSON_String)
		{
			if ((strcmp(pItem->valuestring, "yes") == 0) || (strcmp(pItem->valuestring, "on") == 0))
			{
				pConfig->bEnable = FTM_TRUE;
			}
			else if ((strcmp(pItem->valuestring, "no") == 0) || (strcmp(pItem->valuestring, "off") == 0))
			{
				pConfig->bEnable = FTM_FALSE;
			}
		}
	}

	pSection = cJSON_GetObjectItem(pRoot, "time");
	if (pSection != NULL)
	{
		FTM_TRACE_FIELD_CONFIG_load(&pConfig->xTime, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "function");
	if (pSection != NULL)
	{
		FTM_TRACE_FIELD_CONFIG_load(&pConfig->xFunction, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "line");
	if (pSection != NULL)
	{
		FTM_TRACE_FIELD_CONFIG_load(&pConfig->xLine, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "module");
	if (pSection != NULL)
	{
		FTM_TRACE_FIELD_CONFIG_load(&pConfig->xModule, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "level");
	if (pSection != NULL)
	{
		FTM_TRACE_FIELD_CONFIG_load(&pConfig->xLevel, pSection);	
	}

	return	FTM_RET_OK;
}

FTM_RET		FTM_TRACE_TYPE_CONFIG_save
(
	FTM_TRACE_TYPE_CONFIG_PTR	pConfig,
	cJSON _PTR_				pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pSection;

	cJSON_AddStringToObject(pRoot, "enable", ((pConfig->bEnable)?"yes":"no"));
	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		cJSON_AddStringToObject(pSection, "enable", ((pConfig->xTime.bEnable)?"yes":"no"));
		cJSON_AddStringToObject(pSection, "dynamic", ((pConfig->xTime.bDynamic)?"yes":"no"));
		cJSON_AddNumberToObject(pSection, "size", pConfig->xTime.ulSize);
	
		cJSON_AddItemToObject(pRoot, "time", pSection);
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		cJSON_AddStringToObject(pSection, "enable", ((pConfig->xFunction.bEnable)?"yes":"no"));
		cJSON_AddStringToObject(pSection, "dynamic", ((pConfig->xFunction.bDynamic)?"yes":"no"));
		cJSON_AddNumberToObject(pSection, "size", pConfig->xFunction.ulSize);
	
		cJSON_AddItemToObject(pRoot, "function", pSection);
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		cJSON_AddStringToObject(pSection, "enable", ((pConfig->xLine.bEnable)?"yes":"no"));
		cJSON_AddStringToObject(pSection, "dynamic", ((pConfig->xLine.bDynamic)?"yes":"no"));
		cJSON_AddNumberToObject(pSection, "size", pConfig->xLine.ulSize);
	
		cJSON_AddItemToObject(pRoot, "line", pSection);
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		cJSON_AddStringToObject(pSection, "enable", ((pConfig->xModule.bEnable)?"yes":"no"));
		cJSON_AddStringToObject(pSection, "dynamic", ((pConfig->xModule.bDynamic)?"yes":"no"));
		cJSON_AddNumberToObject(pSection, "size", pConfig->xModule.ulSize);
	
		cJSON_AddItemToObject(pRoot, "module", pSection);
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		cJSON_AddStringToObject(pSection, "enable", ((pConfig->xLevel.bEnable)?"yes":"no"));
		cJSON_AddStringToObject(pSection, "dynamic", ((pConfig->xLevel.bDynamic)?"yes":"no"));
		cJSON_AddNumberToObject(pSection, "size", pConfig->xLevel.ulSize);
	
		cJSON_AddItemToObject(pRoot, "level", pSection);
	}

	pSection = cJSON_CreateArray();
	if (pSection != NULL)
	{
		for(FTM_UINT32 i = 0 ; i < ulModuleCount ; i++)
		{
			cJSON_AddItemToArray(pSection, cJSON_CreateString(xModuleName[i]));
		}
		cJSON_AddItemToObject(pRoot, "module", pSection);
	}

	return	FTM_RET_OK;
}

FTM_RET		FTM_TRACE_CONFIG_load
(
	FTM_TRACE_CONFIG_PTR	pConfig,
	cJSON _PTR_				pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pSection;

	pSection = cJSON_GetObjectItem(pRoot, "common");
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xLog, pSection);	
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xInfo, pSection);	
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xError, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "log");
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xLog, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "info");
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xInfo, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "warn");
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xWarn, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "error");
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xError, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "module");
	if (pSection != NULL)
	{
		if (pSection->type == cJSON_Array)
		{
			FTM_UINT32	ulCount;

			ulCount = cJSON_GetArraySize(pSection);
			for(FTM_UINT32 i = 0 ; i < ulCount ; i++)
			{
				cJSON _PTR_ pObject;

				pObject = cJSON_GetArrayItem(pSection, i);
				if ((pObject != NULL) && (ulModuleCount < 32))
				{
					strncpy(xModuleName[ulModuleCount++], pObject->valuestring, sizeof(xModuleName[0])-1);
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET		FTM_TRACE_CONFIG_save
(
	FTM_TRACE_CONFIG_PTR	pConfig,
	cJSON _PTR_				pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pSection;

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_save(&pConfig->xLog, pSection);	

		cJSON_AddItemToObject(pRoot, "log", pSection);
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_save(&pConfig->xInfo, pSection);	

		cJSON_AddItemToObject(pRoot, "info", pSection);
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_save(&pConfig->xWarn, pSection);	

		cJSON_AddItemToObject(pRoot, "warn", pSection);
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_save(&pConfig->xError, pSection);	

		cJSON_AddItemToObject(pRoot, "error", pSection);
	}

	return	FTM_RET_OK;
}

FTM_RET		FTM_TRACE_setConfig
(
	FTM_TRACE_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	memcpy(&xTraceConfig, pConfig, sizeof(FTM_TRACE_CONFIG));

	return	FTM_RET_OK;
}

FTM_RET		FTM_TRACE_getConfig
(
	FTM_TRACE_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &xTraceConfig, sizeof(FTM_TRACE_CONFIG));

	return	FTM_RET_OK;
}

FTM_VOID	FTM_TRACE_SystemError
(
	const FTM_CHAR_PTR pFormat,
	...
)
{
    va_list xArgs;

    va_start(xArgs, pFormat);
    fprintf(stdout, "ERROR : ");
    vfprintf(stdout, pFormat, xArgs);
    fprintf(stdout, "\n");
    va_end(xArgs);
}

FTM_VOID	FTM_TRACE_Out
(
		  FTM_TRACE_LEVEL	xLevel,
		  FTM_BOOL		bEnable,
	const char *		pFunctionName,
		  FTM_UINT32	ulLine,
	const FTM_CHAR_PTR	pModuleName,
	const FTM_CHAR_PTR 	pFormat,
	...
)
{

    va_list 	xArgs;
	FTM_CHAR	pOutputFormat[256];
	FTM_CHAR	pBuffer[2048];	
	FTM_UINT32	ulLen = 0;
	FTM_TRACE_TYPE_CONFIG_PTR	pConfig;

	if (!bInit)
	{
		FTM_LOCK_init(&xLock);

		bInit = FTM_TRUE;
	}

	if (!bEnable)
	{
		return;
	}

	switch(xLevel)
	{
	case	FTM_TRACE_LEVEL_CONSOLE:pConfig = &xTraceConfig.xConsole; 	break;
	case	FTM_TRACE_LEVEL_LOG:	pConfig = &xTraceConfig.xLog; 		break;
	case	FTM_TRACE_LEVEL_INFO:	pConfig = &xTraceConfig.xInfo; 		break;
	case	FTM_TRACE_LEVEL_WARN:	pConfig = &xTraceConfig.xWarn; 		break;
	case	FTM_TRACE_LEVEL_ERROR:	pConfig = &xTraceConfig.xError; 	break;
	default:	return;
	}

	if (!pConfig->bEnable)
	{
		return;
	}

	bEnable = FTM_FALSE;

	if (pModuleName != 0)
	{
		for(int i = 0 ; i < ulModuleCount ; i++)
		{
			if (strcmp(pModuleName, xModuleName[i]) == 0)
			{
				bEnable =FTM_TRUE;
				break;	
			}
		}
	}

	if (!bEnable)
	{
		return;	
	}

	FTM_LOCK_set(&xLock);

	memset(pOutputFormat, 0, sizeof(pOutputFormat));
	memset(pBuffer, 0, sizeof(pBuffer));

	if (pConfig->xTime.bEnable)
	{
		FTM_CHAR	pTimeString[64];
		
		strcpy(pTimeString, FTM_TIME_printfCurrent(NULL));

		if (pConfig->xTime.bDynamic)
		{
			FTM_UINT32	ulStringLen = strlen(pTimeString);
			if (pConfig->xTime.ulSize < ulStringLen)
			{
				pConfig->xTime.ulSize = ulStringLen;
			}
		}

		if (pConfig->xTime.ulSize == 0)
		{
			sprintf(pOutputFormat, "[%%s] ");
		}
		else
		{
			sprintf(pOutputFormat, "[%%%u.%us] ", pConfig->xTime.ulSize, pConfig->xTime.ulSize);
		}

		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pOutputFormat, pTimeString);
	}

	if (pConfig->xFunction.bEnable)
	{
		if (pFunctionName == NULL)
		{
			pFunctionName = "";
		}

		if (pConfig->xFunction.bDynamic)
		{
			FTM_UINT32	ulStringLen = strlen(pFunctionName);
			if (pConfig->xFunction.ulSize < ulStringLen)
			{
				pConfig->xFunction.ulSize = ulStringLen;
			}
		}

		if (pConfig->xFunction.ulSize == 0)
		{
			sprintf(pOutputFormat, "[%%s] ");
		}
		else
		{
			sprintf(pOutputFormat, "[%%%u.%us] ", pConfig->xFunction.ulSize,pConfig->xFunction.ulSize);
		}
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pOutputFormat, pFunctionName);
	}

	if (pConfig->xLine.bEnable)
	{
		if (pConfig->xLine.bDynamic)
		{
			FTM_CHAR	pNumberString[64];

			sprintf(pNumberString, "%u", ulLine);

			FTM_UINT32	ulStringLen = strlen(pNumberString);
			if (pConfig->xLine.ulSize < ulStringLen)
			{
				pConfig->xLine.ulSize = ulStringLen;
			}
		}

		if (pConfig->xLine.ulSize == 0)
		{
			sprintf(pOutputFormat, "[%%u] ");
		}
		else
		{
			sprintf(pOutputFormat, "[%%%uu] ", pConfig->xLine.ulSize);
		}
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pOutputFormat, ulLine);
	}

	if (pConfig->xModule.bEnable)
	{
		if (pConfig->xModule.bDynamic)
		{
			if (pModuleName != 0)
			{
				FTM_UINT32	ulStringLen = strlen(pModuleName);
				if (pConfig->xModule.ulSize < ulStringLen)
				{
					pConfig->xModule.ulSize = ulStringLen;
				}
			}
		}

		if (pConfig->xModule.ulSize == 0)
		{
			sprintf(pOutputFormat, "[%%s] ");
		}
		else
		{
			sprintf(pOutputFormat, "[%%%u.%us] ", pConfig->xModule.ulSize, pConfig->xModule.ulSize);
		}

		if(pModuleName != NULL)
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pOutputFormat, pModuleName);
		}
		else
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pOutputFormat, "common");
		}
	}

	if (pConfig->xLevel.bEnable)
	{
		FTM_CHAR_PTR	pLevel = FTM_TRACE_LEVEL_string(xLevel);

		if (pConfig->xLevel.bDynamic)
		{
			FTM_UINT32	ulStringLen = strlen(pLevel);
			if (pConfig->xLevel.ulSize < ulStringLen)
			{
				pConfig->xLevel.ulSize = ulStringLen;
			}
		}

		if (pConfig->xLevel.ulSize == 0)
		{
			sprintf(pOutputFormat, "[%%s] ");
		}
		else
		{
			sprintf(pOutputFormat, "[%%%u.%us] ", pConfig->xLevel.ulSize, pConfig->xLevel.ulSize);
		}
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pOutputFormat, pLevel);
	}

    va_start(xArgs, pFormat);
    ulLen += vsnprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pFormat, xArgs);
    va_end(xArgs);

	pBuffer[ulLen++] = '\n';
	pBuffer[ulLen] = '\0';

	if (strcmp(pConfig->pFileName, "console") == 0)
	{
        printf("%s", pBuffer);
	}
	else
	{
		FTM_RET		xRet;
		FTM_CHAR	pFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN + 1];
		FTM_UINT32	ulBufferLen = strlen(pBuffer);
		FTM_UINT32	ulOffset = 0;
		FTM_UINT32	ulFileSize = 0;

		sprintf(pFileName, "%s/%s", xTraceConfig.pPath, pConfig->pFileName);

		xRet = FTM_TRACE_getFileSize(pFileName, &ulFileSize);
		if (xRet == FTM_RET_OK)
		{
			if (ulFileSize >= xTraceConfig.ulFileSize)
			{
				FTM_INT32	i;
				FTM_CHAR	pCommand[128];
				FILE* 		pFP;

				for(i = 10 ; i > 0 ; i--)
				{

					sprintf(pCommand, "mv -f %s.%d %s.%d > /dev/null 2>&1 ", pFileName, i-1, pFileName, i);
					pFP = popen(pCommand, "r");
					if (pFP != NULL)
					{
						pclose(pFP);
					}
				}

				sprintf(pCommand, "mv -f %s %s.0 > /dev/null 2>&1 ", pFileName, pFileName);
				pFP = popen(pCommand, "r");
				if (pFP != NULL)
				{
					pclose(pFP);
				}
			}

			FTM_TRACE_writeToFile(pFileName, &pBuffer[ulOffset], ulBufferLen);
		}
	}

	FTM_LOCK_reset(&xLock);
}

FTM_RET	FTM_TRACE_writeToFile
(
	FTM_CHAR_PTR	pFileName,
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen
)
{
	ASSERT(pFileName != NULL);
	ASSERT(pBuffer != NULL);

	FILE _PTR_	pFile;
	FTM_RET		xRet = FTM_RET_OK;	

#if 1
	pFile = fopen(pFileName, "a");
	if(pFile == NULL)
	{
		xRet = FTM_RET_ERROR;
		fprintf(stderr, "Failed to open file[%s:%s]\n", pFileName,strerror(errno));
		return	xRet;
	}

	fwrite(pBuffer, 1, ulBufferLen, pFile);
	//fflush(pFile);
	fclose(pFile);
#else
	FTM_CHAR	pCommandLine[2048];

	pBuffer[strlen(pBuffer) -1] = 0;
	sprintf(pCommandLine, "echo \"%s\" >> %s", pBuffer, pFileName);
	pFile = popen(pCommandLine, "a");
	if (pFile == NULL)
	{
		xRet = FTM_RET_ERROR;
		fprintf(stderr, "Failed to open file[%s:%s]\n", pFileName,strerror(errno));
		fprintf(stderr, "Command Line : %s\n", pCommandLine);
		return	xRet;
	}

	pclose(pFile);

#endif
	return	xRet;
}

FTM_RET	FTM_TRACE_getFileSize
(
	FTM_CHAR_PTR	pFileName,
	FTM_UINT32_PTR	pSize
)
{
	ASSERT(pFileName != NULL);
	ASSERT(pSize != NULL);

	FILE _PTR_	pFile;

	pFile = fopen(pFileName, "a");
	if(pFile == NULL)
	{
		*pSize = 0;
	}
	else
	{
		fseek(pFile, 0, SEEK_END);
		*pSize = ftell(pFile);
		fclose(pFile);
	}

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTM_TRACE_LEVEL_string
(
	FTM_TRACE_LEVEL	xLevel
)
{
	switch(xLevel)
	{
	case	FTM_TRACE_LEVEL_CONSOLE:	return	"CNSL";
	case	FTM_TRACE_LEVEL_LOG:	return	"LOG";
	case	FTM_TRACE_LEVEL_INFO:	return	"INFO";
	case	FTM_TRACE_LEVEL_WARN:	return	"WARN";
	case	FTM_TRACE_LEVEL_ERROR:	return	"ERROR";
	}

	return	"UNKNOWN";
}

FTM_VOID	FTM_TRACE_asserted
(
)
{
	FTM_VOID_PTR	pArray[10];
 	FTM_UINT32		ulSize;
 	FTM_CHAR_PTR _PTR_ ppStrings;
 	size_t i;

 	ulSize = backtrace (pArray, 10);
 	ppStrings = backtrace_symbols (pArray, ulSize );

 	INFO("Obtained %zd stack frames.\n", ulSize);

 	for (i = 0; i < ulSize; i++)
	{
 		INFO("%s\n", ppStrings[i]);
	}

 	free (ppStrings);
}

FTM_RET	FTM_TRACE_getModuleCount
(
	FTM_UINT32_PTR	pModuleCount
)
{
	*pModuleCount = ulModuleCount;

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTM_TRACE_getModuleName
(
	FTM_UINT32	ulIndex
)
{
	if (ulIndex < ulModuleCount)
	{
		return	xModuleName[ulIndex];
	}

	return	NULL;
}
