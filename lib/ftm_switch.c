#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_switch.h"
#include "ftm_telnet.h"
#include "ftm_ssh.h"
#include "ftm_timer.h"
#include "ftm_log.h"
#include "ftm_catchb.h"

#undef	__MODULE__ 
#define	__MODULE__ "switch"

FTM_SWITCH_INFO	pSwitchInfos[] = 
{
	{
		.xModel	= FTM_SWITCH_MODEL_NST,
		.pName  = "nst",
		.fSetAC	= FTM_SWITCH_NST_setAC
	},
	{
		.xModel	= FTM_SWITCH_MODEL_DASAN,
		.pName  = "dasan",
		.fSetAC	= FTM_SWITCH_DASAN_setAC
	},
	{
		.xModel	= FTM_SWITCH_MODEL_JUNIPER,
		.pName  = "juniper",
		.fSetAC	= FTM_SWITCH_JUNIPER_setAC
	},
	{
		.xModel	= FTM_SWITCH_MODEL_UNKNOWN,
		.pName  = "unknown"
	}
};

FTM_BOOL	FTM_SWITCH_AC_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);
//////////////////////////////////////////////////////////////
//	FTM_SWITCH_CONFIG functions
//////////////////////////////////////////////////////////////
FTM_RET	FTM_SWITCH_CONFIG_create
(
	FTM_SWITCH_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_SWITCH_CONFIG_PTR	pConfig;

	pConfig = (FTM_SWITCH_CONFIG_PTR)FTM_MEM_malloc(sizeof(FTM_SWITCH_CONFIG));
	if (pConfig == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create switch config!\n");
		return	xRet;
	}

	memset(pConfig, 0, sizeof(FTM_SWITCH_CONFIG));
	*ppConfig = pConfig;

	return	xRet;
}

