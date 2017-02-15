
#include <stdio.h>
#include <stdarg.h>
#include <liblogs.h>
#include <time.h>
#include <sys/time.h>

//test
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <liblogs.h>
#include <string.h>

#include <common.h>
#include "catchb_trace.h"

static const CATCHB_CHAR_PTR	pDefaultLogFileName = "/var/log/catchb.log";

CATCHB_VOID	CATCHB_LOG_SystemError
(
	const CATCHB_CHAR_PTR pFormat,
	...
)
{
    va_list xArgs;

    va_start(xArgs, pFormat);
    fprintf(stdout, "ERROR : ");
    vfprintf(stdout, pFormat, xArgs);
    fprintf(stdout, "\n");
    va_end(xArgs);
}

CATCHB_VOID	CATCHB_LOG_Out
(
	const CATCHB_CHAR_PTR pFileName,
	const char *			pFunctionName,
		  CATCHB_UINT32		ulLine,
	const CATCHB_CHAR_PTR pTitle,
	const CATCHB_CHAR_PTR pFormat,
	...
)
{

    FILE*	pFile;
    va_list xArgs;
	time_t	xCurrentTime;
	CATCHB_CHAR		pBuffer[2048];	
	CATCHB_UINT32	ulLen = 0;

	xCurrentTime = time(NULL);
	ulLen += strftime(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%Y-%m-%d %H:%M:%S]", localtime(&xCurrentTime));
	ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%s][%4u]", pFunctionName, ulLine);
	if (pTitle != NULL)
	{
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%s]", pTitle);
	}

    va_start(xArgs, pFormat);
    ulLen += vsnprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pFormat, xArgs);
    va_end(xArgs);

	pBuffer[ulLen++] = '\n';
	pBuffer[ulLen] = '\0';

	if(pFileName == NULL)
	{
    	pFile = fopen(pDefaultLogFileName, "a");
	}
	else
	{
    	pFile = fopen(pFileName, "a");
	}
    if(!pFile)
	{
        cctv_system_error("libcctv/cctv_system_debugger() - %s", strerror(errno));
        return;
    }

    fprintf(pFile, "%s", pBuffer);
    fclose(pFile);
}



