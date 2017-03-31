#include <sys/types.h>
#include <unistd.h>
#include <common_libssh.h>
#include "ftm_trace.h"
#include "ftm_config.h"
#include "ftm_utils.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_mem.h"

extern	
FTM_CHAR_PTR	program_invocation_short_name;
extern	
FTM_SHELL_CMD	pCatchBShellCmdList[];
extern	
FTM_UINT32	ulCatchBShellCmdCount;

static	FTM_BOOL	bDebugMode = FTM_TRUE;
static	FTM_CHAR	pConfigFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN] = "";

FTM_RET	FTM_showUsage();

FTM_RET	FTM_setOptions
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	ppArgv[]
);

FTM_INT	main
(
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	ppArgv[]
)
{
	FTM_RET			xRet;
	//FTM_BOOL		bDuplicated = FTM_FALSE;
	FTM_CONFIG_PTR	pConfig = NULL;
	FTM_CATCHB_PTR	pCatchB = NULL;

	LOG("");
	LOG("########################################");
	LOG("%s started.", program_invocation_short_name);
	FTM_MEM_init();

	xRet = FTM_setOptions(nArgc, ppArgv);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Invalid arguemtns!\n");
		FTM_showUsage();
		goto finished;
	}

#if 0
	FTM_areDuplicatesRunning(program_invocation_short_name,  getpid(), &bDuplicated);
	if (bDuplicated)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
        ERROR(xRet ,"%s is already running!!\n", program_invocation_short_name);
		goto finished;
    }

	FTM_createPIDFile(program_invocation_short_name, getpid());
#endif
	xRet = FTM_CONFIG_create(&pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create config!\n");
		goto finished;
	}

	xRet = FTM_CONFIG_load(pConfig, pConfigFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to load config!\n");
		goto finished;
	}

	FTM_CONFIG_show(pConfig, FTM_TRACE_LEVEL_LOG);
	
    if (!bDebugMode)
	{
        daemon(0, 0);
	}

	FTM_TRACE_setConfig(&pConfig->xTrace);

	xRet = FTM_CATCHB_create(&pCatchB);
	if(xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create %s!\n", program_invocation_short_name);
		goto finished;
	}

	xRet = FTM_CATCHB_setConfig(pCatchB, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to set %s configuration.", program_invocation_short_name);
		goto finished;
	}

	xRet = FTM_CATCHB_start(pCatchB);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to start catchb!\n");
		goto finished;	
	}

	if (!bDebugMode)
	{
    	while(FTM_TRUE)
    	{
			sleep(1);    
    	}
	}
	else
	{
		FTM_SHELL_run2("catchb", pCatchBShellCmdList, ulCatchBShellCmdCount, pCatchB);
	}

finished:
	if (pCatchB != NULL)
	{
		xRet = FTM_CATCHB_destroy(&pCatchB);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to destroy catchb!\n");	
		}
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

		case 'd':
			bDebugMode = FTM_TRUE;
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
