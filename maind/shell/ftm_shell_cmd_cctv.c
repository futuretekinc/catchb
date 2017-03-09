#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"

FTM_RET	FTM_SHELL_CMD_showCCTVList
(
	FTM_CATCHB_PTR pCatchB
);

FTM_RET	FTM_SHELL_CMD_cctv
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

	printf("nArgc : %d\n", nArgc);
	switch(nArgc)
	{
	case	2:
		{
			if (strcasecmp(pArgv[1], "list") == 0)
			{
				xRet = FTM_SHELL_CMD_showCCTVList(pCatchB);
			}
			else
			{
				FTM_CCTV_PTR	pCCTV;
				FTM_UINT32		i;
				FTM_UINT32		ulPortCount;
				FTM_UINT16_PTR	pPortList = NULL;

				xRet = FTM_CATCHB_getCCTV(pCatchB, pArgv[1], &pCCTV);	
				if (xRet != FTM_RET_OK)
				{
					printf("Invalid switch ID[%s]", pArgv[1]);	
					break;
				}

				xRet = FTM_ANALYZER_getPortCount(pCatchB->pAnalyzer, &ulPortCount);
				if (ulPortCount != 0)
				{
					pPortList = (FTM_UINT16_PTR)FTM_MEM_calloc(sizeof(FTM_UINT16), ulPortCount);	
					if (pPortList == NULL)
					{
						xRet = FTM_RET_NOT_ENOUGH_MEMORY;
						ERROR(xRet, "Failed to create port buffer!");
						break;
					}

					xRet = FTM_ANALYZER_getPortList(pCatchB->pAnalyzer, pPortList, ulPortCount, &ulPortCount);
					if (xRet != FTM_RET_OK)
					{
						ERROR(xRet, "Failed to get port list!");
						FTM_MEM_free(pPortList);
						break;
					}
				}

				printf("%16s : %s\n", "ID", 	pCCTV->xConfig.pID);
				printf("%16s : %s\n", "IP", 	pCCTV->xConfig.pIP);
				printf("%16s : %s\n", "Stat", 	FTM_CCTV_STAT_print(pCCTV->xConfig.xStat));
				printf("%16s : %s\n", "Switch ID", pCCTV->xConfig.pSwitchID);
				printf("%16s : %s\n", "Time",   FTM_TIME_printf2(pCCTV->xConfig.ulTime, NULL));
				printf("%16s : %s\n", "Hash",   pCCTV->xConfig.pHash);
				for(i = 0 ; i <ulPortCount ; i++)
				{
					printf("%16d : %s\n", pPortList[i], pCCTV->pPortStat[i]?"open":"closed");
				}

				if (pPortList != NULL)
				{
					FTM_MEM_free(pPortList);
				}
			}
		}
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "del") == 0)
			{
				xRet = FTM_CATCHB_destroyCCTV(pCatchB, pArgv[2]);
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to destroy switch[%s]!", pArgv[2]);	
				}
			}
		}
		break;

	case	5:
		{
			if (strcasecmp(pArgv[1], "add") == 0)
			{
				FTM_CCTV_CONFIG	xConfig;

				memset(&xConfig, 0, sizeof(xConfig));

				strncpy(xConfig.pID, pArgv[2], sizeof(xConfig.pID) - 1);
				strncpy(xConfig.pIP, pArgv[3], sizeof(xConfig.pIP) - 1);
				strncpy(xConfig.pSwitchID, pArgv[4], sizeof(xConfig.pSwitchID) - 1);
		
				xRet = FTM_CATCHB_createCCTV(pCatchB, &xConfig);
				if (xRet != FTM_RET_OK)
				{
					printf("CCTV create failed !");	
				}
			}
		
		}
		break;

	case	6:
		{
			if (strcasecmp(pArgv[1], "add") == 0)
			{
				FTM_CCTV_CONFIG	xConfig;

				memset(&xConfig, 0, sizeof(xConfig));

				strncpy(xConfig.pID, pArgv[2], sizeof(xConfig.pID) - 1);
				strncpy(xConfig.pIP, pArgv[3], sizeof(xConfig.pIP) - 1);
				strncpy(xConfig.pSwitchID, pArgv[4], sizeof(xConfig.pSwitchID) - 1);
				strncpy(xConfig.pComment, pArgv[5], sizeof(xConfig.pComment) - 1);
		
				xRet = FTM_CATCHB_createCCTV(pCatchB, &xConfig);
				if (xRet != FTM_RET_OK)
				{
					printf("CCTV create failed !");	
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

FTM_RET	FTM_SHELL_CMD_showCCTVList
(
	FTM_CATCHB_PTR pCatchB
)
{
	FTM_RET	xRet = FTM_RET_OK;

	FTM_UINT32	i, j, ulCount = 0;
	FTM_CCTV_PTR	pCCTVes = NULL;
	FTM_UINT16_PTR	pPortList = NULL;
	FTM_UINT32		ulPortCount = 0;

	xRet = FTM_CATCHB_getCCTVCount(pCatchB, &ulCount);
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

	pCCTVes = (FTM_CCTV_PTR)FTM_MEM_calloc(sizeof(FTM_CCTV), ulCount);
	if (pCCTVes == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create switch buffer!");
		goto finished;
	}

	xRet = FTM_CATCHB_getCCTVList(pCatchB, pCCTVes, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get switches!");
		goto finished;
	}

	xRet = FTM_ANALYZER_getPortCount(pCatchB->pAnalyzer, &ulPortCount);
	if (ulPortCount != 0)
	{
		pPortList = (FTM_UINT16_PTR)FTM_MEM_calloc(sizeof(FTM_UINT16), ulPortCount);	
		if (pPortList == NULL)
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
			ERROR(xRet, "Failed to create port buffer!");
			goto finished;
		}

		xRet = FTM_ANALYZER_getPortList(pCatchB->pAnalyzer, pPortList, ulPortCount, &ulPortCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to get port list!");
			goto finished;
		}
	}

	printf("%4s   %16s %24s %16s %16s %24s ", "", "ID", "IP", "Stat", "Switch ID", "Time");
	for(i = 0 ; i < ulPortCount ; i++)
	{
		printf(" %8d", pPortList[i]);
	}
	printf(" %s\n", "Hash");

	for(i = 0; i < ulCount ; i++)
	{
		printf("%4d : %16s %24s %16s %16s %24s ", i+1, pCCTVes[i].xConfig.pID, pCCTVes[i].xConfig.pIP, FTM_CCTV_STAT_print(pCCTVes[i].xConfig.xStat), pCCTVes[i].xConfig.pSwitchID, FTM_TIME_printf2(pCCTVes[i].xConfig.ulTime, NULL));
		for(j = 0 ; j < pCCTVes[i].ulPortCount ; j++)
		{
			printf(" %8s", pCCTVes[i].pPortStat[j]?"open":"closed");
		}
		printf(" %s\n", pCCTVes[i].xConfig.pHash);
	}


finished:

	if (pPortList != NULL)
	{
		FTM_MEM_free(pPortList);
	}

	if (pCCTVes != NULL)
	{
		FTM_MEM_free(pCCTVes);
	}

	return	xRet;
}

