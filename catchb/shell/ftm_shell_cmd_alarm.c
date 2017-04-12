#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"
#include "ftm_smtpc.h"

FTM_RET	FTM_SMTP_send
(
	FTM_CHAR_PTR	pServer,
	FTM_CHAR_PTR	pTo,
	FTM_CHAR_PTR	pFrom,
	FTM_CHAR_PTR	pSubject,
	FTM_CHAR_PTR	pMessage
);

FTM_RET	FTM_SHELL_CMD_testSendMail
(
	FTM_CHAR_PTR	pServer, 
	FTM_UINT16		usPort, 
	FTM_CHAR_PTR	pUserID,
	FTM_CHAR_PTR	pPasswd,
	FTM_CHAR_PTR	pTo, 
	FTM_CHAR_PTR	pFrom, 
	FTM_CHAR_PTR	pSubject, 
	FTM_CHAR_PTR	pMessage
);

FTM_RET	FTM_SHELL_CMD_showAlarmList
(
	FTM_CATCHB_PTR pCatchB
);

FTM_RET	FTM_SHELL_CMD_alarm
(
	FTM_SHELL_PTR pShell, 
	FTM_INT nArgc, 
	FTM_CHAR_PTR pArgv[], 
	FTM_VOID_PTR pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);
	ASSERT(pData != NULL);

	FTM_RET	xRet = FTM_RET_INVALID_ARGUMENTS;

	FTM_CATCHB_PTR	pCatchB = (FTM_CATCHB_PTR)pData;

	INFO("Call Alarm!");
	if ((nArgc == 1) || (strcasecmp(pArgv[1], "list") == 0))
	{
		xRet = FTM_SHELL_CMD_showAlarmList(pCatchB);
	}
	else  if (strcasecmp(pArgv[1], "add") == 0)
	{
		FTM_CHAR_PTR	pName = NULL;
		FTM_CHAR_PTR	pEmail = NULL;
		FTM_CHAR_PTR	pMessage = NULL;
		FTM_ALARM_PTR	pAlarm;

		if (nArgc < 4)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			printf("Error : Invalid arguments\n");	
		}
		else
		{
			if (nArgc == 5)
			{
				pMessage = pArgv[4];
			}

			pName = pArgv[2];
			pEmail = pArgv[3];

			xRet = FTM_CATCHB_addAlarm(pCatchB, pName, pEmail, pMessage, &pAlarm);
			if (xRet != FTM_RET_OK)
			{
				printf("Error : Failed to create alarm!");	
			}
		}
	}
	else  if (strcasecmp(pArgv[1], "del") == 0)
	{
		xRet = FTM_CATCHB_delAlarm(pCatchB, pArgv[2]);
		if (xRet != FTM_RET_OK)
		{
			printf("Failed to destroy switch[%s]!\n", pArgv[2]);	
		}
	}
	else  if (strcasecmp(pArgv[1], "count") == 0)
	{
		FTM_UINT32	ulCount = 0;

		xRet = FTM_CATCHB_getAlarmCount(pCatchB, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			printf("Failed to get alarm count!\n");
		}
		else
		{
			printf("Alarm Count : %d\n", ulCount);
		}
	}
	else  if (strcasecmp(pArgv[1], "get") == 0)
	{
		FTM_ALARM	xAlarm;

		xRet = FTM_CATCHB_getAlarm(pCatchB, pArgv[2], &xAlarm);
		if (xRet == FTM_RET_OK)
		{
			printf("%8s : %s\n", "Name", 	xAlarm.pName);
			printf("%8s : %s\n", "e-mail", 	xAlarm.pEmail);
			printf("%8s : %s\n", "Message", xAlarm.pMessage);
		}
		else
		{
			printf("Failed to get alarm[%s]!\n", pArgv[2]);	
		}
	}
	else  if (strcasecmp(pArgv[1], "set") == 0)
	{
		if ((nArgc < 5) || (nArgc % 2 != 1))
		{
			printf("Invalid arguments!");
		}
		else
		{
			FTM_ALARM	xAlarm;
			FTM_INT		i;
			FTM_UINT32	ulFieldFlags = 0;

			for(i = 3 ; i < nArgc ; i+=2)
			{
				if (strcasecmp(pArgv[i], "--email") == 0)
				{
					strncpy(xAlarm.pEmail, pArgv[i+1], sizeof(xAlarm.pEmail) - 1);
					ulFieldFlags |= FTM_ALARM_FIELD_EMAIL;
				}
				else if (ulFieldFlags & FTM_ALARM_FIELD_MESSAGE)
				{
					strncpy(xAlarm.pMessage, pArgv[i+1], sizeof(xAlarm.pMessage) - 1);
					ulFieldFlags |= FTM_ALARM_FIELD_MESSAGE;
				}
				else
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;	
					goto finished;
				}
			}

			xRet = FTM_CATCHB_setAlarm(pCatchB, pArgv[2], &xAlarm, ulFieldFlags);
			if (xRet != FTM_RET_OK)
			{
				printf("Error : Failed to set alarm[%s]\n", pArgv[2]);	
			}
		}
	}
	else if (strcasecmp(pArgv[1], "test") == 0)
	{
		if (nArgc < 10)
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			printf("Error : Invalid arguments\n");	
		}
		else
		{
			FTM_CHAR_PTR	pServer = pArgv[2];
			FTM_UINT16		usPort	= atoi(pArgv[3]);
			FTM_CHAR_PTR	pUserID	= pArgv[4];
			FTM_CHAR_PTR	pPasswd	= pArgv[5];
			FTM_CHAR_PTR	pTo 	= pArgv[6];
			FTM_CHAR_PTR	pFrom 	= pArgv[7];
			FTM_CHAR_PTR	pSubject= pArgv[8];
			FTM_CHAR_PTR	pMessage= pArgv[9];

			xRet = FTM_SHELL_CMD_testSendMail(pServer, usPort, pUserID, pPasswd, pTo, pFrom, pSubject, pMessage);
			if (xRet != FTM_RET_OK)
			{
				printf("Send mail test failed!\n");	
			}
		}
	}

