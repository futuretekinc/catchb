#ifndef	_FTM_CGI_CMD_CCTV_H_
#define	_FTM_CGI_CMD_CCTV_H_

#include "ftm_types.h"
#include "ftm_client.h"
#include "qdecoder.h"

FTM_RET	FTM_CGI_CCTV_add
(
	FTM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_CCTV_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_CCTV_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_CCTV_set
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_CCTV_getIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_CCTV_setIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_CCTV_setPolicy
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_CCTV_reset
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SWITCH_add
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SWITCH_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SWITCH_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SWITCH_set
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SWITCH_getIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);


FTM_RET	FTM_CGI_LOG_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_LOG_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_LOG_getList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_ALARM_add
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_ALARM_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_ALARM_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_ALARM_set
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_ALARM_getList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_ALARM_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_ALARM_getConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_ALARM_setConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYS_getInfoConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYS_setInfoConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_STATUS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_STATUS_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_STATUS_getList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYS_checkPasswd
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYS_setPasswd
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYS_setTime
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYSLOG_add
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYSLOG_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYSLOG_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYSLOG_set
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SYSLOG_getIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SSID_create
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SSID_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_SSID_verify
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);


FTM_RET	FTM_CGI_GET_CCTV_add
(
	FTM_CLIENT_PTR pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_CCTV_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_CCTV_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_CCTV_set
(
	FTM_CLIENT_PTR	pClient,
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_CCTV_getIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_CCTV_setIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_CCTV_setPolicy
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_CCTV_reset
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SWITCH_add
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SWITCH_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SWITCH_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SWITCH_set
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SWITCH_getIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);


FTM_RET	FTM_CGI_GET_LOG_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_LOG_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_LOG_getList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_ALARM_add
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_ALARM_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_ALARM_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_ALARM_set
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_ALARM_getList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_ALARM_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_ALARM_getConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_ALARM_setConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYS_getInfoConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYS_setInfoConfig
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_STATUS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_STATUS_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_STATUS_getList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYS_checkPasswd
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYS_setPasswd
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYS_setTime
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYSLOG_add
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYSLOG_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYSLOG_get
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYSLOG_set
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SYSLOG_getIDList
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SSID_create
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SSID_del
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_GET_SSID_verify
(
	FTM_CLIENT_PTR pClient, 
	qentry_t _PTR_ pReq
);

FTM_RET	FTM_CGI_POST_CCTV_add
(
	FTM_CLIENT_PTR pClient,
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_CCTV_del
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_CCTV_get
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_CCTV_set
(
	FTM_CLIENT_PTR	pClient,
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_CCTV_getIDList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_CCTV_setIDList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_CCTV_setPolicy
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_CCTV_reset
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SWITCH_add
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SWITCH_del
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SWITCH_get
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SWITCH_set
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SWITCH_getIDList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);


FTM_RET	FTM_CGI_POST_LOG_getInfo
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_LOG_del
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_LOG_getList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_LOG_getCount2
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_ALARM_add
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_ALARM_del
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_ALARM_get
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_ALARM_set
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_ALARM_getList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_ALARM_getInfo
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_ALARM_getConfig
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_ALARM_setConfig
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_getInfoConfig
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_setInfoConfig
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_STATUS_getInfo
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_STATUS_del
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_STATUS_getList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_checkPasswd
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_setPasswd
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_setTime
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_upload
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_reboot
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYS_shutdown
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYSLOG_add
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYSLOG_del
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYSLOG_get
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYSLOG_set
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SYSLOG_getIDList
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SSID_create
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SSID_del
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_SSID_verify
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_NET_get
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);

FTM_RET	FTM_CGI_POST_NET_set
(
	FTM_CLIENT_PTR pClient, 
	cJSON _PTR_		pReqRoot,
	cJSON _PTR_		pRespRoot
);


#endif
