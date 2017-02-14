#ifndef __COMMON_H__
#define __COMMON_H__

#define CK_NAME_CCTV_MAIND        "cctv_maind"
#define CK_NAME_CCTV_OPERATION    "cctv_operation"
#define CK_NAME_CCTV_LOGSERV      "cctv_logserv"
#define CK_NAME_CCTV_ALARM        "cctv_alarm"
#define CK_NAME_CCTV_DETECTION    "cctv_detection"
#define CK_NAME_CCTV_DETECTSW    "cctv_detectsw"
#define CK_NAME_CCTV_LINK_SLOG    "cctv_link_slog"
#define CK_NAME_CCTV_ANALYSIS     "cctv_analysis"
#define CK_NAME_CCTV_SCHEDULER     "cctv_scheduler"

#define LT_CK_NAME_CCTV_MAIND        "lt-cctv_maind"
#define LT_CK_NAME_CCTV_OPERATION    "lt-cctv_operation"
#define LT_CK_NAME_CCTV_LOGSERV      "lt-cctv_logserv"
#define LT_CK_NAME_CCTV_ALARM        "lt-cctv_alarm"
#define LT_CK_NAME_CCTV_DETECTION    "lt-cctv_detection"
#define LT_CK_NAME_CCTV_DETECTSW    "lt-cctv_detectsw"
#define LT_CK_NAME_CCTV_LINK_SLOG    "lt-cctv_link_slog"
#define LT_CK_NAME_CCTV_ANALYSIS     "lt-cctv_analysis"
#define LT_CK_NAME_CCTV_SCHEDULER     "lt-cctv_scheduler"


#define SV_SOCK_CCTV_MAIND_PATH             "/tmp/cctv_maind"
#define SV_SOCK_CCTV_OPERATION_PATH         "/tmp/cctv_operation"
#define SV_SOCK_CCTV_ANALYSIS_PATH         "/tmp/cctv_analysis"
//#define SV_SOCK_CCTV_LOGSERV_PATH           "/tmp/cctv_logserv"
#define SV_SOCK_CCTV_ALARM_PATH             "/tmp/cctv_alarm"
#define SV_SOCK_CCTV_DETECTION_PATH         "/tmp/cctv_detection"
#define SV_SOCK_CCTV_DETECTSW_PATH         "/tmp/cctv_detectsw"
#define SV_SOCK_CCTV_LINK_SLOG_PATH         "/tmp/cctv_link_slog"
#define SV_SOCK_CCTV_SCHEDULER_PATH         "/tmp/cctv_scheduler"

#define CK_CCTV_MAIND_LOG_FILE_PATH "/var/log/c_maind.log"
#define CK_CCTV_OPERATION_LOG_FILE_PATH "/var/log/c_operation.log"
#define CK_CCTV_LOGSERV_LOG_FILE_PATH "/var/log/c_logserv.log"
#define CK_CCTV_ALARM_LOG_FILE_PATH "/var/log/c_alarm.log"
#define CK_CCTV_DETECTION_LOG_FILE_PATH "/var/log/c_detection.log"
#define CK_CCTV_DETECTSW_LOG_FILE_PATH "/var/log/c_detectsw.log"
#define CK_CCTV_LINK_SLOG_LOG_FILE_PATH "/var/log/c_link_slog.log"
#define CK_CCTV_ANALYSIS_LOG_FILE_PATH "/var/log/c_analysis.log"
#define CK_CCTV_SCHEDULER_LOG_FILE_PATH "/var/log/c_scheduler.log"

#define DB_HOST "127.0.0.1"
#define DB_USER "root"
#define DB_PASS "admin123"
#define DB_NAME "CCTV_CHECK_DB"
#define DB_PORT 3306

#endif 
