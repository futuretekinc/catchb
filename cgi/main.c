#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "ftm_cgi.h"
#include "ftm_mem.h"
#include "ftm_trace.h"

// ftm manager
#undef	__MODULE__
#define	__MODULE__	"cgi"

FTM_CGI_COMMAND	pCmds[] =
{
	{	"cctv",		FTM_CGI_cctv	},
	{	"switch",	FTM_CGI_switch	},
	{	"log",		FTM_CGI_log		},
	{	"alarm",	FTM_CGI_alarm	},
	{	"stat",		FTM_CGI_stat	},
	{	"sys",		FTM_CGI_sys		},
	{	NULL,		NULL			}
};

FTM_INT	main(FTM_INT	nArgc, FTM_CHAR_PTR pArgv[])
{
    // Parse queries.
    qentry_t *pReq = qcgireq_parse(NULL, 0);

	FTM_RET			xRet;
	FTM_CGI_COMMAND_PTR	pCmd = &pCmds[0];
	FTM_CLIENT_CONFIG	xClientConfig;
	FTM_TRACE_CONFIG	xTraceConfig;

	FTM_MEM_init();	

	FTM_CLIENT_CONFIG_setDefault(&xClientConfig);

	FTM_TRACE_CONFIG_setDefault(&xTraceConfig);
	strcpy(xTraceConfig.xLog.pFileName, "catchb_cgi.log");
	strcpy(xTraceConfig.xInfo.pFileName, "catchb_cgi.log");
	strcpy(xTraceConfig.xWarn.pFileName, "catchb_cgi.log");
	strcpy(xTraceConfig.xError.pFileName, "catchb_cgi.log");

	FTM_TRACE_setConfig(&xTraceConfig);

	INFO("CGI Start!");
	while(pCmd->pName != NULL)
	{
		if (strcasecmp(pCmd->pName, pArgv[0]) == 0)
		{
			break;
		}

		pCmd++;
	}

	if (pCmd->pName == NULL)
	{
		INFO("Command not found[%s]!", pArgv[0]);
		qcgires_error(pReq, "Not supported command!");
	}
	else	
	{
		FTM_CLIENT_PTR	pClient;

		INFO("Command Found!");
		xRet = FTM_CLIENT_create(&xClientConfig, &pClient);
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

    // De-allocate memories
    pReq->free(pReq);

	FTM_MEM_final();


    return 0;
}

