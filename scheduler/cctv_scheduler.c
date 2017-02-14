#include <common.h>
#include <linux_list.h>
#include <liboperation.h>
#include <libsignal.h>
#include <libutil.h>
#include <libsocket.h>
#include <liblogs.h>
#include <libscheduler.h>
#include <cctv_operation.h>
#include <syslog.h>
#include <semaphore.h>
#include <signal.h>
#include "trace.h"

extern char *program_invocation_short_name;
struct list_head check_ip_list_head;  //ip link list head

char source_ip[ADDR_LENG];

struct in_addr local_ip;
struct in_addr http_remote_ip;

sem_t sendthread, mainthread;

int main(int argc, char ** argv)
{
    char ch;
	int	i, n, num_read;
    int debug_mode = 0;
    int sockfd = 0, client_socket = 0;
    int t_num = 0;

    pthread_t thread[2];

    fd_set rfds;
    char app[255];

    struct timespec reqtime;
    reqtime.tv_sec = 0;
    reqtime.tv_nsec = 200000000;
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
    if (check_pid(CK_NAME_CCTV_SCHEDULER) != 0) 
	{
        return -1;
    }

    if (!debug_mode)
	{
        daemon(0, 0);
	}

    write_pid(CK_NAME_CCTV_SCHEDULER);

    if (access(SV_SOCK_CCTV_SCHEDULER_PATH, F_OK) == 0)
	{
        unlink(SV_SOCK_CCTV_SCHEDULER_PATH);
	}

    INIT_LIST_HEAD(&check_ip_list_head);

    if(server_socket_create(&sockfd,(char*)SV_SOCK_CCTV_SCHEDULER_PATH))
	{
        cctv_system_error("cctv_scheduler/main() - server_socket_create : %s",strerror(errno));
        return 0;
    }

    sem_init(&sendthread, 0, 0);
    sem_init(&mainthread, 0, 0);


    db_cctv_info_delete(); 

    log_message(CK_CCTV_SCHEDULER_LOG_FILE_PATH, "cctv_scheduler process start");

    get_local_ip();

    db_cctv_info_select(&t_num);

    memset(app, 0x00, sizeof(app));
    if(t_num)
	{
        pthread_create(&thread[0], NULL, thread_packet_send_operation, (void*)NULL);
        nanosleep(&reqtime, NULL);

        pthread_create(&thread[1], NULL, thread_main_operation, (void*)NULL);

        //nanosleep(&reqtime, NULL);
        for(i = 0; i< 2; i++)
		{
            pthread_join(thread[i], NULL);
        }

        sprintf(app, "%s/%s", "/home/xtra/catchb/analysis", "analysis");
        system(app);
    }

    while(1) 
    {

        FD_ZERO(&rfds);
        FD_SET(sockfd,&rfds);

        if ((n = select(sockfd+1, &rfds, NULL, NULL, NULL)) < 0) 
		{
            sleep(1);
            continue;
        }


        if (FD_ISSET(sockfd, &rfds)) 
		{
            client_socket= accept(sockfd, NULL, 0);
            if (client_socket == -1)
			{
                cctv_system_error("cctv_operation/main() -clinet not accept :%s",strerror(errno));
                return 0;
            }

            while ((num_read = read(client_socket, &signal_info, sizeof(CK_SIGNAL_INFO))) > 0){
                if (num_read == -1)
				{
                    cctv_system_error("cctv_operation/main() - not read : %s",strerror(errno));
                }else
				{

                    switch (signal_info.ck_event_division)
					{
                    case 0:

						sleep(3);

						system("pkill cctv_ana");

						memset(app, 0x00, sizeof(app));

						sleep(1);

        				sprintf(app, "%s/%s", "/home/xtra/catchb/analysis", "analysis");

						system(app);

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

    sem_destroy(&sendthread);
    sem_destroy(&mainthread);


    return 0;
}

