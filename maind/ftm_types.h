#ifndef	FTM_TYPES_H_
#define	FTM_TYPES_H_

#include <stdint.h>
#include <sys/types.h>

#define	FTM_ID_LEN				24
#define	FTM_IP_LEN				24
#define	FTM_USER_LEN			24
#define	FTM_PASSWD_LEN			24
#define	FTM_COMMENT_LEN			64
#define	FTM_NAME_LEN			32
#define	FTM_TIME_LEN			32
#define	FTM_HASH_LEN			256
#define	FTM_LOG_LEN				256
#define	FTM_PATH_LEN			1024
#define	FTM_FILE_NAME_LEN		256
#define	FTM_EMAIL_LEN			128
#define	FTM_ALARM_MESSAGE_LEN	128

#define	_PTR_	*

#define	FTM_RET	unsigned int

#define	FTM_VOID		void
#define	FTM_VOID_PTR	void *
#define	FTM_FALSE	0
#define	FTM_TRUE		!(FTM_FALSE)

#define	FTM_SOCKET	int
typedef	unsigned char	FTM_BOOL,	_PTR_ FTM_BOOL_PTR;
typedef	char			FTM_CHAR,	_PTR_ FTM_CHAR_PTR;
typedef	unsigned char	FTM_UINT8,	_PTR_ FTM_UINT8_PTR;
typedef	int				FTM_INT,	_PTR_ FTM_INT_PTR;
typedef	uint16_t		FTM_UINT16, _PTR_ FTM_UINT16_PTR;
typedef	uint32_t		FTM_UINT32, _PTR_ FTM_UINT32_PTR;
typedef	int64_t			FTM_INT64, 	_PTR_ FTM_INT64_PTR;
typedef	uint64_t		FTM_UINT64, _PTR_ FTM_UINT64_PTR;
typedef	float			FTM_FLOAT,	_PTR_ FTM_FLOAT_PTR;

typedef	FTM_CHAR	FTM_ID[FTM_ID_LEN+1];

typedef	FTM_ID _PTR_	FTM_ID_PTR;
typedef	struct	FTM_ALARM_STRUCT
{
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pEmail[FTM_EMAIL_LEN+1];
	FTM_CHAR	pMessage[FTM_ALARM_MESSAGE_LEN+1];
}	FTM_ALARM, _PTR_ FTM_ALARM_PTR;

typedef	struct	FTM_LOG_STRUCT
{
	FTM_INT	nIndex;
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_CHAR	pLog[FTM_LOG_LEN+1];
	FTM_CHAR	pTime[FTM_TIME_LEN+1];
	FTM_INT	nStatus;
}	FTM_LOG, _PTR_ FTM_LOG_PTR;

typedef	struct	FTM_SWITCH_STRUCT
{
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_CHAR	pUser[FTM_USER_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN+1];
	FTM_CHAR	pComment[FTM_COMMENT_LEN+1];
}	FTM_SWITCH, _PTR_ FTM_SWITCH_PTR;

typedef	struct	FTM_DENY_STRUCT
{
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_UINT32	nIndex;
	FTM_CHAR	pSwitchID[FTM_ID_LEN+1];
}	FTM_DENY, _PTR_ FTM_DENY_PTR;

typedef	struct	FTM_OS_MATRIX_STRUCT
{
    FTM_BOOL 	ttl;
    FTM_BOOL 	df;
    FTM_BOOL 	tcp_mss;
    FTM_BOOL 	window_scale;
    FTM_BOOL 	window_size;
    FTM_BOOL 	syn_pkt_size;
    FTM_BOOL 	options_order;
    FTM_BOOL 	ipid;
}	FTM_OS_MATRIX, _PTR_ FTM_OS_MATRIX_PTR;

typedef struct FTM_IP_INFO_STRUCT
{
    FTM_INT		ck_id;
    FTM_INT		ck_cctv_idx;
    FTM_CHAR	ck_cctv_id[24];
    FTM_CHAR	ck_ip[24];
    FTM_CHAR	ck_signature[128];
}	FTM_IP_INFO, _PTR_ FTM_IP_INFO_PTR;

typedef struct FTM_SCORE_INFO_STRUCT
{
    pid_t 		tid;
    FTM_FLOAT	score;
    FTM_CHAR	analysis_cctv_ip[24];
    //pcap_t* 	descr;
    FTM_OS_MATRIX os_matrix;
}	FTM_SCORE_INFO, _PTR_ FTM_SCORE_INFO_PTR;

#define	FTM_RET_OK						0
#define	FTM_RET_ERROR					1
#define	FTM_RET_NOT_ENOUGH_MEMORY		2
#define	FTM_RET_SEMAPHORE_INIT_FAILED	3	
#define	FTM_RET_OBJECT_NOT_FOUND			4
#define	FTM_RET_LIST_NOT_INSERTABLE		5
#define	FTM_RET_INVALID_ARGUMENTS		6	
#define	FTM_RET_LIST_EMPTY				7	
#define	FTM_RET_NOT_INITIALIZED			8	
#define	FTM_RET_CONFIG_LOAD_FAILED		9 
#define	FTM_RET_FAILED_TO_READ_FILE		10
#define	FTM_RET_CONFIG_INVALID_OBJECT	11
#define	FTM_RET_INVALID_JSON_FORMAT		12
#define	FTM_RET_ALREADY_RUNNING			13
#define	FTM_RET_SOCKET_CREATION_FAILED	14
#define	FTM_RET_SOCKET_ATTRIBUTE_SET_FAILED	15
#define	FTM_RET_SOCKET_BIND_FAILED		16
#define	FTM_RET_SOCKET_LISTEN_FAILED		17
#define	FTM_RET_SOCKET_ACCEPT_FAILED		18
#define	FTM_RET_SOCKET_ABNORMAL_DISCONNECTED	19
#define	FTM_RET_DB_ALREADY_EXIST		21	
#define	FTM_RET_DB_ALREADY_OPENED		22	
#define	FTM_RET_DB_OPEN_FAILED			23	
#define	FTM_RET_DB_EXEC_ERROR			24
#define	FTM_RET_NET_INTERFACE_ERROR		25	
#define	FTM_RET_SOCKET_STAT_GET_FAILED	26
#define	FTM_RET_SOCKET_STAT_SET_FAILED	27
#define	FTM_RET_SOCKET_CONNECTION_FAILED	28
#define	FTM_RET_SOCKET_CONNECTION_TIMEOUT	29
#define	FTM_RET_TIMEOUT					30
#define	FTM_RET_THREAD_CREATION_FAILED	31
#define	FTM_RET_FILE_OPEN_FAILED		32
#define	FTM_RET_EXECUTE_FAILED			33
#define	FTM_RET_INVALID_FORMAT			34	
#define	FTM_RET_THREAD_CREATION_ERROR	35
#define	FTM_RET_NOT_SUPPORTED_MESSAGE	36	

#define	FTM_RET_SHELL_QUIT				100
#define	FTM_RET_INVALID_COMMAND			101
#endif
