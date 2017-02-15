#include <common.h>
#include <libalarm.h>
#include <liblogs.h>
#include <libsmtpc.h>
#include <libsignal.h>
#include <syslog.h>
#include <mysql/mysql.h>
#include <signal.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <fcntl.h>

#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <termios.h>
#include <unistd.h>
#include <pty.h>


#include <common_libssh.h>

    /*
       0. (tb_ck_switch_detection_info) select

       1. (tb_ck_deny_ip) sql switch ip, id, password read
       2. ssh process(ssh connection, ssh command write)
       3. (tb_ck_switch_detection_info) sql detection insert
    */

int db_cctv_switch_info_select(char * switch_ip, char * switch_id, char * switch_passwd, int * switch_kind) ;
int db_cctv_switch_detection_select(CK_SIGNAL_INFO *signal_info, int * rule_state); // 0 : ok & not match, 1:ok & match, 2:no & not match, 3: no & match
int db_cctv_switch_detection_delete(CK_SIGNAL_INFO *signal_info);
int dasan_cctv_ssh_process(char * switch_ip, char* switch_id, char * switch_passwd, char *ck_ip,int rule, int action);  //0 : switch ip delete , 1 : switch ip insert 
int juniper_cctv_ssh_process(char * switch_ip, char* switch_id, char * switch_passwd, char *ck_ip,int rule, int action);  //0 : switch ip delete , 1 : switch ip insert 
int nst_cctv_telnet_process(char * switch_ip, char* switch_id, char * switch_passwd, char *ck_ip,int rule, int action);  //0 : switch ip delete , 1 : switch ip insert 
int db_cctv_switch_detection_insert(CK_SIGNAL_INFO *signal_info);

//static struct termios terminal;

int safe_atoi(const char *row)
{
    if (row) {
        return atoi(row);
    }else {
        return 0;
    }
}

int detection_process(CK_SIGNAL_INFO *signal_info)
{

    int result = 0;
    int r_rule = 0;
    int r_action = 0;
    int db_result = 1;
    int switch_kind = 0;
    int rule_state = 0;
    char switch_ip[24];
    char switch_id[64];
    char switch_passwd[128];


    memset(switch_ip, 0x00, sizeof(switch_ip));
    memset(switch_id, 0x00, sizeof(switch_id));
    memset(switch_passwd, 0x00, sizeof(switch_passwd));
    printf("[%s][%d] signal_info ip::%s, ck_detectoin_flag: %d\n",__func__,__LINE__,signal_info->ck_ip, signal_info->ck_detection_flag);

    db_result = db_cctv_switch_info_select(switch_ip, switch_id, switch_passwd, &switch_kind);

    if(!db_result){

        result = db_cctv_switch_detection_select(signal_info, &rule_state);// 0 : ok & not match, 1:ok & match, 2:no & not match, 3: no & match


        switch(result)
        {

            case 0:
                break;

            case 1:
                printf("[%s][%d]  : OK & MATCH (table info delete)\n",__func__,__LINE__);

                log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection drop ip delete :%s", signal_info->ck_ip);

                r_rule = db_cctv_switch_detection_delete(signal_info);

                if(r_rule){

                    r_action = 0;

                    db_result = db_cctv_switch_info_select(switch_ip, switch_id, switch_passwd, &switch_kind);

                    if(switch_kind == 0)
                        dasan_cctv_ssh_process(switch_ip, switch_id, switch_passwd, signal_info->ck_ip, r_rule, r_action); //0 : switch ip delete , 1 : switch ip insert

                    else if(switch_kind == 1)
                        juniper_cctv_ssh_process(switch_ip, switch_id, switch_passwd, signal_info->ck_ip, r_rule, r_action); //0 : switch ip delete , 1 : switch ip insert 

                    else if(switch_kind == 2)
                        nst_cctv_telnet_process(switch_ip, switch_id, switch_passwd, signal_info->ck_ip, r_rule, r_action); //0 : switch ip delete , 1 : switch ip insert 

                    else
                        ;



                }
                break;

            case 2:

                printf("[%s][%d]  : NO & NOT MATCH (table info insert)\n",__func__,__LINE__);
                log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection drop ip add :%s", signal_info->ck_ip);

                r_rule = db_cctv_switch_detection_insert(signal_info);

                if(r_rule){

                    r_action =1;



                    if (switch_kind == 0)
                        dasan_cctv_ssh_process(switch_ip, switch_id, switch_passwd, signal_info->ck_ip, r_rule, r_action); //0 : switch ip delete , 1 : switch ip insert

                    else if(switch_kind == 1)
                        juniper_cctv_ssh_process(switch_ip, switch_id, switch_passwd, signal_info->ck_ip, r_rule, r_action); //0 : switch ip delete , 1 : switch ip insert

                    else if(switch_kind == 2)
                        nst_cctv_telnet_process(switch_ip, switch_id, switch_passwd, signal_info->ck_ip, r_rule, r_action); //0 : switch ip delete , 1 : switch ip insert 


                    else
                        ;

                }
                break;

            case 3:

                break;

            default:
                break;
        }
    }

    printf("[%s][%d]\n",__func__,__LINE__);

	return	0;

}

