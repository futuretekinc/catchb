#include <common.h>
#include <libalarm.h>
#include <libsmtpc.h>
#include <cctv_alarm.h>
#include <liblogs.h>
#include <libutil.h>
#include <libsignal.h>
#include <libsocket.h>
#include <libdetection.h>
#include <syslog.h>
#include "catchb_trace.h"
#include "catchb_server.h"

//#include <common_libssh.h>

int main(int argc, char *argv[])
{
	CATCHB_RET	xRet;
	CATCHB_SERVER_PTR	pServer = NULL;

    char    ch;
    int     debug_mode = 0;
    CK_SIGNAL_INFO signal_info;


    while ((ch = getopt(argc, argv, "adh")) != -1)
    {
        switch (ch)
        {
            case 'a':
                break;

            case 'd':
                debug_mode = 1;
                break;

            case 'h':
                break;

            default:
                break;
        }
     break;
    }

    if (check_pid(CK_NAME_CCTV_DETECTION) != 0) {
        printf("%s is already running!!\n", CK_NAME_CCTV_DETECTION);
        return -1;
    }

    if (!debug_mode)
        daemon(0, 0);

    write_pid(CK_NAME_CCTV_DETECTION);

	xRet = CATCHB_SERVER_create(&pServer);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to create server!\n");
		return	0;
	}

	xRet = CATCHB_SERVER_open(pServer, SV_SOCK_CCTV_DETECTION_PATH);
    if(xRet == CATCHB_RET_OK)
	{
		LOG("%s started.");

		while(1)
		{
			fd_set	xRFDS;

			FD_ZERO(&xRFDS);
			FD_SET(pServer->xSocket, &xRFDS);
			if (select(pServer->xSocket + 1, &xRFDS, NULL, NULL, NULL) >= 0) 
			{
				if (FD_ISSET(pServer->xSocket, &xRFDS)) 
				{
					CATCHB_SOCKET	xClientSocket;

					xClientSocket = accept(pServer->xSocket, NULL, 0);
					if (xClientSocket >= 0)
					{
						CATCHB_INT	nRead;

						while ((nRead = read(xClientSocket, &signal_info, sizeof(CK_SIGNAL_INFO))) > 0)
						{
							switch (signal_info.ck_event_division)
							{
							case 0:
								detection_process(&signal_info);
								break;
							case 1:
								break;
							case 2:
								break;
							default:
								break;
							}
						}

						close(xClientSocket);

						if (nRead < -1)
						{
							xRet = CATCHB_RET_SOCKET_ABNORMAL_DISCONNECTED;
							ERROR(xRet, "Socket is abnormal disconnected!");
						}
					}
					else
					{
						xRet = CATCHB_RET_SOCKET_ACCEPT_FAILED;

						ERROR(xRet, "Failed to accept socket!");
						break;
					}

				}
			}
			else if (errno != EINTR) 
			{
				sleep(1);
			}
		}
	}
	else
	{
		ERROR(xRet, "Failed to open server");
	}

	if (pServer!= NULL)
	{
		CATCHB_SERVER_destroy(&pServer);	
	}

    return 0;
}

