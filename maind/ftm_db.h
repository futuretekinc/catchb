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

FTM_RET	FTM_DB_createCCTVTable
(
	FTM_DB_PTR		pDB
);

FTM_RET	FTM_DB_isCCTVTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_getCCTVCount
(
	FTM_DB_PTR 		pDB, 
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_addCCTV
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pComment,
	FTM_CHAR_PTR	pTime
);

FTM_RET	FTM_DB_updateCCTV
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pComment,
	FTM_CHAR_PTR	pHash
);

FTM_RET	FTM_DB_deleteCCTV
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_getCCTVList
(
	FTM_DB_PTR			pDB,
	FTM_CCTV_CONFIG_PTR	pCCTVs,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_DB_getCCTVUsingIP
(
	FTM_DB_PTR			pDB,
	FTM_CHAR_PTR		pIP,
	FTM_CCTV_CONFIG_PTR	pCCTV
);

FTM_RET	FTM_DB_updateCCTVHash
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
);

FTM_RET	FTM_DB_setCCTVStat
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_STAT	xStat
);

FTM_RET	FTM_DB_createAlarmTable
(
	FTM_DB_PTR	pDB
);

FTM_RET	FTM_DB_isAlarmTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_getAlarmCount
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
);

FTM_RET	FTM_DB_addAlarm
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pMail,
	FTM_CHAR_PTR	pMessage
);

FTM_RET	FTM_DB_deleteAlarm
(
	FTM_DB_PTR	pDB,
	FTM_CHAR_PTR	pName
);

FTM_RET	FTM_DB_getAlarmList
(
	FTM_DB_PTR			pDB,
	FTM_ALARM_PTR		pAlarms,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
);

FTM_RET	FTM_DB_createLogTable
(
	FTM_DB_PTR		pDB
);

FTM_RET	FTM_DB_isLogTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_getLogCount
(
	FTM_DB_PTR 		pDB, 
	FTM_UINT32_PTR 	pCount
);

FTM_RET	FTM_DB_addLog
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pSign,
	FTM_CHAR_PTR	pLog,
	FTM_CHAR_PTR	pTime,
	FTM_INT			nStatus
);

FTM_RET	FTM_DB_deleteLog
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_getLogList
(
	FTM_DB_PTR		pDB,
	FTM_LOG_PTR		pLogs,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_createSwitchTable
(
	FTM_DB_PTR	pDB
);

FTM_RET	FTM_DB_isSwitchTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_getSwitchCount
(
	FTM_DB_PTR 	pDB, 
	FTM_UINT32_PTR pCount
);

FTM_RET	FTM_DB_addSwitch
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_MODEL	xModel,
	FTM_CHAR_PTR	pUser,
	FTM_CHAR_PTR	pPasswd,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pComment	
);

FTM_RET	FTM_DB_deleteSwitch
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_getSwitchList
(
	FTM_DB_PTR		pDB,
	FTM_SWITCH_CONFIG_PTR	pElements,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount
);


FTM_RET	FTM_DB_createACTable
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID
);

FTM_RET	FTM_DB_destroyACTable
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID
);

FTM_RET	FTM_DB_isACTableExist
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_getACCount
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_addAC
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pIP,
	FTM_UINT32		nIndex,
	FTM_SWITCH_AC_POLICY	xPolicy
);

FTM_RET	FTM_DB_deleteAC
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pIP
);

FTM_RET	FTM_DB_getACList
(
	FTM_DB_PTR			pDB,
	FTM_CHAR_PTR		pSwitchID,
	FTM_SWITCH_AC_PTR	pElements,
	FTM_UINT32			ulMaxCount,
	FTM_UINT32_PTR		pCount
);

#endif
