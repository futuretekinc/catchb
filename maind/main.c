#include <cctv_maind.h>
#include <common_libssh.h>
#include "libutil.h"
#include "liblogs.h"
#include "catchb_trace.h"
#include "catchb_config.h"

extern	CATCHB_CHAR_PTR	program_invocation_short_name;

int main(int argc, char *argv[])
{
	CATCHB_RET	xRet;
    int rt;
    int i;
    time_t  timer;
    struct tm *t;
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

	CATCHB_CONFIG_PTR	pConfig;

	TRACE_ENTRY();
	xRet = CATCHB_CONFIG_create(&pConfig);
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to create config!\n");
		return	0;	
	}

	TRACE_ENTRY();
	xRet = CATCHB_CONFIG_load(pConfig, "./catchb.conf");
	if (xRet != CATCHB_RET_OK)
	{
		ERROR(xRet, "Failed to load config!\n");
		return	0;
	}

	TRACE_ENTRY();
	CATCHB_CONFIG_show(pConfig);

	TRACE_ENTRY();

    if (check_pid(program_invocation_short_name) != 0) 
	{
		xRet = CATCHB_RET_ALREADY_RUNNING;
        ERROR(xRet ,"%s is already running!!\n", program_invocation_short_name);
		goto finished;
    }

    write_pid(program_invocation_short_name);


    if (!debug_mode)
	{
        daemon(0, 0);
	}

    LOG("cctv_maind daemon start");

    create_db();

    initdb();

    while(1)
    {

        timer = time(NULL);

        t = localtime(&timer);

        current_day = t->tm_mday;

        current_hour = t->tm_hour;

        current_min = t->tm_min;

        
        if(current_day == 1)
		{
            if (current_hour == 1)
			{
                if(current_min == 1)
				{
                    LOG("cctv_maind db 2 months ago delete");
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

finished:
	if (pConfig != NULL)
	{
		CATCHB_CONFIG_destroy(&pConfig);
	}
	
    return 0;
}

