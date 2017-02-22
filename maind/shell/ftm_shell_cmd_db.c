#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"


FTM_RET	FTM_SHELL_CMD_db
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
		goto finished;
	}

	if (strcasecmp(pArgv[1], "cctv") == 0)
	{
		if (nArgc < 3)
		{
			FTM_UINT32	ulCount = 0;

			xRet = FTM_DB_getCCTVCount(pCatchB->pDB, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to get CCTV count from DB!\n");
				goto finished;	
			}
			
			if (ulCount == 0)
			{
				printf("CCTV Count is 0.\n");	
				goto finished;
			}

			FTM_CCTV_CONFIG_PTR	pConfigArray = (FTM_CCTV_CONFIG_PTR)FTM_MEM_calloc(sizeof(FTM_CCTV_CONFIG), ulCount);
			if (pConfigArray == NULL)
			{
				xRet = FTM_RET_NOT_ENOUGH_MEMORY;
				printf("Failed to alloc CCTV array!\n");
				goto finished;
			}

			xRet = FTM_DB_getCCTVList(pCatchB->pDB, pConfigArray, ulCount, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				printf("Failed to get CCTV list!\n");
			}
			else
			{
				FTM_UINT32	i;

				printf("%6s %16s %24s %s\n", "", "ID", "IP", "COMMENT");
				for(i = 0 ; i < ulCount ; i++)
				{
					printf("%4d : %16s %24s %s\n", i + 1, pConfigArray[i].pID, pConfigArray[i].pIP, pConfigArray[i].pComment);
				}
			}

			FTM_MEM_free(pConfigArray);
		}
		else
		{
			if ((strcasecmp(pArgv[2], "add") == 0) && (nArgc == 5))
			{
				xRet = FTM_DB_addCCTV(pCatchB->pDB, pArgv[3], pArgv[4], "", FTM_TIME_printfCurrent(NULL));
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to insert cctv[%s] to DB!\n", pArgv[3]);
				}
			}
			else if ((strcasecmp(pArgv[2], "del") == 0) && (nArgc == 4))
			{
				xRet = FTM_DB_deleteCCTV(pCatchB->pDB, pArgv[3]);
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to delete cctv[%s] from DB!\n", pArgv[3]);	
				}
			}
			else
			{
				xRet = FTM_RET_INVALID_COMMAND;
				goto finished;	
			}
		}
	}
	else if (strcasecmp(pArgv[1], "switch") == 0)
	{
		if (nArgc < 3)
		{
			FTM_UINT32	ulCount = 0;

			xRet = FTM_DB_getSwitchCount(pCatchB->pDB, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				ERROR(xRet, "Failed to get switch count from DB!\n");
				goto finished;	
			}
			
			if (ulCount == 0)
			{
				printf("Switch count is 0.\n");	
				goto finished;
			}

			FTM_SWITCH_CONFIG_PTR	pSwitchArray = (FTM_SWITCH_CONFIG_PTR)FTM_MEM_calloc(sizeof(FTM_SWITCH_CONFIG), ulCount);
			if (pSwitchArray == NULL)
			{
				xRet = FTM_RET_NOT_ENOUGH_MEMORY;
				printf("Failed to alloc Switch array!\n");
				goto finished;
			}

			xRet = FTM_DB_getSwitchList(pCatchB->pDB, pSwitchArray, ulCount, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				printf("Failed to get switch list!\n");
			}
			else
			{
				FTM_UINT32	i;

				printf("%6s %16s %24s %16s %16s %s\n", "", "ID", "IP", "USER", "PASSWD", "COMMENT");
				for(i = 0 ; i < ulCount ; i++)
				{
					printf("%4d : %16s %24s %16s %16s %s\n", i + 1, pSwitchArray[i].pID, pSwitchArray[i].pIP, pSwitchArray[i].pUserID, pSwitchArray[i].pPasswd, pSwitchArray[i].pComment);
				}
			}

			FTM_MEM_free(pSwitchArray);
		}
		else
		{
			if ((strcasecmp(pArgv[2], "add") == 0) && (nArgc == 8))
			{
				FTM_SWITCH_MODEL	xModel;

				if (strcasecmp(pArgv[4], "nst") == 0)
				{
					xModel = FTM_SWITCH_MODEL_NST;
				}
				else if (strcasecmp(pArgv[4], "dasan") == 0)
				{
					xModel = FTM_SWITCH_MODEL_DASAN;
				}
				else if (strcasecmp(pArgv[4], "juniper") == 0)
				{
					xModel = FTM_SWITCH_MODEL_JUNIPER;
				}
				else
				{
					printf("Unknown switch type[%s]\n", pArgv[4]);
					goto finished;	
				}

				xRet = FTM_DB_addSwitch(pCatchB->pDB, pArgv[3], xModel, pArgv[5], pArgv[6], pArgv[7], "");
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to insert cctv[%s] to DB!\n", pArgv[3]);
				}
			}
			else if ((strcasecmp(pArgv[2], "del") == 0) && (nArgc == 4))
			{
				xRet = FTM_DB_deleteSwitch(pCatchB->pDB, pArgv[3]);
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to delete switch[%s] from DB!\n", pArgv[3]);	
				}
			}
			else
			{
				xRet = FTM_RET_INVALID_COMMAND;
				goto finished;	
			}
		}
	}

finished:

	return	xRet;
}

