#include <common.h>
#include <linux_list.h>
#include <liblogs.h>
#include <libsignal.h>
#include <liboperation.h>
#include <libping.h>
#include <libparsing.h>
#include <cctv_analysis.h>
#include <syslog.h>
#include <semaphore.h>

#include<unistd.h>
#include<stdio.h>
#include<netinet/in.h>
#include<net/if.h>
#include<sys/ioctl.h>
#include<arpa/inet.h>
#include"trace.h"

extern sem_t sendthread, mainthread;

static CK_SCORE_INFO score_info;


extern struct list_head check_ip_list_head;

extern char source_ip[ADDR_LENG];

extern struct in_addr local_ip;
extern struct in_addr http_remote_ip;


int free_static_url_information_list()
{
    CK_IP_INFO * entry;

    while (!list_empty(&check_ip_list_head)) {
        entry = list_entry(check_ip_list_head.next, CK_IP_INFO, list);
        list_del(&entry->list);
        free(entry);
    }

	return	0;
}

void initialize_os_matrix()
{
    score_info.tid = syscall(SYS_gettid);
    score_info.score = 0.0;
    score_info.os_matrix.ttl = false;
    score_info.os_matrix.df = false;
    score_info.os_matrix.ipid = false;
    score_info.os_matrix.tcp_mss = false;
    score_info.os_matrix.window_scale = false;
    score_info.os_matrix.window_size = false;
    score_info.os_matrix.syn_pkt_size = false;
    score_info.os_matrix.options_order = false;
    
}

int db_cctv_info_select(int * t_num)
{
#if 0
    MYSQL       *connection=NULL, conn;
    MYSQL_RES   *sql_result;
    MYSQL_ROW   sql_row;

    int query_stat;

    mysql_init(&conn);

    connection = mysql_real_connect(&conn, DB_HOST,
            DB_USER, DB_PASS,
            DB_NAME, 3306,
            (char *)NULL, 0);

    if (connection == NULL)
    {
       cctv_system_error("cctv_analysis/db_cctv_info_select() - Mysql connection error : %s",strerror(errno));
        return 1;
    }

    query_stat = mysql_query(connection, "select * from tb_ck_result_info");
    if (query_stat != 0)
    {
       mysql_close(connection);
        return 1;
    }

    sql_result =mysql_store_result(connection);
    
    
    while((sql_row = mysql_fetch_row(sql_result)) != NULL)
    {
        CK_IP_INFO *check_ip_info = (CK_IP_INFO*)malloc(sizeof(CK_IP_INFO));

        check_ip_info->ck_cctv_idx = safe_atoi(sql_row[1]);

        memcpy(check_ip_info->ck_cctv_id, sql_row[2], sizeof(check_ip_info->ck_cctv_id));

        memcpy(check_ip_info->ck_ip, sql_row[3], sizeof(check_ip_info->ck_ip));
       
        if(strlen(sql_row[3]))
        memcpy(check_ip_info->ck_signature, sql_row[4], sizeof(check_ip_info->ck_signature));
        

        (*t_num)++;

        list_add_tail(&check_ip_info->list, &check_ip_list_head);


    }

    mysql_close(connection);
#endif
	return	0;
}



int db_cctv_log_insert(int cctv_idx, char *cctv_id, char *ip, char *hash_value, int status, char * message)
{
#if 0
    MYSQL       *connection=NULL, conn;
    char query[512];
    int query_stat;

    mysql_init(&conn);

    memset(query, 0x00, sizeof(query));

    connection = mysql_real_connect(&conn, DB_HOST,
            DB_USER, DB_PASS,
            DB_NAME, 3306,
            (char *)NULL, 0);

    if (connection == NULL)
    {
        cctv_system_error("cctv_operation/db_cctv_log_insert() - Mysql connection error : %s",strerror(errno));
        return 1;
    }

    sprintf(query, "insert into tb_ck_ip_result_log(C_CCTV_IDX,C_CCTV_ID,C_RE_CCTV_IP,C_RE_CCTV_SIGNATURE, C_RE_CCTV_STATUS,C_RE_CCTV_LOG) values(%d,'%s','%s','%s',%d,'%s');",cctv_idx,cctv_id, ip, hash_value,status, message);

    printf("[%s][%d] query:%s\n",__func__,__LINE__,query);
    query_stat = mysql_query(connection, query);
    if (query_stat != 0)
    {
        mysql_close(connection);
        cctv_system_error("cctv_operation/db_cctv_log_insert() - Mysql query error: %s",strerror(errno));
        return 1;
    }

    mysql_close(connection);
#endif

	return	0;
}

