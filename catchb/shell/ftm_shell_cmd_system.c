#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"
#include "ftm_utils.h"
#include "ftm_smtpc.h"

FTM_RET	FTM_SHELL_CMD_net
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

	switch(nArgc)
	{
	case	2:
		{
			if (strcasecmp(pArgv[1], "info") == 0)
			{
				FTM_NET_INFO	xNetInfo;

				if (FTM_getNetInfo(&xNetInfo) != FTM_RET_OK)
				{
					printf("Failed to get net information!");		
				}
				else
				{
					for(FTM_UINT32	i = 0 ; i < xNetInfo.ulIFCount ; i++)
					{
						printf("%16s : %s %s\n", xNetInfo.pIF[i].pName, xNetInfo.pIF[i].pIP, xNetInfo.pIF[i].pNetmask);
					}
					printf("%16s : %s\n", "Gateway", xNetInfo.pGateway);
				}
			}
		}
		break;

	}

	return	xRet;
}


