#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_switch.h"

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
	FTM_LIST_PTR	pList
)
{
	ASSERT(pList != NULL);

	FTM_RET		xRet;
	FTM_UINT32	i, ulCount;

	LOG("");
	LOG("[ Switch List ]");
	LOG("%4s   %24s %24s", "", "ID", "IP Address");
	FTM_LIST_count(pList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_SWITCH_CONFIG_PTR	pSwitchConfig;

		xRet = FTM_LIST_getAt(pList, i, (FTM_VOID_PTR _PTR_)&pSwitchConfig);
		if (xRet == FTM_RET_OK)
		{
			LOG("%4d : %24s %24s", i+1, pSwitchConfig->pID, pSwitchConfig->pIP);
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
	FTM_BOOL			pIndexTable[48];
	FTM_INT32			nIndex = -1;
	FTM_INT				i;
	FTM_SWITCH_AC_PTR	pAC;

	memset(pIndexTable, 0, sizeof(pIndexTable));

	FTM_LIST_iteratorStart(pSwitch->pACList);
	while(FTM_LIST_iteratorNext(pSwitch->pACList, (FTM_VOID_PTR _PTR_)&pAC) == FTM_RET_OK)
	{
		pIndexTable[pAC->nIndex] = FTM_TRUE;
	}

	for(i = 0 ; i < 48 ; i++)
	{
		if (pIndexTable[i] == FTM_FALSE)	
		{
			nIndex = i;
			break;
		}
	}

	if (nIndex < 0)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "AC List is full!");
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
	pAC->nIndex = nIndex;
	pAC->xPolicy = xPolicy;

	xRet = FTM_LIST_append(pSwitch->pACList, pAC);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to add AC to list!");	
		FTM_MEM_free(pAC);
		return	xRet;
	}

	xRet = FTM_SWITCH_NST_process(pSwitch, pIP, nIndex, xPolicy);
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

	xRet = FTM_LIST_get(pSwitch->pACList, pIP, (FTM_VOID_PTR _PTR_)&pAC);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get AC");
		return	xRet;
	}

	xRet = FTM_SWITCH_NST_process(pSwitch, pIP, pAC->nIndex, FTM_SWITCH_AC_POLICY_ALLOW);
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

FTM_SWITCH_MODEL	FTM_getSwitchModelID
(	
	FTM_CHAR_PTR	pModel
)
{
	ASSERT(pModel != NULL);

	if (strcasecmp(pModel, "nst") == 0)
	{
		return	FTM_SWITCH_MODEL_NST;	
	}
	else if (strcasecmp(pModel, "dasan") == 0)
	{
		return	FTM_SWITCH_MODEL_DASAN;	
	}
	else if (strcasecmp(pModel, "juniper") == 0)
	{
		return	FTM_SWITCH_MODEL_JUNIPER;	
	}

	return	FTM_SWITCH_MODEL_UNKNOWN;
}

