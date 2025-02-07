#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "ftm_cgi.h"
#include "ftm_mem.h"
#include "ftm_trace.h"
#include "ftm_config.h"

// ftm manager
#undef	__MODULE__
#define	__MODULE__	"cgi"

FTM_CGI_COMMAND	pPostCmds[] =
{
	{	"cctv",		FTM_CGI_POST_cctv	},
	{	"switch",	FTM_CGI_POST_switch	},
	{	"log",		FTM_CGI_POST_log		},
	{	"alarm",	FTM_CGI_POST_alarm	},
	{	"ssid",		FTM_CGI_POST_ssid	},
	{	"status",	FTM_CGI_POST_status	},
	{	"sys",		FTM_CGI_POST_sys		},
	{	"syslog",	FTM_CGI_POST_syslog	},
	{	"upload",	FTM_CGI_POST_upload	},
	{	"net",		FTM_CGI_POST_net},
	{	NULL,		NULL			}
};
FTM_CGI_COMMAND	pCmds[] =
{
	{	"cctv",		FTM_CGI_cctv	},
	{	"switch",	FTM_CGI_switch	},
	{	"log",		FTM_CGI_log		},
	{	"alarm",	FTM_CGI_alarm	},
	{	"ssid",		FTM_CGI_ssid	},
	{	"status",	FTM_CGI_status	},
	{	"sys",		FTM_CGI_sys		},
	{	"syslog",	FTM_CGI_syslog	},
	{	NULL,		NULL			}
};

extern	char*	program_invocation_short_name;

FTM_INT	main(FTM_INT	nArgc, FTM_CHAR_PTR pArgv[])
{
    // Parse queries.
    qentry_t *pReq = qcgireq_parse(NULL, 0);

	FTM_RET			xRet;
	FTM_CGI_COMMAND_PTR	pCmd = &pPostCmds[0];
	FTM_CONFIG_PTR		pConfig = NULL;
	FTM_CHAR			pConfigFileName[1024];

	FTM_MEM_init();	

	snprintf(pConfigFileName, sizeof(pConfigFileName) - 1, "/etc/%s.conf", "catchb_cgi");
	
	xRet = FTM_CONFIG_create(&pConfig);
	if (xRet != FTM_RET_OK)
	{
		goto finished;
	}

	FTM_CONFIG_load(pConfig, pConfigFileName);

	FTM_TRACE_CONFIG_setFileName(&pConfig->xTrace, "catchb_cgi.log");
	FTM_TRACE_setConfig(&pConfig->xTrace);

	INFO("CGI Start!");
	while(pCmd->pName != NULL)
	{
		if (strcasecmp(pCmd->pName, program_invocation_short_name) == 0)
		{
			break;
		}

		pCmd++;
	}

	if (pCmd->pName == NULL)
	{
		INFO("Command not found[%s]!", program_invocation_short_name);
		qcgires_error(pReq, "Not supported command[%s]!", program_invocation_short_name);
	}
	else	
	{
		FTM_CLIENT_PTR	pClient;

		INFO("Command Found[%s]!", pCmd->pName);
		xRet = FTM_CLIENT_create(&pConfig->xClient, &pClient);
		if (xRet != FTM_RET_OK)
		{
			qcgires_error(pReq, "Internal Error!");
		}

		xRet = FTM_CLIENT_connect(pClient);
		if (xRet == FTM_RET_OK)
		{
       		pCmd->fService(pClient, pReq);

			FTM_CLIENT_disconnect(pClient);
		}

		FTM_CLIENT_destroy(&pClient);

	}	

finished:
    // De-allocate memories
    pReq->free(pReq);


	if (pConfig != NULL)
	{
		FTM_CONFIG_destroy(&pConfig);	
	}

	FTM_MEM_final();
    return 0;
}

