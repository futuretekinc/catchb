#ifndef	CATCHB_SERVER_H__
#define	CATCHB_SERVER_H__

#include "catchb_types.h"

typedef	struct	CATCHB_SERVER_STRUCT
{
	CATCHB_UINT32	ulBacklog;
	CATCHB_SOCKET	xSocket;
	CATCHB_CHAR		pFileName[CATCHB_PATH_LEN + CATCHB_FILE_NAME_LEN+1];
}	CATCHB_SERVER, _PTR_ CATCHB_SERVER_PTR;

CATCHB_RET	CATCHB_SERVER_create
(
	CATCHB_SERVER_PTR _PTR_ ppServer
);

CATCHB_RET	CATCHB_SERVER_destroy
(
	CATCHB_SERVER_PTR _PTR_ ppServer
);

CATCHB_RET	CATCHB_SERVER_open
(
	CATCHB_SERVER_PTR	pServer,
	CATCHB_CHAR_PTR		pFileName
);
#endif
