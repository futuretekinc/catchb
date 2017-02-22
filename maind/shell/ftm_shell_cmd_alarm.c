#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"

FTM_RET	FTM_SMTP_send
(
	FTM_CHAR_PTR	pServer,
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
	FTM_RET	xRet = FTM_RET_OK;

	FTM_CATCHB_PTR	pCatchB = (FTM_CATCHB_PTR)pData;

	switch(nArgc)
	{
	case	1:
		{
			xRet = FTM_SHELL_CMD_showAlarmList(pCatchB);
		}
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "del") == 0)
			{
				xRet = FTM_CATCHB_destroyAlarm(pCatchB, pArgv[2]);
				if (xRet != FTM_RET_OK)
				{
					printf("Failed to destroy switch[%s]!", pArgv[2]);	
				}
			}
			else
			{
				xRet = FTM_RET_INVALID_ARGUMENTS;
			}
		}
		break;

	case	5:
		{
			if (strcasecmp(pArgv[1], "add") == 0)
			{
				FTM_ALARM_PTR	pAlarm;

				xRet = FTM_CATCHB_createAlarm(pCatchB, pArgv[2], pArgv[3], pArgv[4], &pAlarm);
				if (xRet != FTM_RET_OK)
				{
					printf("Alarm create failed !");	
				}
			}
			else
			{
				xRet = FTM_RET_INVALID_ARGUMENTS;
			}
	
		}
		break;

	case	7:
		{
			if (strcasecmp(pArgv[1], "test") == 0)
			{
				FTM_CHAR_PTR	pServer = pArgv[2];
				FTM_CHAR_PTR	pTo 	= pArgv[3];
				FTM_CHAR_PTR	pFrom 	= pArgv[4];
				FTM_CHAR_PTR	pSubject= pArgv[5];
				FTM_CHAR_PTR	pMessage= pArgv[6];

				xRet = FTM_SMTP_send(pServer, pTo, pFrom, pSubject, pMessage);
				if (xRet != FTM_RET_OK)
				{
					printf("Send mail test failed!\n");	
				}
			}
			else
			{
				xRet = FTM_RET_INVALID_ARGUMENTS;
			}
		}
		break;

	default:
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
		}
		break;
	}

	return	xRet;
}

FTM_RET	FTM_SHELL_CMD_showAlarmList
(
	FTM_CATCHB_PTR pCatchB
)
{
	FTM_RET	xRet = FTM_RET_OK;

	FTM_UINT32	i, ulCount = 0;
	FTM_ALARM_PTR	_PTR_ ppAlarmes = NULL;

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

	ppAlarmes = (FTM_ALARM_PTR _PTR_)FTM_MEM_calloc(sizeof(FTM_ALARM_PTR), ulCount);
	if (ppAlarmes == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create switch buffer!");
		goto finished;
	}

	xRet = FTM_CATCHB_getAlarmList(pCatchB, ppAlarmes, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Failed to get switches!");
	}
	else
	{
		printf("%4s   %16s %24s %s\n", "", "Name", "e-mail", "Message");
		for(i = 0; i < ulCount ; i++)
		{
			printf("%4d : %16s %24s %s\n", i+1, ppAlarmes[i]->pName, ppAlarmes[i]->pEmail, ppAlarmes[i]->pMessage);
		}
	}

	FTM_MEM_free(ppAlarmes);

finished:
	return	xRet;
}

