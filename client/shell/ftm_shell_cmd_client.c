#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_shell.h"
#include "ftm_mem.h"
#include "ftm_client.h"

FTM_RET	FTM_SHELL_CMD_client
(
	FTM_SHELL_PTR pShell, 
	FTM_INT nArgc, 
	FTM_CHAR_PTR pArgv[], 
	FTM_VOID_PTR pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);
	ASSERT(pData != NULL);
	FTM_RET	xRet = FTM_RET_OK;

	return	xRet;
}