int db_cctv_switch_detection_update(CK_SIGNAL_INFO *signal_info, int * rule_state) // 0 : ok & not match, 1:ok & match, 2:no & not match, 3: no & match
{
#if 0
    MYSQL       *connection=NULL, conn;
    MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
    char mail_log[512];
    char query[512];
    int num = 0;

    int query_stat;

    mysql_init(&conn);

    connection = mysql_real_connect(&conn, DB_HOST,
            DB_USER, DB_PASS,
            DB_NAME, 3306,
            (char *)NULL, 0);

    if (connection == NULL)
    {
        cctv_system_error("cctv_detection/db_cctv_switch_detection_update() - Mysql connection error : %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_update() - Mysql connection error : %s",strerror(errno));
        return 1;
    }

    memset(query, 0x00, sizeof(query));

    sprintf(query, "update tb_ck_switch_detection_info set C_RULE_STATE = %d where C_DETECTION_IP = '%s';",*rule_state, signal_info->ck_ip);

    query_stat = mysql_query(connection, query);

    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_detection/db_cctv_switch_detection_update() - Mysql query error: %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_update() - Mysql query error: %s",strerror(errno));
        return 0;
    }

    mysql_close(connection);
#endif
	return	0;
}



int db_cctv_switch_detection_select(CK_SIGNAL_INFO *signal_info, int * rule_state) // 0 : ok & not match, 1:ok & match, 2:no & not match, 3: no & match
{
#if 0
    MYSQL       *connection=NULL, conn;
    MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
    char mail_log[512];
    int num = 0;

    int query_stat;

    mysql_init(&conn);

    connection = mysql_real_connect(&conn, DB_HOST,
            DB_USER, DB_PASS,
            DB_NAME, 3306,
            (char *)NULL, 0);

    if (connection == NULL)
    {
        cctv_system_error("cctv_detection/db_cctv_switch_detection_select() - Mysql connection error : %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_select() - Mysql connection error : %s",strerror(errno));
        return 1;
    }

    query_stat = mysql_query(connection, "select * from tb_ck_switch_detection_info");
    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_detection/db_cctv_switch_detection_select() - Mysql query error: %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_select() - Mysql query error: %s",strerror(errno));
        return 1;
    }

    sql_result =mysql_store_result(connection);


    while((sql_row = mysql_fetch_row(sql_result)) != NULL)
    {
       if(!strcmp(signal_info->ck_ip, sql_row[1]))
       {
         *rule_state = atoi(sql_row[3]); 
         num++;
       }
        
    }

    mysql_close(connection);

    if(num == 0 && signal_info->ck_detection_flag == 1)
    return 0;
    
    else if(num > 0 && signal_info->ck_detection_flag == 1)
    return 1;

    else if(num == 0 && signal_info->ck_detection_flag == 2)
    return 2;

    else if(num > 0 && signal_info->ck_detection_flag == 2)
    return 3;
#endif
	return	0;
}

