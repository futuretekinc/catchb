
#include <stdio.h>
#include <stdarg.h>
#include <liblogs.h>
#include <time.h>
#include <sys/time.h>

//test
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <liblogs.h>
#include <string.h>

#include <common.h>
#include "ftm_trace.h"
#include "ftm_time.h"

static const 
FTM_TRACE_CONFIG	xDefaultConfig =
{
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
		.pFileName="stdout"
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
		.pFileName = "/var/log/catchb.log"
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
		.pFileName = "/var/log/catchb.trace"
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
		.pFileName = "/var/log/catchb.trace"
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
		.pFileName = "/var/log/catchb.trace"
	}
};

FTM_TRACE_CONFIG	xTraceConfig =
{
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
		.pFileName="stdout"
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
		.pFileName = "/var/log/catchb.log"
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
		.pFileName = "/var/log/catchb.trace"
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
		.pFileName = "/var/log/catchb.trace"
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
		.pFileName = "/var/log/catchb.trace"
	}
};

FTM_RET		FTM_TRACE_CONFIG_setDefault
(
	FTM_TRACE_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	memcpy(pConfig, &xDefaultConfig, sizeof(FTM_TRACE_CONFIG));

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

    FILE*		pFile;
    va_list 	xArgs;
	FTM_CHAR	pOutputFormat[256];
	FTM_CHAR	pBuffer[2048];	
	FTM_UINT32	ulLen = 0;
	FTM_TRACE_TYPE_CONFIG_PTR	pConfig;

	if (!bEnable)
	{
		return;
	}

	switch(xLevel)
	{
	case	FTM_TRACE_LEVEL_CONSOLE:pConfig = &xTraceConfig.xConsole; 	break;
	case	FTM_TRACE_LEVEL_LOG:	pConfig = &xTraceConfig.xLog; 	break;
	case	FTM_TRACE_LEVEL_INFO:	pConfig = &xTraceConfig.xInfo; 	break;
	case	FTM_TRACE_LEVEL_WARN:	pConfig = &xTraceConfig.xWarn; 	break;
	case	FTM_TRACE_LEVEL_ERROR:	pConfig = &xTraceConfig.xError; 	break;
	default:	return;
	}

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

	if (strcmp(pConfig->pFileName, "stdout") == 0)
	{
        printf("%s", pBuffer);
	}
	else
	{
   		pFile = fopen(pConfig->pFileName, "a");
		if(pFile)
		{
			fprintf(pFile, "%s", pBuffer);
			fclose(pFile);
		}
	}
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
