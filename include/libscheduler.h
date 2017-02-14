#ifndef	LIB_SCHEDULER_H_
#define LIB_SCHEDULER_H_

int free_static_url_information_list();
void initialize_os_matrix();
bool ip_check(char *ip);
int db_cctv_info_select(int * t_num);
int db_cctv_info_delete();
int db_cctv_info_insert(int cctv_idx, char *cctv_id, char *ip, char *hash_value, int code);
void send_http_request(int port_number, char *analysis_cctv_ip);
void clean_up(void *id);
void* thread_packet_send_operation(void *id);
void* thread_main_operation(void *id);
void sniffer(u_char* useless, const struct pcap_pkthdr* pkthdr, const u_char* packet);
void processing_tcp(uint8_t * packet, int length);
void processing_ip(uint8_t * packet, int length);
int ConnectWait(int sockfd, struct sockaddr *saddr, int addrsize, int sec); 
int port_scan(char *source_ip,char *ip_address, int port, char *scan_result);
void get_local_ip();
u_int16_t cal_checksum(u_int16_t *ptr, int nbytes);
int safe_atoi(const char *row);


#endif