int db_cctv_switch_detection_insert(CK_SIGNAL_INFO *signal_info)
{
#if	0
    MYSQL       *connection=NULL, conn;
    MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
   
    char query[512];

    int query_stat;
    int rule_number;
    int i = 1;
    int j = 0;

    mysql_init(&conn);

    connection = mysql_real_connect(&conn, DB_HOST,
            DB_USER, DB_PASS,
            DB_NAME, 3306,
            (char *)NULL, 0);

    if (connection == NULL)
    {
        cctv_system_error("cctv_detection/db_cctv_switch_detection_insert() - Mysql connection error : %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_insert() - Mysql connection error : %s",strerror(errno));
        return 0;
    }


    while(1){

        j = 0;
        memset(query, 0x00, sizeof(query));

        sprintf(query, "SELECT C_RULE_NUM FROM tb_ck_switch_detection_info;");

        query_stat = mysql_query(connection, query);
        if (query_stat != 0)
        {
            mysql_close(connection);
            cctv_system_error("cctv_detection/db_cctv_switch_detection_insert() - Mysql query error: %s",strerror(errno));
            log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_insert() - Mysql query error: %s",strerror(errno));
            return 0;
        }

        sql_result = mysql_store_result(connection);


        while((sql_row = mysql_fetch_row(sql_result)) != NULL)
        {

            rule_number =  atoi(sql_row[0]);
            if (rule_number == i)
                j++;

        }
        if(j == 0)
            break;
        i++;
    }

    memset(query, 0x00, sizeof(query));

    sprintf(query, "insert into tb_ck_switch_detection_info(C_DETECTION_IP, C_RULE_NUM) values('%s',%d);",signal_info->ck_ip, i);

    query_stat = mysql_query(connection, query);

    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_detection/db_cctv_switch_detection_insert() - Mysql query error: %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_insert() - Mysql query error: %s",strerror(errno));
        return 0;
    }

    mysql_close(connection);

    return i;
#endif
	return	0;
}

int db_cctv_switch_detection_delete(CK_SIGNAL_INFO *signal_info)
{
#if	0
    MYSQL       *connection=NULL, conn;
    MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
    char query[512];

    int query_stat;

    int rule_number = 0;

    mysql_init(&conn);

    connection = mysql_real_connect(&conn, DB_HOST,
            DB_USER, DB_PASS,
            DB_NAME, 3306,
            (char *)NULL, 0);

    if (connection == NULL)
    {
        cctv_system_error("cctv_detection/db_cctv_switch_detection_delete() - Mysql connection error : %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_delete() - Mysql connection error : %s",strerror(errno));
        return 1;
    }

    sprintf(query, "select * from tb_ck_switch_detection_info where C_DETECTION_IP = '%s';",signal_info->ck_ip);

    query_stat = mysql_query(connection,query);
    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_detection/db_cctv_switch_info_select() - Mysql query error: %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_info_select() - Mysql query error: %s",strerror(errno));
        return 1;
    }

    sql_result = mysql_store_result(connection);


    while((sql_row = mysql_fetch_row(sql_result)) != NULL)
    {
        rule_number = atoi(sql_row[2]); 
    }


    memset(query, 0x00, sizeof(query));


    sprintf(query, "delete from tb_ck_switch_detection_info where C_DETECTION_IP = '%s';",signal_info->ck_ip);


    query_stat = mysql_query(connection, query);

    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_detection/db_cctv_switch_detection_delete() - Mysql query error: %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_detection_delete() - Mysql query error: %s",strerror(errno));
        return 1;
    }


    mysql_close(connection);

    return rule_number;
#endif
	return	0;
}


int db_cctv_switch_info_select(char * switch_ip, char * switch_id, char * switch_passwd, int * switch_kind) 
{
#if 0
    MYSQL       *connection=NULL, conn;
    MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;
    char mail_log[512];
    int num = 0;

    int query_stat;

    mysql_init(&conn);

    connection = mysql_real_connect(&conn, DB_HOST,
            DB_USER, DB_PASS,
            DB_NAME, 3306,
            (char *)NULL, 0);

    if (connection == NULL)
    {
        cctv_system_error("cctv_detection/db_cctv_switch_info_select() - Mysql connection error : %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_info_select() - Mysql connection error : %s",strerror(errno));
        return 1;
    }

    query_stat = mysql_query(connection, "select * from tb_ck_deny_ip");
    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_detection/db_cctv_switch_info_select() - Mysql query error: %s",strerror(errno));
        log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_detection/db_cctv_switch_info_select() - Mysql query error: %s",strerror(errno));
        return 1;
    }

    sql_result = mysql_store_result(connection);


    while((sql_row = mysql_fetch_row(sql_result)) != NULL)
    {
        if(strlen(sql_row[2]) != 0 && strlen(sql_row[3]) != 0 && strlen(sql_row[5]) != 0){
            strncpy(switch_id, sql_row[2],64);
            strncpy(switch_passwd, sql_row[3],128);
            strncpy(switch_ip, sql_row[5],24);
            *switch_kind = atoi(sql_row[8]);
            mysql_close(connection);
            return 0;
        }

        else{
            log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "switch information NULL data (database select tables tb_ck_deny_ip error) ");
            mysql_close(connection);
            return 1;
        }


    }

    mysql_close(connection);
    return 1;
#endif	
	return	0;
}

