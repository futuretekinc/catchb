#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ftm_client.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"
#include "ftm_json_utils.h"

#undef	__MODULE__
#define	__MODULE__ "cgi"

static 
FTM_RET	FTM_CGI_service
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq,
	FTM_CGI_COMMAND_PTR	pCmds
);

static 
FTM_RET	FTM_CGI_POST_service
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq,
	FTM_CGI_POST_COMMAND_PTR	pCmds
);

static 
FTM_CGI_COMMAND	pCCTVCmds[] =
{
	{	"add",		FTM_CGI_GET_CCTV_add		},
	{	"del",		FTM_CGI_GET_CCTV_del		},
	{	"get",		FTM_CGI_GET_CCTV_get		},
	{	"set",		FTM_CGI_GET_CCTV_set		},
	{	"list",		FTM_CGI_GET_CCTV_getIDList	},
	{	"policy",	FTM_CGI_GET_CCTV_setPolicy	},
	{	"reset",	FTM_CGI_GET_CCTV_reset		},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pSwitchCmds[] =
{
	{	"add",		FTM_CGI_GET_SWITCH_add		},
	{	"del",		FTM_CGI_GET_SWITCH_del		},
	{	"get",		FTM_CGI_GET_SWITCH_get		},
	{	"set",		FTM_CGI_GET_SWITCH_set		},
	{	"list",		FTM_CGI_GET_SWITCH_getIDList},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pLogCmds[] =
{
	{	"info", 	FTM_CGI_GET_LOG_getInfo			},
	{	"get_list",	FTM_CGI_GET_LOG_getList			},
	{	"del",		FTM_CGI_GET_LOG_del				},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pAlarmCmds[] =
{
	{	"add",  	FTM_CGI_GET_ALARM_add		},
	{	"del",		FTM_CGI_GET_ALARM_del		},
	{	"get",		FTM_CGI_GET_ALARM_get		},
	{	"set",		FTM_CGI_GET_ALARM_set		},
	{	"list",		FTM_CGI_GET_ALARM_getList	},
	{	"info",		FTM_CGI_GET_ALARM_getInfo	},
	{	"get_config",FTM_CGI_GET_ALARM_getConfig},
	{	"set_config",FTM_CGI_GET_ALARM_setConfig},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pSSIDCmds[] =
{
	{	"get",		FTM_CGI_GET_SSID_create		},
	{	"del",		FTM_CGI_GET_SSID_del		},
	{	"verify",	FTM_CGI_GET_SSID_verify		},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pSyslogCmds[] =
{
	{	"add",		FTM_CGI_GET_SYSLOG_add		},
	{	"del",		FTM_CGI_GET_SYSLOG_del		},
	{	"get",		FTM_CGI_GET_SYSLOG_get		},
	{	"set",		FTM_CGI_GET_SYSLOG_set		},
	{	"list",		FTM_CGI_GET_SYSLOG_getIDList},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pSysCmds[] =
{
	{	"info", 		FTM_CGI_GET_SYS_getInfo			},
	{	"get_config", 	FTM_CGI_GET_SYS_getInfoConfig	},
	{	"set_config",	FTM_CGI_GET_SYS_setInfoConfig	},
	{	"check_passwd",	FTM_CGI_GET_SYS_checkPasswd		},
	{	"passwd", 		FTM_CGI_GET_SYS_setPasswd		},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pStatusCmds[] =
{
	{	"info", FTM_CGI_GET_STATUS_getInfo		},
	{	"get",	FTM_CGI_GET_STATUS_getList		},
	{	"del",	FTM_CGI_GET_STATUS_del			},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pCCTVPostCmds[] =
{
	{	"add",		FTM_CGI_POST_CCTV_add		},
	{	"del",		FTM_CGI_POST_CCTV_del		},
	{	"get",		FTM_CGI_POST_CCTV_get		},
	{	"set",		FTM_CGI_POST_CCTV_set		},
	{	"list",		FTM_CGI_POST_CCTV_getIDList	},
	{	"policy",	FTM_CGI_POST_CCTV_setPolicy	},
	{	"reset",	FTM_CGI_POST_CCTV_reset		},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pSwitchPostCmds[] =
{
	{	"add",		FTM_CGI_POST_SWITCH_add		},
	{	"del",		FTM_CGI_POST_SWITCH_del		},
	{	"get",		FTM_CGI_POST_SWITCH_get		},
	{	"set",		FTM_CGI_POST_SWITCH_set		},
	{	"list",		FTM_CGI_POST_SWITCH_getIDList},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pLogPostCmds[] =
{
	{	"info", 	FTM_CGI_POST_LOG_getInfo			},
	{	"get",		FTM_CGI_POST_LOG_getList			},
	{	"del",		FTM_CGI_POST_LOG_del				},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pAlarmPostCmds[] =
{
	{	"add",  	FTM_CGI_POST_ALARM_add		},
	{	"del",		FTM_CGI_POST_ALARM_del		},
	{	"get",		FTM_CGI_POST_ALARM_get		},
	{	"set",		FTM_CGI_POST_ALARM_set		},
	{	"list",		FTM_CGI_POST_ALARM_getList	},
	{	"info",		FTM_CGI_POST_ALARM_getInfo	},
	{	"get_config",FTM_CGI_POST_ALARM_getConfig},
	{	"set_config",FTM_CGI_POST_ALARM_setConfig},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pSSIDPostCmds[] =
{
	{	"get",		FTM_CGI_POST_SSID_create		},
	{	"del",		FTM_CGI_POST_SSID_del		},
	{	"verify",	FTM_CGI_POST_SSID_verify		},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pSyslogPostCmds[] =
{
	{	"add",		FTM_CGI_POST_SYSLOG_add		},
	{	"del",		FTM_CGI_POST_SYSLOG_del		},
	{	"get",		FTM_CGI_POST_SYSLOG_get		},
	{	"set",		FTM_CGI_POST_SYSLOG_set		},
	{	"list",		FTM_CGI_POST_SYSLOG_getIDList},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pSysPostCmds[] =
{
	{	"info", 		FTM_CGI_POST_SYS_getInfo		},
	{	"get_config", 	FTM_CGI_POST_SYS_getInfoConfig	},
	{	"set_config",	FTM_CGI_POST_SYS_setInfoConfig	},
	{	"check_passwd",	FTM_CGI_POST_SYS_checkPasswd	},
	{	"passwd", 		FTM_CGI_POST_SYS_setPasswd		},
	{	"set_time",		FTM_CGI_POST_SYS_setTime		},
	{	"reboot", 		FTM_CGI_POST_SYS_reboot			},
	{	"shutdown",		FTM_CGI_POST_SYS_shutdown		},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pStatusPostCmds[] =
{
	{	"info", FTM_CGI_POST_STATUS_getInfo		},
	{	"get",	FTM_CGI_POST_STATUS_getList		},
	{	"del",	FTM_CGI_POST_STATUS_del			},
	{	NULL,		NULL					}
};

static 
FTM_CGI_POST_COMMAND	pNetPostCmds[] =
{
	{	"get", 		FTM_CGI_POST_NET_get},
	{	"set", 		FTM_CGI_POST_NET_set}
};

FTM_RET	FTM_CGI_finish
(
	qentry_t _PTR_ pReq,
	cJSON _PTR_ pRoot, 
	FTM_RET xRet
)
{
	switch(xRet)
	{
	case	FTM_RET_OK: 
		cJSON_AddStringToObject(pRoot, "result", "success");	
		break;

	case	FTM_RET_INVALID_ARGUMENTS: 
		cJSON_AddStringToObject(pRoot, "result", "Invalid arguments!");	
		break;

	case	FTM_RET_CLIENT_INVALID_SSID:
		cJSON_AddStringToObject(pRoot, "result", "Invalid SSID!");
		break;

	default:	
		cJSON_AddStringToObject(pRoot, "result", "failed");		
		break;
	}


	qcgires_setcontenttype(pReq, "text/json");
	printf("%s", cJSON_Print(pRoot));
	INFO("%s",  cJSON_Print(pRoot));
	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_cctv
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_service(pClient, pReq, pCCTVCmds);
}

FTM_RET	FTM_CGI_switch
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_service(pClient, pReq, pSwitchCmds);
}

FTM_RET	FTM_CGI_log
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_service(pClient, pReq, pLogCmds);
}

FTM_RET	FTM_CGI_alarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_service(pClient, pReq, pAlarmCmds);
}

FTM_RET	FTM_CGI_ssid
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_service(pClient, pReq, pSSIDCmds);
}

FTM_RET	FTM_CGI_sys
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_service(pClient, pReq, pSysCmds);
}

FTM_RET	FTM_CGI_syslog
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_service(pClient, pReq, pSyslogCmds);
}

FTM_RET	FTM_CGI_status
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_service(pClient, pReq, pStatusCmds);
}

FTM_RET	FTM_CGI_service
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq,
	FTM_CGI_COMMAND_PTR	pCmds
)
{
	ASSERT(pReq != NULL);

	FTM_CHAR_PTR			pCmdName = pReq->getstr(pReq, "cmd", false);
	FTM_CGI_COMMAND_PTR	pCmd = pCmds;

	while(pCmd->pName != NULL)
	{
		if (strcasecmp(pCmdName, pCmd->pName) == 0)
		{
			INFO("Call Service[%s]", pCmd->pName);
			return	pCmd->fService(pClient, pReq);	
		}

		pCmd++;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_POST_cctv
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pCCTVPostCmds);
}

FTM_RET	FTM_CGI_POST_switch
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pSwitchPostCmds);
}

FTM_RET	FTM_CGI_POST_log
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pLogPostCmds);
}

FTM_RET	FTM_CGI_POST_alarm
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pAlarmPostCmds);
}

FTM_RET	FTM_CGI_POST_ssid
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pSSIDPostCmds);
}