FTM_RET	FTM_SWITCH_CONFIG_destroy
(
	FTM_SWITCH_CONFIG_PTR _PTR_ ppConfig
)
{
	ASSERT(ppConfig != NULL);
	ASSERT(*ppConfig != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	FTM_MEM_free(*ppConfig);

	*ppConfig = NULL;

	return	xRet;
}

FTM_RET	FTM_SWITCH_CONFIG_loadList
(	
	FTM_LIST_PTR	pList,
	cJSON _PTR_		pRoot
)
{
	ASSERT(pList != NULL);
	ASSERT(pRoot != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	i, ulCount = 0;

	if(pRoot->type != cJSON_Array)
	{
		xRet = FTM_RET_CONFIG_INVALID_OBJECT;	
		ERROR(xRet, "Invalid json object!\n");
		return	xRet;
	}

	ulCount = cJSON_GetArraySize(pRoot);
	for(i = 0 ; i < ulCount ; i++)
	{
		cJSON _PTR_ pObject;

		pObject = cJSON_GetArrayItem(pRoot, i);
		if (pObject != NULL)
		{
			FTM_SWITCH_CONFIG_PTR	pConfig;

			pConfig = FTM_MEM_malloc(sizeof(FTM_SWITCH_CONFIG));
			if (pConfig != NULL)
			{
				xRet = FTM_SWITCH_CONFIG_load(pConfig, pObject);
				if (xRet != FTM_RET_OK)
				{
					ERROR(xRet, "Failed to load switch!");
					FTM_MEM_free(pConfig);
				}
				else
				{
					xRet = FTM_LIST_append(pList, pConfig);	
					if (xRet != FTM_RET_OK)
					{
						ERROR(xRet, "Failed to append switch config!");
						FTM_MEM_free(pConfig);
					}
				}
			}
		}
	}

	return	xRet;
}

FTM_RET	FTM_SWITCH_CONFIG_showList
(
	FTM_LIST_PTR	pList,
	FTM_TRACE_LEVEL	xLevel
)
{
	ASSERT(pList != NULL);

	FTM_RET		xRet;
	FTM_UINT32	i, ulCount;

	OUTPUT(xLevel, "");
	OUTPUT(xLevel, "[ Switch List ]");
	OUTPUT(xLevel, "%4s   %24s %24s", "", "ID", "IP Address");
	FTM_LIST_count(pList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_SWITCH_CONFIG_PTR	pSwitchConfig;

		xRet = FTM_LIST_getAt(pList, i, (FTM_VOID_PTR _PTR_)&pSwitchConfig);
		if (xRet == FTM_RET_OK)
		{
			OUTPUT(xLevel, "%4d : %24s %24s", i+1, pSwitchConfig->pID, pSwitchConfig->pIP);
		}
	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SWITCH_CONFIG_load
(
	FTM_SWITCH_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	cJSON _PTR_ pItem;

	pItem = cJSON_GetObjectItem(pRoot, "ip");
	if (pItem == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to get switch ip!\n");
		return	xRet;
	}

	strncpy(pConfig->pIP, pItem->valuestring, sizeof(pConfig->pIP) - 1);					

	pItem = cJSON_GetObjectItem(pRoot, "id");
	if(pItem != NULL)
	{
		strncpy(pConfig->pID, pItem->valuestring, sizeof(pConfig->pID) - 1);					
	}

	return	xRet;
}

//////////////////////////////////////////////////////////////
//	FTM_SWITCH functions
//////////////////////////////////////////////////////////////
FTM_RET	FTM_SWITCH_create
(
	FTM_SWITCH_CONFIG_PTR	pConfig,
	struct FTM_CATCHB_STRUCT* pCatchB,
	FTM_SWITCH_AC_PTR	pACs,
	FTM_UINT32			ulACCount,
	FTM_SWITCH_PTR _PTR_ ppSwitch
)
{
	ASSERT(ppSwitch != NULL);

	FTM_RET		xRet = FTM_RET_OK;
	FTM_UINT32	i;
	FTM_SWITCH_PTR	pSwitch = NULL;

	pSwitch = (FTM_SWITCH_PTR)FTM_MEM_malloc(sizeof(FTM_SWITCH));
	if (pSwitch == NULL)
	{
		xRet= FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create switch!");
		goto finished;
	}

	if (pConfig != NULL)
	{
		memcpy(&pSwitch->xConfig, pConfig, sizeof(FTM_SWITCH_CONFIG));	
	}

	pSwitch->pCatchB = pCatchB;
	xRet = FTM_LIST_create(&pSwitch->pACList);
	if (xRet != FTM_RET_OK)
	{
		goto finished;	
	}
	FTM_LIST_setSeeker(pSwitch->pACList, FTM_SWITCH_AC_seeker);

	for(i = 0 ; i < ulACCount ; i++)
	{
		FTM_SWITCH_AC_PTR	pAC;
		
		pAC = (FTM_SWITCH_AC_PTR)FTM_MEM_malloc(sizeof(FTM_SWITCH_AC));
		if (pAC == NULL)
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
			ERROR(xRet, "Failed to create AC!");
			goto finished;
		}
		
		memcpy(pAC, &pACs[i], sizeof(FTM_SWITCH_AC));

		FTM_LIST_append(pSwitch->pACList, pAC);
	}

	FTM_LOCK_init(&pSwitch->xLock);

	*ppSwitch = pSwitch;

	return	FTM_RET_OK;

finished:
	if (pSwitch != NULL)
	{
		if (pSwitch->pACList != NULL)
		{
			FTM_SWITCH_AC_PTR	pAC;

			FTM_LIST_iteratorStart(pSwitch->pACList);
			while(FTM_LIST_iteratorNext(pSwitch->pACList, (FTM_VOID_PTR _PTR_)&pAC) == FTM_RET_OK)
			{
				FTM_MEM_free(pAC);	
			}

			
			FTM_LIST_destroy(&pSwitch->pACList);	
		}

		FTM_MEM_free(pSwitch);
	}

	return	xRet;
}

FTM_RET	FTM_SWITCH_destroy
(
	FTM_SWITCH_PTR _PTR_ ppSwitch
)
{
	ASSERT(ppSwitch != NULL);
	ASSERT(*ppSwitch != NULL);

	FTM_LOCK_set(&(*ppSwitch)->xLock);

	if ((*ppSwitch)->pACList != NULL)
	{
		FTM_SWITCH_AC_PTR	pAC;

		FTM_LIST_iteratorStart((*ppSwitch)->pACList);
		while(FTM_LIST_iteratorNext((*ppSwitch)->pACList, (FTM_VOID_PTR _PTR_)&pAC) == FTM_RET_OK)
		{
			FTM_MEM_free(pAC);	
		}

		FTM_LIST_destroy(&(*ppSwitch)->pACList);
	}

	FTM_LOCK_reset(&(*ppSwitch)->xLock);

	FTM_LOCK_final(&(*ppSwitch)->xLock);

	FTM_MEM_free(*ppSwitch);

	*ppSwitch = NULL;

	return	FTM_RET_OK;
}

FTM_BOOL	FTM_SWITCH_AC_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	return	(strcasecmp(((FTM_SWITCH_AC_PTR)pElement)->pIP, (FTM_CHAR_PTR)pIndicator) == 0);
}

FTM_RET	FTM_SWITCH_addAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pIP,
	FTM_SWITCH_AC_POLICY	xPolicy,
	FTM_SWITCH_AC_PTR _PTR_ ppAC
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pIP != NULL);
	ASSERT(ppAC != NULL);
	
	FTM_RET				xRet = FTM_RET_OK;
	FTM_SWITCH_AC_PTR	pAC;
	FTM_SWITCH_INFO_PTR	pInfo;

	xRet = FTM_SWITCH_getInfo(pSwitch, &pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get switch info!");
		return	xRet;
	}

	if (pInfo->fSetAC == NULL)
	{
		xRet = FTM_RET_NOT_SUPPORTED_FUNCTION;
		ERROR(xRet, "Failed to add AC!");
		return	xRet;
	}

	pAC = (FTM_SWITCH_AC_PTR)FTM_MEM_malloc(sizeof(FTM_SWITCH_AC));
	if (pAC == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to alloc AC!");
		return	xRet;	
	}

	strncpy(pAC->pIP, pIP, sizeof(pAC->pIP)- 1);
	pAC->xPolicy = xPolicy;

	xRet = FTM_LIST_append(pSwitch->pACList, pAC);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add AC to list!");	
		FTM_MEM_free(pAC);
		return	xRet;
	}

	xRet = pInfo->fSetAC(pSwitch, pIP, xPolicy);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to deny IP[%s] from switch[%s].", pIP, pSwitch->xConfig.pID);
	}

	*ppAC = pAC;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SWITCH_deleteAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pIP
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pIP != NULL);
	
	FTM_RET				xRet = FTM_RET_OK;
	FTM_SWITCH_AC_PTR	pAC;
	FTM_SWITCH_INFO_PTR	pInfo;

	INFO("Delete switch access control entry");
	xRet = FTM_SWITCH_getInfo(pSwitch, &pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get switch info!");
		return	xRet;
	}

	if (pInfo->fSetAC == NULL)
	{
		xRet = FTM_RET_NOT_SUPPORTED_FUNCTION;
		ERROR(xRet, "Failed to add AC!");
		return	xRet;
	}

	xRet = FTM_LIST_get(pSwitch->pACList, pIP, (FTM_VOID_PTR _PTR_)&pAC);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get AC");
		return	xRet;
	}

	xRet = pInfo->fSetAC(pSwitch, pIP, FTM_SWITCH_AC_POLICY_ALLOW);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to allow IP[%s] from switch[%s].", pIP, pSwitch->xConfig.pID);	
	}
	else
	{
		xRet = FTM_LIST_remove(pSwitch->pACList, pAC);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to remove AC");
			return	xRet;
		}

		FTM_MEM_free(pAC);
	}

	INFO("Delete switch access control exit");

	return	xRet;
}

