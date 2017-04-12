#include <sys/types.h>
#include <unistd.h>
#include <common_libssh.h>
#include "ftm_trace.h"
#include "ftm_config.h"
#include "ftm_client.h"
#include "ftm_utils.h"
#include "ftm_shell.h"
#include "ftm_mem.h"

extern	
FTM_CHAR_PTR	program_invocation_short_name;
extern	
FTM_SHELL_CMD	pCatchBShellCmdList[];
extern	
FTM_UINT32	ulCatchBShellCmdCount;


FTM_RET	FTM_showUsage();

static
FTM_RET	FTM_loadConfig
(
	char* 			pFileName
);

FTM_RET	FTM_setOptions
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	ppArgv[]
);

static	FTM_CHAR			pConfigFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN] = "";
static 	FTM_TRACE_CONFIG	xTraceConfig;
static 	FTM_CLIENT_CONFIG	xClientConfig;

FTM_INT	main
(
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	ppArgv[]
)
{
	FTM_RET			xRet;
	FTM_CLIENT_PTR	pClient;
	FTM_CONFIG_PTR	pConfig = NULL;

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	FTM_MEM_init();

	FTM_CLIENT_CONFIG_setDefault(&xClientConfig);

	xRet = FTM_setOptions(nArgc, ppArgv);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Invalid arguemtns!\n");
		FTM_showUsage();
		goto finished;
	}

	xRet = FTM_loadConfig(pConfigFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to load config!\n");
		goto finished;
	}

	FTM_TRACE_setConfig(&xTraceConfig);

	xRet = FTM_CLIENT_create(&xClientConfig, &pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to client!");
		goto finished;
	}

	if (xClientConfig.bAutoConnect)
	{
		FTM_CLIENT_connect(pClient);
	}

	FTM_SHELL_run2("catchb", pCatchBShellCmdList, ulCatchBShellCmdCount, pClient);

finished:

	if (pClient != NULL)
	{
		FTM_CLIENT_destroy(&pClient);
	}

	if (pConfig != NULL)
	{
		FTM_CONFIG_destroy(&pConfig);
	}
	
	FTM_MEM_final();

    return 0;
}

FTM_RET	FTM_setOptions
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	ppArgv[]
)
{
	FTM_CHAR	xOption;

    while ((xOption = getopt(nArgc, ppArgv, "c:d")) != -1)
    {
        switch (xOption)
        {
		case 'c':
			strncpy(pConfigFileName, optarg, sizeof(pConfigFileName) - 1);
			break;

		default:
			return	FTM_RET_INVALID_ARGUMENTS;
        }
    }

	if (strlen(pConfigFileName) == 0)
	{
		snprintf(pConfigFileName, sizeof(pConfigFileName) - 1, "/etc/%s.conf", program_invocation_short_name);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_showUsage()
{
	return	FTM_RET_OK;
	
}

FTM_RET	FTM_loadConfig
(
	char* 			pFileName
)
{
	ASSERT(pFileName != NULL);

	FILE *pFile; 
	FTM_RET			xRet = FTM_RET_OK;
	FTM_CHAR_PTR	pData = NULL;
	FTM_UINT32		ulFileLen;
	FTM_UINT32		ulReadSize;
	cJSON _PTR_		pRoot = NULL;
	cJSON _PTR_		pSection;

	FTM_TRACE_CONFIG_setDefault(&xTraceConfig);

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

	pRoot = cJSON_Parse(pData);
	if (pRoot == NULL)
	{    
		xRet = FTM_RET_INVALID_JSON_FORMAT;
		ERROR(xRet, "Invalid json format!\n");
		goto finished;
	}

	pSection = cJSON_GetObjectItem(pRoot, "client");
	if (pSection != NULL)
	{
		FTM_CLIENT_CONFIG_load(&xClientConfig, pSection);
	}

	pSection = cJSON_GetObjectItem(pRoot, "trace");
	if (pSection != NULL)
	{
		FTM_TRACE_CONFIG_load(&xTraceConfig, pSection);
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

