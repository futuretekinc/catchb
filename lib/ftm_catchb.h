#ifndef	FTM_CATCHB_H_
#define	FTM_CATCHB_H_

#include <pthread.h>
#include "ftm_types.h"
#include "ftm_msgq.h"
#include "ftm_db.h"
#include "ftm_config.h"
#include "ftm_detector.h"
#include "ftm_analyzer.h"
#include "ftm_notifier.h"
#include "ftm_logger.h"
#include "ftm_timer.h"
#include "ftm_server.h"

typedef	struct 	FTM_CATCHB_CONFIG_STRUCT
{
	struct
	{
		FTM_UINT32	ulUpdateInterval;
	}	xCCTV;

}	FTM_CATCHB_CONFIG, _PTR_ FTM_CATCHB_CONFIG_PTR;

typedef	struct	FTM_CATCHB_STRUCT
{
	FTM_CATCHB_CONFIG	xConfig;

	FTM_CHAR			pName[FTM_NAME_LEN+1];

	FTM_MSGQ_PTR		pMsgQ;	
	FTM_DB_PTR			pDB;	

	FTM_LIST_PTR		pCCTVList;
	FTM_LIST_PTR		pSwitchList;
	FTM_LIST_PTR		pAlarmList;

	FTM_ANALYZER_PTR	pAnalyzer;
	FTM_DETECTOR_PTR	pDetector;
	FTM_NOTIFIER_PTR	pNotifier;
	FTM_SERVER_PTR		pServer;
	FTM_LOGGER_PTR		pLogger;

	FTM_EVENT_TIMER_MANAGER_PTR	pEventManager;
		
	pthread_t		xThread;
	FTM_BOOL		bStop;
}	FTM_CATCHB, _PTR_ FTM_CATCHB_PTR;

FTM_RET	FTM_CATCHB_create
(
	FTM_CATCHB_PTR _PTR_ ppCatchB
);

FTM_RET	FTM_CATCHB_destroy
(
	FTM_CATCHB_PTR _PTR_ ppCatchB
);

FTM_RET	FTM_CATCHB_setConfig
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CATCHB_getConfig
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CATCHB_start
(
	FTM_CATCHB_PTR	pCatchB
);

FTM_RET	FTM_CATCHB_stop
(
	FTM_CATCHB_PTR	pCatchB
);

FTM_RET	FTM_CATCHB_waitingForFinished
(
	FTM_CATCHB_PTR	pCarchb
);

FTM_RET	FTM_CATCHB_addCCTV
(
	FTM_CATCHB_PTR		pCatchB,
	FTM_CCTV_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CATCHB_delCCTV
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_CATCHB_getCCTV
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_PTR _PTR_ pCCTV
);

FTM_RET	FTM_CATCHB_getCCTVProperties
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR pProperties
);

FTM_RET	FTM_CATCHB_setCCTVProperties
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR pProperties,
	FTM_UINT32		ulFieldFlags
);

FTM_RET	FTM_CATCHB_getCCTVCount
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_getCCTVIDList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_ID_PTR		pIDList,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_getCCTVList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulMaxCount,
	FTM_CCTV_PTR	pCCTV,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_setCCTVHash
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
);

FTM_RET	FTM_CATCHB_setCCTVStat
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulTime
);

FTM_RET	FTM_CATCHB_registerCCTV
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
);

FTM_RET	FTM_CATCHB_addSwitch
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_SWITCH_PTR	_PTR_ ppSwitch
);

FTM_RET	FTM_CATCHB_delSwitch
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_CATCHB_getSwitch
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_PTR _PTR_ pSwitch
);

FTM_RET	FTM_CATCHB_getSwitchProperties
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CATCHB_setSwitchProperties
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
);

FTM_RET	FTM_CATCHB_getSwitchCount
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_getSwitchIDList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_getSwitchList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_SWITCH_PTR	_PTR_ ppSwitches,
	FTM_UINT32		ulMaxCount,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_addAlarm
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pEmail,
	FTM_CHAR_PTR	pMessage,
	FTM_ALARM_PTR _PTR_ ppAlarm
);

FTM_RET	FTM_CATCHB_delAlarm
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pName
);

FTM_RET	FTM_CATCHB_getAlarmCount
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_getAlarm
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm
);

FTM_RET	FTM_CATCHB_setAlarm
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm,
	FTM_UINT32		ulFieldFlags
);

FTM_RET	FTM_CATCHB_getAlarmList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_ALARM_PTR	pAlarmList,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_getAlarmNameList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_NAME_PTR  	pNameList,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CATCHB_removeExpiredLog
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulRetentionPeriod
);

FTM_RET	FTM_CATCHB_addLog
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_LOG_TYPE	xType,
	FTM_UINT32		ulTime,
	FTM_CHAR_PTR	pCCTVID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_CHAR_PTR	pHash
);

FTM_RET	FTM_CATCHB_delLog
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_CATCHB_getLogInfo
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pulCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CATCHB_getLogCount
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CATCHB_getLogList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_LOG_PTR		pLogList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CATCHB_addStatistics
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_STATISTICS_PTR	pStatisticsistics
);

FTM_RET	FTM_CATCHB_delStatistics
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount
);

FTM_RET	FTM_CATCHB_getStatisticsInfo
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pulCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CATCHB_getStatisticsCount
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CATCHB_getStatisticsList
(
	FTM_CATCHB_PTR	pCatchB,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_STATISTICS_PTR		pStatisticsList,
	FTM_UINT32_PTR	pulCount
);
#endif
