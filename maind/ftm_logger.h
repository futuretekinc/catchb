#ifndef	FTM_LOGGER_H_
#define	FTM_LOGGER_H_

#include "ftm_types.h"
#include "cjson/cJSON.h"
#include "ftm_msgq.h"

struct FTM_CATCHB_STRUCT;

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
	FTM_CHAR			pName[FTM_NAME_LEN+1];

	FTM_LOGGER_CONFIG	xConfig;

	FTM_MSGQ_PTR		pMsgQ;
	FTM_BOOL			bStop;
	pthread_t			xThread;
}	FTM_LOGGER, _PTR_ FTM_LOGGER_PTR;

FTM_RET	FTM_LOGGER_create
(
	struct FTM_CATCHB_STRUCT _PTR_ pCatchB,
	FTM_LOGGER_PTR _PTR_ ppLogger
);

FTM_RET	FTM_LOGGER_destroy
(
	FTM_LOGGER_PTR _PTR_ ppLogger
);

FTM_RET	FTM_LOGGER_setConfig
(
	FTM_LOGGER_PTR	pLogger,
	FTM_LOGGER_CONFIG_PTR	pConfig
);

FTM_RET	FTM_LOGGER_start
(
	FTM_LOGGER_PTR	pLogger
);

FTM_RET	FTM_LOGGER_stop
(
	FTM_LOGGER_PTR	pLogger
);
#endif
