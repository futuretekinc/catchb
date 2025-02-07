#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ftm_shell.h"
#include "ftm_trace.h"
#include "ftm_list.h"
#include "ftm_mem.h"

#define	FTM_SHELL_LINE_LEN	2048

static 
FTM_RET	FTM_SHELL_parseLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
);

static
FTM_RET	FTM_SHELL_defaultOut
(
	FTM_SHELL_PTR	pShell,
	const FTM_CHAR_PTR	pFormat, 
	...
); 

static 
FTM_RET FTM_SHELL_getCmd
(
	FTM_SHELL_PTR				pShell,
	FTM_CHAR_PTR 				pCmdString, 
	FTM_SHELL_CMD_PTR _PTR_ 	ppCmd
);

static 
FTM_RET	FTM_SHELL_cmdHelp
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[],
	FTM_VOID_PTR	pData
);

static 
FTM_RET	FTM_SHELL_cmdQuit
(	
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[],
	FTM_VOID_PTR	pData
);

static FTM_BOOL	FTM_SHELL_seeker
(
	const FTM_VOID_PTR	pItem, 
	const FTM_VOID_PTR	pKey
);

static FTM_INT	FTM_SHELL_comparator
(
	const FTM_VOID_PTR	pItem1, 
	const FTM_VOID_PTR	pItem2
);

const FTM_SHELL_CMD	xDefaultCmds[] = 
{
	{	
		.pString	= "help",
		.function	= FTM_SHELL_cmdHelp,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help.",
		.pData		= NULL
	},
	{	
		.pString	= "?",
		.function	= FTM_SHELL_cmdHelp,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help.",
		.pData		= NULL
	},
	{	
		.pString	= "quit",
		.function	= FTM_SHELL_cmdQuit,
		.pShortHelp = "Quit program.",
		.pHelp 		= "\n"\
					  "\tQuit program.",
		.pData		= NULL
	}
};

