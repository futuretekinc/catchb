#ifndef	_FTM_MSGQ_H_
#define	_FTM_MSGQ_H_

#include "ftm_types.h"
#include "ftm_queue.h"
#include <semaphore.h>

typedef	struct FTM_MSGQ_STRUCT
{
	FTM_QUEUE	xQueue;

	sem_t		xLock;
}	FTM_MSGQ, _PTR_ FTM_MSGQ_PTR;


FTM_RET	FTM_MSGQ_create(FTM_MSGQ_PTR _PTR_ ppMsgQ);
FTM_RET	FTM_MSGQ_destroy(FTM_MSGQ_PTR _PTR_ ppMsgQ);

FTM_RET	FTM_MSGQ_init(FTM_MSGQ_PTR pMsgQ);
FTM_RET	FTM_MSGQ_final(FTM_MSGQ_PTR pMsgQ);

FTM_RET FTM_MSGQ_push(FTM_MSGQ_PTR pMsgQ, FTM_VOID_PTR pMsg);
FTM_RET FTM_MSGQ_pop(FTM_MSGQ_PTR pMsgQ, FTM_VOID_PTR _PTR_ ppMsg);
FTM_RET FTM_MSGQ_timedPop(FTM_MSGQ_PTR pMsgQ, FTM_UINT32 ulTimeout, FTM_VOID_PTR _PTR_ pMsg);

FTM_RET FTM_MSGQ_count(FTM_MSGQ_PTR pMsgQ, FTM_UINT32_PTR pulCount);

FTM_RET	FTM_MSGQ_pushQuit(FTM_MSGQ_PTR pMsgQ);
#endif