void send_http_request(int port_number, char *analysis_cctv_ip)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;


    inet_aton(analysis_cctv_ip, &http_remote_ip);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        //log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "cctv_operation/send_http_request() - Error while creating the socket:%s\n",analysis_cctv_ip);
        cctv_system_error("cctv_operation/send_http_request() - Error while creating the socket");
        return ;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    serv_addr.sin_addr.s_addr = http_remote_ip.s_addr;


    int flags, error;
    fd_set rset,wset;
    struct timeval tval;
    socklen_t len;
    int nsec = 4;

    error = 0;

    if ((n = connect(sockfd, ((struct sockaddr*)(&serv_addr)), sizeof(serv_addr))) < 0)
    {

        if (errno != EINPROGRESS){

        cctv_system_error("cctv_analysis/send_http_request() - Error while connecting :%s\n", analysis_cctv_ip);
        close(sockfd);
         return ;
        }
    }

    if (n == 0)
	{
        goto done;
	}

	flags = fcntl(sockfd, F_GETFL); 

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		close(sockfd);
		return;
	}


    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;

    if ( (n = select(sockfd+1, &rset, &wset, NULL,

                    nsec ? &tval : NULL)) == 0) {

        close(sockfd);      
        errno = ETIMEDOUT;
        return;

    }



    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
            return;        
    }

    char* http_request = ((char*)("GET / HTTP/1.0\n\n"));


    send(sockfd, http_request, strlen(http_request), 0);

    /* remove comments for ONLY ONE to shutdown or close the port */
    //close(sockfd);

done:

    if (error) {
        close(sockfd);      /* just in case */
        errno = error;
        return;
    }
    //원환경인 BLOCKING MODE 복구
    fcntl(sockfd, F_SETFL, flags );
    close(sockfd);
    return;

    //shutdown(sockfd,SHUT_RDWR);
}//end send_http_request

void clean_up(void *id)
{

    printf("Thread cancel Clean_up function\n");
}

void* thread_packet_send_operation(void *id)
{

    struct list_head *pos;
    CK_IP_INFO * entry;
    int port_number = 80;

	TRACE_ENTRY();
    list_for_each(pos, &check_ip_list_head)
	{
        entry = list_entry(pos, CK_IP_INFO, list);
        if(strlen(entry->ck_ip))
		{
            sem_wait(&sendthread);
			TRACE("send_http_request[%s]\n", entry->ck_ip);
            send_http_request(port_number, entry->ck_ip);
            sem_post(&mainthread);

			TRACE("send_http_request[%s]\n", entry->ck_ip);
            send_http_request(port_number, entry->ck_ip);

            if(score_info.descr)
			{
                sleep(4);
                printf("[%s][%d] pack_loop break\n",__func__,__LINE__);
                pcap_breakloop(score_info.descr);
            }
        }
    }
	TRACE_EXIT();

	return	0;
}

