#ifndef	_FTM_CGI_H_
#define	_FTM_CGI_H_

#include <syslog.h>
#include "ftm_types.h"
#include "ftm_client.h"
#include "qdecoder.h"
#include "mxml.h"
#include "cJSON.h"

typedef	struct FTM_CGI_COMMAND_STRUCT
{
	FTM_CHAR_PTR	pName;
	FTM_RET			(*fService)(FTM_CLIENT_PTR pClient, qentry_t *req);
} FTM_CGI_COMMAND, _PTR_ FTM_CGI_COMMAND_PTR;

FTM_RET	FTM_CGI_finish
(
	qentry_t _PTR_ pReq,
	cJSON _PTR_ pRoot, 
	FTM_RET xRet
);

FTM_RET	FTM_CGI_cctv
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTM_CGI_switch
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTM_CGI_log
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTM_CGI_alarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

const 
char *FTM_CGI_whitespaceCB
(
	mxml_node_t *node,
	int			where
);

FTM_RET	FTM_CGI_getUINT16
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_UINT16_PTR	pusValue,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getUINT32
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_UINT32_PTR	pulValue,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getSTRING
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_BOOL	bAllowEmpty
);


FTM_RET	FTM_CGI_getID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pID,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getUserID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pUserID,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getPasswd
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pPasswd,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getSwitchID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pSwitchID,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getEnable
(
	qentry_t *pReq, 
	FTM_BOOL_PTR	pEnable	,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getComment
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pComment,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTM_CGI_getIndex
(
	qentry_t *pReq, 
	FTM_UINT32_PTR pulIndex,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTM_CGI_getCount
(
	qentry_t *pReq, 
	FTM_UINT32_PTR pulCount,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getIPString
(
	qentry_t *pReq,
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_BOOL bAllowEmpty
);

FTM_RET	FTM_CGI_getPort
(
	qentry_t *pReq, 
	FTM_UINT16_PTR	pusPort,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTM_CGI_getSwitchModel
(
	qentry_t *pReq, 
	FTM_SWITCH_MODEL_PTR pModel,
	FTM_BOOL	bAllowEmpty
);

#endif