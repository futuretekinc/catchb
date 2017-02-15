#include <common.h>
#include <linux_list.h>
#include <liboperation.h>
#include <libsignal.h>
#include <libsocket.h>
#include <libutil.h>
#include <liblogs.h>
#include <cctv_operation.h>
#include <syslog.h>
#include <semaphore.h>
#include <signal.h>
#include "catchb_trace.h"
#include "catchb_server.h"

int main
(
	CATCHB_INT	nArgc, 
	CATCHB_CHAR_PTR	_PTR_ ppArgv
)
{
	CATCHB_RET		xRet;
	CATCHB_SERVER_PTR	pServer = NULL;

    char ch;
    int debug_mode = 0;

    char app[255];

    CK_SIGNAL_INFO signal_info;


    while ((ch = getopt(nArgc, ppArgv, "adh")) != -1)
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
    }

    if (check_pid(CK_NAME_CCTV_OPERATION) != 0) 
	{
        return -1;
    }

    if (!debug_mode)
        daemon(0, 0);

    write_pid(CK_NAME_CCTV_OPERATION);


	xRet = CATCHB_SERVER_create(&pServer);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to create server!\n");
		goto finished;
	}

	xRet = CATCHB_SERVER_open(pServer, SV_SOCK_CCTV_OPERATION_PATH);
    if(xRet != CATCHB_RET_OK)
	{
		CATCHB_SERVER_destroy(&pServer);
        ERROR(xRet, "Failed to open server!");
		goto finished;
    }

    LOG("cctv_operation process start");


    memset(app, 0x00, sizeof(app));

    sprintf(app, "%s/%s", "/root/cctv_check/bin", "cctv_scheduler");
    system(app);

    while(CATCHB_TRUE) 
    {
		CATCHB_INT	n;
		fd_set	xRFDS;
    	struct timeval xTimeVal;

        FD_ZERO(&xRFDS);
        FD_SET(pServer->xSocket,&xRFDS);

        xTimeVal.tv_sec = 500;
        xTimeVal.tv_usec =0;

        n = select(pServer->xSocket+1, &xRFDS, (fd_set*)0, (fd_set *)0, &xTimeVal);
        if (n < 0) 
		{
            sleep(1);
            continue;
        }
		else if ( n == 0)
        {
            memset(app, 0x00, sizeof(app));
            sleep(3);

            log_message(CK_CCTV_OPERATION_LOG_FILE_PATH, "signal analysis process time out");
            system("pkill cctv_ana");

            sleep(1);

            sprintf(app, "%s/%s", "/root/cctv_check/bin", "cctv_analysis");

            system(app);


        }
        else
		{
            if (FD_ISSET(pServer->xSocket, &xRFDS)) 
			{
				CATCHB_INT		nRead;
				CATCHB_SOCKET	xClientSocket;
                xClientSocket= accept(pServer->xSocket, NULL, 0);
                if (xClientSocket == -1)
				{
                    cctv_system_error("cctv_operation/main() -clinet not accept :%s",strerror(errno));
                    return 0;
                }

                while (CATCHB_TRUE)
				{
                	nRead = read(xClientSocket, &signal_info, sizeof(CK_SIGNAL_INFO));
					if (nRead < 0)
					{
						xRet = CATCHB_RET_SOCKET_ABNORMAL_DISCONNECTED;
						ERROR(xRet, "Socket is abnormal disconnected!");
						break;
					}
					else if (nRead == 0)
					{
						xRet = CATCHB_RET_OK;
						ERROR(xRet, "Socket is disconnected!");
						break;
					}
					else
					{
						if (signal_info.ck_event_division == 1)
						{
							log_message(CK_CCTV_OPERATION_LOG_FILE_PATH, "signal main engine");
							memset(app, 0x00, sizeof(app));

							system("pkill cctv_schedu");
							sleep(1);
							system("pkill cctv_ana");


							sprintf(app, "%s/%s", "/root/cctv_check/bin", "cctv_scheduler");

							system(app);
						}
					}
				}

				close(xClientSocket);
			}
		}
    }

finished:
	if (pServer != NULL)
	{
		CATCHB_SERVER_destroy(&pServer);
	}

    return 0;
}

