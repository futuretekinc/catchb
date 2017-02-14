#include <cctv_maind.h>
#include <common_libssh.h>
#include "catchb_trace.h"

int db_cctv_log_check_delete()
{
#if 0
        MYSQL       *connection=NULL, conn;

        int query_stat;
        char query[512];

        mysql_init(&conn);

        memset(query, 0x00, sizeof(query));

        connection = mysql_real_connect(&conn, DB_HOST,
                DB_USER, DB_PASS,
                DB_NAME, 3306,
                (char *)NULL, 0);


        if (connection == NULL)
        {    
            cctv_system_error("cctv_maind/db_cctv_log_check_delete() - Mysql connection error : %s",strerror(errno));
            return 1;
        }    

        sprintf(query, "delete from tb_ck_ip_result_log where C_RE_CCTV_TIME < date_add(now(), interval -2 month);");
        
        query_stat = mysql_query(connection, query);
        if (query_stat != 0)
        {    
            mysql_close(connection);
            cctv_system_error("cctv_maind/db_cctv_log_check_delete() - Mysql query error: %s",strerror(errno));
            return 1;
        }    


        memset(query, 0x00, sizeof(query));

        sprintf(query, "delete from tb_ck_system_resource where C_RE_CCTV_TIME < date_add(now(), interval -2 month);");

        query_stat = mysql_query(connection, query);
        if (query_stat != 0)
        {    
            mysql_close(connection);
            cctv_system_error("cctv_maind/db_cctv_log_check_delete() - Mysql query error: %s",strerror(errno));
            return 1;
        }

        mysql_close(connection);
#endif
	return	0;
}


int check_process(char *process_name)
{   
    DIR* pdir;
    struct dirent *pinfo;
    int is_live = 0;

	TRACE("check_process(%s) entry\n", process_name);
    pdir = opendir("/proc");
    if(pdir == NULL)
    {
        printf("err: NO_DIR\n");
        return 0;
    }

    /** /proc 디렉토리의 프로세스 검색 */
    while(1)
    {
        pinfo = readdir(pdir);
        if(pinfo == NULL)
		{
            break;
		}

        /** 파일이거나 ".", "..", 프로세스 디렉토리는 숫자로 시작하기 때문에 아스키코드 57(9)가 넘을 경우 건너뜀 */
        if(pinfo->d_type != 4 || pinfo->d_name[0] == '.' || pinfo->d_name[0] > 57)
		{
            continue;
		}

        FILE* fp;
        char buff[512];
        char path[512];

        sprintf(path, "/proc/%s/cmdline", pinfo->d_name);
        fp = fopen(path, "rt");
        if(fp)
        {
            fgets(buff, 512, fp);
            fclose(fp);

            /** 프로세스명과 status 파일 내용과 비교 */  
  //          printf("[%s][%d]buff:%s, process_name:%s\n",__func__,__LINE__,buff, process_name);
            if(strstr(buff, process_name))
            {
                is_live = 1;
                break;
            }
        }
        else
        {
            printf("Can't read file [%s]\n", path);
        }
    }

    closedir(pdir);

	TRACE("check_process(%s) exit\n", process_name);
    return is_live;
}

/** 현재 시간을 설정 */
void get_timef(time_t org_time, char *time_str)
{
    struct tm *tm_ptr;
    tm_ptr = localtime(&org_time);

    sprintf(time_str, "%d/%d/%d %d:%d:%d",
            tm_ptr->tm_year+1900,
            tm_ptr->tm_mon+1,
            tm_ptr->tm_mday,
            tm_ptr->tm_hour,
            tm_ptr->tm_min,
            tm_ptr->tm_sec);
}
