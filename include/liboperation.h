#ifndef __LIBOPERATION_H__
#define __LIBOPERATION_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <mysql/mysql.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h> // multi-thread

#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h> //ip hdeader library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> //icmp header
#include <net/if.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h> //internet address library
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ifaddrs.h>
#include <sys/syscall.h>

#include <pcap.h>
#include <errno.h>
#include <stdbool.h>

#include "linux_list.h"

#define STATIC_FOLDER_NUM 3
#define STATIC_FOLDER_LENG 256


#define STATIC_FILE_NUM 2
#define STATIC_FILE_LENG 32

#define CCTV_NORMALITY_TTL 128

#define MAC_HEADER_LEN 14  

#define MAX_THREAD 20

#define ADDR_LENG 24

#define SYN 0
#define NUL 1
#define FIN 2
#define XMAS 3
#define ACK 4
#define UDP 5

#define DATAGRAM_SIZE 4096
#define LOCAL_PORT 54321
#define TRANS_TIMEOUT 1
#define RETRANS_TIMES 3
#define PORT_NUM 8

#define TCPOPT_EOL      0   /* End of options */
#define TCPOPT_NOP      1   /* Nothing */
#define TCPOPT_MAXSEG       2   /* MSS */
#define TCPOPT_WSCALE       3   /* Window scaling */
#define TCPOPT_SACKOK       4   /* Selective ACK permitted */
#define TCPOPT_TIMESTAMP        8   /* Stamp out timestamping! */


#ifndef IP_PROTO_ICMP
#define IP_PROTO_ICMP           1  /* ICMP protocol */
#endif

#ifndef IP_PROTO_TCP
#define IP_PROTO_TCP            6  /* TCP protocol */
#endif

#ifndef IP_PROTO_UDP
#define IP_PROTO_UDP            17 /* UDP protocol */
#endif


#define CHOP(x) x[strlen(x) -1 ] =' '
#define CCTV_NORMALITY_TTL 128

struct ip_hdr
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ip_hl:4;       /* header length */
    unsigned int ip_v:4;        /* version */
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int ip_v:4;        /* version */
    unsigned int ip_hl:4;       /* header length */
#else
#error "Byte ordering not specified "
#endif
    uint8_t ip_tos;         /* type of service */
    uint16_t ip_len;            /* total length */
    uint16_t ip_id;         /* identification */
    uint16_t ip_off;            /* fragment offset field */
#define IP_RF 0x8000            /* reserved fragment flag */
#define IP_DF 0x4000            /* dont fragment flag */
#define IP_MF 0x2000            /* more fragments flag */
#define IP_OFFMASK 0x1fff       /* mask for fragmenting bits */
    uint8_t ip_ttl;         /* time to live */
    uint8_t ip_p;           /* protocol */
    uint16_t ip_sum;            /* checksum */
    struct in_addr ip_src, ip_dst;  /* source and dest address */
} __attribute__ ((packed));



struct tcp_hdr
{
    uint16_t   tcp_src_prt;
    uint16_t   tcp_dst_prt;
    uint32_t   tcp_seq_num;
    uint32_t   tcp_ack_num;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int tcp_rsrvd:4;
    unsigned int tcp_dt_ofst:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int tcp_dt_ofst:4;
    unsigned int tcp_rsrvd:4;
#else
#error "Byte ordering not specified "
#endif
    uint8_t    tcp_flags;
    uint16_t   tcp_window;
    uint16_t   tcp_cksum;
    uint16_t   tcp_urg_ptr;
} __attribute__ ((packed));



typedef struct _packet_os_matrix{
    bool ttl;
    bool df;
    bool tcp_mss;
    bool window_scale;
    bool window_size;
    bool syn_pkt_size;
    bool options_order;
    bool ipid;
}CK_OS_MATRIX;

typedef struct _check_ip_infomation{
    int ck_id;
    int ck_cctv_idx;
    char ck_cctv_id[24];
    char ck_ip[24];
    char ck_signature[128];
    struct list_head list;
}CK_IP_INFO;

typedef struct _score_infomation{
    pid_t tid;
    float score;
    char analysis_cctv_ip[24];
    pcap_t* descr;
    CK_OS_MATRIX os_matrix;
}CK_SCORE_INFO;


struct tcp_pseudo_hdr {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t reserved;
    u_int8_t protocol;
    u_int16_t length;
    struct tcphdr tcp;
};

// udp checksum
struct udp_pseudo_hdr {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t reserved;
    u_int8_t protocol;
    u_int16_t length;
    struct udphdr udp;
};

#endif
