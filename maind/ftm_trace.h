#ifndef	FTM_TRACE_H_
#define	FTM_TRACE_H_

#include <stdio.h>
#include "ftm_types.h"

FTM_VOID	FTM_LOG_SystemError
(
	const FTM_CHAR_PTR pFormat,
	...
);

FTM_VOID	FTM_LOG_Out
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

#define	LOG(format, ...)			FTM_LOG_Out(0, __func__, __LINE__, __MODULE__, "LOG", format, ## __VA_ARGS__)
#define	TRACE(format, ...)			FTM_LOG_Out(1, __func__, __LINE__, __MODULE__, "TRACE", format, ## __VA_ARGS__)
#define	TRACE_ENTRY()				FTM_LOG_Out(1, __func__, __LINE__, __MODULE__, "TRACE", "ENTRY")
#define	TRACE_EXIT()				FTM_LOG_Out(1, __func__, __LINE__, __MODULE__, "TRACE", "EXIT")
#define	ERROR(errno, format, ...)	FTM_LOG_Out(2, __func__, __LINE__, __MODULE__, "ERROR", format, ## __VA_ARGS__)

#define	ASSERT(x)					{ if (!(x)) { printf("ASSERTED[%s:%d] - %s\n", __func__, __LINE__, #x);}; }
#endif
