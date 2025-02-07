#ifndef	FTM_RET_H_
#define	FTM_RET_H_

#define	FTM_RET_OK								0
#define	FTM_RET_ERROR							1
#define	FTM_RET_NOT_ENOUGH_MEMORY				2
#define	FTM_RET_SEMAPHORE_INIT_FAILED			3
#define	FTM_RET_OBJECT_NOT_FOUND				4
#define	FTM_RET_LIST_NOT_INSERTABLE				5
#define	FTM_RET_INVALID_ARGUMENTS				6
#define	FTM_RET_LIST_EMPTY						7
#define	FTM_RET_NOT_INITIALIZED					8
#define	FTM_RET_CONFIG_LOAD_FAILED				9
#define	FTM_RET_FAILED_TO_READ_FILE				10
#define	FTM_RET_CONFIG_INVALID_OBJECT			11
#define	FTM_RET_INVALID_JSON_FORMAT				12
#define	FTM_RET_ALREADY_RUNNING					13
#define	FTM_RET_SOCKET_CREATION_FAILED			14
#define	FTM_RET_SOCKET_ATTRIBUTE_SET_FAILED		15
#define	FTM_RET_SOCKET_BIND_FAILED				16
#define	FTM_RET_SOCKET_LISTEN_FAILED			17
#define	FTM_RET_SOCKET_ACCEPT_FAILED			18
#define	FTM_RET_SOCKET_ABNORMAL_DISCONNECTED	19
#define	FTM_RET_DB_ALREADY_EXIST				21	
#define	FTM_RET_DB_ALREADY_OPENED				22	
#define	FTM_RET_DB_OPEN_FAILED					23	
#define	FTM_RET_DB_EXEC_ERROR					24
#define	FTM_RET_NET_INTERFACE_ERROR				25	
#define	FTM_RET_SOCKET_STAT_GET_FAILED			26
#define	FTM_RET_SOCKET_STAT_SET_FAILED			27
#define	FTM_RET_SOCKET_CONNECTION_FAILED		28
#define	FTM_RET_SOCKET_CONNECTION_TIMEOUT		29
#define	FTM_RET_TIMEOUT							30
#define	FTM_RET_THREAD_CREATION_FAILED			31
#define	FTM_RET_FILE_OPEN_FAILED				32
#define	FTM_RET_EXECUTE_FAILED					33
#define	FTM_RET_INVALID_FORMAT					34	
#define	FTM_RET_THREAD_CREATION_ERROR			35
#define	FTM_RET_NOT_SUPPORTED_MESSAGE			36	
#define	FTM_RET_FUNCTION_NOT_SUPPORTED			37
#define	FTM_RET_CANT_CREATE_SEMAPHORE			38
#define	FTM_RET_INVALID_VALUE					39	

#define	FTM_RET_SHELL_QUIT						100
#define	FTM_RET_INVALID_COMMAND					101

#define	FTM_RET_PING_UNKNOWN_PROTOCOL_ICMP		110
#define	FTM_RET_PING_MUST_RUN_AS_ROOT			111
#define	FTM_RET_PING_SOCKET_ERROR 				112
#define	FTM_RET_PING_UNKNOWN_HOST				113
#define	FTM_RET_INCOMPATIBLE_OPTIONS			114
#define	FTM_RET_PING_RECORD_ROUTE_NOT_AVAILABLE				115
#define	FTM_RET_PING_CANT_DISABLE_MULTICAST_LOOPBACK		116
#define	FTM_RET_PING_CANT_SET_MULTICAST_TIME_TO_LIVE		117
#define	FTM_RET_PING_CANT_SET_MULTICAST_SOURCE_INTERFACE	118
#define	FTM_RET_PACKET_TOO_SHORT				119
#define	FTM_RET_OBJECT_ALREADY_EXIST			120
#define	FTM_RET_DB_TABLE_NOT_EXIST				121	
#define	FTM_RET_SEND_MAIL_ERROR					122
#define	FTM_RET_BUFFER_TOO_SMALL				123	
#define	FTM_RET_SMTPC_ERROR						124
#define	FTM_RET_SOCKET_CLOSED					125
#define	FTM_RET_NOT_SUPPORTED_FUNCTION			126

#define	FTM_RET_SSH_AUTH_ERROR						130
#define	FTM_RET_SSH_AUTH_DENIED						131
#define	FTM_RET_SSH_FAILED_TO_SET_OPTION			132
#define	FTM_RET_SSH_CONNECTION_FAILED				133
#define	FTM_RET_SSH_DISCONNECT_FAILED				134
#define	FTM_RET_SSH_FAILED_TO_GET_PUBLICKEY			135
#define	FTM_RET_SSH_FAILED_TO_GET_PUBLICKEY_HASH	136
#define	FTM_RET_SSH_SERVER_ERROR					137
#define	FTM_RET_SSH_FAILED_TO_SAVE_KEY				138
#define	FTM_RET_SSH_SERVER_FOUND_OTHER				139
#define	FTM_RET_SSH_SERVER_KNOWN_CHANGED			140
#define	FTM_RET_SSH_FAILED_TO_OPEN_CHANNEL			141
#define	FTM_RET_SSH_WRITE_FAILED					142	
#define	FTM_RET_SSH_READ_FAILED						143	

#define	FTM_RET_PCAP_ERROR							150
#define	FTM_RET_PCAP_OPEN_FAILED					151
#define	FTM_RET_PCAP_NOT_OPENED						152

#define	FTM_RET_CLIENT_ERROR						170	
#define	FTM_RET_CLIENT_TIMEOUT						171	
#define	FTM_RET_CLIENT_HANDLE_INVALID				172	

#define	FTM_RET_INVALID_SSID						173	
#define	FTM_RET_CLIENT_INVALID_SSID					173	
#endif

