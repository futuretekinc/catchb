#include <sys/types.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_mem.h"
#include "ftm_message.h"
#include "ftm_notifier.h"
#include "ftm_alarm.h"
#include "ftm_smtpc.h"
#include "ftm_time.h"

#undef	__MODULE__
#define	__MODULE__	"notifier"

extern	FTM_CHAR_PTR	program_invocation_short_name;

FTM_VOID_PTR	FTM_NOTIFIER_process
(
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_NOTIFIER_onSendAlarm
(
	FTM_NOTIFIER_PTR	pNotifier,
	FTM_CHAR_PTR		pID,
	FTM_ALARM_PTR	pAlarm
);

////////////////////////////////////////////////////////////////////////
FTM_RET	FTM_NOTIFIER_CONFIG_setDefault
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig
)
{
	FTM_NOTIFIER_SYSLOG_CONFIG_setDefault(&pConfig->xSyslog);
	FTM_NOTIFIER_SMTP_CONFIG_setDefault(&pConfig->xSMTP);

	return	FTM_RET_OK;
}

FTM_RET	FTM_NOTIFIER_CONFIG_load
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	cJSON _PTR_	pSection;

	pSection = cJSON_GetObjectItem(pRoot, "syslog");
	if (pSection != NULL)
	{
		xRet = FTM_NOTIFIER_SYSLOG_CONFIG_load(&pConfig->xSyslog, pSection);
	}

	pSection = cJSON_GetObjectItem(pRoot, "smtp");
	if (pSection != NULL)
	{
		xRet = FTM_NOTIFIER_SMTP_CONFIG_load(&pConfig->xSMTP, pSection);
	}

	return	xRet;
}

FTM_RET	FTM_NOTIFIER_CONFIG_save
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	cJSON _PTR_	pSection;

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		FTM_NOTIFIER_SYSLOG_CONFIG_save(&pConfig->xSyslog, pSection);

		cJSON_AddItemToObject(pRoot, "syslog", pSection);
	}

	pSection = cJSON_CreateObject();
	if (pSection != NULL)
	{
		FTM_NOTIFIER_SMTP_CONFIG_save(&pConfig->xSMTP, pSection);

		cJSON_AddItemToObject(pRoot, "smtp", pSection);
	}

	return	xRet;
}

FTM_RET	FTM_NOTIFIER_CONFIG_show
(
	FTM_NOTIFIER_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
)
{
	ASSERT(pConfig != NULL);

	FTM_NOTIFIER_SYSLOG_CONFIG_show(&pConfig->xSyslog, xLevel);
	FTM_NOTIFIER_SMTP_CONFIG_show(&pConfig->xSMTP, xLevel);

	return	FTM_RET_OK;

}

