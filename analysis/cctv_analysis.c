#include <common.h>
#include <linux_list.h>
#include <liboperation.h>
#include <libsignal.h>
#include <libutil.h>
#include <cctv_analysis.h>
#include <syslog.h>
#include <semaphore.h>

struct list_head check_ip_list_head;  //ip link list head

char source_ip[ADDR_LENG];
struct in_addr local_ip;
struct in_addr http_remote_ip;

sem_t sendthread, mainthread;
int main(int argc, char ** argv)
{
    char ch;
    int i;
    int debug_mode = 0;
    int t_num = 0;

    CK_SIGNAL_INFO signal_info;
    pthread_t thread[2];

    struct timespec reqtime;
    reqtime.tv_sec = 0;
    reqtime.tv_nsec = 200000000;


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

    if (check_pid(CK_NAME_CCTV_ANALYSIS) != 0) {
        printf("%s is already running!!\n", CK_NAME_CCTV_ANALYSIS);
        return -1;
    }

    if (!debug_mode)
        daemon(0, 0);

    write_pid(CK_NAME_CCTV_ANALYSIS);

    if (access(SV_SOCK_CCTV_ANALYSIS_PATH, F_OK) == 0)
        unlink(SV_SOCK_CCTV_ANALYSIS_PATH);


    signal_info.ck_event_division = 0;
    ck_signal(SV_SOCK_CCTV_OPERATION_PATH,&signal_info);

    INIT_LIST_HEAD(&check_ip_list_head);

    sem_init(&sendthread, 0, 0);
    sem_init(&mainthread, 0, 0);


   
    get_local_ip();

    db_cctv_info_select(&t_num);


        pthread_create(&thread[0], NULL, thread_packet_send_operation, (void*)NULL);
        nanosleep(&reqtime, NULL);
        pthread_create(&thread[1], NULL, thread_main_operation, NULL);
        nanosleep(&reqtime, NULL);
        for( i = 0; i< 2; i++){
            pthread_join(thread[i], NULL);
        }



    signal_info.ck_event_division = 0;

    ck_signal(SV_SOCK_CCTV_OPERATION_PATH,&signal_info);

    sleep(1);
    ck_signal(SV_SOCK_CCTV_SCHEDULER_PATH,&signal_info);

    sem_destroy(&sendthread);
    sem_destroy(&mainthread);

    while(1){
        sleep(10);
    }
    
    return 0;

}