#if 0
static void do_cleanup(int i) {
    (void) i;

    tcsetattr(0,TCSANOW,&terminal);
}

static void do_exit(int i) {
    (void) i;

    do_cleanup(0);
    exit(0);
}
#endif

ssh_channel chan;
int signal_delayed=0;

static void sigwindowchanged(int i){
    (void) i;
    signal_delayed=1;
}

static void setsignal(void){
    signal(SIGWINCH, sigwindowchanged);
    signal_delayed=0;
}

static void sizechanged(void){
    struct winsize win = { 0, 0, 0, 0 };
    ioctl(1, TIOCGWINSZ, &win);
    ssh_channel_change_pty_size(chan,win.ws_col, win.ws_row);
    setsignal();
}


void get_local_ip(char * source_ip) {


    char output[100];
    FILE *p = popen("ifconfig eth0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'", "r");
    if(p != NULL) {
        while(fgets(output, sizeof(output), p) != NULL);
        memcpy(source_ip, output, 24);
    }
    pclose(p);
}


#define DASAN_MAX_COMD 30

static void dasan_select_loop(ssh_session session,ssh_channel channel, char *ck_ip, int rule, int action, char* source_ip){
    fd_set fds;
    char buffer[4096];
    ssh_channel channels[2], outchannels[2];
    int lus;
    int eof=0;
    unsigned int r;
    int ret = 0;
    int i = 0;
    char cmd_buffer[DASAN_MAX_COMD][256];

    memset(cmd_buffer, 0, DASAN_MAX_COMD * 256);


    printf("[%s][%d] action:%d\n",__func__,__LINE__,action);

    if(action == 1)
    {
        memcpy(cmd_buffer[0],"enable\n",sizeof(cmd_buffer[0]));
        memcpy(cmd_buffer[1],"configure terminal\n",sizeof(cmd_buffer[1]));

        sprintf(cmd_buffer[2],"flow catchb_main_%d create\n",rule);
        sprintf(cmd_buffer[3],"ip %s/32 %s/32\n",ck_ip, source_ip); 
        memcpy(cmd_buffer[4],"apply\n",sizeof(cmd_buffer[4]));
        memcpy(cmd_buffer[5],"exit\n",sizeof(cmd_buffer[5]));

        sprintf(cmd_buffer[6],"flow catchb_%d create\n",rule); 
        sprintf(cmd_buffer[7],"ip %s/32 any\n",ck_ip); 
        memcpy(cmd_buffer[8],"apply\n",sizeof(cmd_buffer[8]));
        memcpy(cmd_buffer[9],"exit\n",sizeof(cmd_buffer[9]));

        sprintf(cmd_buffer[10],"policy catchb_main_%d create\n",rule);
        sprintf(cmd_buffer[11],"include-flow catchb_main_%d\n",rule); 
        memcpy(cmd_buffer[12],"priority highest\n",sizeof(cmd_buffer[12]));
        memcpy(cmd_buffer[13],"interface-binding vlan any\n",sizeof(cmd_buffer[13]));
        memcpy(cmd_buffer[14],"action match permit\n",sizeof(cmd_buffer[14]));
        memcpy(cmd_buffer[15],"apply\n",sizeof(cmd_buffer[15]));
        memcpy(cmd_buffer[16],"exit\n",sizeof(cmd_buffer[16]));

        sprintf(cmd_buffer[17],"policy catchb_%d create\n",rule);
        sprintf(cmd_buffer[18],"include-flow catchb_%d\n",rule); 
        memcpy(cmd_buffer[19],"priority medium\n",sizeof(cmd_buffer[19]));
        memcpy(cmd_buffer[20],"interface-binding vlan any\n",sizeof(cmd_buffer[20]));
        memcpy(cmd_buffer[21],"action match deny\n",sizeof(cmd_buffer[21]));
        memcpy(cmd_buffer[22],"apply\n",sizeof(cmd_buffer[22]));
        memcpy(cmd_buffer[23],"end\n",sizeof(cmd_buffer[23]));
        memcpy(cmd_buffer[24],"exit\n",sizeof(cmd_buffer[24]));


    }
    else 
    {
        memcpy(cmd_buffer[0],"enable\n",sizeof(cmd_buffer[0]));
        memcpy(cmd_buffer[1],"configure terminal\n",sizeof(cmd_buffer[1]));

        sprintf(cmd_buffer[2],"no policy catchb_%d\n",rule);
        sprintf(cmd_buffer[3],"no flow catchb_%d\n",rule); 

        sprintf(cmd_buffer[4],"no policy catchb_main_%d\n",rule);
        sprintf(cmd_buffer[5],"no flow catchb_main_%d\n",rule); 

        memcpy(cmd_buffer[6],"end\n",sizeof(cmd_buffer[6]));
        memcpy(cmd_buffer[7],"exit\n",sizeof(cmd_buffer[7]));


    }

    while(channel){

        do{
            int fd = 0;

            FD_ZERO(&fds);
            if(!eof)
                FD_SET(0,&fds);

            fd = ssh_get_fd(session);
            if (fd < 0) {
                fprintf(stderr, "Error getting fd\n");
                return;
            }
            printf("[%s][%d] fd:%d\n",__func__,__LINE__,fd);
            FD_SET(fd, &fds);

            channels[0]=channel; // set the first channel we want to read from
            channels[1]=NULL;
            if(signal_delayed){
                sizechanged();
            }
            if(ret==EINTR)
                continue;
            if(FD_ISSET(fd,&fds)){


                    printf("[%s][%d] cmd_buffer:%s\n",__func__,__LINE__,cmd_buffer[i]);

                    lus = strlen(cmd_buffer[i]);

                    if(lus)
                        ssh_channel_write(channel,cmd_buffer[i],lus);
                    else {
                        eof=1;
                        ssh_channel_send_eof(channel);
                    }
                    sleep(1);
            }

            if(channel && ssh_channel_is_closed(channel)){
                ssh_channel_free(channel);
                channel=NULL;
                channels[0]=NULL;
            }
            if(outchannels[0]){
                while(channel && ssh_channel_is_open(channel) && (r = ssh_channel_poll(channel,0))!=0){
                    lus=ssh_channel_read(channel,buffer,sizeof(buffer) > r ? r : sizeof(buffer),0);
                    if(lus==-1){
                        fprintf(stderr, "Error reading channel: %s\n",
                                ssh_get_error(session));
                        return;
                    }
                    if(lus==0){
                        ssh_channel_free(channel);
                        channel=channels[0]=NULL;
                    } else
                        if (write(1,buffer,lus) < 0) {
                            fprintf(stderr, "Error writing to buffer\n");
                            return;
                        }
                }
                while(channel && ssh_channel_is_open(channel) && (r = ssh_channel_poll(channel,1))!=0){ /* stderr */
                    lus=ssh_channel_read(channel,buffer,sizeof(buffer) > r ? r : sizeof(buffer),1);
                    if(lus==-1){
                        fprintf(stderr, "Error reading channel: %s\n",
                                ssh_get_error(session));
                        return;
                    }
                    if(lus==0){
                        ssh_channel_free(channel);
                        channel=channels[0]=NULL;
                    } else
                        if (write(2,buffer,lus) < 0) {
                            fprintf(stderr, "Error writing to buffer\n");
                            return;
                        }
                }
            }
            if(channel && ssh_channel_is_closed(channel)){
                ssh_channel_free(channel);
                channel=NULL;
            }
        } while (ret==EINTR || ret==SSH_EINTR);
        
        i++;

    }
}


