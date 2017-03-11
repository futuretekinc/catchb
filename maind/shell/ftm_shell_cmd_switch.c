#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"

FTM_RET	FTM_SHELL_CMD_showSwitchList
(
	FTM_CATCHB_PTR pCatchB
);

FTM_RET	FTM_SHELL_CMD_showSwitchACList
(	
	FTM_SWITCH_PTR	pSwitch
);

FTM_RET	FTM_SHELL_CMD_switch
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
	case	1:
		{
			xRet = FTM_SHELL_CMD_showSwitchList(pCatchB);

		}
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "del") == 0)
			{
				xRet = FTM_CATCHB_destroySwitch(pCatchB, pArgv[2]);
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to destroy switch[%s]!", pArgv[2]);	
				}
			}
			else
			{
				FTM_SWITCH_PTR	pSwitch;
				
				xRet = FTM_CATCHB_getSwitch(pCatchB, pArgv[1], &pSwitch);	
				if (xRet != FTM_RET_OK)
				{
					printf("Invalid switch ID[%s]", pArgv[1]);	
					break;
				}

				if (strcasecmp(pArgv[2], "ac") == 0)
				{
					xRet = FTM_SHELL_CMD_showSwitchACList(pSwitch);
				}
			
			}
		}
		break;

	case	5:
		{
			if (strcasecmp(pArgv[1], "ac") == 0)
			{
				FTM_SWITCH_PTR	pSwitch = NULL;
				xRet = FTM_CATCHB_getSwitch(pCatchB, pArgv[2], &pSwitch);
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to get switch[%s]\n", pArgv[2]);	
					break;
				}

				if (strcasecmp(pArgv[4], "allow") == 0)
				{
					FTM_SWITCH_NST_setAC(pSwitch, pArgv[3], FTM_SWITCH_AC_POLICY_ALLOW);
				}
				else if (strcasecmp(pArgv[4], "block") == 0)
				{
					FTM_SWITCH_NST_setAC(pSwitch, pArgv[3], FTM_SWITCH_AC_POLICY_DENY);
				}
			}
		}
		break;

	case	7:
		{
			if (strcasecmp(pArgv[1], "add") == 0)
			{
				FTM_SWITCH_CONFIG	xConfig;

				memset(&xConfig, 0, sizeof(xConfig));

				xConfig.xModel = FTM_getSwitchModelID(pArgv[3]);
				if (xConfig.xModel == FTM_SWITCH_MODEL_UNKNOWN)
				{
					printf("Switch[%s] is not supported!\n", pArgv[3]);	
					break;
				}

				strncpy(xConfig.pID, pArgv[2], sizeof(xConfig.pID) - 1);
				strncpy(xConfig.pIP, pArgv[4], sizeof(xConfig.pIP) - 1);
				strncpy(xConfig.pUserID, pArgv[5], sizeof(xConfig.pUserID) - 1);
				strncpy(xConfig.pPasswd, pArgv[6], sizeof(xConfig.pPasswd) - 1);
		
				xRet = FTM_CATCHB_createSwitch(pCatchB, &xConfig, NULL);
				if (xRet != FTM_RET_OK)
				{
					printf("Switch create failed !");	
				}
			}
		
		}
		break;

	default:
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
		}
		break;
	}

	return	xRet;
}

FTM_RET	FTM_SHELL_CMD_showSwitchList
(
	FTM_CATCHB_PTR pCatchB
)
{
	FTM_RET	xRet = FTM_RET_OK;

	FTM_UINT32	i, ulCount = 0;
	FTM_SWITCH_PTR	_PTR_ ppSwitches = NULL;

	xRet = FTM_CATCHB_getSwitchCount(pCatchB, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		printf("Failed to get switch count!\n");
		goto finished;
	}

	if (ulCount == 0)
	{
		printf("Registered switch does not exist.!\n");
		goto finished;
	}

	ppSwitches = (FTM_SWITCH_PTR _PTR_)FTM_MEM_calloc(sizeof(FTM_SWITCH_PTR), ulCount);
	if (ppSwitches == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create switch buffer!");
		goto finished;
	}

	xRet = FTM_CATCHB_getSwitchList(pCatchB, ppSwitches, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get switches!");
	}
	else
	{
		printf("%4s   %16s %24s %16s %16s\n", "", "ID", "IP", "UserID", "Password");
		for(i = 0; i < ulCount ; i++)
		{
			printf("%4d : %16s %24s %16s %16s\n", i+1, ppSwitches[i]->xConfig.pID, ppSwitches[i]->xConfig.pIP, ppSwitches[i]->xConfig.pUserID, ppSwitches[i]->xConfig.pPasswd);
		}
	}

	FTM_MEM_free(ppSwitches);

finished:
	return	xRet;
}

FTM_RET	FTM_SHELL_CMD_showSwitchACList
(	
	FTM_SWITCH_PTR	pSwitch
)
{
	ASSERT(pSwitch != NULL);
	FTM_RET	xRet;
	FTM_UINT32	ulCount = 0;
	FTM_SWITCH_AC_PTR	pAC;
	FTM_UINT32	i;

	xRet = FTM_LIST_count(pSwitch->pACList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		printf("Failed to get access control list of switch[%s].", pSwitch->xConfig.pID);
		return	xRet;
	}

	printf("%4s   %24s %5s\n", "", "IP", "INDEX");

	i = 0;
	FTM_LIST_iteratorStart(pSwitch->pACList);
	while(FTM_LIST_iteratorNext(pSwitch->pACList, (FTM_VOID_PTR _PTR_)&pAC) == FTM_RET_OK)
	{
		printf("%4d : %24s %5d\n", i+1, pAC->pIP, pAC->nIndex);
		i++;
	}

	return	FTM_RET_OK;
}