///////////////////////////////////////////////////////////////////
FTM_RET	FTM_NOTIFIER_SYSLOG_CONFIG_setDefault
(
	FTM_NOTIFIER_SYSLOG_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	pConfig->bEnable	= FTM_CATCHB_NOTIFIER_SYSLOG_ENABLE;
	pConfig->xMode		= FTM_CATCHB_NOTIFIER_SYSLOG_MODE;
	pConfig->xLogLevel	= FTM_CATCHB_NOTIFIER_SYSLOG_LEVEL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_NOTIFIER_SYSLOG_CONFIG_save
(
	FTM_NOTIFIER_SYSLOG_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON_AddStringToObject(pRoot, "enable", (pConfig->bEnable?"yes":"no"));
	switch(pConfig->xMode)
	{
	case	FTM_NOTIFIER_SYSLOG_MODE_CHANGED: 	cJSON_AddStringToObject(pRoot, "mode", "changed"); break;
	case	FTM_NOTIFIER_SYSLOG_MODE_LEVEL: 	cJSON_AddStringToObject(pRoot, "mode", "level"); break;
	default:	cJSON_AddStringToObject(pRoot, "mode", "none"); break;
	}

	switch(pConfig->xLogLevel)
	{
	case	FTM_LOG_TYPE_NORMAL: cJSON_AddStringToObject(pRoot, "level", "info");	break;
	default:
	cJSON_AddStringToObject(pRoot, "level", "error");	break;
	}

	return	FTM_RET_OK;

}

FTM_RET	FTM_NOTIFIER_SYSLOG_CONFIG_load
(
	FTM_NOTIFIER_SYSLOG_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pItem;

	pItem = cJSON_GetObjectItem(pRoot, "enable");
	if (pItem != NULL)
	{
		if ((strcasecmp(pItem->valuestring, "on") == 0) || (strcasecmp(pItem->valuestring, "yes") == 0))
		{
			pConfig->bEnable = FTM_TRUE;
		}
		else if ((strcasecmp(pItem->valuestring, "off") == 0) || (strcasecmp(pItem->valuestring, "no") == 0))
		{
			pConfig->bEnable = FTM_FALSE;
		}
	}

	pItem = cJSON_GetObjectItem(pRoot, "mode");
	if (pItem != NULL)
	{
		if (strcasecmp(pItem->valuestring, "changed") == 0)
		{
			pConfig->xMode = FTM_NOTIFIER_SYSLOG_MODE_CHANGED;
		}
		else if (strcasecmp(pItem->valuestring, "level") == 0)
		{
			pConfig->xMode = FTM_NOTIFIER_SYSLOG_MODE_LEVEL;
		}
	}

	pItem = cJSON_GetObjectItem(pRoot, "level");
	if (pItem != NULL)
	{
		if (strcasecmp(pItem->valuestring, "error") == 0)
		{
			pConfig->xLogLevel = FTM_LOG_TYPE_ERROR;
		}
		else if (strcasecmp(pItem->valuestring, "info") == 0)
		{
			pConfig->xLogLevel = FTM_LOG_TYPE_NORMAL;
		}
	}

	return	FTM_RET_OK;

}

FTM_RET	FTM_NOTIFIER_SYSLOG_CONFIG_show
(
	FTM_NOTIFIER_SYSLOG_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL					xLevel
)
{
	ASSERT(pConfig != NULL);

	printf("\n[ Syslog Configuration ]\n");
	printf("%16s : %s\n", "Enable", (pConfig->bEnable?"yes":"no"));
	switch(pConfig->xMode)
	{
	case	FTM_NOTIFIER_SYSLOG_MODE_CHANGED: 	printf("%16s : %s\n", "Mode", "changed"); break;
	case	FTM_NOTIFIER_SYSLOG_MODE_LEVEL:  	printf("%16s : %s\n", "Mode", "level"); break;
	default:  	printf("%16s : %s\n", "Mode", "none"); break;
	}

	switch(pConfig->xLogLevel)
	{
	case	FTM_LOG_TYPE_NORMAL: printf("%16s : %s\n", "Level", "info"); break;
	default: printf("%16s : %s\n", "Level", "error"); break;
	}


	return	FTM_RET_OK;
}
///////////////////////////////////////////////////////////////////
FTM_RET	FTM_NOTIFIER_SMTP_CONFIG_setDefault
(
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	pConfig->bEnable = FTM_CATCHB_DEFAULT_SMTP_ENABLE;
	strncpy(pConfig->pServer, FTM_CATCHB_DEFAULT_SMTP_SERVER, sizeof(pConfig->pServer) - 1);
	pConfig->usPort = FTM_CATCHB_DEFAULT_SMTP_PORT;
	strncpy(pConfig->pUserID, FTM_CATCHB_DEFAULT_SMTP_USER_ID, sizeof(pConfig->pUserID) - 1);
	strncpy(pConfig->pPasswd, FTM_CATCHB_DEFAULT_SMTP_PASSWD, sizeof(pConfig->pPasswd) - 1);
	strncpy(pConfig->pSender, FTM_CATCHB_DEFAULT_SMTP_SENDER, sizeof(pConfig->pSender) - 1);

	return	FTM_RET_OK;
}

FTM_RET	FTM_NOTIFIER_SMTP_CONFIG_load
(
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON _PTR_ pItem;

	pItem = cJSON_GetObjectItem(pRoot, "enable");
	if (pItem != NULL)
	{
		if ((strcasecmp(pItem->valuestring, "on") == 0) || (strcasecmp(pItem->valuestring, "yes") == 0))
		{
			pConfig->bEnable = FTM_TRUE;
		}
		else if ((strcasecmp(pItem->valuestring, "off") == 0) || (strcasecmp(pItem->valuestring, "no") == 0))
		{
			pConfig->bEnable = FTM_FALSE;
		}
	}

	pItem = cJSON_GetObjectItem(pRoot, "server");
	if (pItem != NULL)
	{
		strncpy(pConfig->pServer, pItem->valuestring, sizeof(pConfig->pServer) - 1);
	}

	pItem = cJSON_GetObjectItem(pRoot, "port");
	if (pItem != NULL)
	{
		pConfig->usPort = pItem->valueint;
	}

	pItem = cJSON_GetObjectItem(pRoot, "userid");
	if (pItem != NULL)
	{
		strncpy(pConfig->pUserID, pItem->valuestring, sizeof(pConfig->pUserID) - 1);
	}

	pItem = cJSON_GetObjectItem(pRoot, "passwd");
	if (pItem != NULL)
	{
		strncpy(pConfig->pPasswd, pItem->valuestring, sizeof(pConfig->pPasswd) - 1);
	}

	return	FTM_RET_OK;

}

FTM_RET	FTM_NOTIFIER_SMTP_CONFIG_save
(
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
)
{
	ASSERT(pConfig != NULL);
	ASSERT(pRoot != NULL);

	cJSON_AddStringToObject(pRoot, "enable", (pConfig->bEnable?"yes":"no"));
	cJSON_AddStringToObject(pRoot, "server", pConfig->pServer);
	cJSON_AddNumberToObject(pRoot, "port", pConfig->usPort);
	cJSON_AddStringToObject(pRoot, "userid", pConfig->pUserID);
	cJSON_AddStringToObject(pRoot, "passwd", pConfig->pPasswd);

	return	FTM_RET_OK;

}

FTM_RET	FTM_NOTIFIER_SMTP_CONFIG_show
(
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL					xLevel
)
{
	ASSERT(pConfig != NULL);

	printf("\n[ SMTP Configuration ]\n");
	printf("%16s : %s\n", "Enable", (pConfig->bEnable?"yes":"no"));
	printf("%16s : %s\n", "Server", pConfig->pServer);
	printf("%16s : %d\n", "Port", 	pConfig->usPort);
	printf("%16s : %s\n", "user ID", pConfig->pUserID);
	printf("%16s : %s\n", "Password", pConfig->pPasswd);

	return	FTM_RET_OK;
}

///////////////////////////////////////////////////////////////////
FTM_RET	FTM_NOTIFIER_create
(
	struct FTM_CATCHB_STRUCT _PTR_ pCatchB,
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
		goto finished;
	}

	strcpy(pNotifier->pName, __MODULE__);

	xRet = FTM_MSGQ_create(&pNotifier->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNotifier);
		ERROR(xRet, "Failed to create message queue!");
		goto finished;
    }

	pNotifier->pCatchB = pCatchB;
	pNotifier->bStop  = FTM_TRUE;

	*ppNotifier = pNotifier;	