int dasan_cctv_ssh_process(char * switch_ip, char* switch_id, char * switch_passwd, char *ck_ip,int rule, int action)  //0 : switch ip delete , 1 : switch ip insert 
{
    ssh_session session;
    ssh_channel channel;
    char source_ip[24];
    int rc; 

    get_local_ip(source_ip);

    log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_ssh start ip:%s",ck_ip);
    session = connect_ssh(switch_ip, switch_id,switch_passwd, 0); 
    if (session == NULL) {
        ssh_finalize();
        return 1;
    }   

    channel = ssh_channel_new(session);
    if (channel == NULL) {
        ssh_disconnect(session);
        ssh_free(session);
        ssh_finalize();
        return 1;
    }   

    rc = ssh_channel_open_session(channel);
    if (rc < 0) {
        goto failed;
    }  
   


    rc = ssh_channel_request_exec(channel, "/usr/sbin/vtysh");
    if (rc < 0) {
        goto failed;
    }



    log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "select loop start ip:%s",ck_ip);
    dasan_select_loop(session, channel, ck_ip, rule, action, source_ip);
    log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "select loop end ip:%s",ck_ip);

    ssh_disconnect(session);
    ssh_free(session);
    ssh_finalize();
    log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_ssh(su) end ip:%s",ck_ip);
    return 0;
failed:
    ssh_disconnect(session);
    ssh_free(session);
    ssh_finalize();

    log_message(CK_CCTV_DETECTION_LOG_FILE_PATH, "cctv_ssh end(failed) ip:%s",ck_ip);
    return 1;


}



