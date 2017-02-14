#ifndef __LIBDB_H__
#define __LIBDB_H__

#include <mysql/mysql.h>

extern const char* CREATE_DB ;
extern const char* CREATE_CHECK_RESULT_INFO;
extern const char* CREATE_SYSTEM_ADMIN_TABLE;
extern const char* CREATE_CHECK_ALARM_INFO;
extern const char* CREATE_CHECK_IP_INFORMATION;
extern const char* CREATE_CHECK_IP_RESULT_LOG;
extern const char* CREATE_CHECK_LINK_INFO;
extern const char* CREATE_CHECK_SWITCH_IP_DETECTION_INFO;
extern const char* CREATE_CHECK_SYSTEM_RESOURCE;
extern const char* CREATE_CHECK_UPDATE;
extern const char* CREATE_CHECK_CODE_INFO;
extern const char* INSERT_TB_CODEINFO_VALUE_1;
extern const char* INSERT_TB_CODEINFO_VALUE_2;
extern const char* INSERT_TB_CODEINFO_VALUE_3;
extern const char* INSERT_TB_CODEINFO_SWITCH_DASAN_VALUE;
extern const char* INSERT_TB_CODEINFO_SWITCH_JUNIPER_VALUE;
extern const char* INSERT_TB_CODEINFO_SWITCH_NST_VALUE;

#define WDB_SUCCESS 0
#define WDB_ERROR   1

typedef	struct	DB_STRUCT
{
	int	handle;
}	DB;

int db_open(DB *db);
int create_db();
int db_close(DB *db);
int init_create_table(DB *db);
void initdb();
int init_db_open(DB *db);
#endif 
