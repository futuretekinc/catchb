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
#define	FTM_NOTIFIER_SYSLOG_MODE_NONE		0
#define	FTM_NOTIFIER_SYSLOG_MODE_CHANGED	1
#define	FTM_NOTIFIER_SYSLOG_MODE_LEVEL		2

typedef	struct FTM_NOTIFIER_SYSLOG_CONFIG_STRUCT
{
	FTM_BOOL		bEnable;
	FTM_UINT32		xMode;
	FTM_LOG_TYPE	xLogLevel;
}	FTM_NOTIFIER_SYSLOG_CONFIG, _PTR_ FTM_NOTIFIER_SYSLOG_CONFIG_PTR;

FTM_RET	FTM_NOTIFIER_SYSLOG_CONFIG_setDefault
(
	FTM_NOTIFIER_SYSLOG_CONFIG_PTR	pConfig
);

FTM_RET	FTM_NOTIFIER_SYSLOG_CONFIG_load
(
	FTM_NOTIFIER_SYSLOG_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
);

FTM_RET	FTM_NOTIFIER_SYSLOG_CONFIG_save
(
	FTM_NOTIFIER_SYSLOG_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
);

FTM_RET	FTM_NOTIFIER_SYSLOG_CONFIG_show
(
	FTM_NOTIFIER_SYSLOG_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
);

/******************************************************************
 *
 ******************************************************************/
typedef	struct FTM_NOTIFIER_SMTP_CONFIG_STRUCT
{
	FTM_BOOL	bEnable;
	FTM_CHAR	pServer[FTM_HOST_NAME_LEN+1];
	FTM_UINT16	usPort;
	FTM_CHAR	pUserID[FTM_ID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_CHAR	pSender[FTM_NAME_LEN + 1];
}	FTM_NOTIFIER_SMTP_CONFIG, _PTR_ FTM_NOTIFIER_SMTP_CONFIG_PTR;

FTM_RET	FTM_NOTIFIER_SMTP_CONFIG_setDefault
(
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig
);

FTM_RET	FTM_NOTIFIER_SMTP_CONFIG_load
(
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
);

FTM_RET	FTM_NOTIFIER_SMTP_CONFIG_save
(
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
);

FTM_RET	FTM_NOTIFIER_SMTP_CONFIG_show
(
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
);

/******************************************************************
 *
 ******************************************************************/
typedef	struct	FTM_NOTIFIER_CONFIG_STRUCT
{
	FTM_NOTIFIER_SYSLOG_CONFIG	xSyslog;
	FTM_NOTIFIER_SMTP_CONFIG	xSMTP;
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

FTM_RET	FTM_NOTIFIER_CONFIG_save
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
);

FTM_RET	FTM_NOTIFIER_CONFIG_show
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
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

FTM_RET	FTM_NOTIFIER_getConfig
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
	FTM_NOTIFIER_PTR 	pNotifier,
	FTM_LOG_TYPE		xType,
	FTM_CHAR_PTR	 	pID,
	FTM_CHAR_PTR	 	pIP,
	FTM_UINT32			ulTime,
	FTM_CCTV_STAT	 	xNewStat,
	FTM_CHAR_PTR		pHash,
	FTM_CCTV_STAT	 	xOriginalStat
);

#endif
