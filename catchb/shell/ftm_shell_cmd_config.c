#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"

FTM_RET	FTM_SHELL_CMD_config
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

	switch(nArgc)
	{
	case	3:
		{
			if(strcasecmp(pArgv[1], "save") == 0)
			{
				FTM_CONFIG	xConfig;

				xRet = FTM_CATCHB_getConfig(pCatchB, &xConfig);	
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to get config!\n");
				}
				else
				{
					FTM_CONFIG_save(&xConfig, pArgv[2]);
				}
			}
		}
		break;

	}


	return	xRet;
}

