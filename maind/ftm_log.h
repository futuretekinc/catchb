#ifndef	FTM_LOG_H_
#define	FTM_LOG_H_

#include "ftm_types.h"

typedef	struct	FTM_LOG_STRUCT
{
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_CHAR	pLog[FTM_LOG_LEN+1];
	FTM_CHAR	pTime[FTM_TIME_LEN+1];
	FTM_INT		nStatus;
}	FTM_LOG, _PTR_ FTM_LOG_PTR;

#endif
