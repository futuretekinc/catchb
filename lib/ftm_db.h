#ifndef	FTM_DB_H_
#define	FTM_DB_H_

#include <sqlite3.h>
#include "ftm_types.h"
#include "ftm_cctv.h"
#include "ftm_switch.h"
#include "ftm_alarm.h"
#include "ftm_log.h"
#include "ftm_trace.h"
#include "ftm_utils.h"

#define	FTM_DB_QUERY_LEN	1024

typedef	struct	FTM_DB_CONFIG_STRUCT
{
	FTM_CHAR	pFileName[FTM_PATH_LEN + FTM_NAME_LEN + 1];
}	FTM_DB_CONFIG, _PTR_ FTM_DB_CONFIG_PTR;


FTM_RET	FTM_DB_CONFIG_setDefault
(
	FTM_DB_CONFIG_PTR	pConfig
);

FTM_RET	FTM_DB_CONFIG_load
(
	FTM_DB_CONFIG_PTR	pConfig,
	cJSON _PTR_ 		pRoot
);

FTM_RET	FTM_DB_CONFIG_save
(
	FTM_DB_CONFIG_PTR	pConfig,
	cJSON _PTR_ 		pRoot
);

FTM_RET	FTM_DB_CONFIG_show
(
	FTM_DB_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL		xLevel
);

/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////
typedef	struct	FTM_DB_STRUCT
{
	FTM_DB_CONFIG	xConfig;

	FTM_CHAR	pCCTVTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pAlarmTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pLogTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pDenyTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pSwitchTableName[FTM_NAME_LEN+1];
	FTM_CHAR	pStatisticsTableName[FTM_NAME_LEN+1];
	
	FTM_LOCK_PTR	pLock;
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
	FTM_DB_PTR 		pDB
);

FTM_RET	FTM_DB_close
(
	FTM_DB_PTR 		pDB
);

FTM_RET	FTM_DB_setConfig
(
	FTM_DB_PTR	pDB,
	FTM_DB_CONFIG_PTR	pConfig
);

FTM_RET	FTM_DB_getConfig
(
	FTM_DB_PTR	pDB,
	FTM_DB_CONFIG_PTR	pConfig
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
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pComment,
	FTM_UINT32		ulTime
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
	FTM_UINT32			ulMaxCount,
	FTM_CCTV_CONFIG_PTR	pCCTVs,
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

FTM_RET	FTM_DB_setCCTVProperties
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
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
	FTM_DB_PTR		pDB,
	FTM_ALARM_PTR	pAlarms
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

FTM_RET	FTM_DB_getLogInfo
(
	FTM_DB_PTR 		pDB, 
	FTM_UINT32_PTR 	pCount,
	FTM_UINT32_PTR 	pFirstTime,
	FTM_UINT32_PTR 	pLastTime
);

FTM_RET	FTM_DB_addLog
(
	FTM_DB_PTR		pDB,
	FTM_LOG_TYPE	xType,
	FTM_UINT32		ulTime,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_CHAR_PTR	pSign
);

FTM_RET	FTM_DB_deleteLogOfID
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_deleteLog
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_DB_deleteLogFrom
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_DB_getLogList
(
	FTM_DB_PTR		pDB,
	FTM_LOG_TYPE	xType,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_LOG_PTR		pLogs,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_getLogListFrom
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_LOG_PTR		pLogs,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_getLogListOfTimePeriod
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulMaxCount,
	FTM_LOG_PTR		pLogs,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_removeExpiredLog
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulExpirationDate
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
	FTM_BOOL		bSecure,
	FTM_CHAR_PTR	pComment	
);

FTM_RET	FTM_DB_deleteSwitch
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_DB_setSwitchProperties
(
	FTM_DB_PTR		pDB,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
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

FTM_RET	FTM_DB_createStatisticsTable
(
	FTM_DB_PTR		pDB
);

FTM_RET	FTM_DB_isStatisticsTableExist
(
	FTM_DB_PTR		pDB,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTM_DB_getStatisticsCount
(
	FTM_DB_PTR 		pDB, 
	FTM_UINT32_PTR 	pCount
);

FTM_RET	FTM_DB_getStatisticsInfo
(
	FTM_DB_PTR 		pDB, 
	FTM_UINT32_PTR 	pCount,
	FTM_UINT32_PTR 	pFirstTime,
	FTM_UINT32_PTR 	pLastTime
);

FTM_RET	FTM_DB_addStatistics
(
	FTM_DB_PTR		pDB,
	FTM_STATISTICS_PTR	pStatistics
);

FTM_RET	FTM_DB_deleteStatistics
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_DB_deleteStatisticsFrom
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_DB_getStatisticsList
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pStatistics,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_getStatisticsListFrom
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pStatistics,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_getStatisticsListOfTimePeriod
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulStartTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pStatistics,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_DB_removeExpiredStatistics
(
	FTM_DB_PTR		pDB,
	FTM_UINT32		ulExpirationDate
);

#endif
