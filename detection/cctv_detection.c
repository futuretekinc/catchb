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

//#include <common_libssh.h>

int main(int argc, char *argv[])
{
    char    ch;
    int     debug_mode = 0;
    int 	sockfd = 0, client_socket = 0;
    int 	n, num_read = 0;
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

    if (check_pid(CK_NAME_CCTV_DETECTION) != 0) {
        printf("%s is already running!!\n", CK_NAME_CCTV_DETECTION);
        return -1;
    }

    if (!debug_mode)
        daemon(0, 0);

    write_pid(CK_NAME_CCTV_DETECTION);

    if (access(SV_SOCK_CCTV_DETECTION_PATH, F_OK) == 0)
        unlink(SV_SOCK_CCTV_DETECTION_PATH);

    if(server_socket_create(&sockfd,(char*)SV_SOCK_CCTV_DETECTION_PATH))
	{
        cctv_system_error("cctv_detection/main() - server_socket_create : %s", strerror(errno));
        return 0;
    }

    log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection daemon start");



    
    while(1)
    {

        FD_ZERO(&rfds);
        FD_SET(sockfd,&rfds);
        if ((n = select(sockfd+1, &rfds, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR) continue;
            sleep(1);
            continue;
        }
        if (FD_ISSET(sockfd, &rfds)) {
            client_socket= accept(sockfd, NULL, 0);
            if (client_socket == -1){
                cctv_system_error("cctv_detection/main() -clinet not accept :%s", strerror(errno));
                return 0;
            }
            while ((num_read = read(client_socket, &signal_info, sizeof(CK_SIGNAL_INFO))) > 0){
                if (num_read == -1){
                    cctv_system_error("cctv_detection/main() - not read : %s",    strerror(errno));
                }else{
                  
                    switch (signal_info.ck_event_division){
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

            }
            close(client_socket);
        }
    }
    
    return 0;
}

