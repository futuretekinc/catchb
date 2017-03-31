#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "ftm_cgi.h"
#include "ftm_mem.h"
#include "ftm_trace.h"

// ftm manager

FTM_CGI_COMMAND	pCmds[] =
{
	{	"cctv",		FTM_CGI_cctv	},
	{	"switch",	FTM_CGI_switch	},
	{	"log",		FTM_CGI_log	},
	{	"alarm",		FTM_CGI_alarm},
	{	NULL,		NULL			}
};

FTM_INT	main(FTM_INT	nArgc, FTM_CHAR_PTR pArgv[])
{
    // Parse queries.
    qentry_t *pReq = qcgireq_parse(NULL, 0);

	FTM_RET			xRet;
	FTM_CGI_COMMAND_PTR	pCmd = &pCmds[0];

	FTM_MEM_init();	

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
		qcgires_error(pReq, "Not supported command!");
	}
	else	
	{
		FTM_CLIENT_PTR	pClient;
		
		xRet = FTM_CLIENT_create(&pClient);
		if (xRet != FTM_RET_OK)
		{
			qcgires_error(pReq, "Internal Error!");
		}

		xRet = FTM_CLIENT_connect(pClient, "127.0.0.1", 8800);
		if (xRet == FTM_RET_OK)
		{
       		pCmd->fService(pClient, pReq);

			FTM_CLIENT_disconnect(pClient);
		}

		FTM_CLIENT_destroy(&pClient);

	}	

	FTM_MEM_final();

    // De-allocate memories
    pReq->free(pReq);

    return 0;
}

