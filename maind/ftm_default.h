#ifndef	FTM_DEFAULT_H_
#define	FTM_DEFAULT_H_

#define	FTM_ID_LEN				64	
#define	FTM_IP_LEN				24
#define	FTM_USER_LEN			32
#define	FTM_PASSWD_LEN			32
#define	FTM_COMMENT_LEN			64
#define	FTM_NAME_LEN			32
#define	FTM_TIME_LEN			32
#define	FTM_HASH_LEN			256
#define	FTM_HOST_NAME_LEN		256
#define	FTM_LOG_LEN				256
#define	FTM_PATH_LEN			1024
#define	FTM_FILE_NAME_LEN		256
#define	FTM_EMAIL_LEN			128
#define	FTM_ALARM_MESSAGE_LEN	128
#define	FTM_COMMAND_LEN			256

#define	FTM_CATCHB_ANALYZER_DEFAULT_IP_CHECK_INTERVAL	60000
#define	FTM_CATCHB_ANALYZER_MINIMUM_IP_CHECK_INTERVAL	10000
#define	FTM_CATCHB_ANALYZER_DEFAULT_TEST_ENABLE			FTM_FALSE
#define	FTM_CATCHB_ANALYZER_DEFAULT_TEST_ERROR_RATE		10
#define	FTM_CATCHB_ANALYZER_MAX_PORT_COUNT				8

#define	FTM_CATCHB_DEFAULT_SMTP_SERVER			"smtp.cafe24.com"
#define	FTM_CATCHB_DEFAULT_SMTP_PORT			587
#define	FTM_CATCHB_DEFAULT_SMTP_USER_ID			"devweb@futuretek.co.kr"
#define	FTM_CATCHB_DEFAULT_SMTP_PASSWD			"futuretek1204"
#define	FTM_CATCHB_DEFAULT_SMTP_SENDER			"devweb@futuretek.co.kr"

#define	FTM_CATCHB_DB_DEFAULT_FILE_NAME			"./catchb.db"

#define	FTM_CATCHB_LOGGER_DEFAULT_RETENTION		30

#define FTM_CATCHB_TRACE_DEFAULT_TIME_FIELD_LEN			20
#define FTM_CATCHB_TRACE_DEFAULT_FUNCTION_FIELD_LEN		24
#define FTM_CATCHB_TRACE_DEFAULT_LINE_FIELD_LEN			4
#define FTM_CATCHB_TRACE_DEFAULT_MODULE_FIELD_LEN		8
#define FTM_CATCHB_TRACE_DEFAULT_LEVEL_FIELD_LEN		4

#define	FTM_CATCHB_TRACE_LOG_SIZE				(1024*1024)
#endif
