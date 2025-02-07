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
	FTM_CHAR		pTmpPath[FTM_PATH_LEN + 1];
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

FTM_RET	FTM_CLIENT_CONFIG_save
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	cJSON _PTR_ pRoot
);

FTM_RET	FTM_CLIENT_CONFIG_loadFromFile
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	FTM_CHAR_PTR			pFileName
);

FTM_RET	FTM_CLIENT_CONFIG_show
(
	FTM_CLIENT_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
);

typedef struct FTM_CLIENT_STRUCT
{
	FTM_CLIENT_CONFIG	xConfig;

	FTM_INT		hSock;
	FTM_INT		nTimeout;
	FTM_LOCK	xLock;
}	FTM_CLIENT, _PTR_ FTM_CLIENT_PTR;



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

FTM_RET FTM_CLIENT_CCTV_add
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CHAR_PTR	pSwitchID,
	FTM_CHAR_PTR	pComment
);

FTM_RET	FTM_CLIENT_CCTV_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_CLIENT_CCTV_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CLIENT_CCTV_setProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFields
);

FTM_RET	FTM_CLIENT_CCTV_getProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_CCTV_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CLIENT_CCTV_getIDList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_CCTV_getList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_CCTV_CONFIG_PTR		pCCTVList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_CCTV_setPolicy
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_AC_POLICY	xPolicy
);

FTM_RET	FTM_CLIENT_CCTV_reset
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pHash
);

FTM_RET FTM_CLIENT_SWITCH_add
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_SWITCH_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CLIENT_SWITCH_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID
);

FTM_RET	FTM_CLIENT_SWITCH_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_SWITCH_setProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_UINT32		ulFields
);

FTM_RET	FTM_CLIENT_SWITCH_getProperties
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pID,
	FTM_SWITCH_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CLIENT_SWITCH_getIDList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_ID_PTR		pIDList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_SWITCH_getList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_SWITCH_CONFIG_PTR	pSwitchList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_SWITCH_getModelList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulMaxCount,
	FTM_SWITCH_MODEL_INFO_PTR	pModelList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_LOG_getInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pulCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CLIENT_LOG_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_LOG_getCount2
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_LOG_TYPE	xType,
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pIP,
	FTM_CCTV_STAT	xStat,
	FTM_UINT32		ulBeginTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_LOG_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CLIENT_LOG_getList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
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

FTM_RET	FTM_CLIENT_ALARM_add
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pEmail,
	FTM_CHAR_PTR	pMessage
);

FTM_RET	FTM_CLIENT_ALARM_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pName
);

FTM_RET	FTM_CLIENT_ALARM_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CLIENT_ALARM_get
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm
);

FTM_RET	FTM_CLIENT_ALARM_set
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_CHAR_PTR	pName,
	FTM_ALARM_PTR	pAlarm,
	FTM_UINT32		ulFieldFlags
);


FTM_RET	FTM_CLIENT_ALARM_getNameList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulMaxCount,
	FTM_NAME_PTR	pNameList,
	FTM_UINT32_PTR	pCount
);

FTM_RET	FTM_CLIENT_SMTP_get
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig
);

FTM_RET	FTM_CLIENT_SMTP_set
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pConfig,
	FTM_NOTIFIER_SMTP_CONFIG_PTR	pResultConfig
);

FTM_RET	FTM_CLIENT_STAT_setInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_SYSTEM_INFO_PTR	pRequestInfo,
	FTM_SYSTEM_INFO_PTR	pResultInfo
);

FTM_RET	FTM_CLIENT_STAT_getInfo
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_SYSTEM_INFO_PTR	pInfo
);

FTM_RET	FTM_CLIENT_STAT_getCount
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_STAT_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CLIENT_STAT_del2
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulBeginTime,
	FTM_UINT32		ulEndTime,
	FTM_UINT32_PTR	pulRemainCount,
	FTM_UINT32_PTR	pulFirstTime,
	FTM_UINT32_PTR	pulLastTime
);

FTM_RET	FTM_CLIENT_STAT_getList
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulIndex,
	FTM_UINT32		ulCount,
	FTM_STATISTICS_PTR		pStatList,
	FTM_UINT32_PTR	pulCount
);

FTM_RET	FTM_CLIENT_SSID_create
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pID,
	FTM_UINT32		ulIDLen,
	FTM_CHAR_PTR	pPasswd,
	FTM_UINT32		ulPasswdLen,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulSSIDLen
);

FTM_RET	FTM_CLIENT_SSID_del
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulSSIDLen
);

FTM_RET	FTM_CLIENT_SSID_verify
(
	FTM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pSSID,
	FTM_UINT32		ulSSIDLen
);

#endif
