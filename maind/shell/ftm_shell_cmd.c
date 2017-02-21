#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"

FTM_RET	FTM_SHELL_CMD_db(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_analyzer(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);

FTM_SHELL_CMD	pCatchBShellCmdList[] = 
{
	{
		.pString	= "db",
		.function	= FTM_SHELL_CMD_db,
		.pShortHelp	= "DB",
		.pHelp		= "DB",
		.pData		= NULL
	},
	{
		.pString	= "analyzer",
		.function	= FTM_SHELL_CMD_analyzer,
		.pShortHelp	= "Analyzer",
		.pHelp		= "Analyzer",
		.pData		= NULL
	},
};

FTM_UINT32	ulCatchBShellCmdCount = sizeof(pCatchBShellCmdList) / sizeof(FTM_SHELL_CMD);
