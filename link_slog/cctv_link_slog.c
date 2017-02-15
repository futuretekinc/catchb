#include <common.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <cctv_link_slog.h>
#include <liblogs.h>
#include <libsignal.h>
#include <libsocket.h>
#include <libutil.h>
#include <syslog.h>
#include "catchb_trace.h"
#include "catchb_server.h"

extern	CATCHB_CHAR_PTR program_invocation_short_name;

int main
(
	CATCHB_INT		nArgc, 
	CATCHB_CHAR_PTR	ppArgv[]
)
{
	CATCHB_RET	xRet;
	CATCHB_SERVER_PTR	pServer = NULL;

    char ch;
    CATCHB_BOOL	bDebugMode = CATCHB_FALSE;
    CK_SIGNAL_INFO xSignalInfo;

    while ((ch = getopt(nArgc, ppArgv, "adh")) != -1)
    {
        switch (ch)
        {
            case 'a':
                break;

            case 'd':
                bDebugMode = 1;
                break;

            case 'h':
                break;

            default:
                break;
        }
      break;
    }


    if (check_pid(CK_NAME_CCTV_LINK_SLOG) != 0) 
	{
        return -1;
    }

    write_pid(CK_NAME_CCTV_LINK_SLOG);

    if (!bDebugMode)
	{
        daemon(0, 0);
	}

	xRet = CATCHB_SERVER_create(&pServer);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to create server!\n");
		goto finished;
	}

	xRet = CATCHB_SERVER_open(pServer, SV_SOCK_CCTV_LINK_SLOG_PATH);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to open server!\n");
        goto finished;
    }


    LOG("%s started.", program_invocation_short_name);

    syslog_init();

    while(CATCHB_TRUE)
    {
		CATCHB_INT	nFD;
		fd_set	xRFDS;

        FD_ZERO(&xRFDS);
        FD_SET(pServer->xSocket, &xRFDS);

        nFD = select(pServer->xSocket+1, &xRFDS, NULL, NULL, NULL);
		if (nFD > 0)
		{
			if (FD_ISSET(pServer->xSocket, &xRFDS)) 
			{
				CATCHB_SOCKET	xClientSocket= accept(pServer->xSocket, NULL, 0);
				if (xClientSocket < 0)
				{
					xRet = CATCHB_RET_SOCKET_ACCEPT_FAILED;
					ERROR(xRet, "Failed to accept socket!");
					goto finished;
				}

				while (CATCHB_TRUE)
				{
					CATCHB_INT	nRead;

					nRead = read(xClientSocket, &xSignalInfo, sizeof(CK_SIGNAL_INFO));
					if (nRead < 0)
					{
						xRet = CATCHB_RET_SOCKET_ABNORMAL_DISCONNECTED;
						ERROR(xRet, "Socket is abnormal disconnected.");
						break;
					}
					if (nRead == 0)
					{
						xRet = CATCHB_RET_OK;
						TRACE("Socket is disconnected.");
						break;
					}
					else
					{

						if (xSignalInfo.ck_event_division == 1)
						{
							LOG("signal syslog server ip changed.");
							syslog_init(); 
						}
					}
				}

				close(xClientSocket);
			}
		}
		else if (nFD < 0)
		{
			sleep(1);
		}
	}

finished:
	if (pServer != NULL)
	{
		CATCHB_SERVER_destroy(&pServer);	
	}

    return 0;
}