void* thread_main_operation(void *id)
{
    char hash_string[1024];
    char hash_value[64];
    int port[PORT_NUM] = {80, 135, 139, 443, 445, 554, 4520, 49152};
    //int port[PORT_NUM] = {22, 21, 20, 443, 445, 554, 4520, 49152};
    int i;
    int result_ttl = 0;
    int cctv_idx = 0;


    int snap_leng = 0;
    bpf_u_int32 net;
    bpf_u_int32 mask;

    char scan_result[PORT_NUM][12];
    char* dev;
    char errbuf[PCAP_ERRBUF_SIZE];
    char cctv_id[24];
    char mac_buff[48];
    struct list_head *pos;

    CK_IP_INFO * entry;
    CK_SIGNAL_INFO signal_info;

	TRACE_ENTRY();

    sleep(2);

    list_for_each(pos, &check_ip_list_head)
	{

	    entry = list_entry(pos, CK_IP_INFO, list);
	    memcpy(score_info.analysis_cctv_ip, entry->ck_ip,sizeof(score_info.analysis_cctv_ip));


	    memset(hash_string, 0x00, sizeof(hash_string));
	    memset(hash_value, 0x00, sizeof(hash_value));
	    memset(cctv_id, 0x00, sizeof(cctv_id));
	    memset(&signal_info, 0x00, sizeof(CK_SIGNAL_INFO));

	    memcpy(cctv_id, entry->ck_cctv_id, sizeof(cctv_id));

        cctv_idx = entry->ck_cctv_idx;

	    initialize_os_matrix();

	    result_ttl = 0;



        //log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "analysis setting ip :%s start",score_info.analysis_cctv_ip);


        for( i = 0; i< 3; i++)
		{
            cctv_ping_check(score_info.analysis_cctv_ip, &result_ttl);

            usleep( 1000 * 700 );

        //log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "for(cctv_ping_check)function  ip :%s, ttl :%d",score_info.analysis_cctv_ip, result_ttl);


            if(result_ttl > 0)
                break;
        }

        //log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "analysis setting ip :%s ttl:%d",score_info.analysis_cctv_ip,result_ttl);
        memcpy(signal_info.ck_ip, score_info.analysis_cctv_ip, sizeof(signal_info.ck_ip));
        memcpy(signal_info.ck_cctv_id, entry->ck_cctv_id, sizeof(signal_info.ck_cctv_id));

	    if(result_ttl <= 0 || strlen(entry->ck_signature) == 0){


		    db_cctv_log_insert(cctv_idx, cctv_id, score_info.analysis_cctv_ip, NULL, UNUSAL, PING_UNUSAL_LOG);
		    memcpy(signal_info.ck_log, PING_UNUSAL_LOG, sizeof(signal_info.ck_log));
		    signal_info.ck_detection_flag = UNUSAL;

            signal_info.ck_event_division = 0;

		    ck_signal(SV_SOCK_CCTV_ALARM_PATH,&signal_info);
		    ck_signal(SV_SOCK_CCTV_DETECTION_PATH,&signal_info);

            sem_post(&sendthread);
            sem_wait(&mainthread);
            sleep(5);
		    syslog(LOG_NOTICE | LOG_LOCAL7, "THE CCTV IP : %s , LOG : %s[UNUSUAL]",score_info.analysis_cctv_ip, PING_UNUSAL_LOG);

	    }
	    else{

            sprintf(hash_string,"[ip : %s , ", score_info.analysis_cctv_ip);

            arp_parsing(score_info.analysis_cctv_ip, mac_buff);

            sprintf(hash_string,"%s mac : %s],",hash_string, mac_buff);

            for(i =0 ;i < PORT_NUM ; i++){
                port_scan(source_ip, score_info.analysis_cctv_ip, port[i], scan_result[i]);
                sprintf(hash_string,"%s port : %d : %s,",hash_string, port[i], scan_result[i]);

                usleep( 1000 * 300 );
            }

            dev = pcap_lookupdev(errbuf);
            if(dev == NULL)
            {

                cctv_system_error("cctv_analysis/thread_main_operation() - error not device : %s", score_info.analysis_cctv_ip);
                log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "cctv_operation/thread_main_operation() - error not device :%s",score_info.analysis_cctv_ip);

                sem_post(&sendthread);
                sem_wait(&mainthread);
                sleep(5);
                continue;
            }
            score_info.descr = pcap_open_live(dev, BUFSIZ, 0, -1, errbuf);
            if(score_info.descr == NULL)
            {
                cctv_system_error("cctv_analysis/thread_main_operation() - error not pcap_open_live function:%s",score_info.analysis_cctv_ip);
                log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "cctv_analysis/thread_main_operation() - error not pcap_open_live function :%s",score_info.analysis_cctv_ip);

                sem_post(&sendthread);
                sem_wait(&mainthread);
                sleep(5);

                continue;
            }

            snap_leng = pcap_snapshot(score_info.descr);
            if(BUFSIZ < snap_leng){
                sem_post(&sendthread);
                sem_wait(&mainthread);
                sleep(6);

                cctv_system_error("cctv_analysis/thread_main_operation() -pcap_snapshot error");
                log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "cctv_analysis/thread_main_operation() - error not pcap_snapshot :%s",score_info.analysis_cctv_ip);
                continue;
            }
           
            if(pcap_lookupnet(dev, &net, &mask, errbuf) < 0){
                sem_post(&sendthread);
                sem_wait(&mainthread);
                sleep(6);

                cctv_system_error("cctv_analysis/thread_main_operation() -pcap_lookupnet error");
                log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "cctv_analysis/thread_main_operation() - error not pcap_lookupnet :%s",score_info.analysis_cctv_ip);

                continue;

            }

            struct bpf_program fcode;
            char filter_rule[512];
            memset(filter_rule, 0x00, sizeof(filter_rule));

            sprintf(filter_rule,"src %s and dst %s and tcp",score_info.analysis_cctv_ip, source_ip);

            if(pcap_compile(score_info.descr, &fcode, filter_rule, 0, mask) < 0){
                sem_post(&sendthread);
                sem_wait(&mainthread);
                sleep(6);

                cctv_system_error("cctv_analysis/thread_main_operation() -pcap_compile error");
                log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "cctv_analysis/thread_main_operation() - error not pcap_compile :%s",score_info.analysis_cctv_ip);

                continue;

            }

            if(pcap_setfilter(score_info.descr, &fcode) < 0 ){
                sem_post(&sendthread);
                sem_wait(&mainthread);
                sleep(6);

                cctv_system_error("cctv_analysis/thread_main_operation() -pcap_setfilter error");
                log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "cctv_analysis/thread_main_operation() - error not pcap_setfilter :%s",score_info.analysis_cctv_ip);

                continue;

            }

            //int datalink;
            //datalink = pcap_datalink(score_info.descr);

            sem_post(&sendthread);
            sem_wait(&mainthread);
            pcap_loop(score_info.descr, 0, sniffer, NULL);
            pcap_close(score_info.descr);
         
            sprintf(hash_string,"%s, score : %f", hash_string, score_info.score);


            //log_message(CK_CCTV_ANALYSIS_LOG_FILE_PATH, "ip:%s, string%s",score_info.analysis_cctv_ip, hash_string);
            cctv_hash_sha1(hash_string, hash_value, strlen(hash_string));

        	TRACE("ip:%s, score:%f\n",score_info.analysis_cctv_ip, score_info.score);

		    if(!strncmp(entry->ck_signature,hash_value, sizeof(hash_value)))
		    {
                memcpy(signal_info.ck_log, NOMALITY_LOG, sizeof(signal_info.ck_log));
                signal_info.ck_detection_flag = NOMALITY;
                memcpy(signal_info.ck_signature, hash_value, sizeof(signal_info.ck_signature));
                signal_info.ck_event_division = 0;

                db_cctv_log_insert(cctv_idx, cctv_id, score_info.analysis_cctv_ip, hash_value, NOMALITY, NOMALITY_LOG);
                ck_signal(SV_SOCK_CCTV_DETECTION_PATH,&signal_info);
    //            syslog(LOG_NOTICE | LOG_LOCAL7, "THE CCTV IP : %s , LOG : %s[NORMAL]",score_info.analysis_cctv_ip, NOMALITY_LOG);

            }
            else
            {
                memcpy(signal_info.ck_log, SIGNATURE_UNUSAL_LOG, sizeof(signal_info.ck_cctv_id));
                signal_info.ck_detection_flag = UNUSAL;
                memcpy(signal_info.ck_signature, hash_value, sizeof(signal_info.ck_signature));

                db_cctv_log_insert(cctv_idx, cctv_id, score_info.analysis_cctv_ip, hash_value, UNUSAL, SIGNATURE_UNUSAL_LOG);
                ck_signal(SV_SOCK_CCTV_ALARM_PATH,&signal_info);
                ck_signal(SV_SOCK_CCTV_DETECTION_PATH,&signal_info);

                syslog(LOG_NOTICE | LOG_LOCAL7, "THE CCTV IP : %s , LOG : %s[NORMAL]",score_info.analysis_cctv_ip, SIGNATURE_UNUSAL_LOG);

            }

	    }
    }

	TRACE_EXIT();

	return	0;
}


