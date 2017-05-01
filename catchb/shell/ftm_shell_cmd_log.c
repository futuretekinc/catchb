#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"

FTM_RET	FTM_SHELL_CMD_showLogListFrom
(
	FTM_CATCHB_PTR	pCabchB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_SHELL_CMD_showLogList
(
	FTM_CATCHB_PTR	pCabchB,
	FTM_LOG_TYPE	xType,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIp,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
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

	FTM_CATCHB_PTR	pCatchB = (FTM_CATCHB_PTR)pData;

	switch(nArgc)
	{
	case	1:
		{
			FTM_UINT32	ulCount = 0;

			FTM_DB_getLogCount(pCatchB->pDB, &ulCount);

			FTM_LOGGER_CONFIG_show(&pCatchB->pLogger->xConfig, FTM_TRACE_LEVEL_CONSOLE);

			printf("\n[ Logger Statistics ]\n");
			printf("%16s : %u\n", "Count" , ulCount);
		}
		break;

	case	2:
		{
			if (strcasecmp(pArgv[1], "show") == 0)
			{
				FTM_SHELL_CMD_showLogListFrom(pCatchB, 0, 100);
			}
		}
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "show") == 0)
			{
				FTM_UINT32	ulCount;
				
				ulCount = strtoul(pArgv[2], 0, 10);

				FTM_SHELL_CMD_showLogListFrom(pCatchB, 0, ulCount);
			}
		}
		break;

	case	4:
		{
			if (strcasecmp(pArgv[1], "show") == 0)
			{
				FTM_UINT32	ulIndex, ulCount;
				
				ulIndex = strtoul(pArgv[2], 0, 10);
				ulCount = strtoul(pArgv[3], 0, 10);

				FTM_SHELL_CMD_showLogListFrom(pCatchB, ulIndex, ulCount);
			}
			else if (strcasecmp(pArgv[1], "id") == 0)
			{
				FTM_UINT32	ulCount;

				ulCount = strtoul(pArgv[3], 0, 10);

				FTM_SHELL_CMD_showLogList(pCatchB, 0, pArgv[2], NULL, 0, 0, 0, ulCount);	
			}
			else if (strcasecmp(pArgv[1], "ip") == 0)
			{
				FTM_UINT32	ulCount;

				ulCount = strtoul(pArgv[3], 0, 10);

				FTM_SHELL_CMD_showLogList(pCatchB, 0, NULL, pArgv[2], 0, 0, 0, ulCount);	
			}
			else if (strcasecmp(pArgv[1], "stat") == 0)
			{
				FTM_CCTV_STAT xStat;
				FTM_UINT32		ulCount;

				if (strcasecmp(pArgv[2], "normal") == 0)
				{
					xStat = FTM_CCTV_STAT_NORMAL;	
				}
				else if (strcasecmp(pArgv[2], "abnormal") == 0)
				{
					xStat = FTM_CCTV_STAT_ABNORMAL;	
				}
				else if (strcasecmp(pArgv[2], "unused") == 0)
				{
					xStat = FTM_CCTV_STAT_UNUSED;	
				}
				else
				{
					printf("Error : Invalid CCTV stat[%s]\n", pArgv[3]);
				}

				ulCount = strtoul(pArgv[3], 0, 10);

				FTM_SHELL_CMD_showLogList(pCatchB, 0, NULL, NULL, xStat, 0, 0, ulCount);	
			}

		}
		break;

	case	5:
		{	
			if (strcasecmp(pArgv[1], "del") == 0)
			{
				if (strcasecmp(pArgv[2], "id") == 0)
				{
					FTM_UINT32	ulCount;

					ulCount = strtoul(pArgv[4], 0, 10);

					FTM_DB_deleteLog(pCatchB->pDB, pArgv[3], NULL, 0, 0, 0, ulCount);
				}
				else if (strcasecmp(pArgv[2], "ip") == 0)
				{
					FTM_UINT32	ulCount;

					ulCount = strtoul(pArgv[4], 0, 10);

					FTM_DB_deleteLog(pCatchB->pDB, NULL, pArgv[3], 0, 0, 0, ulCount);
				}
				else if (strcasecmp(pArgv[2], "stat") == 0)
				{
					FTM_UINT32	ulCount;
					FTM_CCTV_STAT	xStat = FTM_CCTV_STAT_UNREGISTERED;

					if (strcasecmp(pArgv[3], "normal") == 0)
					{
						xStat = FTM_CCTV_STAT_NORMAL;	
					}
					else if (strcasecmp(pArgv[3], "abnormal") == 0)
					{
						xStat = FTM_CCTV_STAT_ABNORMAL;	
					}
					else if (strcasecmp(pArgv[3], "unused") == 0)
					{
						xStat = FTM_CCTV_STAT_UNUSED;	
					}
					else
					{
						printf("Error : Invalid CCTV stat[%s]\n", pArgv[3]);
					}

					ulCount = strtoul(pArgv[4], 0, 10);

					FTM_DB_deleteLog(pCatchB->pDB, NULL, NULL, xStat, 0, 0, ulCount);
				}
				else if (strcasecmp(pArgv[2], "range") == 0)
				{
					FTM_TIME	xStartTime, xEndTime;
					FTM_UINT32	ulStartTime, ulEndTime;


					xRet = FTM_TIME_setString(&xStartTime, pArgv[3]);
					if (xRet != FTM_RET_OK)
					{
						printf("Invalid time : %s\n", pArgv[3]);
						break;
					}

					xRet = FTM_TIME_setString(&xEndTime, pArgv[4]);
					if (xRet != FTM_RET_OK)
					{
						printf("Invalid time : %s\n", pArgv[4]);
						break;
					}

					FTM_TIME_toSecs(&xStartTime, &ulStartTime);
					FTM_TIME_toSecs(&xEndTime, &ulEndTime);

					FTM_DB_deleteLog(pCatchB->pDB, NULL, NULL, 0, ulStartTime, ulEndTime, 0);
				}
			}
			else if (strcasecmp(pArgv[1], "range") == 0)
			{
				FTM_TIME	xStartTime, xEndTime;
				FTM_UINT32	ulStartTime, ulEndTime, ulCount;


				xRet = FTM_TIME_setString(&xStartTime, pArgv[2]);
				if (xRet != FTM_RET_OK)
				{
					printf("Invalid time : %s\n", pArgv[2]);
					break;
				}

				xRet = FTM_TIME_setString(&xEndTime, pArgv[3]);
				if (xRet != FTM_RET_OK)
				{
					printf("Invalid time : %s\n", pArgv[3]);
					break;
				}

				FTM_TIME_toSecs(&xStartTime, &ulStartTime);
				FTM_TIME_toSecs(&xEndTime, &ulEndTime);
				ulCount = strtoul(pArgv[4], 0, 10);

				FTM_SHELL_CMD_showLogList(pCatchB, 0, NULL, NULL, 0, ulStartTime, ulEndTime, ulCount);
			}
		}
		break;

	}


	return	xRet;
}

