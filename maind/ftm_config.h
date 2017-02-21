#ifndef	FTM_CONFIG_H_
#define	FTM_CONFIG_H_

#include "ftm_types.h"
#include "ftm_list.h"

typedef	struct	FTM_SWITCH_CONFIG_STRUCT
{
	char	pID[FTM_ID_LEN+1];
	char	pIP[FTM_IP_LEN+1];
}	FTM_SWITCH_CONFIG, _PTR_ FTM_SWITCH_CONFIG_PTR;

FTM_RET	FTM_SWITCH_CONFIG_create(FTM_SWITCH_CONFIG_PTR _PTR_ ppConfig);
FTM_RET	FTM_SWITCH_CONFIG_destroy(FTM_SWITCH_CONFIG_PTR _PTR_ ppConfig);

typedef	struct	FTM_CONFIG_STRUCT
{
	struct
	{
		FTM_CHAR		pPath[FTM_PATH_LEN+1];
		FTM_CHAR		pMain[FTM_FILE_NAME_LEN+1];
		FTM_UINT32	ulRetentionPeriod;		// days
	}	xLog;

	FTM_LIST_PTR		pProcessList;
	FTM_LIST_PTR		pSwitchList;
}	FTM_CONFIG, _PTR_ FTM_CONFIG_PTR;

FTM_RET	FTM_CONFIG_create(FTM_CONFIG_PTR _PTR_ ppConfig);
FTM_RET	FTM_CONFIG_destroy(FTM_CONFIG_PTR _PTR_ ppConfig);

FTM_RET	FTM_CONFIG_load(FTM_CONFIG_PTR pConfig, char* pFileName);
FTM_RET	FTM_CONFIG_show(FTM_CONFIG_PTR 	pConfig);

#endif
