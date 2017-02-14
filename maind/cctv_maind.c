#include <cctv_maind.h>
#include <common_libssh.h>

int main(int argc, char *argv[])
{
    int rt;
    int i;
    time_t  timer;
    struct tm *t;
    char buffer[255];
    char app[255];
    int debug_mode = 0;
    char ch;
    int current_day = 0;
    int current_hour = 0;
    int current_min = 0;
    char check_process_name[5][256] ={CK_NAME_CCTV_ALARM,CK_NAME_CCTV_LINK_SLOG, CK_NAME_CCTV_DETECTION,CK_NAME_CCTV_SCHEDULER, CK_NAME_CCTV_OPERATION};
    char s_process_name[5][256] ={CK_NAME_CCTV_ALARM,CK_NAME_CCTV_LINK_SLOG, CK_NAME_CCTV_DETECTION, CK_NAME_CCTV_SCHEDULER, CK_NAME_CCTV_OPERATION};



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


    if (check_pid(CK_NAME_CCTV_MAIND) != 0) {
        printf("%s is already running!!\n", CK_NAME_CCTV_MAIND);
        return -1;
    }


    if (!debug_mode)
        daemon(0, 0);


    write_pid(CK_NAME_CCTV_MAIND);

    

    log_message(CK_CCTV_MAIND_LOG_FILE_PATH, "cctv_maind daemon start");

    create_db();

    initdb();



    while(1)
    {

        timer = time(NULL);

        t = localtime(&timer);


        current_day = t->tm_mday;

        current_hour = t->tm_hour;

        current_min = t->tm_min;

        
        if(current_day == 1){

            if (current_hour == 1){

                if(current_min == 1){

                    log_message(CK_CCTV_MAIND_LOG_FILE_PATH, "cctv_maind db 2 months ago delete");
                    db_cctv_log_check_delete();
                    
                    
                }
            }
        }


        for(i =0; i< 6; i++)
        {

            memset(app, 0x00, sizeof(app));
            sprintf(app, "%s/%s", APP_PATH, s_process_name[i]);
            rt = check_process(check_process_name[i]);

            if(rt == 0)  //죽었을 시
            {
                // 새로 뛰움
                log_message(CK_CCTV_MAIND_LOG_FILE_PATH, "cctv_maind process restart :%s",app);
                system(app);
            }


        }
        system("rm -rf /var/log/*.1");
        // 검사 후, process sleep 
        sleep(CHECK_SECOND);    
    }

    return 0;
}

