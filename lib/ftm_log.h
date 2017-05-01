#ifndef	FTM_LOG_H_
#define	FTM_LOG_H_

#include "ftm_types.h"
#include "ftm_cctv.h"

typedef	enum	FTM_LOG_TYPE_ENUM
{
	FTM_LOG_TYPE_UNKNOWN = 0,
	FTM_LOG_TYPE_NORMAL,
	FTM_LOG_TYPE_ERROR
}	FTM_LOG_TYPE, _PTR_ FTM_LOG_TYPE_PTR;

typedef	struct	FTM_LOG_STRUCT
{
	FTM_LOG_TYPE	xType;		
	FTM_UINT32		ulTime;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_CHAR		pIP[FTM_IP_LEN+1];
	FTM_CHAR		pHash[FTM_HASH_LEN+1];
	FTM_CCTV_STAT	xStat;
}	FTM_LOG, _PTR_ FTM_LOG_PTR;

FTM_CHAR_PTR	FTM_LOG_TYPE_print
(
	FTM_LOG_TYPE	xType
);

#endif