FTM_RET	FTM_CGI_POST_sys
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pSysPostCmds);
}

FTM_RET	FTM_CGI_POST_syslog
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pSyslogPostCmds);
}

FTM_RET	FTM_CGI_POST_net
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pNetPostCmds);
}

FTM_RET	FTM_CGI_POST_status
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTM_CGI_POST_service(pClient, pReq, pStatusPostCmds);
}

FTM_RET	FTM_CGI_POST_upload
(
	FTM_CLIENT_PTR	pClient,
	qentry_t *pReq
)
{
	// get queries
	FTM_RET		xRet = FTM_RET_INVALID_ARGUMENTS;
	cJSON _PTR_ pRespRoot = cJSON_CreateObject();

	char pFileName[32];
	char pFullPathName[1024];
	char *pData      = pReq->getstr(pReq, "file", false);
	int  nLength     = pReq->getint(pReq, "file.length");
	int  nWriteLen;

	qentobj_t *obj;

	for(obj = pReq->first ; obj ; obj = obj->next)
	{
		INFO("Entry : %s", obj->name);	
	}

	// check queries
	if ((pData == NULL) || (nLength == 0)) 
	{   
		ERROR(xRet, "pData : %08x, nLength = %d", pData, nLength);	
		goto finished;
	}   

	time_t	xTime = time(NULL);
	memset(pFileName, 0, sizeof(pFileName));
	snprintf(pFileName, sizeof(pFileName) - 1, "tmp_%d", (FTM_INT32)xTime);
	snprintf(pFullPathName, sizeof(pFullPathName) - 1, "%s/%s", pClient->xConfig.pTmpPath, pFileName);

	FILE *fp = fopen(pFullPathName, "w");
	if (fp == NULL)
	{   
		ERROR(xRet, "Faile to open file!");
		goto finished;
	}   

	nWriteLen = fwrite(pData, 1, nLength, fp);
	fclose(fp); 

	if (nWriteLen != nLength)
	{   
		ERROR(xRet, "Faile to save to file!");
		goto finished;
	}   

	cJSON_AddStringToObject(pRespRoot, "file_name", pFileName);

	xRet = FTM_RET_OK;

finished:
	return	FTM_CGI_finish(pReq, pRespRoot, xRet);
}