finished:
	return	xRet;
}

FTM_RET	FTM_SHELL_CMD_showAlarmList
(
	FTM_CATCHB_PTR pCatchB
)
{
	FTM_RET	xRet = FTM_RET_OK;

	FTM_UINT32	i, ulCount = 0;
	FTM_ALARM_PTR	pAlarmList = NULL;

	xRet = FTM_CATCHB_getAlarmCount(pCatchB, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		printf("Failed to get switch count!\n");
		goto finished;
	}

	if (ulCount == 0)
	{
		printf("Registered switch does not exist.!\n");
		goto finished;
	}

	pAlarmList = (FTM_ALARM_PTR)FTM_MEM_calloc(sizeof(FTM_ALARM), ulCount);
	if (pAlarmList == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create switch buffer!");
		goto finished;
	}

	xRet = FTM_CATCHB_getAlarmList(pCatchB, 0, ulCount, pAlarmList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get switches!");
	}
	else
	{
		printf("%4s   %16s %24s %s\n", "", "Name", "e-mail", "Message");
		for(i = 0; i < ulCount ; i++)
		{
			printf("%4d : %16s %24s %s\n", i+1, pAlarmList[i].pName, pAlarmList[i].pEmail, pAlarmList[i].pMessage);
		}
	}

	FTM_MEM_free(pAlarmList);

finished:
	return	xRet;
}

FTM_RET	FTM_SHELL_CMD_testSendMail
(
	FTM_CHAR_PTR	pServer, 
	FTM_UINT16		usPort, 
	FTM_CHAR_PTR	pUserID,
	FTM_CHAR_PTR	pPasswd,
	FTM_CHAR_PTR	pTo, 
	FTM_CHAR_PTR	pFrom, 
	FTM_CHAR_PTR	pSubject, 
	FTM_CHAR_PTR	pMessage
)
{
	FTM_RET	xRet;
	FTM_SMTPC_PTR	pSMTPC;
	FTM_CHAR		pBody[2048];
	FTM_UINT32		ulBodyLen = 0;

	ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "Date:%s\r\n", FTM_TIME_printfCurrent(NULL));
	ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "From:<%s>\r\n", pFrom);
	ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "To:<%s>\r\n", pTo);
	ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "Subject:%s\r\n\r\n", pSubject);
	ulBodyLen += snprintf(&pBody[ulBodyLen], 2048 - ulBodyLen, "%s\r\n\r\n", pMessage);


	xRet = FTM_SMTPC_create(pServer, usPort, &pSMTPC);
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

	xRet = FTM_SMTPC_sendAuth(pSMTPC, pUserID, pPasswd);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_SMTPC_sendFrom(pSMTPC, pFrom);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_SMTPC_sendTo(pSMTPC, pTo);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_SMTPC_startBody(pSMTPC);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_SMTPC_sendRawBody(pSMTPC, pBody);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	xRet = FTM_SMTPC_endBody(pSMTPC);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	
	xRet = FTM_SMTPC_disconnect(pSMTPC);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

finished:
	if (pSMTPC != NULL)
	{
		FTM_SMTPC_destroy(&pSMTPC);	
	}

	return	xRet;

}