#define JUNIPER_MAX_COMD 20

static void juniper_select_loop(ssh_session session,ssh_channel channel, char *ck_ip, int rule, int action, char* source_ip){
    fd_set fds;
    ssh_channel channels[2], outchannels[2];
    int lus;
    int eof=0;
    unsigned int r;
    int ret = 0;
    int i = 0;
    char buffer[4096];
    char strtok_buffer[1024];

    char cmd_buffer[JUNIPER_MAX_COMD][512];

    char *ptr;
    int ndx = 0;

    memset(cmd_buffer, 0, JUNIPER_MAX_COMD * 512);
    memset(strtok_buffer, 0x00, sizeof(strtok_buffer));


    if(action == 1)
    {
        memcpy(cmd_buffer[0],"configure\n",sizeof(cmd_buffer[0]));

        sprintf(cmd_buffer[1],"set firewall family ethernet-switching filter catchb_filter term 10 from source-address %s\n",source_ip);
        sprintf(cmd_buffer[2],"set firewall family ethernet-switching filter catchb_filter term 10 from destination-address %s\n",ck_ip);
        memcpy(cmd_buffer[3],"set firewall family ethernet-switching filter catchb_filter term 10 then accept\n",sizeof(cmd_buffer[3]));

        sprintf(cmd_buffer[4],"set firewall family ethernet-switching filter catchb_filter term 20 from source-address %s\n",ck_ip);
        sprintf(cmd_buffer[5],"set firewall family ethernet-switching filter catchb_filter term 20 from destination-address %s\n",source_ip);
        memcpy(cmd_buffer[6],"set firewall family ethernet-switching filter catchb_filter term 20 then accept\n",sizeof(cmd_buffer[6]));

        sprintf(cmd_buffer[7],"set firewall family ethernet-switching filter catchb_filter term 30 from source-address %s\n",ck_ip);
        memcpy(cmd_buffer[8],"set firewall family ethernet-switching filter catchb_filter term 30 then discard\n",sizeof(cmd_buffer[8]));
        
        memcpy(cmd_buffer[9],"set firewall family ethernet-switching filter catchb_filter term 40 then accept\n",sizeof(cmd_buffer[9]));

        memcpy(cmd_buffer[10],"set vlans default filter input catchb_filter\n",sizeof(cmd_buffer[10]));
        memcpy(cmd_buffer[11],"commit\n",sizeof(cmd_buffer[11]));
        memcpy(cmd_buffer[12],"exit\n",sizeof(cmd_buffer[12]));
        memcpy(cmd_buffer[13],"exit\n",sizeof(cmd_buffer[13]));
        memcpy(cmd_buffer[14],"exit\n",sizeof(cmd_buffer[14]));


    }
    else 
    {
        memcpy(cmd_buffer[0],"configure\n",sizeof(cmd_buffer[0]));
        memcpy(cmd_buffer[1],"show firewall family ethernet-switching filter catchb_filter\n",sizeof(cmd_buffer[1]));

        memcpy(cmd_buffer[2],"\n",sizeof(cmd_buffer[2]));
        memcpy(cmd_buffer[3],"\n",sizeof(cmd_buffer[3]));

        sprintf(cmd_buffer[4],"delete firewall family ethernet-switching filter catchb_filter term 10 from destination-address %s\n",ck_ip);
        
        sprintf(cmd_buffer[5],"delete firewall family ethernet-switching filter catchb_filter term 20 from source-address %s\n",ck_ip);
        
        sprintf(cmd_buffer[6],"delete firewall family ethernet-switching filter catchb_filter term 30 from source-address %s\n",ck_ip);

        memcpy(cmd_buffer[7],"commit\n",sizeof(cmd_buffer[7])); 
        memcpy(cmd_buffer[8],"exit\n",sizeof(cmd_buffer[8])); 
        memcpy(cmd_buffer[9],"exit\n",sizeof(cmd_buffer[9])); 
        memcpy(cmd_buffer[10],"exit\n",sizeof(cmd_buffer[10])); 

    }

    while(channel){

        do{
            int fd = 0;

            FD_ZERO(&fds);
            if(!eof)
                FD_SET(0,&fds);

            fd = ssh_get_fd(session);
            if (fd < 0) {
                fprintf(stderr, "Error getting fd\n");
                return;
            }
            FD_SET(fd, &fds);

            channels[0]=channel; // set the first channel we want to read from
            channels[1]=NULL;
            if(signal_delayed){
                sizechanged();
            }
            if(ret==EINTR)
                continue;
            if(FD_ISSET(fd,&fds)){


                lus = strlen(cmd_buffer[i]);


                if(lus)
                    ssh_channel_write(channel,cmd_buffer[i],lus);
                else {
                    eof=1;
                    ssh_channel_send_eof(channel);
                }
                sleep(1);
            }

            if(channel && ssh_channel_is_closed(channel)){
                ssh_channel_free(channel);
                channel=NULL;
                channels[0]=NULL;
            }
            if(outchannels[0]){
                while(channel && ssh_channel_is_open(channel) && (r = ssh_channel_poll(channel,0))!=0){
                    lus=ssh_channel_read(channel,buffer,sizeof(buffer) > r ? r : sizeof(buffer),0);
                    if(lus==-1){
                        fprintf(stderr, "Error reading channel: %s\n",
                                ssh_get_error(session));
                        return;
                    }
                    if(lus==0){
                        ssh_channel_free(channel);
                        channel=channels[0]=NULL;
                    } else
                        if (write(1,buffer,lus) < 0) {
                            fprintf(stderr, "Error writing to buffer\n");
                            return;
                        }

                    if( action != 1){

                        strcpy(strtok_buffer, buffer);

                        ptr = strtok(strtok_buffer, " ");
                        while((ptr = strtok(NULL, " ")) != NULL)
                        {
                            if(strstr(ptr, "/32")){
                                ndx++;
                            }

                        }
                        if(ndx  < 6)
                        {
                            if(strstr(buffer, ck_ip))
                            {
                                memcpy(cmd_buffer[2],"delete vlans default filter input catchb_filter\n",sizeof(cmd_buffer[2]));
                                memcpy(cmd_buffer[3],"delete firewall family ethernet-switching filter catchb_filter\n",sizeof(cmd_buffer[3]));
                            }
                        }
                    }

                }


                while(channel && ssh_channel_is_open(channel) && (r = ssh_channel_poll(channel,1))!=0){ /* stderr */
                    lus=ssh_channel_read(channel,buffer,sizeof(buffer) > r ? r : sizeof(buffer),1);
                    if(lus==-1){
                        fprintf(stderr, "Error reading channel: %s\n",
                                ssh_get_error(session));
                        return;
                    }
                    if(lus==0){
                        ssh_channel_free(channel);
                        channel=channels[0]=NULL;
                    } else
                        if (write(2,buffer,lus) < 0) {
                            fprintf(stderr, "Error writing to buffer\n");
                            return;
                        }
                }
            }
            if(channel && ssh_channel_is_closed(channel)){
                ssh_channel_free(channel);
                channel=NULL;
            }
        } while (ret==EINTR || ret==SSH_EINTR);

        i++;

    }
}