void sniffer(u_char* useless, const struct pcap_pkthdr* pkthdr, const u_char* packet)
{

    if(!((packet[12] == 8) && (packet[13] == 0)))    // ip packet
    {
        return;
    }

    struct ip_hdr* rx_ip_hdr = ((struct ip_hdr*)(packet + 14));


    if((rx_ip_hdr-> ip_src.s_addr != http_remote_ip.s_addr) || (rx_ip_hdr-> ip_dst.s_addr != local_ip.s_addr))

    {
        return;
    }

    processing_ip((uint8_t*)packet, pkthdr->len);

    switch(rx_ip_hdr->ip_p)
    {
        case IP_PROTO_TCP:
            processing_tcp((uint8_t*)packet, pkthdr->len);
            usleep( 1000*100);
            break;

        default:
            break;

    }//end switch



    return;

} //end sniffer


void processing_tcp(uint8_t * packet, int length)
{
    struct ip_hdr* rx_ip_hdr = ((struct ip_hdr*)(packet + 14));
    int length_ip_header = rx_ip_hdr->ip_hl * 4;  // obtain the length of header in bytes to check for options

    struct tcp_hdr* rx_tcp_hdr = ((struct tcp_hdr*)(packet + 14 + length_ip_header));
    int length_tcp_header = rx_tcp_hdr->tcp_dt_ofst * 4;  // obtain the length of header in bytes to check for options
    int length_tcp_options = length_tcp_header - 20;



    uint16_t tcp_mss = 0;
    int window_scale = -1;

    if ((rx_tcp_hdr->tcp_flags & 2) == 2)
    {
    //    ;
    //}
        int option_index = 0;
        bool end_of_options = false;
        while((end_of_options == false) & (option_index < length_tcp_options))
        {
            switch(packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index])
            {
                case TCPOPT_EOL:
                    end_of_options = true;
                    break;

                case TCPOPT_NOP:
                    option_index++;
                    break;

                case TCPOPT_MAXSEG:
                    tcp_mss = (packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index + 2] * 256) +
                        packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index + 3];
                    option_index += 4;
                    break;

                case TCPOPT_WSCALE:
                    window_scale = packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index + 2];
                    option_index += 3;
                    break;

                case TCPOPT_SACKOK:
                    option_index += 2;
                    break;

                case TCPOPT_TIMESTAMP:
                    option_index += 10;
                    break;
            }
        }


        if(score_info.os_matrix.tcp_mss == false)
        {
            if (tcp_mss == 1380)
            {
                score_info.score+=1;      // MacOS 9.1
            }

            score_info.os_matrix.tcp_mss = true;
        }


        if(score_info.os_matrix.window_scale == false)
        {
            if (window_scale == -1)     //no window scale appears in tcp options
            {
                score_info.score+=2;       // Linux 1.2
            }
            else if (window_scale == 0)
            {
                score_info.score+=3;       // Linux 1.2
            }
            else if (window_scale == 1)
            {
                score_info.score+=4;       // Linux 1.2
            }
            else if (window_scale == 2)
            {
                score_info.score+=5;       // Linux 1.2
            }


            else if ((window_scale == 5) | (window_scale == 6) | (window_scale == 7))
            {
                score_info.score+=6;       // Linux 1.2
            }
            else if (window_scale == 8)
            {
                score_info.score+=7;       // Linux 1.2
            }

            score_info.os_matrix.window_scale = true;
        }
        if(score_info.os_matrix.window_size == false)
        {
            int temp_window = htons(rx_tcp_hdr->tcp_window);

            if ((temp_window <= (tcp_mss + 50)) & (temp_window >= (tcp_mss - 50)))
            {
                score_info.score+=10;       // Linux 1.2
            }

            if ((temp_window <= (32736 + 70)) & (temp_window >= (32736 - 70)))
            {
                score_info.score+=20;       // Linux 1.2
            }

            if (((temp_window <= (512 + 50)) & (temp_window >= (512 - 50))) |
                    ((temp_window <= (16384 + 70)) & (temp_window >= (16384 - 70))))
            {
                score_info.score+=30;       // Linux 1.2
            }


            if (((temp_window <= (tcp_mss * 11) + 70) & (temp_window >= (tcp_mss * 11) - 70 )) |
                    ((temp_window <= (tcp_mss * 20) + 70) & (temp_window >= (tcp_mss * 20) - 70 )))
            {
                score_info.score+=40;       // Linux 1.2
            }

            if (((temp_window <= (tcp_mss * 2) + 70) & (temp_window >= (tcp_mss * 2) - 70 )) |
                    ((temp_window <= (tcp_mss * 3) + 70) & (temp_window >= (tcp_mss * 3) - 70 )) |
                    ((temp_window <= (tcp_mss * 4) + 70) & (temp_window >= (tcp_mss * 4) - 70 )))
            {
                score_info.score+=50;       // Linux 1.2
            }

            if ((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70)))
            {
                score_info.score+=60;       // Linux 1.2
            }

            if (((temp_window <= (tcp_mss * 44) + 70) & (temp_window >= (tcp_mss * 44) - 70 )))
            {
                score_info.score+=70;       // Linux 1.2
            }

            if ((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70)))
            {
                score_info.score+=80;       // Linux 1.2
            }
            if (((temp_window <= 65535) & (temp_window >= (65535 - 70))) |
                ((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70))) |
                ((temp_window <= (32767 + 70)) & (temp_window >= (32767 - 70))) |
                ((temp_window <= (37300 + 70)) & (temp_window >= (37300 - 70))) |
                ((temp_window <= (46080 + 70)) & (temp_window >= (46080 - 70))) |
                ((temp_window <= (60352 + 70)) & (temp_window >= (60352 - 70))) |
                ((temp_window <= (tcp_mss * 44) + 70) & (temp_window >= (tcp_mss * 44) - 70 )) |
                ((temp_window <= (tcp_mss * 4) + 70) & (temp_window >= (tcp_mss * 4) - 70 )) |
                ((temp_window <= (tcp_mss * 6) + 70) & (temp_window >= (tcp_mss * 6) - 70 )) |
                ((temp_window <= (tcp_mss * 12) + 70) & (temp_window >= (tcp_mss * 12) - 70 )) |
                ((temp_window <= (tcp_mss * 16) + 70) & (temp_window >= (tcp_mss * 16) - 70 )) |
                ((temp_window <= (tcp_mss * 26) + 70) & (temp_window >= (tcp_mss * 26) - 70 )))
            {
                score_info.score+=90;       // Linux 1.2
            }

            if ((temp_window <= (44620 + 70)) & (temp_window >= (44620 - 70)))
            {
                score_info.score+=100;       // Linux 1.2
            }

            if ((temp_window <= (64512 + 70)) & (temp_window >= (64512 - 70)))
            {
                score_info.score+=110;       // Linux 1.2
            }

            if ((temp_window <= (64512 + 70)) & (temp_window >= (64512 - 70)))
            {
                score_info.score+=120;       // Linux 1.2
            }

            if (((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70))) |
                ((temp_window <= (tcp_mss * 6) + 70) & (temp_window >= (tcp_mss * 6) - 70 )))
            {
                score_info.score+=130;       // Linux 1.2
            }

            if (((temp_window <= (64512 + 70)) & (temp_window >= (64512 - 70))) |
                    ((temp_window <= (tcp_mss * 44) + 70) & (temp_window >= (tcp_mss * 44) - 70 )))
            {
                score_info.score+=140;       // Linux 1.2
            }

            if (((temp_window <= 65535) & (temp_window >= (65535 - 70))) |
                ((temp_window <= (40320 + 70)) & (temp_window >= (40320 - 70))) |
                ((temp_window <= (32767 + 70)) & (temp_window >= (32767 - 70))) |
                ((temp_window <= (tcp_mss * 45) + 70) & (temp_window >= (tcp_mss * 45) - 70 )))
            {
                score_info.score+=150;       // Linux 1.2
            }

            if (((temp_window <= 65535) & (temp_window >= (65535 - 70))) |
                ((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70))) |
                ((temp_window <= (64512 + 70)) & (temp_window >= (64512 - 70))) |
                ((temp_window <= (32767 + 70)) & (temp_window >= (32767 - 70))) |
                ((temp_window <= (tcp_mss * 45) + 70) & (temp_window >= (tcp_mss * 45) - 70 )) |
                ((temp_window <= (tcp_mss * 44) + 70) & (temp_window >= (tcp_mss * 44) - 70 )) |
                ((temp_window <= (tcp_mss * 12) + 70) & (temp_window >= (tcp_mss * 12) - 70 )))
            {
                score_info.score+=160;       // Linux 1.2
            }

            if (((temp_window <= 65535) & (temp_window >= (65535 - 70))) |
                ((temp_window <= (32768 + 70)) & (temp_window >= (32768 - 70))) |
                ((temp_window <= (16384 + 70)) & (temp_window >= (16384 - 70))))
            {
                score_info.score+=170;       // Linux 1.2
            }

            if ((temp_window <= (8192 + 70)) & (temp_window >= (8192 - 70)))
            {
                score_info.score+=180;       // Linux 1.2
            }


            if ((temp_window <= (16616 + 70)) & (temp_window >= (16616 - 70)))
            {
                score_info.score+=190;       // Linux 1.2
            }

            if (((temp_window <= (tcp_mss * 2) + 70) & (temp_window >= (tcp_mss * 2) - 70 )))
            {
                score_info.score+=200;       // Linux 1.2
            }

            if ((temp_window <= (32768 + 70)) & (temp_window >= (32768 - 70)))
            {
                score_info.score+=210;       // Linux 1.2
            }

            if (((temp_window <= (32768 + 70)) & (temp_window >= (32768 - 70))) |
                    ((temp_window <= 65535) & (temp_window >= (65535 - 70))))
            {
                score_info.score+=220;       // Linux 1.2
            }

            if ((temp_window <= (33304 + 70)) & (temp_window >= (33304 - 70)))
            {
                score_info.score+=230;       // Linux 1.2
            }

            score_info.os_matrix.window_size = true;
        }


        if(score_info.os_matrix.syn_pkt_size == false)
        {
            if (htons(rx_ip_hdr->ip_len) == 44)
            {
                score_info.score+=100;       // Linux 1.2
            }
            else if (htons(rx_ip_hdr->ip_len) == 48)
            {
                score_info.score+=200;       // Linux 1.2
            }
            else if (htons(rx_ip_hdr->ip_len) == 52)
            {
                score_info.score+=300;       // Linux 1.2
            }
            else if (htons(rx_ip_hdr->ip_len) == 60)
            {
                score_info.score+=400;       // Linux 1.2
            }

            score_info.os_matrix.syn_pkt_size = true;
        }


        if(score_info.os_matrix.options_order == false)
        {
            option_index = 0;
            end_of_options = false;

            char* options = ((char*)(malloc(sizeof(char) * 20)));
            memset(options, 0, 20);

            while((end_of_options == false) & (option_index < length_tcp_options))
            {
                switch(packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index])
                {
                    case TCPOPT_EOL:
                        end_of_options = true;
                        break;

                    case TCPOPT_NOP:
                        strcat(options, "N");
                        option_index++;
                        break;

                    case TCPOPT_MAXSEG:
                        strcat(options, "M");
                        option_index += packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index + 1];
                        break;

                    case TCPOPT_WSCALE:
                        strcat(options, "W");
                        option_index += packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index + 1];
                        break;

                    case TCPOPT_SACKOK:
                        strcat(options, "S");
                        option_index += packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index + 1];
                        break;

                    case TCPOPT_TIMESTAMP:
                        strcat(options, "T");
                        option_index += packet[MAC_HEADER_LEN + length_ip_header + 20 + option_index + 1];
                        break;
                }
            }

            if (strcmp(options, "M") == 0)
            {
                score_info.score+=1000;       // Linux 1.2
            }
            else if (strcmp(options, "MSTNW") == 0)
            {
                score_info.score+=2000;       // Linux 1.2
            }

            if (strcmp(options, "MNWNNTNNS") == 0)
            {
                score_info.score+=3000;       // Linux 1.2
            }

            if ((strcmp(options, "MNNS") == 0) | (strcmp(options, "MNWNNTNNS") == 0) | (strcmp(options, "MNWNNS") == 0))
            {
                score_info.score+=4000;       // Linux 1.2
            }

            if (strcmp(options, "MNNS") == 0)
            {
                score_info.score+=5000;       // Linux 1.2
            }
            if ((strcmp(options, "MNWNNS") == 0) | (strcmp(options, "MNNS") == 0))
            {
                score_info.score+=6000;       // Linux 1.2
            }

            if (strcmp(options, "MWNNNS") == 0)
            {
                score_info.score+=7000;       // Linux 1.2
            }


            if (strcmp(options, "MW") == 0)
            {
                score_info.score+=8000;       // Linux 1.2
            }

            if (strcmp(options, "MNNN") == 0)
            {
                score_info.score+=9000;       // Linux 1.2
            }

            if (strcmp(options, "MWN") == 0)
            {
                score_info.score+=10000;       // Linux 1.2
            }

            if (strcmp(options, "MNNNN") == 0)
            {
                score_info.score+=11000;       // Linux 1.2
            }

            if (strcmp(options, "MNWNNT") == 0)
            {
                score_info.score+=12000;       // Linux 1.2
            }

            score_info.os_matrix.options_order = true;
        }
        if (score_info.os_matrix.ipid == false)
        {
            if (htons(rx_ip_hdr->ip_id) != 0)       //IPID != 0
            {
                score_info.score+=10000;       // Linux 1.2
            }
            else if (htons(rx_ip_hdr->ip_id) == 0)  //IPID = 0
            {
                score_info.score+=20000;       // Linux 1.2
            }

            score_info.os_matrix.ipid = true;
        }
    }
    
}//end processing_tCP

