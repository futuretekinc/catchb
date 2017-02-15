#include <common.h>
#include <libalarm.h>
#include <liblogs.h>
//#include <libsmtpc.h>
#include <libsignal.h>
#include <syslog.h>
#include "mysql/mysql.h"

int sendalarmmail(char *mail_address, char *message)
{
#if 0
    struct smtpclient *smtpc;
    int ret;

    char body[1024];

    memset(body, 0x00, sizeof(body));

    /* Mail Send Start */
    smtpc = smtpc_new_client("localhost", 25);

    if(!smtpc)
        return -1;

    ret = smtpc_connect(smtpc);

    // MAIL FROM
    ret = smtpc_send_from(smtpc, "support@CatchB.co.kr");

    // RCPT TO
    ret = smtpc_send_to(smtpc, mail_address);

    // DATA
    ret = smtpc_start_body(smtpc);

    sprintf(body, "From:CatchB<support@CatchB.co.kr>\nTo:%s<%s>\nSubject:[CatchB] 알람\n%s", "관리자", mail_address,message);

    ret = smtpc_send_raw_body(smtpc, body);

    ret = smtpc_end_body(smtpc);

    // QUIT
    smtpc_quit(smtpc);

    smtpc_close(smtpc);
    smtpc_free_client(smtpc);

#endif
    log_message(CK_CCTV_ALARM_LOG_FILE_PATH,"alarm mail send :mail address : %s, message:%s",mail_address, message);

    return 0;
}


