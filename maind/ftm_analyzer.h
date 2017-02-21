#ifndef	FTM_ANALYZER_H_
#define	FTM_ANALYZER_H_

#include <pthread.h>
#include "ftm_types.h"
#include "ftm_list.h"
#include "ftm_db.h"
#include "ftm_msgq.h"
#include "ftm_lock.h"

typedef	struct FTM_ANALYZER_STRUCT
{
	FTM_CHAR		pName[FTM_NAME_LEN+1];

	struct	FTM_CATCHB_STRUCT _PTR_	pCatchB;

	FTM_MSGQ_PTR	pMsgQ;

	FTM_LIST_PTR	pList;	
	FTM_LOCK_PTR	pLock;

	pthread_t		xThread;
	FTM_BOOL		bStop;
} FTM_ANALYZER, _PTR_ FTM_ANALYZER_PTR;

FTM_RET	FTM_ANALYZER_create
(
	struct	FTM_CATCHB_STRUCT _PTR_	pCatchB,
	FTM_ANALYZER_PTR _PTR_ ppAnalyzer
);

FTM_RET	FTM_ANALYZER_destroy
(
	FTM_ANALYZER_PTR _PTR_ ppAnalyzer
);

FTM_RET	FTM_ANALYZER_start
(
	FTM_ANALYZER_PTR pAnalyzer
);

FTM_RET	FTM_ANALYZER_stop
(
	FTM_ANALYZER_PTR pAnalyzer
);

FTM_RET	FTM_ANALYZER_CCTV_add
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTM_ANALYZER_CCTV_delete
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTM_ANALYZER_CCTV_count
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_ANALYZER_CCTV_getList
(
	FTM_ANALYZER_PTR	pAnalyzer,
	FTM_ID_PTR			pID,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
);
#endif
