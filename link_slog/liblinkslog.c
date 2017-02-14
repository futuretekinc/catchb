#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <cctv_link_slog.h>
#include <common.h>
#include <libdb.h>
#include <syslog.h>


int systemf(char *fmt, ...)
{
    va_list ap;
    char buffer[1024];

    /* format 문자열을 만든다 */
    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);

    return system(buffer);
}


int syslog_init()
{
    db_cctv_link_server_select();

	return	0;
}

int db_cctv_link_server_select()
{
#if 0
    MYSQL       *connection=NULL, conn;
    MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
    int rownum, i;
    FILE *fp;

    int query_stat;

    mysql_init(&conn);

    connection = mysql_real_connect(&conn, DB_HOST,
            DB_USER, DB_PASS,
            DB_NAME, 3306,
            (char *)NULL, 0);

    if (connection == NULL)
    {
        cctv_system_error("cctv_operation/db_cctv_info_select() - Mysql connection error : %s",strerror(errno));
        return 1;
    }

    query_stat = mysql_query(connection, "select * from tb_ck_link_info");
    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_operation/db_cctv_info_select() - Mysql query error: %s",strerror(errno));
        return 1;
    }

    sql_result =mysql_store_result(connection);

    if(!sql_result) return WDB_ERROR;

    rownum = sql_result->row_count;

    systemf("rm -rf /etc/rsyslog.conf");
    systemf("cp /etc/rsyslog.conf.backup /etc/rsyslog.conf");

    if(rownum){

        //fp = fopen("/etc/rsyslog.conf", "a+");

        //if(fp == NULL)
        //    return 1;

        while((sql_row = mysql_fetch_row(sql_result)) != NULL)
        {
            printf("%s\n", sql_row[2]);
            log_message(CK_CCTV_LINK_SLOG_LOG_FILE_PATH, "rsyslog add ip :%s",sql_row[2]);
            systemf("echo local7.* @%s >>/etc/rsyslog.conf", sql_row[2]);
            usleep(1);
        }
        systemf("service rsyslog restart");
    }
    else{
        systemf("service rsyslog restart");
    }
    mysql_free_result(sql_result);
    mysql_close(connection);


    log_message(CK_CCTV_LINK_SLOG_LOG_FILE_PATH, "rsyslog file change");
#endif
    return 0;
}