FTM_RET	FTM_SWITCH_getAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pIP,
	FTM_SWITCH_AC_PTR _PTR_ ppAC
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pIP != NULL);
	ASSERT(ppAC != NULL);

	return	FTM_LIST_get(pSwitch->pACList, pIP, (FTM_VOID_PTR _PTR_)ppAC);
}

FTM_RET	FTM_SWITCH_getInfo
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_SWITCH_INFO_PTR _PTR_ ppInfo
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(ppInfo != NULL);

	FTM_SWITCH_INFO_PTR	pInfo = pSwitchInfos;

	while(pInfo->xModel != FTM_SWITCH_MODEL_UNKNOWN)
	{
		if (pInfo->xModel == pSwitch->xConfig.xModel)
		{
			*ppInfo = pInfo;

			return	FTM_RET_OK;
		}

		pInfo++;
	}

	return	FTM_RET_OBJECT_NOT_FOUND;

}

FTM_SWITCH_MODEL	FTM_getSwitchModelID
(	
	FTM_CHAR_PTR	pModel
)
{
	ASSERT(pModel != NULL);

	FTM_SWITCH_INFO_PTR	pInfo = pSwitchInfos;

	while(pInfo->xModel != FTM_SWITCH_MODEL_UNKNOWN)
	{
		if (strcasecmp(pModel, pInfo->pName) == 0)
		{
			break;
		}
		pInfo++;
	}

	return	pInfo->xModel;
}

