#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_client.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"

FTM_RET	FTM_SHELL_CMD_showLogList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIp,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_SHELL_CMD_logger
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

	if (nArgc < 2)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
	}
	else if (strcasecmp(pArgv[1], "show") == 0)
	{
		if (nArgc == 2)
		{
			FTM_SHELL_CMD_showLogList(pClient, NULL, NULL, 0, 0, 0, 0, 100);
		}
		else if (nArgc == 3)
		{
			FTM_UINT32	ulCount;

			ulCount = strtoul(pArgv[2], 0, 10);
			FTM_SHELL_CMD_showLogList(pClient, NULL, NULL, 0, 0, 0, 0, ulCount);
		}
		else if (nArgc % 2 == 0)
		{
			FTM_UINT32	i;
			FTM_UINT32	ulIndex = 0;
			FTM_UINT32	ulCount = 0;

			for(i = 2 ; i < nArgc ; i+=2)
			{
				if (strcasecmp(pArgv[i], "index") == 0)
				{
					ulIndex = strtoul(pArgv[i+1], 0, 10);
				}
				else if (strcasecmp(pArgv[i], "count") == 0)
				{
					ulCount = strtoul(pArgv[i+1], 0, 10);
				}
				else 
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;
					goto finished;
				}
			}

			FTM_SHELL_CMD_showLogList(pClient, NULL, NULL, 0, 0, 0, ulIndex, ulCount);
		}
		else
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;
		}
	}
	else if (strcasecmp(pArgv[1], "info") == 0)
	{
		FTM_UINT32	ulCount = 0;
		FTM_UINT32	ulFirstTime = 0;
		FTM_UINT32	ulLastTime = 0;

		xRet = FTM_CLIENT_getLogInfo(pClient, &ulCount, &ulFirstTime, &ulLastTime);
		if (xRet != FTM_RET_OK)
		{
			printf("Error : Failed to get log info!\n");	
		}
		else
		{
			printf("%16s : %d\n", "Count", ulCount);
			printf("%16s : %s\n", "First Time", FTM_TIME_printf2(ulFirstTime, NULL));
			printf("%16s : %s\n", "Last Time", FTM_TIME_printf2(ulLastTime, NULL));
		}
	}
	else if (strcasecmp(pArgv[1], "count") == 0)
	{
		FTM_UINT32	ulCount = 0;
		xRet = FTM_CLIENT_getLogCount(pClient, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			printf("Error : Failed to get log count!\n");	
		}
		else
		{
			printf("Log Count : %d\n", ulCount);
		}
	}
	else if (strcasecmp(pArgv[1], "del") == 0)
	{
		FTM_UINT32	ulIndex = 0;
		FTM_UINT32	ulCount = 0;
		FTM_UINT32	ulRemainCount = 0;
		FTM_UINT32	ulFirstTime = 0;
		FTM_UINT32	ulLastTime = 0;

		if (nArgc < 4)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;
		}
		else
		{
			FTM_UINT32	i;

			for(i = 2 ; i < nArgc ; i+=2)
			{
				if (strcasecmp(pArgv[i], "--index") == 0)
				{
					ulIndex = strtoul(pArgv[i+1], 0, 10);	
				}
				else if (strcasecmp(pArgv[i], "--count") == 0)
				{
					ulCount = strtoul(pArgv[i+1], 0, 10);	
				}
				else
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;	
					goto finished;
				}
			}

			xRet = FTM_CLIENT_delLog(pClient, ulIndex, ulCount, &ulRemainCount, &ulFirstTime, &ulLastTime);
			if (xRet != FTM_RET_OK)
			{
				printf("Error : Failed to delete log list!");	
			}
			else
			{
				printf("The log has been deleted!");	
				printf("%16s : %d\n", "Count", ulCount);
				printf("%16s : %s\n", "First Time", FTM_TIME_printf2(ulFirstTime, NULL));
				printf("%16s : %s\n", "Last Time", FTM_TIME_printf2(ulLastTime, NULL));
			}
		}
	}

finished:

	return	xRet;
}

FTM_RET	FTM_SHELL_CMD_showLogList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIp,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLogList = NULL;
	FTM_UINT32	ulCount = 0;

	pLogList = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG) * ulMaxCount);
	if (pLogList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;

		printf("Error : Failed to alloc memory[%d]\n", sizeof(FTM_LOG) * ulMaxCount);
		return	xRet;	
	}

	xRet = FTM_CLIENT_getLogList(pClient, 0, NULL, NULL, FTM_CCTV_STAT_UNREGISTERED, 0, 0, ulIndex, ulMaxCount, pLogList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_UINT32	i;

		printf("       %16s %16s %16s %s\n", "TIME", "CCTV ID", "IP", "HASH");
		for(i = 0 ; i < ulCount ; i++)
		{
			printf("%4d : %16s %16s %16s %s\n", i+1, FTM_TIME_printf2(pLogList[i].ulTime, NULL), pLogList[i].pID, pLogList[i].pIP, pLogList[i].pHash);	
		}
	}

	return	xRet;
}

