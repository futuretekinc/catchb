#ifndef	CATCHB_CONFIG_H_
#define	CATCHB_CONFIG_H_

#include "catchb_types.h"
#include "catchb_list.h"

typedef	struct	CATCHB_SWITCH_CONFIG_STRUCT
{
	char	pID[CATCHB_ID_LEN+1];
	char	pIP[CATCHB_IP_LEN+1];
}	CATCHB_SWITCH_CONFIG, _PTR_ CATCHB_SWITCH_CONFIG_PTR;

CATCHB_RET	CATCHB_SWITCH_CONFIG_create(CATCHB_SWITCH_CONFIG_PTR _PTR_ ppConfig);
CATCHB_RET	CATCHB_SWITCH_CONFIG_destroy(CATCHB_SWITCH_CONFIG_PTR _PTR_ ppConfig);

typedef	struct	CATCHB_CONFIG_STRUCT
{
	struct
	{
		CATCHB_CHAR		pPath[CATCHB_PATH_LEN+1];
		CATCHB_CHAR		pMain[CATCHB_FILE_NAME_LEN+1];
		CATCHB_UINT32	ulRetentionPeriod;		// days
	}	xLog;

	CATCHB_LIST_PTR		pProcessList;
	CATCHB_LIST_PTR		pSwitchList;
}	CATCHB_CONFIG, _PTR_ CATCHB_CONFIG_PTR;

CATCHB_RET	CATCHB_CONFIG_create(CATCHB_CONFIG_PTR _PTR_ ppConfig);
CATCHB_RET	CATCHB_CONFIG_destroy(CATCHB_CONFIG_PTR _PTR_ ppConfig);

CATCHB_RET	CATCHB_CONFIG_load(CATCHB_CONFIG_PTR pConfig, char* pFileName);
CATCHB_RET	CATCHB_CONFIG_show(CATCHB_CONFIG_PTR 	pConfig);

#endif