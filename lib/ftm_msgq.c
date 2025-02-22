#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <errno.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_queue.h"
#include "ftm_mem.h"
#include "ftm_msgq.h"
#include "ftm_message.h"

FTM_RET	FTM_MSGQ_create(FTM_MSGQ_PTR _PTR_ ppMsgQ)
{
	FTM_MSGQ_PTR	pMsgQ;

	pMsgQ = (FTM_MSGQ_PTR)FTM_MEM_malloc(sizeof(FTM_MSGQ));
	if (pMsgQ == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTM_MSGQ_init(pMsgQ);

	*ppMsgQ = pMsgQ;
	
	return	FTM_RET_OK;
}

FTM_RET FTM_MSGQ_init(FTM_MSGQ_PTR pMsgQ)
{

	memset(pMsgQ, 0, sizeof(FTM_MSGQ));
	FTM_QUEUE_init(&pMsgQ->xQueue);	
	sem_init(&pMsgQ->xLock, 0, 0);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MSGQ_final(FTM_MSGQ_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);
	
	FTM_VOID_PTR pItem;

	while (FTM_QUEUE_pop(&pMsgQ->xQueue, &pItem) == FTM_RET_OK)
	{
		FTM_MEM_free(pItem);	
	}

	sem_destroy(&pMsgQ->xLock);
	FTM_QUEUE_final(&pMsgQ->xQueue);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MSGQ_destroy(FTM_MSGQ_PTR _PTR_ ppMsgQ)
{

	FTM_MSGQ_final((*ppMsgQ));

	FTM_MEM_free(*ppMsgQ);

	*ppMsgQ = NULL;

	return	FTM_RET_OK;

}

FTM_RET FTM_MSGQ_push(FTM_MSGQ_PTR pMsgQ, FTM_VOID_PTR pMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;

	xRet = FTM_QUEUE_push(&pMsgQ->xQueue, pMsg);
	if (xRet == FTM_RET_OK)
	{
		sem_post(&pMsgQ->xLock);
	}

	return	xRet;
}

FTM_RET FTM_MSGQ_pop(FTM_MSGQ_PTR pMsgQ, FTM_VOID_PTR _PTR_ ppMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(ppMsg != NULL);

	FTM_RET			xRet;
	FTM_VOID_PTR	pMsg;

	sem_wait(&pMsgQ->xLock);

	xRet = FTM_QUEUE_pop(&pMsgQ->xQueue, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		sem_post(&pMsgQ->xLock);
	}
	else
	{
		*ppMsg = pMsg;
	}

	return	xRet;
}

FTM_RET FTM_MSGQ_timedPop(FTM_MSGQ_PTR pMsgQ, FTM_UINT32 ulTimeout, FTM_VOID_PTR _PTR_ ppMsg)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(ppMsg != NULL);

	FTM_RET	xRet;
	FTM_VOID_PTR	pMsg;
	struct timespec	xTime;
	FTM_INT	nRet;

	clock_gettime(CLOCK_REALTIME, &xTime);	

	xTime.tv_nsec += (ulTimeout % 1000) * 1000000;
	xTime.tv_sec  += (ulTimeout / 1000) + (xTime.tv_nsec / 1000000000);
	xTime.tv_nsec %= 1000000000;

	nRet = sem_timedwait(&pMsgQ->xLock, &xTime);
	if (nRet != 0)
	{
		return	FTM_RET_TIMEOUT;
	}

	xRet = FTM_QUEUE_pop(&pMsgQ->xQueue, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		sem_post(&pMsgQ->xLock);
	}
	else
	{
		*ppMsg = pMsg;
	}

	return	xRet;
}

FTM_RET FTM_MSGQ_count(FTM_MSGQ_PTR pMsgQ, FTM_UINT32_PTR pulCount)
{
	ASSERT(pMsgQ != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_QUEUE_count(&pMsgQ->xQueue, pulCount);
}

FTM_RET	FTM_MSGQ_pushQuit(FTM_MSGQ_PTR pMsgQ)
{
	ASSERT(pMsgQ != NULL);

	FTM_RET	xRet;
	FTM_MSG_PTR	pMsg;

	pMsg = (FTM_MSG_PTR)FTM_MEM_malloc(sizeof(FTM_MSG));
	if (pMsg == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR(xRet, "Failed to create quit message!");
		return	xRet;	
	}

	pMsg->xType = FTM_MSG_TYPE_QUIT;
	pMsg->ulLen = sizeof(FTM_MSG);

	xRet = FTM_MSGQ_push(pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to push message!");
		FTM_MEM_free(pMsg);	
	}

	return	xRet;
}
