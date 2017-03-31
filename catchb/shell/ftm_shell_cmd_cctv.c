#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_client.h"
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

	if (nArgc == 1)
	{
		FTM_CCTV	pCCTVList[10];
		FTM_UINT32	i, ulCount = 0;

		printf("%16s %16s %16s %16s %16s %s\n", "ID", "IP", "STATE", "SWITCH ID", "Registered Time", "Hash");

		xRet = FTM_CATCHB_getCCTVList(pCatchB, 10, pCCTVList, &ulCount);
		for(i = 0 ; i < ulCount ; i++)
		{
			printf("%16s %16s %16s %16s %16s %s\n", pCCTVList[i].xConfig.pID, pCCTVList[i].xConfig.pIP, FTM_CCTV_STAT_print(pCCTVList[i].xConfig.xStat), pCCTVList[i].xConfig.pSwitchID, FTM_TIME_printf2(pCCTVList[i].xConfig.ulTime, NULL), pCCTVList[i].xConfig.pHash);
		}
	}
	else if(strcasecmp(pArgv[1], "add") == 0)
	{
		FTM_INT			i;
		FTM_CCTV_CONFIG	xConfig;

		memset(&xConfig, 0, sizeof(xConfig));
		if ((nArgc < 3) || (nArgc % 2)	 != 1)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;
		}

		strcpy(xConfig.pID, pArgv[2]);

		for(i = 3 ; i < nArgc ; i+=2)
		{
			if(strcasecmp(pArgv[i], "--ip") == 0)
			{
				if (strlen(pArgv[i+1]) > FTM_NAME_LEN)
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;
					goto finished;
				}

				strcpy(xConfig.pIP, pArgv[i+1]);
			}
			else if(strcasecmp(pArgv[i], "--switchid") == 0)
			{
				if (strlen(pArgv[i+1]) > FTM_ID_LEN)
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;
					goto finished;
				}

				strcpy(xConfig.pSwitchID, pArgv[i+1]);
			}
			else if(strcasecmp(pArgv[i], "--comment") == 0)
			{
				if (strlen(pArgv[i+1]) > FTM_COMMENT_LEN)
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;
					goto finished;
				}

				strcpy(xConfig.pComment, pArgv[i+1]);
			}
			else
			{
				xRet = FTM_RET_INVALID_ARGUMENTS;
				goto finished;
			}
		}


		xRet = FTM_CATCHB_addCCTV(pCatchB, &xConfig);
		if (xRet == FTM_RET_OK)
		{
			printf("CCTV[%s] created!\n", xConfig.pID);	
		}
		else
		{
			printf("Failed to create CCTV[%s]\n", xConfig.pID);	
		}
	}
	else if(strcasecmp(pArgv[1], "del") == 0)
	{
		FTM_INT			i;

		if (nArgc < 3)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;
		}

		for(i = 2 ; i < nArgc ; i++)
		{
			xRet = FTM_CATCHB_delCCTV(pCatchB, pArgv[i]);	
			if (xRet == FTM_RET_OK)
			{
				printf("CCTV[%s] deleted!", pArgv[i]);	
			}
			else
			{
				printf("Failed to delete CCTV[%s]!", pArgv[i]);	
			}
		}
	}
	else if(strcasecmp(pArgv[1], "count") == 0)
	{
		FTM_UINT32	ulCount = 0;

		xRet = FTM_CATCHB_getCCTVCount(pCatchB, &ulCount);	
		if (xRet == FTM_RET_OK)
		{
			printf("Number of CCTV : %d", ulCount);	
		}
		else
		{
			printf("Failed to get CCTV count!");	
		}
	}
	else if(strcasecmp(pArgv[1], "set") == 0)
	{
		FTM_INT			i;
		FTM_UINT32		ulFieldFlags = 0;
		FTM_CCTV_CONFIG	xConfig;

		if (nArgc < 5)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;
		}

		memset(&xConfig, 0, sizeof(xConfig));

		for(i = 3 ; i < nArgc ; i+=2)
		{
			if (strncmp(pArgv[i], "--", 2) == 0)
			{
				if(strcasecmp(pArgv[i], "--ip") == 0)
				{
					if (strlen(pArgv[i+1]) > FTM_NAME_LEN)
					{
						xRet = FTM_RET_INVALID_ARGUMENTS;
						goto finished;
					}

					strcpy(xConfig.pIP, pArgv[i+1]);
					ulFieldFlags |= FTM_CCTV_FIELD_IP;
				}
				else if(strcasecmp(pArgv[i], "--switchid") == 0)
				{
					if (strlen(pArgv[i+1]) > FTM_ID_LEN)
					{
						xRet = FTM_RET_INVALID_ARGUMENTS;
						goto finished;
					}

					strcpy(xConfig.pSwitchID, pArgv[i+1]);
					ulFieldFlags |= FTM_CCTV_FIELD_SWITCH_ID;
				}
				else if(strcasecmp(pArgv[i], "--comment") == 0)
				{
					if (strlen(pArgv[i+1]) > FTM_COMMENT_LEN)
					{
						xRet = FTM_RET_INVALID_ARGUMENTS;
						goto finished;
					}

					strcpy(xConfig.pComment, pArgv[i+1]);
					ulFieldFlags |= FTM_CCTV_FIELD_COMMENT;
				}
				else
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;
					goto finished;
				}
			}
		}

		xRet = FTM_CATCHB_setCCTVProperties(pCatchB, pArgv[2], &xConfig, ulFieldFlags);
		if (xRet == FTM_RET_OK)
		{
			printf("CCTV[%s] created!\n", pArgv[2]);	
		}
		else
		{
			printf("Failed to create CCTV[%s]\n", pArgv[2]);	
		}
	}
	else
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
	}

finished:
	return	xRet;
}

