#ifndef	_FTM_CGI_CMD_CCTV_H_
#define	_FTM_CGI_CMD_CCTV_H_

#include "ftm_types.h"
#include "ftm_client.h"
#include "qdecoder.h"

FTM_RET	FTM_CGI_addCCTV
(
	FTM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_delCCTV
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getCCTV
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_setCCTV
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getCCTVIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_setCCTVIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_addSwitch
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_delSwitch
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getSwitch
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_setSwitch
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getSwitchIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);


FTM_RET	FTM_CGI_getLogInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_delLog
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getLogList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_addAlarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_delAlarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getAlarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_setAlarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getAlarmList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getAlarmInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getSysInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getStatusInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_delStatus
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_getStatusList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

#endif