int ConnectWait(int sockfd, struct sockaddr *saddr, int addrsize, int sec) 
{ 
    int newSockStat; 
    int orgSockStat; 
    int res, n; 
    fd_set  rset, wset; 
    struct timeval tval; 

    int error = 0; 
    int esize; 

    if ( (newSockStat = fcntl(sockfd, F_GETFL, NULL)) < 0 )  
    { 
        perror("F_GETFL error"); 
        return -1; 
    } 

    orgSockStat = newSockStat; 
    newSockStat |= O_NONBLOCK; 

    // Non blocking 상태로 만든다.  
    if(fcntl(sockfd, F_SETFL, newSockStat) < 0) 
    { 
        perror("F_SETLF error"); 
        return -1; 
    } 

    // 연결을 기다린다. 
    // Non blocking 상태이므로 바로 리턴한다. 
    if((res = connect(sockfd, saddr, addrsize)) < 0) 
    { 
        if (errno != EINPROGRESS) 
            return -1; 
    } 

    // 즉시 연결이 성공했을 경우 소켓을 원래 상태로 되돌리고 리턴한다.  
    if (res == 0) 
    { 
        printf("Connect Success\n"); 
        fcntl(sockfd, F_SETFL, orgSockStat); 
        return 1; 
    } 

    FD_ZERO(&rset); 
    FD_SET(sockfd, &rset); 
    wset = rset; 

    tval.tv_sec        = sec;     
    tval.tv_usec    = 0; 

    if ( (n = select(sockfd+1, &rset, &wset, NULL, &tval)) == 0) 
    { 
        // timeout 
        errno = ETIMEDOUT;     
        return -1; 
    }

    // 읽거나 쓴 데이터가 있는지 검사한다.  
    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset) ) 
    { 
        printf("Read data\n"); 
        esize = sizeof(int); 
        if ((n = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&esize)) < 0) 
            return -1; 
    } 
    else 
    { 
        perror("Socket Not Set"); 
        return -1; 
    } 


    fcntl(sockfd, F_SETFL, orgSockStat); 
    if(error) 
    { 
        errno = error; 
        perror("Socket"); 
        return -1; 
    } 

    return 1; 
} 

