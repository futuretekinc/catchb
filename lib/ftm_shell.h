#ifndef	__FTM_SHELL_H__
#define	__FTM_SHELL_H__

#include "ftm_types.h"
#include "ftm_list.h"
#include <pthread.h>

#define		FTM_SHELL_MAX_ARGS	16


struct FTM_SHELL_STRUCT;

typedef	FTM_RET	(*FTM_SHELL_PRINT)(struct FTM_SHELL_STRUCT _PTR_ pShell,const FTM_CHAR_PTR	pFormat, ...); 

typedef	FTM_RET	(*FTM_SHELL_CMD_CB)
(
	struct FTM_SHELL_STRUCT _PTR_ pShell, 
	FTM_INT nArgc, 
	FTM_CHAR_PTR pArgv[], 
	FTM_VOID_PTR pData
);

typedef struct	FTM_SHELL_CMD_STRUCT
{
	FTM_CHAR_PTR		pString;
	FTM_SHELL_CMD_CB	function;
	FTM_CHAR_PTR		pShortHelp;
	FTM_CHAR_PTR		pHelp;
	FTM_VOID_PTR		pData;
}	FTM_SHELL_CMD, _PTR_ FTM_SHELL_CMD_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pPrompt;
	FTM_UINT32			ulCmdCount;
	FTM_SHELL_CMD_PTR	pCmdList;
	FTM_VOID_PTR		pDefaultData;
}	FTM_SHELL_CONFIG, _PTR_ FTM_SHELL_CONFIG_PTR;

typedef	struct FTM_SHELL_STRUCT
{
	FTM_CHAR		pPrompt[128];
	FTM_LIST_PTR	pCmdList;
	pthread_t		xThread;
	FTM_BOOL		bStop;
	FTM_VOID_PTR	pData;

	FTM_SHELL_PRINT	fPrintOut;
}	FTM_SHELL, _PTR_ FTM_SHELL_PTR;

FTM_RET FTM_SHELL_init
(
	FTM_SHELL_PTR	pShell,
	FTM_CHAR_PTR	pPrompt,
	FTM_SHELL_CMD_PTR	pCmds,
	FTM_UINT32		ulCmdCount,
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_SHELL_final
(
	FTM_SHELL_PTR	pShell
);

FTM_RET	FTM_SHELL_run
(
	FTM_SHELL_PTR	pShell
);

FTM_RET	FTM_SHELL_setPrompt
(
	FTM_SHELL_PTR	pShell,
	FTM_CHAR_PTR	pPrompt
);

FTM_RET	FTM_SHELL_addCmds
(
	FTM_SHELL_PTR		pShell,
	FTM_SHELL_CMD_PTR 	pCmds, 
	FTM_UINT32 			ulCmds
);

FTM_RET	FTM_SHELL_appendCmd
(
	FTM_SHELL_PTR		pShell,
	FTM_SHELL_CMD_PTR 	pCmd
);

FTM_RET	FTM_SHELL_run2
(
	FTM_CHAR_PTR	pPrompt,
	FTM_SHELL_CMD_PTR	pCmds,
	FTM_UINT32		ulCmdCount,
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_SHELL_print
(
	FTM_SHELL_PTR	pShell,
	const FTM_CHAR_PTR	pFormat, 
	...
); 
#endif
