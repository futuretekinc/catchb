#include <common.h>
#include <cctv_alarm.h>
#include <libalarm.h>
#include <liblogs.h>
//#include <libsmtpc.h>
#include <libsignal.h>
#include <syslog.h>
#include "mysql/mysql.h"

int db_cctv_info_select(CK_SIGNAL_INFO *signal_info)
{
#if 0
    MYSQL       *connection=NULL, conn;
    MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
    char mail_log[512];

    int query_stat;

    mysql_init(&conn);

    connection = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0); 
    if (connection == NULL)
    {
        cctv_system_error("cctv_alarmn/db_cctv_info_select() - Mysql connection error : %s",strerror(errno));
        return 1;
    }

    query_stat = mysql_query(connection, "select * from tb_ck_alarm_info");
    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_alarm/db_cctv_info_select() - Mysql query error: %s",strerror(errno));
        return 1;
    }

    sql_result =mysql_store_result(connection);


    while((sql_row = mysql_fetch_row(sql_result)) != NULL)
    {
        memset(mail_log, 0x00, sizeof(mail_log));
        sprintf(mail_log, "[CCTV ERROR] CCTV_ID : %s, CCTV_IP : %s,CCTV_LOG : %s", signal_info->ck_cctv_id, signal_info->ck_ip, signal_info->ck_log); 
        
        sendalarmmail(sql_row[2], mail_log);
    }

    mysql_close(connection);
#endif
	return	0;
}


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


