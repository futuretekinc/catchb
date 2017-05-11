#ifndef	FTM_PARAMS_H_
#define	FTM_PARAMS_H_

#include "ftm_types.h"
#include "ftm_cctv.h"
#include "ftm_system.h"
#include "ftm_switch.h"
#include "ftm_alarm.h"
#include "ftm_utils.h"
#include "ftm_log.h"

//#define	FTM_PARAM_MAX_LEN	1500
#define	FTM_PARAM_MAX_LEN	4096

typedef	enum
{
	FTM_CMD_UNKNOWN,

	FTM_CMD_ADD_CCTV,
	FTM_CMD_DEL_CCTV,
	FTM_CMD_SET_CCTV_PROPERTIES,
	FTM_CMD_GET_CCTV_PROPERTIES,
	FTM_CMD_GET_CCTV_COUNT,
	FTM_CMD_GET_CCTV_ID_LIST,
	FTM_CMD_GET_CCTV_LIST,
	FTM_CMD_SET_CCTV_POLICY,
	FTM_CMD_RESET_CCTV,

	FTM_CMD_ADD_SWITCH,
	FTM_CMD_DEL_SWITCH,
	FTM_CMD_SET_SWITCH_PROPERTIES,
	FTM_CMD_GET_SWITCH_PROPERTIES,
	FTM_CMD_GET_SWITCH_COUNT,
	FTM_CMD_GET_SWITCH_ID_LIST,
	FTM_CMD_GET_SWITCH_LIST,

	FTM_CMD_GET_LOG_INFO,
	FTM_CMD_GET_LOG_COUNT,
	FTM_CMD_GET_LOG_LIST,
	FTM_CMD_DEL_LOG,

	FTM_CMD_ADD_ALARM,
	FTM_CMD_DEL_ALARM,
	FTM_CMD_GET_ALARM_COUNT,
	FTM_CMD_GET_ALARM,
	FTM_CMD_SET_ALARM,
	FTM_CMD_GET_ALARM_NAME_LIST,

	FTM_CMD_GET_STAT_INFO,
	FTM_CMD_SET_STAT_INFO,
	FTM_CMD_GET_STAT_COUNT,
	FTM_CMD_GET_STAT_LIST,
	FTM_CMD_DEL_STAT,
	FTM_CMD_DEL_STAT2,

}	FTM_CMD, _PTR_ FTM_CMD_PTR;

typedef	struct
{
	FTM_CMD		xCmd;
	FTM_UINT32	ulLen;
}	FTM_REQ_PARAMS, _PTR_ FTM_REQ_PARAMS_PTR;

