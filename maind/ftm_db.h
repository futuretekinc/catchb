#ifndef	FTM_DB_H_
#define	FTM_DB_H_

#include <sqlite3.h>
#include "ftm_types.h"
#include "ftm_cctv.h"
#include "ftm_switch.h"

#define	FTM_DB_QUERY_LEN	1024

typedef	struct	FTM_DB_STRUCT
{
	FTM_CHAR	pCCTVTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pAlarmTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pLogTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pDenyTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pSwitchTableName[FTM_NAME_LEN+1];

	sqlite3 _PTR_	pSQLite3;
}	FTM_DB, _PTR_ FTM_DB_PTR;

FTM_RET	FTM_DB_create
(
	FTM_DB_PTR _PTR_ ppDB
);

FTM_RET	FTM_DB_destroy
(
	FTM_DB_PTR _PTR_ ppDB
);

FTM_RET	FTM_DB_open
(
	FTM_DB_PTR 		pDB, 
	FTM_CHAR_PTR 	pFileName
);

FTM_RET	FTM_DB_close
(
	FTM_DB_PTR 		pDB
);

FTM_RET FTM_DB_isExistTable
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR    pTableName, 
	FTM_BOOL_PTR    pExist
);

FTM_RET	FTM_DB_CCTV_createTable
(
	FTM_DB_PTR		pDB
);

FTM_RET	FTM_DB_CCTV_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_CCTV_count
(
	FTM_DB_PTR 		pDB, 
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_CCTV_insert
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pComment,
	FTM_CHAR_PTR	pTime
);

FTM_RET	FTM_DB_CCTV_update
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pComment,
	FTM_CHAR_PTR	pHash
);

FTM_RET	FTM_DB_CCTV_delete
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_CCTV_getList
(
	FTM_DB_PTR			pDB,
	FTM_CCTV_CONFIG_PTR	pCCTVs,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_DB_CCTV_getUsingIP
(
	FTM_DB_PTR			pDB,
	FTM_CHAR_PTR		pIP,
	FTM_CCTV_CONFIG_PTR	pCCTV
);

FTM_RET	FTM_DB_CCTV_hashUpdated
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
);

FTM_RET	FTM_DB_CCTV_setStat
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_STAT	xStat
);

FTM_RET	FTM_DB_ALARM_createTable
(
	FTM_DB_PTR	pDB
);

FTM_RET	FTM_DB_ALARM_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_ALARM_count
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
);

FTM_RET	FTM_DB_ALARM_insert
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pMail
);

FTM_RET	FTM_DB_ALARM_delete
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_ALARM_getList
(
	FTM_DB_PTR			pDB,
	FTM_ALARM_PTR		pAlarms,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_DB_LOG_createTable
(
	FTM_DB_PTR		pDB
);

FTM_RET	FTM_DB_LOG_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_LOG_count
(
	FTM_DB_PTR 		pDB, 
	FTM_UINT32_PTR 	pCount
);

FTM_RET	FTM_DB_LOG_insert
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pSign,
	FTM_CHAR_PTR	pLog,
	FTM_CHAR_PTR	pTime,
	FTM_INT			nStatus
);

FTM_RET	FTM_DB_LOG_delete
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_LOG_getList
(
	FTM_DB_PTR		pDB,
	FTM_LOG_PTR		pLogs,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_SWITCH_createTable
(
	FTM_DB_PTR	pDB
);

FTM_RET	FTM_DB_SWITCH_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_SWITCH_count
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
);

FTM_RET	FTM_DB_SWITCH_add
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_TYPE	xType,
	FTM_CHAR_PTR	pUser,
	FTM_CHAR_PTR	pPasswd,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pComment	
);

FTM_RET	FTM_DB_SWITCH_delete
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_SWITCH_getList
(
	FTM_DB_PTR		pDB,
	FTM_SWITCH_CONFIG_PTR	pElements,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_DENY_createTable
(
	FTM_DB_PTR		pDB
);

FTM_RET	FTM_DB_DENY_isTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_DENY_count
(
	FTM_DB_PTR		pDB,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_DENY_insert
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pIP,
	FTM_UINT32		nIndex,
	FTM_CHAR_PTR	pSwitchID
);

FTM_RET	FTM_DB_DENY_delete
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pIP
);

FTM_RET	FTM_DB_DENY_get
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pIP,
	FTM_DENY_PTR	pDeny
);

FTM_RET	FTM_DB_DENY_getList
(
	FTM_DB_PTR		pDB,
	FTM_DENY_PTR	pElements,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount
);
#endif