int port_scan(char *source_ip,char *ip_address, int port, char *scan_result) {

    struct sockaddr_in serveraddr; 
    int sockfd; 
    int len; 

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) 
    { 
        perror("error"); 
        return 1; 
    } 
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(ip_address);

    len = sizeof(serveraddr); 

    if (ConnectWait(sockfd, (struct sockaddr *)&serveraddr, len, 2) < 0) 
    { 
        strcpy(scan_result, "closed");
    } 
    else 
    { 
        strcpy(scan_result, "open");
    }

    close(sockfd);
    return 1; 
} 




void processing_ip(uint8_t * packet, int length)
{
    
    struct ip_hdr* rx_ip_hdr = ((struct ip_hdr*)(packet + 14));

    if (score_info.os_matrix.ttl ==false)
    {
        if ((rx_ip_hdr->ip_ttl > 0) & (rx_ip_hdr->ip_ttl <= 32))
        {
            score_info.score +=0.1;
        }
        else if ((rx_ip_hdr->ip_ttl > 32) & (rx_ip_hdr->ip_ttl <= 64))
        {
            score_info.score +=0.2;
        }
        else if ((rx_ip_hdr->ip_ttl > 64) & (rx_ip_hdr->ip_ttl <= 128))
        {
            score_info.score +=0.3;
        }
        else if ((rx_ip_hdr->ip_ttl > 128) & (rx_ip_hdr->ip_ttl <= 255))
        {
            score_info.score +=0.4;
        }

        score_info.os_matrix.ttl = true;
    }


    if (score_info.os_matrix.df == false)
    {
        if ((htons(rx_ip_hdr->ip_off) & 0x4000) == 0x0000)      //DF = 0
        {
            score_info.score +=0.1;
        }
        else if ((htons(rx_ip_hdr->ip_off) & 0x4000) == 0x4000)     //DF = 1
        {
            score_info.score +=0.2;
        }

        score_info.os_matrix.df = true;
    }


}//end Process_IP


void get_local_ip() 
{
    char output[100];
    FILE *p = popen("ifconfig eth0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'", "r");

    if(p != NULL) 
	{
        while(fgets(output, sizeof(output), p) != NULL);
        memcpy(source_ip, output, 24);

        inet_aton(source_ip, &local_ip);

    }
    pclose(p);
}





u_int16_t cal_checksum(u_int16_t *ptr, int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum = 0;
    while(nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if(nbytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }

    sum = (sum>>16) + (sum & 0xffff);
    sum = sum + (sum>>16);
    answer = (short)~sum;

    return answer;
}


int safe_atoi(const char *row)
{
        if (row) {
            return atoi(row);
        }else {
                return 0;
            }
}

