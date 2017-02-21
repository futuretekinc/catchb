#ifndef	FTM_CATCHB_H_
#define	FTM_CATCHB_H_

#include "ftm_types.h"
#include "ftm_msgq.h"
#include "ftm_db.h"
#include <pthread.h>
#include "ftm_detector.h"
#include "ftm_analyzer.h"
#include "ftm_timer.h"

typedef	struct 	FTM_CATCHB_CONFIG_STRUCT
{
	struct
	{
		FTM_CHAR	pFileName[FTM_PATH_LEN+FTM_FILE_NAME_LEN+1];
	}	xDB;

	struct
	{
		FTM_UINT32	ulUpdateInterval;
	}	xCCTV;
}	FTM_CATCHB_CONFIG, _PTR_ FTM_CATCHB_CONFIG_PTR;

typedef	struct	FTM_CATCHB_STRUCT
{
	FTM_CATCHB_CONFIG	xConfig;

	FTM_MSGQ_PTR		pMsgQ;	
	FTM_DB_PTR			pDB;	

	FTM_LIST_PTR		pCCTVList;
	FTM_LIST_PTR		pSwitchList;

	FTM_ANALYZER_PTR	pAnalyzer;
	FTM_DETECTOR_PTR	pDetector;
	FTM_EVENT_TIMER_MANAGER_PTR	pEventManager;
		
	pthread_t		xThread;
	FTM_BOOL		bStop;
}	FTM_CATCHB, _PTR_ FTM_CATCHB_PTR;

FTM_RET	FTM_CATCHB_create
(
	FTM_CATCHB_PTR _PTR_ ppCatchb
);

FTM_RET	FTM_CATCHB_destroy
(
	FTM_CATCHB_PTR _PTR_ ppCatchb
);

FTM_RET	FTM_CATCHB_start
(
	FTM_CATCHB_PTR	pCatchb
);

FTM_RET	FTM_CATCHB_stop
(
	FTM_CATCHB_PTR	pCatchb
);

FTM_RET	FTM_CATCHB_waitingForFinished
(
	FTM_CATCHB_PTR	pCarchb
);

FTM_RET	FTM_CATCHB_CCTV_get
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_PTR _PTR_ pCCTV
);

FTM_RET	FTM_CATCHB_CCTV_count
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_CCTV_hashUpdated
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
);

FTM_RET	FTM_CATCHB_CCTV_setStat
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_STAT	xStat
);

FTM_RET	FTM_CATCHB_CCTV_register
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
);
#endif
