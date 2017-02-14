#ifndef	CATCHB_TRACE_H_
#define	CATCHB_TRACE_H_

#include <stdio.h>
#include "catchb_types.h"

CATCHB_VOID	CATCHB_LOG_SystemError
(
	const CATCHB_CHAR_PTR pFormat,
	...
);

CATCHB_VOID	CATCHB_LOG_Out
(
	const CATCHB_CHAR_PTR 	pFileName,
	const char *			pFunctionName,
		  CATCHB_UINT32		ulLine,
	const CATCHB_CHAR_PTR 	pTitle,
	const CATCHB_CHAR_PTR 	pFormat,
	...
);

#define	TRACE(format, ...)			CATCHB_LOG_Out(NULL, __func__, __LINE__, "TRACE", format, ## __VA_ARGS__)
#define	TRACE_ENTRY()				CATCHB_LOG_Out(NULL, __func__, __LINE__, "TRACE", "ENTRY")
#define	TRACE_EXIT()				CATCHB_LOG_Out(NULL, __func__, __LINE__, "TRACE", "EXIT")
#define	ERROR(errno, format, ...)	CATCHB_LOG_Out(NULL, __func__, __LINE__, "ERROR", format, ## __VA_ARGS__)
#define	LOG(format, ...)			CATCHB_LOG_Out(NULL, __func__, __LINE__, "LOG", format, ## __VA_ARGS__)

#define	ASSERT(x)	
#endif
