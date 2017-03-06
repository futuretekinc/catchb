
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

static const 
FTM_TRACE_CONFIG	xDefaultConfig =
{
	.xLog =
	{
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "/var/log/catchb.log"
	},
	.xTrace =
	{
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "/var/log/catchb.trace"
	},
	.xError =
	{
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "/var/log/catchb.trace"
	}
};

static 
FTM_TRACE_CONFIG	xConfig =
{
	.xLog =
	{
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "/var/log/catchb.log"
	},
	.xTrace =
	{
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN
		},
		.pFileName = "/var/log/catchb.trace"
	},
	.xError =
	{
		.xTime = 
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN
		},
		.xFunction =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN
		},
		.xLine =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN
		},
		.xModule =
		{
			.bEnable= FTM_TRUE,
			.ulSize = FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN
		},
		.xLevel =
		{
			.bEnable= FTM_TRUE,
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
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xTrace, pSection);	
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xError, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "log");
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xLog, pSection);	
	}

	pSection = cJSON_GetObjectItem(pRoot, "trace");
	if (pSection != NULL)
	{
		FTM_TRACE_TYPE_CONFIG_load(&pConfig->xTrace, pSection);	
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

	memcpy(&xConfig, pConfig, sizeof(FTM_TRACE_CONFIG));

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
		  FTM_UINT32	ulOutput,
	const char *		pFunctionName,
		  FTM_UINT32	ulLine,
	const FTM_CHAR_PTR	pModuleName,
	const FTM_CHAR_PTR 	pTitle,
	const FTM_CHAR_PTR 	pFormat,
	...
)
{

    FILE*		pFile;
    va_list 	xArgs;
	time_t		xCurrentTime;
	FTM_CHAR	pBuffer[2048];	
	FTM_UINT32	ulLen = 0;
	FTM_TRACE_TYPE_CONFIG_PTR	pConfig;

	switch(ulOutput)
	{
	case	0:	pConfig = &xConfig.xLog; break;
	case	1:	pConfig = &xConfig.xTrace; break;
	case	2:	pConfig = &xConfig.xError; break;
	default:	return;
	}

	memset(pBuffer, 0, sizeof(pBuffer));

	xCurrentTime = time(NULL);
	ulLen += strftime(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%Y-%m-%d %H:%M:%S] ", localtime(&xCurrentTime));
	if (ulOutput != 0)
	{
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[");
		snprintf(&pBuffer[ulLen], 25, "%24s", pFunctionName);
		ulLen += 24;
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "][%4u]", ulLine);

		if (pModuleName != NULL)
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[");
			snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "%8s", pModuleName);
			ulLen += 8;
			if(strlen(pModuleName) > 8)
			{
				pBuffer[ulLen] = '\0';
			}
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "]");
		}
		else
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%8s]", "COMMON");
		}

		if (pTitle != NULL)
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[");
			snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "%8s", pTitle);
			ulLen += 8;
			if(strlen(pTitle) > 8)
			{
				pBuffer[ulLen] = '\0';
			}
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "] ");
		}
		else
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%8s] ", "COMMON");
		}
	}

    va_start(xArgs, pFormat);
    ulLen += vsnprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pFormat, xArgs);
    va_end(xArgs);

	pBuffer[ulLen++] = '\n';
	pBuffer[ulLen] = '\0';

   	pFile = fopen(pConfig->pFileName, "a");
    if(!pFile)
	{
        printf("%s", pBuffer);
    }
	else
	{
    	fprintf(pFile, "%s", pBuffer);
    	fclose(pFile);
	}
}