FTM_RET	FTM_CGI_POST_service
(
	FTM_CLIENT_PTR pClient, 
	qentry_t *pReq,
	FTM_CGI_POST_COMMAND_PTR	pCmds
)
{
	ASSERT(pReq != NULL);

	FTM_RET		xRet = FTM_RET_INVALID_ARGUMENTS;
	cJSON _PTR_ pReqRoot = NULL;
	cJSON _PTR_ pRespRoot = NULL;
	pRespRoot = cJSON_CreateObject();

	FTM_CHAR_PTR	pBody = qcgireq_getquery(Q_CGI_POST);
	if (pBody == NULL)
	{
		goto finished;
	}

	pReqRoot = cJSON_Parse(pBody);
	if (pReqRoot == NULL)
	{
		ERROR(xRet, "Failed to post service with invalid json format!");
		goto finished;	
	}

	INFO("BODY : %s", pBody);
	FTM_CHAR	pCmdName[FTM_NAME_LEN+1];

	xRet = FTM_JSON_getString(pReqRoot, "cmd", sizeof(pCmdName), FTM_FALSE, pCmdName);
	if (xRet == FTM_RET_OK)
	{
		FTM_CGI_POST_COMMAND_PTR	pCmd = pCmds;

		xRet = FTM_RET_INVALID_ARGUMENTS;

		while(pCmd->pName != NULL)
		{
			if (strcasecmp(pCmdName, pCmd->pName) == 0)
			{
				INFO("Call Service[%s]", pCmd->pName);
				xRet = pCmd->fService(pClient, pReqRoot, pRespRoot);	
				break;
			}

			pCmd++;
		}
	}

finished:

	if (pBody != NULL)
	{
		free(pBody);		
	}

	if (pReqRoot != NULL)
	{
		cJSON_Delete(pReqRoot);
	}

	return	FTM_CGI_finish(pReq, pRespRoot, xRet);
}