int juniper_cctv_ssh_process(char * switch_ip, char* switch_id, char * switch_passwd, char *ck_ip,int rule, int action)  //0 : switch ip delete , 1 : switch ip insert 
{
    ssh_session session;
    ssh_channel channel;
    char source_ip[24];
    int rc; 

    
    get_local_ip(source_ip);



    session = connect_ssh(switch_ip, switch_id,switch_passwd, 0); 
    if (session == NULL) {
        ssh_finalize();
        return 1;
    }   

    channel = ssh_channel_new(session);
    if (channel == NULL) {
        ssh_disconnect(session);
        ssh_free(session);
        ssh_finalize();
        return 1;
    }   

    rc = ssh_channel_open_session(channel);
    if (rc < 0) {
        goto failed;
    }  
   


    rc = ssh_channel_request_exec(channel, "cli");
    if (rc < 0) {
        goto failed;
    }


    juniper_select_loop(session, channel, ck_ip, rule, action, source_ip);

    ssh_disconnect(session);
    ssh_free(session);
    ssh_finalize();
    return 0;
failed:
    ssh_disconnect(session);
    ssh_free(session);
    ssh_finalize();

    return 1;


}



#define NST_MAX_COMD 20
int nst_cctv_telnet_process(char * switch_ip, char* switch_id, char * switch_passwd, char *ck_ip,int rule, int action)  //0 : switch ip delete , 1 : switch ip insert 
{


    int    fd;
    FILE * fp;
    char telnet_cmd[64];
    int i = 0;
    int rule_num = 0;
    char source_ip[24];
    char cmd_buffer[NST_MAX_COMD][512];

    if (pipe < 0) {
        perror("pipe");
        return 1;
    }

    memset(source_ip, 0x00, sizeof(source_ip));
    memset(telnet_cmd, 0x00, sizeof(telnet_cmd));
    memset(cmd_buffer, 0, NST_MAX_COMD * 512);

    get_local_ip(source_ip);

    if(source_ip[strlen(source_ip)-1] =='\n' || source_ip[strlen(source_ip)] =='\r')
        source_ip[strlen(source_ip)-1] ='\0';

    sprintf(telnet_cmd, "telnet %s", switch_ip);

    fp = popen(telnet_cmd, "w");
    fd = fileno(fp);

    rule_num = rule*10;


    
    if(action == 1)
    {
        sprintf(cmd_buffer[0],"%s\n",switch_id);
        sprintf(cmd_buffer[1],"%s\n",switch_passwd);
        memcpy(cmd_buffer[2],"configure\n",sizeof(cmd_buffer[2]));

        memcpy(cmd_buffer[3],"interface range GigabitEthernet 1-28\n",sizeof(cmd_buffer[3]));
        memcpy(cmd_buffer[4],"no ip acl\n",sizeof(cmd_buffer[4]));

        memcpy(cmd_buffer[5],"exit\n",sizeof(cmd_buffer[5]));

        memcpy(cmd_buffer[6],"ip acl catchb_filter\n",sizeof(cmd_buffer[6]));

        sprintf(cmd_buffer[7],"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255\n",rule_num,source_ip, ck_ip);
        sprintf(cmd_buffer[8],"sequence %d permit ip %s/255.255.255.255 %s/255.255.255.255\n",rule_num +1,ck_ip, source_ip);
        sprintf(cmd_buffer[9],"sequence %d deny ip %s/255.255.255.255 any\n",rule_num +2, ck_ip);
        sprintf(cmd_buffer[10],"sequence 2147483640 permit %s any any\n",ck_ip);
        memcpy(cmd_buffer[11],"exit\n",sizeof(cmd_buffer[11]));
        memcpy(cmd_buffer[12],"interface range GigabitEthernet 1-28\n",sizeof(cmd_buffer[12]));
        memcpy(cmd_buffer[13],"ip acl catchb_filter\n",sizeof(cmd_buffer[13]));
        memcpy(cmd_buffer[14],"exit\n",sizeof(cmd_buffer[14]));
        memcpy(cmd_buffer[15],"exit\n",sizeof(cmd_buffer[15]));
        memcpy(cmd_buffer[16],"exit\n",sizeof(cmd_buffer[16]));
        memcpy(cmd_buffer[17],"exit\n",sizeof(cmd_buffer[17]));

    }
    else 
    {
        sprintf(cmd_buffer[0],"%s\n",switch_id);
        sprintf(cmd_buffer[1],"%s\n",switch_passwd);
        memcpy(cmd_buffer[2],"configure\n",sizeof(cmd_buffer[2]));


        memcpy(cmd_buffer[3],"interface range GigabitEthernet 1-28\n",sizeof(cmd_buffer[3]));
        memcpy(cmd_buffer[4],"no ip acl\n",sizeof(cmd_buffer[4]));

        memcpy(cmd_buffer[5],"exit\n",sizeof(cmd_buffer[5]));
        memcpy(cmd_buffer[6],"ip acl catchb_filter\n",sizeof(cmd_buffer[6]));

        sprintf(cmd_buffer[7],"no sequence %d\n",rule_num);
        sprintf(cmd_buffer[8],"no sequence %d\n",rule_num+1);
        sprintf(cmd_buffer[9],"no sequence %d\n",rule_num+2);

        memcpy(cmd_buffer[10],"exit\n",sizeof(cmd_buffer[10]));

        memcpy(cmd_buffer[11],"interface range GigabitEthernet 1-28\n",sizeof(cmd_buffer[11]));
        memcpy(cmd_buffer[12],"ip acl catchb_filter\n",sizeof(cmd_buffer[12]));

        memcpy(cmd_buffer[13],"end\n",sizeof(cmd_buffer[13]));
        memcpy(cmd_buffer[14],"exit\n",sizeof(cmd_buffer[14]));
        memcpy(cmd_buffer[15],"exit\n",sizeof(cmd_buffer[15]));
    }


    while (1)
    {
        int n = strlen(cmd_buffer[i]);

        if (n > 0){
            write(fd, cmd_buffer[i], n);

        }
        else{
            break;
        }

        usleep( 1000 * 700 );

        if(i == 16){

            break;
        }

        i++;

    }
            pclose(fp);
    return 1;
}




























