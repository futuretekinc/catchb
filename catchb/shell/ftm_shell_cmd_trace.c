#include <string.h>
#include <stdlib.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"
#include "ftm_smtpc.h"

extern	FTM_TRACE_CONFIG	xTraceConfig;

FTM_RET	FTM_SHELL_CMD_showTraceConfig(FTM_VOID)
{
	printf("%8s %7s %21s %21s %21s %21s %21s\n", "", "", "TIME", "FUNCTION", "LINE", "MODULE", "LEVEL");
	printf("%8s %7s %7s %8s %4s %7s %8s %4s %7s %8s %4s %7s %8s %4s %7s %8s %4s %s\n", 
		"", "Enable", "Enable", "Dynamic", "Size", "Enable", "Dynamic", "Size", "Enable", "Dynamic", "Size", "Enable", "Dynamic", "Size", "Enable", "Dynamic", "Size", "File");
	printf("%8s %7s %7s %8s %4d %7s %8s %4d %7s %8s %4d %7s %8s %4d %7s %8s %4d %s\n", 
		"LOG", (xTraceConfig.xLog.bEnable)?"Yes":"No",
		(xTraceConfig.xLog.xTime.bEnable)?"Yes":"No", 		(xTraceConfig.xLog.xTime.bDynamic)?"Yes":"No", 		xTraceConfig.xLog.xTime.ulSize,
		(xTraceConfig.xLog.xFunction.bEnable)?"Yes":"No", 	(xTraceConfig.xLog.xFunction.bDynamic)?"Yes":"No", 	xTraceConfig.xLog.xFunction.ulSize,
		(xTraceConfig.xLog.xLine.bEnable)?"Yes":"No", 		(xTraceConfig.xLog.xLine.bDynamic)?"Yes":"No", 		xTraceConfig.xLog.xLine.ulSize,
		(xTraceConfig.xLog.xModule.bEnable)?"Yes":"No", 		(xTraceConfig.xLog.xModule.bDynamic)?"Yes":"No", 	xTraceConfig.xLog.xModule.ulSize,
 		(xTraceConfig.xLog.xLevel.bEnable)?"Yes":"No", 		(xTraceConfig.xLog.xLevel.bDynamic)?"Yes":"No", 		xTraceConfig.xLog.xLevel.ulSize,
		xTraceConfig.xLog.pFileName);

	printf("%8s %7s %7s %8s %4d %7s %8s %4d %7s %8s %4d %7s %8s %4d %7s %8s %4d %s\n", 
		"INFO", (xTraceConfig.xInfo.bEnable)?"Yes":"No",
		(xTraceConfig.xInfo.xTime.bEnable)?"Yes":"No", 		(xTraceConfig.xInfo.xTime.bDynamic)?"Yes":"No", 		xTraceConfig.xInfo.xTime.ulSize,
		(xTraceConfig.xInfo.xFunction.bEnable)?"Yes":"No", 	(xTraceConfig.xInfo.xFunction.bDynamic)?"Yes":"No", 	xTraceConfig.xInfo.xFunction.ulSize,
		(xTraceConfig.xInfo.xLine.bEnable)?"Yes":"No", 		(xTraceConfig.xInfo.xLine.bDynamic)?"Yes":"No", 		xTraceConfig.xInfo.xLine.ulSize,
		(xTraceConfig.xInfo.xModule.bEnable)?"Yes":"No", 	(xTraceConfig.xInfo.xModule.bDynamic)?"Yes":"No", 	xTraceConfig.xInfo.xModule.ulSize,
 		(xTraceConfig.xInfo.xLevel.bEnable)?"Yes":"No", 		(xTraceConfig.xInfo.xLevel.bDynamic)?"Yes":"No", 	xTraceConfig.xInfo.xLevel.ulSize,
		xTraceConfig.xInfo.pFileName);

	printf("%8s %7s %7s %8s %4d %7s %8s %4d %7s %8s %4d %7s %8s %4d %7s %8s %4d %s\n", 
		"WARN", (xTraceConfig.xWarn.bEnable)?"Yes":"No",
		(xTraceConfig.xWarn.xTime.bEnable)?"Yes":"No", 		(xTraceConfig.xWarn.xTime.bDynamic)?"Yes":"No", 		xTraceConfig.xWarn.xTime.ulSize,
		(xTraceConfig.xWarn.xFunction.bEnable)?"Yes":"No", 	(xTraceConfig.xWarn.xFunction.bDynamic)?"Yes":"No", 	xTraceConfig.xWarn.xFunction.ulSize,
		(xTraceConfig.xWarn.xLine.bEnable)?"Yes":"No", 		(xTraceConfig.xWarn.xLine.bDynamic)?"Yes":"No", 		xTraceConfig.xWarn.xLine.ulSize,
		(xTraceConfig.xWarn.xModule.bEnable)?"Yes":"No", 	(xTraceConfig.xWarn.xModule.bDynamic)?"Yes":"No", 	xTraceConfig.xWarn.xModule.ulSize,
 		(xTraceConfig.xWarn.xLevel.bEnable)?"Yes":"No", 		(xTraceConfig.xWarn.xLevel.bDynamic)?"Yes":"No", 	xTraceConfig.xWarn.xLevel.ulSize,
		xTraceConfig.xWarn.pFileName);

	printf("%8s %7s %7s %8s %4d %7s %8s %4d %7s %8s %4d %7s %8s %4d %7s %8s %4d %s\n", 
		"ERROR", (xTraceConfig.xError.bEnable)?"Yes":"No",
		(xTraceConfig.xError.xTime.bEnable)?"Yes":"No", 		(xTraceConfig.xError.xTime.bDynamic)?"Yes":"No", 	xTraceConfig.xError.xTime.ulSize,
		(xTraceConfig.xError.xFunction.bEnable)?"Yes":"No", 	(xTraceConfig.xError.xFunction.bDynamic)?"Yes":"No", xTraceConfig.xError.xFunction.ulSize,
		(xTraceConfig.xError.xLine.bEnable)?"Yes":"No", 		(xTraceConfig.xError.xLine.bDynamic)?"Yes":"No", 	xTraceConfig.xError.xLine.ulSize,
		(xTraceConfig.xError.xModule.bEnable)?"Yes":"No", 	(xTraceConfig.xError.xModule.bDynamic)?"Yes":"No", 	xTraceConfig.xError.xModule.ulSize,
 		(xTraceConfig.xError.xLevel.bEnable)?"Yes":"No", 	(xTraceConfig.xError.xLevel.bDynamic)?"Yes":"No", 	xTraceConfig.xError.xLevel.ulSize,
		xTraceConfig.xError.pFileName);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_CMD_trace
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

	switch(nArgc)
	{
	case	1:
		{
			xRet = FTM_SHELL_CMD_showTraceConfig();
		}
		break;

	}

	return	xRet;
}


