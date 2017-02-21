#ifndef	FTM_ALARAM_H_
#define	FTM_ALARAM_H_

#include <pthread.h>
#include "ftm_types.h"
#include "ftm_msgq.h"
#include "ftm_message.h"
#include "ftm_db.h"

typedef	struct	FTM_NOTIFIER_STRUCT
{
	FTM_BOOL		bStop;
	FTM_MSGQ_PTR	pMsgQ;
	FTM_BOOL		bInternalDB;
	FTM_DB_PTR		pDB;

	pthread_t		xThread;
}	FTM_NOTIFIER, _PTR_ FTM_NOTIFIER_PTR;

FTM_RET	FTM_NOTIFIER_create
(
	FTM_NOTIFIER_PTR _PTR_ ppNotifier
);

FTM_RET	FTM_NOTIFIER_destroy
(
	FTM_NOTIFIER_PTR _PTR_ ppNotifier
);

FTM_RET	FTM_NOTIFIER_start
(
	FTM_NOTIFIER_PTR pNotifier
);

FTM_RET	FTM_NOTIFIER_stop
(
	FTM_NOTIFIER_PTR pNotifier
);

FTM_RET	FTM_NOTIFIER_sendMessage
(
	FTM_NOTIFIER_PTR pNotifier,
	FTM_MSG_PTR	 pMsg
);

FTM_RET	FTM_NOTIFIER_sendAlarm
(
	FTM_NOTIFIER_PTR pNotifier,
	FTM_CHAR_PTR	 pID
);

#endif
