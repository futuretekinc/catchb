#ifndef	__FTM_CLIENT_H__
#define	__FTM_CLIENT_H__

#include "ftm_types.h"
#include "ftm_params.h"
#include "ftm_cctv.h"
#include "ftm_switch.h"

typedef struct FTM_CLIENT_STRUCT _PTR_ FTM_CLIENT_PTR;


FTM_RET	FTM_CLIENT_create
(
	FTM_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_CLIENT_destroy
(
	FTM_CLIENT_PTR _PTR_ ppClient
);

FTM_RET FTM_CLIENT_connect
(
	FTM_CLIENT_PTR 	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_UINT16 		usPort 
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
	FTM_UINT32		ulMaxCount,
	FTM_NAME_PTR	pNameList,
	FTM_UINT32_PTR	pCount
);

#endif
