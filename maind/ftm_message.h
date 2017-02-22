#ifndef	__FTM_MSG_H__
#define	__FTM_MSG_H__

#include "ftm_types.h"
#include "ftm_cctv.h"
#include "ftm_switch.h"

typedef	FTM_UINT32	FTM_MSG_TYPE, _PTR_ FTM_MSG_TYPE_PTR;

#define	FTM_MSG_TYPE_SEND_ALARM		1
#define	FTM_MSG_TYPE_IP_DETECTED	2
#define	FTM_MSG_TYPE_ADD_CCTV		3
#define	FTM_MSG_TYPE_DELETE_CCTV	4
#define	FTM_MSG_TYPE_CHECK_NEW_CCTV	5
#define	FTM_MSG_TYPE_FOUND_NEW_CCTV_IN_DB	6
#define	FTM_MSG_TYPE_REMOVED_CCTV_IN_DB		7
#define	FTM_MSG_TYPE_CCTV_HASH_UPDATED		8
#define	FTM_MSG_TYPE_INITIALIZED			9
#define	FTM_MSG_TYPE_CCTV_SET_STAT			10
#define	FTM_MSG_TYPE_CCTV_REGISTER			11
#define	FTM_MSG_TYPE_SWITCH_CONTROL			12
#define	FTM_MSG_TYPE_CHECK_NEW_SWITCH		13
#define	FTM_MSG_TYPE_FOUND_NEW_SWITCH_IN_DB	14
#define	FTM_MSG_TYPE_REMOVED_SWITCH_IN_DB	15

#define	FTM_MSG_MEMBER	\
			FTM_MSG_TYPE	xType;\
			FTM_UINT32		ulLen;\
			FTM_UINT32		ulSenderID;

typedef	struct	FTM_MSG_STRUCT
{
	FTM_MSG_TYPE	xType;
	FTM_UINT32		ulLen;
	FTM_UINT32		ulSenderID;
}	FTM_MSG, _PTR_ FTM_MSG_PTR;

typedef	struct	FTM_MSG_SEND_ALARM_STRUCT
{
	FTM_MSG		xHead;
	FTM_CHAR	pID[FTM_ID_LEN+1];
} FTM_MSG_SEND_ALARM, _PTR_ FTM_MSG_SEND_ALARM_PTR;

typedef	struct	FTM_MSG_IP_DETECTED_STRUCT
{
	FTM_MSG		xHead;
	FTM_CHAR	pIP[FTM_IP_LEN+1];
	FTM_BOOL	bAbnormal;
}	FTM_MSG_IP_DETECTED, _PTR_ FTM_MSG_IP_DETECTED_PTR;

typedef	struct	FTM_MSG_CHECK_CCTV_LIST_STRUCT
{
	FTM_MSG		xHead;
}	FTM_MSG_CHECK_NEW_CCTV, _PTR_ FTM_MSG_CHECK_NEW_CCTV_PTR;

typedef	struct	FTM_MSG_ADD_CCTV_STRUCT
{
	FTM_MSG		xHead;
	FTM_CHAR	pID[FTM_ID_LEN+1];
}	FTM_MSG_ADD_CCTV, _PTR_ FTM_MSG_ADD_CCTV_PTR;

typedef	struct	FTM_MSG_DELETE_CCTV_STRUCT
{
	FTM_MSG		xHead;
	FTM_CHAR	pID[FTM_ID_LEN+1];
}	FTM_MSG_DELETE_CCTV, _PTR_ FTM_MSG_DELETE_CCTV_PTR;

typedef	struct	FTM_MSG_FOUND_NEW_CCTV_IN_DB_STRUCT
{
	FTM_MSG		xHead;
	FTM_CCTV_CONFIG	xConfig;
}	FTM_MSG_FOUND_NEW_CCTV_IN_DB, _PTR_ FTM_MSG_FOUND_NEW_CCTV_IN_DB_PTR;
	
typedef	struct	FTM_MSG_REMOVED_CCTV_IN_DB_STRUCT
{
	FTM_MSG		xHead;
	FTM_CHAR	pID[FTM_ID_LEN+1];
}	FTM_MSG_REMOVED_CCTV_IN_DB, _PTR_ FTM_MSG_REMOVED_CCTV_IN_DB_PTR;

typedef	struct	FTM_MSG_CCTV_HASH_UPDATED_STRUCT
{
	FTM_MSG		xHead;
	FTM_CHAR	pID[FTM_ID_LEN+1];
	FTM_CHAR	pHash[FTM_HASH_LEN+1];
}	FTM_MSG_CCTV_HASH_UPDATED, _PTR_ FTM_MSG_CCTV_HASH_UPDATED_PTR;

typedef	struct	FTM_MSG_CCTV_SET_STAT_STRUCT
{
	FTM_MSG			xHead;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_CCTV_STAT	xStat;
}	FTM_MSG_CCTV_SET_STAT, _PTR_ FTM_MSG_CCTV_SET_STAT_PTR;

typedef	struct	FTM_MSG_CCTV_SET_REGISTER_STRUCT
{
	FTM_MSG			xHead;
	FTM_CHAR		pID[FTM_ID_LEN+1];
	FTM_CHAR		pHash[FTM_HASH_LEN+1];
}	FTM_MSG_CCTV_REGISTER, _PTR_ FTM_MSG_CCTV_REGISTER_PTR;

typedef	struct	FTM_MSG_CHECK_SWITCH_LIST_STRUCT
{
	FTM_MSG		xHead;
}	FTM_MSG_CHECK_NEW_SWITCH, _PTR_ FTM_MSG_CHECK_NEW_SWITCH_PTR;

typedef	struct	FTM_MSG_SWITCH_CONTROL_STRUCT
{
	FTM_MSG			xHead;
	FTM_CHAR		pSwitchID[FTM_ID_LEN+1];
	FTM_CHAR		pTargetIP[FTM_IP_LEN+1];
	FTM_SWITCH_AC_POLICY	xPolicy;
}	FTM_MSG_SWITCH_CONTROL, _PTR_ FTM_MSG_SWITCH_CONTROL_PTR;

typedef	struct	FTM_MSG_FOUND_NEW_SWITCH_IN_DB_STRUCT
{
	FTM_MSG		xHead;
	FTM_SWITCH_CONFIG	xConfig;
}	FTM_MSG_FOUND_NEW_SWITCH_IN_DB, _PTR_ FTM_MSG_FOUND_NEW_SWITCH_IN_DB_PTR;
	
typedef	struct	FTM_MSG_REMOVED_SWITCH_IN_DB_STRUCT
{
	FTM_MSG		xHead;
	FTM_CHAR	pID[FTM_ID_LEN+1];
}	FTM_MSG_REMOVED_SWITCH_IN_DB, _PTR_ FTM_MSG_REMOVED_SWITCH_IN_DB_PTR;

#endif
