
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
#include "ftm_trace.h"

static const 
FTM_CHAR_PTR	pDefaultLogFileNames[] =
{
	"/var/log/catchb.log",
	"/var/log/catchb.trace",
	"/var/log/catchb.trace"
};

FTM_VOID	FTM_LOG_SystemError
(
	const FTM_CHAR_PTR pFormat,
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

FTM_VOID	FTM_LOG_Out
(
		  FTM_UINT32	ulOutput,
	const char *		pFunctionName,
		  FTM_UINT32	ulLine,
	const FTM_CHAR_PTR	pModuleName,
	const FTM_CHAR_PTR 	pTitle,
	const FTM_CHAR_PTR 	pFormat,
	...
)
{

    FILE*		pFile;
    va_list 	xArgs;
	time_t		xCurrentTime;
	FTM_CHAR	pBuffer[2048];	
	FTM_UINT32	ulLen = 0;

	memset(pBuffer, 0, sizeof(pBuffer));

	xCurrentTime = time(NULL);
	ulLen += strftime(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%Y-%m-%d %H:%M:%S] ", localtime(&xCurrentTime));
	if (ulOutput != 0)
	{
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[");
		snprintf(&pBuffer[ulLen], 25, "%24s", pFunctionName);
		ulLen += 24;
		ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "][%4u]", ulLine);

		if (pModuleName != NULL)
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[");
			snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "%8s", pModuleName);
			ulLen += 8;
			if(strlen(pModuleName) > 8)
			{
				pBuffer[ulLen] = '\0';
			}
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "]");
		}
		else
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%8s]", "COMMON");
		}

		if (pTitle != NULL)
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[");
			snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "%8s", pTitle);
			ulLen += 8;
			if(strlen(pTitle) > 8)
			{
				pBuffer[ulLen] = '\0';
			}
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "] ");
		}
		else
		{
			ulLen += snprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, "[%8s] ", "COMMON");
		}
	}

    va_start(xArgs, pFormat);
    ulLen += vsnprintf(&pBuffer[ulLen], sizeof(pBuffer) - ulLen, pFormat, xArgs);
    va_end(xArgs);

	pBuffer[ulLen++] = '\n';
	pBuffer[ulLen] = '\0';

   	pFile = fopen(pDefaultLogFileNames[ulOutput], "a");
    if(!pFile)
	{
        printf("%s", pBuffer);
    }
	else
	{
    	fprintf(pFile, "%s", pBuffer);
    	fclose(pFile);
	}
}



