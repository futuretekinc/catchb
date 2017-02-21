#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"


FTM_RET	FTM_SHELL_CMD_analyzer
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
		FTM_UINT32	ulCount;

		xRet = FTM_ANALYZER_CCTV_count(pCatchB->pAnalyzer, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			printf("Failed to get CCTV count in analyzer!");	
		}
		else if (ulCount != 0)
		{
			FTM_ID_PTR	pIDArray = (FTM_ID_PTR)FTM_MEM_calloc(sizeof(FTM_ID), ulCount);
			if (pIDArray == NULL)
			{
				printf("Failed to alloc memory for ID array!");	
			}
			else
			{
				xRet = FTM_ANALYZER_CCTV_getList(pCatchB->pAnalyzer, pIDArray, ulCount, &ulCount);
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to get CCTV array!");	
				}
				else
				{
					FTM_CCTV_PTR	pCCTV;
					FTM_UINT32		i;


					printf("%4s   %16s %24s %s\n", "", "ID", "IP", "COMMENT");
					for(i = 0; i < ulCount ; i++)
					{
						FTM_LIST_get(pCatchB->pCCTVList, pIDArray[i], (FTM_VOID_PTR _PTR_)&pCCTV);
						printf("%4d : %16s %24s %s\n", i+1, pCCTV->xConfig.pID, pCCTV->xConfig.pIP, pCCTV->xConfig.pComment);	
					
					}
				}
			}
		
		}
	
	}

finished:

	return	xRet;
}

