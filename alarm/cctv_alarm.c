#include <common.h>
#include <libalarm.h>
#include <cctv_alarm.h>
#include <libalarm.h>
#include <liblogs.h>
#include <libsignal.h>
#include <libsocket.h>
#include <libutil.h>
#include <syslog.h>
#include "trace.h"

int main(int argc, char *argv[])
{
    char    ch;
    int     debug_mode = 0;

    int sockfd = 0, client_socket = 0;

    int n, num_read = 0;
    fd_set rfds;
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

    if (check_pid(CK_NAME_CCTV_ALARM) != 0) 
	{
        ERROR(-1, "%s is already running!!\n", CK_NAME_CCTV_ALARM);

        return -1;
    }

    if (!debug_mode)
	{
        daemon(0, 0);
	}

    write_pid(CK_NAME_CCTV_ALARM);

    if (access(SV_SOCK_CCTV_ALARM_PATH, F_OK) == 0)
	{
        unlink(SV_SOCK_CCTV_ALARM_PATH);
	}

    if(server_socket_create(&sockfd,(char*)SV_SOCK_CCTV_ALARM_PATH))
	{
        cctv_system_error("cctv_alarm/main() - server_socket_create : %s", strerror(errno));
        return 0;
    }

    log_message(CK_CCTV_ALARM_LOG_FILE_PATH, "cctv_alarm daemon start");

    while(1)
    {

        FD_ZERO(&rfds);
        FD_SET(sockfd,&rfds);
        if ((n = select(sockfd+1, &rfds, NULL, NULL, NULL)) > 0) 
		{
			if (FD_ISSET(sockfd, &rfds)) 
			{
				client_socket= accept(sockfd, NULL, 0);
				if (client_socket == -1)
				{
					cctv_system_error("cctv_alarm/main() -clinet not accept :%s", strerror(errno));
					return 0;
				}
				while ((num_read = read(client_socket, &signal_info, sizeof(CK_SIGNAL_INFO))) > 0)
				{
					if (num_read == -1)
					{
						cctv_system_error("cctv_alarm/main() - not read : %s",    strerror(errno));
					}else
					{

						switch (signal_info.ck_event_division)
						{
						case 0:
							log_message(CK_CCTV_ALARM_LOG_FILE_PATH, "signal mail server ip change");
							db_cctv_info_select(&signal_info);
							break;
						case 1:
							break;
						case 2:
							break;
						default:
							break;
						}
					}
				}
				close(client_socket);
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


    return 0;
}

