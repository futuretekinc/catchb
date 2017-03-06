#ifndef	FTM_TRACE_H_
#define	FTM_TRACE_H_

#include <stdio.h>
#include "ftm_types.h"
#include "cjson/cJSON.h"

typedef struct FTM_TRACE_FIELD_CONFIG_STRUCT
{
	FTM_BOOL	bEnable;
	FTM_UINT32	ulSize;
}	FTM_TRACE_FIELD_CONFIG, _PTR_ FTM_TRACE_FIELD_CONFIG_PTR;

typedef	struct	FTM_TRACE_TYPE_CONFIG_STRUCT
{
	FTM_TRACE_FIELD_CONFIG	xTime;
	FTM_TRACE_FIELD_CONFIG	xFunction;
	FTM_TRACE_FIELD_CONFIG	xLine;
	FTM_TRACE_FIELD_CONFIG	xModule;
	FTM_TRACE_FIELD_CONFIG	xLevel;
	FTM_CHAR				pFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN + 1];
}	FTM_TRACE_TYPE_CONFIG, _PTR_ FTM_TRACE_TYPE_CONFIG_PTR;

typedef	struct	FTM_TRACE_CONFIG_STRUCT
{
	FTM_TRACE_TYPE_CONFIG	xLog;
	FTM_TRACE_TYPE_CONFIG	xTrace;
	FTM_TRACE_TYPE_CONFIG	xError;
}	FTM_TRACE_CONFIG, _PTR_ FTM_TRACE_CONFIG_PTR;

FTM_RET		FTM_TRACE_CONFIG_setDefault
(
	FTM_TRACE_CONFIG_PTR	pConfig
);

FTM_RET		FTM_TRACE_CONFIG_load
(
	FTM_TRACE_CONFIG_PTR	pConfig,
	cJSON _PTR_				pRoot
);

FTM_RET		FTM_TRACE_setConfig
(
	FTM_TRACE_CONFIG_PTR	pConfig
);

FTM_VOID	FTM_TRACE_SystemError
(
	const FTM_CHAR_PTR pFormat,
	...
);

FTM_VOID	FTM_TRACE_Out
(
		  FTM_UINT32	ulOutput,
	const char *		pFunctionName,
		  FTM_UINT32	ulLine,
	const FTM_CHAR_PTR	pModuleName,
	const FTM_CHAR_PTR 	pTitle,
	const FTM_CHAR_PTR 	pFormat,
	...
);

#define	__MODULE__				NULL

#define	LOG(format, ...)			FTM_TRACE_Out(0, __func__, __LINE__, __MODULE__, "LOG", format, ## __VA_ARGS__)
#define	TRACE(format, ...)			FTM_TRACE_Out(1, __func__, __LINE__, __MODULE__, "TRACE", format, ## __VA_ARGS__)
#define	TRACE_ENTRY()				FTM_TRACE_Out(1, __func__, __LINE__, __MODULE__, "TRACE", "ENTRY")
#define	TRACE_EXIT()				FTM_TRACE_Out(1, __func__, __LINE__, __MODULE__, "TRACE", "EXIT")
#define	ERROR(errno, format, ...)	FTM_TRACE_Out(2, __func__, __LINE__, __MODULE__, "ERROR", format, ## __VA_ARGS__)

#define	ASSERT(x)					{ if (!(x)) { printf("ASSERTED[%s:%d] - %s\n", __func__, __LINE__, #x);}; }
#endif
