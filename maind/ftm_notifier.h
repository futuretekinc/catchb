#ifndef	FTM_NOTIFIER_H_
#define	FTM_NOTIFIER_H_

#include <pthread.h>
#include "ftm_types.h"
#include "ftm_msgq.h"
#include "ftm_message.h"
#include "ftm_db.h"
#include "cjson/cJSON.h"

/******************************************************************
 *
 ******************************************************************/
typedef	struct FTM_NOTIFIER_MAIL_CONFIG_STRUCT
{
	FTM_CHAR	pServer[FTM_HOST_NAME_LEN+1];
	FTM_UINT16	usPort;
	FTM_CHAR	pUserID[FTM_ID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_CHAR	pSender[FTM_NAME_LEN + 1];
}	FTM_NOTIFIER_MAIL_CONFIG, _PTR_ FTM_NOTIFIER_MAIL_CONFIG_PTR;

FTM_RET	FTM_NOTIFIER_MAIL_CONFIG_setDefault
(
	FTM_NOTIFIER_MAIL_CONFIG_PTR	pConfig
);

FTM_RET	FTM_NOTIFIER_MAIL_CONFIG_load
(
	FTM_NOTIFIER_MAIL_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
);

FTM_RET	FTM_NOTIFIER_MAIL_CONFIG_show
(
	FTM_NOTIFIER_MAIL_CONFIG_PTR	pConfig
);

/******************************************************************
 *
 ******************************************************************/
typedef	struct	FTM_NOTIFIER_CONFIG_STRUCT
{
	FTM_NOTIFIER_MAIL_CONFIG	xMail;
}	FTM_NOTIFIER_CONFIG, _PTR_ FTM_NOTIFIER_CONFIG_PTR;

FTM_RET	FTM_NOTIFIER_CONFIG_setDefault
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig
);

FTM_RET	FTM_NOTIFIER_CONFIG_load
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
);

FTM_RET	FTM_NOTIFIER_CONFIG_show
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig
);


/******************************************************************
 *
 ******************************************************************/
typedef	struct	FTM_NOTIFIER_STRUCT
{
	FTM_NOTIFIER_CONFIG	xConfig;

	struct FTM_CATCHB_STRUCT _PTR_ pCatchB;

	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_BOOL		bStop;
	FTM_MSGQ_PTR	pMsgQ;

	pthread_t		xThread;
}	FTM_NOTIFIER, _PTR_ FTM_NOTIFIER_PTR;

FTM_RET	FTM_NOTIFIER_create
(
	struct FTM_CATCHB_STRUCT _PTR_ pCatchB,
	FTM_NOTIFIER_PTR _PTR_ ppNotifier
);

FTM_RET	FTM_NOTIFIER_destroy
(
	FTM_NOTIFIER_PTR _PTR_ ppNotifier
);

FTM_RET	FTM_NOTIFIER_setConfig
(
	FTM_NOTIFIER_PTR	pNotifier,
	FTM_NOTIFIER_CONFIG_PTR	pConfig
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
