#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_shell.h"
#include "ftm_mem.h"
#include "ftm_client.h"

FTM_RET	FTM_SHELL_CMD_connect
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

	FTM_CLIENT_PTR	pClient = (FTM_CLIENT_PTR)pData;
	switch(nArgc)
	{
	case	1:
		{
			xRet = FTM_CLIENT_connect(pClient);
			if (xRet != FTM_RET_OK)
			{
				printf("Failed to connect to %s\n", pArgv[1]);	
			}
			else
			{
				printf("connected to %s\n", pArgv[1]);	
			}
		}
		break;

	case	3:
		{
			FTM_CLIENT_CONFIG	xConfig;

			strncpy(xConfig.pIP, pArgv[1], sizeof(xConfig.pIP));
			xConfig.usPort = strtoul(pArgv[2], 0, 10);

			xRet = FTM_CLIENT_setConfig(pClient, &xConfig, FTM_CLIENT_FIELD_IP | FTM_CLIENT_FIELD_PORT);
			if (xRet != FTM_RET_OK)
			{
				printf("Failed to set server information.\n");	
			}

			xRet = FTM_CLIENT_connect(pClient);
			if (xRet != FTM_RET_OK)
			{
				printf("Failed to connect to %s\n", pArgv[1]);	
			}
			else
			{
				printf("connected to %s\n", pArgv[1]);	
			}
		}
		break;

	default:
		xRet= FTM_RET_INVALID_ARGUMENTS;
	}

	return	xRet;
}


