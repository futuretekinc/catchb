#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <liblogs.h>
#include <libsignal.h>
#include <libsocket.h>
#include <libutil.h>
#include <syslog.h>
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_message.h"
#include "ftm_notifier.h"

extern	FTM_CHAR_PTR	program_invocation_short_name;

FTM_VOID_PTR	FTM_NOTIFIER_process
(
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_NOTIFIER_create
(
	FTM_NOTIFIER_PTR _PTR_ ppNotifier
)
{
	ASSERT(ppNotifier != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_NOTIFIER_PTR	pNotifier;

	pNotifier = (FTM_NOTIFIER_PTR)FTM_MEM_malloc(sizeof(FTM_NOTIFIER));
	if (pNotifier == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create notifier!\n");	
	}
	else
	{
		memset(pNotifier, 0, sizeof(FTM_NOTIFIER));
		xRet = FTM_MSGQ_create(&pNotifier->pMsgQ);
		if (xRet != FTM_RET_OK)
		{
			FTM_MEM_free(pNotifier);
			ERROR(xRet, "Failed to create message queue!");
    	}
		else
		{
			*ppNotifier = pNotifier;	
		}
	}

	return	xRet;

}

FTM_RET	FTM_NOTIFIER_destroy
(
	FTM_NOTIFIER_PTR _PTR_ ppNotifier
)
{
	ASSERT(ppNotifier != NULL);
	ASSERT(*ppNotifier != NULL);

	FTM_RET	xRet = FTM_RET_OK;

	if ((*ppNotifier)->pMsgQ != NULL)
	{
		xRet = FTM_MSGQ_destroy(&(*ppNotifier)->pMsgQ);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to destroy message queue!\n");
		}
	}

	FTM_MEM_free(*ppNotifier);

	*ppNotifier = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_NOTIFIER_start
(
	FTM_NOTIFIER_PTR pNotifier
)
{
	ASSERT(pNotifier != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pNotifier->xThread != 0)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
		TRACE("The notifier is already running!\n");	
		return	xRet;
	}

	if (pthread_create(&pNotifier->xThread, NULL, FTM_NOTIFIER_process, (FTM_VOID_PTR)pNotifier) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		TRACE("Failed to start notifier!");
	}

	return	xRet;
}

FTM_RET	FTM_NOTIFIER_stop
(
	FTM_NOTIFIER_PTR pNotifier
)
{
	ASSERT(pNotifier != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pNotifier->xThread != 0)
	{
		pNotifier->bStop = FTM_TRUE;	
		pthread_join(pNotifier->xThread, NULL);

		pNotifier->xThread = 0;
	}

	return	xRet;
}


FTM_VOID_PTR	FTM_NOTIFIER_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_RET	xRet;
	FTM_NOTIFIER_PTR	pNotifier = (FTM_NOTIFIER_PTR)pData;

	pNotifier->bStop = FTM_FALSE;
    LOG("%s started.", program_invocation_short_name);

	if(pNotifier->pDB == NULL)
	{
		FTM_DB_PTR	pDB;

		xRet = FTM_DB_create(&pDB);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to create DB!\n");
			goto finished;
		}

		xRet = FTM_DB_open(pDB, "/tmp/catchb.db");
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to open DB!\n");
			FTM_DB_destroy(&pDB);

			goto finished;
		}

		pNotifier->pDB = pDB;
		pNotifier->bInternalDB = FTM_TRUE;
	}

    while(!pNotifier->bStop)
    {
		FTM_MSG_PTR	pRcvdMsg;

		xRet= FTM_MSGQ_timedPop(pNotifier->pMsgQ, 1000, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if (xRet == FTM_RET_OK)
		{
			FTM_UINT32	ulCount = 0;

			xRet = FTM_DB_ALARM_count(pNotifier->pDB, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				if(ulCount != 0)
				{
					FTM_ALARM_PTR	pAlarms;

					pAlarms = (FTM_ALARM_PTR)FTM_MEM_calloc(sizeof(FTM_ALARM), ulCount);
					if (pAlarms != NULL)
					{
						xRet = FTM_DB_ALARM_getList(pNotifier->pDB, pAlarms, ulCount, &ulCount);	
						if (xRet == FTM_RET_OK)
						{
							FTM_UINT32	i;

							for(i = 0 ; i < ulCount ; i++)
							{
								TRACE("Send e-mail to %s - %s:%s\n", pAlarms[i].pEmail, pAlarms[i].pID, pAlarms[i].pMessage);	
							}
						}
						else
						{
							ERROR(xRet, "Failed to get alarm list from DB!\n");	
						}

						FTM_MEM_free(pAlarms);
					}
					else
					{
						xRet = FTM_RET_NOT_ENOUGH_MEMORY;
						ERROR(xRet, "Failed to alloc alarm buffer[%lu * %lu]!\n", sizeof(FTM_ALARM), ulCount);
					}
				}
			}
			else
			{
				ERROR(xRet, "Failed to get alarm count!\n");
			}

			FTM_MEM_free(pRcvdMsg);
		}
	}

finished:

	if (pNotifier->bInternalDB)
	{
		if(pNotifier->pDB != NULL)
		{
			FTM_DB_destroy(&pNotifier->pDB);	
		}

		pNotifier->bInternalDB = FTM_FALSE;	
	}
	return	0;
}

FTM_RET	FTM_NOTIFIER_sendMessage
(
	FTM_NOTIFIER_PTR pNotifier,
	FTM_MSG_PTR	 pMsg
)
{
	ASSERT(pNotifier != NULL);
	ASSERT(pMsg != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	if (pNotifier->pMsgQ == NULL)
	{
		xRet = FTM_RET_NOT_INITIALIZED;
	}
	else
	{
		xRet = FTM_MSGQ_push(pNotifier->pMsgQ, pMsg);
	}

	return	xRet;
}

FTM_RET	FTM_NOTIFIER_sendAlarm
(
	FTM_NOTIFIER_PTR pNotifier,
	FTM_CHAR_PTR	 pID
)
{
	ASSERT(pNotifier != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet = FTM_RET_OK;
	FTM_MSG_SEND_ALARM_PTR	pMsg;

	pMsg = (FTM_MSG_SEND_ALARM_PTR)FTM_MEM_malloc(sizeof(FTM_MSG_SEND_ALARM));
	if (pMsg == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	else
	{
		pMsg->xHead.xType = FTM_MSG_TYPE_SEND_ALARM;
		pMsg->xHead.ulLen = sizeof(FTM_MSG_SEND_ALARM);
		strncpy(pMsg->pID, pID, sizeof(pMsg->pID) - 1);
	
		xRet = FTM_NOTIFIER_sendMessage(pNotifier, (FTM_VOID_PTR)pMsg);
		if(xRet != FTM_RET_OK)
		{
			FTM_MEM_free(pMsg);
			ERROR(xRet, "Failed to send message!\n");
		}
	}
	return	xRet;
}
