#include <unistd.h>
#include <common.h>
#include <libalarm.h>
#include <liblogs.h>
#include <libsignal.h>
#include <libsocket.h>
#include <libutil.h>
#include <syslog.h>
#include "catchb_alarm.h"
#include "catchb_trace.h"

extern	CATCHB_CHAR_PTR	program_invocation_short_name;

int main
(
	int argc, 
	char *argv[]
)
{
	CATCHB_RET	xRet;
	CATCHB_ALARM_PTR	pAlarm;

    char    ch;
    int     debug_mode = 0;



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

    write_pid(CK_NAME_CCTV_ALARM);

    if (!debug_mode)
	{
        daemon(0, 0);
	}

	xRet = CATCHB_ALARM_create(&pAlarm);
	if (xRet != CATCHB_RET_OK)
	{
        ERROR(xRet, "Failed to create alarm");
		goto finished;
    }

	CATCHB_ALARM_run(pAlarm);

finished:

    return 0;
}

