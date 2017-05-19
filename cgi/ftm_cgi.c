#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ftm_client.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"

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
FTM_CGI_COMMAND	pCCTVCmds[] =
{
	{	"add",		FTM_CGI_addCCTV			},
	{	"del",		FTM_CGI_delCCTV			},
	{	"get",		FTM_CGI_getCCTV			},
	{	"set",		FTM_CGI_setCCTV			},
	{	"list",		FTM_CGI_getCCTVIDList	},
	{	"policy",	FTM_CGI_setCCTVPolicy	},
	{	"reset",	FTM_CGI_resetCCTV		},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pSwitchCmds[] =
{
	{	"add",		FTM_CGI_addSwitch		},
	{	"del",		FTM_CGI_delSwitch		},
	{	"get",		FTM_CGI_getSwitch		},
	{	"set",		FTM_CGI_setSwitch		},
	{	"list",		FTM_CGI_getSwitchIDList	},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pLogCmds[] =
{
	{	"info", FTM_CGI_getLogInfo		},
	{	"get",	FTM_CGI_getLogList		},
	{	"del",	FTM_CGI_delLog			},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pAlarmCmds[] =
{
	{	"add",  FTM_CGI_addAlarm			},
	{	"del",	FTM_CGI_delAlarm			},
	{	"get",	FTM_CGI_getAlarm			},
	{	"set",	FTM_CGI_setAlarm			},
	{	"list",	FTM_CGI_getAlarmList		},
	{	"info",	FTM_CGI_getAlarmInfo		},
	{	"get_config",	FTM_CGI_getAlarmConfig		},
	{	"set_config",	FTM_CGI_setAlarmConfig		},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pSyslogCmds[] =
{
	{	"add",		FTM_CGI_addLogServer	},
	{	"del",		FTM_CGI_delLogServer	},
	{	"get",		FTM_CGI_getLogServer	},
	{	"set",		FTM_CGI_setLogServer	},
	{	"list",		FTM_CGI_getLogServerIDList	},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pSysCmds[] =
{
	{	"info", 		FTM_CGI_getSysInfo			},
	{	"get_config", 	FTM_CGI_getSysInfoConfig	},
	{	"set_config",	FTM_CGI_setSysInfoConfig	},
	{	"check_passwd",	FTM_CGI_checkPasswd			},
	{	"passwd", 		FTM_CGI_setPasswd			},
	{	NULL,		NULL					}
};

static 
FTM_CGI_COMMAND	pStatusCmds[] =
{
	{	"info", FTM_CGI_getStatusInfo		},
	{	"get",	FTM_CGI_getStatusList		},
	{	"del",	FTM_CGI_delStatus			},
	{	NULL,		NULL					}
};

FTM_RET	FTM_CGI_finish
(
	qentry_t _PTR_ pReq,
	cJSON _PTR_ pRoot, 
	FTM_RET xRet
)
{
	cJSON_AddStringToObject(pRoot, "result", (xRet == FTM_RET_OK)?"success":"failed");	

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
	FTM_SWITCH_MODEL_PTR pModel,
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
		*pModel = FTM_getSwitchModelID(pValue);
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