FTM_CHAR_PTR	FTM_getSwitchModelName
(
	FTM_SWITCH_MODEL	xModel
)
{
	FTM_SWITCH_INFO_PTR	pInfo = pSwitchInfos;

	while(pInfo->xModel != FTM_SWITCH_MODEL_UNKNOWN)
	{
		if (pInfo->xModel == xModel)
		{
			return	pInfo->pName;
		}
		pInfo++;
	}

	return	pInfo->pName;
}

FTM_RET	FTM_SWITCH_TELNET_setAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWS_CMD_PTR	pLines,
	FTM_UINT32		ulCount
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_RET		xRet;
	FTM_TIMER	xTimer;
	FTM_UINT32	ulTimeout = 3000;
	FTM_BOOL	bSuccess = FTM_TRUE;
	FTM_TELNET_CLIENT_PTR	pClient;


	xRet = FTM_TELNET_CLIENT_create(&pClient);
	if (xRet != FTM_RET_OK)
	{
		FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "The telnet connection to switch failed.");
		goto finished;
	}

	FTM_TIMER_initMS(&xTimer, ulTimeout);

	xRet = FTM_TELNET_CLIENT_open(pClient, pSwitch->xConfig.pIP, 23);
	if (xRet == FTM_RET_OK)
	{
		FTM_UINT32	ulLine = 0;

		while(ulLine < ulCount)
		{
			FTM_CHAR	pReadLine[512];
			FTM_UINT32	ulReadLen;
			usleep(1000);

			memset(pReadLine, 0, sizeof(pReadLine));

			xRet = FTM_TELNET_CLIENT_readline(pClient, pReadLine, sizeof(pReadLine), &ulReadLen);
			if ((ulReadLen != 0) && isprint(pReadLine[0]))
			{
				INFO("READ : %s", pReadLine);
				if (strncasecmp(pReadLine, pLines[ulLine].pPrompt, strlen(pLines[ulLine].pPrompt)) == 0)
				{
					FTM_TELNET_CLIENT_writel(pClient, pLines[ulLine].pInput, strlen(pLines[ulLine].pInput));
					ulLine++;
				}
				FTM_TIMER_initMS(&xTimer, ulTimeout);
			}

			if (FTM_TIMER_isExpired(&xTimer))
			{
				bSuccess = FTM_FALSE;
				break;
			}

		}

		if (bSuccess)
		{
			FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "Applied the ACL policy to switch.");
			INFO("Applied the ACL policy to switch.");
		}
		else
		{
			FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "Failed to apply ACL policy.");
			INFO("Failed to apply ACL policy.");
		}

		xRet = FTM_TELNET_CLIENT_close(pClient);
		if (xRet != FTM_RET_OK)
		{
		}

	}
	else
	{
		FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "The telnet connection to switch failed.");
	}

	xRet = FTM_TELNET_CLIENT_destroy(&pClient);
	if (xRet != FTM_RET_OK)
	{
	}

