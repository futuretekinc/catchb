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

int main(int argc, char *argv[])
{

    int sockfd = 0, client_socket = 0;
    char ch;
    int debug_mode = 0;
    int n, num_read = 0;
    CK_SIGNAL_INFO signal_info;

    fd_set rfds;


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


    if (check_pid(CK_NAME_CCTV_LINK_SLOG) != 0) {
        return -1;
    }

    if (!debug_mode)
        daemon(0, 0);

    write_pid(CK_NAME_CCTV_LINK_SLOG);

    if (access(SV_SOCK_CCTV_LINK_SLOG_PATH, F_OK) == 0)
        unlink(SV_SOCK_CCTV_LINK_SLOG_PATH);


    if(server_socket_create(&sockfd,(char*)SV_SOCK_CCTV_LINK_SLOG_PATH)){
        //에러 부분 확인
        return 0;
    }


    log_message(CK_CCTV_LINK_SLOG_LOG_FILE_PATH,"cctv_link_slog daemon start");

    syslog_init();

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
                cctv_system_error("cctv_link_slog/main() -clinet not accept :%s", strerror(errno));
                return 0;
            }
            while ((num_read = read(client_socket, &signal_info, sizeof(CK_SIGNAL_INFO))) > 0){
                if (num_read == -1){
                    cctv_system_error("cctv_link_slog/main() - not read : %s",    strerror(errno));
                }else{

                    switch(signal_info.ck_event_division){
                        case 0:
                            break;
                        case 1:
                        log_message(CK_CCTV_LINK_SLOG_LOG_FILE_PATH, "signal syslog server ip change");
                            syslog_init(); 
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