finished:
	if (xRet != FTM_RET_OK)
	{
		if (pNotifier != NULL)
		{
			FTM_MEM_free(pNotifier);	
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

	FTM_NOTIFIER_stop((*ppNotifier));

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

FTM_RET	FTM_NOTIFIER_setConfig
(
	FTM_NOTIFIER_PTR	pNotifier,
	FTM_NOTIFIER_CONFIG_PTR	pConfig
)
{
	ASSERT(pNotifier != NULL);
	ASSERT(pConfig != NULL);
	
	FTM_RET	xRet = FTM_RET_OK;

	if (!pNotifier->bStop)
	{
		xRet = FTM_RET_ALREADY_RUNNING;
		ERROR(xRet, "Failed to set notifier configuration!");
	}
	else
	{
		memcpy(&pNotifier->xConfig, pConfig, sizeof(FTM_NOTIFIER_CONFIG));
	}

	return	xRet;
}

FTM_RET	FTM_NOTIFIER_getConfig
(
	FTM_NOTIFIER_PTR	pNotifier,
	FTM_NOTIFIER_CONFIG_PTR	pConfig
)
{
	ASSERT(pNotifier != NULL);
	ASSERT(pConfig != NULL);
	
	FTM_RET	xRet = FTM_RET_OK;

	memcpy(pConfig, &pNotifier->xConfig, sizeof(FTM_NOTIFIER_CONFIG));

	return	xRet;
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
		INFO("The notifier is already running!\n");	
		return	xRet;
	}

	if (pthread_create(&pNotifier->xThread, NULL, FTM_NOTIFIER_process, (FTM_VOID_PTR)pNotifier) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_FAILED;
		INFO("Failed to start notifier!");
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
		FTM_MSGQ_pushQuit(pNotifier->pMsgQ);

		pNotifier->bStop = FTM_TRUE;	
		pthread_join(pNotifier->xThread, NULL);

		pNotifier->xThread = 0;
	}

	return	xRet;
}

FTM_RET	FTM_NOTIIFER_sendSyslog
(
	FTM_INT			nLevel,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xOriginalStat,
	FTM_CCTV_STAT	xNewStat
)
{
	FTM_CHAR	pMessage[1024];

	switch(xNewStat)
	{
	case	FTM_CCTV_STAT_ABNORMAL:
		{
			sprintf(pMessage, "CCTV ID : %s, IP : %s, LOG : %s[%s -> %s]", pID, pIP,
					"SIGNATURE NOT MATCH CCTV",
					FTM_CCTV_STAT_print(xOriginalStat), 
					FTM_CCTV_STAT_print(xNewStat));
		}
		break;

	case	FTM_CCTV_STAT_NORMAL:
		{
			sprintf(pMessage, "CCTV ID : %s, IP : %s, LOG : %s[%s -> %s]", pID,  pIP,
					"NORMALITY CCTV",
					FTM_CCTV_STAT_print(xOriginalStat), 
					FTM_CCTV_STAT_print(xNewStat));
		}
		break;

	case	FTM_CCTV_STAT_UNUSED:
		{
			sprintf(pMessage, "CCTV ID : %s, IP : %s, LOG : %s[%s -> %s]", pID,  pIP,
					"NETWORK ERROR CCTV",
					FTM_CCTV_STAT_print(xOriginalStat), 
					FTM_CCTV_STAT_print(xNewStat));
		}
		break;

	default:
		{
			sprintf(pMessage, "CCTV ID : %s, IP : %s, LOG : STAT CHANGED[%s -> %s]", pID, pIP,
					FTM_CCTV_STAT_print(xOriginalStat), 
					FTM_CCTV_STAT_print(xNewStat));
		}
		break;
	}


	syslog(nLevel | LOG_LOCAL0, pMessage);
	INFO("SYSLOG : %s", pMessage);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_NOTIFIER_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_RET	xRet;
	FTM_NOTIFIER_PTR	pNotifier = (FTM_NOTIFIER_PTR)pData;

	INFO("%s started.", pNotifier->pName);

	pNotifier->bStop = FTM_FALSE;

    while(!pNotifier->bStop)
    {
		FTM_MSG_PTR	pRcvdMsg;

		xRet= FTM_MSGQ_timedPop(pNotifier->pMsgQ, 10, (FTM_VOID_PTR _PTR_)&pRcvdMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pRcvdMsg->xType)
			{
			case	FTM_MSG_TYPE_QUIT:
				{
					pNotifier->bStop = FTM_TRUE;
				}
				break;

			case	FTM_MSG_TYPE_SEND_ALARM:
				{
					FTM_MSG_SEND_ALARM_PTR	pMsg = (FTM_MSG_SEND_ALARM_PTR)pRcvdMsg;

					if (pNotifier->xConfig.xSyslog.bEnable)
					{
						switch(pNotifier->xConfig.xSyslog.xMode)
						{
						case	FTM_NOTIFIER_SYSLOG_MODE_CHANGED:
							{
								if (pMsg->xNewStat != pMsg->xOriginalStat)
								{
									FTM_INT	nLevel = LOG_INFO;

									if (pMsg->xLogType == FTM_LOG_TYPE_ERROR)
									{
										nLevel = LOG_ERR ;
									}

									FTM_NOTIIFER_sendSyslog(nLevel, pMsg->pID, pMsg->pIP, pMsg->xOriginalStat, pMsg->xNewStat);

								}
							}
							break;

						case	FTM_NOTIFIER_SYSLOG_MODE_LEVEL:
							{
								if (pNotifier->xConfig.xSyslog.xLogLevel >= pMsg->xLogType)
								{
									FTM_INT	nLevel = LOG_INFO;

									if (pMsg->xLogType == FTM_LOG_TYPE_ERROR)
									{
										nLevel = LOG_ERR ;
									}

									FTM_NOTIIFER_sendSyslog(nLevel, pMsg->pID, pMsg->pIP, pMsg->xOriginalStat, pMsg->xNewStat);
								}
							}
							}
							break;
						}

					if (pNotifier->xConfig.xSMTP.bEnable)
					{
						FTM_UINT32		ulAlarmCount = 0;
						FTM_ALARM_PTR	pAlarms = NULL;

						xRet = FTM_CATCHB_getAlarmCount(pNotifier->pCatchB, &ulAlarmCount);
						if (xRet != FTM_RET_OK)
						{
							ERROR(xRet, "Failed to get alarm count!");	
							break;
						}

						if (ulAlarmCount != 0)
						{
							pAlarms = (FTM_ALARM_PTR)FTM_MEM_calloc(sizeof(FTM_ALARM), ulAlarmCount);
							if (pAlarms == NULL)
							{
								xRet = FTM_RET_NOT_ENOUGH_MEMORY;
								ERROR(xRet, "Failed to alloc memory!\n");
								break;
							}

							xRet = FTM_CATCHB_getAlarmList(pNotifier->pCatchB, 0, ulAlarmCount, pAlarms, &ulAlarmCount);
							if (xRet == FTM_RET_OK)
							{
								FTM_UINT32	i;

								for(i = 0 ; i < ulAlarmCount ; i++)
								{
									FTM_NOTIFIER_onSendAlarm(pNotifier, pMsg->pID, &pAlarms[i]);
								}
							}

							FTM_MEM_free(pAlarms);
						}
					}
				}
				break;

				default:
				{
					INFO("Unknown command[%x]", pRcvdMsg->xType);
				}
			}

			FTM_MEM_free(pRcvdMsg);
		}
	}

	INFO("%s stopped.", pNotifier->pName);

	return	NULL;
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
	FTM_NOTIFIER_PTR 	pNotifier,
	FTM_LOG_TYPE		xLogType,
	FTM_CHAR_PTR	 	pID,
	FTM_CHAR_PTR	 	pIP,
	FTM_UINT32			ulTime,
	FTM_CCTV_STAT	 	xNewStat,
	FTM_CHAR_PTR		pHash,
	FTM_CCTV_STAT	 	xOriginalStat
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
		pMsg->xLogType = xLogType;
		strncpy(pMsg->pID, pID, sizeof(pMsg->pID) - 1);
		strncpy(pMsg->pIP, pIP, sizeof(pMsg->pIP) - 1);
		pMsg->ulTime = ulTime;
		pMsg->xNewStat = xNewStat;
		strncpy(pMsg->pHash, pHash, sizeof(pMsg->pHash) - 1);
		pMsg->xOriginalStat = xOriginalStat;

		xRet = FTM_NOTIFIER_sendMessage(pNotifier, (FTM_VOID_PTR)pMsg);
		if(xRet != FTM_RET_OK)
		{
			FTM_MEM_free(pMsg);
			ERROR(xRet, "Failed to send message!\n");
		}
	}
	return	xRet;
}

