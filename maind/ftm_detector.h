#ifndef	FTM_DETECTOR_H_
#define	FTM_DETECTOR_H_

#include "ftm_types.h"
#include "ftm_msgq.h"
#include "ftm_db.h"
#include <pthread.h>

typedef	struct	FTM_DETECTOR_STRUCT
{
	struct	FTM_CATCHB_STRUCT _PTR_	pCatchB;

	FTM_MSGQ_PTR	pMsgQ;	

	pthread_t		xThread;
	FTM_BOOL		bStop;
}	FTM_DETECTOR, _PTR_ FTM_DETECTOR_PTR;

FTM_RET	FTM_DETECTOR_create
(
	struct	FTM_CATCHB_STRUCT _PTR_	pCatchB,
	FTM_DETECTOR_PTR _PTR_ ppDetector
);

FTM_RET	FTM_DETECTOR_destroy
(
	FTM_DETECTOR_PTR _PTR_ ppDetector
);

FTM_RET	FTM_DETECTOR_start
(
	FTM_DETECTOR_PTR	pDetector
);

FTM_RET	FTM_DETECTOR_stop
(
	FTM_DETECTOR_PTR	pDetector
);

#endif