finished:

	return	xRet;
}


FTM_RET	FTM_SWITCH_SSH_setAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWS_CMD_PTR	pLines,
	FTM_UINT32		ulCount
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pTargetIP != NULL);

	FTM_RET		xRet;
	FTM_SSH_PTR	pSSH = NULL;
	FTM_SSH_CHANNEL_PTR	pChannel = NULL;
	FTM_TIMER	xTimer;
	FTM_UINT32	ulTimeout = 3000;
	FTM_BOOL	bSuccess = FTM_TRUE;

	xRet = FTM_SSH_create(&pSSH);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to create SSH!");	
		goto finished;
	}

	xRet = FTM_SSH_connect(pSSH, pSwitch->xConfig.pIP, pSwitch->xConfig.pUserID, pSwitch->xConfig.pPasswd);
	if (xRet != FTM_RET_OK)
	{
		
		FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "The ssh connection to switch failed.");
		ERROR(xRet, "Failed to connect ssh!");	
		goto finished;
	}

	xRet = FTM_SSH_CHANNEL_create(pSSH, &pChannel);
	if (xRet != FTM_RET_OK)
	{
		FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "The ssh connection to switch failed.");
		ERROR(xRet, "Failed to create channel!");	
		goto finished;
	}

	FTM_TIMER_initMS(&xTimer, ulTimeout);

	xRet = FTM_SSH_CHANNEL_open(pChannel);
	if (xRet == FTM_RET_OK)
	{
		FTM_UINT32	ulLine = 0;

		FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_NORMAL, 0, "", pTargetIP, 0, "The ssh connected to switch.");

		while(ulLine < ulCount)
		{
			FTM_CHAR	pReadLine[512];
			FTM_UINT32	ulReadLen = 0;

			memset(pReadLine, 0, sizeof(pReadLine));
			xRet = FTM_SSH_CHANNEL_read(pChannel, pReadLine, sizeof(pReadLine)-1, &ulReadLen);
			if ((ulReadLen != 0) && isprint(pReadLine[0]))
			{
				INFO("ReadLine : %s", pReadLine);
				INFO("Waiting : %s", pLines[ulLine].pPrompt);
				if (strncasecmp(pReadLine, pLines[ulLine].pPrompt, strlen(pLines[ulLine].pPrompt)) == 0)
				{
					FTM_SSH_CHANNEL_writeLine(pChannel, pLines[ulLine].pInput);
					ulLine++;
					usleep(100000);
				}
				else if (strncasecmp(pReadLine, "% Authentication", 16) == 0)
				{
					FTM_SSH_CHANNEL_writeLine(pChannel, "\n");
					ulLine = 0;
				}
				else if (strncasecmp(pReadLine, "Press any key", 12) == 0)
				{
					FTM_SSH_CHANNEL_writeLine(pChannel, "");
				}
				else if (strncasecmp(pReadLine, "****", 4)== 0)
				{
					FTM_SSH_CHANNEL_writeLine(pChannel, "");
					FTM_SSH_CHANNEL_close(pChannel);
					FTM_SSH_CHANNEL_open(pChannel);
				}
				else if (strncasecmp(pReadLine, "%% Already exist flow", 20) == 0)
				{
					FTM_SSH_CHANNEL_writeLine(pChannel, "exit");
					break;	
				}
				FTM_TIMER_initMS(&xTimer, ulTimeout);
			}


			if (FTM_TIMER_isExpired(&xTimer))
			{
				bSuccess = FTM_FALSE;
				break;
			}

			usleep(1000);
		}

		if (bSuccess)
		{
			FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "Applied the ACL policy to switch.");
			INFO("Applied the ACL policy to switch.");
		}
		else
		{
			FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "Failed to apply ACL policy.");
			INFO("Failed to apply ACL policy.");
		}
		FTM_SSH_CHANNEL_close(pChannel);
	}
	else
	{
		FTM_CATCHB_addLog(pSwitch->pCatchB, FTM_LOG_TYPE_ERROR, 0, "", pTargetIP, 0, "The ssh connection to switch failed.");
		ERROR(xRet, "Failed to open channel.");	
	}