FTM_RET	FTM_NOTIFIER_onSendAlarm
(
	FTM_NOTIFIER_PTR	pNotifier,
	FTM_CHAR_PTR	pID,
	FTM_ALARM_PTR	pAlarm
)
{
	ASSERT(pAlarm != NULL);

	FTM_RET			xRet = FTM_RET_OK;
	FTM_SMTPC_PTR	pSMTPC = NULL;


	if (pNotifier->xConfig.xSMTP.bEnable)
	{
		FTM_CHAR		pBody[2048];
		FTM_UINT32		ulBodyLen = 0;

		INFO("##################################################");
		INFO("Send mail to %s", pAlarm->pEmail);
		ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "Date:%s\r\n", FTM_TIME_printfCurrent(NULL));
		ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "From:<%s>\r\n", pNotifier->xConfig.xSMTP.pSender);
		ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "To:<%s>\r\n", pAlarm->pEmail);
		ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "Subject:%s\r\n\r\n", "ALARM!");
		ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "%s\r\n\r\n", pAlarm->pMessage);


		xRet = FTM_SMTPC_create(pNotifier->xConfig.xSMTP.pServer, pNotifier->xConfig.xSMTP.usPort, &pSMTPC);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}

		xRet = FTM_SMTPC_connect(pSMTPC);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}

		xRet = FTM_SMTPC_sendGreeting(pSMTPC);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}

		INFO("AUTH %s %s", pNotifier->xConfig.xSMTP.pUserID, pNotifier->xConfig.xSMTP.pPasswd);
		xRet = FTM_SMTPC_sendAuth(pSMTPC, pNotifier->xConfig.xSMTP.pUserID, pNotifier->xConfig.xSMTP.pPasswd);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}

		xRet = FTM_SMTPC_sendFrom(pSMTPC, pNotifier->xConfig.xSMTP.pSender);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}

		xRet = FTM_SMTPC_sendTo(pSMTPC, pAlarm->pEmail);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}

		xRet = FTM_SMTPC_sendMessage(pSMTPC, pBody);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}

		xRet = FTM_SMTPC_disconnect(pSMTPC);
		if (xRet != FTM_RET_OK)
		{
			goto finished;
		}

	}

finished:
	if (pSMTPC != NULL)
	{
		FTM_SMTPC_destroy(&pSMTPC);	
	}

	return	xRet;
}