FTM_RET	FTM_SHELL_CMD_showLogListFrom
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
)
{
	FTM_RET	xRet;

	FTM_LOG_PTR	pLogs;

	pLogs = (FTM_LOG_PTR)FTM_MEM_calloc(sizeof(FTM_LOG), ulCount);
	if (pLogs == NULL)
	{
		printf("Error : Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_DB_getLogListFrom(pCatchB->pDB, ulIndex, ulCount, pLogs, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_UINT32	i;

		printf("%6s %8s %24s %8s %16s %8s %s\n", "", "Type", "Time", "ID", "IP", "Stat", "Hash");
		for(i = 0 ; i < ulCount ; i++)
		{
			printf("%4d : %8s %24s %8s %16s %8s %s\n", i+1, FTM_LOG_TYPE_print(pLogs[i].xType), FTM_TIME_printf2(pLogs[i].ulTime, NULL), pLogs[i].pID, pLogs[i].pIP, FTM_CCTV_STAT_print(pLogs[i].xStat), pLogs[i].pHash);
		}
	}

	FTM_MEM_free(pLogs);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_CMD_showLogList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_LOG_TYPE	xType,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulCount
)
{
	FTM_RET	xRet;

	FTM_LOG_PTR	pLogs;
	
	pLogs = (FTM_LOG_PTR)FTM_MEM_calloc(sizeof(FTM_LOG), ulCount);
	if (pLogs == NULL)
	{
		printf("Error : Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTM_DB_getLogList(pCatchB->pDB, xType, pID, pIP, xStat, ulStartTime, ulEndTime, 0,ulCount, pLogs, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_UINT32	i;

		printf("%6s %8s %24s %8s %16s %8s %s\n", "", "Type", "Time", "ID", "IP", "Stat", "Hash");
		for(i = 0 ; i < ulCount ; i++)
		{
			printf("%4d : %8s %24s %8s %16s %8s %s\n"	, i+1, FTM_LOG_TYPE_print(pLogs[i].xType), FTM_TIME_printf2(pLogs[i].ulTime, NULL), pLogs[i].pID, pLogs[i].pIP, FTM_CCTV_STAT_print(pLogs[i].xStat), pLogs[i].pHash);
		}
	}

	FTM_MEM_free(pLogs);

	return	FTM_RET_OK;
}
