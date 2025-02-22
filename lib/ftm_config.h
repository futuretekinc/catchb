#ifndef	FTM_CONFIG_H_
#define	FTM_CONFIG_H_

#include "ftm_types.h"
#include "ftm_list.h"
#include "ftm_system.h"
#include "ftm_analyzer.h"
#include "ftm_notifier.h"
#include "ftm_logger.h"
#include "ftm_trace.h"
#include "ftm_server.h"
#include "ftm_client.h"
#include "ftm_db.h"

typedef	struct	FTM_CONFIG_STRUCT
{
	FTM_CHAR			pFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN];
	FTM_SYSTEM_CONFIG	xSystem;

	struct
	{
		FTM_CHAR		pPath[FTM_PATH_LEN+1];
		FTM_LIST_PTR	pList;
	}	xSwitchModels;

	FTM_LIST_PTR		pSwitchList;

	FTM_DB_CONFIG		xDB;
	FTM_ANALYZER_CONFIG	xAnalyzer;
	FTM_NOTIFIER_CONFIG	xNotifier;
	FTM_LOGGER_CONFIG	xLogger;
	FTM_TRACE_CONFIG	xTrace;
	FTM_SERVER_CONFIG	xServer;
	FTM_CLIENT_CONFIG	xClient;

}	FTM_CONFIG, _PTR_ FTM_CONFIG_PTR;

FTM_RET	FTM_CONFIG_create(FTM_CONFIG_PTR _PTR_ ppConfig);
FTM_RET	FTM_CONFIG_destroy(FTM_CONFIG_PTR _PTR_ ppConfig);

FTM_RET	FTM_CONFIG_setDefault(FTM_CONFIG_PTR pConfig);

FTM_RET	FTM_CONFIG_load(FTM_CONFIG_PTR pConfig, char* pFileName);
FTM_RET	FTM_CONFIG_save(FTM_CONFIG_PTR pConfig, char* pFileName);
FTM_RET	FTM_CONFIG_show(FTM_CONFIG_PTR 	pConfig, FTM_TRACE_LEVEL xLevel);

#endif