typedef	struct
{
	FTM_CMD		xCmd;
	FTM_UINT32	ulLen;
	FTM_RET		xRet;
}	FTM_RESP_PARAMS, _PTR_ FTM_RESP_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_CHAR		pIP[FTM_IP_LEN+1];
	FTM_CHAR		pSwitchID[FTM_ID_LEN+1];
	FTM_CHAR		pComment[FTM_COMMENT_LEN+1];
}	FTM_REQ_ADD_CCTV_PARAMS, _PTR_ FTM_REQ_ADD_CCTV_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_ADD_CCTV_PARAMS, _PTR_ FTM_RESP_ADD_CCTV_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
}	FTM_REQ_DEL_CCTV_PARAMS, _PTR_ FTM_REQ_DEL_CCTV_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_DEL_CCTV_PARAMS, _PTR_ FTM_RESP_DEL_CCTV_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
}	FTM_REQ_GET_CCTV_COUNT_PARAMS, _PTR_ FTM_REQ_GET_CCTV_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
}	FTM_RESP_GET_CCTV_COUNT_PARAMS, _PTR_ FTM_RESP_GET_CCTV_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_UINT32		ulFieldFlags;
	FTM_CCTV_CONFIG	xConfig;
}	FTM_REQ_SET_CCTV_PROPERTIES_PARAMS, _PTR_ FTM_REQ_SET_CCTV_PROPERTIES_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_SET_CCTV_PROPERTIES_PARAMS, _PTR_ FTM_RESP_SET_CCTV_PROPERTIES_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
}	FTM_REQ_GET_CCTV_PROPERTIES_PARAMS, _PTR_ FTM_REQ_GET_CCTV_PROPERTIES_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_CCTV_CONFIG	xConfig;
}	FTM_RESP_GET_CCTV_PROPERTIES_PARAMS, _PTR_ FTM_RESP_GET_CCTV_PROPERTIES_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_GET_CCTV_ID_LIST_PARAMS, _PTR_ FTM_REQ_GET_CCTV_ID_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_ID			pIDList[];
}	FTM_RESP_GET_CCTV_ID_LIST_PARAMS, _PTR_ FTM_RESP_GET_CCTV_ID_LIST_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_GET_CCTV_LIST_PARAMS, _PTR_ FTM_REQ_GET_CCTV_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_CCTV_CONFIG	pCCTVList[];
}	FTM_RESP_GET_CCTV_LIST_PARAMS, _PTR_ FTM_RESP_GET_CCTV_LIST_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_SWITCH_AC_POLICY	xPolicy;
}	FTM_REQ_SET_CCTV_POLICY_PARAMS, _PTR_ FTM_REQ_SET_CCTV_POLICY_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_SET_CCTV_POLICY_PARAMS, _PTR_ FTM_RESP_SET_CCTV_POLICY_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_CHAR		pHash[FTM_HASH_LEN+1];
}	FTM_REQ_RESET_CCTV_PARAMS, _PTR_ FTM_REQ_RESET_CCTV_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_RESET_CCTV_PARAMS, _PTR_ FTM_RESP_RESET_CCTV_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_SWITCH_CONFIG	xConfig;
}	FTM_REQ_ADD_SWITCH_PARAMS, _PTR_ FTM_REQ_ADD_SWITCH_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_ADD_SWITCH_PARAMS, _PTR_ FTM_RESP_ADD_SWITCH_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
}	FTM_REQ_DEL_SWITCH_PARAMS, _PTR_ FTM_REQ_DEL_SWITCH_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_DEL_SWITCH_PARAMS, _PTR_ FTM_RESP_DEL_SWITCH_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_UINT32		ulFieldFlags;
	FTM_SWITCH_CONFIG	xConfig;
}	FTM_REQ_SET_SWITCH_PROPERTIES_PARAMS, _PTR_ FTM_REQ_SET_SWITCH_PROPERTIES_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_SET_SWITCH_PROPERTIES_PARAMS, _PTR_ FTM_RESP_SET_SWITCH_PROPERTIES_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pID[FTM_ID_LEN+1];
}	FTM_REQ_GET_SWITCH_PROPERTIES_PARAMS, _PTR_ FTM_REQ_GET_SWITCH_PROPERTIES_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_SWITCH_CONFIG	xConfig;
}	FTM_RESP_GET_SWITCH_PROPERTIES_PARAMS, _PTR_ FTM_RESP_GET_SWITCH_PROPERTIES_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
}	FTM_REQ_GET_SWITCH_COUNT_PARAMS, _PTR_ FTM_REQ_GET_SWITCH_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
}	FTM_RESP_GET_SWITCH_COUNT_PARAMS, _PTR_ FTM_RESP_GET_SWITCH_COUNT_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_GET_SWITCH_ID_LIST_PARAMS, _PTR_ FTM_REQ_GET_SWITCH_ID_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_ID			pIDList[];
}	FTM_RESP_GET_SWITCH_ID_LIST_PARAMS, _PTR_ FTM_RESP_GET_SWITCH_ID_LIST_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_GET_SWITCH_LIST_PARAMS, _PTR_ FTM_REQ_GET_SWITCH_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_SWITCH_CONFIG	pSwitchList[];
}	FTM_RESP_GET_SWITCH_LIST_PARAMS, _PTR_ FTM_RESP_GET_SWITCH_LIST_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
}	FTM_REQ_GET_LOG_INFO_PARAMS, _PTR_ FTM_REQ_GET_LOG_INFO_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_UINT32		ulFirstTime;
	FTM_UINT32		ulLastTime;
}	FTM_RESP_GET_LOG_INFO_PARAMS, _PTR_ FTM_RESP_GET_LOG_INFO_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
}	FTM_REQ_GET_LOG_COUNT_PARAMS, _PTR_ FTM_REQ_GET_LOG_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
}	FTM_RESP_GET_LOG_COUNT_PARAMS, _PTR_ FTM_RESP_GET_LOG_COUNT_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_LOG_TYPE	xType;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_CHAR		pIP[FTM_IP_LEN+1];
	FTM_CCTV_STAT	xStat;
	FTM_UINT32		ulBeginTime;
	FTM_UINT32		ulEndTime;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_GET_LOG_LIST_PARAMS, _PTR_ FTM_REQ_GET_LOG_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
	FTM_LOG			pLogList[];
}	FTM_RESP_GET_LOG_LIST_PARAMS, _PTR_ FTM_RESP_GET_LOG_LIST_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_DEL_LOG_PARAMS, _PTR_ FTM_REQ_DEL_LOG_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_UINT32		ulFirstTime;
	FTM_UINT32		ulLastTime;
}	FTM_RESP_DEL_LOG_PARAMS, _PTR_ FTM_RESP_DEL_LOG_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_CHAR		pEmail[FTM_EMAIL_LEN+1];
	FTM_CHAR		pMessage[FTM_ALARM_MESSAGE_LEN+1];
}	FTM_REQ_ADD_ALARM_PARAMS, _PTR_ FTM_REQ_ADD_ALARM_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_ADD_ALARM_PARAMS, _PTR_ FTM_RESP_ADD_ALARM_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pName[FTM_NAME_LEN+1];
}	FTM_REQ_DEL_ALARM_PARAMS, _PTR_ FTM_REQ_DEL_ALARM_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
}	FTM_RESP_DEL_ALARM_PARAMS, _PTR_ FTM_RESP_DEL_ALARM_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
}	FTM_REQ_GET_ALARM_COUNT_PARAMS, _PTR_ FTM_REQ_GET_ALARM_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
}	FTM_RESP_GET_ALARM_COUNT_PARAMS, _PTR_ FTM_RESP_GET_ALARM_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pName[FTM_NAME_LEN+1];
}	FTM_REQ_GET_ALARM_PARAMS, _PTR_ FTM_REQ_GET_ALARM_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_ALARM		xAlarm;
}	FTM_RESP_GET_ALARM_PARAMS, _PTR_ FTM_RESP_GET_ALARM_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_ALARM		xAlarm;
	FTM_UINT32		ulFieldFlags;
}	FTM_REQ_SET_ALARM_PARAMS, _PTR_ FTM_REQ_SET_ALARM_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_ALARM		xAlarm;
}	FTM_RESP_SET_ALARM_PARAMS, _PTR_ FTM_RESP_SET_ALARM_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_GET_ALARM_NAME_LIST_PARAMS, _PTR_ FTM_REQ_GET_ALARM_NAME_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_NAME		pNameList[];
}	FTM_RESP_GET_ALARM_NAME_LIST_PARAMS, _PTR_ FTM_RESP_GET_ALARM_NAME_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_SYSTEM_INFO	xInfo;
}	FTM_REQ_SET_STAT_INFO_PARAMS, _PTR_ FTM_REQ_SET_STAT_INFO_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_SYSTEM_INFO	xInfo;
}	FTM_RESP_SET_STAT_INFO_PARAMS, _PTR_ FTM_RESP_SET_STAT_INFO_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
}	FTM_REQ_GET_STAT_INFO_PARAMS, _PTR_ FTM_REQ_GET_STAT_INFO_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_SYSTEM_INFO	xInfo;
}	FTM_RESP_GET_STAT_INFO_PARAMS, _PTR_ FTM_RESP_GET_STAT_INFO_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
}	FTM_REQ_GET_STAT_COUNT_PARAMS, _PTR_ FTM_REQ_GET_STAT_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
}	FTM_RESP_GET_STAT_COUNT_PARAMS, _PTR_ FTM_RESP_GET_STAT_COUNT_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_GET_STAT_LIST_PARAMS, _PTR_ FTM_REQ_GET_STAT_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
	FTM_STATISTICS	pStatList[];
}	FTM_RESP_GET_STAT_LIST_PARAMS, _PTR_ FTM_RESP_GET_STAT_LIST_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulIndex;
	FTM_UINT32		ulCount;
}	FTM_REQ_DEL_STAT_PARAMS, _PTR_ FTM_REQ_DEL_STAT_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_UINT32		ulFirstTime;
	FTM_UINT32		ulLastTime;
}	FTM_RESP_DEL_STAT_PARAMS, _PTR_ FTM_RESP_DEL_STAT_PARAMS_PTR; 

typedef	struct
{
	FTM_REQ_PARAMS	xCommon;
	FTM_UINT32		ulBeginTime;
	FTM_UINT32		ulEndTime;
}	FTM_REQ_DEL_STAT2_PARAMS, _PTR_ FTM_REQ_DEL_STAT2_PARAMS_PTR;

typedef	struct
{
	FTM_RESP_PARAMS	xCommon;
	FTM_UINT32		ulCount;
	FTM_UINT32		ulFirstTime;
	FTM_UINT32		ulLastTime;
}	FTM_RESP_DEL_STAT2_PARAMS, _PTR_ FTM_RESP_DEL_STAT2_PARAMS_PTR; 

#endif
