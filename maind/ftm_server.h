#ifndef	FTM_SERVER_H__
#define	FTM_SERVER_H__

#include "ftm_types.h"

typedef	struct	FTM_SERVER_STRUCT
{
	FTM_UINT32	ulBacklog;
	FTM_SOCKET	xSocket;
	FTM_CHAR		pFileName[FTM_PATH_LEN + FTM_FILE_NAME_LEN+1];
}	FTM_SERVER, _PTR_ FTM_SERVER_PTR;

FTM_RET	FTM_SERVER_create
(
	FTM_SERVER_PTR _PTR_ ppServer
);

FTM_RET	FTM_SERVER_destroy
(
	FTM_SERVER_PTR _PTR_ ppServer
);

FTM_RET	FTM_SERVER_open
(
	FTM_SERVER_PTR	pServer,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTM_SERVER_close
(
	FTM_SERVER_PTR	pServer
);

#endif