FTM_RET	FTM_CGI_getUINT16
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_UINT16_PTR	pusValue,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pName != NULL);
	ASSERT(pusValue != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, pName, false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pusValue = (FTM_UINT16)strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getUINT32
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_UINT32_PTR pulValue,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pName != NULL);
	ASSERT(pulValue != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, pName, false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulValue = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getSTRING
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pName != NULL);
	ASSERT(pBuff != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, pName, false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		strncpy(pBuff, pValue, ulBuffLen);
	}
	
	return	FTM_RET_OK;
}

	
FTM_RET	FTM_CGI_getSSID
(
	qentry_t 		*pReq, 
	FTM_CHAR_PTR	pSessionID,
	FTM_BOOL		bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pSessionID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "ssid", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strlen(pValue) > FTM_SESSION_ID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pSessionID, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "id", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strlen(pValue) > FTM_ID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pID, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getUserID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pUserID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pUserID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "userid", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strlen(pValue) > FTM_ID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pUserID, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getServer
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pServer,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pServer != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "server", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strlen(pValue) > FTM_ID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pServer, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getSecure
(
	qentry_t *pReq, 
	FTM_BOOL_PTR	pSecure,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pSecure != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "secure", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else 
	{
		*pSecure = (strcasecmp(pValue, "on") == 0);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getSwitchID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pSwitchID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pSwitchID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "switchid", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strlen(pValue) > FTM_ID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pSwitchID, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getEnable
(
	qentry_t *pReq, 
	FTM_BOOL_PTR	pEnable,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pEnable != NULL);
	
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "enable", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strcasecmp(pValue, "true") == 0) || (strcasecmp(pValue, "yes") == 0))
	{
		*pEnable = FTM_TRUE;	
	}
	else if((strcasecmp(pValue, "false") == 0) || (strcasecmp(pValue, "no") == 0))
	{
		*pEnable = FTM_FALSE;	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getHash
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pHash,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pHash != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "hash", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strlen(pValue) > FTM_HASH_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pHash, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getComment
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pComment,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pComment != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "comment", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strlen(pValue) > FTM_ID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pComment, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getTimeout
(
	qentry_t *pReq, 
	FTM_UINT32_PTR	pTimeout,	
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pTimeout != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "timeout", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else	
	{
		*pTimeout = strtoul(pValue, 0, 10);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getDate
(
	qentry_t *pReq, 
	FTM_UINT32_PTR	pDate,	
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pDate != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "time", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else	
	{
		*pDate = strtoul(pValue, 0, 10);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getInterval
(
	qentry_t *pReq, 
	FTM_UINT32_PTR	pInterval,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pInterval != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "interval", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pInterval = strtoul(pValue, 0, 10);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getReportInterval
(
	qentry_t *pReq, 
	FTM_UINT32_PTR	pInterval,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pInterval != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "report", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pInterval = strtoul(pValue, 0, 10);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getIndex
(
	qentry_t *pReq, 
	FTM_UINT32_PTR pulIndex,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulIndex != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "index", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulIndex = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getMaxCount
(
	qentry_t *pReq, 
	FTM_UINT32_PTR pulCount,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulCount != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "max count", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulCount = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getCount
(
	qentry_t *pReq, 
	FTM_UINT32_PTR pulCount,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulCount != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "count", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulCount = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getIPString
(
	qentry_t *pReq,
	FTM_CHAR_PTR	pBuff,
	FTM_UINT32		ulBuffLen,
	FTM_BOOL bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pBuff != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "ip", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		memset(pBuff, 0, ulBuffLen);
		strncpy(pBuff, pValue, ulBuffLen - 1);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getPort
(
	qentry_t *pReq, 
	FTM_UINT16_PTR	pusPort,
	FTM_BOOL	bAllowEmpty
)
{
	return	FTM_CGI_getUINT16(pReq, "port", pusPort, bAllowEmpty);
}

FTM_RET FTM_CGI_getBeginTime
(
	qentry_t *pReq, 
	FTM_UINT32_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulTime != NULL);

		FTM_RET		xRet;
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "begin", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if(!bAllowEmpty)
		{
			xRet = FTM_RET_OBJECT_NOT_FOUND;	
			ERROR(xRet, "Failed to get begin time!");
			return	xRet;	
		}
	}
	else
	{
		FTM_TIME	xTime;

		INFO("Begin Time : %s", pValue);
		xRet = FTM_TIME_setString(&xTime, pValue);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to set time!");
			return	xRet;	
		}
			
		FTM_TIME_toSecs(&xTime, pulTime);	
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTM_CGI_getEndTime
(
	qentry_t *pReq, 
	FTM_UINT32_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulTime != NULL);

		FTM_RET		xRet;
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "end", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if(!bAllowEmpty)
		{
			xRet = FTM_RET_OBJECT_NOT_FOUND;	
			ERROR(xRet, "Failed to get end time!");
			return	xRet;	
		}
	}
	else
	{
		FTM_TIME	xTime;

		INFO("End Time : %s", pValue);
		xRet = FTM_TIME_setString(&xTime, pValue);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Failed to set time!");
			return	xRet;	
		}
			
			FTM_TIME_toSecs(&xTime, pulTime);	
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTM_CGI_getDetectTime
(
	qentry_t *pReq, 
	FTM_UINT32_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulTime != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "detect", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulTime = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTM_CGI_getHoldTime
(
	qentry_t *pReq, 
	FTM_UINT32_PTR pulTime,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulTime != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "hold", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulTime = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getSwitchModel
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pModel,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pModel != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "model", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		strncpy(pModel, pValue, FTM_MODEL_LEN);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getLogType
(
	qentry_t *pReq, 
	FTM_LOG_TYPE_PTR	pType,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pType!= NULL);
	
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "type", false);
	if((pValue == NULL) || (strlen(pValue) == 0))
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strcasecmp(pValue, "normal") == 0) || (strcasecmp(pValue, "nomal") == 0))
	{
		*pType = FTM_LOG_TYPE_NORMAL;	
	}
	else if(strcasecmp(pValue, "error") == 0)
	{
		*pType = FTM_LOG_TYPE_ERROR;	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getPolicy
(
	qentry_t *pReq, 
	FTM_SWITCH_AC_POLICY_PTR	pPolicy,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pPolicy != NULL);
	
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "policy", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if(strcasecmp(pValue, "allow") == 0)
	{
		*pPolicy = FTM_SWITCH_AC_POLICY_ALLOW;	
	}
	else if(strcasecmp(pValue, "deny") == 0)
	{
		*pPolicy = FTM_SWITCH_AC_POLICY_DENY;	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getPasswd
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pBuffer != NULL);
	
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "passwd", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) < ulBufferLen)
	{
		strncpy(pBuffer, pValue, ulBufferLen);
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getNewPasswd
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pBuffer != NULL);
	
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "new_passwd", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) < ulBufferLen)
	{
		strncpy(pBuffer, pValue, ulBufferLen);
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CGI_getSender
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pBuffer,
	FTM_UINT32		ulBufferLen,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pBuffer != NULL);
	
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "sender", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) < ulBufferLen)
	{
		strncpy(pBuffer, pValue, ulBufferLen);
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}
