#ifndef	__FTM_CLIENT_H__
#define	__FTM_CLIENT_H__

#include "ftm_types.h"
#include "ftm_params.h"
#include "ftm_cctv.h"
#include "ftm_switch.h"

#define	FTM_CLIENT_FIELD_IP		(1 << 0)
#define	FTM_CLIENT_FIELD_PORT	(1 << 1)
#define	FTM_CLIENT_FIELD_ALL	(0xFF)

typedef	struct	FTM_CLIENT_CONFIG_STRUCT
{
	FTM_CHAR		pIP[16];
	FTM_UINT16 		usPort;
	FTM_BOOL		bAutoConnect;
}	FTM_CLIENT_CONFIG, _PTR_ FTM_CLIENT_CONFIG_PTR;


FTM_RET	FTM_CLIENT_CONFIG_setDefault
(
	FTM_CLIENT_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CLIENT_CONFIG_load
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
);

typedef struct FTM_CLIENT_STRUCT _PTR_ FTM_CLIENT_PTR;


FTM_RET	FTM_CLIENT_create
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	FTM_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_CLIENT_destroy
(
	FTM_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_CLIENT_setConfig
(
	FTM_CLIENT_PTR	pClient,
	FTM_CLIENT_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFieldFlags
);

FTM_RET	FTM_CLIENT_getConfig
(
	FTM_CLIENT_PTR	pClient,
	FTM_CLIENT_CONFIG_PTR	pConfig
);

FTM_RET FTM_CLIENT_connect
(
	FTM_CLIENT_PTR 	pClient
);

FTM_RET FTM_CLIENT_disconnect
(
	FTM_CLIENT_PTR	pClient
);

FTM_RET FTM_CLIENT_isConnected
(
	FTM_CLIENT_PTR	pClient,
	FTM_BOOL_PTR	pbConnected
);

FTM_RET FTM_CLIENT_addCCTV
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pComment
);

FTM_RET	FTM_CLIENT_delCCTV
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_CLIENT_getCCTVCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CLIENT_setCCTVProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFields
);

FTM_RET	FTM_CLIENT_getCCTVProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CLIENT_getCCTVIDList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_getCCTVList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulMaxCount,
	FTM_CCTV_CONFIG_PTR		pCCTVList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_setCCTVPolicy
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_AC_POLICY	xPolicy
);

FTM_RET	FTM_CLIENT_resetCCTV
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
);

FTM_RET FTM_CLIENT_addSwitch
(
	FTM_CLIENT_PTR	pClient,
	FTM_SWITCH_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CLIENT_delSwitch
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_CLIENT_getSwitchCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_setSwitchProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFields
);

FTM_RET	FTM_CLIENT_getSwitchProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CLIENT_getSwitchIDList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_getSwitchList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulMaxCount,
	FTM_SWITCH_CONFIG_PTR	pSwitchList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_getLogInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pulCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CLIENT_getLogCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_delLog
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CLIENT_getLogList
(
	FTM_CLIENT_PTR	pClient,
	FTM_LOG_TYPE	xType,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	pStat,
	FTM_UINT32		ulBeginTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_LOG_PTR		pLogList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_addAlarm
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pEmail,
	FTM_CHAR_PTR	pMessage
);

FTM_RET	FTM_CLIENT_delAlarm
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pName
);

FTM_RET	FTM_CLIENT_getAlarmCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CLIENT_getAlarm
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm
);

FTM_RET	FTM_CLIENT_setAlarm
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm,
	FTM_UINT32		ulFieldFlags
);


FTM_RET	FTM_CLIENT_getAlarmNameList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_NAME_PTR	pNameList,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CLIENT_setStatInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_SYSTEM_INFO_PTR	pRequestInfo,
	FTM_SYSTEM_INFO_PTR	pResultInfo
);

FTM_RET	FTM_CLIENT_getStatInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_SYSTEM_INFO_PTR	pInfo
);

FTM_RET	FTM_CLIENT_getStatCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_delStat
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CLIENT_delStat2
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulBeginTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CLIENT_getStatList
(
	FTM_CLIENT_PTR	pClient,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_STATISTICS_PTR		pStatList,
	FTM_UINT32_PTR	pulCount
);
#endif
