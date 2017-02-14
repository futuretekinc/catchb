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


int main(int argc, char ** argv)
{
    char ch;
    int n, num_read = 0;
    int debug_mode = 0;
    int sockfd = 0, client_socket = 0;

    struct timeval tv;

    fd_set rfds;
    char app[255];

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
    if (check_pid(CK_NAME_CCTV_OPERATION) != 0) {
        return -1;
    }

    if (!debug_mode)
        daemon(0, 0);

    write_pid(CK_NAME_CCTV_OPERATION);

    if (access(SV_SOCK_CCTV_OPERATION_PATH, F_OK) == 0)
        unlink(SV_SOCK_CCTV_OPERATION_PATH);


    if(server_socket_create(&sockfd,(char*)SV_SOCK_CCTV_OPERATION_PATH)){
        cctv_system_error("cctv_operation/main() - server_socket_create : %s",strerror(errno));
        return 0;
    }

    log_message(CK_CCTV_OPERATION_LOG_FILE_PATH, "cctv_operation process start");


    memset(app, 0x00, sizeof(app));

    sprintf(app, "%s/%s", "/root/cctv_check/bin", "cctv_scheduler");
    system(app);

    while(1) 
    {

        FD_ZERO(&rfds);
        FD_SET(sockfd,&rfds);

        tv.tv_sec = 500;
        tv.tv_usec =0;

        if ((n = select(sockfd+1, &rfds, (fd_set*)0, (fd_set *)0, &tv)) < 0) {
            sleep(1);
            continue;

        }

        if ( n == 0)
        {
            memset(app, 0x00, sizeof(app));
            sleep(3);

            log_message(CK_CCTV_OPERATION_LOG_FILE_PATH, "signal analysis process time out");
            system("pkill cctv_ana");

            sleep(1);

            sprintf(app, "%s/%s", "/root/cctv_check/bin", "cctv_analysis");

            system(app);


        }
        else{
            if (FD_ISSET(sockfd, &rfds)) {
                client_socket= accept(sockfd, NULL, 0);
                if (client_socket == -1){
                    cctv_system_error("cctv_operation/main() -clinet not accept :%s",strerror(errno));
                    return 0;
                }
                while ((num_read = read(client_socket, &signal_info, sizeof(CK_SIGNAL_INFO))) > 0){
                    if (num_read == -1){
                        cctv_system_error("cctv_operation/main() - not read : %s",strerror(errno));
                    }else{

                        switch (signal_info.ck_event_division){

                            case 0:
                                break;

                            case 1:
                                log_message(CK_CCTV_OPERATION_LOG_FILE_PATH, "signal main engine");
                                memset(app, 0x00, sizeof(app));

                                system("pkill cctv_schedu");
                                sleep(1);
                                system("pkill cctv_ana");


                                sprintf(app, "%s/%s", "/root/cctv_check/bin", "cctv_scheduler");

                                system(app);
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
    }
    return 0;
}

