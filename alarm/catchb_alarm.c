#include <common.h>
#include <libalarm.h>
#include <cctv_alarm.h>
#include <libalarm.h>
#include <liblogs.h>
#include <libsignal.h>
#include <libsocket.h>
#include <libutil.h>
#include <syslog.h>
#include "catchb_trace.h"
#include "catchb_alarm.h"
#include "catchb_db.h"

extern	CATCHB_CHAR_PTR	program_invocation_short_name;

CATCHB_RET	CATCHB_ALARM_create
(
	CATCHB_ALARM_PTR _PTR_ ppAlarm
)
{
	ASSERT(ppAlarm != NULL);
	CATCHB_RET	xRet = CATCHB_RET_OK;
	CATCHB_ALARM_PTR	pAlarm;

	pAlarm = (CATCHB_ALARM_PTR)malloc(sizeof(CATCHB_ALARM));
	if (pAlarm == NULL)
	{
		xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Failed to create alarm!\n");	
	}
	else
	{
		xRet = CATCHB_SERVER_create(&pAlarm->pServer);
		if (xRet != CATCHB_RET_OK)
		{
			free(pAlarm);
			ERROR(xRet, "Failed to create server");
    	}
		else
		{
			*ppAlarm = pAlarm;	
		}
	}

	return	xRet;

}

CATCHB_RET	CATCHB_ALARM_destroy
(
	CATCHB_ALARM_PTR _PTR_ ppAlarm
)
{
	ASSERT(ppAlarm != NULL);
	ASSERT(*ppAlarm != NULL);

	CATCHB_RET	xRet = CATCHB_RET_OK;

	if ((*ppAlarm)->pServer != NULL)
	{
		xRet = CATCHB_SERVER_destroy(&(*ppAlarm)->pServer);
		if (xRet != CATCHB_RET_OK)
		{
			ERROR(xRet, "Failed to destroy server!\n");
		}
	}

	free(*ppAlarm);

	*ppAlarm = NULL;

	return	CATCHB_RET_OK;
}


CATCHB_RET	CATCHB_ALARM_run
(
	CATCHB_ALARM_PTR	pAlarm
)
{
	ASSERT(pAlarm != NULL);

	CATCHB_RET	xRet;

	xRet = CATCHB_SERVER_open(pAlarm->pServer, SV_SOCK_CCTV_ALARM_PATH);
	if (xRet != CATCHB_RET_OK)
	{
        ERROR(xRet, "Failed to open server");
		CATCHB_SERVER_destroy(&pAlarm->pServer);
        return 0;
    }

	pAlarm->bStop = CATCHB_FALSE;
    LOG("%s started.", program_invocation_short_name);

    while(!pAlarm->bStop)
    {
    	fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(pAlarm->pServer->xSocket, &rfds);

        if (select(pAlarm->pServer->xSocket + 1, &rfds, NULL, NULL, NULL) > 0) 
		{
			if (FD_ISSET(pAlarm->pServer->xSocket, &rfds)) 
			{
				CATCHB_INT		nRead = 0;
    			CATCHB_SOCKET	xClientSocket;
    			CK_SIGNAL_INFO 	xSignalInfo;

				xClientSocket= accept(pAlarm->pServer->xSocket, NULL, 0);
				if (xClientSocket == -1)
				{
					xRet = CATCHB_RET_SOCKET_ACCEPT_FAILED;
					ERROR(xRet, "Failed to accept socket!");
					goto finished;
				}

				while (!pAlarm->bStop)
				{	
					nRead = read(xClientSocket, &xSignalInfo, sizeof(CK_SIGNAL_INFO));
					if (nRead < 0)
					{
						xRet = CATCHB_RET_SOCKET_ABNORMAL_DISCONNECTED;
						ERROR(xRet, "Socket is abnormal disconnected!\n");
						break;
					}
					if (nRead == 0)
					{
						xRet = CATCHB_RET_OK;
						TRACE("Socket is disconnected!\n");
						break;
					}
					else
					{

						if (xSignalInfo.ck_event_division == 0)
						{
							CATCHB_DB_PTR	pDB;

							xRet = CATCHB_DB_create(&pDB);
							if (xRet == CATCHB_RET_OK)
							{
								xRet = CATCHB_DB_open(pDB, "/tmp/catchb.db");
								if (xRet == CATCHB_RET_OK)
								{
									CATCHB_UINT32	ulCount = 0;
									xRet = CATCHB_DB_getAlarmMailInfoCount(pDB, &ulCount);
									if (xRet == CATCHB_RET_OK)
									{
										if (ulCount != 0)
										{
											CATCHB_ALARM_MAIL_INFO_PTR	pAlarmMailInfo;

											pAlarmMailInfo = (CATCHB_ALARM_MAIL_INFO_PTR)malloc(sizeof(CATCHB_ALARM_MAIL_INFO) * ulCount);
											if (pAlarmMailInfo != NULL)
											{
												xRet = CATCHB_DB_getAlarmMailInfoList(pDB, pAlarmMailInfo, ulCount, &ulCount);
												if (xRet == CATCHB_RET_OK)
												{	
													CATCHB_UINT32	i;

													for(i = 0 ; i < ulCount ; i++)
													{
														TRACE("%4d : %s\n", i+1, pAlarmMailInfo[i].pMail);	
													}
												}

												free(pAlarmMailInfo);
											}
											else
											{
												xRet = CATCHB_RET_NOT_ENOUGH_MEMORY;	
											}
										}
									}
								}

								CATCHB_DB_destroy(&pDB);
							}
						}
					}
				}

				close(xClientSocket);
			}
		}
		else
		{
            if (errno != EINTR)
			{
            	sleep(1);
			}
        }
    }


finished:

    return 0;
}

