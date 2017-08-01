#ifndef	FTM_TRACE_H_
#define	FTM_TRACE_H_

#include <stdio.h>
#include "ftm_types.h"
#include "cjson/cJSON.h"

typedef	enum	FTM_TRACE_LEVEL_ENUM
{
	FTM_TRACE_LEVEL_CONSOLE,
	FTM_TRACE_LEVEL_LOG,
	FTM_TRACE_LEVEL_INFO,
	FTM_TRACE_LEVEL_WARN,
	FTM_TRACE_LEVEL_ERROR
}	FTM_TRACE_LEVEL, _PTR_ FTM_TRACE_LEVEL_PTR;

FTM_CHAR_PTR	FTM_TRACE_LEVEL_string
(
	FTM_TRACE_LEVEL	xLevel
);

/////////////////////////////////////////////////////////////////////////////
typedef struct FTM_TRACE_FIELD_CONFIG_STRUCT
{
	FTM_BOOL	bEnable;
	FTM_BOOL	bDynamic;
	FTM_UINT32	ulSize;
}	FTM_TRACE_FIELD_CONFIG, _PTR_ FTM_TRACE_FIELD_CONFIG_PTR;

typedef	struct	FTM_TRACE_TYPE_CONFIG_STRUCT
{
	FTM_BOOL				bEnable;
	FTM_CHAR				pFileName[FTM_FILE_NAME_LEN + 1];
	FTM_TRACE_FIELD_CONFIG	xTime;
	FTM_TRACE_FIELD_CONFIG	xFunction;
	FTM_TRACE_FIELD_CONFIG	xLine;
	FTM_TRACE_FIELD_CONFIG	xModule;
	FTM_TRACE_FIELD_CONFIG	xLevel;
}	FTM_TRACE_TYPE_CONFIG, _PTR_ FTM_TRACE_TYPE_CONFIG_PTR;

typedef	struct	FTM_TRACE_CONFIG_STRUCT
{
	FTM_CHAR				pPath[FTM_PATH_LEN + 1];
	FTM_UINT32				ulFileSize;
	FTM_TRACE_TYPE_CONFIG	xConsole;
	FTM_TRACE_TYPE_CONFIG	xLog;
	FTM_TRACE_TYPE_CONFIG	xInfo;
	FTM_TRACE_TYPE_CONFIG	xWarn;
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

FTM_RET		FTM_TRACE_CONFIG_save
(
	FTM_TRACE_CONFIG_PTR	pConfig,
	cJSON _PTR_				pRoot
);

FTM_RET		FTM_TRACE_CONFIG_show
(
	FTM_TRACE_CONFIG_PTR	pConfig,
	FTM_TRACE_LEVEL			xLevel
);

FTM_RET		FTM_TRACE_setConfig
(
	FTM_TRACE_CONFIG_PTR	pConfig
);

FTM_RET		FTM_TRACE_getConfig
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
		  FTM_TRACE_LEVEL	xLevel,
		  FTM_BOOL			bEnable,
	const FTM_CHAR _PTR_ 	pFunctionName,
		  FTM_UINT32		ulLine,
	const FTM_CHAR_PTR		pModuleName,
	const FTM_CHAR_PTR 		pFormat,
	...
);

FTM_VOID	FTM_TRACE_asserted(FTM_VOID);

FTM_RET	FTM_TRACE_getModuleCount
(
	FTM_UINT32_PTR	pModuleCount
);

FTM_CHAR_PTR	FTM_TRACE_getModuleName
(
	FTM_UINT32	ulIndex
);

FTM_RET	FTM_TRACE_addModuleName
(
	FTM_CHAR_PTR	pModuleName
);
#define	__MODULE__				NULL

#define	OUTPUT(level, format, ...)	FTM_TRACE_Out(level, 				FTM_TRUE, __func__, __LINE__, __MODULE__, format, ## __VA_ARGS__)
#define	LOG(format, ...)			FTM_TRACE_Out(FTM_TRACE_LEVEL_LOG, 	FTM_TRUE, __func__, __LINE__, __MODULE__, format, ## __VA_ARGS__)
#define	INFO(format, ...)			FTM_TRACE_Out(FTM_TRACE_LEVEL_INFO, FTM_TRUE, __func__, __LINE__, __MODULE__, format, ## __VA_ARGS__)
#define	INFO_ENTRY()				FTM_TRACE_Out(FTM_TRACE_LEVEL_INFO, FTM_TRUE, __func__, __LINE__, __MODULE__, "ENTRY")
#define	INFO_EXIT()					FTM_TRACE_Out(FTM_TRACE_LEVEL_INFO, FTM_TRUE, __func__, __LINE__, __MODULE__, "EXIT")
#define	WARN(errno, format, ...)	FTM_TRACE_Out(FTM_TRACE_LEVEL_WARN,	FTM_TRUE, __func__, __LINE__, __MODULE__, format, ## __VA_ARGS__)
#define	ERROR(errno, format, ...)	FTM_TRACE_Out(FTM_TRACE_LEVEL_ERROR,FTM_TRUE, __func__, __LINE__, __MODULE__, format, ## __VA_ARGS__)

#define	ASSERT(x)					{ if (!(x)) { printf("ASSERTED[%s:%d] - %s\n", __func__, __LINE__, #x); FTM_TRACE_asserted();}; }
#endif
