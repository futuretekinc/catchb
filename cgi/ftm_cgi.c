#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ftm_client.h"
#include "ftm_cgi.h"
#include "ftm_cgi_command.h"

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
	{	"list",		FTM_CGI_getCCTVIDList		},
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

	qcgires_setcontenttype(pReq, "text/xml");
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
			return	pCmd->fService(pClient, pReq);	
		}

		pCmd++;
	}

	return	FTM_RET_OK;
}

/*
 * 'whitespace_cb()' - Let the mxmlSaveFile() function know when to insert
 *                     newlines and tabs...
 */

const 
char *FTM_CGI_whitespaceCB
(
	mxml_node_t *node,
	int			where
)
{
	mxml_node_t	*parent;		/* Parent node */
	int		level;			/* Indentation level */
	const char	*name;			/* Name of element */
	static const char *tabs = "\t\t\t\t\t\t\t\t";
	/* Tabs for indentation */


	/*
	 * We can conditionally break to a new line before or after any element.
	 * These are just common HTML elements...
	 */

	name = node->value.element.name;

	if (!strcmp(name, "html") || !strcmp(name, "head") || !strcmp(name, "body") ||
		!strcmp(name, "pre") || !strcmp(name, "p") ||
		!strcmp(name, "h1") || !strcmp(name, "h2") || !strcmp(name, "h3") ||
		!strcmp(name, "h4") || !strcmp(name, "h5") || !strcmp(name, "h6"))
	{
		/*
		 * Newlines before open and after close...
		 */

		if (where == MXML_WS_BEFORE_OPEN || where == MXML_WS_AFTER_CLOSE)
			return ("\n");
	}
	else if (!strcmp(name, "dl") || !strcmp(name, "ol") || !strcmp(name, "ul"))
	{
		/*
		 * Put a newline before and after list elements...
		 */

		return ("\n");
	}
	else if (!strcmp(name, "dd") || !strcmp(name, "dt") || !strcmp(name, "li"))
	{
		/*
		 * Put a tab before <li>'s, <dd>'s, and <dt>'s, and a newline after them...
		 */

		if (where == MXML_WS_BEFORE_OPEN)
			return ("\t");
		else if (where == MXML_WS_AFTER_CLOSE)
			return ("\n");
	}
	else if (!strncmp(name, "?xml", 4))
	{
		if (where == MXML_WS_AFTER_OPEN)
			return ("\n");
		else
			return (NULL);
	}
	else if (where == MXML_WS_BEFORE_OPEN ||
			((!strcmp(name, "choice") || !strcmp(name, "option")) &&
			 where == MXML_WS_BEFORE_CLOSE))
	{
		for (level = -1, parent = node->parent;
				parent;
				level ++, parent = parent->parent);

		if (level > 8)
			level = 8;
		else if (level < 0)
			level = 0;

		return (tabs + 8 - level);
	}
	else if (where == MXML_WS_AFTER_CLOSE ||
			((!strcmp(name, "group") || !strcmp(name, "option") ||
			  !strcmp(name, "choice")) &&
			 where == MXML_WS_AFTER_OPEN))
		return ("\n");
	else if (where == MXML_WS_AFTER_OPEN && !node->child)
		return ("\n");

	/*
	 * Return NULL for no added whitespace...
	 */

	return (NULL);
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

FTM_RET	FTM_CGI_getPasswd
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pPasswd,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pPasswd != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "passwd", false);
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
		strcpy(pPasswd, pValue);
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
	else if(strcasecmp(pValue, "true") == 0)
	{
		*pEnable = FTM_TRUE;	
	}
	else if(strcasecmp(pValue, "false") == 0)
	{
		*pEnable = FTM_FALSE;	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
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
	if(pValue == NULL)
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
	if(pValue == NULL)
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
	if (pValue == NULL)
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

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "begin", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		if (strlen(pValue) == 14)
		{
			FTM_RET		xRet;
			FTM_TIME	xTime;

			xRet = FTM_TIME_setString(&xTime, pValue);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;	
			}
			
			FTM_TIME_toSecs(&xTime, pulTime);	
		}
		else
		{
			*pulTime = strtoul(pValue, 0, 10);
		}
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

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "end", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		if (strlen(pValue) == 14)
		{
			FTM_RET		xRet;
			FTM_TIME	xTime;

			xRet = FTM_TIME_setString(&xTime, pValue);
			if (xRet != FTM_RET_OK)
			{
				return	xRet;	
			}
			
			FTM_TIME_toSecs(&xTime, pulTime);	
		}
		else
		{
			*pulTime = strtoul(pValue, 0, 10);
		}
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

