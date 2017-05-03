#ifndef	FTM_SWITCH_H_
#define	FTM_SWITCH_H_

#include "ftm_types.h"
#include "ftm_lock.h"
#include "ftm_list.h"
#include "cjson/cJSON.h"
#include "ftm_trace.h"

typedef	enum	FTM_SWITCH_MODEL_ENUM
{
	FTM_SWITCH_MODEL_UNKNOWN = 0,
	FTM_SWITCH_MODEL_NST = 1,
	FTM_SWITCH_MODEL_DASAN,
	FTM_SWITCH_MODEL_JUNIPER
}	FTM_SWITCH_MODEL, _PTR_ FTM_SWITCH_MODEL_PTR;

#define	FTM_SWITCH_FIELD_ID			(1 << 0)
#define	FTM_SWITCH_FIELD_MODEL		(1 << 1)
#define	FTM_SWITCH_FIELD_IP			(1 << 2)
#define	FTM_SWITCH_FIELD_USER_ID	(1 << 3)
#define	FTM_SWITCH_FIELD_PASSWD		(1 << 4)
#define	FTM_SWITCH_FIELD_SECURE		(1 << 5)
#define	FTM_SWITCH_FIELD_COMMENT	(1 << 6)
#define	FTM_SWITCH_FIELD_ALL		(x0FF)

typedef	struct	FTM_SWITCH_CONFIG_STRUCT
{
	FTM_CHAR			pID[FTM_ID_LEN+1];
	FTM_SWITCH_MODEL	xModel;
	FTM_CHAR			pIP[FTM_IP_LEN+1];
	FTM_BOOL			bSecure;
	FTM_CHAR			pUserID[FTM_USER_LEN+1];
	FTM_CHAR			pPasswd[FTM_PASSWD_LEN+1];
	FTM_CHAR			pComment[FTM_COMMENT_LEN+1];
}	FTM_SWITCH_CONFIG, _PTR_ FTM_SWITCH_CONFIG_PTR;

typedef	enum	FTM_SWITCH_AC_POLICY_ENUM
{
	FTM_SWITCH_AC_POLICY_DENY = 0,
	FTM_SWITCH_AC_POLICY_ALLOW= 1
}	FTM_SWITCH_AC_POLICY, _PTR_ FTM_SWITCH_AC_POLICY_PTR;

typedef	struct	FTM_SWITCH_AC_STRUCT
{
	FTM_UINT32	nIndex;
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_SWITCH_AC_POLICY	xPolicy;
}	FTM_SWITCH_AC, _PTR_ FTM_SWITCH_AC_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pPrompt;
	FTM_CHAR			pInput[256];
	FTM_UINT32			ulSubIndex;
	FTM_UINT32			pNext[8];
	FTM_UINT32			ulNextCount;
}	FTM_SWS_CMD, _PTR_ FTM_SWS_CMD_PTR;

typedef	struct
{
	FTM_SWS_CMD	pCommands[32];
}	FTM_SWITCH_SCRIPT, _PTR_ FTM_SWITCH_SCRIPT_PTR;

FTM_RET	FTM_SWITCH_CONFIG_create
(
	FTM_SWITCH_CONFIG_PTR _PTR_ ppConfig
);

FTM_RET	FTM_SWITCH_CONFIG_destroy
(
	FTM_SWITCH_CONFIG_PTR _PTR_ ppConfig
);

FTM_RET	FTM_SWITCH_CONFIG_load
(
	FTM_SWITCH_CONFIG_PTR	pConfig,
	cJSON _PTR_		pRoot
);

FTM_RET	FTM_SWITCH_CONFIG_loadList
(	
	FTM_LIST_PTR	pList,
	cJSON _PTR_		pRoot
);

FTM_RET	FTM_SWITCH_CONFIG_showList
(
	FTM_LIST_PTR	pList,
	FTM_TRACE_LEVEL	xLevel
);


typedef	struct	FTM_SWITCH_STRUCT
{
	FTM_SWITCH_CONFIG	xConfig;

	FTM_LIST_PTR		pACList;
	FTM_LOCK			xLock;
}	FTM_SWITCH, _PTR_ FTM_SWITCH_PTR;

FTM_RET	FTM_SWITCH_create
(
	FTM_SWITCH_CONFIG_PTR	pConfig,
	FTM_SWITCH_AC_PTR		pACs,
	FTM_UINT32				ulACCount,
	FTM_SWITCH_PTR	_PTR_	ppSwitch
);

FTM_RET	FTM_SWITCH_destroy
(
	FTM_SWITCH_PTR	_PTR_	ppSwitch
);

FTM_RET	FTM_SWITCH_addAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pIP,
	FTM_SWITCH_AC_POLICY	xPolicy,
	FTM_SWITCH_AC_PTR _PTR_ ppAC
);

FTM_RET	FTM_SWITCH_deleteAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pIP
);

FTM_RET	FTM_SWITCH_getAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pIP,
	FTM_SWITCH_AC_PTR _PTR_ pAC
);

FTM_SWITCH_MODEL	FTM_getSwitchModelID
(	
	FTM_CHAR_PTR	pModel
);

FTM_CHAR_PTR	FTM_getSwitchModelName
(
	FTM_SWITCH_MODEL	xModel
);

typedef	FTM_RET	(*FTM_SWITCH_SET_AC)
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pIP,
	FTM_SWITCH_AC_POLICY	xPolicy
);

typedef	struct	FTM_SWITCH_INFO_STRUCT
{
	FTM_SWITCH_MODEL	xModel;
	FTM_CHAR			pName[FTM_NAME_LEN+1];
	FTM_SWITCH_SET_AC	fSetAC;
}	FTM_SWITCH_INFO, _PTR_ FTM_SWITCH_INFO_PTR;

FTM_RET	FTM_SWITCH_getInfo
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_SWITCH_INFO_PTR _PTR_ ppInfo
);

FTM_RET	FTM_SWITCH_TELNET_setAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWITCH_SCRIPT_PTR	pScript 
);

FTM_RET	FTM_SWITCH_SSH_setAC
(
	FTM_SWITCH_PTR	pSwitch,
	FTM_CHAR_PTR	pTargetIP,
	FTM_SWITCH_SCRIPT_PTR	pScript 
);

#include "ftm_switch_nst.h"
#include "ftm_switch_dasan.h"
#include "ftm_switch_juniper.h"
#endif
