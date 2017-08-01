#ifndef	JSON_UTILS_H_
#define	JSON_UTILS_H_
#include "ftm_types.h"
#include "cjson/cJSON.h"
#include "ftm_client.h"

FTM_RET	FTM_JSON_getBool
(
	cJSON _PTR_ 	pRoot,
	FTM_CHAR_PTR	pTitle,
	FTM_BOOL		bEmptyAllow,
	FTM_BOOL_PTR	pValue
);

FTM_RET	FTM_JSON_getUINT16
(
	cJSON _PTR_ 	pRoot,
	FTM_CHAR_PTR	pTitle,
	FTM_BOOL		bEmptyAllow,
	FTM_UINT16_PTR	pValue
);

FTM_RET	FTM_JSON_getUINT32
(
	cJSON _PTR_ 	pRoot,
	FTM_CHAR_PTR	pTitle,
	FTM_BOOL		bEmptyAllow,
	FTM_UINT32_PTR	pValue
);

FTM_RET	FTM_JSON_getString
(
	cJSON _PTR_ 	pRoot,
	FTM_CHAR_PTR	pTitle,
	FTM_UINT32		ulMaxLen,
	FTM_BOOL		bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getSSID
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pSSID
);

FTM_RET	FTM_JSON_getName
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pName
);

FTM_RET	FTM_JSON_getModel
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getID
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getHash
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getUserID
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getPasswd
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getNewPasswd
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getIP
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getComment
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getCount
(
	cJSON _PTR_ 	pRoot,
	FTM_BOOL		bEmptyAllow,
	FTM_UINT32_PTR	pValue
);

FTM_RET	FTM_JSON_getIndex
(
	cJSON _PTR_ 	pRoot,
	FTM_BOOL		bEmptyAllow,
	FTM_UINT32_PTR	pValue
);

FTM_RET	FTM_JSON_getSecure
(
	cJSON _PTR_ 	pRoot,
	FTM_BOOL		bEmptyAllow,
	FTM_BOOL_PTR	pValue
);

FTM_RET	FTM_JSON_getSwitchID
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_CHAR_PTR	pValue
);

FTM_RET	FTM_JSON_getPolicy
(
	cJSON _PTR_ pRoot,
	FTM_BOOL	bEmptyAllow,
	FTM_SWITCH_AC_POLICY_PTR	pPolicy
);

FTM_RET	FTM_JSON_getLogType
(
	cJSON _PTR_ pRoot, 
	FTM_BOOL	bAllowEmpty,
	FTM_LOG_TYPE_PTR	pType
);

FTM_RET	FTM_JSON_getBeginTime
(
	cJSON _PTR_ pRoot, 
	FTM_BOOL	bAllowEmpty,
	FTM_UINT32_PTR	pValue
);

FTM_RET	FTM_JSON_getEndTime
(
	cJSON _PTR_ pRoot, 
	FTM_BOOL	bAllowEmpty,
	FTM_UINT32_PTR	pValue
);

FTM_RET	FTM_JSON_getInterval
(
	cJSON _PTR_ 	pRoot,
	FTM_BOOL		bEmptyAllow,
	FTM_UINT32_PTR	pValue
);

FTM_RET	FTM_JSON_getDate
(
	cJSON _PTR_ 	pRoot,
	FTM_BOOL		bEmptyAllow,
	FTM_UINT32_PTR	pValue
);

#endif
