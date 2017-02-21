#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_switch.h"

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

FTM_RET	FTM_SWITCH_create
(
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_SWITCH_PTR _PTR_ ppSwitch
)
{
	ASSERT(ppSwitch != NULL);

	FTM_SWITCH_PTR	pSwitch;

	pSwitch = (FTM_SWITCH_PTR)FTM_MEM_malloc(sizeof(FTM_SWITCH));
	if (pSwitch == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	if (pConfig != NULL)
	{
		memcpy(&pSwitch->xConfig, pConfig, sizeof(FTM_SWITCH_CONFIG));	
	}

	FTM_LOCK_init(&pSwitch->xLock);

	*ppSwitch = pSwitch;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SWITCH_destroy
(
	FTM_SWITCH_PTR _PTR_ ppSwitch
)
{
	ASSERT(ppSwitch != NULL);
	ASSERT(*ppSwitch != NULL);

	FTM_LOCK_final(&(*ppSwitch)->xLock);

	FTM_MEM_free(*ppSwitch);

	*ppSwitch = NULL;

	return	FTM_RET_OK;
}