FTM_VOID_PTR	FTM_SHELL_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_SHELL_PTR	pShell = (FTM_SHELL_PTR)pData;

	FTM_RET			nRet;
	FTM_CHAR		pCmdLine[2048];
	FTM_INT			nArgc;
	FTM_CHAR_PTR	pArgv[FTM_SHELL_MAX_ARGS];

	pShell->bStop = FTM_FALSE;
	
	while(!pShell->bStop)
	{
		printf("%s> ", pShell->pPrompt);

		memset(pCmdLine, 0, sizeof(pCmdLine));
		fgets(pCmdLine, sizeof(pCmdLine), stdin);

		FTM_SHELL_parseLine(pCmdLine, pArgv, FTM_SHELL_MAX_ARGS, &nArgc);

		if (nArgc != 0)
		{
			FTM_SHELL_CMD_PTR 	pCmd;

			nRet = FTM_SHELL_getCmd(pShell, pArgv[0], &pCmd);
			if (nRet == FTM_RET_OK)
			{
				INFO("Command : %s", pArgv[0]);
				nRet = pCmd->function(pShell, nArgc, pArgv, (pCmd->pData != NULL)?pCmd->pData:pShell->pData);
				switch(nRet)
				{
				case	FTM_RET_INVALID_ARGUMENTS:
					{
						printf("Usage : %s %s\n", pCmd->pString, pCmd->pHelp);
					}
					break;

				case	FTM_RET_SHELL_QUIT:
					{
						pShell->bStop = FTM_TRUE;
					}
				}
			}
			else
			{
				FTM_CHAR_PTR pNewArgv[] = {"help"};
				printf("%s is invalid command.\n", pArgv[0]);
				FTM_SHELL_cmdHelp(pShell, 1, pNewArgv, pShell);

			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_SHELL_init
(
	FTM_SHELL_PTR	pShell,
	FTM_CHAR_PTR	pPrompt,
	FTM_SHELL_CMD_PTR	pCmds,
	FTM_UINT32		ulCmdCount,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pShell != NULL);

	FTM_SHELL_CMD_PTR	pCmd;
	FTM_INT				i;

	if (pPrompt != NULL)
	{
		strncpy(pShell->pPrompt, pPrompt, sizeof(pShell->pPrompt));
	}

	if (pData != NULL)
	{
		pShell->pData = pData;
	}

	pShell->bStop = FTM_TRUE;
	pShell->fPrintOut = FTM_SHELL_defaultOut;

	FTM_LIST_create(&pShell->pCmdList);
	FTM_LIST_setSeeker(pShell->pCmdList, FTM_SHELL_seeker);
	FTM_LIST_setComparator(pShell->pCmdList, FTM_SHELL_comparator);

	for(i = 0 ; i < sizeof(xDefaultCmds) / sizeof(FTM_SHELL_CMD) ; i++)
	{
		pCmd = (FTM_SHELL_CMD_PTR)FTM_MEM_malloc(sizeof(FTM_SHELL_CMD));
		if (pCmd != NULL)
		{
			memcpy(pCmd, &xDefaultCmds[i], sizeof(FTM_SHELL_CMD));
			FTM_LIST_insert(pShell->pCmdList, pCmd, FTM_LIST_POS_ASSENDING);
		}
	}

	return	FTM_SHELL_addCmds(pShell, pCmds, ulCmdCount);
}	

FTM_RET	FTM_SHELL_final
(
	FTM_SHELL_PTR	pShell
)
{
	ASSERT(pShell != NULL);

	FTM_RET				xRet;
	FTM_SHELL_CMD_PTR	pCmd;

	if (pShell->pCmdList != NULL)
	{
		FTM_LIST_iteratorStart(pShell->pCmdList);
		while(FTM_LIST_iteratorNext(pShell->pCmdList, (FTM_VOID_PTR _PTR_)&pCmd) == FTM_RET_OK)
		{
			xRet = FTM_LIST_remove(pShell->pCmdList, pCmd);
			if (xRet == FTM_RET_OK)
			{
				FTM_MEM_free(pCmd);	
			}
		}

		FTM_LIST_destroy(&pShell->pCmdList);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_start
(
	FTM_SHELL_PTR	pShell
)
{
	ASSERT(pShell != NULL);

	pthread_create(&pShell->xThread, NULL, FTM_SHELL_process, pShell);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_stop
(
	FTM_SHELL_PTR	pShell
)
{
	ASSERT(pShell != NULL);

	pShell->bStop = FTM_TRUE;

	pthread_join(pShell->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_run
(
	FTM_SHELL_PTR	pShell
)
{
	ASSERT(pShell != NULL);

	FTM_SHELL_process(pShell);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_run2
(
	FTM_CHAR_PTR	pPrompt,
	FTM_SHELL_CMD_PTR	pCmds,
	FTM_UINT32		ulCmdCount,
	FTM_VOID_PTR	pData
)
{
	FTM_RET		xRet;
	FTM_SHELL	xShell;

	xRet = FTM_SHELL_init(&xShell, pPrompt, pCmds, ulCmdCount, pData);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_SHELL_run(&xShell);

	FTM_SHELL_final(&xShell);

	return	xRet;
}

FTM_RET	FTM_SHELL_setPrompt
(
	FTM_SHELL_PTR	pShell,
	FTM_CHAR_PTR	pPrompt
)
{
	ASSERT(pShell != NULL);

	strncpy(pShell->pPrompt, pPrompt, sizeof(pShell->pPrompt));

	return	FTM_RET_OK;
}


FTM_RET	FTM_SHELL_addCmds
(
	FTM_SHELL_PTR		pShell,
	FTM_SHELL_CMD_PTR 	pCmds, 
	FTM_UINT32 			ulCmds
)
{
	FTM_UINT32			i;
	FTM_RET				xRet;

	for(i = 0 ; i < ulCmds; i++)
	{
		xRet = FTM_SHELL_appendCmd(pShell, &pCmds[i]);
		if (xRet != FTM_RET_OK)
		{
			ERROR(xRet, "Command append failed.\n");	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_appendCmd
(
	FTM_SHELL_PTR		pShell,
	FTM_SHELL_CMD_PTR 	pCmd
)
{
	FTM_SHELL_CMD_PTR	pExistCmd;
	FTM_SHELL_CMD_PTR	pNewCmd;
	FTM_RET				xRet;

	if (FTM_LIST_get(pShell->pCmdList, pCmd->pString, (FTM_VOID_PTR _PTR_)&pExistCmd) == FTM_RET_OK)
	{
		xRet = FTM_LIST_remove(pShell->pCmdList, pExistCmd);
		if (xRet == FTM_RET_OK)
		{
			FTM_MEM_free(pExistCmd);	
		}
	}

	pNewCmd = (FTM_SHELL_CMD_PTR)FTM_MEM_malloc(sizeof(FTM_SHELL_CMD));
	if (pNewCmd == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR(xRet, "Not enough memory!\n");
		return	xRet;
	}

	memcpy(pNewCmd, pCmd, sizeof(FTM_SHELL_CMD));
	xRet = FTM_LIST_insert(pShell->pCmdList, pNewCmd, FTM_LIST_POS_ASSENDING);
	if (xRet != FTM_RET_OK)
	{
		ERROR(xRet, "Command insert failed.\n");	
		FTM_MEM_free(pNewCmd);
	}
	
	return	xRet;
}

FTM_RET FTM_SHELL_getCmd
(
	FTM_SHELL_PTR	pShell,
	FTM_CHAR_PTR	pCmdString, 
	FTM_SHELL_CMD_PTR _PTR_ ppCmd
)
{
	FTM_SHELL_CMD_PTR pCmd;

	FTM_LIST_iteratorStart(pShell->pCmdList);
	while(FTM_LIST_iteratorNext(pShell->pCmdList, (FTM_VOID_PTR _PTR_)&pCmd) == FTM_RET_OK)
	{
		if (strcasecmp(pCmdString, pCmd->pString) == 0)
		{
			*ppCmd = pCmd;
			return	FTM_RET_OK;
		}
	}

	if (strlen(pCmdString) > 1)
	{
		FTM_LIST_iteratorStart(pShell->pCmdList);
		while(FTM_LIST_iteratorNext(pShell->pCmdList, (FTM_VOID_PTR _PTR_)&pCmd) == FTM_RET_OK)
		{
			if (strncasecmp(pCmdString, pCmd->pString, strlen(pCmdString)) == 0)
			{
				*ppCmd = pCmd;
				return	FTM_RET_OK;
			}
		}
	}

	return	FTM_RET_INVALID_COMMAND;
}

#if 0
FTM_RET	FTM_SHELL_parseLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
)
{
	FTM_INT		nCount = 0;
	FTM_CHAR_PTR	pWord = NULL;
	FTM_CHAR_PTR	pSeperator = "\t \n\r";	

	pWord = strtok(pLine, pSeperator); 
	while((pWord != NULL) && (nCount < nMaxArgs))
	{
		pArgv[nCount++] = pWord;
		pWord = strtok(NULL, pSeperator);
	}

	*pArgc = nCount;

	return	FTM_RET_OK;
}
#endif
FTM_RET	FTM_SHELL_parseLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
)
{
	FTM_INT			nCount = 0;

	while(*pLine != '\0')
	{
		if (nMaxArgs <= nCount)
		{
			break;
		}

		if ((*pLine != '\t') && (*pLine != ' ') && (*pLine != '\n') && (*pLine != '\r'))
		{
			if (*pLine == '"')
			{
				FTM_CHAR_PTR	pEnd;

				pEnd = strchr(pLine+1, '"');
				if (pEnd == NULL)
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				pArgv[nCount++] = pLine+1;
				*pEnd = '\0';

				pLine = pEnd+1;
			}
			else
			{
				pArgv[nCount++] = pLine;
				while((*pLine != '\0') && (*pLine != '\t') && (*pLine != ' ') && (*pLine != '\n') && (*pLine != '\r'))
				{
					pLine++;
				}

				if (*pLine != '\0')
				{
					*pLine = '\0';	
					pLine++;
				}
			}
		}
		else
		{
			pLine++;	
		}

	}

	*pArgc = nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_cmdHelp
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	switch(nArgc)
	{
	case	1:
		{
			FTM_SHELL_CMD_PTR	pCmd; 
		
			FTM_LIST_iteratorStart(pShell->pCmdList);
			while(FTM_LIST_iteratorNext(pShell->pCmdList, (FTM_VOID_PTR _PTR_)&pCmd) == FTM_RET_OK)
			{
				printf("%-16s    %s\n", pCmd->pString, pCmd->pShortHelp); 
			}
		}
		break;

	case	2:
		{
			FTM_SHELL_CMD_PTR pCmd;

			if (FTM_SHELL_getCmd(pShell, pArgv[1], &pCmd) == FTM_RET_OK)
			{
				printf("Usage : %s %s\n", pArgv[1], pCmd->pHelp);
			}
		}
		break;

	default:
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_cmdQuit
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR	pData
)
{
	return	FTM_RET_SHELL_QUIT;
}

FTM_BOOL FTM_SHELL_seeker(const FTM_VOID_PTR pItem, const FTM_VOID_PTR pKey)
{
	FTM_SHELL_CMD_PTR	pCmd = (FTM_SHELL_CMD_PTR)pItem;
	FTM_CHAR_PTR			pCmdString = (FTM_CHAR_PTR)pKey;

	return	(strcmp(pCmd->pString, pCmdString) == 0);
}

FTM_INT	FTM_SHELL_comparator(const FTM_VOID_PTR pItem1, const FTM_VOID_PTR pItem2)
{
	FTM_SHELL_CMD_PTR	pCmd1 = (FTM_SHELL_CMD_PTR)pItem1;
	FTM_SHELL_CMD_PTR	pCmd2 = (FTM_SHELL_CMD_PTR)pItem2;

	return	strcmp(pCmd1->pString, pCmd2->pString);
}

FTM_RET	FTM_SHELL_print
(
	FTM_SHELL_PTR		pShell,
	const FTM_CHAR_PTR	pFormat, 
	...
)
{
	ASSERT(pShell != NULL);
	ASSERT(pFormat != NULL);

	return	pShell->fPrintOut(pShell, pFormat);
}

FTM_RET	FTM_SHELL_defaultOut
(
	FTM_SHELL_PTR		pShell,
	const FTM_CHAR_PTR	pFormat, 
	...
)
{
    va_list 		pArg;
	FTM_INT			nLen = 0;
	FTM_CHAR		szBuff[FTM_SHELL_LINE_LEN + 1];

	va_start ( pArg, pFormat );           

	nLen += vsnprintf( &szBuff[nLen], FTM_SHELL_LINE_LEN - nLen, pFormat, pArg);

	va_end(pArg);

	printf("%s", szBuff);

	return	FTM_RET_OK;
}