finished:
	
	if (pChannel != NULL)
	{
		INFO("FTM_SSH_CHANNEL_destroy");
		FTM_SSH_CHANNEL_destroy(&pChannel);	
	}

	if (pSSH != NULL)
	{
		FTM_SSH_disconnect(pSSH);

		INFO("FTM_SSH_destroy");
		FTM_SSH_destroy(&pSSH);	
	}

	return	xRet;
}

const FTM_CHAR_PTR	pConvertFormat = "_IDX=%d _TARGET=%s _LOCAL=%s acl.sh %s";

FTM_RET	FTM_SWITCH_loadScript
(
	FTM_CHAR_PTR	pFileName,
	FTM_UINT32		ulIndex,
	FTM_CHAR_PTR	pLocalIP,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWITCH_SCRIPT_PTR _PTR_ ppScript
)
{
	FTM_RET			xRet = FTM_RET_OK;
	FILE *pFile 	= NULL; 
	FTM_CHAR_PTR	pData = NULL;
	FTM_UINT32		ulFileLen = 0;
	FTM_UINT32		ulReadSize = 0;
	FTM_CHAR		pBuffer[1024];
	FTM_SWITCH_SCRIPT_PTR	pScript = NULL;
	FTM_UINT32		ulAllowLines = 0;
	FTM_UINT32		ulDenyLines = 0;
	cJSON _PTR_		pRoot = NULL;
	cJSON _PTR_		pSection = NULL;

	ASSERT(pFileName != NULL);
	ASSERT(pLocalIP != NULL);
	ASSERT(pTargetIP != NULL);
	ASSERT(ppScript != NULL);

	pFile = fopen(pFileName, "rt");
	if (pFile == NULL)
	{    
		xRet = FTM_RET_CONFIG_LOAD_FAILED; 
		ERROR(xRet, "Can't open file[%s]\n", pFileName);
		return  xRet; 
	}    

	fseek(pFile, 0L, SEEK_END);
	ulFileLen = ftell(pFile);
	fseek(pFile, 0L, SEEK_SET);
	fclose(pFile);

	if (ulFileLen > 0)
	{
		ulFileLen += 1024;
		pData = (FTM_CHAR_PTR)FTM_MEM_malloc(ulFileLen);
		if (pData == NULL)
		{    
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;  
			ERROR(xRet, "Failed to alloc buffer[size = %u]\n", ulFileLen);
			goto finished;
		}    
		memset(pData, 0, ulFileLen);
	}

	sprintf(pBuffer, pConvertFormat, ulIndex, pLocalIP, pTargetIP, pFileName);
	
	pFile = popen(pBuffer, "r");
	if (pFile == NULL)
	{
		xRet = FTM_RET_ERROR;
		ERROR(xRet, "Failed to covert script!");
		goto finished;
	}

	ulReadSize = fread(pData, 1, ulFileLen, pFile); 
	if (ulReadSize == 0)
	{    
		xRet = FTM_RET_FAILED_TO_READ_FILE;
		ERROR(xRet, "Failed to read configuration file[%u:%u]\n", ulFileLen, ulReadSize);
		goto finished;
	}    

	pclose(pFile);
	pFile = NULL;

	pRoot = cJSON_Parse(pData);
	if (pRoot == NULL)
	{    
		xRet = FTM_RET_INVALID_JSON_FORMAT;
		ERROR(xRet, "Invalid json format!\n");
		INFO("%s", pData);
		goto finished;
	}    


	pSection = cJSON_GetObjectItem(pRoot, "allow");
	if (pSection == NULL)
	{
		ERROR(xRet, "Invlalid script!");
		goto finished;	
	}

	if (pSection->type != cJSON_Array)
	{
		ERROR(xRet, "Invlalid script!");
		goto finished;	
	}
	ulAllowLines = cJSON_GetArraySize(pSection);

	pSection = cJSON_GetObjectItem(pRoot, "deny");
	if (pSection == NULL)
	{
		ERROR(xRet, "Invlalid script!");
		goto finished;	
	}

	if (pSection->type != cJSON_Array)
	{
		ERROR(xRet, "Invlalid script!");
		goto finished;	
	}
	ulDenyLines = cJSON_GetArraySize(pSection);

	pScript = (FTM_SWITCH_SCRIPT_PTR)FTM_MEM_malloc(sizeof(FTM_SWITCH_SCRIPT) + sizeof(FTM_SWS_CMD) * (ulAllowLines + ulDenyLines));
	if (pScript == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet,	"Not enough memory[%d]", sizeof(FTM_SWITCH_SCRIPT) + sizeof(FTM_SWS_CMD) * (ulAllowLines + ulDenyLines));
		goto finished;
	}

	pScript->xAllow.pLines = (FTM_SWS_CMD_PTR)((FTM_UINT8_PTR)pScript + sizeof(FTM_SWITCH_SCRIPT));
	pScript->xDeny.pLines = (FTM_SWS_CMD_PTR)((FTM_UINT8_PTR)pScript + sizeof(FTM_SWITCH_SCRIPT) + sizeof(FTM_SWS_CMD)*ulAllowLines);

	pSection = cJSON_GetObjectItem(pRoot, "allow");
	for(FTM_UINT32 i = 0 ; i < ulAllowLines ; i++)
	{
		cJSON _PTR_ pLine = cJSON_GetArrayItem(pSection, i);
		if (pLine == NULL)
		{
			ERROR(xRet, "Invlalid script!");
			goto finished;	
		}

		cJSON _PTR_ pPrompt = cJSON_GetObjectItem(pLine, "prompt");
		cJSON _PTR_ pCommand= cJSON_GetObjectItem(pLine, "command");

		if ((pPrompt == NULL) || (pCommand == NULL) || (pPrompt->type != cJSON_String) || (pCommand->type != cJSON_String))
		{
			ERROR(xRet, "Invlalid script!");
			goto finished;	
		}

		pScript->xAllow.ulCount++;
		strncpy(pScript->xAllow.pLines[i].pPrompt, pPrompt->valuestring, sizeof(pScript->xAllow.pLines[i].pPrompt) - 1);
		strncpy(pScript->xAllow.pLines[i].pInput, pCommand->valuestring, sizeof(pScript->xAllow.pLines[i].pInput) - 1);
	}
	
	pSection = cJSON_GetObjectItem(pRoot, "deny");
	for(FTM_UINT32 i = 0 ; i < ulDenyLines ; i++)
	{
		cJSON _PTR_ pLine = cJSON_GetArrayItem(pSection, i);
		if (pLine == NULL)
		{
			ERROR(xRet, "Invlalid script!");
			goto finished;	
		}

		cJSON _PTR_ pPrompt = cJSON_GetObjectItem(pLine, "prompt");
		cJSON _PTR_ pCommand= cJSON_GetObjectItem(pLine, "command");

		if ((pPrompt == NULL) || (pCommand == NULL) || (pPrompt->type != cJSON_String) || (pCommand->type != cJSON_String))
		{
			ERROR(xRet, "Invlalid script!");
			goto finished;	
		}

		pScript->xDeny.ulCount++;
		strncpy(pScript->xDeny.pLines[i].pPrompt, 	pPrompt->valuestring, 	sizeof(pScript->xDeny.pLines[i].pPrompt) - 1);
		strncpy(pScript->xDeny.pLines[i].pInput, 	pCommand->valuestring, 	sizeof(pScript->xDeny.pLines[i].pInput) - 1);
	}

	*ppScript = pScript;

finished:
	if (xRet != FTM_RET_OK)
	{
		if (pScript != NULL)
		{
			FTM_MEM_free(pScript);
		}
	}

	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
		pRoot = NULL;
	}

	if (pData != NULL)
	{
		FTM_MEM_free(pData);
		pData = NULL;
	}

	if (pFile != NULL)
	{
		pclose(pFile);	
		pFile = NULL;
	}

	return	xRet;
}
