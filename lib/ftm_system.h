#ifndef	_SYSTEM_H
#define	_SYSTEM_H

#include "ftm_types.h"
#include "ftm_trace.h"

#define	FTM_SYSTEM_FIELD_STATISTICS_INTERVAL		(1 << 0)
#define	FTM_SYSTEM_FIELD_STATISTICS_MAX_COUNT		(1 << 1)
#define	FTM_SYSTEM_FIELD_STATISTICS_COUNT			(1 << 2)
#define	FTM_SYSTEM_FIELD_STATISTICS_FIRST_TIME		(1 << 3)
#define	FTM_SYSTEM_FIELD_STATISTICS_LAST_TIME		(1 << 4)

typedef	struct	FTM_SYSTEM_INFO_STRUCT
{
	FTM_UINT32	ulFields;
	struct
	{
		FTM_UINT32	ulMaxCount;
		FTM_UINT32	ulInterval;
		FTM_UINT32	ulCount;
		FTM_UINT32	ulFirstTime;
		FTM_UINT32	ulLastTime;
	}	xStatistics;
}	FTM_SYSTEM_INFO, _PTR_ FTM_SYSTEM_INFO_PTR;

typedef	struct 	FTM_SYSTEM_CONFIG_STRUCT
{
	struct
	{
		FTM_UINT32	ulUpdateInterval;
	}	xCCTV;

	struct
	{
		FTM_UINT32	ulCount;
		FTM_UINT32	ulInterval;
	}	xStatistics;
}	FTM_SYSTEM_CONFIG, _PTR_ FTM_SYSTEM_CONFIG_PTR;

FTM_RET	FTM_SYSTEM_CONFIG_setDefault
(
	FTM_SYSTEM_CONFIG_PTR	pConfig
);

FTM_RET	FTM_SYSTEM_CONFIG_load
(
	FTM_SYSTEM_CONFIG_PTR	pConfig,
	cJSON _PTR_ 		pRoot
);

FTM_RET	FTM_SYSTEM_CONFIG_save
(
	FTM_SYSTEM_CONFIG_PTR	pConfig,
	cJSON _PTR_ 		pRoot
);

FTM_RET	FTM_SYSTEM_CONFIG_show
(
	FTM_SYSTEM_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL		xLevel
);

#endif
