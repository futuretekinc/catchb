#ifndef	FTM_LOGGER_H_
#define	FTM_LOGGER_H_

#include "ftm_types.h"
#include "cjson/cJSON.h"

typedef	struct FTM_LOGGER_CONFIG_STRUCT
{
	FTM_CHAR	pPath[FTM_PATH_LEN+1];
	FTM_CHAR	pMain[FTM_FILE_NAME_LEN+1];
	FTM_UINT32	ulRetentionPeriod;		// days
}	FTM_LOGGER_CONFIG, _PTR_ FTM_LOGGER_CONFIG_PTR;

FTM_RET	FTM_LOGGER_CONFIG_load
(
	FTM_LOGGER_CONFIG_PTR	pConfig,
	cJSON _PTR_			pRoot
);

FTM_RET	FTM_LOGGER_CONFIG_save
(
	FTM_LOGGER_CONFIG_PTR	pConfig,
	cJSON _PTR_			pRoot
);

FTM_RET	FTM_LOGGER_CONFIG_show
(
	FTM_LOGGER_CONFIG_PTR	pConfig
);

//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////
typedef	struct	FTM_LOGGER_STRUCT
{
	FTM_LOGGER_CONFIG	xConfig;

	FTM_BOOL			bStop;
}	FTM_LOGGER, _PTR_ FTM_LOGGER_PTR;
	
FTM_RET	FTM_LOGGER_setConfig
(
	FTM_LOGGER_PTR	pLogger,
	FTM_LOGGER_CONFIG_PTR	pConfig
);

#endif
