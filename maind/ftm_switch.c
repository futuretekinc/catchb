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

	FTM_LIST_setSeeker(pSwitch->pACList, FTM_SWITCH_AC_seeker);

	FTM_LOCK_init(&pSwitch->xLock);

	*ppSwitch = pSwitch;

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
	FTM_INT32_PTR	pIndex
)
{
	ASSERT(pSwitch != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pIndex != NULL);
	
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

	*pIndex = nIndex;

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

	xRet = FTM_LIST_remove(pSwitch->pACList, pAC);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to remove AC");
		return	xRet;
	}

	FTM_MEM_free(pAC);

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
