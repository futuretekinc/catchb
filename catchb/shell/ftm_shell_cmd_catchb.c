#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"


FTM_RET	FTM_SHELL_CMD_catchb
(
	FTM_SHELL_PTR pShell, 
	FTM_INT nArgc, 
	FTM_CHAR_PTR pArgv[], 
	FTM_VOID_PTR pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);
	ASSERT(pData != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	FTM_CATCHB_PTR	pCatchB = (FTM_CATCHB_PTR)pData;

	if (nArgc < 2)
	{
	}
	else if (strcasecmp(pArgv[1], "deny") == 0)
	{
		if (nArgc < 4)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;	
		}


		xRet = FTM_CATCHB_setCCTVStat(pCatchB, pArgv[3], FTM_CCTV_STAT_ABNORMAL, 0, NULL);
		if (xRet != FTM_RET_OK)
		{
			printf("Error : Failed to set access control!\n");	
		}
	}
	else if (strcasecmp(pArgv[1], "allow") == 0)
	{
		if (nArgc < 4)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;	
		}

		xRet = FTM_CATCHB_setCCTVStat(pCatchB, pArgv[3], FTM_CCTV_STAT_NORMAL, 0, NULL);
		if (xRet != FTM_RET_OK)
		{
			printf("Error : Failed to set access control!\n");	
		}
	}
	else
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
	}

finished:
	return	xRet;
}

